#ifndef __DBSINK_H__
#define __DBSINK_H__

#include "DBConnection.h"
#include "cmd_vchat.h"
#include "comm_struct.h"
#include "utils.h"
#include "errcode.h"

#define IF_TRUE_APPEND_SQL(cond, buf, apdfmt, value) \
		if (cond) sprintf(buf, "%s AND "apdfmt, buf, value);

#define IS_STR_EMPTY(pstr) (NULL == (pstr) || 0 == *(pstr))

#define PRELOADED_SYS_PARAM_PHP_API_DOMAIN "PHP_API_DOMAIN"

class CDBSink :
	public Dbconnection
{
public:
	CDBSink();
	CDBSink(Sink *s);
	virtual ~CDBSink(void);
	static CDBSink *getSinkByHost(const std::string &host);

	//校验关键字是否存在
	bool existAdKeyword(char *pKeyword);
	bool addAdKeyword(CMDAdKeywordInfo_t *pKeyword);
	bool delAdKeyword(CMDAdKeywordInfo_t *pKeyword);
	int getAdKeywordList(int startline, int linenum, std::vector<CMDAdKeywordInfo_t> &vKeyWord);

	int write_addlastgoroom_DB(uint32 userid, uint32 vcbid, uint32 type);
	int isin_blackIpMac_DB(int userid, int vcbid, int scopetype, char* szip, char* szmac, char* szdiannao, DDViolationInfo_t* pInfo = NULL);
	int update_vcbmedia_DB(uint32 vcbid, const char *paddr);

	//更新用户月消费记录
	int func_updateusermonthcost_DB(unsigned int userid, int64 cost_nk);

	//logonsvr
	int get_user_fullinfo_DB(StUserFullInfo_t & ptf, char * liginId, int loginType);
	int func_get_visitor_unique_id(const unsigned int area);
	//插入游客登陆状态数据
	bool insert_vistorlogon_DB(const VistorLogon_t &vsl);
	bool get_user_pwdinfo_DB(int userid, DDUserPwdInfo_t &ppf);
	bool update_userpwd_DB(int userid, char* pPwd, int pwdtype);
	int get_user_secure_info_DB(int userid, CMDSecureInfo_t &pTab);
	int get_privilegelist_DB(vector<DDQuanxianAction_t> &vAction);

	//查询系统参数
	int qrySysParameterInt(const char * paramname, const int defValue = 0);
	string qrySysParameterString(const char *paramname, const char *defValue = "");
	int getPreloadedSysParamInt(const char *paramname, int &result);
	int getPreloadedSysParamString(const char *paramname, string &result);

	//查询字典参数
	bool qryDictItemByGroupId(const char *group, vector<DictItem_t> &vOut);

	bool write_room_maxclientnum(int vcbid, int maxclientnum);
	int get_isroommanager_DB(uint32 userid, uint32 vcbid);

	int check_isMaJia(uint32 userid);
	int check_isLiuLiangZhu(uint32 userid);
	////////////////////////////////////new//////////////////////////////////////////////////////////////////////////
	//room info and setting
	int getOneGroupfullinfo_DB(stRoomFullInfo & record, uint32 vcbid);
	int getGroupBroadInfo_DB(unsigned int group_id, StRoomNotice_t & oBroadInfo);
	bool updateGroupBroadInfo_DB(unsigned int group_id, const StRoomNotice_t & oBroadInfo);
	bool updateGroupBasicInfo_DB(unsigned int group_id, std::map<std::string, std::string> & oFieldValueMap);
	int getActiveGroupfullinfo_DB(std::list<stRoomFullInfo> & oDBRecords);
	bool updateGroupMsgMute(unsigned int userid, unsigned int groupid, unsigned int muteAction);
	bool getGroupMsgMute(unsigned int userid, unsigned int groupid);
	bool checkBlockUserLst_DB(unsigned int userid, unsigned int groupid);
	bool initRoomUserCountDB(uint32_t vcbid, unsigned int & userNum, unsigned int & maleNum, unsigned int & femaleNum);
	bool updateRoomVisitorNum(uint32_t vcbid, uint32_t visitorNum);
	bool updateRoomUserCount(uint32_t vcbid, const std::map<std::string, std::string> & oFieldAndIncMap);
	void getGroupAdminLst_DB(unsigned int groupid, std::set<unsigned int> & oGroupAdminLst);

	int write_logonrecord(const CMDUserLoginLog_t & oLogonlog);
	int check_user_token_DB(unsigned int loginid, const std::string & strToken, bool checkTime);
	int resetTokenValidTime(unsigned int loginid, const std::string & strToken);
	bool checkUserHasAuth(unsigned int userid, unsigned int groupid, unsigned int authType);
	int userApplyJoinGroupAuth(unsigned int userid, unsigned int groupid, unsigned int authType, const std::string & user_msg);
	bool updateJoinGroupAuthMsgID(unsigned int authID, unsigned int mongoMsgID);
	bool qryJoinGroupAuth(unsigned int id, unsigned int & groupAssistID, unsigned int & apply_userID, unsigned int & groupID);
	bool handleJoinGroupAuth(unsigned int id, bool agree);
	int qryGroupOwner(unsigned int groupid);
	int updateGroupOwner(unsigned int groupid, unsigned int new_owner);
	bool updateGroupMicMode(unsigned int groupid, int mode);

	//get user info
	int get_user_fullinfo_DB(StUserFullInfo_t & ptf, uint32 userid);
	int get_user_fullinfoByLoginID_DB(StUserFullInfo_t & ptf, uint32 loginID, const std::string & patternlock);
	template<typename Type>
	bool update_userinfo_DB(unsigned int userid, const std::string field, const Type &value);
	bool update_userinfo_DB(unsigned int userid, const std::map<std::string, std::string> & field_value_map);
	bool updateUserRoletype_DB(unsigned int userid, unsigned int groupid, unsigned int new_roletype);

	//user group info
	int getGroupIDLst_DB(std::list<unsigned int> & groupIDLst);
	int getGroupMemebr_DB(unsigned groupid, std::list<stJoinGroupInfo> & oGroupMemLst);
	int getUserRelateGroupLst_DB(unsigned int userid, std::list<stGroupInfo> & groupLst);
	bool addUserGroup_DB(const stJoinGroupInfo & oJoinObj);
	bool delUserGroup_DB(unsigned int userid, unsigned int groupid);
	bool isUserInGroup(unsigned int userid, unsigned int groupid, bool checkDBRecord);
	bool getGroupMaxUnreadNum(uint32 groupid, int &num);
	bool updateSelfChatPermission(unsigned int userid, unsigned int groupid);
	int get_userGroupInfo_DB(unsigned int userid, unsigned int groupid, StUserGroupInfo & oUserGroupInfo);
	int get_groupUserInfoList_DB(unsigned int groupid, std::map<uint32, StUserGroupInfo> &vGroupUserInfo);
	int get_userBaseAndGroupInfo_DB(unsigned int userid, unsigned int groupid, StUserBaseAndGroupInfo & oUserInfo);
	int get_groupUserInfoList_EX(unsigned int groupid, std::map<uint32, StUserBaseAndGroupInfo> &mUserInfo);

	//red packet
	int getRedPacketBasicInfo(unsigned int packetID, stRedPacketInfo & packetInfo);
	long long takeRandomRedPacket(unsigned int packetID, unsigned int packetType, unsigned int groupID, unsigned int take_userID, unsigned long long rand_money, unsigned int taken_percent = 0, unsigned int m_goldCandyExchange = 1);
	long long takeUniRedPacket(unsigned int packetID, unsigned int packetType, unsigned int groupID, unsigned int take_userID, unsigned int taken_percent = 0, unsigned int m_goldCandyExchange = 1);
	long long takeDirectRedPacket(unsigned int packetID, unsigned int packetType, unsigned int groupID, unsigned int take_userID, unsigned int taken_percent = 0, unsigned int m_goldCandyExchange = 1);
	string buildSendRedPacketSQL(const stRedPacketInfo & oPacketReq, unsigned int gold_contribute_exchange = 1);
	int getRedPacketLog(uint32 packetID, std::vector<stRedPacketLog> & vLog, uint32 limit = 0, uint32 offset = 0);
	int getRedPacketToRecycle(unsigned int timeout, std::vector<stRedPacketInfo> & vList);
	bool recycleOneRedPacket(unsigned int packetID, unsigned int goldCandyExchange, int &result);
	bool getWeekCharmUserLst(unsigned int groupid, std::list<stContributeValue> & oContributeLst);
	bool getWeekContributeUserLst(unsigned int groupid, std::list<stContributeValue> & oContributeLst);
	long long getUserTakeMoney(unsigned int userID, unsigned int packetID);
	bool getGroupGainPercent(uint32 groupid, uint32 userid, uint32 &percent);
	unsigned int getTopRedPacketTakeUserID(unsigned int packetID);
	bool check_user_has_takephoto(unsigned int userID, unsigned int packetID);

	//更新群用户信息
	bool updGroupMemberInfo(uint32 groupid, uint32 userid, const std::map<std::string, std::string> &mValues);
	template <class Type>
	bool updGroupMemberInfo(uint32 groupid, uint32 userid, const std::string field, Type value);
	//更新群基本信息
	template <class Type>
	bool getGroupBasicInfo(uint32 groupid, const std::string field, Type &value);
	bool updGroupBasicInfo(uint32 groupid, std::map<std::string, std::string> &mValues);
	template <class Type>
	bool updGroupBasicInfo(uint32 groupid, const std::string field, Type value);
	//更新群插件设置
	template <class Type>
	bool getGroupPluginSetting(uint32 groupid, const std::string field, Type &value);
	bool updGroupPluginSetting(uint32 groupid, std::map<std::string, std::string> &mValues);
	template <class Type>
	bool updGroupPluginSetting(uint32 groupid, const std::string field, Type value);
	bool updWhenGroupPChatThresholdMod(uint32 groupid, uint32 threshold, bool isMale);
	int getUserMinGroupPChatThreshold(uint32 userid, uint32 groupid);

	uint32 getUserListOverChatLimitAndUpdate(uint32 groupid, uint32 limit, std::vector<uint32> &vList);
	bool getGroupPChatPermission(uint32 groupid, uint32 userid, uint32 &permission, uint32 &contribution, uint32 &threshold);
	bool getUnreadSysNoticeMsgList(uint32 userid, std::string tel, int8 devtype, std::string appVersion, std::map<uint32, StNoticeMsg_t> &mMsg, std::map<uint32, std::vector<StCondition_t> > &mCond);
	int32 getUnreadSysNoticeMsgList(uint32 userid, std::vector<StNoticeMsg_t> &vList);
	int32 getUnreadUserNoticeMsgList(uint32 userid, std::vector<StNoticeMsg_t> &vList);
	bool chkUnreadSysNoticeMsgCondition(uint32 msgid, uint32 userid, std::string tel, int8 devtype, std::string appVersion);
	void updReadNoticeMsgIdOfUser(uint32 userid, uint32 msgid, bool isUser = false);
	int getAllUnhandledPushMsg(const int beginMinutesSinceNow, const int endMinutesSinceNow, std::vector<uint32> &msgIds);

	bool getGroupUserIntimacy(uint32 userid, uint32 dstuid, uint32 groupid, uint32 &threshold, uint32 &intimacy);

	bool modUserFriendship(uint32 userid, uint32 dstuid, int action);
	int32 getUserFriendList(uint32 userid, std::vector<uint32> &list, int relation);
	int getUserFriendship(uint32 userid, uint32 friendid);


	int getNotifySysConfig_DB(uint32 userid, int devtype, std::list<SysConfig_t > & sysconfigLst);
	
	int getAppId2Key(std::map<string,string>& vak);

	int get_jiabing_DB(unsigned int userid, unsigned int groupid);
	int get_token_DB(std::string& token,unsigned int& tokentime);
	bool updateForbidUserChat(unsigned int groupid, int status);
	bool updateCourseFinish(unsigned int groupid);
	bool getAllYesterDayCourse(std::vector<uint32> & vgids);
	bool getAllYesterDayLiveidAndCourseCount(std::map<uint32,uint32> & mapgids);
	bool getAllLiveidAndCourseCount(std::map<uint32,uint32> & mapgids);
	bool insertLiveFocusRobot(uint32 liveid,std::string name);
	bool getRobotNamebyLiveid(uint32 liveid,std::vector<std::string>& vRobots);
	bool getAllBeginCourseBefore(int begin,int end,std::vector<uint32>& vcids);
	bool getAllBeginCourseBefore(int begin,int end,std::vector<uint32>& vcids,std::vector<uint32>& uids,
		std::vector<std::string>& classNames, std::vector<std::string>& teacherNames, std::vector<std::string> &beginTimestamps);
	bool updateRobots(uint32 liveid,uint32 count);
	bool getTotalTipAmount(uint32 groupid,std::list<tipUserinfo>& infolst);
	bool updateOnlineRoomUserCount(uint32 liveid, uint32 count);

	bool updateOnlineCourseUserCount(uint32 liveid, uint32 count);

	unsigned int getRedPacketLogByTakeUserAndID(unsigned int packetID, unsigned int takeUser);
	//删除 talk_recommend_log记录
	bool delTalkRecommendLog(unsigned int logid);
 
	//查询用户是否助教
	bool qryUserAssistantType(unsigned int userid, unsigned int liveid);

	//更新统计
	bool updateStatistics(unsigned int userid, unsigned int liveid, unsigned int type, unsigned int staticTime, unsigned int ntotal);
	//根据userid查课程
	int get_CourseByUserid(uint32 userid, std::vector<uint32> &vList);

	//根据课程id查 userid 
	int get_UseridByCourseID(int32_t& userid, const int32_t liveid, const int32_t type);
	 
	//根据课程id查课程是否被关闭
	int get_LiveStatusByID(uint32 &status, uint32 liveid);

	// 根据课程id查已购买或报名该课程的用户id
	int queryCourseStudents(uint32 courseId, std::set<uint32> &studentUserIds);

	// 根据系列课主课程id查已购买或报名该课程的用户id
	int querySerialCourseStudents(uint32 courseId, std::set<uint32> &studentUserIds);

	// 根据课程id查询其所属讲师被哪些用户关注
	int queryFocusUsersByCourseId(uint32 courseId, std::set<uint32> &userIds);

	// 根据课程id查询课程类型等
	int queryCourseTypeAndParentId(const uint32 courseId, int &type, uint32 &parentCourseId);

	// 根据七牛id和课程id查询PPT图片在数据库的id
	int queryPptImageInfo(const char *qiniuId, int courseId, int &dbId, std::string &imgUrl);

	// 根据子课id，查询子课名称等信息
	int querySubCourseInfo(const uint32 subCourseId, uint32 &courseId, uint32 &userId, std::string &subCourseName, std::string &courseName, std::string &beginTime);

	// 根据单节课或系列主课id，查询课程名称等信息
	int queryCourseInfo(const uint32 courseId, uint32 &userId, uint32 &liveId, std::string &courseName, std::string &teacherAlias);

	// 获取直播流总数、获取直播流groupid、更新直播流状态
	int getLiveFlowCount(void);
	int getLiveFlowGroupIds(int startRow, int count, std::vector<int> &groupIds);
	bool updateLiveFlowState(int groupid, int state, time_t updatetime);

	static int m_nPlatId;

private:
	string build_blacklist_sql(int userid, int vcbid, int scopetype, char* szip, char* szmac, char* szdiannao);
	void buildSetValuesList(std::stringstream &sqlValues, const std::map<std::string, std::string> &mValues);

	static std::map<std::string , int> m_mHostPlatid;
	SL_Sync_ThreadMutex threadMutex;
};

