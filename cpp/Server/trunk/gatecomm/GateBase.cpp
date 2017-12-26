#include "GateBase.h"
#include "utils.h"

CClientManager CGateBase::m_ClientMgr;
CRoomClientMgr CGateBase::m_RoomClientMgr;
CSvrConnMgr CGateBase::m_SvrConnMgr;
CCmdConfigMgr * CGateBase::m_pCmdConfigMgr = NULL;
ConnectionMgr CGateBase::m_ConnectionMgr;

unsigned int CGateBase::m_uAlarmQueueSize = 10000;
unsigned int CGateBase::m_uClientTimeout = 180;
unsigned int CGateBase::m_uOnMicClientTimeout = 60;
unsigned int CGateBase::m_uServerTimeout = 60;

unsigned int CGateBase::m_uAlarmNotifyInterval = 300;
GATE_TYPE CGateBase::m_eGateType = GATE_TYPE_GATEWAY;
std::map<std::string, unsigned int> CGateBase::m_svrname_type;
std::map<unsigned int, std::string> CGateBase::m_type_svrname;

CGateBase::CGateBase()
{
	m_pNodeClient = NULL;
}

CGateBase::~CGateBase()
{
}

unsigned int CGateBase::getAlarmQueueSize()
{
	return m_uAlarmQueueSize;
}

unsigned int CGateBase::getClientTimeout()
{
	return m_uClientTimeout;
}

unsigned int CGateBase::getOnMicClientTimeout()
{
	return m_uOnMicClientTimeout;
}

unsigned int CGateBase::getAlarmNotifyInterval()
{
	return m_uAlarmNotifyInterval;
}

unsigned int CGateBase::getReqRespCheckTime()
{
	return m_pCmdConfigMgr->getReqRspCheckTime();
}

unsigned short CGateBase::Port()
{
	return Application::getPort();
}

std::string CGateBase::ProcName()
{
	return Application::getInstance()->getProcName();
}

void CGateBase::setGateType(GATE_TYPE type)
{
	m_eGateType = type;
}

GATE_TYPE CGateBase::getGateType()
{
	return m_eGateType;
}

std::string CGateBase::change_type_to_svrname(const unsigned int svr_type)
{
	std::string ret = "unknown";
    std::map<unsigned int, std::string>::iterator iter = m_type_svrname.find(svr_type);
    if (m_type_svrname.end() != iter)
    {
        ret = iter->second;
    }

	return ret;
}

unsigned int CGateBase::change_svrname_to_type(std::string svr_name)
{
    std::string find_svrname = svr_name;
    std::transform(find_svrname.begin(), find_svrname.end(), find_svrname.begin(), ::tolower);

    std::map<std::string, unsigned int>::iterator iter = m_svrname_type.find(find_svrname);
    if (m_svrname_type.end() != iter)
    {
        return iter->second;
    }
    else
    {
        unsigned int svr_type = Application::getInstance()->m_commConfig.getInt("server", find_svrname);
        LOG_PRINT(log_info, "find config:%s.", find_svrname.c_str());
        if (!svr_type)
        {
            LOG_PRINT(log_error, "cannot find this svr_type of %s,please check comm.conf", svr_name.c_str());
            return 0;
        }

        m_svrname_type[svr_name] = svr_type;
        m_type_svrname[svr_type] = svr_name;
        return svr_type;
    }
}

void CGateBase::initNode()
{
	if (m_pNodeClient)
	{
		delete m_pNodeClient;
		m_pNodeClient = NULL;
	}
	LOG_PRINT(log_info, "create node client: svr[%s], port[%u]", getProcName().c_str(), getPort());
	m_pNodeClient = INodeClient::CreateObject(getProcName().c_str(), Application::get_io_service(), this, "", getPort());

	// 2. subscribe service event if need.
	std::string subscribe(m_config.getString("server", "svr_name"));
	LOG_PRINT(log_info, "node subscribe:%s", subscribe.c_str());
	m_pNodeClient->subscribe(subscribe.c_str());

	// 3. add node service ip/dns. begin connect node.
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
}

void CGateBase::unInitNode()
{
	if (m_pNodeClient)
	{
		delete m_pNodeClient;
		m_pNodeClient = NULL;
	}
}

void CGateBase::onNodeEvent(const std::string & svrname, const std::string & ip, int port, en_node_event event, std::string desc/* = ""*/)
{
	LOG_PRINT(log_info, "receive event(%d) from nodemgrsvr, addr[%s:%d]..", event, ip.c_str(), port);
	if (ip.empty() || port <= 0)
	{
		LOG_PRINT(log_warning, "receive invalid addr from nodemgrsvr, svr[%s], addr[%s:%d]..", svrname.c_str(), ip.c_str(), port);
		return;
	}

	switch(event)
	{
	case en_svr_add:
		if (!m_ConnectionMgr.chkConnection(svrname, ip, port))
		{
			uint32 svr_type = change_svrname_to_type(svrname);
			
			LOG_PRINT(log_info, "[node add]new %s.addr:%s:%d,svr_type:%u.", svrname.c_str(), ip.c_str(), port, svr_type);
			CConnection_ptr svr_client(new CConnection("Frame"));
			if (e_usermgrsvr_type == svr_type)
			{
				svr_client->setEvent(&m_usermgrevent);
			}
			else if (e_roomsvr_type <= svr_type && svr_type <= e_goldtrade_type)
			{
				svr_client->setEvent(&m_roomevent);
			}
			else
			{
				svr_client->setEvent(&m_svrevent);
			}

			svr_client->get_connecthandle()->setgateid(m_usGateId);
			svr_client->get_connecthandle()->setsvrtype(svr_type);
			svr_client->connect(ip.c_str(), port);
			m_SvrConnMgr.add_svr_type(svr_type);
			m_ConnectionMgr.addConnection(svrname, svr_client);
		}
		break;
	case en_svr_rem:
		{
			LOG_PRINT(log_info, "[node delete]delete %s.addr:%s:%d.", svrname.c_str(), ip.c_str(), port);
			m_ConnectionMgr.delConnection(svrname, ip, port);
		}
		break;
	default:
		LOG_PRINT(log_warning, "receive unknown event(%d) from nodemgrsvr..", event);
		return;
	}
}
