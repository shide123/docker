#pragma once

#include <map>
#include <vector>
#include "yc_defines.h"
#include "message_vchat.h"
#include "UserObj.h"
#include "json/json.h"
#include "ProtocolsBase.h"
#include "RoomSvr.pb.h"
#include "comm_struct.h"
#include "CFieldName.h"
#include "timer.h"
#include "redis_map_set.h"
#define MAX_PUBMIC_COUNT 6
#define MAX_PUBLIC_USECOUNT 6  //3,2
#define MAX_ROOMOPUSER_COUNT 4
#define MAX_ROOMNOTICE_COUNT 4
#define MIC_TIME_PER	(300)
const int MAX_GUEST	 =	100;
enum eJoinGroupAuthType{
	NoNeedAuth = 0,
	RedPacket = 1,
	VideoAuth = 2,
	MsgAuth = 3,
	Forbidden = 4
};

//公麦状态
typedef struct tag_PubMicState
{
	tag_PubMicState():userid(0), mictimetype(0), userendtime(0), status(0), onmic(0){}
	uint32 userid;       //当前麦userid
	int32  mictimetype;  //麦时类型, -1不限麦时,0关闭,>0 麦时时间,-2 管理麦
	uint32 userendtime;  //用户下麦时间,0-不限时
	int status;//0:disable 1:enable
	int32 onmic;
}PubMicState_t;

typedef struct tag_WaitMicState
{
	tag_WaitMicState():userid(0), optime(0), status(0){}
	uint32 userid;       //排麦用户id
	uint32 optime;       //操作时间,用来做超时
	bool operator == (const tag_WaitMicState& r)
	{
		return userid == r.userid;
	}
	uint32 status;
}WaitMicState_t;

typedef struct tag_UserOnlineTimeState
{
	uint32  userid;
	uint32  onlinetime;
	int32   userroomlevel; //用户房间等级
}UserOnlineTimeState_t;

typedef struct tag_RobotUserInfo
{
   uint32 userid;      //用户id
   int    headiconid;  //

   byte   viplevel;
   byte   yiyuanlevel;
   byte   shoufulevel;
   byte   zhongshenlevel;

   byte   caifulevel;
   byte   lastmonthcostlevel;     //上月消费排行
   byte   thismonthcostlevel;     //本月消费排行
   byte   thismonthcostgrade;     //本月累计消费等级

   byte   gender;
   byte   age;
   byte   reserve1;
   char   szalias[17];   //显示呢称
}RobotUserInfo_t;

typedef struct tag_MicInfo
{
	tag_MicInfo(): micindex(-1), userid(0), mictype(0), status(0), onmic(0){}
	int32 micindex;
	int32 userid;
	int32 mictype;
	int32 status;
	int32 onmic;
}MicInfo;

//////////////////////////////////////////////////////////////////////////
class redisOpt;
class CRoomManager;

class CRoomObj :public boost::enable_shared_from_this<CRoomObj>
{
public:
	CRoomObj(CRoomManager * pMgr);
	CRoomObj(const vector<string> &fields, const vector<string> &values, CRoomManager * pMgr);
	virtual ~CRoomObj(void);

    string getRedisKey();
    string getRedisFields();
    void getRedisValues(Json::Value &jval, const string &fields);

    int getMicList(std::vector<PubMicState_t>& list);
	int getWaitMicUserCount();
	int checkWaitMicToPubMic(bool bcast,bool once = false);
	void checkPubMicToWaitMic(bool bcast,bool badd);
	int getUserWaitMicNum(uint32 userId);
	bool addWaitMicUser(uint32 userId,int nInserIndex,bool once = false);

    void setRedis(redisOpt *pRedis);
    void setSvrid(uint16_t svrid);
	void reset();
	void resetInfo(const stRoomFullInfo &info);
	bool delWaitMicUser(uint32 userId, int micId = -1);
	
	int eraseVisitUser(uint32 userID, bool downMic = true);
	int eraseVisitUser(UserObject_Ref & pUserObjRef, bool downMic = true);
	int InsertVisitUser(UserObject_Ref & pUserObjRef);
	bool findVisitUser(UserObject_Ref & userobjRef, uint32 userId);
	static bool findVisitUser(CRoomManager * pRoomMgr, redisOpt * pRedis, UserObject_Ref & userobjRef, uint32 userId);
	bool findVisitUser(uint32 userId);
	bool findMemberUser(UserObject_Ref & userobjRef, uint32 userId);
	bool findMemberUser(uint32 userId);
	int eraseMemberUser(UserObject_Ref & pUserRef);
	void loadBatchVisitor(vector<string> &userids, vector<UserObject_Ref> &vecUserRef);

