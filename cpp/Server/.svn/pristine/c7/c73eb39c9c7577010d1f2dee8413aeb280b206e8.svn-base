
#ifndef __SERVER_MSG_MGR_H__
#define __SERVER_MSG_MGR_H__

#include "Application.h"
#include "CMiPushProcessor.h"
#include "CBDPushProcessor.h"
#include "CMZPushProcessor.h"
#include "CHWPushProcessor.h"
#include "CApplePushProcessor.h"
#include "tppushsvr/TTpPushSvrIf_types.h"
#include "DBSink.h"

using namespace std;

class ServerMsgMgr:public IWorkerEvent
{
public:
    ServerMsgMgr(void);
	~ServerMsgMgr(void);
    bool Init(Config &cfg, const char* Main = "ServerMsgMgr");
	bool onConnect(clienthandler_ptr connection);
	bool onMessage(task_proc_data* message);
	bool onClose(clienthandler_ptr connection);
	bool onError(clienthandler_ptr connection,int code,string msg);

	bool procNotifyChatMsg(const TChatMsgPush& msg, const int32_t groupid, const bool notify_offline);
private:
	void getPushSet(CDBSink &sink, Result &res, int row, std::map<uint32, string> *pSet, int nSize, uint32 filterUser, std::set<uint32> &filterSet, bool notify_offline = true);
	void logPushSet(std::map<uint32, string> *pSet, int nSize, const char *preLog = "");
    CMiPushProcessor m_mipush_processor;	
    CBDPushProcessor m_bdpush_processor;
    CMZPushProcessor m_mzpush_processor;
    CHWPushProcessor m_hwpush_processor;
	CApplePushProcessor m_apush_processor;
};

#endif //__SERVER_MSG_MGR_H__

