/*
 *  Description: This is the core for rommsvr, above the full flow for logicCore.h
 */
#pragma once 

#include "RoomObj.h"
#include "curl.h"
#include "redisOpt.h"
#include "redisdataHelper.h"
#include "ProtocolsBase.h"
#include "clienthandler.h"
#include "TRoomSvrIf_types.h"
#include "TChatSvrIf_types.h"
#include "CommonLogic.h"
#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <cryptopp/modes.h>
#include "CommonLogic.h"
#include <algorithm>
#include "utils.h"
typedef struct UserTimerid_tag
{
	UserTimerid_tag()
	{
		userid = 0;
		timerid = -1;
		ts = 0;
	}
	uint32 userid;
	uint32 toid;
	uint32 vcbid;
	int timerid;
	unsigned long ts;
}UserTimerid;
typedef struct tag_ExceptExitRecord{
	uint32 vcbid;
	uint32 userid;
	uint16 ngateid;
	time_t time;
	char *pdata;
	int len;

	tag_ExceptExitRecord(){
		vcbid = userid = ngateid = len = time = 0;
		pdata = NULL;
	}
	~tag_ExceptExitRecord(){
		if (pdata) {
			free(pdata);
			pdata = NULL;
		}
	}

	tag_ExceptExitRecord& operator= (const tag_ExceptExitRecord& r) {
		vcbid = r.vcbid;
		userid = r.userid;
		ngateid = r.ngateid;
		time = r.time;
		len = r.len;
		pdata = (char*)malloc(len);
		memcpy(pdata, r.pdata, r.len);
		return *this;
	}

	bool operator==(const tag_ExceptExitRecord &other) const {
		if (userid == other.userid && vcbid == other.vcbid)
			return true;
		return false;
	}
}ExceptExitRecord_t;

class ServerEvent;
class CLogicCore
{
public:
	CLogicCore(void);
	~CLogicCore(void);

	void setEvent(ServerEvent* event);
	void setSvrId(uint16_t svrid) { m_svrid = svrid; }
	int onMessage(task_proc_data * message);
	void onTimer();

public:
	//排麦
	int proc_upwaitmic_req(task_proc_data *task_node);
	void resperrinf(COM_MSG_HEADER* reqHead, ClientGateMask_t* pMask, uint16 errcode, task_proc_data *pTaskNode);
	//加入房间
	int proc_joinroom_req(task_proc_data *task_node);
	//房间后处理
	int proc_afterjoinroom_req(task_proc_data *task_node);
	//rejoin房间
	int proc_gatejoinroom_req(task_proc_data *task_node);
	//退出房间
	int proc_exitroom_req(task_proc_data *task_node);
	//异常退出房间
	int proc_except_exitroom_req(task_proc_data *task_node);
	int handle_except_exitroom_delay(ExceptExitRecord_t *pexitRecord);
	//踢人(踢游客和用户)
	int proc_kickoutuser_req(task_proc_data * task_node);
	int proc_kickoutuser_req(unsigned int src_userid, unsigned int to_userid, unsigned int groupid, unsigned int reasonid, unsigned int kickout_min);
	//gateway超时踢人(只能踢游客)
	int proc_gatekickoutuser_req(task_proc_data * task_node);
	int proc_gatekickoutuser_req(unsigned int to_userid, unsigned int groupid, unsigned int reasonid);
	int proc_client_reportmediagateip_req(task_proc_data *task_node);
	int proc_setmicstate_req(task_proc_data *task_node);
	int proc_micstatelist_req(task_proc_data *task_node);
	int proc_setmicmode_req(task_proc_data* task_node);
	int proc_setmicposStatus_req(task_proc_data* message);
	int proc_getroominfo_ext_req(task_proc_data* message);
	//int proc_upwaitmic_req(task_proc_data *task_node);
	//int proc_self_addandroid_req(task_proc_data *task_node);
	int proc_setdevstate_req(task_proc_data *task_node);
	int proc_setroomopstatus_req(task_proc_data *task_node );
	int proc_setroommedia_req(task_proc_data *task_node );
	int proc_softbot_joinroom_req(task_proc_data *task_node);
	int proc_softbot_exitroom_req(task_proc_data *task_node);
	int proc_changewaitmicindex_req(task_proc_data *task_node);
	int proc_inviteonmic_req(task_proc_data* message);
	int proc_setroominfo_req(task_proc_data *task_node);
	int proc_groupmember_req(task_proc_data * message);
	int proc_queryUserGroupInfo_req(task_proc_data * message);
	int proc_setGroupMsgMute_req(task_proc_data * message);
	int proc_joingroup_req(task_proc_data * message);
	int proc_joingroup_req(unsigned int userid, unsigned int groupid, unsigned int iRoleType, bool checkInGroup, int inGroupWay = 0);
	int proc_quitgroup_req(task_proc_data * message);
	int proc_setUserPriority_req(task_proc_data * message);
	int proc_setuserpriority_req(unsigned int run_userid, unsigned int to_userid, unsigned int groupid, unsigned int roleType, bool checkRunID = true);
	void proc_getGroupVisitorLst(unsigned int groupid, std::vector<TUserGroupInfo> & oVisitorLst);
	int proc_qryGroupVisitCount_req(task_proc_data * message);
	int proc_qryJoinGroupConditionReq(task_proc_data * message);
	int proc_kickAllOnlookers(uint32 runid, uint32 groupid);
	void proc_GroupSettingStatReq(task_proc_data * message);
	int proc_applyJoinGroupAuthReq(task_proc_data * message);
	int proc_handleJoinGroupAuthReq(task_proc_data * message);
	bool proc_setGroupOwner(unsigned int groupid, unsigned int ownerid);
	void proc_handleGroupUserSettingReq(task_proc_data * message);
	int proc_UserAuthReq(task_proc_data* message);
	int queryInGroupList(std::map< ::e_INGROUP_TYPE::type, std::vector<int32_t> > & _return, const uint32_t userid, const uint32_t groupid);
	void procForbidUserChat(task_proc_data * message);
	void procCourseFinish(task_proc_data * message);
	int procGiveTip(const int32_t srcid, const int32_t toid, const int32_t groupid, const std::string& gifname, const int32_t tiptype,
		const std::string& srcname, const std::string& toname, const std::string& gifpicture);
	int procSendLink(const int32_t groupid, const std::string& title, const std::string& source, const std::string& summary, const int32_t bTip,
		const int32_t amount, const int32_t pointId, const std::string& head_add, const int32_t recommendid, const int32_t user_id);
	int procNotice(const string& notice);
	int procSendCourse(const int32_t groupid, const std::string& title, const std::string& source, const std::string& summary, const int32_t bTip,
		const int32_t amount, const int32_t courseId, const std::string& head_add, const std::string& teachername, const std::string& srcimg, const int32_t recommendid, const int32_t user_id);

