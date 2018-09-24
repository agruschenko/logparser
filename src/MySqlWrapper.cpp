#include "MySqlWrapper.h"
#include <sstream>
#include <iostream>

#define MAX_DB_PUSH_ITEMS 500

MySqlWrapper::MySqlWrapper()
{
	
}

MySqlWrapper::~MySqlWrapper()
{
	disconnect();
}

bool MySqlWrapper::connect(const std::string& host, const std::string& user, const std::string& pass, int port, const std::string& dbName)
{
	bool res = setupDB(host, user, pass, port, dbName);
	res &= init(host, user, pass, port, dbName);

	return res;
}

bool MySqlWrapper::setupDB(const std::string& host, const std::string& user, const std::string& pass, int port, const std::string& dbName)
{
	MySql::MySql mysqlClient;
	bool ret = mysqlClient.connect(host.c_str(), user.c_str(), pass.c_str(), NULL, port, nullptr, 0, 1);
	if (!ret)
	{
		std::string s = mysqlClient.get_last_error();
		std::cout << "Failed SQL: "  << mysqlClient.get_last_error() << std::endl;
		return false;
	}

	std::stringstream query;
	MySql::Result sqlRet;

	query << "CREATE DATABASE IF NOT EXISTS " << dbName << ";";
	sqlRet = mysqlClient.query(query.str());
	if (!checkResult(mysqlClient, sqlRet))
	{
      std::cout << "Failed SQL: " << query.str() << " error: " << mysqlClient.get_last_error();
      return false; 
	}
   
	query.str(std::string());
	query.clear();

	query << "GRANT ALL ON " << dbName << ".* TO '" << user << "'@'localhost';";
	sqlRet = mysqlClient.query(query.str());
	if (!checkResult(mysqlClient, sqlRet))
	{
		std::cout << "Failed SQL: " << query.str() << " error: " << mysqlClient.get_last_error() << std::endl;
		return false;
	}
	
	return true;
}

bool MySqlWrapper::init(const std::string& host, const std::string& user, const std::string& pass, int port, const std::string& dbName)
{
	bool ret = _mySqlClient.connect(host.c_str(), user.c_str(), pass.c_str(), dbName.c_str(), port, nullptr, CLIENT_MULTI_STATEMENTS);
	if (!ret)
	{
		return ret;
	}

	std::stringstream query;
	MySql::Result sqlRet;

	dropTable("log_items");
	query << "CREATE TABLE IF NOT EXISTS log_items (id BIGINT(50) UNSIGNED NOT NULL, date varchar(50) COLLATE utf8_unicode_ci NOT NULL, ip varchar(50) COLLATE utf8_unicode_ci NOT NULL, UNIQUE INDEX id (id)) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;";
	sqlRet = _mySqlClient.query(query.str());
	if (!checkResult(_mySqlClient, sqlRet))
	{
		std::cout << "Failed SQL: " << query.str() << " error: " << _mySqlClient.get_last_error() << std::endl;
		return false;
	}

	query.str(std::string());
	query.clear();

	dropTable("blocked_items");
	query << "CREATE TABLE IF NOT EXISTS blocked_items (id BIGINT(50) UNSIGNED NOT NULL, ip varchar(50) COLLATE utf8_unicode_ci NOT NULL, msg varchar(256) COLLATE utf8_unicode_ci, UNIQUE INDEX id (id)) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;";
	sqlRet = _mySqlClient.query(query.str());
	if (!checkResult(_mySqlClient, sqlRet))
	{
		std::cout << "Failed SQL: " << query.str() << " error: " << _mySqlClient.get_last_error() << std::endl;
		return false;
	}

	_mySqlClient.flush_result();

   return true;
}

bool MySqlWrapper::pushLogData(const ILogDataHandler::LOG_DATA& data)
{
	if (data.size() == 0)
		return false;

	bool result = true;
	size_t i = 0;
	do
	{
		size_t startIdx = i * MAX_DB_PUSH_ITEMS;
		size_t endIdx = i == data.size() / MAX_DB_PUSH_ITEMS ? data.size() : startIdx + MAX_DB_PUSH_ITEMS;

		result &= pushLogDataInternal(data, startIdx, endIdx);
	} while (i++ < data.size() / MAX_DB_PUSH_ITEMS);

	return result;
}

bool MySqlWrapper::pushLogDataInternal(const ILogDataHandler::LOG_DATA& data, size_t begin, size_t end)
{
	auto query = logDataQuery(data, begin, end);

	MySql::Result result = _mySqlClient.query(query);
	if (!checkResult(_mySqlClient, result))
	{
		std::cout << "Failed SQL: " << _mySqlClient.get_last_error() << std::endl;
		return false;
	}

	_mySqlClient.flush_result();
	return true;
}

bool MySqlWrapper::pushBlockedData(const ILogDataHandler::LOG_DATA& data)
{
	if (data.size() == 0)
		return false;

	bool result = true;
	size_t i = 0;
	do
	{
		size_t startIdx = i * MAX_DB_PUSH_ITEMS;
		size_t endIdx = i == data.size() / MAX_DB_PUSH_ITEMS ? data.size() : startIdx + MAX_DB_PUSH_ITEMS;

		result &= pushBlockedDataInternal(data, startIdx, endIdx);
	} while (i++ < data.size() / MAX_DB_PUSH_ITEMS);

	return result;
}

