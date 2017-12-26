#ifndef __TPPUSHSVR_HANDLER_H__
#define __TPPUSHSVR_HANDLER_H__
#include "tppushsvr/TpPushSvr.h"

class TTpPushSvrHandler :
	public TpPushSvrIf
{
public:
	TTpPushSvrHandler(void);
	~TTpPushSvrHandler(void);
	virtual void proc_notifyChatMsg(const TChatMsgPush& msg, const int32_t groupid, const bool notify_offline);
};


class TpPushSvrAsyncHandler : public TpPushSvrCobSvIf {
public:
	TpPushSvrAsyncHandler() {
		syncHandler_ = std::auto_ptr<TTpPushSvrHandler>(new TTpPushSvrHandler);
		// Your initialization goes here
	}
	virtual ~TpPushSvrAsyncHandler();

	void proc_notifyChatMsg(tcxx::function<void()> cob, const TChatMsgPush& msg, const int32_t groupid, const bool notify_offline) {
		syncHandler_->proc_notifyChatMsg(msg, groupid, notify_offline);
		return cob();
	}

protected:
	std::auto_ptr<TTpPushSvrHandler> syncHandler_;
};

#endif //__TPPUSHSVR_HANDLER_H__
