#include "Application.h"
#include <assert.h>
#include "tcpserver.h"
#include "utils.h"
#include "message_vchat.h"
#include <string>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <thrift/TProcessor.h>
using namespace apache::thrift;
int Application::m_interval_time = 100;

using namespace std;
io_service_pool Application::m_io_service_pool(MAX_IO_SERVICE_POOL_NUM);

static const std::string dirname(const std::string &pathname)
{
	std::string::size_type idx = pathname.rfind("/");
	if (idx == std::string::npos)
		return std::string("./");
	return pathname.substr(0, idx);
}

static const std::string procname(const std::string &pathname)
{
	std::string::size_type idx = pathname.rfind("/");
	if (idx == std::string::npos)
		return std::string("./");
	return pathname.substr(idx + 1, pathname.length() - idx - 1);
}

void Application::saveMasterPid()
{
	master_pid = getpid();
	FILE* fp = fopen(pid_file_path.c_str(),"w+");
	if(fp != NULL)
	{
		char buf[128];
		sprintf(buf, "%d", master_pid);
		fputs(buf, fp);
		fclose(fp);
	}
	else
	{
		LOG_PRINT(log_error,"======open file fail!!! %s   ====\n",pid_file_path.c_str());
	}
}

int Application::getMasterPid()
 {
	int pid = 0;
	if (access(pid_file_path.c_str(),F_OK) !=0)
	{
		return pid;
	}
	FILE* fp = fopen(pid_file_path.c_str(),"r+");
	char buf[128];
	if(fp != NULL)
	{
		if(fgets(buf,128,fp)==NULL)
		{
			fclose(fp);
			return -1;
		}
		sscanf(buf, "%d", &pid);
		fclose(fp);
	}
	else
	{
		printf("open file fail!!! %s   ====\n",pid_file_path.c_str());
		return -1;
	}
	return pid;
 }

Application* Application::app = 0;
Application::Application(void)
	: m_bdaemon(0)
	, m_procnum(1)
	, m_nserverid(0)
	, m_listen_port(0)
	, m_workerEvent(NULL)
	, m_bdebug(true)
{
	m_timer		= NULL;
	m_log			= NULL;
	m_reloadable	 = true;
	app = this;
	m_stdstatusinfo = false;
	m_bAuth			= false;
	m_bStarting		= false;
}
Application::Application(string protoname,int port,IWorkerEvent* event)
	: m_bdaemon(0)
	, m_procnum(1)
	, m_nserverid(0)
	, m_listen_port(0)
	, m_workerEvent(NULL)
	, m_bdebug(true)

{
	m_protoname = protoname;
	m_svr.init(port, -1,1, m_protoname, event);
	m_svr.setsvrid(m_nserverid);
	m_svr.start(-1);
	m_timer		= NULL;
	m_log		= NULL;
	m_bStarting	= false;
}

Application::~Application(void)
{
	if(m_workerEvent)
		m_workerEvent->onWorkerStop(this);

	if (m_log) {
		delete m_log;
		m_log = NULL;
	}

	if(m_timer)
	{
		delete m_timer;
		m_timer = NULL;
	}
}

int Application::runAll(int argc, char* argv[])
{
	m_argc = argc;
	m_argv = argv;
	parseCommand();
	init();
	//set daemon Mode
	if (m_bdaemon)
	{
		daemon (1, 0);
	}
	saveMasterPid();
	m_max_conns = m_max_conns == 0?1000:m_max_conns;
	printf("maxconns:%d\n",m_max_conns);
	m_svr.init(m_listen_port, m_procnum,1 , m_protoname,m_workerEvent,m_max_conns);
	m_svr.setsvrid(m_nserverid);
	if(m_listen_rpcport)
	{
		m_rpcsvr.init(m_listen_rpcport);
	}
	installSignal();
	cout<<m_listen_port<<endl;
	forkWorkers();
	monitorWorkers();
	printf("over\n");
	return 0;
}

Application * Application::getInstance()
{
	//static Application m_netMonitorInstance;
	assert(app);
	return app;
}

