
#ifndef __USERMGR_APP_HH__
#define __USERMGR_APP_HH__

#include <boost/regex.hpp>
#include "Application.h"
#include "libnode/node_client.h"
#include "UsermgrEvent.h"

using namespace std;
class UsermgrApp: public Application, public IWorkerEvent, public INodeNotify
{
public:
	UsermgrApp();
	virtual ~UsermgrApp();

	virtual bool onWorkerStart(Application * worker);
	virtual bool onWorkerStop(Application * worker);
	int loadconfig();
	static UsermgrApp * getInstance();

public:
	virtual void onNodeEvent(const std::string & svrname, const std::string & ip, int port, en_node_event event, std::string desc = "");
	UsermgrEvent * m_usermgrevent;

private:
	void init_node(int port);
	void unInitNode();

private:
	int m_lanPort;
	Application	*	_innerTcpWorker;
	INodeClient *	m_pNodeClient;
	INodeClient *	m_pRpcNodeClient;
	boost::regex 	m_expression;
    int m_nTimeid;
};

#endif //__USERMGR_APP_HH__

