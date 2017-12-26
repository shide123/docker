#include "ConsumeApp.h"
#include <stdio.h>
#include "CLogThread.h"
#include "timer.h"
#include "utils.h"
#include "boost/bind.hpp"
#include "errcode.h"
#include "macro_define.h"
#include "clienthandler.h"
#include "DBTools.h"
#include "msgcommapi.h"
#include "CUserGroupinfo.h"
#include "CThriftSvrMgr.h"
#include "RoomManager.h"
#include "CRedPacketMgr.h"
#include "CUserBasicInfo.h"
#include "GroupBasicInfo.h"
#include "TConsumeSvrHandler.h"
#include "DBSink.h"
#include "TreasureMgr.h"

ConsumeApp::ConsumeApp():
	m_tcpworker(NULL),
	m_redpacketmgr(NULL),
	m_room_mgr(NULL),
	m_pRedisMgr(NULL),
	m_dbconn(NULL),
	m_lanPort(0),
	m_timerid(0),
	m_lasttime_modconfig(0),
	m_lasttime_recovercoin(0),
	m_configfile(""),
	m_giftFilePath(""),
	m_strRedisHost(""),
	m_nRedisPort(0),
	m_strRedisPwd(""),
	m_redisThreadNum(0),
	m_pRecycleTimer(NULL),
	m_pRecycleServicePool(NULL)
{
	m_sSubRpcServer.clear();
}

ConsumeApp::~ConsumeApp()
{
	DELETE_POINT(m_pRedisMgr);
	DELETE_POINT(m_redpacketmgr);
	DELETE_POINT(m_room_mgr);
	DELETE_POINT(m_pNodeClient);
	DELETE_POINT(m_pRpcNodeClient);
	DELETE_POINT(m_tcpworker);
	DELETE_POINT(m_pRecycleTimer);
	DELETE_POINT(m_pRecycleServicePool);
}

bool ConsumeApp::onWorkerStart(Application * worker)
{
	LOG_PRINT(log_info, "onWorkerStart %s worker id:%d.", getProcName().c_str(), worker->m_id);
	srand(time(NULL));

	m_lanPort = m_config.getInt("self", "startport") + worker->m_id;
	std::string protocol = m_config.getString("self", "startprotocol");
	m_tcpworker = new Application(protocol, m_lanPort, this);

	boost::shared_ptr<TConsumeSvrHandler> handler(new TConsumeSvrHandler());
	TProcessor * processer = new TConsumeSvrProcessor(handler);
	worker->setRpcProcessor(processer);

	initRedis();
	initDB();
	initMgr();
	initNode();
	
	LOG_PRINT(log_info, "start timer.timeid:%u.", m_timerid);
	m_timerid = Timer::getInstance()->add(10, boost::bind(&ConsumeApp::onTimer, this), TIMES_UNLIMIT);
	startRecycleTimer();
	
	return true;
}

bool ConsumeApp::onWorkerStop(Application * worker)
{
	LOG_PRINT(log_info, "onWorkerStop %s worker id:%d.", getProcName().c_str(), worker->m_id);
	LOG_PRINT(log_info, "stop timer.timeid:%u.", m_timerid);
	Timer::getInstance()->del(m_timerid);
	stopRecycleTimer();
	return true;
}

bool ConsumeApp::onConnect(clienthandler_ptr client_conn)
{
	if (!client_conn)
	{
		LOG_PRINT(log_error, "onConnect fail,connection is null.");
		return false;
	}

	LOG_PRINT(log_info, "connection onConnect,client connid:%u.", client_conn->getconnid());

	return true;
}

