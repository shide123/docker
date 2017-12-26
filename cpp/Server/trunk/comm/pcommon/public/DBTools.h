#ifndef __DBMANAGER_H__
#define __DBMANAGER_H__

#include "DBConnection.h"
#include "table2.h"
#include "timestamp.h"
#include "cmd_vchat.h"
#include <vector>
#include <list>
#include "comm_struct.h"
#include "utils.h"

#define IF_TRUE_APPEND_SQL(cond, buf, apdfmt, value) \
		if (cond) sprintf(buf, "%s AND "apdfmt, buf, value);

#define IS_STR_EMPTY(pstr) (NULL == (pstr) || 0 == *(pstr))

enum e_QueryType
{
	QUERYTYPE_ALL,		//查找所有
	QUERYTYPE_VALID,	//查找生效的
	QUERYTYPE_INVALID,	//查找失效的
};
class CMDStampInfo;
class CDBTools
{
public:
	CDBTools();
	virtual ~CDBTools();
	static void initDB(Dbconnection * db){m_db = db;}
	static bool initSyncDB()
	{
		m_syncdb = Dbconn::getSyncDB();
		if (NULL == m_syncdb)
			return false;
		return true;
	}

	//增加、删除房间管理员
	static bool modRoomManager(unsigned int vcbid, unsigned int userid, int mgrtype, bool bAdd /*= true*/);
	//获取转播房间信息
	static int getZhuanboInfoByMainRoom(int* list, unsigned int roomid, unsigned int userid);
	//获取主房间上麦讲师id
	static int getMainroomByZhuanboInfo(unsigned int roomid, unsigned int userid);
	
	static int checkTeacherScore(unsigned int teacherid, unsigned int userid, char* logtime);
	static bool addTeacherScoreLog(CMDTeacherScoreRecord_t2* teacherlog);
	static int getTeacherGiftList(std::vector<CMDTeacherThisWeekFans_t> &vOut, unsigned int roomid,unsigned int teacherid);
	static int teacherSubscript(unsigned int userid, unsigned int teacherid, bool bSub);
	static int getSubscriptState(unsigned int userid,unsigned int teacherid);

	//校验关键字是否存在
	static bool existAdKeyword(char *pKeyword);
	static bool addAdKeyword(CMDAdKeywordInfo_t *pKeyword);
	static bool delAdKeyword(CMDAdKeywordInfo_t *pKeyword);
	static int getAdKeywordList(int startline, int linenum, std::vector<CMDAdKeywordInfo_t> &vKeyWord);
	static int getAdKeywordList(int startline, int linenum, std::vector<std::string> &vKeyWord);

	static bool updateRoomBaseInfo(int vcbid, const DDRoomBaseInfo_t &pInfo);
	static int updateRoomOptStat(unsigned int vcbid, int nopstat);
	static int updateRoomNotice(unsigned int vcbid, unsigned int userid, const char* alias, int noticeid, char* szText);

	static bool isCollectRoom(int userid, int vcbid);
	static bool collectRoom(unsigned int userid, unsigned int vcbid, int action);

	static int get_push_type_and_token(unsigned int userid, std::vector<PushInfo_t> &lstOut);