template<typename Type>
bool CDBSink::update_userinfo_DB(unsigned int userid, const std::string field, const Type &value)
{
	if (userid == 0 || field.empty())
	{
		LOG_PRINT(log_error, "update_userinfo_DB by userid error,user[%u] field[%s].", userid, field.c_str());
		return false;
	}
	std::string strValue = bitTostring(value);
	std::string sql = stringFormat("update mc_user set %s = '%s' where id = %u ", field.c_str(), value.c_str(), userid);

	return run_sql(sql.c_str());
}

template <class Type>
bool CDBSink::updGroupMemberInfo(uint32 groupid, uint32 userid, const std::string field, Type value)
{
	std::string strValue = bitTostring(value);
	if (!groupid || !userid || field.empty() || strValue.empty())
		return false;

	std::stringstream sql;
	sql << " update mc_group_member set " << field << " = '" << strValue << "'" <<
		" where is_del = 0 " <<
		"   and group_id = " << groupid <<
		"   and user_id = " << userid;

	if (!run_sql(sql.str().c_str()))
		return false;

	return getAffectedRow() > 0;
}

template <class Type>
bool CDBSink::getGroupBasicInfo(uint32 groupid, const std::string field, Type &value)
{
	if (!groupid || field.empty())
		return false;

	std::stringstream sql;
	sql << " select " << field.c_str() << " from talk_course where open_status = 1 and status in (3,4) and id = " << groupid;

	int row = 0;
	Result res;
	if (!run_sql(sql.str().c_str(), &res, &row) || 0 == row)
	{
		CleanRes(&res);
		return false;
	}

	GetFromRes(value, &res, 0, 0);
	CleanRes(&res);
	return true;
}