	bool findUser(UserObject_Ref& userobjRef, uint32 userId);
	//bool getSoftbot(UserObject_Ref& userobjRef, uint8 nuserviplevel);
	void close();  //关闭房间
	bool isClosed();
	bool isNormalUser(uint32 userid, bool block = true);
	bool isZhuanboUser(uint32 userid);
	bool isRoomFangzhu(uint32 userid);   //是不是房主
	bool isRoomFuFangzhu(uint32 userid); //是不是副房主

	int HitTestFreePubMic(void);  //return 1-free, 0- no free
	int addPubMicUser(uint32 userId, int nSpecMicIndex, int bIsGuanliMic);   //兼容是否管理麦操作
	uint32 getUserIsOnMic();
	int setOnMicUser(uint32 userid);
	void delPubMicByUserId(uint32 userId);
	void downPublicMic(UserObject_Ref& userobjref);
	void forceUserDownMic(UserObject_Ref& userobjref);
	int clearPubMicByUserid(uint32 userId);
	int getUserPubMicType(uint32 userid);
	bool downMics(bool visitor = false);
	int getOnMicCount();

	void sendGroupVisitCount(task_proc_data * task_node);
	void sendMemberList(task_proc_data * task_node, uint32 userId,int64 gateObj,int64 gateObjId);
	void sendVisitorList(vector<UserObject_Ref> & vecUserRef);
	void setRoomUserInfo(CMDRoomUserInfo& info, UserObject_Ref &userobjRef);
	void sendPubMicState(task_proc_data *task_node,int64 gateObj,int64 gateObjId);
	void sendRoomNotice(task_proc_data *task_node,int64 gateObj,int64 gateObjId);
	void sendRoomNoticeInfoNotify(task_proc_data *task_node,int64 gateObj,int64 gateObjId);
	void sendJoinRoomResp(task_proc_data *task_node, UserObject_Ref &userobjRef,uint64 gateObj,uint64 gateObjId,int biscollectroom = 0);
	void sendAdKeywordsList(task_proc_data *task_node, int64 gateObj,int64 gateObjId);
    void sendTeacherUserid(task_proc_data *task_node, int64 gateObj,int64 gateObjId);
    void castSendTeacherUserid();
	void sendTeacherFansList(task_proc_data *task_node, int64 gateObj,int64 gateObjId);//发送当前讲师忠实排行榜
	void sendRoomAndSubRoomId(task_proc_data *task_node, int64 gateObj,int64 gateObjId);//IOS和Android需要得到主房间和子房间ID

	void castSendMsgXXX(COM_MSG_HEADER* pOutMsg, uint32 userId = 0, bool inGroupOnly = false);
	void castSendMsgXXX_i(COM_MSG_HEADER* pOutMsg, uint32 userId = 0, bool inGroupOnly = false);
	void castSendRoomMaxclientnum();

	void castSendRoomUserComeInfo(UserObject_Ref &userobjRef);
	void castSendRoomUserLeftInfo(UserObject_Ref &userobjRef);
	void castSendRoomUserExceptLeftInfo(UserObject_Ref &userobjRef);
	void castSendRoomKickoutUserInfo(uint32 srcId, UserObject_Ref &toUserobjRef, int reasionId, int minsId);
	void castSendRoomBaseInfo(void);
	void castSendRoomOPStatus(void);
	void castSendRoomMediaInfo(void);
	void castSendRoomAndSubRoomId();//IOS和Android需要得到主房间和子房间ID
	void castSendRoomWaitMicList(void);
	void castSendRoomVistorCount(void);
	void castSendUserPriorityChange(unsigned int userid, unsigned int roleType);
	void castSendOnMicState(uint32 userid,uint32 micstate,int32 micindex,int32 MicSec = -1);
	void castSendMicList();

