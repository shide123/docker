#pragma once

#include "ChatCommon.h"
#include "ChatSvr.pb.h"
#include "redisMgr.h"
#include "redis_map_map.h"
class CChatRedisMgr
{
public:
	CChatRedisMgr(void);
	virtual ~CChatRedisMgr(void);

public:
	static bool writePrivateChatMsg(CMDPrivateMsgReq &chatMsg);
	static bool writeGroupChatMsg(CMDGroupMsgReq &chatMsg);

	static bool updatePMsgId(uint32 userid, uint32 pchatUserid, uint32 msgid);
	static bool updateGMsgId(uint32 userid, uint32 groupid, uint32 msgid);

	static bool modUserGroupMsgStat(uint32 userid, uint32 groupid, bool ready = true);
	static bool chkUserGroupMsgStat(uint32 userid, uint32 groupid);

	static bool getCommentAuditState(std::string &live_comment_statu,std::string &course_comment_status);

	static	bool getUserInfoByUid(int iUid,map<string,string> &mUserInfo);
private:
	inline static std::string getUserMsgIdKey(uint32 userid)
	{
		return CChatCommon::getFormatString("user_msg:%u", userid);
	}
	inline static std::string getUserGroupMsgStatField(uint32 groupid)
	{
		return CChatCommon::getFormatString("group:%u", groupid);
	}
	inline static std::string getUserGroupPMsgStatField(uint32 groupid, uint32 dstuid)
	{
		return CChatCommon::getFormatString("group%u:%u", groupid, dstuid);
	}

};

class CGroupMsgReqInfo
{
public:
	CGroupMsgReqInfo(){}
	~CGroupMsgReqInfo(){}
	static int init(redisMgr * pRedisMgr);
	static void getGroupMsgReq(const std::string field,CMDGroupMsgReq& req);
	static void setGroupMsgReq(const std::string field,const CMDGroupMsgReq& req);
private:
	static redis_map_map m_GroupMsgReq_hashmap;

};