int Application::init()
{
	//init process config
	std::string conf_file = getPath(PATH_TYPE_CONFIG) + m_strProcName + ".conf";
	if (loadconfig(conf_file.c_str()) < 0){
		return EXIT_FAILURE;
	}

	conf_file = getPath(PATH_TYPE_CONFIG) + "comm.conf";
	if (!m_commConfig.loadfile(conf_file))
	{
		std::cerr << "load config file error, path: " << conf_file << std::endl;
	}

	//init pidMap
	m_pidMap.reserve(m_procnum);
	for (int i = 0; i < m_procnum; i++)
	{
		m_pidMap.push_back(0);
	}

	init_log(0, false);

	m_inotify.startup(getPath(PATH_TYPE_CONFIG).c_str());

	installAlarmNotify();
	m_status = STATUS_STARTING;

	return 0;
}

void Application::init_log(int id, bool child)
{
	//Log
	if (child) {
		delete m_log;
		m_log = NULL;
		CLogThread::Instance()->stop();
		delete CLogThread::Instance();
		CLogThread::reset();
	}

	char logfile[256];
	sprintf(logfile,"%s_%d_%d_log", m_strProcName.c_str(), m_nserverid, id);
	
	m_log = new SL_Log();
	m_log->init(m_nlogmode, m_nloglevel, getPath(PATH_TYPE_LOG).c_str(), logfile, "txt", 2);
	CLogThread::Instance(m_log, m_nusesyslog);
	
	LOG_PRINT(log_info, "worker is running");
	LOG_PRINT(log_info, "log name[%s] mode[%d:%d]",logfile, m_nlogmode, m_nloglevel);
}

void Application::installProtocol(string protoname)
{
	m_protoname = protoname;
}

void Application::installAlarmNotify()
{
	std::string ip = m_commConfig.getString("alarm", "ip");
	int port = m_commConfig.getInt("alarm", "port");
	if (ip.empty() || 0 == port)
	{
		printf("port == 0\n");
		LOG_PRINT(log_error, "Failed to connect to alarm notify server[%s:%d], please check the config file..", ip.c_str(), port);
	}

	CAlarmNotify::init(ip, port);
}

int Application::loadconfig(const char *confile)
{
	if (!confile || *confile == 0)
		return -1;

	int line;

	m_config.loadfile(confile);
	if(!m_config.load(line)){
		std::cerr << "load configure file " << confile << " failed, error: " << strerror(errno) << endl;
		return -1;
	}

	m_protoname = m_config.getString("self", "protocol");

	m_nserverid = m_config.getInt("self", "svrid");
	m_listen_port = m_config.getInt("self", "port");
	m_listen_rpcport = m_config.getInt("self", "rpcport");
	if(m_listen_port == 0)
	{
		printf("m_listen_port == 0 \n");
	}
	m_procnum = m_config.getInt("self", "workproc");
	m_appPath = m_config.getString("launch_app", "path");

	m_nlogmode = m_config.getInt("log", "logmode");
	m_nloglevel = m_config.getInt("log", "loglevel");
	m_nusesyslog = m_config.getInt("log", "usesyslog");
	m_bdebug	= m_config.getInt("self","debug");
	m_max_conns = m_config.getInt("self","maxconns");
	m_interval_time = m_config.getInt("self","timeinterval");
	m_interval_time = m_interval_time<10?10:m_interval_time;

	m_bAuth = m_config.getInt("self","auth");
	printf("auth:%d\n",m_bAuth);
	return 0;
}

