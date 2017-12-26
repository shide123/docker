
#ifndef __ROOM_SERVER_APPLICATION_HH__
#define __ROOM_SERVER_APPLICATION_HH__

#include "SL_Socket_CommonAPI.h"
#include "SL_Socket_TcpClient_AutoConnect.h"
#include "SL_ObjectPool.h"
#include "SL_Singleton.h"
#include "SL_Log.h"

#include "TaskNetMsgProc.h"
#include "SL_Timer_List_Time.h"
#include "redisOpt.h"
#include "redisMgr.h"
#include "fileinotify.h"
#include "CAlarmNotify.h"
#include "singleton.h"
#include "json/json.h"

namespace __gnu_cxx
{
    template<> struct hash<const string>
    {
        size_t operator()(const string& s) const
        { return hash<const char*>()( s.c_str() ); } //__stl_hash_string
    };
    template<> struct hash<string>
    {
        size_t operator()(const string& s) const
        { return hash<const char*>()( s.c_str() ); }
    };
}

#define MSG_LEN 2048
#define ERR_LOOP_CONTINUE 1

typedef struct {
	string ip;
	uint16_t port;
	string passwd;
}redisHost;

class server;
class connmgr;
class SyncServerApplication
{
public:
	SyncServerApplication(void);
	virtual ~SyncServerApplication(void);

	int init();
	int initRedis();
	int loadconfig(const char *confile);
	int load_permission_config(const char *confile);
	int connect_sync_server(boost::asio::io_service &ioservice, server *pserver);
	int clearRedis();
	uint64_t new_synclog_seq() {
		boost::mutex::scoped_lock lock(m_synclog_index_mutex);
		return ++m_synclog_index;
	}
	uint64_t curr_synclog_seq() {
		boost::mutex::scoped_lock lock(m_synclog_index_mutex);
		return m_synclog_index;
	}
	clienthandler* getgateclient(uint16_t gateid);
	void syncsvr_notify(const char* data, int len);
	int done_redis_msg(const char *buffer, int buflen, Json::Value &root, bool bwrite);
	static void* redis_msg_thread_proc(void* arg);
	void set_sync_master_ip(const char* ip);
	const string &get_sync_master_ip();
	connecthandler_ptr get_conn_by_ip(const char* ip);
	redisHost get_redishosts(const string &addrinfo);
	void check_deadline(const boost::system::error_code& e);
	void launchapp(const char* appname);
	uint32 new_index() {
		boost::mutex::scoped_lock lock(m_msgindex_mutex);
		if (m_msgindex + 1 > UINT_MAX)
			m_msgindex = 0;
		return ++m_msgindex;
	}

public:
	server 			*m_pSyncsvr;
	connmgr 		*m_pConnMgr;
	redisMgr		*m_pRedisMgr;
	redisOpt 		*m_pRedisMsg;
	redisOpt 		*m_pRedisPub;
	vector<redisOpt_ptr> m_vecRemoteRedisData;
	vector<redisOpt_ptr> m_vecRemoteRedisPub;
	SL_Log 			m_log;
	
	typedef map<string, vector<string> > HVALSNAME_MAP;
    HVALSNAME_MAP m_hvalsName_map;
    map<int, string> m_Sub_Hvals;
    map<int, int> m_SubReq_SubResp;
    
	CTaskNetMsgProc  m_online_task;
	fileinotify m_inotify;
	SL_Thread<SL_Sync_ThreadMutex> rdsMsgThread;
	REDIS_SYNC_STATUS	m_sync_status;
	string   m_sync_master_ip;
	boost::mutex m_sync_master_ip_lock;

	std::string m_sAlarmhost;
	ushort 		m_nAlarmport;
	// redis hosts
	string 		m_strRedisHost;
	uint16_t	m_nRedisPort;
	string		m_sRedisPass;

	std::string m_appPath;
	std::string m_sSyncsvrAddr;
	int         m_msg_threadnum;
	uint16      m_nserverid;
	int 		m_nlogmode;
	int 		m_nloglevel;
	int			m_nusesyslog;
	bool 		m_bdaemon;
	bool		m_bsyncing;
	boost::mutex m_synclog_index_mutex;
	uint64_t    m_synclog_index;
	uint64_t	m_last_sync_index;
	std::string m_strbinpath;
	std::string m_strsynclogpath;
	uint16_t     m_listen_port;
	uint32		m_msgindex;
	boost::mutex m_msgindex_mutex;

	int          m_bdbinited;
	uint32       m_nDBauthedTime;
	FILE		*m_synclogfile;
	SL_Sync_Mutex m_synclogfile_Mutex;
	SL_Sync_Mutex m_sync_redis_Mutex;
	deadline_timer *deadline_;

	unsigned int m_last_printstackinfotime; //最后打印队列信息时间
};

#define appInstance SingleInstance<SyncServerApplication>::Instance()

#endif //__LOGON_SERVER_APPLICATION_HH__

