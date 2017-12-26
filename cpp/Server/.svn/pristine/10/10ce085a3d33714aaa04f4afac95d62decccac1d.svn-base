
#ifndef __LOGONSVRAPP_HH__
#define __LOGONSVRAPP_HH__

#include "Application.h"
#include "libnode/node_client.h"

class LogonEvent;
using namespace std;
class CLogonSvrApp:public Application,public IWorkerEvent
{
public:
	CLogonSvrApp(void);
	virtual ~CLogonSvrApp(void);

	virtual bool onWorkerStart(Application* worker);
	virtual bool onWorkerStop(Application* worker);

private:
	void init_node();

private:
    LogonEvent		* m_pLogonEvent; 
	Application     * m_pApplication;
	INodeClient		* m_pNodeClient;

	uint32		m_uLanPort;
};

//#define appInstance SingleInstance<ExampleApplication>::Instance()
#endif //__LOGON_SERVER_APPLICATION_HH__

