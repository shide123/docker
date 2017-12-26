
#ifndef __AppDelegate_HH__
#define __AppDelegate_HH__

#include <boost/regex.hpp>
#include "Application.h"
#include "serverEvent.h"
#include "libnode/node_client.h"
#include "DBSink.h"

using namespace std;

class AppDelegate : public Application, public INodeNotify, public IWorkerEvent
{
private:
	int m_lanPort;
	ServerEvent *m_pServerEvent;
	Application* _innerTcpWorker;
public:
	AppDelegate(void);
	virtual ~AppDelegate(void);
	static AppDelegate * getInstance();

	virtual bool onWorkerStart(Application* worker);
	virtual bool onWorkerStop(Application* worker);
	virtual void onNodeEvent(const std::string & svrname, const std::string & ip, int port, en_node_event event, std::string desc = "");
	ServerEvent *getServerEvent()
	{
		return m_pServerEvent;
	}

	void init_node(uint16_t port);
protected:
	void unInitNode();

	INodeClient *	m_pNodeClient;
	INodeClient *	m_pRpcNodeClient;
	boost::regex	m_expression;
	std::set<std::string> m_sSubRpcServer;
public:
	int m_Version;
	string m_Appid;
	string m_key;
	int m_usetoken;
};

#endif //__LOGON_SERVER_APPLICATION_HH__

