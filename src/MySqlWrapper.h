#pragma once
#include "LogDataHandler.h"
#include "mysql.hpp"


class MySqlWrapper
{
public:
	MySqlWrapper();
	~MySqlWrapper();
	
	bool connect(const std::string& host, const std::string& user, const std::string& pass, int port, const std::string& dbName);
	void disconnect();
	
	bool pushLogData(const ILogDataHandler::LOG_DATA& data);
	bool pushBlockedData(const ILogDataHandler::LOG_DATA& data);

	bool addLogItem(const LogData& item);
	bool addBlockedItem(const FilterData<LogData>& item);
	bool getLogItem(long id);

protected:
	bool setupDB(const std::string& host, const std::string& user, const std::string& pass, int port, const std::string& dbName);
	bool init(const std::string& host, const std::string& user, const std::string& pass, int port, const std::string& dbName);
	bool checkResult(MySql::MySql& client, const MySql::Result& result);
	void dropTable(const std::string& table);
	bool pushLogDataInternal(const ILogDataHandler::LOG_DATA& data, size_t begin, size_t end);
	bool pushBlockedDataInternal(const ILogDataHandler::LOG_DATA& data, size_t begin, size_t end);

	std::string logDataQuery(const ILogDataHandler::LOG_DATA& data, size_t begin, size_t end) const;
	std::string blockedDataQuery(const ILogDataHandler::LOG_DATA& data, size_t begin, size_t end) const;
	
private:
   MySql::MySql _mySqlClient;
};