	void OpenAllPubMic(void);
	void SetWaitMicMode(bool bVal);
	void SetChairManMode(bool bVal);
	bool changePubMicState(uint32 ruunerId, int micId, int optype, int param1);  //修改公麦状态
	bool lootUserMic(uint32 runnerId, uint32 userId, int micId);  //夺麦

	//设置用户mic状态
	int setUserMicState_0(UserObject_Ref &userobjRef, int newMicState, int nspecmicindex, int giftid, int giftnum, bool& bWaitMicListChanged);  //预设值,用于检查和自动转换状态
	int setUserMicState(UserObject_Ref &pUserRef, int newMicState, int nspecmicindex, int giftid, int giftnum, bool& bWaitMicListChanged);
	void setTeacherid(uint32 userid, int newMicState);
	void getTeacheridFromRobotid(int robotid);
	void updateActiveTime(bool bUpDb = true);
    
	//权限检查
	bool IsNoneedkey(uint32 userid,byte viplevel,byte roomlevel);  //进房间不要密码
	int  IsForbidChatUser(int userId);
	int updateUserForbidChat(uint32 userid);
	bool getAllVisitorid(vector<string> &userids);
	unsigned int getVisitorNum();
	void newRobotUser(AndroidParam_t &param);
	int loadRoomBroadInfo();
	int update_zhuanbo_robot_DB(uint32 vcbid, int micstate, UserObject_Ref micuser);
	int update_zhuanbo_robot_DB(uint32 vcbid, int micstate, uint32 micuserid);
	void UpdateVcbStatusDB();
	int redisSetRoomid();
	int redisInitRoomInfo();
	int redisSetRoomInfo(const char *pfields = NULL);
    void sendOnMicMediaMGR(int micstate, UserObject_Ref pToUserRef);

    uint32 getSoftbotCountPerViplevel(uint8 viplevel);
    void addoneSoftbotCountPerViplevel(uint8 viplevel);
    void suboneSoftbotCountPerViplevel(uint8 viplevel);
    uint32 getUserCountPerViplevel(uint8 viplevel);
    void addoneUserCountPerViplevel(uint8 viplevel);
    void suboneUserCountPerViplevel(uint8 viplevel);
	
	bool redisMsg_RoomMgrInfoUpdate(const string fields = "");

	bool delWaitMicUser_new(uint32 userId,int micId);
	int delAllWaitMicUser();
	bool changeWaitMicIndex_new(uint32 userId,int micId, int optype);
	int queryUserGroupInfo(unsigned int userid, CMDRoomUserInfo & oGroupInfo);
	void initGroupUserCount();
	void UpdateGroupUserCount(unsigned int userid, bool bAdd);
	//统计房间游客人数
	unsigned int UpdateVistorCount();

	bool kickAllLookers();

	int checkNextMic();

	bool isNowInGroup(uint32 userid);
	template<typename ListType>
	bool getInGroupList(std::vector<ListType> &vList, bool getVisitor = false);
	bool inGroup(uint32 userid);
	bool outGroup(uint32 userid);
	
	void sendOnlineMemberList(task_proc_data *task_node);
	void sendJoinGroupCondition(task_proc_data * message, uint32 userid);
	static std::string genJoinGroupCond(unsigned int authType, unsigned int groupid, int gender, unsigned int redpacketTotal, unsigned int redpacketNum);
	bool updateRedisInfo(std::string fields = "");

	int getForbidUserChat(int userid);

	int setForbidUserChat(int runuserid, int userid, int status);
	void setCourseFinish();

	static bool findOnlineUser(CRoomManager * pRoomMgr, redisOpt *pRedisOpt, uint32 userId, UserObject_Ref & userobjRef);