bool ConsumeApp::onMessage(task_proc_data * message)
{
	if (!message)
	{
		LOG_PRINT(log_error, "onMessage failed, message is null.");
		return false;
	}

	clienthandler_ptr client = message->connection;
	if (!client)
	{
		LOG_PRINT(log_error, "onMessage failed, message connection is null.");
		return false;
	}

	if (!message->pdata || !message->datalen)
	{
		LOG_PRINT(log_error, "onMessage failed, message data or datalen is null.");
		return false;
	}

	COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *)message->pdata;
	if (in_msg->version != MDM_Version_Value)
	{
		LOG_PRINT(log_error, "packet version checks fail!client connid:%u,%s:%d.", client->getconnid(), client->getremote_ip(), client->getremote_port());
		return false;
	}

	if(in_msg->length != message->datalen)
	{
		LOG_PRINT(log_error, "packet length checks fail!subcmd:%u,client connid:%u,%s:%d.", in_msg->subcmd, client->getconnid(), client->getremote_ip(), client->getremote_port());
		return false;
	}

	if (Sub_Vchat_ClientHello == in_msg->subcmd)
	{
		handle_hello_msg((char *)in_msg, client);
		return true;
	}

	if (!client->bSayHello)
	{
		LOG_PRINT(log_error, "throw message!because this connection do not say hello.maincmd:%u:subcmd:%u,client:%s:%u,gateid:%u,client connid:%u.", \
			in_msg->maincmd, in_msg->subcmd, client->getremote_ip(), client->getremote_port(), client->getgateid(), client->getconnid());
		return false;
	}

	client->lastalivetime = time(NULL);

	if (Sub_Vchat_ClientPing == in_msg->subcmd)
	{
		handle_ping_msg(message);
		return true;
	}

	LOG_PRINT(log_info, "message come.len:%d,maincmd:%u:subcmd:%u,client:%s:%u,gateid:%u,client connid:%u.", \
		message->datalen, in_msg->maincmd, in_msg->subcmd, client->getremote_ip(), client->getremote_port(), client->getgateid(), client->getconnid());

	dispatchMessage(message);
	return true;
}

void ConsumeApp::handle_hello_msg(char * in_msg, clienthandler_ptr client)
{
	if (!in_msg)
	{
		LOG_PRINT(log_error, "in_msg is null.");
		return;
	}

	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)in_msg;
	if (pHead->length != sizeof(COM_MSG_HEADER) + sizeof(CMDGateHello_t))
	{
		LOG_PRINT(log_error, "hello msg length is wrong,input len:%d.", pHead->length);
		return;
	}

	CMDGateHello_t * pHelloMsg = (CMDGateHello_t *)pHead->content;
	if (pHelloMsg->param1 == 12 && pHelloMsg->param2 == 8 && pHelloMsg->param3 == 7)
	{
		client->bSayHello = true;
		unsigned int connid = client->getconnid();
		int gateid = pHelloMsg->gateid;
		client->setgateid(gateid);

		LOG_PRINT(log_info, "recv hello message,client:%s:%u,client connid:%u,gateid:%u,svr_type:%d.", client->getremote_ip(), client->getremote_port(), connid, gateid, (int)pHelloMsg->param4);

		if (e_gateway_type == pHelloMsg->param4)
		{
			if (gateid)
			{
				m_room_mgr->m_mapConn.insert(std::make_pair(connid, client));
				LOG_PRINT(log_info, "save gate conn to memory,client:%s:%u,client connid:%u,gateid:%u", client->getremote_ip(), client->getremote_port(), connid, gateid);
			}

			if (!m_id)
			{
				register_cmd_msg(client);
			}
		}
	}

	return;
}

void ConsumeApp::register_cmd_msg(clienthandler_ptr client)
{
	std::string cmdlist = m_config.getString("cmd", "cmdlist");

	std::list<std::string> cmdrange_lst;
	cmdrange_lst.clear();

	int i = 1;
	while (1)
	{
		char cmdrange[LEN128] = {0};
		sprintf(cmdrange, "cmdrange_%d", i);
		std::string strcmdrange = m_config.getString("cmd", cmdrange);
		if (strcmdrange.empty())
		{
			break;
		}

		cmdrange_lst.push_back(std::string(cmdrange) + "=" + strcmdrange);
		++i;
	}

	if (cmdrange_lst.empty() && cmdlist.empty())
	{
		LOG_PRINT(log_info, "[Register cmd]this server has no cmd to register.");
		return;
	}

	char szBuf[512] = {0};
	int msglen = CMsgComm::Build_RegisterCmd_Msg(szBuf, sizeof(szBuf), e_consumesvr_type, cmdlist, cmdrange_lst);
	if (msglen > 0)
	{
		client->write_message(szBuf, msglen);
	}
}

