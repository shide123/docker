
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string>

#include "SL_Socket_CommonAPI.h"
#include "SyncServerApplication.h"
#include "Config.h"

#include "server.h"


#ifndef WIN32
static const std::string dirname (const std::string &pathname)
{
	std::string::size_type idx = pathname.rfind("/");
	if (idx == std::string::npos)
		return std::string ("./");

	return pathname.substr (0, idx +1);
}
#else
static const std::string dirname(const std::string &pathname)
{
	std::string::size_type idx = pathname.rfind("\\");
	if (idx == std::string::npos)
		return std::string ("./");

	return pathname.substr(0, idx);
}
#endif

static void usage(const char* procname)
{
	printf("usage: %s -h[help]\r\n"
			"    -c [load config file]\r\n",
			procname);
}


int main(int argc, char* argv[])
{
	int  ch;
	char binPath[256], logPath[256], confPath[256];
	char prog[256] = {0};
	std::string conf_file, basename;

	while ((ch = getopt(argc, argv, "hc:")) > 0)
	{
		switch (ch)
		{
		case 'h':
			usage(argv[0]);
			return 0;
		case 'c':
			conf_file = optarg;
			break;
		default:
			usage(argv[0]);
			return EXIT_FAILURE;
		}
	}

	//目录路径
	strcpy(prog, argv[0]);
	char *pfilename = strrchr(prog, '/');
	if (pfilename) {
		basename = ++pfilename;
	}
	sprintf(binPath, "%s", dirname(prog).c_str());
	sprintf(logPath, "%s/../log", binPath);
	sprintf(confPath, "%s/../etc", binPath);
	appInstance->m_strbinpath = binPath;
	appInstance->m_strsynclogpath = logPath;
	appInstance->m_strsynclogpath += "/.synclog";

	//配置文件
	if (conf_file.empty()){
		conf_file = confPath;
		conf_file.append("/");
		conf_file += basename + ".conf";
	}

	if (appInstance->loadconfig(conf_file.c_str()) < 0)
		return EXIT_FAILURE;

	if (appInstance->m_listen_port == 0){
		cerr << "Invalid listen port, syncsvr will exit" << endl;
		return EXIT_FAILURE;
	}

	signal (SIGPIPE, SIG_IGN );
	signal (SIGSEGV, SIG_IGN );
	signal (SIGALRM, SIG_IGN );
	//set daemon Mode
	if (appInstance->m_bdaemon)
		daemon (1, 0);

	//初始化日志线程
	char logfile[256];
	sprintf(logfile,"syncsvr_%d_log", appInstance->m_nserverid);
	appInstance->m_log.init(appInstance->m_nlogmode, appInstance->m_nloglevel, logPath, logfile, "txt", 2);
	CLogThread::Instance(&(appInstance->m_log), appInstance->m_nusesyslog);

	LOG_PRINT(log_info, "syncsvr is running");
	LOG_PRINT(log_info, "log name[%s] mode[%d:%d]",logfile, appInstance->m_nlogmode, appInstance->m_nloglevel);

	//应用程序初始化
	if (appInstance->init() < 0)
		return EXIT_FAILURE;

	boost::asio::io_service io_service;

    server svr(appInstance->m_listen_port, 1);
    svr.setsvrid(appInstance->m_nserverid);
    appInstance->m_pSyncsvr = &svr;
    appInstance->connect_sync_server(io_service, appInstance->m_pSyncsvr);
	boost::thread netthread(boost::bind(&boost::asio::io_service::run, &io_service));
    svr.run();
    netthread.join();

    return 0;

}

