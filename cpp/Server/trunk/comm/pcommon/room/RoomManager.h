#pragma once
//保存公用的变量,如大礼物列表,城主列表

#include <map>
#include <list>
#include "yc_defines.h"
#include "message_vchat.h"
#include "cmd_vchat.h"
#include "comm_struct.h"
#include "RoomObj.h"
#include "redis_map.h"
#include "redis_map_set.h"
#include "Config.h"
#include "clienthandler.h"
#include "macro_define.h"
#include "DBmongoConnection.h"
#include "DBSink.h"
//////////////////////////////////////////////////////////////////////////

typedef struct tag_CMDThrowUserInfo_Ext
{
	uint32 vcbid;
	uint32 runnerid;
	uint32 toid;
	byte   viplevel;       //(roomsvr填写)
	byte   nscopeid;       //封杀范围 :1-房间,2-全站
	byte   ntimeid;        //封杀时长
	byte   nreasionid;     //封杀理由
	char   szip[IPADDRLEN];
	char   uuid[64];
}CMDThrowUserInfoExt_t;

typedef struct tag_LevelWaitMicSpeciality
{
   int levelid;
   int maxwaitmicnum;
   int keepwaitmic;
}LevelWaitMicSpeciality_t;

typedef struct tag_RoomZhuanboRobotConfig
{
	int nid;				//记录号
	uint32 vcbid;			//主房间号
	int pubmicindex;		//麦序号
	uint32 robotid;			//子房间里的机器人id
	int robotviplevel;		
	uint32 robotvcbid;		//子房间号
	int robotstatus;		
	uint32 userid;
}RoomZhuanboRobotConfig_t;

typedef struct TipGiven
{
	std::list<tipUserinfo> lstinfo;
	uint32 timerid;
};
namespace __gnu_cxx
{
    template<> struct hash<const string>
    {
        size_t operator()(const string& s) const
        { return hash<const char*>()( s.c_str() ); } //__stl_hash_string
    };
    template<> struct hash<string>
    {
        size_t operator()(const string& s) const
        { return hash<const char*>()( s.c_str() ); }
    };
}

//////////////////////////////////////////////////////////////////////////

class CRoomManager
{
public:
	friend class CRoomObj;
	CRoomManager(void);
	virtual ~CRoomManager(void);

	int init(void);
	void finishInitData();
	void setConfSetting(Config &config, string configPath);
	void setRedisConf(redisMgr *pRedisMgr);
	int load_permission_config(const char *confile);
	redisOpt* getDataRedis();
	void loadKeyword();

	int LoadActiveRooms(uint32 vcbid = 0, bool create = false, const std::string host = std::string(""));
	int LoadDelRoom(uint32 vcbid);
	void LoadModRoom(uint32 vcbid, const std::string host = std::string(""));
	bool IsRoomExist(int vcbId);
	void updateRoomInfo(uint32 vcbid, int action);
	vector<string> getAllRoomid();
	RoomObject_Ref FindRoom(uint32 vcbId, bool updateFromDB = false);
	RoomObject_Ref FindRoom_Redis(uint32 vcbId);
	RoomObject_Ref FindRoom_DB(uint32 vcbId, RoomObject_Ref oldRoomObj = RoomObject_Ref(), bool writeRedis = true, const std::string host = std::string(""));
	void initwaitmicspeciality(void);
	void updateRoomPopularity();
	bool isMainRoom(uint32 vcbid);
	uint32 getMainRoom(uint32 nrobotid);
	uint makeTeacherRobotPack(CRoomObj *pRoomObj, CMDRoborTeacherIdNoty_t* pInfo);
	int LoadZhuanboRebot(void);
	void set_subroom_micstate(int vcbid, int userid, int pubmicindex, const char* szalias, int action);  //action:1-upmic, 0-downmic
	void setRoommicstate(int vcbid, int userid, int pubmicindex, const char* szalias, int action);  //action:1-upmic, 0-downmic
	int LoadPermissionFile(void);

	//CUserObj * CreateUserFromDB(int vcbid, int userid, char * pszUserPwd, int loginsource, unsigned int devtype);
	CUserObj * CreateUserFromDB(int vcbid, int userid);
	int redisSetRoomid(uint32 roomid);
	int redisSetRoomInfo(CRoomObj* p);
	clienthandler_ptr findconn(uint32 connid);
	//void exitroom_record(UserObject_Ref &userObjRef);
	int onGatewayDisconnected(clienthandler_ptr connection);
public:
	void CastSendMsgXXX(COM_MSG_HEADER* pOutMsg);
	void CastSendMsgXXX_i(COM_MSG_HEADER* pOutMsg);
	void castRoomSendMsg(COM_MSG_HEADER* pOutMsg, uint32 vcbid, bool IsMaskCur);
	void castUserSendMsg(COM_MSG_HEADER* pOutMsg, uint32 userid);
	void castGateway(COM_MSG_HEADER * pOutMsg);
	void castSubroom(COM_MSG_HEADER * pOutMsg, uint32 roomid, uint32 teacherid);
	void castRoom(COM_MSG_HEADER * pOutMsg, uint32 roomid, bool IsMaskCur);
	void castRoom_onesvr(COM_MSG_HEADER * pOutMsg, uint32 roomid);
	void castRooms_onesvr(const char * pData, unsigned int datalen, unsigned int maincmd, unsigned int subcmd, const std::set<unsigned int> & roomset);

