#include "UsermgrApp.h"
#include <stdio.h>
#include "timer.h"
#include "utils.h"
#include "macro_define.h"
#include "DBTools.h"
#include "CThriftSvrMgr.h"
#include "TUsermgrHandler.h"
#include "DBSink.h"

using namespace apache::thrift;

UsermgrApp::UsermgrApp()
{
	m_lanPort = 0;
	m_nTimeid = 0;

	_innerTcpWorker = NULL;
	m_usermgrevent = NULL;
	m_pNodeClient = NULL;
	m_pRpcNodeClient = NULL;
}

UsermgrApp::~UsermgrApp()
{
	DELETE_POINT(_innerTcpWorker);
	DELETE_POINT(m_usermgrevent);
}

bool UsermgrApp::onWorkerStart(Application * worker)
{
	printf("onWorkerStart worker id:%d\n", worker->m_id);
	LOG_PRINT(log_normal, "onWorkerStart worker id:%d.", worker->m_id);
	std::string conf_file = getPath(PATH_TYPE_CONFIG) + "comm.conf";
	if(!Dbconn::m_config.loadfile(conf_file))
	{
		std::cerr << "load configure file " << conf_file << " failed, error: " << strerror(errno) << endl;
		killAll();
	}

	CDBTools::initDB(Dbconn::getInstance("maindb"));
	Dbconn::initDBSinkPool(CDBSink::m_nPlatId);

	m_usermgrevent = new UsermgrEvent();
	m_usermgrevent->setWorkID(worker->m_id);
	m_usermgrevent->loadConfig(m_commConfig);
	m_usermgrevent->init(m_config);

	boost::shared_ptr<TUsermgrHandler> handler(new TUsermgrHandler());
	TProcessor * processer = new TUserMsgSvrProcessor(handler);
	worker->setRpcProcessor(processer);

	m_lanPort = m_config.getInt("self", "startport") + worker->m_id;
	std::string protocol = m_config.getString("self", "protocol");
	_innerTcpWorker = new Application(protocol, m_lanPort, m_usermgrevent);
	init_node(m_lanPort);

	//m_nTimeid = Timer::getInstance()->add(1, boost::bind(&UsermgrEvent::onTimer, &m_usermgrevent), TIMES_UNLIMIT);
	return true;
}

bool UsermgrApp::onWorkerStop(Application * worker)
{
	printf("onWorkerStop worker id:%d\n", worker->m_id);
	LOG_PRINT(log_normal, "onWorkerStop worker id:%d.", worker->m_id);

	if (m_nTimeid)
	{
		Timer::getInstance()->del(m_nTimeid);
	}

	unInitNode();

	return true;
}

UsermgrApp * UsermgrApp::getInstance()
{
	return (UsermgrApp *)Application::getInstance();
}

void UsermgrApp::onNodeEvent(
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
    LOG_PRINT(log_info, "recv node event: svrname[%s] ip[%s] port[%d] event[%d].", svrname.c_str(), ip.c_str(), port, event);
	if (boost::regex_match(ip, m_expression))
	{
		//TODO
	}
}

void UsermgrApp::init_node(int port)
{
	unInitNode();

	//RPC 只关注本机房,匹配ip段正则表达式
	std::string strIpExpression = m_config.getString("server", "ip_expression");
	LOG_PRINT(log_info, "ip_expression:%s", strIpExpression.c_str());
	m_expression = boost::regex(strIpExpression);
	CThriftSvrMgr::init(&(Application::getInstance()->m_commConfig));

	// 1. create node client. register watcher.
	std::string svrname(Application::getInstance()->getProcName());
	m_pNodeClient = INodeClient::CreateObject(svrname.c_str(), Application::get_io_service(), NULL, "", port);
	m_pRpcNodeClient = INodeClient::CreateObject((getProcName() + "_rpc").c_str(), Application::get_io_service(), this, "", getRpcPort());

	LOG_PRINT(log_info, "Create NodeClient, svrname:%s, port:%d", svrname.c_str(), port);
	if (!m_pNodeClient || !m_pRpcNodeClient)
	{
		LOG_PRINT(log_error, "Create NodeClient error, svrname:%s port:%d", svrname.c_str(), port);
		killAll();
		return;
	}

	// 2. subscribe service event if need.
	std::string subscribe(m_config.getString("server", "rpcsvr_name"));
	if (!subscribe.empty())
	{
		LOG_PRINT(log_info, "node subscribe:%s", subscribe.c_str());
		m_pRpcNodeClient->subscribe(subscribe.c_str());
	}

	// 3. add node service ip/dns. begin connect node.
	std::vector<std::string> addrs;
	std::string straddrs = m_commConfig.getString("node", "svr");
	LOG_PRINT(log_info, "node addr:%s", straddrs.c_str());

	if (straddrs.empty()) 
	{
		killAll();
		return;
	}

    m_pNodeClient->set_address(straddrs.c_str());
    m_pNodeClient->connect();

	m_pRpcNodeClient->set_address(straddrs.c_str());
	m_pRpcNodeClient->connect();
    return;
}

void UsermgrApp::unInitNode()
{
	DELETE_POINT(m_pNodeClient);
	DELETE_POINT(m_pRpcNodeClient);
}