	int32_t proc_CloseLiveAndCourse(const int32_t runuserid, const int32_t liveid, const int32_t status, const int32_t type);

	int32_t proc_liveStart(const std::string& streamID);
	int32_t proc_liveStop(const std::string& streamID);

	int32_t proc_sendNewSubCourseNotice(const int32_t courseId, const int32_t subCourseId, const int32_t userId, const std::string& courseName, const std::string& subCourseName, const std::string& beginTime);
	int32_t proc_sendNewCourseNotice(int32_t courseId);

	int  proc_reloadRoomVirtualNumInfo(const int32_t liveid, const int32_t numbers);
	bool cast_SendChat_rpc(uint32 groupid, const string& content, e_MsgType msgType, uint32 userid=0);
private:
	void sendInfoAfterJoinRoom(task_proc_data * message, RoomObject_Ref pRoomObjRef, uint32 userid, bool isMember);
	void proc_handleAuthState(uint32 userid, uint32 groupid, uint32 authState);
	void addSettingStat(CMDGroupSettingStatNotify &notify, e_SettingType type, int stat);
	int hello_response(task_proc_data * task_node);
	void register_cmd_msg(clienthandler_ptr client);
	bool proc_joingroup_rpc(unsigned int userid, unsigned int groupid, unsigned int iRoleType = e_MemberRole);
	int add_group_assistmsg_rpc(unsigned int userid, unsigned int groupid, unsigned int iRoleType, bool bLevelUp, bool bNotyAdmin = true, bool bNotyUser = true, bool bQuitGroup = false);
	int add_group_assistmsg_rpc(const TGroupAssistMsg & assistMsg, bool bNotyAdmin, bool bNotyUser);
	void proc_clean_visitor_user(unsigned int userid, unsigned int groupid);
	bool userAllowJoinGroup(unsigned int userid, RoomObject_Ref & roomObjRef, unsigned int & errCode);
	bool castJoinGoupMsg_rpc(uint32 groupid, uint32 userid);
	bool castRoleChange_rpc(uint32 groupid, uint32 userid, uint32 oldRoleType, uint32 newRoleType);

	//join room handle function(jr = joinroom)
	int jr_check_crc32(task_proc_data *task_node);
	bool jr_room_isallow(task_proc_data *task_node, RoomObject_Ref &roomObjRef);
	bool jr_is_roomowner(task_proc_data *task_node, RoomObject_Ref &roomObjRef);
	bool jr_user_in_blacklist(task_proc_data *task_node);
	bool jr_user_in_kickoutlist(task_proc_data *task_node);
	bool jr_room_isfull(task_proc_data *task_node, RoomObject_Ref &roomObjRef, UserObject_Ref &NewUserObjRef);
	int jr_check_room_passwd(task_proc_data * task_node, RoomObject_Ref & roomObjRef, UserObject_Ref &NewUserObjRef, bool checkroompwd);
	int jr_check_user_visit_exist(task_proc_data *task_node, RoomObject_Ref &pRoomObjRef, UserObject_Ref &pUserObjRef, UserObject_Ref &NewUserObjRef, bool &bsame_user_and_room);
	int jr_check_user_exist(task_proc_data *task_node, RoomObject_Ref &pRoomObjRef, UserObject_Ref &pUserObjRef, UserObject_Ref &NewUserObjRef, bool &bsame_user_and_room);
	void jr_send_resp_err(task_proc_data *task_node, int errid);
	void jr_reset_user_info(task_proc_data *task_node, RoomObject_Ref &pRoomObjRef, UserObject_Ref &pUserObjRef, string &areaid);
	clienthandler_ptr findconn(uint32 connid);
	void initKV(const char* key);
	string encrypt(string plainText);
	string decrypt(string cipherTextHex);
	string getAppIdKey(string appid);
	void genAllToken(task_proc_data * task_node);
	void queryAndUpdateLiveFlowStatus();
	void pushCourseStartNotice(void);
	void pushNewCourseNoticeInBatch(void);
	void notifyPhpToPushMessages(void);

protected:
	map<uint32, clienthandler_ptr> m_mapConn;
	ServerEvent* m_pSvrEvent;
	uint16_t m_gateid;
	uint16_t m_svrid;
	byte m_key[ CryptoPP::AES::DEFAULT_KEYLENGTH ];
	byte m_iv[ CryptoPP::AES::BLOCKSIZE];
	std::map<string,string> m_AppId2Key;
	std::vector<std::string> m_zombies;
	std::map<int32_t,int32_t> m_groupid2timerid;
};