void ConsumeApp::dispatchMessage(task_proc_data * message)
{
	COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *)message->pdata;
	ClientGateMask_t * pGateMask = (ClientGateMask_t *)in_msg->content;

	unsigned long begin_time = SL_Socket_CommonAPI::util_process_clock_ms();
	switch(in_msg->subcmd)
	{
	case Sub_Vchat_SendRedPacketReq:
		m_redpacketmgr->handle_sendRedPacketReq(message);
		break;
	case Sub_Vchat_CatchRedPacketReq:
		m_redpacketmgr->handle_catchRedPacketReq(message);
		break;
	case Sub_Vchat_TakeRedPacketReq:
		m_redpacketmgr->handle_takeRedPacketReq(message);
		break;
	case Sub_Vchat_QryRedPacketInfoReq:
		m_redpacketmgr->handle_qryRedPacketInfoReq(message);
		break;
	case Sub_Vchat_QryWeekListReq:
		m_redpacketmgr->handle_qryWeekContributeReq(message);
		break;
	case Sub_Vchat_QryRedPacketReq:
		m_redpacketmgr->handle_QryFixTimeRedPacketReq(message);
		break;
	case Sub_Vchat_SponsorTreasureReq:
		CTreasureMgr::procSponsorTreasureReq(*message);
		break;
	case Sub_Vchat_BuyTreasure:
		CTreasureMgr::procBuyTreasure(*message);
		break;
	case Sub_Vchat_QryMyTreasureDetail:
		CTreasureMgr::procQryMyTreasureDetail(*message);
		break;
	case Sub_Vchat_QryGroupTreasureList:
		CTreasureMgr::procQryGroupTreasureList(*message);
		break;
	case Sub_Vchat_QryMyTreasureList:
		CTreasureMgr::procQryMyTreasureList(*message);
		break;
	case Sub_Vchat_QryTreasureInfo:
		CTreasureMgr::procQryTreasureInfo(*message);
		break;
	default:
		LOG_PRINT(log_warning, "unknown request subcmd:%d", in_msg->subcmd);  			    
		message->connection->resperrinf(in_msg, pGateMask, ERR_CODE_FAILED_UNKNONMESSAGETYPE);
		break;
	}
	unsigned long end_time = SL_Socket_CommonAPI::util_process_clock_ms();
	LOG_PRINT(log_info, "[cost]proc cmd:%u ms = %lu.", in_msg->subcmd, end_time - begin_time);
}

void ConsumeApp::handle_ping_msg(task_proc_data * task_node)
{
	if (task_node && task_node->connection && task_node->pdata)
	{
		COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *)task_node->pdata;

		SL_ByteBuffer outbuf(SIZE_IVM_HEADER + sizeof(CMDClientPingResp_t));

		COM_MSG_HEADER * pmsgheader = (COM_MSG_HEADER *)outbuf.buffer();
		memcpy(pmsgheader, in_msg, SIZE_IVM_HEADER);
		pmsgheader->subcmd = Sub_Vchat_ClientPingResp;

		CMDClientPingResp_t * rsp = (CMDClientPingResp_t *)(pmsgheader->content);
		CMDClientPing_t * req = (CMDClientPing_t *)in_msg->content;
		rsp->userid = req->userid;
		rsp->roomid = req->roomid;

		pmsgheader->length = SIZE_IVM_HEADER + sizeof(CMDClientPingResp_t);
		outbuf.data_end(pmsgheader->length);
		task_node->connection->write_message(outbuf, true);
	}
}

bool ConsumeApp::onClose(clienthandler_ptr client_conn)
{
	if (client_conn)
	{
		if (m_room_mgr->m_mapConn.end() != m_room_mgr->m_mapConn.find(client_conn->getconnid()))
		{
			m_room_mgr->m_mapConn.erase(client_conn->getconnid());
			LOG_PRINT(log_info, "remove gate conn from memory:%s:%u,gateid:%u,client connid:%u.", \
				client_conn->getremote_ip(), client_conn->getremote_port(), client_conn->getgateid(), client_conn->getconnid());
		}
		LOG_PRINT(log_info, "connection onClose,client connid:%u.", client_conn->getconnid());
	}
	return true;
}

bool ConsumeApp::onError(clienthandler_ptr client_conn, int code, std::string msg)
{
	return true;
}

void ConsumeApp::onNodeEvent(const std::string & svrname, const std::string & ip, int port, en_node_event event, std::string desc /*= ""*/)
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
			if (m_sSubRpcServer.end() != m_sSubRpcServer.find(svrname))
			{
				LOG_PRINT(log_info, "found native new svr_type:%d ip:%s port:%d, connect", svrtype, ip.c_str(), port);
				CThriftSvrMgr::addThriftClient(ip, port, svrtype, true);
			}
		}
		else
		{
			if (m_sSubRpcServer.end() != m_sSubRpcServer.find(svrname))
			{
				LOG_PRINT(log_info, "found remote new svr_type:%d ip:%s port:%d, connect", svrtype, ip.c_str(), port);
				CThriftSvrMgr::addThriftClient(ip, port, svrtype, false);
			}
		}
	}
	else if (event == en_svr_rem)
	{
		if (m_sSubRpcServer.end() != m_sSubRpcServer.find(svrname))
		{
			LOG_PRINT(log_info, "svr_type:%d ip:%s port:%d had closed", svrtype, ip.c_str(), port);
			CThriftSvrMgr::delThriftClient(ip, port, svrtype);
		}
	}
}

