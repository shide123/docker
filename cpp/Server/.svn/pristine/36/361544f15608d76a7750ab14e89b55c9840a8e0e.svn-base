
#ifndef __CHATSERVER_APP_HH__
#define __CHATSERVER_APP_HH__

#include "Application.h"
#include "redisMgr.h"
#include "redisOpt.h"
#include "redis_map.h"
#include "redis_map_map.h"
#include "redis_map_set.h"
#include "DBConnection.h"
#include "KeywordMgr.h"
#include "ConnectionMgr.h"
#include "libnode/node_client.h"
#include "ChatLogic.h"
#include "GateEvent.h"
#include "DBSink.h"
#include <boost/regex.hpp>
#include "RoomManager.h"

#define IFNOTNULL_DELETE(ptr) if (ptr) {delete (ptr); ptr = NULL;}


class CChatServerApp:public Application,public IWorkerEvent, public INodeNotify
{
public:
	CChatServerApp();
	virtual ~CChatServerApp();

	virtual bool onWorkerStart(Application* worker);
	virtual bool onWorkerStop(Application* worker);
	static CChatServerApp *getInstance();
	uint16 getServiceId();

private:
	void loadSelfConfig();
	bool initRedis();
	void init_node();
	void initDbRelated();
	void releaseDbRelated();
	virtual void onNodeEvent(const std::string & svrname, const std::string & ip, int port,
		en_node_event event, std::string desc = "");

public: 
	uint32 m_uAlarmQueueSize;
	uint32 m_uAlarmNotifyInterval;
	uint32 m_uAlarmNotifyTime;

	CKeywordMgr			m_KeywordMgr;
	CConnectionMgr		m_GateConnMgr;
	CGateEvent			m_GateEvent;

	redisMgr 	*m_pRedisMgr;
	redis_map_map m_rdsOnlineUser;

	//for room info
	CRoomManager *	m_room_mgr;

private:
	uint32 m_uLanPort;
	Application * m_pInnerTcpWorker;
	INodeClient	* m_pNodeClient;
	INodeClient	* m_pRpcNodeClient;
	std::set<std::string> m_sSubRpcServer;
	boost::regex  m_expression;
};

#define AppInstance() CChatServerApp::getInstance()

#endif //__CHATSERVER_APP_HH__

