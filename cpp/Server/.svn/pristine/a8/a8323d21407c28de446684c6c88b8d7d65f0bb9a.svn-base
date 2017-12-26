/*
 * serverEvent.cpp
 *
 *  Created on: Aug 22, 2016
 *      Author: testtest
 */

#include "serverEvent.h"
#include "AppDelegate.h"
#include "DBTools.h"
#include "msgcommapi.h"
#include "CUserGroupMgr.h"
#include "GroupBasicInfo.h"
#include "CUserBasicInfo.h"

ServerEvent *g_pEvent = NULL;

ServerEvent::ServerEvent(int workid, uint16_t port)
	: m_pApp(NULL)
	, m_nRedisPort(0)
	, m_nexcept_exit_interval(10)
	, m_pRedisMgr(NULL)
//	, m_pRedisMsg(NULL)
//	, m_pRedisSub(NULL)
	, m_workid(workid)
	, m_port(port)
	, m_svrtimerid(-1)
	, m_logictimerid(-1)
	, m_coursemgrtimerid(-1)
{
	m_logicCore.setEvent(this);
	m_maxuserwaitmic = 1;
	m_maxwaitmicuser = 5;
}

ServerEvent::~ServerEvent()
{
	if (m_svrtimerid != -1)
	{
		Timer::getInstance()->del(m_svrtimerid);
		m_svrtimerid = -1;
	}

	if (m_logictimerid != -1)
	{
		Timer::getInstance()->del(m_logictimerid);
		m_logictimerid = -1;
	}

	if (m_coursemgrtimerid != -1)
	{
		Timer::getInstance()->del(m_coursemgrtimerid);
		m_coursemgrtimerid = -1;
	}
}

int ServerEvent::Init(AppDelegate *pApp)
{
	setApp(pApp);
	CDBTools::initDB(Dbconn::getInstance("maindb"));

    m_strRedisHost 	= m_pApp->m_commConfig.getString("redis", "ip");
    m_nRedisPort	= m_pApp->m_commConfig.getInt("redis", "port");
	m_sRedisPass 	= m_pApp->m_commConfig.getString("redis", "password");
	m_nexcept_exit_interval = m_pApp->m_config.getInt("self", "except_exit_interval");
	LOG_PRINT(log_info, "[%s]SvrId:%u.", m_pApp->getProcName().c_str(), m_pApp->getSvrId());
	m_logicCore.setSvrId(m_pApp->getSvrId());

	int ret = -1;

	do 
	{
		ret = InitRedis();
		CUserObj::init(m_pRedisMgr);
		CUserGroupMgr::init(m_pRedisMgr);
		CUserBasicInfo::init(m_pRedisMgr);
		CUserGroupMgr::setRoomManager(&m_roomMgr);
		CGroupBasicInfo::init(m_pRedisMgr);
		m_roomMgr.setConfSetting(m_pApp->m_config, m_pApp->getPath(PATH_TYPE_CONFIG));
		m_roomMgr.setSvrId(m_pApp->getSvrId());
		m_roomMgr.setWorkID(m_pApp->m_id);
		m_roomMgr.setRedisConf(m_pRedisMgr);
		m_roomMgr.init();
		m_roomMgr.m_clientconn.setEvent(this);
		m_roomMgr.m_clientconn.connect("127.0.0.1", m_port);
		m_roomMgr.finishInitData();
		
	}while(0);

	if (!CMongoDBConnectionPool::init(m_pApp->m_commConfig)/* || !Dbconn::initDBSinkPool(CDBSink::m_nPlatId)*/)
	{
		LOG_PRINT(log_error, "initialized db instance error, please check the config file");
		Application::getInstance()->killAll();
	}
	if (0 == ret && m_pApp->m_id==0)
	{
		m_svrtimerid = Timer::getInstance()->add(30, boost::bind(&ServerEvent::onTimer, this), TIMES_UNLIMIT);
		m_logictimerid = Timer::getInstance()->add(1, boost::bind(&CLogicCore::onTimer, &m_logicCore), TIMES_UNLIMIT);
	}
	return ret;
}

