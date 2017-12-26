#ifndef __ROOMSVR_THRIFT_HANDLER_H__
#define __ROOMSVR_THRIFT_HANDLER_H__

#include "TRoomSvr.h"
#include "RoomSvr.pb.h"

class TRoomSvrHandler : public TRoomSvrIf
{
public:
	TRoomSvrHandler();
	~TRoomSvrHandler();

	virtual int32_t proc_joingroup(const TJoinGroup & oReq);
	virtual int32_t proc_blockUserInGroup(const int32_t runid, const int32_t userid, const int32_t groupid, const int32_t reasonid, const int32_t kickout_min);
	virtual bool proc_setGroupOwner(const int32_t groupid, const int32_t ownerid);
	virtual bool proc_loadGroup(const int32_t groupid, const std::string& host);
	virtual bool proc_modGroup(const int32_t groupid, const std::string& host);
	virtual int32_t proc_modGroupSettings(const int32_t runid, const int32_t groupid, const std::map< ::E_THRIFT_GROUP_SETTING::type, std::string> & mSetting);
	virtual int32_t proc_modVisitSwitch(const int32_t runid, const int32_t groupid, const bool Switch, const bool kickOnlooker);
	virtual int32_t proc_modJoinGroupMode(const int32_t runid, const int32_t groupid, const  ::TJoinGroupCondition& maleCondition, const  ::TJoinGroupCondition& femaleCondition);
	virtual int32_t proc_kickAllOnlookers(const int32_t runid, const int32_t groupid);
	virtual int32_t proc_modVoiceChatSwitch(const int32_t runid, const int32_t groupid, const bool Switch, const bool visitOnMic);
	virtual int32_t proc_modVideoAuthSwitch(const int32_t runid, const int32_t groupid, const bool Switch);
	virtual int32_t proc_modGroupUserRole(const int32_t runid, const int32_t groupid, const int32_t userid, const int32_t roleType);
	virtual void proc_getGroupVisitorLst(std::vector< ::TUserGroupInfo> & _return, const int32_t groupid);
	virtual void proc_queryInGroupList(std::map< ::e_INGROUP_TYPE::type, std::vector<int32_t> > & _return, const int32_t userid, const int32_t groupid);
	//送礼
	virtual int32_t proc_give_tip(const int32_t srcid, const int32_t toid, const int32_t groupid, const std::string& gifname, const int32_t tiptype,
		const std::string& srcname, const std::string& toname, const std::string& gifpicture);
	//发送观点
	virtual int32_t proc_send_link(const int32_t groupid, const std::string& title, const std::string& source, const std::string& summary, const int32_t bTip,
		const int32_t amount, const int32_t pointId, const std::string& head_add, const int32_t recommendid, const int32_t user_id);
	//公告
	virtual int32_t proc_send_notice(const std::string& notice);
	//发送课程
	virtual int32_t proc_send_course(const int32_t groupid, const std::string& title, const std::string& source, const std::string& summary, const int32_t bTip,
		const int32_t amount, const int32_t courseId, const std::string& head_add, const std::string& teachername, const std::string& srcimg, const int32_t recommendid, const int32_t user_id);
	virtual int32_t proc_reloadUserInfo(const int32_t userid);
	virtual int32_t proc_reloadRoomVirtualNumInfo(const int32_t liveid, const int32_t numbers);

	virtual int32_t proc_CloseLiveAndCourse(const int32_t runuserid, const int32_t liveid, const int32_t status, const int32_t type);

	virtual int32_t proc_liveStart(const std::string& streamID);
	virtual int32_t proc_liveStop(const std::string& streamID);

	virtual int32_t proc_sendNewSubCourseNotice(const int32_t courseId, const int32_t subCourseId, const int32_t userId, const std::string& courseName, const std::string& subCourseName, const std::string& beginTime);
	virtual int32_t proc_sendNewCourseNotice(const int32_t courseId, const bool isSubCourse);
private:
	bool checkJoinGroupConditionValid(const TJoinGroupCondition &condition);
	void setSettingStat(CMDSettingStat &Stat, e_SettingType type, const int32_t stat){Stat.set_type(type); Stat.set_stat(stat);}
};

#endif /* __ROOMSVR_THRIFT_HANDLER_H__ */
