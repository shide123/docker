#ifndef __MONGODB_MANAGER_H__
#define __MONGODB_MANAGER_H__

#include <string>
#include <list>
#include "yc_datatypes.h"
#include "DBmongoConnection.h"
#include "comm_struct.h"
#include "ChatCommon.h"
#include "ChatSvr.pb.h"
#include "CLogThread.h"
#include "message_vchat.h"

//mongodb db name
#define MONGO_SEQUENCES_DB			"common"
#define MONGO_PRIVATE_CHAT_MSG_DB	"private_chat_msg"
#define MONGO_PRIVATE_CHAT_STAT_DB	"private_chat_stat"
#define MONGO_GROUP_CHAT_MSG_DB		"group_chat_msg"
#define MONGO_GROUP_CHAT_STAT_DB	"group_chat_stat"
#define MONGO_GROUP_PCHAT_MSG_DB	"group_pchat_msg"
#define MONGO_GROUP_PCHAT_STAT_DB	"group_pchat_stat"
#define MONGO_GROUP_ASSIST_DB		"group_assist_msg"
#define MONGO_GROUP_ASSIST_STAT_DB	"group_assist_stat"

#define MONGO_GROUP_CHAT_PPT_RELATION_DB		"group_chat_ppt_relation"
//mongodb field name
#define MONGO_KEY				"_id"
#define MONGO_CHAT_GROUP		"group"
#define MONGO_CHAT_SRC_USERID	"srcuid"
#define MONGO_CHAT_DST_USERID	"dstuid"
#define MONGO_CHAT_MSG_TIME		"msgtime"
#define MONGO_CHAT_MSG_TYPE		"msgtype"
#define MONGO_CHAT_CONTENT		"content"
#define MONGO_CHAT_CLIENT_MSGID "client_msgid"
#define  MONGO_CHAT_MASTER_MSGID "master_msgid"
#define MONGO_CHAT_READ_MSGID	"read_msgid"
#define MONGO_CHAT_BEG_MSGID	"beg_msgid"
#define MONGO_CHAT_CHAT_TIME	"chattime"
#define MONGO_CHAT_AT_LIST		"@list"
#define MONGO_CHAT_RECALL		"recall"
#define MONGO_CHAT_MEDIA_LENGTH		"media_length"
#define MONGO_CHAT_EXTEND_TYPE		"extendtype"

#define MONGO_ASSIST_USERID		"uid"
#define MONGO_ASSIST_GROUPID	"groupid"
#define MONGO_ASSIST_SVRTYPE	"msg_svrtype"
#define MONGO_ASSIST_SVRSWITCH	"msg_svrswitch"
#define MONGO_ASSIST_SVRLEVELUP	"msg_svrlevelup"
#define MONGO_ASSIST_SVRLEVEL	"msg_svrlevel"
#define MONGO_ASSIST_STATE		"state"
#define MONGO_ASSIST_MSGTIME	"msg_time"
#define MONGO_ASSIST_URL		"url"
#define MONGO_ASSIST_MSGTYPE	"msg_type"
#define MONGO_ASSIST_RELATE_MSGID	"relate_msgid"
#define MONGO_ASSIST_USERCONTEXT	"user_context"
#define MONGO_ASSIST_AUTHID		"authid"
#define MONGO_ASSIST_INVEST_USERID "invest_userid"
#define MONGO_ASSIST_EXTRA_INFO	"extra"
#define MONGO_ASSIST_EXTRA_PACKETID	"packetid"
#define MONGO_ASSIST_EXTRA_BALANCE	"balance"


