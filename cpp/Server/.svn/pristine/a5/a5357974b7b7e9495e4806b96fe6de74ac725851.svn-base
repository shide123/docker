#include "AppDelegate.h"
#include <stdio.h>
#include "timer.h"
#include "DBConnection.h"
#include "singleton.h"
#include "CThriftSvrMgr.h"
#include "TRoomSvrHandler.h"

AppDelegate::AppDelegate(void)
	: m_lanPort(0),m_Version(1000)
{
	m_pNodeClient = NULL;
	m_pRpcNodeClient = NULL;
	m_sSubRpcServer.clear();
}

AppDelegate::~AppDelegate(void)
{
	unInitNode();
	DELETE_POINT(_innerTcpWorker);
}

AppDelegate * AppDelegate::getInstance()
{
	return (AppDelegate *)Application::getInstance();
}

bool AppDelegate::onWorkerStart(Application* worker)
{
	Dbconn::m_config.loadfile(getPath(PATH_TYPE_CONFIG) + "comm.conf");

	if (!Dbconn::initDBSinkPool(CDBSink::m_nPlatId))
	{
		LOG_PRINT(log_error, "initDBSinkPool error");
		killAll();
	}

	m_lanPort = m_config.getInt("self", "startport") + worker->m_id;
	m_pServerEvent =  new ServerEvent(worker->m_id, m_lanPort);
	g_pEvent = m_pServerEvent;
	_innerTcpWorker = new Application("Frame", m_lanPort, m_pServerEvent);
	m_pServerEvent->Init(this);

	boost::shared_ptr<TRoomSvrHandler> handler(new TRoomSvrHandler());
	TProcessor * processer = new TRoomSvrProcessor(handler);
	worker->setRpcProcessor(processer);

	init_node(m_lanPort);

	return true;
}

bool AppDelegate::onWorkerStop(Application* worker)
{
	return true;
}

void AppDelegate::onNodeEvent(const std::string & svrname, const std::string & ip, int port, en_node_event event, std::string desc /*= ""*/)
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
				LOG_PRINT(log_info, "found new rpc native svr_type:%d ip:%s port:%d, connect", svrtype, ip.c_str(), port);
				CThriftSvrMgr::addThriftClient(ip, port, svrtype, true);
			}
		}
		else
		{
			if (m_sSubRpcServer.find(svrname) != m_sSubRpcServer.end())
			{
				LOG_PRINT(log_info, "found new rpc remote svr_type:%d ip:%s port:%d, connect", svrtype, ip.c_str(), port);
				CThriftSvrMgr::addThriftClient(ip, port, svrtype, false);
			}
		}
	}
	else if (event == en_svr_rem)
	{
		if (m_sSubRpcServer.find(svrname) != m_sSubRpcServer.end())
		{
			LOG_PRINT(log_info, "svr_type:%d ip:%s port:%d had closed", svrtype, ip.c_str(), port);
			CThriftSvrMgr::delThriftClient(ip, port, svrtype);
		}
	}
}

void AppDelegate::init_node(uint16_t port)
{
	//RPC 只关注本机房,匹配ip段正则表达式
	std::string strIpExpression = m_config.getString("server", "ip_expression");
	LOG_PRINT(log_info, "ip_expression:%s", strIpExpression.c_str());
	m_expression = boost::regex(strIpExpression);
	CThriftSvrMgr::init(&(Application::getInstance()->m_commConfig));

	// 1. create node client. register watcher.
	m_pNodeClient = INodeClient::CreateObject(getProcName().c_str(), Application::get_io_service(), NULL, "", port);
	m_pRpcNodeClient = INodeClient::CreateObject((getProcName()+"_rpc").c_str(), Application::get_io_service(), this, "", getRpcPort());

	if (m_pNodeClient == NULL || m_pRpcNodeClient == NULL) 
	{
		LOG_PRINT(log_error, "error, create node obj failed!");
		killAll();
	}

	// 2. subscribe service event if need.
	std::string subscribe(m_config.getString("server", "rpcsvr_name"));
	if (!subscribe.empty())
	{
		LOG_PRINT(log_info, "node subscribe:%s", subscribe.c_str());
		m_pRpcNodeClient->subscribe(subscribe.c_str());
		splitStrToSet(subscribe, ",", m_sSubRpcServer);
	}

	// 3. add node service ip/dns. begin connect node.
	std::string straddrs = m_commConfig.getString("node", "svr");
	m_Appid = m_config.getString("app", "appid");
	m_key = m_config.getString("app", "key");
	m_Version = m_config.getInt("app", "version");
	m_usetoken = m_config.getInt("app", "usetoken");
	LOG_PRINT(log_info, "node addr:%s", straddrs.c_str());
	printf("node addr:%s\n", straddrs.c_str());
	if (straddrs.empty()) 
	{
		LOG_PRINT(log_error, "error, node cfg is wrong.");
		killAll();
	}

	m_pNodeClient->set_address(straddrs.c_str());
	m_pNodeClient->connect();

	m_pRpcNodeClient->set_address(straddrs.c_str());
	m_pRpcNodeClient->connect();
}

void AppDelegate::unInitNode()
{
	DELETE_POINT(m_pNodeClient);
	DELETE_POINT(m_pRpcNodeClient);
}
