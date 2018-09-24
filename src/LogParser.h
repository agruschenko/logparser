#pragma once

#include "LogDataHandler.h"

class LogParser
{

public:
	LogParser();
	~LogParser();
	
	bool initialize();
	void uninitialize();
	
	bool parse(const std::string& filename, ILogDataHandler::LOG_DATA& data);
	bool parse(const std::string& filename, time_t from, time_t to, int threshold, ILogDataHandler::LOG_DATA& data, ILogDataHandler::LOG_DATA& filteredData);

	bool open(const std::string& filename);
	void close();

protected:
	ILogDataHandler* createDataHandler() const;
	
private:
	std::ifstream _file;
	ILogDataHandler* _handler = nullptr;
};