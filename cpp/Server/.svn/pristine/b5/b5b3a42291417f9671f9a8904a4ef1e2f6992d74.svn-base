
#ifndef __WEBGATE_APP_HH__
#define __WEBGATE_APP_HH__

#include "Application.h"
#include "GateBase.h"

class CWebgateApp:public IWorkerEvent,public CGateBase
{
public:
	CWebgateApp();
	virtual ~CWebgateApp();

	virtual bool onWorkerStart(Application* worker);
	virtual bool onWorkerStop(Application* worker);
	virtual bool onConnect(clienthandler_ptr connection);
	virtual bool onMessage(task_proc_data* message);
	virtual bool onWrite(clienthandler_ptr connection, const char *pdata, unsigned int datalen, SL_ByteBuffer &outBuff);
	virtual bool onClose(clienthandler_ptr connection);
	virtual bool onError(clienthandler_ptr connection, int code, std::string msg);
	static CWebgateApp *getInstance();
	static bool ParseMessage(const char *pdata, int len, SL_ByteBuffer &out);
private:
	void loadSelfConfig();

	void onTimer(clienthandler_ptr connection);
};

#endif //__WEBGATE_APP_HH__

