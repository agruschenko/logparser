#include "LogParser.h"
#include <ctime>
#include <iostream>

LogParser::LogParser()
{
	initialize();
}

LogParser::~LogParser()
{
	uninitialize();
}

bool LogParser::initialize()
{
	_handler = createDataHandler();
	return _handler != nullptr;
}

void LogParser::uninitialize()
{
	close();

	delete _handler;
	_handler = nullptr;
}

ILogDataHandler* LogParser::createDataHandler() const
{
	return new LogDataHandler();
}

bool LogParser::open(const std::string& filename)
{
	close();

	_file.open(filename);
	return _file.is_open();
}

bool LogParser::parse(const std::string& filename, ILogDataHandler::LOG_DATA& data)
{
	ILogDataHandler::LOG_DATA dummy;
	return (open(filename) && _handler) ? _handler->parse(_file, nullptr, data, dummy) : false;
}

bool LogParser::parse(const std::string& filename, time_t from, time_t to, int threshold, ILogDataHandler::LOG_DATA& data, ILogDataHandler::LOG_DATA& filteredData)
{
	LogDataFilter filter(from, to, threshold);
	return (open(filename) && _handler) ? _handler->parse(_file, &filter, data, filteredData) : false;
}

void LogParser::close()
{
	_file.close();
}


