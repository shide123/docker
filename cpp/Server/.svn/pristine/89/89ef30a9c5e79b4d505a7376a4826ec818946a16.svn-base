
#ifndef __AppDelegate_HH__
#define __AppDelegate_HH__

#include "Application.h"
#include "node_client.h"

#include "clienthandler.h"
#include "PushmsgLogic.h"
#include <boost/regex.hpp>


using namespace std;

class AppDelegate: public Application, public IWorkerEvent, public INodeNotify
{
public:
	AppDelegate(void);
	virtual ~AppDelegate(void);

	virtual bool onWorkerStart(Application* worker);
	virtual bool onWorkerStop(Application* worker);
	virtual bool onConnect(clienthandler_ptr connection);
	virtual bool onMessage(task_proc_data * message);
	virtual bool onClose(clienthandler_ptr connection);
	static AppDelegate * getInstance();
	PushmsgLogic       * m_pushmsgLogic;

	int loadconfig();
	void initApp();

	virtual void onNodeEvent(const std::string & svrname, const std::string & ip, int port, en_node_event event, std::string desc = "");

private:
	void init_node(int port);

private:
	int m_lanPort;
	Application			* _innerTcpWorker;
	INodeClient 		* m_pNodeClient;
	
	boost::regex 		m_expression;
	std::set<std::string> m_sSubRpcServer;
	string 				m_protocol;

    int m_nTimeid;

};

#define AppInstance() AppDelegate::getInstance()

#endif //__LOGON_SERVER_APPLICATION_HH__

