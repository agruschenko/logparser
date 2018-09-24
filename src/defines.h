#pragma once

#include <algorithm>
#include <cctype>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>

#define CMD_START_DATE 	"--startDate="
#define CMD_DURATION   	"--duration="
#define CMD_THRESHOLD 	"--threshold="

#define LOG_FILE		"../logs/access.log"

#define DB_NAME			"logDB"
#define DB_SRV_HOST		"localhost"
#define DB_SRV_USER		"root"
#define DB_SRV_PSW		""
#define DB_SRV_PORT		3306

static const std::string OPTION_HOURLY = "HOURLY";
static const std::string OPTION_DAILY = "DAILY";

//////////////////////////////////////////////////////////////////////////////
// helpers

class cmpNoCase
{
public:
	static bool compareChar(const char & c1, const char & c2)
	{
		if (c1 == c2)
			return true;
		else if (std::toupper(c1) == std::toupper(c2))
			return true;
		return false;
	}

	static bool compareNoCase(const std::string & str1, const std::string &str2)
	{
		return ((str1.size() == str2.size()) &&
			std::equal(str1.begin(), str1.end(), str2.begin(), &compareChar));
	}
};

class PassInput
{
public:
	static int getch()
	{
		int ch;
		struct termios t_old, t_new;

		tcgetattr(STDIN_FILENO, &t_old);
		t_new = t_old;
		t_new.c_lflag &= ~(ICANON | ECHO);
		tcsetattr(STDIN_FILENO, TCSANOW, &t_new);

		ch = getchar();

		tcsetattr(STDIN_FILENO, TCSANOW, &t_old);
		return ch;
	}

	static std::string getpass(const char *prompt, bool show_asterisk=true)
	{
		const char BACKSPACE = 127;
		const char RETURN = 10;

		std::string password;
		unsigned char ch = 0;

		std::cout << prompt << std::endl;

		while((ch=getch())!=RETURN)
		{
			if(ch==BACKSPACE)
			{
				if(password.length()!=0)
				{
					if(show_asterisk)
						std::cout <<"\b \b";

					password.resize(password.length()-1);
				}
			}
			else
			{
				password += ch;
				if(show_asterisk)
					std::cout << '*';
			}
		}
		
		std::cout << std::endl;
		return password;
	}	
};