#define  MONGO_GROUP_CHAT_PPT_RELATION_VOICE_LIST	"voicelist"
class CChatMongoMgr
{
public:
	CChatMongoMgr(void);
	virtual ~CChatMongoMgr(void);

public:
	//获取指定序列值
	static bool getNextSequence(std::string strSeqName, uint32 &value, uint32 inc = 1);
	//获取当前序列值
	static bool getCurSequence(const std::string & strSeqName, uint32 &value);
	//写入私聊消息
	static bool writePrivateMsg(const CMDPrivateMsgReq &req);
	//查询私聊消息
	static bool qryPrivateMsg(uint32 srcuid, uint32 dstuid, uint32 msgid, ChatMsg_t &msg);
	//查询私聊消息列表
	static size_t qryPrivateMsgList(uint32 requid, uint32 dstuid, std::list<ChatMsg_t> &vMsg,
		uint32 msgid = 0, int32 count = 0, bool forward = true);
	//解析私聊消息
	static bool parsePrivateMsg(const mongo::BSONObj &bsonMsg, ChatMsg_t &msg);
	//更新用户私聊消息msgid
	static bool updPrivateMsgId(uint32 userid, uint32 srcuid, uint32 msgid);
	//查询用户所有私聊用户及最新msgid
	static bool qryUserPrivateMsgIdList(uint32 userid, std::map<uint32, uint32> &mMsgId);
	//查询未读私聊消息
	static size_t qryUnreadPrivateMsg(uint32 srcuid, uint32 dstuid, std::list<ChatMsg_t> &lstMsg, uint32 msgid /*= 0*/, int32 count /*= 0*/);
	//回收私聊消息 nodefined
	static bool recallGroupPrivateMsg(uint32 srcuid, uint32 dstuid, uint32 msgid);

	//写入群聊消息
	static bool writeGroupMsg(const CMDGroupMsgReq &req,bool blobby = false);
	//查询群聊消息
	static bool qryGroupMsg(uint32 groupid, uint32 msgid, ChatMsg_t &msg, bool blobby = false);
	static bool qryGroupPMsgCount(uint32 groupid, uint32& count);
	static bool queryCurrentDayGroupMsgCount(uint32 groupid, uint32& count);
	//查询群聊消息列表
	static size_t qryGroupMsgList(uint32 userid, uint32 groupid, std::list<ChatMsg_t> &lstMsg,
		uint32 msgid = 0, int32 count = 0, bool forward = true,bool blobby = false);
	static size_t qryGroupMsgList_V2(uint32 userid, uint32 groupid, std::list<ChatMsg_t> &lstMsg,int64 querytime,
		uint32 msgid = 0, int32 count = 0, bool forward = true,bool blobby = false);

	static size_t qryUnreadGroupMsg(uint32 groupid, uint32 userid, std::list<ChatMsg_t> &lstMsg, int32 count = 0);
	//解析群聊消息
	static bool parseGroupMsg(const mongo::BSONObj &bsonMsg, ChatMsg_t &msg, const uint32 optionalUserId = 0);
	//更新用户群聊消息msgid
	static bool updUserGroupMsgId(uint32 userid, uint32 groupid, uint32 msgid);
	static bool qryUserGroupMsgId(uint32 userid, uint32 groupid, uint32 &msgid, const char *field = MONGO_CHAT_READ_MSGID, uint32 dstuid = 0);
	static bool updUserGroupBeginMsgId(uint32 userid, uint32 groupid, uint32 msgid);
	//更新用户最后的聊天时间
	static bool updUserGroupChatTime(uint32 userid, uint32 groupid, uint32 chattime);
	//获取群聊最大的msgid
	static bool qryMaxGroupMsgId(uint32 groupid, uint32 &msgid);
	//回收群消息
	static bool recallGroupMsg(uint32 groupid, uint32 msgid,bool blobby = false);