	static int get_add_zhuanbo_robots_DB(std::list<DDzhaunborobotinfo_t> &lstOut);
	static int get_del_zhuanbo_robots_DB(std::list<DDzhaunborobotinfo_t> &lstOut);
	static int get_vcballroominfolist_DB(std::list<DDRoomFullInfo_t> &lstOut);
	static int get_vcbfullinfolist_DB(std::list<DDRoomFullInfo_t> &lstOut);
	static int get_delvcbIdlist_DB(int* vcbIds, int maxnum);
	static int get_vcbbaseinfolist_DB(std::list<DDRoomBaseInfo_t> &lstOut);
	static int get_visitor_records(uint32 userid);
	static int write_addlastgoroom_DB(uint32 userid, uint32 vcbid, uint32 type);
	static int isin_blackIpMac_DB(int userid, int vcbid, int scopetype, char* szip, char* szmac, char* szdiannao, DDViolationInfo_t* pInfo = NULL);
	static int get_room_collect_num(uint32_t vcbid, uint32_t &pNum);
	static int get_vcbmedia_addr_byareaid(uint32 vcbid, const char* areadid, char * mediaip, size_t len);
	static int update_vcbmedia_DB(uint32 vcbid, const char *paddr);
	static int update_room_activetime(uint32_t roomid, uint64_t tActive);
	static int update_room_popularity(uint32_t vcbid, uint64_t accesstimes);
	static int get_syscasts_DB(list<DDSysCastNews_t> &listout, int casttype, int recid);
	static int get_teacherid_from_robotid(uint32 subroomid, uint32 robotid, uint32 &teacherid, uint32 &mroomid);
	//赠送礼物存储过程(调用存储过程版本)
	static int func_tradegift_DB(unsigned int src_id, unsigned int dst_id, unsigned int gift_id, unsigned int gift_count, int gift_type, int64 total_out, int64 total_in, int trade_type, int room_id, \
		int64 * src_now_nk, int64 * src_now_nb, int64 * dst_now_nk, int64 * dst_now_nb,	int * src_add_nchestnum, int * src_now_nchestnum);
	//赠送普通礼物(不调用存储过程版本)
	static int func_trade_normal_gift_DB(unsigned int src_id, unsigned int dst_id, unsigned int gift_id, unsigned int gift_count, int gift_type, int64 total_out, int64 total_in, int trade_type, int room_id, \
		int64 * src_now_nk, int64 * src_now_nb, int64 * dst_now_nk, int64 * dst_now_nb,	int * src_add_nchestnum, int * src_now_nchestnum);

	//更新用户月消费记录
	static int func_updateusermonthcost_DB(unsigned int userid, int64 cost_nk);

	//logonsvr
	static bool delete_teacherfans_DB(unsigned int teacherID, unsigned int fansID);
	static int check_teacherfans_exist_DB(unsigned int teacherID, unsigned int fansID);
	static bool insert_teacherfans_DB(unsigned int teacherID, unsigned int fansID);
	static int get_user_vip_info(unsigned int nuserid, unsigned int platform_id, CUserVip_t & userVipInfo);
	static int get_user_exit_message(int userid, CMDSecureInfo_t &pTab, int &hit_times);
	//获取可用的游客id
	static int get_vistorlogon_DB(char *_uuid, std::vector<VistorLogon_t> &vVistor);
	////更新游客登陆状态数据
	static bool update_vistorlogon_DB(const VistorLogon_t &vsl);
	static int func_get_visitor_unique_id(const unsigned int area);
	//插入游客登陆状态数据
	static bool insert_vistorlogon_DB(const VistorLogon_t &vsl);
	static bool get_user_pwdinfo_DB(int userid, DDUserPwdInfo_t &ppf);
	static bool update_userpwd_DB(int userid, char* pPwd, int pwdtype);
	static int get_user_secure_info_DB(int userid, CMDSecureInfo_t &pTab);
	static int get_privilegelist_DB(vector<DDQuanxianAction_t> &vAction);

	//查询系统参数
	static int qrySysParameterInt(const char * paramname, const int defValue = 0);
	static string qrySysParameterString(const char *paramname, const char *defValue = "");

	//查询字典参数
	static bool qryDictItemByGroupId(const char *group, vector<DictItem_t> &vOut);

	static bool write_room_maxclientnum(int vcbid, int maxclientnum);
	static int get_isroommanager_DB(uint32 userid, uint32 vcbid);

