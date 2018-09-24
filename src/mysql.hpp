#ifndef MYSQL_HPP
#define MYSQL_HPP

#pragma once

#include <string>
#include <string.h>
#include <unistd.h>
#include <mysql.h>

namespace MySql
{

class Result;
class MySql
{
public:
	MySql() : handle(mysql_init(0))
	{
		if (handle != nullptr)
		{
			mysql_options(handle, MYSQL_SET_CHARSET_NAME, "utf8");
			mysql_options(handle, MYSQL_INIT_COMMAND, "SET NAMES utf8");
		}
	}

	~MySql()
	{
		if (handle != nullptr)
		{
			mysql_close(handle);
		}
	}

	bool connect(const char* host, const char* user, const char* password,
			const char* db, unsigned int port, const char* unix_socket, unsigned long client_flag)
	{
		my_bool recon = true;
		mysql_options(handle, MYSQL_OPT_RECONNECT, &recon);
		MYSQL* h = mysql_real_connect(handle, host, user, password,
									  db, port, unix_socket, client_flag);
		return h ? true : false;
	}
	
	bool connect(const char* host, const char* user, const char* password,
			const char* db, unsigned int port, const char* unix_socket, unsigned long client_flag, int retries)
	{
		 bool ret = false;
		 int sleepTime = 1;
		 while(true)
		 {
			ret = connect(host, user, password, db, port, unix_socket, client_flag);
			if (ret || --retries <= 0)
			{
				break;
			}
			
			sleep(sleepTime);
			sleepTime *= 2;
		 }

		 return ret;
	}
	
	void disconnect()
	{
	   if (handle != nullptr)
		{
			mysql_close(handle);
			handle = nullptr;
		}
	}

	inline Result query(const std::string& s);
	inline Result query(const char* s, unsigned long n);

	inline int next_result();
	inline Result use_result();
	inline const char* get_last_error() const
	{
		if (handle)
		{
		   return mysql_error(handle);
		}
		
		return nullptr;
	};

	inline unsigned int get_last_errno() const 
	{
		if (handle)
		{
			return mysql_errno(handle);
		}

		return 0;
	 }       
	
	inline void flush_result();
	inline void flush_result(Result& result);

private:
	MYSQL* handle;
};

class Row
{
public:
	Row() : row(nullptr), lengths(nullptr) {}
	Row(MYSQL_ROW row, unsigned long* lengths) : row(row), lengths(lengths) {}

	Row(Row&& x) : row(x.row), lengths(x.lengths) {
		x.row     = nullptr;
		x.lengths = nullptr;
	}

	Row& operator=(Row&& x) {
		using std::swap;
		swap(*this, x);
		return *this;
	}

	operator bool() const {
		return !!row;
	}

	std::string operator[](size_t n) {
		return std::string(row[n], lengths[n]);
	}
	
	const char* get_data(size_t n) 
	{
	   return row[n];
	}
	
	size_t get_data_len(size_t n) 
	{
	   return lengths[n];
	}
	
private:
	MYSQL_ROW row;
	unsigned long *lengths;
};

class Result
{
public:
	Result(MYSQL_RES* res=nullptr) : res(res) {}
	Result(Result&& r)
	{
		res = r.res;
		r.res = 0;
	}

	~Result()
	{
		if (res) mysql_free_result(res);
	}

	operator bool() const {
		return !!res;
	}

	Result& operator=(Result&& r) {
		mysql_free_result(res);
		res = r.res;
		r.res = 0;
		return *this;
	}

	Result& operator=(const Result&) = delete;
	Result(const Result&) = delete;

	Row fetch_row()
	{
		MYSQL_ROW row = mysql_fetch_row(res);
		unsigned long* lengths = mysql_fetch_lengths(res);
		return Row{row, lengths};
	}

	inline Row next() {
		return fetch_row();
	}
	
private:
	MYSQL_RES* res;
};

inline Result MySql::use_result()
{
    MYSQL_RES* result = mysql_use_result(handle);
    if (!result)
    {
        return Result{};
    }
    return Result{result};
}

inline int MySql::next_result()
{
    int x = mysql_next_result(handle);
    return x;
}

inline Result MySql::query(const char* s, unsigned long n)
{
    int x = mysql_real_query(handle, s, n);
    if (x != 0)
    {
        return Result{};
    }
    return use_result();
}

inline Result MySql::query(const std::string& s)
{
    return query(s.c_str(), s.length()); 
}

inline void MySql::flush_result()
{
	while(!next_result())
	{
		use_result();
	}
}

inline void MySql::flush_result(Result& result)
{
	while (Row row = result.fetch_row()) {}
	flush_result();
}

}
#endif
