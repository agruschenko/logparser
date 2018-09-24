#pragma once

#include "interfaces.h"
#include <ctime>

class LogData : public ILogData
{
	typedef std::vector<std::string> RAW_DATA;
	static const char delimiter = '|';
	enum Field { Date = 0, IP = 1, Request = 2, Status = 3, UserAgent = 4, Count = 5 };

public:
	LogData(){}
	virtual ~LogData(){}

	bool deserialize(const std::string& data) override;
	std::string description() const override;
	time_t timestamp() const override;

	void setId(int id) { _id = id; }
	int id() const { return _id; }

	std::string date() const { return _data[Date]; }
	std::string ip() const { return _data[IP]; }

	void operator = (const LogData& src)
	{
		_data = src._data;
		_id = src._id;
		_timestamp = src._timestamp;
	}

private:
	RAW_DATA _data;
	int _id = 0;
	mutable time_t _timestamp = -1;
};

template <class T>
class FilterData : public T
{
public:
	FilterData(int trigger) : _trigger(trigger) {}

	void operator = (const T& src)
	{
		(T&)*this = src;
	}

	int trigger() const {
		return _trigger;
	}

private:
	int _trigger = 0;
};

class LogDataFilter : public ILogDataFilter
{
public:
	LogDataFilter(time_t from, time_t to, int threshold)
		: _from(from), _to(to), _threshold(threshold) {}
	virtual ~LogDataFilter() {}

	bool filter(const ILogData& data) const override;
	bool filter(const ILogDataHandler::LOG_DATA& data, ILogDataHandler::LOG_DATA& filteredData) const override;

private:
	time_t _from = -1;
	time_t _to = -1;
	int _threshold = 0;
};

class LogDataHandler : public ILogDataHandler
{
	static int _currentId;

public:
	LogDataHandler(){}
	virtual ~LogDataHandler(){}

	bool parse(std::ifstream& file, LOG_DATA& data) override;
	bool parse(std::ifstream& file, const ILogDataFilter* filter, LOG_DATA& data, LOG_DATA& filtered) override;
};