	//写入群组私聊消息
	static bool writeGroupPrivateMsg(const CMDGroupPrivateMsgReq &req);
	//查询群组私聊消息
	static bool qryGroupPrivateMsg(uint32 groupid, uint32 srcuid, uint32 dstuid, uint32 msgid, ChatMsg_t &msg);
	//查询群组私聊消息列表
	static size_t qryGroupPrivateMsgList(uint32 groupid, uint32 srcuid, uint32 dstuid, std::list<ChatMsg_t> &lstMsg,
		uint32 msgid = 0, int32 count = 0, bool forward = true);
	//查询未读群组私聊消息
	static size_t qryUnreadGroupPrivateMsg(uint32 groupid, uint32 srcuid, uint32 dstuid, std::list<ChatMsg_t> &lstMsg, uint32 msgid /*= 0*/, int32 count /*= 0*/);
	//更新用户群组私聊消息msgid
	static bool updUserGroupPMsgId(uint32 userid, uint32 groupid, uint32 dstuid, uint32 msgid);
	//查询用户指定群所有私聊用户及最新msgid
	static bool qryUserGroupPMsgIdList(uint32 userid, uint32 groupid, std::map<uint32, uint32> &mMsgId);
	//回收群私聊消息
	static bool recallGroupPrivateMsg(uint32 groupid, uint32 srcuid, uint32 dstuid, uint32 msgid);

	//删除聊天消息
	static bool delUserMsgHistory(uint32 groupid, uint32 userid, uint32 dstuid = 0);

	//增加群助手信息
	static bool addAssistMsg2MongoDB(StGPAssistMsg_t & oMsg, unsigned int & msgID);
	//给人员增加群助手信息
	static bool addAssistMsg2User(unsigned int msgID, unsigned int userID);
	//更新该用户的已读群助手信息msgid
	static bool updateUserAssistReadMsgId(unsigned int userID, unsigned int msgID);
	//更新用户助手信息的状态
	static bool updateAssistMsgState(unsigned int msgID, e_MsgState state);
	//查询群助手消息的状态
	static void qryAssistMsgState(const std::list<unsigned int > & msgIDLst, std::list<MsgState_t > & msgStateLst);
	//查询用户的未读群助手信息
	static size_t qryUnreadAssistMsgLst(unsigned int userID, std::list<StGPAssistMsg_t > & unreadMsgLst, int32 count = 0);
	//查询群助手消息列表
	static size_t qryAssistMsgList(uint32 userID, std::list<StGPAssistMsg_t > & lstMsg, uint32 msgID = 0, int32 count = 0, bool forward = true);
	//获取群助手消息信息
	static bool qryAssistMsgInfo(uint32 msgID, StGPAssistMsg_t & oMsg);
	static bool updateMsgContent(unsigned int msgid,unsigned int groupid,std::string content);

	static bool updateMsgExtendType(unsigned int msgid,unsigned int groupid,int  extendtype);

	static bool getgroupmsg_t(int groupid,std::list<ChatMsg_t> &lstMsg);

	//PPT课程
	static bool  writePPTrelationMsg(const CMDGroupMsgReq &req,const std::string &sPicMediaId);
	static bool  updatePPTrelationMsg(const uint32 &groupId,const std::string &sPicMediaId,const std::vector<int32> &vAddMsgId);
	static bool queryPPTrelationMsg(const uint32 &groupId,const std::string &sPicMediaId,std::vector<int32> &vMsgId);
	static bool deletePPTrelationMsg(const uint32 &groupId,const std::string &sPicMediaId);
private:
	//校验msgID的有效性
	static bool checkMsgIDValid(const std::string & strSeqName, uint32 msgID);
	//查询用户已读消息的msgid
	static bool qryUserReadAssistMsgId(uint32 userID, uint32 & msgid);
	//将mongodb里的BSON转成助手消息结构体
	static bool parseAssistMsg(const mongo::BSONObj & bsonMsg, StGPAssistMsg_t & msg);
	//查询用户消息msgid list
	static size_t qryUserAssistMsgIdLst(uint32 userID, uint32 msgid, std::list<uint32> & msgIdLst, int32 count, bool forward);

	void processPictAndVoiceOffLine(const std::string filename);
};

#endif //__MONGODB_MANAGER_H__
