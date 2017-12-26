#ifndef __COMMON_LOGIC_H__
#define __COMMON_LOGIC_H__

#include "yc_datatypes.h"

class redisMgr;
class CCommonLogic
{
public:
	CCommonLogic();
	virtual ~CCommonLogic();


	static int chkGroupPrivateChatQualitication(redisMgr &redismgr, uint32 groupid, uint32 userid, uint32 dstuid, uint32 *pThreshold  = 0, uint32 *pIntimacy = 0);

};

#endif //__COMMON_LOGIC_H__