int ServerEvent::InitRedis()
{
	m_pRedisMgr = new redisMgr(m_strRedisHost.c_str(), m_nRedisPort, m_sRedisPass.c_str(), 2);
	if (!m_pRedisMgr){
		LOG_PRINT(log_error, "Failed to create redis manager object");
		return -1;
	}
	m_pRedisMgr->redis_ConnExceptCallback(ServerEvent::redis_conn_exception);

// codis 不支持发布订阅功能
//	m_pRedisMsg = new redisOpt(m_strRedisHost.c_str(), m_nRedisPort, false, m_sRedisPass.c_str());
//	if (m_pRedisMsg->runloop() < 0){
//		LOG_PRINT(log_error, "Failed to connect to redis server: %s:%u!", m_strRedisHost.c_str(), m_nRedisPort);
//		return -1;
//	}
//	m_pRedisMsg->redis_ConnExceptCallback(ServerEvent::redis_conn_exception);
//
//	m_pRedisSub = new redisOpt(m_strRedisHost.c_str(), m_nRedisPort, true, m_sRedisPass.c_str());
//	if (m_pRedisSub){
//		m_pRedisSub->redis_SetHandleMsgProc(ServerEvent::redis_handle_message);
//		if (m_pRedisSub->runloop() < 0){
//			LOG_PRINT(log_error, "Failed to connect to redis server: %s:%u!", m_strRedisHost.c_str(), m_nRedisPort);
//			return -1;
//		}
//	}
//	else
//	{
//		LOG_PRINT(log_error, "Failed to connect to redis server: %s:%u!", m_strRedisHost.c_str(), m_nRedisPort);
//		return -1;
//	}

	return 0;
}

void ServerEvent::redis_conn_exception()
{
	CAlarmNotify::sendAlarmNoty(e_all_notitype, e_redis_connect, Application::getInstance()->getProcName(), "roomsvr disconnected with redis", "Yunwei,Room",
			"roomsvr[%u] disconnected with redis[%s:%u]", g_pEvent->m_pApp->getPort(), g_pEvent->m_strRedisHost.c_str(), g_pEvent->m_nRedisPort);
}

/*
void ServerEvent::redis_handle_message(const char* channel, const char* msg, int msglen)
{
	if (!msg || 0 == *msg)
		return;

	if (0 == strcasecmp(channel, CHANNEL_REDIS_SYNC)){
		Json::Reader reader(Json::Features::strictMode());
		Json::Value root;
		if (!reader.parse(msg, root)) {
			LOG_PRINT(log_error, "invalid Json format: %s", msg);
			return;
		}
	}
}
*/
void ServerEvent::send_hello(clienthandler_ptr conn)
{
	char buf[128] = {0};
	CMDGateHello_t cmdHello = {12, 8, 7, e_gateway_type, 1};
	COM_MSG_HEADER *in_msg = (COM_MSG_HEADER *)buf;
	in_msg->checkcode = CHECKCODE;
	in_msg->maincmd = MDM_Vchat_Room;
	in_msg->subcmd = Sub_Vchat_ClientHello;
	CMDGateHello_t *pcmdHello = (CMDGateHello_t *)in_msg->content;
	memcpy(pcmdHello, &cmdHello, sizeof(cmdHello));
	in_msg->length = SIZE_IVM_HEADER + sizeof(cmdHello);
	if (conn && in_msg->length)
		conn->write_message(buf, in_msg->length);
}

void ServerEvent::onTimer()
{
	unsigned int ntime_begin, ntime_end;
	ntime_begin = SL_Socket_CommonAPI::util_process_clock_ms();

	if (m_workid != 0)
		return;

	time_t tNow = time(NULL);
//	if (tNow % 60 == 0) {
		vector<string> vecRoomid = m_roomMgr.getAllRoomid();
		m_roomMgr.updateRoomPopularity();
//	}
	
	ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();

	if (tNow % 30 == 0) {
		LOG_PRINT(log_info, "------ timespan ms:%u for onTimer", ntime_end-ntime_begin);
	}
}

AppDelegate* ServerEvent::getApp()
{
	return m_pApp;
}

void ServerEvent::setApp(AppDelegate *pApp)
{
	m_pApp = pApp;
}

bool ServerEvent::onConnect(clienthandler_ptr  connection)
{
	if (connection == m_roomMgr.m_clientconn.get_connecthandle()) // TODO: necessary??
		send_hello(connection);
	return true;
}

bool ServerEvent::onMessage(task_proc_data * message)
{
	m_logicCore.onMessage(message);
	return true;
}

bool ServerEvent::onClose(clienthandler_ptr connection)
{
	m_roomMgr.m_mapConn.erase(connection->getconnid());
	LOG_PRINT(log_error, "[connid:%u,gateid:%u,%s:%u] connection disconnected", connection->getconnid(), connection->getgateid(),
			connection->getremote_ip(), connection->getremote_port());

	if (0 == m_workid)
		m_roomMgr.onGatewayDisconnected(connection);
	return true;
}

bool ServerEvent::onError(clienthandler_ptr connection, int code, std::string msg)
{
	LOG_PRINT(log_error, "[connid:%u,gateid:%u,%s:%u] connection error", connection->getconnid(), connection->getgateid(),
			connection->getremote_ip(), connection->getremote_port());
	return true;
}




