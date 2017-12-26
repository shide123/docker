#include "CommonLogic.h"
#include "errcode.h"
#include "CUserGroupinfo.h"
#include "GroupBasicInfo.h"
#include "DBSink.h"
#include "redisMgr.h"

CCommonLogic::CCommonLogic()
{
}


CCommonLogic::~CCommonLogic()
{
}

int CCommonLogic::chkGroupPrivateChatQualitication(redisMgr &redismgr, uint32 groupid, uint32 userid, uint32 dstuid, uint32 *pThreshold /*= NULL*/, uint32 *pIntimacy /*= NULL*/)
{
	//是否被拉黑
	if (CDBSink().getUserFriendship(dstuid, userid) == 3)
		return ERR_CODE_GROUP_PCHAT_IN_USER_BLACKLIST;

	uint32 roleType = 0, toRoleType = 0;
	if (CUserGroupinfo::getUserRoleType(userid, groupid, roleType) < 0 || CUserGroupinfo::getUserRoleType(dstuid, groupid, toRoleType) < 0)
	{
		return ERR_CODE_FAILED;
	}

	bool inRoom = (e_VisitorRole != toRoleType);
	if (!inRoom)
	{
		std::string key = KEY_SET_ROOM_USERIDS":" + bitTostring(groupid);
		std::string member = bitTostring(dstuid);
		inRoom = (redismgr.getOne(0)->redis_sismember(key, member) > 0);
	}

	if (!inRoom)
		return ERR_CODE_USER_NOT_IN_GROUP;

	//管理员不受限制
	if (roleType >= e_AdminRole || toRoleType >= e_AdminRole)
	{
		return ERR_CODE_SUCCESS;
	}

	uint32 uPrivateChatSwitch = 0;
	if (!CGroupBasicInfo::getValue(groupid, CGroupBasicInfo::e_Field_Private_Chat_Switch, uPrivateChatSwitch))
		return ERR_CODE_FAILED_DB;
	
	if (0 == uPrivateChatSwitch)
	{
		return ERR_CODE_GROUP_PRIVATE_CHAT_SWITCH_OFF;
	}

	uint32 threshold = 0, intimacy = 0;
	if (!CDBSink().getGroupUserIntimacy(dstuid, userid, groupid, threshold, intimacy))
	{
		return ERR_CODE_FAILED_DB;
	}

	if (intimacy >= threshold)	//达到对方私聊门槛
	{
		return ERR_CODE_SUCCESS;
	}

	if (pThreshold) *pThreshold = threshold;	//对方私聊亲密度门槛
	if (pIntimacy) *pIntimacy = intimacy;		//当前与对方的亲密度

	return ERR_CODE_GROUP_PRIVATE_CHAT_LIMIT;
}