	////////////////////////////////////new//////////////////////////////////////////////////////////////////////////
	//room info and setting
	static int getOneGroupfullinfo_DB(stRoomFullInfo & record, uint32 vcbid);
	static int getGroupBroadInfo_DB(unsigned int group_id, StRoomNotice_t & oBroadInfo);
	static bool updateGroupBroadInfo_DB(unsigned int group_id, const StRoomNotice_t & oBroadInfo);
	static bool updateGroupBasicInfo_DB(unsigned int group_id, std::map<std::string, std::string> & oFieldValueMap);
	static int getActiveGroupfullinfo_DB(std::list<stRoomFullInfo> & oDBRecords);
	static bool updateGroupMsgMute(unsigned int userid, unsigned int groupid, unsigned int muteAction);
	static bool getGroupMsgMute(unsigned int userid, unsigned int groupid);
	static bool checkBlockUserLst_DB(unsigned int userid, unsigned int groupid);
	static bool initRoomUserCountDB(uint32_t vcbid, unsigned int & userNum, unsigned int & maleNum, unsigned int & femaleNum);
	static bool updateRoomVisitorNum(uint32_t vcbid, uint32_t visitorNum);
	static bool updateRoomUserCount(uint32_t vcbid, const std::map<std::string, std::string> & oFieldAndIncMap);
	
	static int write_logonrecord(const CMDUserLoginLog_t & oLogonlog);
	static int check_user_token_DB(unsigned int loginid, const std::string & strToken, bool checkTime);
	static bool get_user_token_DB(int userid, char * token, char * codetime);
	static bool update_user_token_DB(unsigned int userid, const char * token, const char * validtime);
	static int resetTokenValidTime(unsigned int loginid, const std::string & strToken);
	static bool checkUserHasAuth(unsigned int userid, unsigned int groupid, unsigned int authType);
	static int userApplyJoinGroupAuth(unsigned int userid, unsigned int groupid, unsigned int authType, const std::string & user_msg);
	static bool updateJoinGroupAuthMsgID(unsigned int authID, unsigned int mongoMsgID);
	static bool qryJoinGroupAuth(unsigned int id, unsigned int & groupAssistID, unsigned int & apply_userID, unsigned int & groupID);
	static bool handleJoinGroupAuth(unsigned int id, bool agree);
	static int qryGroupOwner(unsigned int groupid);
	static int updateGroupOwner(unsigned int groupid, unsigned int new_owner);
	static bool updateGroupMicMode(unsigned int groupid, int mode);

	//get user info
	static int get_user_fullinfo_DB(StUserFullInfo_t & ptf, uint32 userid);
	static int get_user_fullinfoByLoginID_DB(StUserFullInfo_t & ptf, uint32 loginID, const std::string & patternlock);
	static bool update_userinfo_DB(unsigned int userid, const std::map<std::string, std::string> & field_value_map);
	static bool updateUserRoletype_DB(unsigned int userid, unsigned int groupid, unsigned int new_roletype);

	//user group info
	static int getGroupIDLst_DB(std::list<unsigned int> & groupIDLst);
	static int getGroupMemebr_DB(unsigned groupid, std::list<stJoinGroupInfo> & oGroupMemLst);
	static int getUserRelateGroupLst_DB(unsigned int userid, std::list<stGroupInfo> & groupLst);
	static bool addUserGroup_DB(const stJoinGroupInfo & oJoinObj);
	static bool delUserGroup_DB(unsigned int userid, unsigned int groupid);
	static bool isUserInGroup(unsigned int userid, unsigned int groupid, bool checkDBRecord);
	static bool getGroupMaxUnreadNum(uint32 groupid, int &num);
	static bool updateSelfChatPermission(unsigned int userid, unsigned int groupid);
	static int get_userGroupInfo_DB(unsigned int userid, unsigned int groupid, StUserGroupInfo & oUserGroupInfo);
	static int get_groupUserInfoList_DB(unsigned int groupid, std::map<uint32, StUserGroupInfo> &vGroupUserInfo);
	static int get_userBaseAndGroupInfo_DB(unsigned int userid, unsigned int groupid, StUserBaseAndGroupInfo & oUserInfo);
	static int get_groupUserInfoList_EX(unsigned int groupid, std::map<uint32, StUserBaseAndGroupInfo> &mUserInfo);

