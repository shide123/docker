
#ifndef __GATEWAY_APP_HH__
#define __GATEWAY_APP_HH__

#include "Application.h"
#include "GateBase.h"

class GatewayApp: public IWorkerEvent, public CGateBase
{
public:

	GatewayApp();

	virtual ~GatewayApp();

	virtual bool onWorkerStart(Application * worker);

	virtual bool onWorkerStop(Application * worker);

	virtual bool onConnect(clienthandler_ptr connection);

	virtual bool onMessage(task_proc_data * message);

	virtual bool onClose(clienthandler_ptr connection);

	virtual bool onError(clienthandler_ptr connection, int code, std::string msg);

	static GatewayApp * getInstance();

private:

	void loadSelfConfig();

	void onTimer(clienthandler_ptr connection);

};

#endif //__GATEWAY_APP_HH__

