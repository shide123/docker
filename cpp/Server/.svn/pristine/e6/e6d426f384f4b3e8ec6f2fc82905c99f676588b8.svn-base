#include "LogonSvrApp.h"
#include <stdio.h>
#include "timer.h"
#include "DBConnection.h"
#include "LogonEvent.h"
#include "GlobalSetting.h"
#include "DBTools.h"
#include "utils.h"

CLogonSvrApp::CLogonSvrApp(void)
{
}

CLogonSvrApp::~CLogonSvrApp(void)
{
	if (m_pLogonEvent)
	{
		delete m_pLogonEvent;
		m_pLogonEvent = NULL;
	}

	if (m_pNodeClient)
	{
		delete m_pNodeClient;
		m_pNodeClient = NULL;
	}

	if (CGlobalSetting::m_app)
	{
		delete CGlobalSetting::m_app;
		CGlobalSetting::m_app = NULL;
	}

	if(m_pApplication)
	{
		delete m_pApplication;
		m_pApplication = NULL;
	}
}

bool CLogonSvrApp::onWorkerStart(Application* worker)
{
    LOG_PRINT(log_info, "onWorkerStart worker id:%d.", worker->m_id);

    std::string conf_file = getPath(PATH_TYPE_CONFIG) + "comm.conf";
    if(!Dbconn::m_config.loadfile(conf_file))
	{
        std::cerr << "load configure file " << conf_file << " failed, error: " << strerror(errno) << endl;
        killAll();
		return false;
    }

	CDBTools::initDB(Dbconn::getInstance("maindb"));
	if (!CDBTools::initSyncDB())
	{
		LOG_PRINT(log_error, "initSyncDB error, please check the config file: %s", conf_file.c_str());
		killAll();
		return false;
	}

    m_pLogonEvent = new LogonEvent(worker->m_id);
	m_pLogonEvent->loadConfig();
	m_pLogonEvent->init();
   
    m_uLanPort = m_config.getInt("self", "startport") + worker->m_id;
    LOG_PRINT(log_info, "listen port %d.", m_uLanPort);
    string protocol = m_config.getString("self", "startprotocol");
    m_pApplication = new Application(protocol, m_uLanPort, m_pLogonEvent);

    LogonServerApplication * app = new LogonServerApplication();
    CGlobalSetting::m_app = app;
    CGlobalSetting::m_app->init();

	init_node();

    return true;
}

bool CLogonSvrApp::onWorkerStop(Application* worker)
{
	LOG_PRINT(log_info, "onWorkerStop worker id:%d.", worker->m_id);
    return true;
}

void CLogonSvrApp::init_node()
{
	// 1. create node client. register watcher.
	LOG_PRINT(log_info, "Create Node Client: svr[%s], port[%u]", getProcName().c_str(), m_uLanPort);
	m_pNodeClient = INodeClient::CreateObject(getProcName().c_str(), Application::get_io_service(), NULL, "", m_uLanPort);

	// 2. add node service ip/dns. begin connect node.
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
}

