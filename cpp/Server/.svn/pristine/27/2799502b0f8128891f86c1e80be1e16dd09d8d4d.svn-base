
#ifndef __CONSUMESVR_APP_HH__
#define __CONSUMESVR_APP_HH__

#include <boost/regex.hpp>
#include "Application.h"
#include "clienthandler.h"
#include "node_client.h"
#include "DBConnection.h"
#include "redisMgr.h"
#include "RoomManager.h"

class CRedPacketMgr;

class ConsumeApp: public Application, public INodeNotify, public IWorkerEvent
{
public:
	ConsumeApp();
	virtual ~ConsumeApp();
	static ConsumeApp * getInstance();

	virtual bool onWorkerStart(Application * worker);
	virtual bool onWorkerStop(Application * worker);
	virtual bool onConnect(clienthandler_ptr client_conn);
	virtual bool onMessage(task_proc_data * message);
	virtual bool onClose(clienthandler_ptr client_conn);
	virtual bool onError(clienthandler_ptr client_conn, int code, std::string msg);
	virtual void onNodeEvent(const std::string & svrname, const std::string & ip, int port, en_node_event event, std::string desc = "");

private:
	void initRedis();
	void initNode();
	void initDB();
	void initMgr();

	void onTimer();
	void startRecycleTimer();
	void stopRecycleTimer();
	void onRecycleTimer(const boost::system::error_code &e);
	void check_config_file();

	void handle_hello_msg(char * in_msg, clienthandler_ptr client);
	void register_cmd_msg(clienthandler_ptr client);
	void dispatchMessage(task_proc_data * message);
	void handle_ping_msg(task_proc_data * task_node);

//DATA
public:
	Application *	m_tcpworker;
	CRedPacketMgr * m_redpacketmgr;
	CRoomManager *	m_room_mgr;
	redisMgr *		m_pRedisMgr;
	Dbconnection *	m_dbconn;

private:
	unsigned int	m_lanPort;
	unsigned int	m_timerid;

	unsigned int	m_lasttime_modconfig;
	unsigned int	m_lasttime_recovercoin;
	std::string		m_configfile;
	std::string		m_giftFilePath;

	std::string		m_strRedisHost;
	unsigned int	m_nRedisPort;
	std::string		m_strRedisPwd;
	unsigned int	m_redisThreadNum;

	INodeClient *	m_pNodeClient;
	INodeClient *	m_pRpcNodeClient;
	boost::regex 	m_expression;
	std::set<std::string> m_sSubRpcServer;
	
	//recycle thread
	boost::asio::deadline_timer* m_pRecycleTimer;
	io_service_pool *m_pRecycleServicePool;
	boost::mutex m_mtxRecycleTimer;

};

#endif //__CONSUMESVR_APP_HH__

