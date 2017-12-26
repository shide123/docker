#include "AppPush.h"
#include "CLogThread.h"
#include "DBConnection.h"
#include "TTpPushSvrHandler.h"
#include "CThriftSvrMgr.h"
#include "CUserGroupinfo.h"
#include "macro_define.h"

#define CONFIG_SERVERMSGMGR "ServerMsgMgr"

AppPush::AppPush(void):
m_pListen(NULL), 
m_pNodeClient(NULL), 
m_pRpcNodeClient(NULL), 
m_pRedisMgr(NULL)
{
}

AppPush::~AppPush(void)
{
}

bool AppPush::onWorkerStart(Application* worker)
{
    char version[100];
	const char ver[] = "1.0.0.0";
	sprintf(version, "Ver%s (Build %s %s)", ver, __DATE__, __TIME__);
	LOG_PRINT(log_info, "   /***********************************************************/");
	LOG_PRINT(log_info, "                                                                ");
	LOG_PRINT(log_info, "                                                                ");
	LOG_PRINT(log_info, "       Start run AppPush %s", version);
	LOG_PRINT(log_info, "                                                                ");
	LOG_PRINT(log_info, "                                                                ");
	LOG_PRINT(log_info, "   /***********************************************************/");
	LOG_PRINT(log_info, "worker id:%d",worker->m_id);
    
	Dbconn::m_config.loadfile(getPath(PATH_TYPE_CONFIG) + "comm.conf"); 
    do
	{
		m_config.get(m_ListenPort, CONFIG_SERVERMSGMGR, "startport", 9010);
		m_ListenPort += m_id;

		string protocol;
		m_config.get(protocol, CONFIG_SERVERMSGMGR, "protocol");
		m_pListen = new Application(protocol, m_ListenPort, &m_ServerMsgMgr);
		initNodeClient();

		if (!initRedis())
			break;

        if (!m_ServerMsgMgr.Init(m_config))
            break;

		boost::shared_ptr<TTpPushSvrHandler> handler(new TTpPushSvrHandler());
		worker->setRpcProcessor(new TpPushSvrProcessor(handler));

        return true;
    } while(0);
    killAll();
    LOG_PRINT(log_error, "Get incorrect config, process will stop. please correct the config and restart");
	cout<<"Get incorrect config, process will stop. please correct the config and restart"<<endl;     

    return false;
}

bool AppPush::onWorkerStop(Application* worker)
{
	LOG_PRINT(log_info, "worker id:%d",worker->m_id);
	worker->setRpcProcessor(NULL);
	DELETE_POINT(m_pListen);

	return true;
}

bool AppPush::initRedis()
{
	// redis配置信息
	string strRedisHost = m_commConfig.getString("redis", "ip");
	uint16_t nRedisPort = m_commConfig.getInt("redis", "port");
	string strRedisPass = m_commConfig.getString("redis", "password");
	int nRedisThreadNum = m_commConfig.getInt("redis", "threadnum", 1);

	if (strRedisHost.empty() || 0 == nRedisPort || strRedisPass.empty() || 0 == nRedisThreadNum)
	{
		LOG_PRINT(log_error, "redis configure is error host[%s:%d] auth[%s] threadnum[%d].", strRedisHost.c_str(), nRedisPort, strRedisPass.c_str(), nRedisThreadNum);
		return false;
	}

	LOG_PRINT(log_error, "initialize redisMgr, host[%s:%d] threadnum[%d].", strRedisHost.c_str(), nRedisPort, nRedisThreadNum);
	m_pRedisMgr = new redisMgr(strRedisHost.c_str(), nRedisPort, strRedisPass.c_str(), nRedisThreadNum);
	if (!m_pRedisMgr){
		LOG_PRINT(log_error, "Failed to create redis manager object");
		return false;
	}

	CUserGroupinfo::init(m_pRedisMgr);
	m_rdsOnlineUser.init(DB_USER_MGR, "user:", m_pRedisMgr);
	return true;
}

void AppPush::onNodeEvent( const std::string & svrname, const std::string & ip, int port, en_node_event event, std::string desc /*= ""*/ )
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

void AppPush::initNodeClient()
{

	//只关注本机房,匹配ip段正则表达式
	string strIpExpression = m_config.getString("server", "ip_expression");
	LOG_PRINT(log_info, " ip_expression : %s", strIpExpression.c_str());
	m_expression =  boost::regex(strIpExpression);

	LOG_PRINT(log_info, "Create Node Client: svr[%s], port[%u]", getProcName().c_str(), m_ListenPort);
	m_pNodeClient = INodeClient::CreateObject(getProcName().c_str(), Application::get_io_service(), this, "", m_ListenPort);

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
	m_pRpcNodeClient = INodeClient::CreateObject((getProcName()+"_rpc").c_str(), Application::get_io_service(), NULL, "", getRpcPort());
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