template <class Type>
bool CDBSink::updGroupBasicInfo(uint32 groupid, const std::string field, Type value)
{
	std::string strValue = bitTostring(value);
	if (!groupid || field.empty() || strValue.empty())
		return false;

	std::stringstream sql;
	sql << " update mc_group set " << field << " = '" << strValue << "'" <<
		" where is_del = 0 and id = " << groupid;

	if (!run_sql(sql.str().c_str()))
		return false;

	return getAffectedRow() > 0;
}

template <class Type>
bool CDBSink::getGroupPluginSetting(uint32 groupid, const std::string field, Type &value)
{
	if (!groupid || field.empty())
		return false;

	std::stringstream sql;
	sql << " select " << field.c_str() << " from mc_group_plugin_setting_detail where group_id = " << groupid;

	int row = 0;
	Result res;
	if (!run_sql(sql.str().c_str(), &res, &row) || 0 == row)
	{
		CleanRes(&res);
		return false;
	}

	GetFromRes(value, &res, 0, 0);
	CleanRes(&res);
	return true;
}

template <class Type>
bool CDBSink::updGroupPluginSetting(uint32 groupid, const std::string field, Type value)
{
	std::string strValue = bitTostring(value);
	if (!groupid || field.empty() || strValue.empty())
		return false;

	std::stringstream sql;
	sql << " update mc_group_plugin_setting_detail set " << field << " = '" << strValue << "'" <<
		" where group_id = " << groupid;

	if (!run_sql(sql.str().c_str()))
		return false;

	return getAffectedRow() > 0;
}

#endif //__DBSINK_H__
