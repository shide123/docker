
#ifndef __APP_PUSH_H__
#define __APP_PUSH_H__

#include "Application.h"
#include "ServerMsgMgr.h"
#include "node_client.h"
#include <boost/regex.hpp>
#include "redis_map_map.h"
#include "CUserGroupinfo.h"

using namespace std;

class AppPush:public Application,public IWorkerEvent, public INodeNotify
{
public:
	AppPush(void);
	virtual ~AppPush(void);

	virtual bool onWorkerStart(Application* worker);
	virtual bool onWorkerStop(Application* worker);
	ServerMsgMgr m_ServerMsgMgr;
private:
	bool initRedis();
	virtual void onNodeEvent(const std::string & svrname, const std::string & ip, int port, 
		en_node_event event, std::string desc = "");
	void initNodeClient();
	int m_ListenPort;
public:
	redisMgr    *m_pRedisMgr;
	redis_map_map m_rdsOnlineUser;
private:
	Application *m_pListen;
	INodeClient* m_pNodeClient;
	INodeClient* m_pRpcNodeClient;
	std::set<std::string> m_sSubRpcServer;
	boost::regex 		m_expression;
};

#define AppInstance() ((AppPush *)Application::getInstance())

#endif //__APP_PUSH_H__