	//red packet
	static int getRedPacketBasicInfo(unsigned int packetID, stRedPacketInfo & packetInfo);
	static long long takeRandomRedPacket(unsigned int packetID, unsigned int packetType, unsigned int groupID, unsigned int take_userID, unsigned long long rand_money, unsigned int taken_percent = 0, unsigned int goldCandyExchange = 1);
	static long long takeUniRedPacket(unsigned int packetID, unsigned int packetType, unsigned int groupID, unsigned int take_userID, unsigned int taken_percent = 0, unsigned int goldCandyExchange = 1);
	static long long takeDirectRedPacket(unsigned int packetID, unsigned int packetType, unsigned int groupID, unsigned int take_userID, unsigned int taken_percent = 0, unsigned int goldCandyExchange = 1);
	static long long takeRedPacket(unsigned int packetID, unsigned int send_user, unsigned int packetType, unsigned int groupID, unsigned int take_user, unsigned long long take_money, unsigned int taken_percent = 0, unsigned int goldCandyExchange = 1);
	static string buildSendRedPacketSQL(const stRedPacketInfo & oPacketReq, unsigned int gold_contribute_exchange = 1);
	static int getRedPacketLog(uint32 packetID, std::vector<stRedPacketLog> & vLog, uint32 limit = 0, uint32 offset = 0);
	static int getRedPacketToRecycle(unsigned int timeout, std::vector<unsigned int> & vList);
	static bool recycleOneRedPacket(unsigned int packetID, unsigned int goldCandyExchange, std::string &result);
	static bool getWeekCharmUserLst(unsigned int groupid, std::list<stContributeValue> & oContributeLst);
	static bool getWeekContributeUserLst(unsigned int groupid, std::list<stContributeValue> & oContributeLst);
	static long long getUserTakeMoney(unsigned int userID, unsigned int packetID);
	static bool getGroupGainPercent(uint32 groupid, uint32 userid, uint32 &percent);
	static unsigned int getTopRedPacketTakeUserID(unsigned int packetID);
	static bool check_user_has_takephoto(unsigned int userID, unsigned int packetID);
	
	//发送印章请求
	static int SendPressStampReq(unsigned int userid, unsigned int  dstuserid, unsigned int groupID, unsigned int money, std::string strMessage);
	//获取印章信息
	static int getStampInfo(unsigned int userid, unsigned int  dstuserid, unsigned int groupID, CMDStampInfo& info);
	//获取当前群定时红包信息
	static int getFixtimeRedPacketInfoByGrouID(unsigned int userid, unsigned int groupID, unsigned int currow, unsigned int rows, unsigned int fixtype, unsigned int & effrow, unsigned int packtype, std::list<stRedPacketInfo> &lstPacketInfo);
	//获取结果集
	static int getFixtimeRedPacketData(const std::stringstream& strSelectSQL, unsigned int fixtype, unsigned int &effrow, std::list<stRedPacketInfo> &lstPacketInfo);
	//更新群用户信息 
	static bool updGroupMemberInfo(uint32 groupid, uint32 userid, std::map<std::string, std::string> &mValues);
	template <class Type>
	static bool updGroupMemberInfo(uint32 groupid, uint32 userid, const std::string field, Type value);
	//更新群基本信息
	template <class Type>
	static bool getGroupBasicInfo(uint32 groupid, const std::string field, Type &value);
	static bool updGroupBasicInfo(uint32 groupid, std::map<std::string, std::string> &mValues);
	template <class Type>
	static bool updGroupBasicInfo(uint32 groupid, const std::string field, Type value);
	//更新群插件设置
	template <class Type>
	static bool getGroupPluginSetting(uint32 groupid, const std::string field, Type &value);
	static bool updGroupPluginSetting(uint32 groupid, std::map<std::string, std::string> &mValues);
	template <class Type>
	static bool updGroupPluginSetting(uint32 groupid, const std::string field, Type value);

	static bool getGroupPChatPermission(uint32 groupid, uint32 userid, uint32 &permission, uint32 &contribution, uint32 &threshold);

	//红包相关
	static int select_luckynum_by_packetid(unsigned int packetID);
	/*夺宝相关*/
	static bool checkSponsorTreasure(uint32 groupid);

