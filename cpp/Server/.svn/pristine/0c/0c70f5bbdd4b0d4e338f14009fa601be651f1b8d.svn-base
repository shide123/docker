#ifndef __CHATLOGIC_H__
#define __CHATLOGIC_H__

#include "ProtocolsBase.h"
#include "ChatSvr.pb.h"
#include "CUserBasicInfo.h"
#include "KeywordMgr.h"
#include <list>
#include "redis_def.h"
#include "chatsvr/TChatSvrIf_types.h"

#define MAX_CHAT_PACKET_SIZE (32*1024)

using namespace std;

class CChatLogic
{
public:
	CChatLogic();
	~CChatLogic(void); 

	static void init(redisMgr* predisMgr);
	static void uninit();
	/*群无关的用户私聊功能暂未使用到，代码未完善，慎用 begin*/
	//私聊消息请求
	static void procPrivateMsgReq(task_proc_data &task_data);
	static bool dealPrivateMsgReq(CMDPrivateMsgReq &req);
	//私聊消息提示
	static void notifyPrivateMsg(const CMDPrivateMsgReq &req);
	//私聊消息收到反馈
	static void procPrivateMsgNotifyRecv(task_proc_data &task_data);
	//历史私聊消息请求
	static void procPrivateMsgHisReq(task_proc_data &task_data);
	//未读私聊消息请求
	static void procUnreadPrivateMsgReq(task_proc_data &task_data);
	/*群无关的用户私聊功能暂未使用到，代码未完善，慎用 end*/

	//获取Recommendid
	static long getRecommendid(const string &content);

	static void getPPTPics(const CMDGroupMsgHisReq &req,string &courseInfo);
	static bool getMediaId(const std::string &sInputContent,std::string &sVoiceMediaId,std::string &sPicMediaId);
	//群聊消息请求
	static void procGroupMsgReq(task_proc_data &task_data);
	static int dealGroupMsgReq(CMDGroupMsgReq &req, bool blobby = false, uint16 subcmd=0);
	//群聊消息提示 bOnlooker：围观消息，onlineOnly：只发给在线用户的消息
	static void notifyGroupMsg(const CMDGroupMsgReq &req, bool bOnlooker = false, bool onlineOnly = false, bool memberOnly = false,bool blobby = false);
	//群聊消息收到反馈
	static void procGroupMsgNotifyRecv(task_proc_data &task_data);
	//历史群聊消息请求
	static void procGroupMsgHisReq(task_proc_data &task_data);
	//未读群聊消息请求
	static void procUnreadGroupMsgReq(task_proc_data &task_data);

	//群组私聊消息请求
	static void procGroupPMsgReq(task_proc_data &task_data);
	static int dealGroupPMsgReq(CMDGroupPrivateMsgReq &req, CMDGroupPrivateMsgRecv *recv = NULL);
	//群组私聊消息提示 onlineOnly：只发给在线用户的消息
	static void notifyGroupPMsg(const CMDGroupPrivateMsgReq &req, bool onlineOnly = false);
	//群组私聊消息收到反馈
	static void procGroupPMsgNotifyRecv(task_proc_data &task_data);
	//群组私聊历史消息请求
	static void procGroupPMsgHisReq(task_proc_data &task_data);

	//用户群私聊资格查询
	static void procGroupPChatQualiticationReq(task_proc_data &task_data);

	//围观群众发言请求
	static void procGroupOnlookerChatReq(task_proc_data &task_data);

	//删除历史聊天消息（实际上只是不让用户查到历史）
	static void procDeleteChatMsgHis(task_proc_data &task_data);

	/****************RPC接口处理逻辑**************/
	//用户进群设置msgid
	static bool procUserJoinGroupNotify(uint32 groupid, uint32 userid, uint32 roletype);

	static void addVoiceInfo(std::string inputkey,CMDGroupMsgReq& req);
	static void delVoiceInfo(std::string inputkey);
	//获取语音文件信息
	static bool getVoiceInfo(std::string inputkey, CMDGroupMsgReq &req);

	static void addUnsuccReq(std::string inputkey, const CMDGroupMsgReq& req);
	static void delUnsuccReq(const CMDGroupMsgReq& req);
	static bool getUnsuccReq(std::string inputkey, CMDGroupMsgReq &req);

	static bool isMsgPushOnlineOnly(const ChatMsg_t &msg);

	static bool dealUnsuccessData();

	//语音的时候组装短信内容
	static void getMediaIdAndLength(const string &content, string &MediaId, string&length);
	static void setMediaIdAndLength(string &content, const string &MediaId, const string&length);
	static void processoffline(CMDGroupMsgReq &req);

	//ppt 图片操作
	static bool  procOptPPTPic( const std::vector<TPPTPicInfo>& vecPicId,const int32_t optType);
	static void   notifyPPTPicChangeMsg(const std::vector<TPPTPicInfo>& vecPicId,const int32_t optType);

	static bool updateMongoPPTpic(uint32 groupid,const std::vector<int32> &vMsgId,const string &sMaxPicId);

	static bool  procCommentAudit(const TChatAuditMsg &tTChatMsg) ;
	static int  notifyPHPMsgCenter(const CMDGroupMsgReq &req,const std::string &sCmd);
	static void notifyLiveUpdate(int iGroupID,int iTeacherUid);
private:

	static std::string genPushMsgContent(const ChatMsg_t &msg, bool inAlias = false);
	static void getUserInfo(UserInfo_t &user);
	static void getUserInfoWithMap(UserInfo_t &user,unsigned int liveid=0);
	static void getRoomUserStatus(uint32 groupid,uint32 userid,uint32& status);
	static std::map<uint32, stUserBasicInfo> m_mapUser;
public:
	static std::map<std::string ,CMDGroupMsgReq> m_VoiceCompleteInfo;
	static std::map<std::string,uint32> m_req_last;
	static boost::mutex m_voice_mutex;
	static boost::mutex m_unsucc_mutex;
	static CKeywordMgr m_keyworkmgr;
	static std::map<uint32,CMDGroupMsgReq> m_unSuccessReq;
	static int m_timerid;
	static redis_map_map m_rmRoomuserSetting;
	static redis_map_map m_rmGroupChatMsg;
	static redis_map_map m_rmUnSuccessGroupChatMsg;

	static redis_map_map m_rmCommentAudit;

	static int m_isCommentAudit;
	
};

#endif //__CHATLOGIC_H__
