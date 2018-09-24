CC=g++
IDIR=/usr/include/mysql
CFLAGS=-I$(IDIR)
#LDIR=/usr/lib/mysql
#LFLAGS=-L$(LDIR)
#$(LDIR) -lmysqlclient

logparser: main.cpp AppDelegate.cpp LogParser.cpp LogDataHandler.cpp
	$(CC) -std=c++14 $(CFLAGS) main.cpp AppDelegate.cpp LogParser.cpp LogDataHandler.cpp MySqlWrapper.cpp `mysql_config --cflags --libs` -o logparser

clean:
	rm -f logparser

test: clean logparser
	echo "Running logparser test"
	./logparser --startDate=2017-01-01.10:00:00  --duration=daily  --threshold=250