void Application::parseCommand()
{
	//char* procname = argv[0];
	 m_strProcName = procname(m_argv[0]);
	 if (m_argc<2)
	 {
		 printf("Usage: %s  {start|stop|restart|reload|status|kill}\n",m_strProcName.c_str());
		 _exit(0);
	 }
	
	 m_strBinPath = dirname(m_argv[0]).c_str();
	 string command  = m_argv[1];
	 string command2 = m_argc>2 ? m_argv[2] : " ";

	 if (command=="start") {
			 if (command2 == "-d" || m_bdaemon) {
				 //redirectStdout();
				 m_bdaemon = true;
				 printf("%s %s in DAEMON mode\n",m_strProcName.c_str(),command.c_str());
			 } else {
				 printf("%s %s in DEBUG mode\n",m_strProcName.c_str(),command.c_str());
			 }
	  }
	 if(pid_file_path.empty())
	 	 pid_file_path = m_strBinPath + "/"+m_strProcName+".pid";
	 if(statics_file_path.empty())
		 statics_file_path = m_strBinPath + "/"+m_strProcName+".status";
	 // Get master process PID.
	 master_pid = getMasterPid();
	 if(master_pid<0)
	 {
		 _exit(0);
	 }
	 //string master_pid      = file_get_contents(pidFile);
	bool master_is_alive = master_pid && (kill(master_pid, 0)==0);
//	printf("get master_pid!!!!!%d,%s,%d\n",master_pid,command.c_str(),master_is_alive);
	 // Master is still alive?
	 if (master_is_alive) {
		 if (command == "start") {
			 printf(" already running\n");
			 _exit(0);
		 }
	 } else if (command != "start" && command != "restart" && command != "kill") {
		 printf("have not run\n");
		 _exit(0);
	 }
	 //command
	 if (command == "kill") {
		 string cmd1 = "ps aux | grep "+m_strProcName+" | grep -v grep | awk '{print $2}' |xargs kill -SIGINT";
		 string cmd2 = "ps aux | grep "+m_strProcName+" | grep -v grep | awk '{print $2}' |xargs kill -SIGKILL";
		 system(cmd1.c_str());
		 usleep(100000);
		 system(cmd2.c_str());
	 }else if(command == "start"){
		 if (command2 == "-d") {
			 m_bdaemon = true;
		 }
	 }else if(command == "status"){
//	                 if (is_file(self::$_statisticsFile)) {
//	                     @unlink(self::$_statisticsFile);
//	                 }
		 // Master process will send status signal to all child processes.
	         kill(master_pid, SIGUSR2);
	         // Waiting amoment.
	         usleep(1000*500);
	         // Display statisitcs data from a disk file.
	         if (access(statics_file_path.c_str(),F_OK) !=0)
		 {
	        	 printf("statics file not exit!!\n");
	        	 _exit(0);
		 }
	        FILE* fp = fopen(statics_file_path.c_str(),"r+");
		char buf[1024];
		if(fp != NULL)
		{
			while(fgets(buf,1024,fp))
			{
				printf("%s",buf);
			}
			fclose(fp);
		}
	        _exit(0);
	}else if(command == "restart" || command == "stop"){
		 bool alive_before = master_pid && (kill(master_pid, 0)==0);
		 // Send stop signal to master process.
		 master_pid && kill(master_pid, SIGINT);
		 // Timeout.
		 int timeout    = 5;
		 int start_time = time(0);
		 // Check master process is still alive?
		 while (1) {
			 master_is_alive = master_pid && (kill(master_pid, 0)==0);
			 if (master_is_alive) {
				 // Timeout?
				 if (time(0) - start_time >= timeout) {
					 printf("stop fail!!!!!!\n");
					 _exit(0);
				 }
				 // Waiting amoment.
				 usleep(10000);
				 continue;
			 }
			 // Stop success.
			 printf(" stop success\n");
			 if (command == "stop") {
				 _exit(0);
			 }
			 else if (alive_before)
			 {
				 CAlarmNotify::sendAlarmNoty(e_all_notitype, e_logic, m_strProcName.c_str(), "Service Restart", "Yunwei", "Service has been restarted\n");
			 }
			 if (command2 == "-d") {
				 m_bdaemon = true;
			 }
			 break;
		 }
	}else if(command == "reload"){
		 kill(master_pid, SIGUSR1);
		 _exit(0);
	}else{
		 printf("Usage: ./%s {start|stop|restart|reload|status|kill}\n",m_strProcName.c_str());
		 _exit(0);
	}
}

void Application::killAll()
{
	if (m_bStarting)
		CAlarmNotify::sendAlarmNoty(e_all_notitype, e_logic, m_strProcName.c_str(), "Service Start", "Yunwei", "Service start failed\n");
	kill(master_pid, SIGINT);
}

