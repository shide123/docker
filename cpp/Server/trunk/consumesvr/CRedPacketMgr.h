#ifndef __CREDPACKET_MGR_H__
#define __CREDPACKET_MGR_H__

#include "json/json.h"
#include "redisMgr.h"
#include "redis_map_map.h"
#include "redis_map_set.h"
#include "redis_map_zset.h"
#include "comm_struct.h"
#include "ProtocolsBase.h"
#include "DBConnection.h"
#include "ConsumeSvr.pb.h"
#include "RoomObj.h"
#include "Common.pb.h"

typedef enum
{
	eHasMoney	= 0,
	eNoLeft		= 1
}eFinishState;

class CRedPacketMgr
{
public:
	CRedPacketMgr(Dbconnection * dbconn);
	virtual ~CRedPacketMgr();
	int init(redisMgr * pRedisMgr);

	//发红包入口
	int handle_sendRedPacketReq(task_proc_data * message);
	//抢红包入口
	int handle_catchRedPacketReq(task_proc_data * message);
	//领红包入口
	int handle_takeRedPacketReq(task_proc_data * message);
	//红包详情请求
	int handle_qryRedPacketInfoReq(task_proc_data * message);
	//查询群的贡献榜
	int handle_qryWeekContributeReq(task_proc_data * message);
	//查询群定时红包
	int handle_QryFixTimeRedPacketReq(task_proc_data * message);
	

	static void recycleRedPacket();
	static void cleanRedPacketCache(unsigned int now_time);
	static unsigned int m_validtime;
	static unsigned int m_cacheTime;
	static unsigned int m_comboTime;
	static unsigned int m_goldCandyExchange;
	static unsigned int m_goldContributeExchange;
	static unsigned int m_cleanCacheTime;
	//地点红包误差范围
	static unsigned int m_location_diff;
	//印章起拍价
	static unsigned int m_stampStartPrice;
	//印章每次加价不低于多少元
	static unsigned int m_stampIncreasePrice;

private:
	//求弧度
	double radian(double d);
	//计算两个经纬度间距离，单位米
	double get_distance(double lat1, double lng1, double lat2, double lng2);

	//定时红包查询和组包逻辑 
	int getFixRePacketList(unsigned int userid, unsigned int groupID, unsigned int currow, unsigned int rows, unsigned int fixtype, unsigned int packtype, CMDQryRedPacketRsp&rspData);
	
	int sendRedPacketLuckType(CMDSendRedPacketReq & reqData, unsigned int & packetID, unsigned int & comboNum);
	int sendRedPacketUniType(CMDSendRedPacketReq & reqData, unsigned int & packetID, unsigned int & comboNum);
	int sendRedPacketDirectType(CMDSendRedPacketReq & reqData, unsigned int & packetID, unsigned int & comboNum, bool isPrivate);
	int sendRedPacketLightType(CMDSendRedPacketReq & reqData, unsigned int & packetID, unsigned int & comboNum);
	int sendRedPacketJoinGroup(CMDSendRedPacketReq & reqData, unsigned int & packetID, unsigned int & comboNum);
	int sendRedPacketCommHandle(stRedPacketInfo & oRedPacketInfo, Json::Value & oPacketJson, unsigned int & packetID, unsigned int & comboNum, bool isPrivate);
	unsigned int getComboNumber(unsigned int send_user_id, const stRedPacketInfo & oNewRedPacket);
	void notifyWeekList(unsigned int userid, unsigned int groupid, bool bCharmList = false);

	void setInvalidTime(stRedPacketInfo &oRedPacketInfo);

	int checkUserValid(unsigned int userID, unsigned int groupID, unsigned int & roleType);
	int checkRedPacketCanTake(unsigned int userID, unsigned int groupID, unsigned int packetID, double lg, double	la, CMDErrCode *pErr = NULL);
	int checkRedPacketValid(unsigned int userID, unsigned int roleType, unsigned int packetID, std::map<std::string, std::string > & field_value_map, double lg, double	la, CMDErrCode *pErr = NULL,const string&strmsg="");
	int checkJoinGroupRedPacketNeed(unsigned int userID, RoomObject_Ref & roomObjRef, unsigned int & packetMoney, unsigned int & packetNum);
	int takeRedPacket(unsigned int userID, unsigned int groupID, unsigned int packetID, unsigned int & takeMoney, stRedPacketInfo &packInfo, double	lg, double	la, CMDErrCode *pErr = NULL, const string&strmsg = "");
	bool sendRedPacketInfo(task_proc_data & message, uint32 userID, uint32 packetID, uint32 groupID, uint32 count = 20, uint32 offset = 0);
	bool getRedPacketInfo(CMDRedPacketInfo & info, uint32 packetID, uint32 groupID, uint32 count = 20, uint32 offset = 0);
	void getWeekList(unsigned int userid, unsigned int groupid, CMDWeekListNotify & oNotyData, bool & bNeedNotify, bool bCharmList = false);

	long long getRandomPacket(const stRedPacketInfo & packInfo);
	void genPacketObj2MsgJson(const CMDSendRedPacketReq & oRedPacketReq, Json::Value & oPacketJson);
	void genPacketObj2FieldValue(const stRedPacketInfo & oRedPacketInfo, std::map<std::string, std::string> & field_value_map);
	bool genPacketObjFromRedis(unsigned int packetID, stRedPacketInfo & oRedPacketInfo);
	void reloadPacket2Redis(unsigned int packetID);
	void reloadPacket2Redis(unsigned int packetID, std::map<std::string, std::string > & oFieldValueDBMap);
	unsigned int callJoinGroupRPC(unsigned int userid, unsigned int groupid);
	unsigned int callSendChatRPC(unsigned int src_userid, unsigned int dst_userid, unsigned int groupid, const std::string & chatmsg, bool isPrivate, uint32 msgtype = MSGTYPE_REDPACK);

	static redis_map_map m_redpacket_hash;
	static redis_map_map m_redpacket_users;
	static redis_map_map m_redpacket_senduser;
	static redis_map_zset m_redpacket_ids;
	static unsigned int m_timer_redpacket_validtime;
	
	Dbconnection * m_dbconn;
};

#endif //__CREDPACKET_MGR_H__