	int Build_NetMsg(char * szBuf, int nBufLen, int mainCmdId, int subCmdId, ClientGateMask_t ** ppGateMask, void * pData, int pDataLen, int reqid = 0);
	int string2set(std::string &lst, std::set<int> &st);
	int set2string(std::set<int> &st, std::string &lst);

	/* for softbot */
	int load_softbot_alias(const char* filepath);
	void init_softbotids();
	void init_softbot_vip_percent();
	uint32 getsoftbotid() const;
	uint32 getnextsoftbotid();

	void getuseralias(int userid, char *alias, unsigned int aliasSize);
	int LoadVipTotalNum();
    /*
        task_node 不为空时不会进行广播而是发送到task_node的链路
    */
    void castSendWeekFansList(uint32 teacherid, uint32 roomid, task_proc_data *task_node = NULL);
    void castSendTopTeamList(task_proc_data *task_node = NULL);
    void calcTeamTopMoney(vector<CMDTeamTopNResp_t> &vecTeamTopMoney);
	void updateGroupUserCount(unsigned int groupid, unsigned int userid, bool bAdd);

public:
	std::map<uint32, clienthandler_ptr> m_mapConn;
    CConnection m_clientconn;
	/* for softbot */
	vector<uint32> m_vecSoftbotids;
	vector<uint32> randindex_arr;
	uint32 m_softbot_index;
	boost::mutex m_softbot_id_mutex;
	vector<string> m_vecSoftbotAlias;
	vector<string> m_vecSoftbotAlias_used;

	//用户等级排麦特性
	std::map<int, LevelWaitMicSpeciality_t >  m_viplevel_waitmic_speciality;
	std::map<int, LevelWaitMicSpeciality_t >  m_yiyuanlevel_waitmic_speciality;

	typedef std::vector<RoomZhuanboRobotConfig_t > ZHUANBO_ROBOT_VECTOR;
	SL_Sync_Mutex m_zhuanbo_robots_mutex;
	ZHUANBO_ROBOT_VECTOR  m_add_zhuanbo_robots;

	void setWorkID(uint16_t workID) { m_workid = workID; }
	uint16_t m_workid;

	void setSvrId(uint16_t svrid) { m_nsvrid = svrid; }
	uint16_t m_nsvrid;

	SL_Sync_Mutex m_questanswer_mutex;
	unsigned long m_QuestionAnswerNewestTime;

	redisMgr 	*m_pRedisMgr;

protected:
	bool m_initData;

	boost::shared_mutex m_mtxKeywordMap;
	typedef std::map<std::string, CMDAdKeywordInfo_t> KEYWORD_MAP;
	KEYWORD_MAP m_keyword_map;

	SL_Sync_Mutex m_keywork_mutex;
	redis_map	m_redis_viptotal_map;
	redis_map_set   m_HR_set;
	//非直播判断超时时间(s)
	uint32 m_uNonLiveTimeout;
	std::set<int>  noroomkeyRoomlevelset;
	std::set<int>  noroomkeylevelset;
	/* for softbot */
	uint32	m_softbot_min_no;
	uint32	m_softbot_max_no;
	string 	m_szSoftbotaliasfile;
	string	m_szPermissionFilePath;
	string 	m_szPtime;
	string 	m_szPFileMD5;

	//看悄悄话
	string m_WhisperRoomLevels;
	string m_WhisperLevels;
	string m_WhisperUserIDs;
	set<int>  whisperRoomlevelset;
	set<int>  whisperlevelset;
	set<int>  whisperUserIDset;

	//免费喇叭设置
	uint m_bsiege_laba;
	string m_FreelabaRoomLevels;
	string m_FreelabaLevels;
	set<int>  labaRoomlevelset;
	set<int>  labalevelset;

	//无须房间密码
	uint m_bsiege_noroomkey;
	string m_noroomkeyRoomLevels;
	string m_noroomkeyLevels;
	std::map<int , TipGiven > m_roomid2Appre;
public :
	void changeWaitUser(uint32 userid,uint32 vcbid,int action);
	void OnSilenceTimer(int32_t groupid);
	int procTip(const int32_t srcid, const int32_t toid, const int32_t groupid, const std::string& gifname, const int32_t tiptype,
		const std::string& srcname, const std::string& toname, const std::string& gifpicture);
};
