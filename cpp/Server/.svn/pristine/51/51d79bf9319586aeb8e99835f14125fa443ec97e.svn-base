#include "LogonServerApplication.h"
#include "GlobalSetting.h"
#include <iostream>
#include <assert.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "com/slist.h"
#include "com/hash.h"
#include "LogonSvrApp.h"
#include "DBTools.h"


LogonServerApplication::LogonServerApplication()
{
	memset(m_ipaddrs, 0, sizeof(m_ipaddrs));
	m_udpsocket_ = -1;
	m_area = 0;
	m_limitversion = 0;
}

LogonServerApplication::~LogonServerApplication()
{
	clear();
}

int LogonServerApplication::init()
{
    //init redis
    m_sRedisHost = CLogonSvrApp::getInstance()->m_commConfig.getString("redis", "ip");
    m_nRedisPort = CLogonSvrApp::getInstance()->m_commConfig.getInt("redis", "port");
    m_redisThreadNum = CLogonSvrApp::getInstance()->m_commConfig.getInt("redis", "threadnum");
    m_sRedisPass = CLogonSvrApp::getInstance()->m_commConfig.getString("redis", "password");
    LOG_PRINT(log_info, "redis ip:%s port:%d thread_num:%d pass:%s.", m_sRedisHost.c_str(), m_nRedisPort, m_redisThreadNum, m_sRedisPass.c_str());
    m_pRedisMgr = new redisMgr(m_sRedisHost.c_str(), m_nRedisPort, m_sRedisPass.c_str(), m_redisThreadNum);
    m_redis_vistor.init(DB_VISITORID, KEY_VISTOR_INFO, m_pRedisMgr);

	m_token_time    = CLogonSvrApp::getInstance()->m_config.getInt("self", "tokentime");
	m_limitversion  = CLogonSvrApp::getInstance()->m_config.getInt("self", "limitversion");
	m_area          = CLogonSvrApp::getInstance()->m_config.getInt("visitoid", "area");
	m_hostArea      = CLogonSvrApp::getInstance()->m_commConfig.getString("self", "area");
	LOG_PRINT(log_info, "token_time:%d,limit_version:%u,area:%u,hostArea:%s", m_token_time, m_limitversion, m_area, m_hostArea.c_str());

	srand((uint)time(0));
   
	return 0;
}

void LogonServerApplication::clear()
{
	if (m_pRedisMgr)
	{
		delete m_pRedisMgr;
		m_pRedisMgr = NULL;
	}
}
