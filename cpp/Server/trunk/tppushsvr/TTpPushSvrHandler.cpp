#include "TTpPushSvrHandler.h"
#include "AppPush.h"

TTpPushSvrHandler::TTpPushSvrHandler(void)
{
}

TTpPushSvrHandler::~TTpPushSvrHandler(void)
{
}

void TTpPushSvrHandler::proc_notifyChatMsg(const TChatMsgPush& msg, const int32_t groupid, const bool notify_offline)
{
	Application::get_io_service().post(boost::bind(&ServerMsgMgr::procNotifyChatMsg,
		&(AppInstance()->m_ServerMsgMgr), msg, groupid, notify_offline));
}