void Application::forkWorkers()
{
	while(0 != count (m_pidMap.begin(),m_pidMap.end(),0))
	{
		pid_t pid =  fork();
		int id = getId(0);
		switch(pid)
		{
		case 0:
		{
			m_id = id;
			m_nserverid += m_id;
			m_pidMap.clear();
			init_log(id + 1, true);
			if(m_workerEvent)
			{
				m_bStarting = true;
				m_workerEvent->onWorkerStart(this);
				m_bStarting = false;
			}
			if(m_listen_rpcport)
			{
				m_rpcsvr.fork_notify();
				m_rpcsvr.start();
			}
			m_svr.fork_notify();
			m_svr.start(id);
			m_io_service_pool.run();
			boost::shared_ptr<boost::thread> thread(new boost::thread(
						boost::bind(&Application::waitSignal, this)));
			while(m_status != STATUS_SHUTDOWN)
			{
				static unsigned int last_activetime_;
				time_t now = time(0);

				if (now - last_activetime_ >= 20){
					last_activetime_ = now;
					if (m_lstTask_data.size() > 0)
					{
						LOG_PRINT(log_debug, "Application queue-size:%u .", m_lstTask_data.size());
					}
				}

				task_proc_data* pmsg = 0;
				{
					boost::mutex::scoped_lock lock(m_mutex);
					while(m_lstTask_data.empty())
					{
						m_condition.wait(m_mutex);
					}
					if(!m_lstTask_data.empty())
					{
						pmsg = m_lstTask_data.front();
						m_lstTask_data.pop_front();
					}
				}

				if(pmsg != 0)
				{
					if(pmsg->msgtye == TASK_MSGTYPE_TIMER)
					{
						task_proc_param* param = (task_proc_param*)pmsg->pdata;
						if(param->message == MSG_TYPE_TIMER)
						{
							Timer::getInstance()->trigger(param->param1);
						}
					}
					if(pmsg->msgtye == TASK_MSGTYPE_LOCAL)
					{
						task_proc_param* param = (task_proc_param*)pmsg->pdata;
						if(param->message == MSG_TYPE_CLIENT_CONNECT || param->message == MSG_TYPE_SERVER_CONNECT)
						{
							if(pmsg->connection && pmsg->connection->m_workerEvent)
							{
								pmsg->connection->m_workerEvent->onConnect(pmsg->connection);
							}
						}else if(param->message == MSG_TYPE_CLOSE)
						{
							if(pmsg->connection && pmsg->connection->m_workerEvent)
							{
								pmsg->connection->m_workerEvent->onClose(pmsg->connection);
							}
						}else if(param->message == MSG_TYPE_ERROR)
						{
							if(pmsg->connection && pmsg->connection->m_workerEvent)
							{
								pmsg->connection->m_workerEvent->onError(pmsg->connection, param->param1, param->msg);
							}
						}else
						{
							if(pmsg->connection && pmsg->connection->m_workerEvent)
							{
								pmsg->connection->m_workerEvent->onMessage(pmsg);
							}
						}
					}
					else if(pmsg->connection && pmsg->connection->m_workerEvent)
					{
						try
						{
							pmsg->connection->m_workerEvent->onMessage(pmsg);
						}
						catch(exception &e)
						{
							LOG_PRINT(log_error, "catch execption: %s", e.what());
						}
						catch(...)
						{
							LOG_PRINT(log_error, "catch execption: unknown");
						}
					}

					if(pmsg->pdata != 0)
					{
						delete[] pmsg->pdata;
						pmsg->pdata = NULL;
					}

					delete pmsg;
					pmsg = NULL;
				}

			}
			_exit(0);
			//return 0;
		}
		case -1:
			printf("fork failed!\n");
			break;
		default:
			m_pidMap[id] = pid;
			break;
		}
	}

}

void signalHandler(int signal)
{
	Application * app = Application::getInstance();
	switch (signal) {
		// Stop.
		case SIGINT:
			app->stopAll();
			break;
		// Reload.
		case SIGUSR1:
			app->initRestartPid();
			app->reload();
			break;
		// Show status.
		case SIGUSR2:
//			app->writeStatisticsToStatusFile();
			app->m_stdstatusinfo = true;
			break;
	}
}

