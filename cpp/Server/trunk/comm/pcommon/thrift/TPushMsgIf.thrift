# TPushMsgIf.thrift

include "TCommonIf.thrift"

namespace php Thrift.ThriftStub.RPC_Group_Push

############################################################################

service TPushMsg 
{
	########################################################################
	# function    ：proc_modGroupSettings                                  #
	# description ：修改群设置                                             #
	# input param ：runid   执行者userid                                   #
	#             ：groupid  群id                                          #
	#             ：mSetting  设置key-value                                #
	########################################################################
	i32 proc_modGroupSettings(1: i32 runid, 2: i32 groupid; 3: map<TCommonIf.E_THRIFT_GROUP_SETTING, string> mSetting);

	########################################################################
	# function    ：proc_modVisitSwitch                                    #
	# description ：修改围观限制                                           #
	# input param ：runid   执行者userid                                   #
	#             ：Switch  是否开放围观                                   #
	#             ：kickOnlooker  禁止围观时是否踢出游客                   #
	########################################################################
	i32 proc_modVisitSwitch(1:i32 runid, 2: i32 groupid, 3: bool Switch, 4: bool kickOnlooker),
	
	########################################################################
	# function    ：proc_modJoinGroupMode                                  #
	# description ：修改入群方式                                           #
	# input param ：runid   执行者userid                                   #
	#             ：setting  进群开关设置                                  #
	########################################################################
	i32 proc_modJoinGroupMode(1:i32 runid, 2: i32 groupid, 3: TCommonIf.TJoinGroupCondition maleCondition, 4: TCommonIf.TJoinGroupCondition femaleCondition),

	########################################################################
	# function    ：proc_kickAllOnlookers                                  #
	# description ：踢除所有围观用户                                       #
	# input param ：runid   执行者userid                                   #
	#             ：groupid  群ID                                          #
	########################################################################
	i32 proc_kickAllOnlookers(1:i32 runid, 2:i32 groupid),

	########################################################################
	# function    ：proc_modContributionListSwitch                         #
	# description ：修改贡献榜开关                                         #
	# input param ：runid   执行者userid                                   #
	#             ：groupid  群ID                                          #
	#             ：Switch  开关，false 关闭，true 开启                    #
	########################################################################
	i32 proc_modContributionListSwitch(1:i32 runid, 2:i32 groupid, 3: bool Switch),

	########################################################################
	# function    ：proc_modVoiceChatSwitch                                #
	# description ：修改群语音聊天开关                                     #
	# input param ：runid   执行者userid                                   #
	#             ：groupid  群ID                                          #
	#             ：Switch  开关，false 关闭，true 开启                    #
	#             ：visitOnMic  游客上麦开关，false 关闭，true 开启        #
	########################################################################
	i32 proc_modVoiceChatSwitch(1:i32 runid, 2:i32 groupid, 3:bool Switch, 4: bool visitOnMic = false),

	########################################################################
	# function    ：proc_modVideoAuthSwitch                                #
	# description ：修改群视频认证开关                                     #
	# input param ：runid   执行者userid                                   #
	#             ：groupid  群ID                                          #
	#             ：Switch  开关，false 关闭，true 开启                    #
	########################################################################
	i32 proc_modVideoAuthSwitch(1:i32 runid, 2:i32 groupid, 3:bool Switch),

	########################################################################
	# function    ：proc_modGroupGainSetting                               #
	# description ：修改群收入分成比例                                     #
	# input param ：runid   执行者userid                                   #
	#             ：groupid  群ID                                          #
	#             ：Switch  开关，false 关闭，true 开启                    #
	#             ：rangeType 收入分成范围，0 游客，1 仅正式成员，2 所有人 #
	#             ：percent  分成比例                                      #
	########################################################################
	i32 proc_modGroupGainSetting(1:i32 runid, 2:i32 groupid, 3:bool Switch, 4:byte rangeType, 5: byte percent),

	########################################################################
	# function    ：proc_modGroupUserGainSetting                           #
	# description ：修改群红包分成比例                                     #
	# input param ：runid   执行者userid                                   #
	#             ：groupid  群ID                                          #
	#             ：userid  用户ID                                         #
	#             ：percent  分成比例                                      #
	########################################################################
	i32 proc_modGroupUserGainSetting(1:i32 runid, 2:i32 groupid, 3:i32 userid, 4:byte percent),

	########################################################################
	# function    ：proc_modGroupUserRole                                  #
	# description ：调整成员等级                                           #
	# input param ：runid   执行者userid                                   #
	#             ：groupid  群ID                                          #
	#             ：userid  用户ID                                         #
	#             ：roleType  角色类型                                     #
	########################################################################
	i32 proc_modGroupUserRole(1:i32 runid, 2:i32 groupid, 3:i32 userid, 4:i32 roleType),

	########################################################################
	# function    ：proc_modGroupBlackList                                 #
	# description ：修改黑名单列表                                         #
	# input param ：runid   执行者userid                                   #
	#             ：groupid  群ID                                          #
	#             ：userid  用户ID                                         #
	#             ：isAdd  是否加入黑名单, false 否, true 是               #
	########################################################################
	i32 proc_modGroupBlackList(1:i32 runid, 2:i32 groupid, 3:i32 userid, 4:bool isAdd),
	
	########################################################################
	# function     ：proc_getGroupVisitorLst                               #
	# description  ：获取群游客列表                                        #
	# input param  ：groupid  群ID                                         #
	# output param ：TUserGroupInfo  游客列表                              #
	########################################################################
	list<TCommonIf.TUserGroupInfo> proc_getGroupVisitorLst(1:i32 groupid),
	
	########################################################################
	# function    ：proc_reloadUserInfo                                    #
	# description :刷新用户基本信息                                        #
	# input param ：userid   用户ID                                        #
	########################################################################
	i32 proc_reloadUserInfo(1:i32 userid),

	########################################################################
	# function    ：proc_sendSysNoticeMsg                                  #
	# description ：发送系统消息                                           #
	# input param ：msg   	系统消息详情                                   #
	# input param ：lstCondition   发送条件                                #
	########################################################################
	i32 proc_sendSysNoticeMsg(1: TCommonIf.TNoticeMsg msg, 2: list<TCommonIf.TCondition> lstCondition, 3: bool onlineOnly = false),
	
	########################################################################
	# function    ：proc_sendUserNoticeMsg                                 #
	# description ：发送用户系统通知                                       #
	# input param ：userid  用户ID                                         #
	# input param ：msg   	系统消息详情                                   #
	########################################################################
	i32 proc_sendUserNoticeMsg(1: i32 userid, 2: TCommonIf.TNoticeMsg msg),
	
	########################################################################
	# function    ：proc_sendAdminVideoAuditMsg                            #
	# description ：用户申请视频审核通知                                   #
	# input param ：userid  用户ID                                         #
	# input param ：groupid	群ID                                           #
	# input param ：url   	url                                            #
	########################################################################
	i32 proc_sendAdminVideoAuditMsg(1: i32 userid, 2: i32 groupid, 3: string url),
	
	########################################################################
	# function    ：proc_handleVideoAuditMsg                               #
	# description ：处理用户视频审核通知                                   #
	# input param ：handle_userid  处理用户ID                              #
	# input param ：groupid  群ID                                          #
	# input param ：msgid   需审批的群助手消息ID                           #
	# input param ：agree  	同意为true,拒绝为false                         #
	########################################################################
	bool proc_handleVideoAuditMsg(1: i32 handle_userid, 2: i32 groupid, 3: i32 msgid, 4: bool agree, 5: i32 apply_userid),
	
	########################################################################
	# function    ：proc_loadGroup                                          #
	# description ：加载群信息                                              #
	# input param ：groupid   发送条件                                      #
	########################################################################
	i32 proc_loadGroup(1:i32 groupid, 2: string host),
	
	########################################################################
	# function    ：proc_modGroup                                          #
	# description ：加载群信息                                             #
	# input param ：groupid   发送条件                                     #
	########################################################################
	i32 proc_modGroup(1:i32 groupid, 2: string host),
	
	########################################################################
	# function    ：proc_setGroupOwner                                     #
	# description ：设置群主                                               #
	# input param ：groupid   群ID                                         #
	# input param ：new_ownerid   新的群主ID                               #
	########################################################################
	bool proc_setGroupOwner(1:i32 groupid, 2:i32 new_ownerid),

	########################################################################
	# function    ：proc_queryInGroupList                                  #
	# description ：查询当前在群列表                                       #
	# input param ：userid   查询者ID                                      #
	# input param ：groupid  群ID                                          #
	########################################################################
	map<TCommonIf.e_INGROUP_TYPE, list<i32> > proc_queryInGroupList(1:i32 userid, 2:i32 groupid),

	########################################################################
	# function    ：proc_joingroup                                         #
	# description ：进群请求                                               #
	# input param ：oReq   进群请求结构体                                  #
	########################################################################
	i32 proc_joingroup(1: TCommonIf.TJoinGroup oReq),
}