void ConsumeApp::initRedis()
{
	m_strRedisHost = m_commConfig.getString("redis", "ip");
	m_nRedisPort = m_commConfig.getInt("redis", "port");
	m_strRedisPwd = m_commConfig.getString("redis", "password");
	m_redisThreadNum = m_commConfig.getInt("redis", "threadnum");
	LOG_PRINT(log_info, "redis config:%s:%u,pwd:%s,thread num:%u.", m_strRedisHost.c_str(), m_nRedisPort, m_strRedisPwd.c_str(), m_redisThreadNum);
	
	m_pRedisMgr = new redisMgr(m_strRedisHost.c_str(), m_nRedisPort, m_strRedisPwd.c_str(), m_redisThreadNum);
}

void ConsumeApp::initNode()
{
	//RPC 只关注本机房,匹配ip段正则表达式
	std::string strIpExpression = m_config.getString("server", "ip_expression");
	LOG_PRINT(log_info, "ip_expression:%s", strIpExpression.c_str());
	m_expression = boost::regex(strIpExpression);
	CThriftSvrMgr::init(&(Application::getInstance()->m_commConfig));

	// 1. create node client. register watcher.
	LOG_PRINT(log_info, "Create Node Client: svr[%s], port[%u]", getProcName().c_str(), m_lanPort);
	m_pNodeClient = INodeClient::CreateObject(getProcName().c_str(), Application::get_io_service(), NULL, "", m_lanPort);
	m_pRpcNodeClient = INodeClient::CreateObject((getProcName()+"_rpc").c_str(), Application::get_io_service(), this, "", Application::getRpcPort());

	// 2. subscribe service event if need.
	std::string subscribe(m_config.getString("server", "rpcsvr_name"));
	splitStrToSet(subscribe, ",", m_sSubRpcServer);
	LOG_PRINT(log_info, "node subscribe:%s", subscribe.c_str());
	m_pRpcNodeClient->subscribe(subscribe.c_str());

	// 3. add node service ip/dns. begin connect node.
	std::string straddrs = m_commConfig.getString("node", "svr");
	LOG_PRINT(log_info, "node addr:%s", straddrs.c_str());

	if (straddrs.empty() || !m_pNodeClient || !m_pRpcNodeClient) 
	{
		LOG_PRINT(log_error, "node server config is empty.please check the config.");
		killAll();
		return;
	}

	m_pNodeClient->set_address(straddrs.c_str());
	m_pNodeClient->connect();
	m_pRpcNodeClient->set_address(straddrs.c_str());
	m_pRpcNodeClient->connect();
}

void ConsumeApp::initDB()
{
	std::string conf_file = getPath(PATH_TYPE_CONFIG) + "comm.conf";
	if (!Dbconn::m_config.loadfile(conf_file) || !Dbconn::initDBSinkPool(CDBSink::m_nPlatId))
	{
		LOG_PRINT(log_error, "load configure file %s failed.", conf_file.c_str());
		killAll();
		return;
	}

	CDBTools::initDB(Dbconn::getInstance("maindb"));
	CDBTools::initSyncDB();
	m_dbconn = Dbconn::getSyncDB();
	if (NULL == m_dbconn)
	{
		LOG_PRINT(log_error, "getSyncDB error, check config file:%s", conf_file.c_str());
		killAll();
		return;
	}
}

void ConsumeApp::initMgr()
{
	m_configfile = getPath(PATH_TYPE_CONFIG) + getProcName() + ".conf";
	LOG_PRINT(log_info, "configfile:%s.", m_configfile.c_str());
	struct stat buf;
	stat(m_configfile.c_str(), &buf);
	m_lasttime_modconfig = buf.st_mtime;

	//roommgr,redis should be initial first.
	m_room_mgr = new CRoomManager();
	m_room_mgr->setRedisConf(m_pRedisMgr);

	//user group info initial
	CUserGroupinfo::init(m_pRedisMgr);
	CUserBasicInfo::init(m_pRedisMgr);
	CGroupBasicInfo::init(m_pRedisMgr);

	//red packet mgr
	m_redpacketmgr = new CRedPacketMgr(Dbconn::getInstance("maindb"));
	m_redpacketmgr->init(m_pRedisMgr);

	return;
}

