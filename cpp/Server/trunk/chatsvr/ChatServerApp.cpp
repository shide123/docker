#include "ChatServerApp.h"
#include <stdio.h>
#include "CLogThread.h"
#include "ipdatabase.h"
#include "DBTools.h"
#include "DBmongoConnection.h"
#include <json/json.h>
#include "TChatSvrHandler.h"
#include "CUserBasicInfo.h"
#include "GroupBasicInfo.h"
#include "CUserGroupinfo.h"
#include "GroupAssistLogic.h"
#include "CThriftSvrMgr.h"
#include "timer.h"
CChatServerApp::CChatServerApp()
{
	m_uAlarmQueueSize = 1000;
	m_uAlarmNotifyInterval = 300;
	m_uLanPort = 0;
	m_pInnerTcpWorker = NULL;
	m_pNodeClient = NULL;
	m_pRpcNodeClient = NULL;

	m_room_mgr=NULL;
}

CChatServerApp::~CChatServerApp()
{
	IFNOTNULL_DELETE(m_pNodeClient)
	IFNOTNULL_DELETE(m_pRpcNodeClient)
	IFNOTNULL_DELETE(m_room_mgr)
	releaseDbRelated();
}

bool CChatServerApp::onWorkerStart(Application* worker)
{
	m_uLanPort = m_config.getInt("self", "startport") + worker->m_id;
	string protocol = m_config.getString("self", "startprotocol");
	m_pInnerTcpWorker = new Application(protocol, m_uLanPort, &m_GateEvent);

	LOG_PRINT(log_info, "onWorkerStart worker id:%d.", worker->m_id);
	
	loadSelfConfig();
	initDbRelated();

	if (!initRedis())
	{
		LOG_PRINT(log_error, "initialize redis error..");
		return false;
	}

	string ipdatabase = getPath(PATH_TYPE_CONFIG) + "ipdata.dat";
	CCzip::GetInstance(ipdatabase.c_str());

    boost::shared_ptr<TChatSvrHandler> handler(new TChatSvrHandler());
    TProcessor * processer = new TChatSvrProcessor(handler);
    worker->setRpcProcessor(processer);

    //CKeywordMgr::loadKeyword();
    CChatLogic::init(m_pRedisMgr);
	init_node();
	return true;
}

bool CChatServerApp::onWorkerStop(Application* worker)
{
	CChatLogic::uninit();
	releaseDbRelated();
	IFNOTNULL_DELETE(m_pInnerTcpWorker)
	LOG_PRINT(log_info, "onWorkerStop worker id:%d.", worker->m_id);
	return true;
}

CChatServerApp *CChatServerApp::getInstance()
{
	return (CChatServerApp *)Application::getInstance();
}

uint16 CChatServerApp::getServiceId()
{
	return getInstance()->getSvrId();
}

void CChatServerApp::initDbRelated()
{
	std::string strCommFile = getPath(PATH_TYPE_CONFIG) + "comm.conf";
	Dbconn::m_config.loadfile(strCommFile);
	CDBTools::initDB(Dbconn::getInstance());
	if (NULL == Dbconn::getInstance() || !CMongoDBConnectionPool::init(m_commConfig)/* || !Dbconn::initDBSinkPool(CDBSink::m_nPlatId)*/)
	{
		LOG_PRINT(log_error, "initialized db instance error, please check the config file: %s.", strCommFile.c_str());
		killAll();
	}

}

void CChatServerApp::releaseDbRelated()
{
}

void CChatServerApp::loadSelfConfig()
{
	unsigned int nalarmqueuesize = m_commConfig.getInt("noticesvr", "alarmqueuesize", m_uAlarmQueueSize);
	CGroupAssistLogic::initMsgFormat(m_config);
}