void Application::writeStatisticsToStatusFile()
{
	//for master process


	char buf[1024];
	if(master_pid == getpid())
	{
		FILE* fp = fopen(statics_file_path.c_str(),"w+");
		if(fp == NULL)
		{
			printf("write to file error!\n");
			return;
		}
		sprintf(buf, "------------------------------PROCESS STATUS-----------------------------\n");
		fputs(buf, fp);
		sprintf(buf,"pid\tmemory(K)\tlistening\tconnections\tsend_fail\tName\n");
		fputs(buf, fp);
		sprintf(buf,"%d\t%d\t\t%d\t\t0\t\t0\t\tMaster:%s\n",getpid(),getmemsize(),m_listen_port,m_strProcName.c_str());
		fputs(buf, fp);
		fclose(fp);
		for (size_t  i = 0; i < m_pidMap.size(); i++)
		{
			if(m_pidMap[i]!=0)
			{
				kill(m_pidMap[i], SIGUSR2);
			}
		}
		return;
	}
	//for child process
	FILE* fp = fopen(statics_file_path.c_str(),"at");
	if(fp == NULL)
	{
		printf("write to file error!\n");
		return;
	}
	int sport = m_config.getInt("self", "startport");
	if(sport == 0)
		sprintf(buf,"%d\t%d\t\t%u\t\t%d\t\t%d\t\tSlave:%s\n",getpid(),getmemsize(),0,tcpserver::getconns_size(),0,m_strProcName.c_str());
	else
		sprintf(buf,"%d\t%d\t\t%u\t\t%d\t\t%d\t\tSlave:%s\n",getpid(),getmemsize(),sport + m_id,tcpserver::getconns_size(),0,m_strProcName.c_str());
	fputs(buf, fp);
	fclose(fp);
}

void Application::installSignal()
{
	printf("installSignal\n");
	// stop
	signal(SIGINT,signalHandler);
	// reload
	signal(SIGUSR1, signalHandler);
	// status
	signal(SIGUSR2, signalHandler);
	// ignore
	signal(SIGPIPE, SIG_IGN);
	signal (SIGSEGV, SIG_IGN );
	signal (SIGALRM, SIG_IGN );
}

void Application::reinstallSignal()
{

}

void Application::monitorWorkers()
{
	boost::shared_ptr<boost::thread> thread(new boost::thread(boost::bind(&Application::waitSignal, this)));
	m_status =STATUS_RUNNING;
	while(1)
	{
		int status;
		pid_t pid = wait(&status);
		// If a child has already exited.
		if (pid > 0) {
			// Find out witch worker process exited.
			int idx = getId(pid);
			m_pidMap[idx] = 0;
			//alarm
			// Is still running state then fork a new worker process
			if (m_status != STATUS_SHUTDOWN) {
				LOG_PRINT(log_info, "worker is restart, index: %d", idx);
				CAlarmNotify::sendAlarmNoty(e_all_notitype, e_logic, m_strProcName.c_str(), "Core alarm", "Yunwei", "program  Cored\n");

				forkWorkers();
				// If reloading continue.
				vector<int>::iterator it = find(m_pidToRestart.begin(),m_pidToRestart.end(),pid);
				if(it!=m_pidToRestart.end())
				{
					m_pidToRestart.erase(it);
					reload();
				}

			} else {
				// If shutdown state and all child processes exited then master process exit.
				if(count (m_pidMap.begin(),m_pidMap.end(),0)==m_pidMap.size()){
					CAlarmNotify::sendAlarmNoty(e_all_notitype, e_logic, m_strProcName.c_str(), "Service Stop", "Yunwei", "Service has been stopped\n");
					unlink(pid_file_path.c_str());
					unlink(statics_file_path.c_str());
					m_svr.delshm();
					_exit(0);
				}
			}
		} else {
			// If shutdown state and all child processes exited then master process exit.
			if (m_status == STATUS_SHUTDOWN && count (m_pidMap.begin(),m_pidMap.end(),0)==m_pidMap.size()) {
				unlink(pid_file_path.c_str());
				unlink(statics_file_path.c_str());
				printf("STATUS_SHUTDOWN\n");
				m_svr.delshm();
				_exit(0);
			}
		}
	}
}

void Application::stopAll()
{
//	kill(master_pid, SIGINT);
	m_status = STATUS_SHUTDOWN;
	// For master process.
	if (master_pid == getpid())
	{
		printf("Stopping ...%d\n",master_pid);
		// Send stop signal to all child processes.
		for (int i = 0; i < m_pidMap.size(); i++)
		{
			if(m_pidMap[i] != 0)
			{
				kill(m_pidMap[i], SIGINT);
				//boost::asio::deadline_timer t(Application::get_io_service(), boost::posix_time::seconds(10));
				//m_timer->async_wait(boost::bind(&Application::killProcess,this,m_pidMap[i]));
			}
		}
	}
	else
	{
		// For child processes.
		// Execute exit.
		this->stop();
		_exit(0);
	}
}

void Application::waitSignal()
{
	while(1)
	{
		if(m_stdstatusinfo)
		{
			writeStatisticsToStatusFile();
			m_stdstatusinfo = false;
		}
		usleep(100*1000);
	}
}

