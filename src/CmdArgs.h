#pragma once

#include "defines.h"
#include <vector>
#include <sstream>
#include <iostream>
#include <iomanip>


class CmdArgs
{
	typedef std::vector<std::string> ARGS;
	
public:
	CmdArgs(int argc, char **argv)
	{
        for (int i = 1; i < argc; ++i)
		{
			inputArgs.push_back(std::string(argv[i]));
		}
    }

    virtual std::string getOption(const std::string &option) const
	{
		ARGS::const_iterator it = std::find_if(inputArgs.begin(), inputArgs.end(),
			[option](const std::string& cmd)->bool {return cmd.find(option) != std::string::npos; });

		if (it != inputArgs.end())
		{
            return *it;
        }
            
		static const std::string empty_string("");
        return empty_string;
    }

protected:
    ARGS inputArgs;
};

class LogOptions : public CmdArgs
{
	static const char delimiter = '=';

public:
	LogOptions(int argc, char **argv) : CmdArgs(argc, argv) {}

	std::string getOption(const std::string &option) const override
	{
		std::string result;
		auto command = CmdArgs::getOption(option);
		
		if (!command.empty())
		{
			size_t pos = command.find(delimiter);
			if (pos != std::string::npos)
			{
				result = command.substr(pos+1);
			}
		}

		return result;
	}

	time_t getFromTime() const
	{
		std::string startDate = getOption(CMD_START_DATE);

		std::tm tm = {};
		//strptime(date.c_str(), "%Y-%m-%d.%H:%M:%S", &tm);
		std::stringstream ss(startDate);
		ss >> std::get_time(&tm, "%Y-%m-%d.%H:%M:%S");

		return !ss.fail() ? std::mktime(&tm) : -1;
	}

	time_t getToTime() const
	{
		time_t from = getFromTime();

		return from == -1 ? -1 : from + duration();
	}

	unsigned int getThreshold() const
	{
		std::string threshold = getOption(CMD_THRESHOLD);

		return !threshold.empty() ? std::stoi(threshold) : 0;
	}

protected:
	time_t duration() const
	{
		time_t result = 0;
		std::string duration = getOption(CMD_DURATION);

		if (cmpNoCase::compareNoCase(duration, OPTION_HOURLY))
		{
			result = 60*60;
		}
		else if (cmpNoCase::compareNoCase(duration, OPTION_DAILY))
		{
			result = 60 * 60 * 24;
		}

		return result;
	}
};