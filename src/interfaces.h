#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <memory>

class ILogData
{
public:
	virtual bool deserialize(const std::string& data) = 0; 
	virtual std::string description() const = 0;
	virtual time_t timestamp() const = 0;
};

class ILogDataFilter;
class ILogDataHandler
{
public:
	typedef std::vector<std::shared_ptr<ILogData>> LOG_DATA;

	virtual bool parse(std::ifstream& file, LOG_DATA& data) = 0;
	virtual bool parse(std::ifstream& file, const ILogDataFilter* filter, LOG_DATA& data, LOG_DATA& filtered) = 0;
};

class ILogDataFilter
{
public:
	virtual bool filter(const ILogData& data) const = 0;
	virtual bool filter(const ILogDataHandler::LOG_DATA& data, ILogDataHandler::LOG_DATA& filteredData) const = 0;
};



