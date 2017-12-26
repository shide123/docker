
#ifndef __LOGON_SERVER_APPLICATION_HH__
#define __LOGON_SERVER_APPLICATION_HH__

#include <map>
#include <list>
#include <vector>

#include "SL_Socket_CommonAPI.h"
#include "UserManager.h"
#include "CLogonMgr.h"
#include "Config.h"
#include "bridgeMgr.h"
#include "redisMgr.h"
#include "redisOpt.h"
#include "redis_map_map.h"
#include "redis_set.h"
#include "utils.h"

class LogonServerApplication
{
public:
	LogonServerApplication();
	virtual ~LogonServerApplication();

	int init();
	void clear();

	CUserManager           m_user_mgr;
    CLogonMgr              m_logon_mgr;
	
	int         m_token_time;

    redisMgr *  m_pRedisMgr;
	std::string m_sRedisHost;
	uint16		m_nRedisPort;
	std::string	m_sRedisPass;
	int m_redisThreadNum;
	
	redis_map_map m_redis_roominfo;
	redis_set    m_redis_roomid_set;
	redis_map_map m_redis_vistor;
	unsigned int m_ipaddrs[8];

	int          m_udpsocket_;
	unsigned int m_area;
	unsigned int m_limitversion;

	std::string m_hostArea;
};


#endif //__LOGON_SERVER_APPLICATION_HH__

