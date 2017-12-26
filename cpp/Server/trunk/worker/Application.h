

#ifndef __APPLICATION_HH__
#define __APPLICATION_HH__

#include <vector>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>

#include "tcpserver.h"
#include "rpcserver.h"
#include "Config.h"
#include "fileinotify.h"
#include "CAlarmNotify.h"
#include "ProtocolsBase.h"
#include "io_service_pool.h"
#include "message_comm.h"
#include "timer.h"

#include <thrift/TProcessor.h>
using namespace apache::thrift;

enum {
	STATUS_STARTING = 1 ,
	STATUS_RUNNING = 2,
	STATUS_SHUTDOWN = 4,
	STATUS_RELOADING = 8,
};

typedef enum {
	PATH_TYPE_BIN,
	PATH_TYPE_CONFIG,
	PATH_TYPE_LOG,
	PATH_TYPE_COMM_CONFIG_FILE,
}PATH_TYPE;
#define MAX_IO_SERVICE_POOL_NUM 	(5)
class tcpserver;
class Application
{
public:
	Application(void);
	Application(string protoname, int port, IWorkerEvent * event);
	virtual ~Application(void);

	int init();
	int loadconfig(const char * confile);
	int runAll(int argc, char * argv[]);

	void killAll();
	static Application * getInstance();
	void setConnectEvent(IWorkerEvent * event){m_workerEvent = event;};
	void redirectStdoutbegin();
	void redirectStdoutend();
	const std::string &getProcName();
	std::string getPath(PATH_TYPE type);
	static uint16_t getPort();
	static uint16_t getRpcPort();
	static boost::asio::io_service& get_io_service(int index = -1);
	void reload();
	uint16 getSvrId();

	//Do not use the follow functions
	void stopAll();

	void initRestartPid();
	void setRpcProcessor(TProcessor* processor);
public:
	int					m_id;
	SL_Log *			m_log;
	Config 				m_config;
	Config				m_commConfig;
	fileinotify 		m_inotify;
	bool 				m_reloadable;
	string				m_strProcName;
	bool				m_stdstatusinfo;
	bool				m_bStarting;
protected:
    u_int64_t newconnid() {return ++connid;}
    unsigned int getmemsize();
private:
	void waitSignal();
	void parseCommand();
	void forkWorkers();
	void monitorWorkers();
	void installSignal();
	void reinstallSignal();
	int getId(int pid);
	void stop();
	int getMasterPid();
	void saveMasterPid();
	void installProtocol(string protoname);
	void init_log(int id, bool child);
	void writeStatisticsToStatusFile();

	void installAlarmNotify();

	bool 				m_bdaemon;
	int					m_status;
	int					master_pid;
	int         				m_procnum;
	vector<int>		 	m_pidMap;
	vector<int>			m_pidToRestart;
	string 				m_appPath;
	string 				m_strBinPath;
	static Application * 	app;
	uint16      		m_nserverid;
	uint16_t     		m_listen_port;
	uint16_t     		m_listen_rpcport;
	int 				m_nlogmode;
	int 				m_nloglevel;
	int					m_nusesyslog;
	int 				m_argc;
	char** 				m_argv;
	string				m_protoname;
	IWorkerEvent* 		m_workerEvent;
	unsigned int  		connid;
	boost::mutex 		m_mutex;
	boost::condition_variable_any m_condition;
	string				pid_file_path;
	string				statics_file_path;
	static io_service_pool		m_io_service_pool;
	bool 				m_bdebug;
	boost::asio::deadline_timer *m_timer;
	int m_max_conns;
public:
	tcpserver			m_svr;
	rpcserver			m_rpcsvr;
	void add_message(task_proc_data* data);
	std::deque<task_proc_data*> m_lstTask_data;
	static int 			m_interval_time;
	bool				m_bAuth;
};

#endif //__APPLICATION_HH__

