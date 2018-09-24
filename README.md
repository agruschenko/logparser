# logparser

Tool that  parses  web  server  access  log  file (../logs/access.log),  loads  the  log  to  MySQL  and  checks  if  a  given  IP  makes  more  than  a  certain  number  of  requests  for  the  given  duration. 

1. Precondition (install build tools and mySQL client/server):
	- sudo apt-get install build-essential
	- sudo apt-get install libmysqlclient-dev
	- sudo apt-get install mysql-server

2. Build application:
	- run make
		- [clean] 	- clean solution
		- [test]  	- rebuild and test tool

3. Run tool:
	- Command line arguments
		[--startDate] 	- starting date "yyyy-MM-dd.HH:mm:ss".
		[--duration] 	- duration "hourly" or "daily".
		[--threshold]	- request count
	 (ex. ./logparser  --startDate=2017-01-01.13:00:00  --duration=hourly  --threshold=100)


Enjoy!