	//ppt课程相关
	static int getPPTMaxPicIdByID(const int32 &groupId, string & sPicId);
	static int getPPTPicIdsByGroupID(const int32 &groupId, std::vector<pptPicInfo> & vPicInfo);

	//停开播流水
	static bool insertLiveState(uint32 iGroupId,uint32 iUid,int state,const string&sLiveUrl);
	static bool getLiveUrlByGroupid(uint32 iGroupId, string&sLiveUrl);

	static bool insertLiveFlow(uint32 iGroupId,uint32 iUid,int state,const string&sLiveUrl);

	//
	static bool insertCommentAudit(const commentAudit &tcommentAudit	);
	static bool transBegin() { return m_db->transBegin() >= 0; }
	static void transCommit() { m_db->transCommit(); }
	static void transRollBack() { m_db->transRollBack(); }

	static bool updateCommentAuditMsgId(int modifyId,int msgId,int pMsgId );
	
protected:
	static string build_blacklist_sql(int userid, int vcbid, int scopetype, char* szip, char* szmac, char* szdiannao);
	static void buildSetValuesList(std::stringstream &sqlValues, const std::map<std::string, std::string> &mValues);

private:
	static Dbconnection *m_db;
	static Dbconnection *m_syncdb;
};

template <class Type>
bool CDBTools::updGroupMemberInfo(uint32 groupid, uint32 userid, const std::string field, Type value)
{
	std::string strValue = bitTostring(value);
	if (!groupid || !userid || field.empty() || strValue.empty())
		return false;

	std::stringstream sql;
	sql << " update mc_group_member set " << field << " = '" << strValue << "'" <<
			" where is_del = 0 " <<
			"   and group_id = " << groupid <<
			"   and user_id = " << userid;

	if (!m_db->run_sql(sql.str().c_str()))
		return false;

	return m_db->getAffectedRow() > 0;
}

template <class Type>
bool CDBTools::getGroupBasicInfo(uint32 groupid, const std::string field, Type &value)
{
	if (!groupid || field.empty())
		return false;

	std::stringstream sql;
	sql << " select " << field.c_str() << " from mc_group where is_del = 0 and id = " << groupid;

	int row = 0;
	Result res;
	if (!m_db->run_sql(sql.str().c_str(), &res, &row) || 0 == row)
	{
		m_db->CleanRes(&res);
		return false;
	}

	m_db->GetFromRes(value, &res, 0, 0);
	m_db->CleanRes(&res);
	return true;
}

template <class Type>
bool CDBTools::updGroupBasicInfo(uint32 groupid, const std::string field, Type value)
{
	std::string strValue = bitTostring(value);
	if (!groupid || field.empty() || strValue.empty())
		return false;

	std::stringstream sql;
	sql << " update mc_group set " << field << " = '" << strValue << "'" <<
		" where is_del = 0 and id = " << groupid;

	if (!m_db->run_sql(sql.str().c_str()))
		return false;

	return m_db->getAffectedRow() > 0;
}

template <class Type>
bool CDBTools::getGroupPluginSetting(uint32 groupid, const std::string field, Type &value)
{
	if (!groupid || field.empty())
		return false;

	std::stringstream sql;
	sql << " select " << field.c_str() << " from mc_group_plugin_setting_detail where group_id = " << groupid;

	int row = 0;
	Result res;
	if (!m_db->run_sql(sql.str().c_str(), &res, &row) || 0 == row)
	{
		m_db->CleanRes(&res);
		return false;
	}

	m_db->GetFromRes(value, &res, 0, 0);
	m_db->CleanRes(&res);
	return true;
}

template <class Type>
bool CDBTools::updGroupPluginSetting(uint32 groupid, const std::string field, Type value)
{
	std::string strValue = bitTostring(value);
	if (!groupid || field.empty() || strValue.empty())
		return false;

	std::stringstream sql;
	sql << " update mc_group_plugin_setting_detail set " << field << " = '" << strValue << "'" <<
		" where group_id = " << groupid;

	if (!m_db->run_sql(sql.str().c_str()))
		return false;

	return m_db->getAffectedRow() > 0;
}


#endif //__DBMANAGER_H__
