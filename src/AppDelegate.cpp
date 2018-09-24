#include "AppDelegate.h"
#include "defines.h"
#include "CmdArgs.h"
#include "LogParser.h"
#include "MySqlWrapper.h"

using namespace std;

AppDelegate::AppDelegate()
{
}

AppDelegate::~AppDelegate()
{	
}

int AppDelegate::run(int argc, char** argv)
{
	LogOptions options(argc, argv);
	if(options.getFromTime() == -1 || options.getToTime() == -1 || options.getThreshold() == 0)
	{
		
		std::cout << "Wrong arguments: (--startDate=yyyy-MM-dd.HH:mm:ss  --duration=daily/hourly  --threshold=10)" << std::endl;
		return 0;
	}

	LogParser logParser;
	MySqlWrapper mySql;

	ILogDataHandler::LOG_DATA allData;
	ILogDataHandler::LOG_DATA blockedData;
	typedef FilterData<LogData> BLOCKED_IP;

	bool parseResult = logParser.parse(LOG_FILE, options.getFromTime(), options.getToTime(), options.getThreshold(), allData, blockedData);
	bool connectDbResult = mySql.connect(DB_SRV_HOST, DB_SRV_USER, DB_SRV_PSW, DB_SRV_PORT, DB_NAME);

	if(parseResult && !connectDbResult)
	{
		std::string pass = PassInput::getpass("Enter mySQL server pass: ", true);
		if(pass != DB_SRV_PSW)
			connectDbResult = mySql.connect(DB_SRV_HOST, DB_SRV_USER, pass, DB_SRV_PORT, DB_NAME);
	}

	if (parseResult && connectDbResult)
	{
		mySql.pushLogData(allData);
		mySql.pushBlockedData(blockedData);
	}

	for (auto blockedItem : blockedData)
	{
		BLOCKED_IP* blockedIp = dynamic_cast<BLOCKED_IP*>(&*blockedItem);
		if (blockedIp)
		{
			std::cout << blockedIp->description() << " Blocked due to threshold. Requests counter: " << blockedIp->trigger() << std::endl;
		}
	}

	std::cout << "count " << allData.size() << std::endl;
	return 0;
}