bool MySqlWrapper::pushBlockedDataInternal(const ILogDataHandler::LOG_DATA& data, size_t begin, size_t end)
{
	auto query = blockedDataQuery(data, begin, end);

	MySql::Result result = _mySqlClient.query(query);
	if (!checkResult(_mySqlClient, result))
	{
		std::cout << "Failed SQL: " << query << " error: " << _mySqlClient.get_last_error() << std::endl;
		return false;
	}

	_mySqlClient.flush_result();

	return true;
}

std::string MySqlWrapper::logDataQuery(const ILogDataHandler::LOG_DATA& data, size_t begin, size_t end) const
{
	if (data.size() == 0)
		return "";

	std::stringstream query;
	query << "INSERT INTO log_items (id, date, ip) VALUES ";

	for(auto i = begin; i < end; i++)
	{
		const LogData& logData = (const LogData&)*data[i];
		query << "(" << (long)logData.id() << ", '" << logData.date() << "', '" << logData.ip() << "')";

		if (i == end - 1)
			query << ";";
		else
			query << ",";
	}

	return query.str();
}

std::string MySqlWrapper::blockedDataQuery(const ILogDataHandler::LOG_DATA& data, size_t begin, size_t end) const
{
	if (data.size() == 0)
		return "";

	std::stringstream query;
	std::stringstream msg;
	size_t counter = 0;

	query << "INSERT INTO blocked_items(id, ip, msg) VALUES ";
	for (auto i = begin; i < end; i++)
	{
		const FilterData<LogData>& blockedData = (const FilterData<LogData>&)*data[i];
		msg.str(std::string());
		msg.clear();

		msg << "Blocked due to threshold. Requests counter: " << blockedData.trigger();
		query << "(" << (long)blockedData.id() << ", '" << blockedData.ip() << "', '" << msg.str() << "')";

		if (i == end - 1)
			query << ";";
		else
			query << ",";
	}

	return query.str();
}

bool MySqlWrapper::getLogItem(long id)
{
	std::ostringstream ss;
	ss << "SELECT id, date, ip FROM log_items WHERE";
	bool query_empty = true;

	if (id > 0)
	{
		if (!query_empty)
			ss << " AND";

		ss << " id = '" << id << "'";
		query_empty = false;
	}

	ss << ";";

	MySql::Result queryResult = _mySqlClient.query(ss.str());
	if (!checkResult(_mySqlClient, queryResult))
	{
		std::cout << "Failed SQL SELECT by id: " << id << " error: " << _mySqlClient.get_last_error() << std::endl;
		return false;
	}

	while (MySql::Row row = queryResult.fetch_row())
	{
		std::cout << "Result: "
			<< "id " << std::stoi(row[0])
			<< ", date " << row[1]
			<< ", ip " << row[2]
			<< std::endl;
	}

	return true;
}

void MySqlWrapper::disconnect()
{
	_mySqlClient.disconnect();
}

bool MySqlWrapper::addLogItem(const LogData& item)
{
	std::stringstream query;
	query << "INSERT INTO log_items (id, date, ip) "
	            << "VALUES (" << (long)item.id()
				<< ", '" << item.date()
				<< "', '" << item.ip() << "');";

	MySql::Result result = _mySqlClient.query(query.str());
	if (!checkResult(_mySqlClient, result))
	{
		std::cout << "Failed SQL: " << query.str() << " error: " << _mySqlClient.get_last_error() << std::endl;
		return false;
	}

	_mySqlClient.flush_result();
	return true;
}

bool MySqlWrapper::addBlockedItem(const FilterData<LogData>& item)
{
	std::stringstream msg;
	msg << "Blocked due to threshold. Requests counter: " << item.trigger();

	std::stringstream query;
	query << "INSERT INTO blocked_items (id, ip, msg) "
		<< "VALUES (" << (long)item.id()
		<< ", '" << item.ip()
		<< "', '" << msg.str() << "');";

	MySql::Result result = _mySqlClient.query(query.str());
	if (!checkResult(_mySqlClient, result))
	{
		std::cout << "Failed SQL: " << query.str() << " error: " << _mySqlClient.get_last_error();
		return false;
	}

	_mySqlClient.flush_result();
	return true;
}

void MySqlWrapper::dropTable(const std::string& table)
{
	std::stringstream query;
	query << "DROP TABLE IF EXISTS " << table << ";";

	MySql::Result result = _mySqlClient.query(query.str());
	if (!checkResult(_mySqlClient, result))
	{
		std::cout << "Failed SQL: " << query.str() << " error: " << _mySqlClient.get_last_error();
	}

	_mySqlClient.flush_result();
}

bool MySqlWrapper::checkResult(MySql::MySql& client, const MySql::Result& result)
{
	if (result)
		return true;

	if (client.get_last_errno() == 0)
		return true;

	return false;
}