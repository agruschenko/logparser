#include "LogDataHandler.h"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <map>
#include <algorithm>

///////////////////////////////////////////////////////////
// LogData
bool LogData::deserialize(const std::string& data)
{	
	// Data format: Date,  IP,  Request,  Status,  User  Agent
	// Date  Format:  "yyyy-MM-dd  HH:mm:ss.SSS"
	std::stringstream dataStream(data);
	std::string token;

	for(int i = 0; i < LogData::Field::Count; ++i)
	{
		if(!std::getline(dataStream, token, LogData::delimiter))
		{
			std::cout << "Wrong log format! [" << data << "]" << std::endl;
			_data.clear();
			break;			
		}

		_data.push_back(token);
	}

	return _data.size() == LogData::Field::Count;
}

std::string LogData::description() const
{
	if (_data.empty())
		return "";

	std::stringstream ss;
	ss << _data[IP];

	return ss.str();
}

time_t LogData::timestamp() const
{
	if (_timestamp == -1)
	{
		std::string date = _data[LogData::Field::Date];
		std::tm tm = {};
		//strptime(date.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
		std::stringstream ss(date);
		ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

		_timestamp = std::mktime(&tm);
	}

	return _timestamp;
}

///////////////////////////////////////////////////////////
// LogDataFilter
bool LogDataFilter::filter(const ILogData& data) const
{
	bool result = false;

	time_t timestamp = data.timestamp();
	if (timestamp >= _from && timestamp <= _to)
		result = true;

	return result;
}

bool LogDataFilter::filter(const ILogDataHandler::LOG_DATA& data, ILogDataHandler::LOG_DATA& filteredData) const
{
	if ((_from <= 0 || _to <= 0) || _from > _to || _threshold <= 0)
	{
		return false;
	}

	std::map<std::string, int> thresholdData;
	for (size_t i = 0; i < data.size(); ++i)
	{
		if (filter(*data[i]))
		{
			auto logData = (const LogData&)*data[i];
			auto it = thresholdData.find(logData.ip());

			if (it == thresholdData.end())
			{
				thresholdData.insert(std::make_pair(logData.ip(), 1));
			}
			else
			{
				it->second++;
			}
		}
	}

	for (auto it = thresholdData.begin(); it != thresholdData.end(); ++it)
	{
		if (it->second >= _threshold)
		{
			std::string ip = it->first;
			auto iter = std::find_if(data.begin(), data.end(),
				[ip](std::shared_ptr<ILogData> logDataPtr)->bool {return ip == ((const LogData&)(*logDataPtr)).ip();});

			if (iter != data.end())
			{
				FilterData<LogData> filteredItem(it->second);
				filteredItem = (const LogData&)**iter;

				filteredData.emplace_back(std::shared_ptr<FilterData<LogData>>(std::make_shared<FilterData<LogData>>(filteredItem)));
			}
		}	
	}

	return true;
}

///////////////////////////////////////////////////////////
// LogDataHandler
int LogDataHandler::_currentId = 1000;

bool LogDataHandler::parse(std::ifstream& file, LOG_DATA& data)
{
	data.clear();
	
	if(file.is_open())
	{
		std::string rawData;
		while (std::getline(file, rawData))
		{
			LogData logItem;
			if(logItem.deserialize(rawData))
			{
				logItem.setId(_currentId++);
				data.emplace_back(std::shared_ptr<LogData>(std::make_shared<LogData>(logItem)));
			}
		}
	}

	return data.size() > 0;
}

bool LogDataHandler::parse(std::ifstream& file, const ILogDataFilter* filter, LOG_DATA& data, LOG_DATA& filtered)
{
	bool result = parse(file, data);

	if (result && filter)
	{
		result = filter->filter(data, filtered);
	}

	return result;
}