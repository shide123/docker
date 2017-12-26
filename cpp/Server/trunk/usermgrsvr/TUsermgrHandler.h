#ifndef __USERMGR_THRIFT_HANDLER_H__
#define __USERMGR_THRIFT_HANDLER_H__

#include "TUserMsgSvr.h"

class TUsermgrHandler : public TUserMsgSvrIf
{
public:
	TUsermgrHandler();
	~TUsermgrHandler();

	int32_t proc_reloadUserInfo(const int32_t userid);
};

#endif /* __USERMGR_THRIFT_HANDLER_H__ */