	bool isPPTRoom();
	string getRoomLiveUrl();
private:
	void init();
	int addVisitorUser(UserObject_Ref &userobjRef);    //增加新用户
	int delVisitUser(UserObject_Ref &userobjRef, bool downMic = true);
	//int UpdateVistorCount(UserObject_Ref & userobjRef, uint8_t useraction);
	static void loadVisitUserData(CRoomManager* pRoomMgr, redisOpt *pRedis, UserObject_Ref &userObjRef, uint32 userid);
	static void loadMemberUserData(UserObject_Ref & userObjRef, uint32 userid, uint32 groupid);
	void getGroupMemLst(std::list<CMDRoomUserInfo> & memberLst);
	void getRedisValuesMap(std::map<std::string, std::string> &jval, const string &fields);

public:
	int Build_NetMsg_JoinRoomRespPack(char* szBuf, int bufLen, UserObject_Ref &userobjRef, ClientGateMask_t** ppGateMask,int biscollectroom);
	int Build_NetMsg_RoomUserComePack(char* szBuf, int bufLen, UserObject_Ref &userobjRef, ClientGateMask_t** ppGateMask);
	int Build_NetMsg_RoomUserLeftPack(char* szBuf, int bufLen, UserObject_Ref &userobjRef, ClientGateMask_t** ppGateMask);
	int Build_NetMsg_RoomUserExceptLeftPack(char* szBuf, int bufLen, UserObject_Ref &userobjRef, ClientGateMask_t** ppGateMask);
	int Build_NetMsg_RoomKickoutUserPack(char * szBuf, int bufLen, uint32 srcId, UserObject_Ref &toUserobjRef, int reasionId, int minsId,ClientGateMask_t** ppGateMask);
	int Build_NetMsg_RoomBaseInfoPack(char* szBuf, int bufLen, ClientGateMask_t** ppGateMask);
	int Build_NetMsg_RoomOPStatusPack(char* szBuf, int bufLen, ClientGateMask_t** ppGateMask);
	int Build_NetMsg_RoomMediaPack(char* szBuf, int bufLen,ClientGateMask_t** ppGateMask);
	int Build_NetMsg_AdKeyworkListPack(char* pOutBuf, char* pInBuf, int inLen, ClientGateMask_t** ppGateMask);
    int Build_NetMsg_TeacheridPack(char* szBuf, int bufLen, ClientGateMask_t** ppGateMask);
	int Build_NetMsg_RoomNoticeNotyPack(char* szBuf, int bufLen, int noticeid, int userid, string alias, ClientGateMask_t** ppGateMask);
	int Build_NetMsg_WaitMicListPack(char* szBuf, int bufLen, ClientGateMask_t** ppGateMask);

public:
	uint32 nvcbid_;
	uint32 nuser_id;							//群主
	uint32 nlive_id;
	std::string class_name;
	std::string teacher_name;
	std::string teacher_info;
	string img;
	string src_img;
	uint32 nprice;
	uint32 nlevel;
	std::string strpwd;
	std::string invite_code;
	std::string strremark;
	std::string tags;
	std::string goal;
	std::string dest_user;
	std::string requirements;
	std::string nbegin_time;
	std::string nend_time;
	uint32 nstatus;
	uint32 nopenstatus;
	std::string npublish_time;
	std::string ncreate_time;
	std::string nupdate_time;
	uint32 talk_status;
	uint32 nopuser[MAX_GUEST];			//jiabing

//	uint32 nvcbid_;                                 //房间id
	std::string icon;
	std::string name;
	std::string remark;
	uint32 user_num;
	uint32 male_num;
	uint32 female_num;
	uint32 audience_num;
	uint32 virtual_num;
	uint32 form;//课程类型
	int32 status;
	int32 is_del;
	int32 show_square;
	uint32 capacity;								//房间设定总人数
	StRoomNotice_t m_roomnotice;
	uint32 showid;
	int32  iUnreadMsgNum;
	uint32 uAllowVisitGroup;
	uint32 gainPercent;
	uint32 gainUserRange;
	uint32 contribution_switch;
	uint32 voice_chat_switch;
	uint32 allow_visitor_on_mic;
	uint32 male_join_group_auth;
	uint32 male_red_packet_num;
	uint32 male_red_packet_total;
	uint32 female_join_group_auth;
	uint32 female_red_packet_num;
	uint32 female_red_packet_total;