bool CChatServerApp::initRedis()
{
	// redis配置信息
	std::string strRedisHost = m_commConfig.getString("redis", "ip");
	uint16 uRedisPort = m_commConfig.getInt("redis", "port");
	std::string strRedisPass = m_commConfig.getString("redis", "password");
	int nRedisThreadNum = m_commConfig.getInt("redis", "threadnum", 2);
	
	if (strRedisHost.empty() || 0 == uRedisPort || strRedisPass.empty())
	{
		LOG_PRINT(log_error, "redis configure is error [%s:%d].", strRedisHost.c_str(), uRedisPort);
		return false;
	}

	m_pRedisMgr = new redisMgr(strRedisHost.c_str(), uRedisPort, strRedisPass.c_str(), nRedisThreadNum);
	if (!m_pRedisMgr){
		LOG_PRINT(log_error, "Failed to create redis manager object");
		return false;
	}

	m_room_mgr = new CRoomManager();
	m_room_mgr->setRedisConf(m_pRedisMgr);

	CUserBasicInfo::init(m_pRedisMgr);
	CGroupBasicInfo::init(m_pRedisMgr);
	CUserGroupinfo::init(m_pRedisMgr);
	CGlobalInfo::init(m_pRedisMgr);
	CKeywordMgr::init(m_pRedisMgr);
	m_rdsOnlineUser.init(DB_USER_MGR, "user:", m_pRedisMgr);
	return true;
}

void CChatServerApp::init_node()
{
	CThriftSvrMgr::init(&m_commConfig);
	//只关注本机房,匹配ip段正则表达式
	string strIpExpression = m_config.getString("server", "ip_expression");
	LOG_PRINT(log_info, " ip_expression : %s", strIpExpression.c_str());
	m_expression = boost::regex(strIpExpression);

	LOG_PRINT(log_info, "Create Node Client: svr[%s], port[%u]", getProcName().c_str(), m_uLanPort);
	m_pNodeClient = INodeClient::CreateObject(getProcName().c_str(), Application::get_io_service(), NULL, "", m_uLanPort);

	std::vector<std::string> addrs;
	std::string straddrs = m_commConfig.getString("node", "svr");
	LOG_PRINT(log_info, "node addr:%s", straddrs.c_str());

	if (straddrs.empty() || !m_pNodeClient) 
	{
		LOG_PRINT(log_error, "node server config is empty.please check the config.");
		killAll();
		return;
	}

	m_pNodeClient->set_address(straddrs.c_str());
	m_pNodeClient->connect();

	//init rpc node client object
	m_pRpcNodeClient = INodeClient::CreateObject((getProcName() + "_rpc").c_str(), Application::get_io_service(), this, "", getRpcPort());
	if (!m_pRpcNodeClient)
	{
		LOG_PRINT(log_error, "create rpc node client error.please check the config.");
		killAll();
		return;
	}
	std::string subscribe(m_config.getString("server", "svr_name_rpc"));
	splitStrToSet(subscribe, ",", m_sSubRpcServer);
	m_pRpcNodeClient->subscribe(subscribe.c_str());

	m_pRpcNodeClient->set_address(straddrs.c_str());
	m_pRpcNodeClient->connect();
}


void CChatServerApp::onNodeEvent(const std::string & svrname, const std::string & ip, int port, en_node_event event, std::string desc /*= ""*/)
{
	/*
	* @svrname: 	service name, such as "logonsvr", "roomsvr".
	* @ip:			service ip.
	* @port:		service port.
	* @event:		add or remove. connect if add, and disconnect where remove.
	*/
	LOG_PRINT(log_info, "recv node event: svrname[%s] ip[%s] port[%d] event[%d].", svrname.c_str(), ip.c_str(), port, event);
	if (svrname.empty() || ip.empty() || 0 == port)
		return;

	e_SvrType svrtype = (e_SvrType)CThriftSvrMgr::change_svrname_to_type(svrname);
	if (event == en_svr_add)
	{
		if (boost::regex_match(ip, m_expression))
		{
			if (m_sSubRpcServer.find(svrname) != m_sSubRpcServer.end())
			{
				LOG_PRINT(log_info, "found new native rpc server, svr_type:%d ip:%s port:%d, connect", svrtype, ip.c_str(), port);
				CThriftSvrMgr::addThriftClient(ip, port, svrtype, true);
			}
		}
		else
		{
			if (m_sSubRpcServer.find(svrname) != m_sSubRpcServer.end())
			{
				LOG_PRINT(log_info, "found new remote rpc server, svr_type:%d ip:%s port:%d, connect", svrtype, ip.c_str(), port);
				CThriftSvrMgr::addThriftClient(ip, port, svrtype, false);
			}
		}
	}
	else if (event == en_svr_rem)
	{
		LOG_PRINT(log_info, "svr_type:%d ip:%s port:%d had closed", svrtype, ip.c_str(), port);
		if (m_sSubRpcServer.find(svrname) != m_sSubRpcServer.end())
		{
			CThriftSvrMgr::delThriftClient(ip, port, svrtype);
		}
	}
}