void Application::initRestartPid()
{
	m_pidToRestart.assign(m_pidMap.begin(),m_pidMap.end());
}

void Application::reload()
{

	// For master process.
	if (master_pid == getpid()) {
		// Set reloading state.
		if (m_status != STATUS_RELOADING && m_status != STATUS_SHUTDOWN) {
			m_status =STATUS_RELOADING;
		}
		// Send reload signal to all child processes.
		vector<int> reloadable_pid;
		if(!m_reloadable)
		{
			for(int i=0;i<m_pidMap.size();i++)
			{
				kill(m_pidMap[i],SIGUSR1);
			}
		}
		else
		{
			reloadable_pid.assign(m_pidMap.begin(),m_pidMap.end());
		}
		// Reload complete.
		vector<int> result;
		result.resize(m_pidToRestart.size()+reloadable_pid.size());
		vector<int>::iterator retEndPos;
		sort(m_pidToRestart.begin(),m_pidToRestart.end());
		sort(reloadable_pid.begin(),reloadable_pid.end());
		retEndPos = set_intersection( m_pidToRestart.begin(), m_pidToRestart.end(), reloadable_pid.begin(), reloadable_pid.end() ,result.begin());
		result.resize( retEndPos - result.begin() ) ;
		m_pidToRestart.clear();
		m_pidToRestart.assign(result.begin(),result.end());

		if(m_pidToRestart.empty())
		{
			if(m_status != STATUS_SHUTDOWN)
			{
				m_status = STATUS_RUNNING;
			}
			return;
		}
		// Continue reload.
		int pid = m_pidToRestart[0];
		// Send reload signal to a worker process.
		kill(pid,SIGUSR1);
	} // For child processes.
	else
	{
	        // Try to emit onWorkerReload callback.
		if(m_workerEvent)
			m_workerEvent->onWorkerReload(this);
		if(m_reloadable)
		{
			stopAll();
		}
	}
}

uint16 Application::getSvrId()
{
	return m_nserverid;
}

int Application::getId(int pid)
{
	for(int i=0;i<m_pidMap.size();i++)
	{
		if(m_pidMap[i]==pid)
		{
			return i;
		}
	}
	return -1;
}

void Application::stop()
{
	//uninit
	if(m_workerEvent)
		m_workerEvent->onWorkerStop(this);
	m_workerEvent = NULL;
}
void Application::redirectStdoutbegin()
{
	printf("11\n");
	if(freopen("../log/std.out","w",stdout) == NULL)
	{
		cerr<<"redirect error"<<endl;
	}
}
void Application::redirectStdoutend()
{
	fclose(stdout);
}

const std::string &Application::getProcName()
{
	return m_strProcName;
}

std::string Application::getPath(PATH_TYPE type)
{
	switch (type)
	{
	case PATH_TYPE_BIN:
		return m_strBinPath;
	case PATH_TYPE_CONFIG:
		return m_strBinPath + "/../etc/";
	case PATH_TYPE_LOG:
		return m_strBinPath + "/../log/";
	case PATH_TYPE_COMM_CONFIG_FILE:
		return m_strBinPath + "/../etc/comm.conf";
	default:
		return std::string("");
	}
}

uint16_t Application::getPort()
{
	return getInstance()->m_listen_port;
}

uint16_t Application::getRpcPort()
{
	return getInstance()->m_listen_rpcport;
}

void Application::add_message(task_proc_data* data)
{
	boost::mutex::scoped_lock lock(m_mutex);
	m_lstTask_data.push_back(data);
	m_condition.notify_one();
}
boost::asio::io_service& Application::get_io_service(int index/* = -1*/)
{
	return m_io_service_pool.get_io_service(index);
}
unsigned int Application::getmemsize()
{
	char cmd[128]  = {0};
	sprintf(cmd,"/proc/%d/statm",getpid());
	FILE* f = fopen(cmd,"r");
	unsigned int memsize = 0;
	if(NULL == f)
	{
		printf("getmemsize err\n");
		return 0;
	}
	fscanf(f,"%*u%*u%u%*u*u%*u%*u",&memsize);
	fclose(f);
	return memsize;
}
void Application::setRpcProcessor(TProcessor* processor)
{
	m_rpcsvr.setProcessor(processor);
}
