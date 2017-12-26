#include "ChatRedisMgr.h"
#include "ChatServerApp.h"
#include "json/json.h"

using namespace std;

redis_map_map CGroupMsgReqInfo::m_GroupMsgReq_hashmap;

void CGroupMsgReqInfo::getGroupMsgReq(const std::string field,CMDGroupMsgReq& req)
{

}
void CGroupMsgReqInfo::setGroupMsgReq(const std::string field,const CMDGroupMsgReq& req)
{
	do
	{
		std::map<std::string, std::string> oFieldValueMap;
		oFieldValueMap["groupId"] = bitTostring(req.groupid());
		oFieldValueMap["srcUser"] = bitTostring(req.msg().srcuser().userid());
		oFieldValueMap["srcUser"] = bitTostring(req.msg().srcuser().userid());
		m_GroupMsgReq_hashmap.insert(field, oFieldValueMap);
	}while(0);
}
int CGroupMsgReqInfo::init(redisMgr * pRedisMgr)
{
	if (!pRedisMgr)
	{
		LOG_PRINT(log_error, "redisMgr is null.init failed.");
		return -1;
	}

	m_GroupMsgReq_hashmap.init(DB_ROOM, KEY_HASH_GROUPMSG_INFO, pRedisMgr);
	return 0;
}
CChatRedisMgr::CChatRedisMgr(void)
{
}

CChatRedisMgr::~CChatRedisMgr(void)
{
}

bool CChatRedisMgr::writePrivateChatMsg(CMDPrivateMsgReq &chatMsg)
{
	const ChatMsg_t &msg = chatMsg.msg();
	Json::Value jsonValue;
	jsonValue["msgid"] = Json::Value(msg.msgid());
	jsonValue["srcuid"] = Json::Value(msg.srcuser().userid());
	jsonValue["dstuid"] = Json::Value(msg.dstuser().userid());
	jsonValue["msgtime"] = Json::Value(msg.msgtime());
	jsonValue["msgtype"] = Json::Value(msg.msgtype());
	jsonValue["content"] = Json::Value(msg.content());

	std::string strMsgid = CChatCommon::getFormatString("%u", msg.msgid());
	Json::FastWriter writer;
	std::string jsonStr = writer.write(jsonValue);
	int ret = AppInstance()->m_pRedisMgr->getOne(DB_CHATRELA)->redis_zadd(CChatCommon::getPChatSeqName(chatMsg.msg().srcuser().userid(), chatMsg.msg().dstuser().userid()), strMsgid, jsonStr);
	return 0 == ret;
}

bool CChatRedisMgr::writeGroupChatMsg(CMDGroupMsgReq &chatMsg)
{
	const ChatMsg_t &msg = chatMsg.msg();
	Json::Value jsonValue;
	jsonValue["msgid"] = Json::Value(msg.msgid());
	jsonValue["msgtype"]  = Json::Value(msg.msgtype());
	jsonValue["timestamp"] = Json::Value(msg.msgtime());
	jsonValue["userid"] = Json::Value(msg.srcuser().userid());
	jsonValue["groupid"] = Json::Value(chatMsg.groupid());
	for (int i = 0; i < msg.atlist_size(); i++)
	{
		jsonValue["@list"][i] = Json::Value(msg.atlist(i));
	}
	jsonValue["content"] = Json::Value(msg.content());

	std::string strMsgid = CChatCommon::getFormatString("%u", msg.msgid());;
	Json::FastWriter writer;
	std::string jsonStr = writer.write(jsonValue);
	int ret = AppInstance()->m_pRedisMgr->getOne(DB_CHATRELA)->redis_zadd(CChatCommon::getGChatSeqName(chatMsg.groupid()), strMsgid, jsonStr);
	return 0 == ret;
}

bool CChatRedisMgr::updatePMsgId(uint32 userid, uint32 pchatUserid, uint32 msgid)
{
	std::string strKey = getUserMsgIdKey(userid);
	std::string strFiled = CChatCommon::getFormatString("pchat:%u", pchatUserid);
	std::string strMsgid = CChatCommon::getFormatString("%u", msgid);

	int ret = AppInstance()->m_pRedisMgr->getOne(DB_CHATRELA)->redis_hset(strKey.c_str(), strFiled.c_str(), strMsgid.c_str());
	return 0 == ret;
}

bool CChatRedisMgr::updateGMsgId(uint32 userid, uint32 groupid, uint32 msgid)
{
	std::string strKey = getUserMsgIdKey(userid);
	std::string strFiled = CChatCommon::getFormatString("gchat:%u", groupid);
	std::string strMsgid = CChatCommon::getFormatString("%u", msgid);

	int ret = AppInstance()->m_pRedisMgr->getOne(DB_CHATRELA)->redis_hset(strKey.c_str(), strFiled.c_str(), strMsgid.c_str());
	return 0 == ret;
}

bool CChatRedisMgr::modUserGroupMsgStat(uint32 userid, uint32 groupid, bool ready /*= true*/)
{
	std::string strKey = getUserMsgIdKey(userid);
	std::string strField = getUserGroupMsgStatField(groupid);
	std::string strValue = ready ? "1" : "0";

	int ret = AppInstance()->m_pRedisMgr->getOne(DB_CHATRELA)->redis_hset(strKey.c_str(), strField, strValue);

	return 0 == ret;
}

bool CChatRedisMgr::chkUserGroupMsgStat(uint32 userid, uint32 groupid)
{
	std::string strKey = getUserMsgIdKey(userid);
	std::string strField = getUserGroupMsgStatField(groupid);
	std::string strValue("0");
	int ret = AppInstance()->m_pRedisMgr->getOne(DB_CHATRELA)->redis_hget(strKey.c_str(), strField, strValue);

	return ret >= 0 && "1" == strValue;
}



bool CChatRedisMgr::getCommentAuditState(std::string &live_comment_statu,std::string &course_comment_status)
{
	string strKey=KEY_HASH_COMMENT_AUDIT_SWITCH;

	map<string,string> mCommentStatus;

	int ret = AppInstance()->m_pRedisMgr->getOne(DB_USER_MGR)->redis_hgetall(strKey.c_str(), mCommentStatus);
	if (ret==0 && mCommentStatus.size()>=2 )
	{		
		live_comment_statu=mCommentStatus["live_comment_status"];
		course_comment_status=mCommentStatus["course_comment_status"];

		return true;
	}

	return false;
}

bool CChatRedisMgr::getUserInfoByUid(int iUid,map<string,string> &mUserInfo)
{

	stringstream key;
	key<<KEY_HASH_USER_INFO":"<<iUid;

	int ret = AppInstance()->m_pRedisMgr->getOne(DB_USERBASIC_DB)->redis_hgetall(key.str().c_str(), mUserInfo);
	if (ret==0 )
	{		
		return true;
	}

	return false;
}