void ConsumeApp::onTimer()
{
	check_config_file();
}

void ConsumeApp::startRecycleTimer()
{
	int cacheTimeOut = m_config.getInt("self", "redpacket_cache_timeout", 0);
	if (cacheTimeOut)
	{
		CRedPacketMgr::m_cacheTime = cacheTimeOut;
	}

	int combotime = m_config.getInt("self", "redpacket_combo_time", CRedPacketMgr::m_comboTime);
	if (combotime > 0 && combotime != CRedPacketMgr::m_comboTime)
	{
		LOG_PRINT(log_info, "redpacket_combo_time is %d", combotime);
		CRedPacketMgr::m_comboTime = combotime;
	}

	CRedPacketMgr::m_validtime = m_config.getInt("self", "redpacket_timeout", CRedPacketMgr::m_validtime);
	int run = m_config.getInt("self", "redpacket_recycle", 0);
	if (run && 0 == m_id)
	{
		LOG_PRINT(log_info, "start timer to recycle redpacket, validtime: %u.", CRedPacketMgr::m_validtime);
		m_pRecycleServicePool = new io_service_pool(1);
		m_pRecycleTimer = new boost::asio::deadline_timer(m_pRecycleServicePool->get_io_service());
		m_pRecycleTimer->expires_from_now(boost::posix_time::seconds(60));
		m_pRecycleTimer->async_wait(boost::bind(&ConsumeApp::onRecycleTimer, this, boost::asio::placeholders::error));
		m_pRecycleServicePool->run();
	}
}

void ConsumeApp::stopRecycleTimer()
{
	boost::mutex::scoped_lock lock(m_mtxRecycleTimer);
	if (m_pRecycleTimer)
	{
		LOG_PRINT(log_info, "stop recycle redpacket timer");
		boost::system::error_code e;
		m_pRecycleTimer->cancel(e);
		delete m_pRecycleTimer;
		m_pRecycleTimer = NULL;
	}

	if (m_pRecycleServicePool)
	{
		m_pRecycleServicePool->stop();
		delete m_pRecycleServicePool;
		m_pRecycleServicePool = NULL;
	}

}

void ConsumeApp::onRecycleTimer(const boost::system::error_code &e)
{
	if (!e)
	{
		CRedPacketMgr::recycleRedPacket();

//		unsigned int now_time = time(NULL);
//		if (now_time - CRedPacketMgr::m_cleanCacheTime > 1 * 3600)
//		{
//			//1 day run 1 time
//			CRedPacketMgr::cleanRedPacketCache(now_time);
//			CRedPacketMgr::m_cleanCacheTime = now_time;
//		}

		boost::mutex::scoped_lock lock(m_mtxRecycleTimer);
		if (m_pRecycleTimer)
		{
			m_pRecycleTimer->expires_from_now(boost::posix_time::seconds(60));
			m_pRecycleTimer->async_wait(boost::bind(&ConsumeApp::onRecycleTimer, this, boost::asio::placeholders::error));
		}
	}
	else                                                                                                                                                                                                   
	{
		LOG_PRINT(log_error, "onRecycleTimer error: [%s] ", boost::system::system_error(e).what());
	}
}

void ConsumeApp::check_config_file()
{
	struct stat buf;
	stat(m_configfile.c_str(), &buf);
	if (buf.st_mtime == m_lasttime_modconfig)
	{
		return;
	}

	m_lasttime_modconfig = buf.st_mtime;

	int line = 0;
	if(!m_config.load(line))
	{
		LOG_PRINT(log_error, "[Error]load config file fail.path:%s.", m_configfile.c_str());
		return;
	}

	int validtime = m_config.getInt("self", "redpacket_timeout", CRedPacketMgr::m_validtime);
	if (validtime > 0 && validtime != CRedPacketMgr::m_validtime)
	{
		LOG_PRINT(log_info, "change redpacket_timeout from %u to %d", CRedPacketMgr::m_validtime, validtime);
		CRedPacketMgr::m_validtime = validtime;
	}

	int combotime = m_config.getInt("self", "redpacket_combo_time", CRedPacketMgr::m_comboTime);
	if (combotime > 0 && combotime != CRedPacketMgr::m_comboTime)
	{
		LOG_PRINT(log_info, "change redpacket_combo_time from %u to %d", CRedPacketMgr::m_comboTime, combotime);
		CRedPacketMgr::m_comboTime = combotime;
	}
}

ConsumeApp * ConsumeApp::getInstance()
{
	return (ConsumeApp *)Application::getInstance();
}
