#include "AppDelegate.h"
#include <stdio.h>
#include "timer.h"
#include "utils.h"
#include "DBConnection.h"
#include <thrift/TProcessor.h>
#include "TPushMsgHandler.h"
#include "rpcclienthandler.h"
#include "CThriftSvrMgr.h"
using namespace apache::thrift;

#define DECLARE_PROCESSER(x,y) boost::shared_ptr<x##Handler> handler(new x##Handler());\
								TProcessor* y = new x##Processor(handler);

AppDelegate::AppDelegate(void)
{
	m_lanPort = 0;
	m_nTimeid = 0;
	_innerTcpWorker = NULL;
	m_pushmsgLogic 	= NULL;
	m_sSubRpcServer.clear();
}

AppDelegate::~AppDelegate(void)
{
	if (_innerTcpWorker) {
		delete _innerTcpWorker;
		_innerTcpWorker = NULL;
	}
	
	if (m_pushmsgLogic) {
		delete m_pushmsgLogic;
		m_pushmsgLogic = NULL;
	}
}

bool AppDelegate::onWorkerStart(Application* worker)
{
	printf("onWorkerStart worker id:%d\n",worker->m_id);
	
	DECLARE_PROCESSER(TPushMsg, processer);
	worker->setRpcProcessor(processer);

	Dbconn::m_config.loadfile(getPath(PATH_TYPE_CONFIG) + "comm.conf");

	m_pushmsgLogic = new PushmsgLogic();
	m_pushmsgLogic->loadConfig(m_commConfig);
	m_pushmsgLogic->init(m_config);

	m_lanPort = m_config.getInt("self", "startport") + worker->m_id;
	m_protocol = m_config.getString("self", "protocol");
	_innerTcpWorker = new Application(m_protocol, m_lanPort, m_pushmsgLogic);

	//只关注本机房,匹配ip段正则表达式
	string strIpExpression = m_config.getString("server", "ip_expression");
	LOG_PRINT(log_info, " ip_expression : %s", strIpExpression.c_str());
	m_expression =  boost::regex(strIpExpression);

	CThriftSvrMgr::init(&m_commConfig);
	init_node(m_lanPort);

	return true;
}

bool AppDelegate::onWorkerStop(Application* worker)
{
	LOG_PRINT(log_info, "onWorkerStop worker id:%d.", worker->m_id);

	if (m_nTimeid)
		Timer::getInstance()->del(m_nTimeid);

	return true;
}

bool AppDelegate::onMessage(task_proc_data * message)
{
	return m_pushmsgLogic->onMessage(message);
}
bool AppDelegate::onConnect(clienthandler_ptr connection)
{
	return m_pushmsgLogic->onConnect(connection);
}

bool AppDelegate::onClose(clienthandler_ptr connection)
{
	return m_pushmsgLogic->onClose(connection);
}

AppDelegate * AppDelegate::getInstance()
{
	return (AppDelegate *)Application::getInstance();
}

void AppDelegate::onNodeEvent(
		const std::string & svrname,
		const std::string & ip,
        int port,
        en_node_event event,
        std::string desc)
{
	/*
	 * @svrname: 	service name, such as "logonsvr", "roomsvr".
	 * @ip:			service ip.
	 * @port:		service port.
	 * @event:		add or remove. connect if add, and disconnect where remove.
	 */
	LOG_PRINT(log_info, "recieve node event: svrname[%s] ip[%s] port[%d] event[%d]", svrname.c_str(), ip.c_str(), port, event);
	if (svrname.empty() || ip.empty() || 0 == port)
		return;

	int svrtype = CThriftSvrMgr::change_svrname_to_type(svrname);
	if (!svrtype)
	{
		return;
	}

	if (event == en_svr_add)
	{
		if (boost::regex_match(ip, m_expression))
		{
			if (m_sSubRpcServer.end() != m_sSubRpcServer.find(svrname))
			{
				LOG_PRINT(log_info, "add new native server:%s svr_type:%d ip:%s port:%d.", svrname.c_str(), svrtype, ip.c_str(), port);
				CThriftSvrMgr::addThriftClient(ip, port, (e_SvrType)svrtype, true);
			}
		}
		else
		{
			if (m_sSubRpcServer.end() != m_sSubRpcServer.find(svrname))
			{
				LOG_PRINT(log_info, "add new remote server:%s svr_type:%d ip:%s port:%d.", svrname.c_str(), svrtype, ip.c_str(), port);
				CThriftSvrMgr::addThriftClient(ip, port, (e_SvrType)svrtype, false);
			}
		}
	}
	else if (event == en_svr_rem)
	{
		if (m_sSubRpcServer.end() != m_sSubRpcServer.find(svrname))
		{
			LOG_PRINT(log_info, "del rpc server:%s svr_type:%d ip:%s port:%d", svrname.c_str(), svrtype, ip.c_str(), port);
			CThriftSvrMgr::delThriftClient(ip, port, (e_SvrType)svrtype);
		}
	}
}

void AppDelegate::init_node(int port)
{
	std::string svrname(Application::getInstance()->getProcName());

	// 1. create node client. register watcher.
	m_pNodeClient = INodeClient::CreateObject(svrname.c_str(), Application::get_io_service(), this, "", port);	
	LOG_PRINT(log_info, "Create NodeClient, svrname:%s, port:%d", svrname.c_str(), port);
	if (!m_pNodeClient)
	{
		LOG_PRINT(log_error, "Create NodeClient error, svrname:%s port:%d", svrname.c_str(), port);
		killAll();
	}

	// 2. subscribe service event if need.
	std::string subscribe(m_config.getString("server", "svr_name"));
	m_pNodeClient->subscribe(subscribe.c_str());
	splitStrToSet(subscribe, ",", m_sSubRpcServer);

	// 3. add node service ip/dns. begin connect node.
	std::vector<std::string> addrs;
	std::string straddrs = m_commConfig.getString("node", "svr");
	LOG_PRINT(log_info, "node addr:%s", straddrs.c_str());

	if (straddrs.empty()) {
		return;
	}
    m_pNodeClient->set_address(straddrs.c_str());
    m_pNodeClient->connect();
    return;
}