    uint32 ncreatorid_;                             //建房间的人 
	uint32 nopuserid_[MAX_ROOMOPUSER_COUNT];        //4个副房主
	uint32 group_master;							//群主
	uint32 nlevel_;                                 //限制进入等级，和数据库对应相同:0-不限制,1-限制vip以上进入,2-限制管理以上进入
	uint32 nroomtype;								//房间类型,对应tbvcbinformation的type:0-普通房间,1-白金课程房间
	uint32 nplatformid_;							//服务公众号
	uint32 nopstate_;                               //房间操作状态
	uint32 nlastcheckA4time_;						//最后检查机器人加入时间
	uint32 nlastcheckUsertime_;                     //最后检查用户过期时间
	uint32 nlastcheckUserOnlinetime_;               //计算用户在线时间
	uint32 nlastcheckSendVCBStatustime_;            //计算状态发送时间
	uint32 nlastcheckTestShowUserStatusTime_;       //计算用户当前状态
	int32  nCurPubMicCount_;                        //当前公麦人数
	int32  nMaxPubMicCount_;                        //最大公麦人数，可变
	char   busepwd_;                                //是否使用了密码?
	char   reserve1_;                               //
	int16  nattribute_;                             //房间属性(类型)
	uint32 nMaxclientNum_;							//当天房间人气峰值
	uint32 nrobotcount_;
	uint32 nfilterusercount_;						//过滤用户人数(主要是公司内部用户，这些用户不用参与统计)
	uint32 nfilterguestcount_;						//过滤游客用户人数(主要是公司内部用户，这些用户不用参与统计)
	uint32 nsoftbotcount_;							//软件机器人数量
	map<uint8, uint32> map_softbotvipcount_;		//每个vip等级的软件机器人数量
	map<uint8, uint32> map_uservipcount_;			//每个vip等级的注册用户数量
	uint32 nipcount_;
	uint32 ntotalcount_;
	uint32 nguestcount_;
	uint32 nmaxwaitmicuser;                         //最多排麦人数
	uint32 nmaxuserwaitmic;                         //每人最多排麦次数
	uint32 nmroomid_;								//主房间id 如果没有则为0
	uint32 nteacherid_;								//讲师id
	uint32 nteamid_;								//战队id
	uint32 nrobotid_;								//当前在麦机器人id
	uint32 nuseridonmic_;							//当前在麦用户id
	uint64 npopularity_;							//房间访问人气(人次)
    volatile int ncloseflag_;                       //房间是否要关闭的标志,1-关闭
    int nchairmanmod;								//chairman mode 1-yes
    PubMicState_t pubmicstate_[MAX_PUBMIC_COUNT];   //公麦状态
    MicInfo m_vMicInfo[MAX_PUBMIC_COUNT];
	std::string strmedia_;                          //媒体服务器信息
	std::string roomtheme_;
	std::string strpwd_;							//密码
	std::string areaid;

	StRoomNotice_t m_RoomNoticeArray[MAX_ROOMNOTICE_COUNT];	//公告信息
	std::map<uint32, UserObject_Ref >  forbidchatusers_;	//禁言用户列表

	std::vector<WaitMicState_t >       vecWaitMics_;      //排麦列表
	uint64 m_tLastActive;			//最后一次下麦时间
	uint64 m_tLastNonLiveNotify;	//定时器最后一次下发非直播时段提示的时间
	CRoomManager * m_pRoomMgr;
	redisOpt * m_pRedis;
	uint16_t m_svrid;
	e_MicMode m_micMode;
	int timerid;
	int m_micBeginTime;

	redis_map_map m_rmGroupInfo;
	redis_map_set m_rsMemberInGroup;
	redis_map_set m_rsVisitorInGroup;
	redis_map_map m_rmRoomuserSetting;
public:
	std::vector<WaitMicState_t > getWaitMics();
	PubMicState_t* getPubMicState();
	void updateMicState();
	void updateWaitUsers();
	void updateFields(string& key, string& fields);
	int setMicposStatus(int index ,int status);
	int setMicMode(e_MicMode mode);
	e_MicMode getMicMode();
	int getCurPubMicCount();
	void updateCurPubMicCount();
	int getMaxPubMicCount();
	void updateMaxPubMicCount();
	void updateMicMode();
	void updateTimerid();
	void OnTimer();
	void clearTimerid();
	void addTimerid();
	void changePos(int pos);
	void inviteonmic(CMDInviteOnMic& pReq);
	int getInterval();
};

typedef boost::shared_ptr<CRoomObj> RoomObject_Ref;

template<typename ListType>
bool CRoomObj::getInGroupList(std::vector<ListType> &vList, bool getVisitor/* = false*/)
{
	if (getVisitor)
		return m_rsVisitorInGroup.find(nvcbid_, vList);
	else
		return m_rsMemberInGroup.find(nvcbid_, vList);
}

