# TRoomSvrIf.thrift

include "TCommonIf.thrift"

/**
 * The first thing to know about are types. The available types in Thrift are:
 *
 *  bool        Boolean, one byte
 *  byte        Signed byte
 *  i16         Signed 16-bit integer
 *  i32         Signed 32-bit integer
 *  i64         Signed 64-bit integer
 *  double      64-bit floating point value
 *  string      String
 *  binary      Blob (byte array)
 *  map<t1,t2>  Map from one type to another
 *  list<t1>    Ordered list of one type
 *  set<t1>     Set of unique elements of one type
 *
 * Did you also notice that Thrift supports C style comments?
 */

############################################################################

service TRoomSvr 
{
	#加群请求
	i32 proc_joingroup(1: TCommonIf.TJoinGroup oReq),
	#踢人
	i32 proc_blockUserInGroup(1: i32 runid, 2: i32 userid, 3: i32 groupid, 4: i32 reasonid, 5: i32 kickout_min),
	#加群主请求
	bool proc_setGroupOwner(1: i32 groupid, 2: i32 ownerid),
	#加载群信息
	bool proc_loadGroup(1: i32 groupid, 2: string host),
	#刷新群信息
	bool proc_modGroup(1: i32 groupid, 2: string host),


	#修改群设置
	i32 proc_modGroupSettings(1: i32 runid, 2: i32 groupid; 3: map<TCommonIf.E_THRIFT_GROUP_SETTING, string> mSetting);
	#修改围观限制
	i32 proc_modVisitSwitch(1:i32 runid, 2: i32 groupid, 3: bool Switch, 4: bool kickOnlooker),
	#修改入群方式
	i32 proc_modJoinGroupMode(1:i32 runid, 2: i32 groupid, 3: TCommonIf.TJoinGroupCondition maleCondition, 4: TCommonIf.TJoinGroupCondition femaleCondition),
	#踢除所有围观用户
	i32 proc_kickAllOnlookers(1:i32 runid, 2:i32 groupid),
	#修改群语音聊天开关
	i32 proc_modVoiceChatSwitch(1:i32 runid, 2:i32 groupid, 3:bool Switch, 4: bool visitOnMic = false),
	#修改群视频认证开关
	i32 proc_modVideoAuthSwitch(1:i32 runid, 2:i32 groupid, 3:bool Switch),
	#调整成员等级
	i32 proc_modGroupUserRole(1:i32 runid, 2:i32 groupid, 3:i32 userid, 4:i32 roleType),
	#获取游客列表
	list<TCommonIf.TUserGroupInfo> proc_getGroupVisitorLst(1:i32 groupid),
	#获取当前在群用户列表
	map<TCommonIf.e_INGROUP_TYPE, list<i32> > proc_queryInGroupList(1:i32 userid, 2:i32 groupid),
	#
	i32 proc_give_tip(1:i32 srcid,2:i32 toid,3:i32 groupid,4:string gifname,5:i32 tiptype,6:string srcname,7:string toname,8:string gifpicture),
	#
	i32 proc_send_link(1:i32 groupid,2:string title,3:string source,4:string summary,5:i32 bTip,6:i32 amount,7:i32 pointId,8:string head_add,9:i32 recommendid,10:i32 user_id),#bTip = 0 and amount = 0
	i32 proc_send_notice(1:string notice),
	i32 proc_send_course(1:i32 groupid,2:string title,3:string source,4:string summary,5:i32 bTip,6:i32 amount,7:i32 courseId,8:string head_add,9:string teachername,10:string srcimg,11:i32 recommendid,12:i32 user_id),
	i32 proc_reloadUserInfo(1:i32 userid),
	i32 proc_reloadRoomVirtualNumInfo(1:i32 liveid, 2:i32 numbers ),
	
	#type 1,直播间，2用户所有课程，3课程
	i32 proc_CloseLiveAndCourse(1:i32 runuserid, 2:i32 liveid, 3:i32 status, 4:i32 type ),

	#直播开始
	i32 proc_liveStart(1:string streamId ),
	#直播结束
	i32 proc_liveStop(1:string streamId ),
	# 系列课新子课通知（20171212：该接口可由proc_sendNewCourseNotice()代替或调用，视具体需要而定）
	# 注：除subCourseId是必传之外，其它字段的值根据具体情况，可能由PHP传，可能由C++自行查数据库
	i32 proc_sendNewSubCourseNotice(1:i32 courseId, 2:i32 subCourseId, 3:i32 userId, 4:string courseName, 5:string subCourseName, 6:string beginTime),
	# 发布新的单节课、系列课或系列课子课通知
	# isSubCourse为true时，courseId传子课ID，否则传单节课或系列课ID
	# 注：除courseId是必传之后，其它字段的值根据具体情况，可能由PHP传，可能由C++自行查数据库
	i32 proc_sendNewCourseNotice(1:i32 courseId, 2:bool isSubCourse),
}
