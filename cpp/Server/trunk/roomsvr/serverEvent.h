
#ifndef __SERVEREVENT_H__
#define __SERVEREVENT_H__

#include "ProtocolsBase.h"
#include "clienthandler.h"
#include "LogicCore.h"
#include "RoomManager.h"
#include "redisOpt.h"
#include "redisMgr.h"
#include "json/json.h"
#include "tscon.hpp"
#include "singleton.h"


class AppDelegate;

class ServerEvent: public IWorkerEvent, public boost::enable_shared_from_this<ServerEvent>
{
public:
	ServerEvent(int workid, uint16_t port);
	virtual ~ServerEvent();

	virtual bool onConnect(clienthandler_ptr connection);
	virtual bool onMessage(task_proc_data * message);
	virtual bool onError(clienthandler_ptr connection, int code, std::string msg);
	virtual bool onClose(clienthandler_ptr connection);

public:
	int Init(AppDelegate *pApp);
	int InitRedis();
	CRoomManager* getRoomMgr() { return &m_roomMgr; }
	void setApp(AppDelegate *pApp);
	AppDelegate* getApp();
	static void redis_conn_exception();
	//废弃，codis之后不支持发布订阅功能
	//static void redis_handle_message(const char* channel, const char* msg, int msglen);
	void send_hello(clienthandler_ptr conn);
protected:
	void onTimer();
public:
	AppDelegate *	m_pApp;
	CLogicCore 		m_logicCore;
	CRoomManager 	m_roomMgr;

	string 		m_strRedisHost;
	uint16_t 	m_nRedisPort;
	string		m_sRedisPass;
	int			m_nexcept_exit_interval;

	redisMgr *	m_pRedisMgr;
//	redisOpt *	m_pRedisMsg;
//	redisOpt *	m_pRedisSub;
	tsmap<string, ExceptExitRecord_t> m_mapExceptExit;

protected:
	int m_workid;
	uint16_t m_port;
	int m_svrtimerid;
	int m_logictimerid;
	int m_coursemgrtimerid;
public:
	uint16      m_maxwaitmicuser;
	uint16      m_maxuserwaitmic;
};

typedef boost::shared_ptr<ServerEvent> ServerEvent_ptr;

extern ServerEvent *g_pEvent;

#endif /* __SERVEREVENT_H__ */
