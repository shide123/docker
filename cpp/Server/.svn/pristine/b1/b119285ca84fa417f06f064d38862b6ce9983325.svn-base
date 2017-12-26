
#ifndef __MESSAGE_VCHAT_HH_20130715__
#define __MESSAGE_VCHAT_HH_20130715__

#include "message_comm.h"

#define MDM_Vchat_Login  103  //登陆
#define MDM_Vchat_Hall   104  //大厅
#define MDM_Vchat_Room   105  //房间
#define MDM_Vchat_Gate   106  //网关
#define MDM_Vchat_WEB    107  //来自web的通知
#define MDM_Vchat_Text   108   //文字直播服务器
#define MDM_Vchat_Usermgr	109		//来自用户管理服务器
#define MDM_Vchat_Frtiy		110		//外部 111
#define MDM_Vchat_Trust		112
#define MDM_Vchat_Notify	113		//房间通知服务器
#define MDM_Vchat_Subscrib	114		//订阅服务器
#define MDM_Vchat_Redis_Sync	115	//redis同步
#define MDM_Vchat_Bridge	116		//桥接服务器
#define MDM_Vchat_Timesvr	117		//timesvr服务器
#define MDM_Vchat_Stastics	198		//统计服务器
#define MDM_Vchat_Alarm		199		//告警服务器
#define MDM_Vchat_Nodemgr	200		//节点服务器
#define MDM_Vchat_GoldTrade	201		//黄金交易服务器
#define MDM_Vchat_MediaMGR	300		//媒体数据服务器
#define MDM_Stk_DCL			301		//行情大策略
#define MDM_Version_Value 10	//协议版本
#define CHECKCODE 0

#define  MDM_GR_ShouQuan 10
#define  Sub_GR_clientHello 200
#define  Sub_GR_ServerAuthReq 3
#define  Sub_GR_ServerAuthResp 4
#define  TM_SERVERAUTH_RESP 0x0400+1

#define UsermgrPingRoom 1299921
#define WebSocketPingRoom 1299922
#define BridgePingServer 1299923


enum {
	ERR_SYNC_NOT_READY		= 1000  //redis数据同步服务没有准备好
};

enum {
	Sub_Vchat_ClientHello                               =1    ,       //握手每个MDMmainCmd不一样
	Sub_Vchat_ClientPing                                =2    ,       //ping每个MDMmainCmd不一样
	Sub_Vchat_ClientPingResp                            =3    ,       //ping回应
	Sub_Vchat_GetAllUserReq                             =4    ,       //获取所有的在线用户
	Sub_Vchat_Resp_ErrCode                              =5    ,       //返回错误应答
	Sub_Vchat_DoNotReachRoomServer                      =6    ,       //
	Sub_Vchat_MgrRefreshListReq                         =7    ,       //刷新请求
	Sub_Vchat_MgrRefreshListNotify                      =8    ,       //
	Sub_Vchat_MgrRelieveBlackDBReq                      =9    ,       //解除黑名单请求
	Sub_Vchat_MgrRelieveBlackDBNoty                     =10   ,       //
	Sub_Vchat_ClientHelloResp							=11	  ,		  //Sub_Vchat_ClientHello回应
	Sub_Vchat_TransKeyReq								=12	  ,       //客户端获取传输密码请求
	Sub_Vchat_TransKeyResp                              =13	  ,       //服务器给客户端返回传输密码

	//usermgrsvr 300-699
	Sub_Vchat_LogonNot                                  =361  ,       // 用户上线通知
	Sub_Vchat_LogoutNot                                 =362  ,       // 用户下线通知
	Sub_Vchat_ClientExistNot                            =363  ,       // 踢人用户下线通知--给客户端
	Sub_Vchat_UserGroupLstResp							=364  ,       // 用户所在的组响应
	Sub_Vchat_UserModInfoReq							=365  ,       // 修改用户信息
	Sub_Vchat_UserModInfoResp							=366  ,       // 用户修改信息响应
	Sub_Vchat_OperFriendshipReq							=367  ,       // 用户关系操作
	Sub_Vchat_OperFriendshipResp						=368  ,		  // 用户关系操作反馈
	Sub_Vchat_QryUserAttentionList						=369  ,		  // 查询用户关注列表
	Sub_Vchat_UserAttentionList							=370  ,		  // 用户关注列表
	Sub_Vchat_UserModInfoNotify							=371  ,       // 群成员信息更新通知
	Sub_Vchat_UserSysConfigNotify						=372  ,       // 系统配置通知
	Sub_Vchat_MicStateTransNoty							=373  ,       // mic状态转移
	Sub_Vchat_UserLocationInfo							=374  ,
	Sub_Vchat_UserLocationInfoResp						=375  ,

	Sub_Vchat_ClientNotify                              =10194,       // 推送命令
	Sub_Vchat_notifyReq                                 =10197,       // 通告请求

	//房间服务协议段 2000-2999
	Sub_Vchat_JoinRoomReq                               =2000 ,       // 加入房间请求
	Sub_Vchat_JoinRoomErr                               =2001 ,       // 错误
	Sub_Vchat_JoinRoomResp                              =2002 ,       // 成功含管理列表
	Sub_Vchat_JoinOtherRoomNoty                         =2003 ,       // 进入其他房间通知(服务器之间用)
	Sub_Vchat_RoomUserListReq                           =2004 ,       // 房间用户列表请求
	Sub_Vchat_RoomUserListBegin                         =2005 ,       // 用户列表开始
	Sub_Vchat_RoomUserListResp                          =2006 ,       // 用户列表
	Sub_Vchat_RoomUserListFinished                      =2007 ,       // 用户列表结束
	Sub_Vchat_RoomUserNoty                              =2008 ,       // 列表更新通知
	Sub_Vchat_RoomPubMicState                           =2009 ,       // 房间公麦状态
	Sub_Vchat_RoomUserExitReq                           =2010 ,       // 用户自己退出房间
	Sub_Vchat_RoomUserExitResp                          =2011 ,       //
	Sub_Vchat_RoomUserExitNoty                          =2012 ,       // 通知
	Sub_Vchat_RoomKickoutUserReq                        =2013 ,       // 踢出用户请求
	Sub_Vchat_RoomKickoutUserResp                       =2014 ,       // 踢出用户反馈
	Sub_Vchat_RoomKickoutUserNoty                       =2015 ,       // 通知
	Sub_Vchat_ChatReq                                   =2016 ,       // 聊天发出消息
	Sub_Vchat_ChatErr                                   =2017 ,       //
	Sub_Vchat_ChatNotify                                =2018 ,       // 转发消息
	Sub_Vchat_TradeGiftReq                              =2019 ,       // 赠送礼物(跨房间跨平台)请求
	Sub_Vchat_TradeGiftResp                             =2020 ,       //
	Sub_Vchat_TradeGiftErr                              =2021 ,       //
	Sub_Vchat_TradeGiftNotify                           =2022 ,       //
	Sub_Vchat_SysNoticeInfo                             =2023 ,       // 系统广播(专门)消息
	Sub_Vchat_UserAccountInfo                           =2024 ,       // 用户账户(余额)信息
	Sub_Vchat_RoomInfoNotify                            =2025 ,       // 房间信息资料
	Sub_Vchat_RoomManagerNotify                         =2026 ,       // 房间管理员
	Sub_Vchat_RoomMediaNotify                           =2027 ,       // 房间媒体
	Sub_Vchat_RoomNoticeNotify                          =2028 ,       // 房间公告
	Sub_Vchat_RoomOPStatusNotify                        =2029 ,       // 房间状态
	Sub_Vchat_SetMicStateReq                            =2030 ,       // 设置上/下麦状态
	Sub_Vchat_SetMicStateResp                           =2031 ,       //
	Sub_Vchat_SetMicStateErr                            =2032 ,       //
	Sub_Vchat_SetMicStateNotify                         =2033 ,       //
	Sub_Vchat_SetDevStateReq                            =2034 ,       // 设置设备状态
	Sub_Vchat_SetDevStateResp                           =2035 ,       //
	Sub_Vchat_SetDevStateErr                            =2036 ,       //
	Sub_Vchat_SetDevStateNotify                         =2037 ,       //
	Sub_Vchat_SetUserAliasReq                           =2038 ,       // 设置用户呢称
	Sub_Vchat_SetUserAliasResp                          =2039 ,       //
	Sub_Vchat_SetUserAliasErr                           =2040 ,       //
	Sub_Vchat_SetUserAliasNotify                        =2041 ,       //
	Sub_Vchat_SetUserPriorityReq                        =2042 ,       // 设置用户权限
	Sub_Vchat_SetUserPriorityResp                       =2043 ,       //
	Sub_Vchat_SetUserPriorityNotify                     =2044 ,       //
	Sub_Vchat_SeeUserIpReq                              =2045 ,       // 查看用户IP请求
	Sub_Vchat_SeeUserIpResp                             =2046 ,       //
	Sub_Vchat_SeeUserIpErr                              =2047 ,       //
	Sub_Vchat_ThrowUserReq                              =2048 ,       // 封杀用户
	Sub_Vchat_ThrowUserResp                             =2049 ,       //
	Sub_Vchat_ThrowUserNotify                           =2050 ,       //
	Sub_Vchat_ForbidUserChatReq                         =2051 ,       // 禁言用户请求
	Sub_Vchat_ForbidUserChatNotify                      =2052 ,       //
	Sub_Vchat_FavoriteVcbReq                            =2053 ,       //  收藏房间请求
	Sub_Vchat_FavoriteVcbResp                           =2054 ,       //
	Sub_Vchat_SetRoomInfoReq                            =2055 ,       //  设置房间信息请求
	Sub_Vchat_SetRoomInfoResp                           =2056 ,       //  设置房间信息反馈
	Sub_Vchat_SetRoomOPStatusReq                        =2057 ,       //  设置房间运行属性(状态)
	Sub_Vchat_SetRoomOPStatusResp                       =2058 ,       //
	Sub_Vchat_SetRoomNoticeReq                          =2059 ,       //  设置房间公告信息请求
	Sub_Vchat_SetRoomNoticeResp                         =2060 ,       //
	Sub_Vchat_SetRoomMediaReq                           =2061 ,       //  设置房间媒体服务器请求addbyguchengzhi20150202
	Sub_Vchat_QueryUserAccountReq                       =2064 ,       //  用户账户查询(银行查询)
	Sub_Vchat_QueryUserAccountResp                      =2065 ,       //
	Sub_Vchat_QueryVcbExistReq                          =2066 ,       //  查询某房间是否存在
	Sub_Vchat_QueryVcbExistResp                         =2067 ,       //  该消息没有err
	Sub_Vchat_QueryUserExistReq                         =2068 ,       //  查询某用户是否存在
	Sub_Vchat_QueryUserExistResp                        =2069 ,       //  该消息没有err
	Sub_Vchat_GateCloseObjectReq                        =2070 ,       //  关闭网关上面的对象
	Sub_Vchat_CloseRoomNotify                           =2071 ,       //  关闭房间通知
	Sub_Vchat_RoomGatePing                              =2072 ,       //
	Sub_Vchat_SetRoomInfoReq_v2                         =2073 ,       //  设置房间信息请求飞叉版
	Sub_Vchat_SetRoomInfoResp_v2                        =2074 ,       //
	Sub_Vchat_SetRoomInfoNoty_v2                        =2075 ,       //
	Sub_Vchat_QueryRoomGateAddrReq                      =2076 ,       //  获取房间网关地址
	Sub_Vchat_QueryRoomGateAddrResp                     =2077 ,       //
	Sub_Vchat_AdKeyWordOperateReq                       =2078 ,       //   关键字操作请求
	Sub_Vchat_AdKeyWordOperateResp                      =2079 ,       //  关键字操作回应
	Sub_Vchat_AdKeyWordOperateNoty                      =2080 ,       //   关键字广播通知
	Sub_Vchat_TeacherScoreReq                           =2081 ,       //  讲师发送评分处理请求
	Sub_Vchat_TeacherScoreResp                          =2082 ,       //  讲师发送评分处理响应
	Sub_Vchat_TeacherScoreRecordReq                     =2083 ,       //  用户评分请求
	Sub_Vchat_TeacherScoreRecordResp                    =2084 ,       //  用户评分响应
	Sub_Vchat_RoborTeacherIdNoty                        =2085 ,       //  机器人对应讲师ID通知
	Sub_Vchat_TeacherGiftListReq                        =2086 ,       //  讲师忠实度周版请求
	Sub_Vchat_TeacherGiftListResp                       =2087 ,       //  讲师忠实度周版响应
	Sub_Vchat_ReportMediaGateReq                        =2088 ,       //  客户端报告媒体服务器和网关服务器
	Sub_Vchat_ReportMediaGateResp                       =2089 ,       //  客户端报告媒体服务器和网关服务器的回应
	Sub_Vchat_RoomUserExceptExitReq                     =2090 ,       //  用户异常退出,gateway通知roomsvr
	Sub_Vchat_UserScoreNotify                           =2091 ,       //  用户对讲师的评分
	Sub_Vchat_UserScoreListNotify                       =2092 ,       //  用户对讲师的评分广播
	Sub_Vchat_RoomAndSubRoomId_Noty                     =2093 ,       //  IOS和Android方面，需要在进入房间和讲师上麦的时候得到主房间和子房间的ID
	Sub_Vchat_TeacherAvarageScore_Noty                  =2094 ,       //  某个讲师的平均分
	Sub_Vchat_SysCast_Resp                              =2095 ,       //  房间发送系统公告
	Sub_Vchat_RoomUserExceptExitNoty                    =2096 ,       //  用户异常退出通知
	Sub_Vchat_GateJoinRoom                              =2097 ,       //  gate切换用户到roomsvr
	Sub_Vchat_RoomAmong_Notify                          =2098 ,       //  roomsvr之间的消息通知
	Sub_Vchat_RoomTeacherSubscriptionReq                =2099 ,       //  订阅请求
	Sub_Vchat_RoomTeacherSubscriptionResp               =2100 ,       //  订阅响应
	Sub_Vchat_RoomTeacherSubscriptionStateQueryReq      =2101 ,       //  查询订阅状态请求
	Sub_Vchat_RoomTeacherSubscriptionStateQueryResp     =2102 ,       //  查询订阅状态响应
	Sub_Vchat_RoomDataSyncReq                           =2103 ,       //
	Sub_Vchat_RoomDataSyncResp                          =2104 ,       //
	Sub_Vchat_RoomDataSyncComplete                      =2105 ,       //
	Sub_Vchat_RoomDataSync_JoinRoom                     =2106 ,       //
	Sub_Vchat_RoomDataSync_LeftRoom                     =2107 ,       //
	Sub_Vchat_RoomDataSync_MicChanged                   =2108 ,       //
	Sub_Vchat_RoomDataSync_UserUpdated                  =2109 ,       //
	Sub_Vchat_RedisRoomInfo_Req                         =2110 ,       //
	Sub_Vchat_RedisRoomInfo_Resp                        =2111 ,       //
	Sub_Vchat_RedisUserInfo_Req                         =2112 ,       //
	Sub_Vchat_RedisUserInfo_Last_Req                    =2113 ,       //
	Sub_Vchat_RedisUserInfo_Resp                        =2114 ,       //
	Sub_Vchat_RedisUserInfo_Complete                    =2115 ,       //
	Sub_Vchat_RedisSync_Err_Resp                        =2116 ,       //
	Sub_Vchat_RedisSync_log_index_Req                   =2117 ,       //
	Sub_Vchat_RedisSync_log_index_Resp                  =2118 ,       //
	Sub_Vchat_RedisSync_data_Req                        =2119 ,       //
	Sub_Vchat_RedisSync_data_Resp                       =2120 ,       //
	Sub_Vchat_RedisSync_data_Complete                   =2121 ,       //
	Sub_Vchat_RedisSync_Report_Status                   =2122 ,       //
	Sub_Vchat_ChatResp                                  =2123 ,       //聊天发出消息应答
	Sub_Vchat_PreJoinRoomReq                            =2124,        //加入房间预处理请求
	Sub_Vchat_PreJoinRoomResp                           =2125,        //加入房间预处理响应
	Sub_Vchat_WaitiMicListInfo                          =2126 ,       //排麦用户列表
	Sub_Vchat_GetUserInfoReq                            =2127,        //查询用户个人资料请求
	Sub_Vchat_GetUserInfoResp                           =2128,        //查询用户个人资料响应
	Sub_Vchat_GetTeacherInfoResp                        =2129,        //查询文字直播讲师个人资料响应
	Sub_Vchat_GetUserInfoErr                            =2130,        //查询用户个人资料请求
    Sub_Vchat_UpWaitMicReq                              =2131,        //
    Sub_Vchat_UpWaitMicResp                             =2132,
    Sub_Vchat_UpWaitMicErr                              =2133,
	Sub_Vchat_TeamTopNReq                               =2134,	      //最强战队周榜请求
	Sub_Vchat_TeamTopNResp                              =2135,	      //最强战队周榜回应
	Sub_Vchat_AfterJoinRoomReq                          =2136,	      //加入房间成功后请求推送信息
    Sub_Vchat_ViewpointTradeGiftReq                     =2137,        //观点送礼请求
    Sub_Vchat_ViewpointTradeGiftNoty                    =2138,        //观点送礼房间广播
    Sub_Vchat_ViewpointTradeGiftResp                    =2139,        //观点送礼响应
    Sub_Vchat_AskQuestionReq                            =2140,        //提问请求
    Sub_Vchat_AskQuestionResp                           =2141,        //提问响应
	Sub_Vchat_GateJoinRoomResp							=2142,		  //rejoin response
	Sub_Vchat_RedisRoomMgrInfo_Req                      =2143,		  //房间管理信息同步请求
	Sub_Vchat_RedisRoomMgrInfo_Resp                     =2144,		  //房间管理信息同步响应
    Sub_Vchat_TeacherGiftListResp_v2                    =2145,        //讲师忠实度周版响应(新版)
    Sub_Vchat_HistoricalRecordNoty                      =2146,        //历史战绩
    Sub_Vchat_GetAskOpportunityReq                      =2147,        //领取提问次数
	Sub_Vchat_VipUserNoty                               =2148,        //用户VIP信息
	Sub_Vchat_QuestionAnswerNoty						=2149,		  //推送问答列表
	Sub_Vchat_RoomNoticeInfoNotify                      =2150,        //房间公告信息
    Sub_Vchat_GetRoomAssistantReq                       =2151,        //查询房间助理请求
	Sub_Vchat_GetRoomAssistantResp                      =2152,        //查询房间助理响应
	Sub_Vchat_PrivateMsgStart                           =2153,        //私聊开始
	Sub_Vchat_PrivateMsgNoty                            =2154,        //私聊消息
	Sub_Vchat_GetPtCourseNoty							=2155,		  //查询白金课程信息返回
	Sub_Vchat_NonLiveTimeNotify							=2156,		  //房间非直播时间通知
	Sub_Vchat_PotentialVIPNoty							=2157,		  //潜在VIP通知
	Sub_Vchat_VipPresenceNotify							=2160,		  //VIP进场通知
	Sub_Vchat_PtCourseStateNoty							=2161,        //白金课程状态变更通知
	Sub_Vchat_NewPtCourseNoty							=2162,        //有新的白金课程通知
	Sub_Vchat_NewPtCourseReq							=2163,        //请求新的白金课程信息
	Sub_Vchat_VipZoneNotify								=2164,        //VIP专区推送提醒
	Sub_Vchat_MaxClientNum								=2165,        //当天最大用户数
	Sub_Vchat_VIPCourseStateNoty						=2166,		  //VIP课程状态变更通知

	Sub_Vchat_ChangeWaitMicIndexReq						=2167,        //设置排麦麦序
	Sub_Vchat_ChangeWaitMicIndexResp         			=2168,		  //
	Sub_Vchat_ChangeWaitMicIndexNotify					=2169,

	Sub_Vchat_GroupMemberReq							=2170,		  //用户请求群成员列表
	Sub_Vchat_QryUserGroupInfoReq						=2174,		  //用户群信息请求
	Sub_Vchat_QryUserGroupInfoResp						=2175,		  //用户群信息响应
	Sub_Vchat_SetGroupMsgMuteReq						=2176,		  //设置群消息免打扰
	Sub_Vchat_SetGroupMsgMuteResp						=2177,		  //设置群消息免打扰响应
	Sub_Vchat_QuitGroupReq								=2178,		  //用户退群请求
	Sub_Vchat_QuitGroupResp								=2179,		  //用户退群响应
	Sub_Vchat_QryGroupVisitCountReq						=2180,		  //查询群里观众数请求
	Sub_Vchat_QryGroupVisitCountResp					=2181,		  //查询群里观众数响应
	Sub_Vchat_JoinGroupReq								=2182,		  //申请加入群请求
	Sub_Vchat_JoinGroupResp								=2183,		  //申请加入群响应
	Sub_Vchat_ForbidJoinGroupKillVisitor				=2184,		  //禁止加群踢出游客提示
	Sub_Vchat_GroupSettingStatNotify					=2185,		  //房间设置状态通知

	Sub_Vchat_MicStateListReq							=2186,			//maixu req
	Sub_Vchat_MicStateListResp							=2187,			//maixu resp
	Sub_Vchat_SetMicModeReq								=2188,
	Sub_Vchat_SetMicModeResp							=2189,
	Sub_Vchat_SetMicModeNoty							=2190,
	Sub_Vchat_SetMicPosStatusReq						=2191,
	Sub_Vchat_SetMicPosStatusResp						=2192,
	Sub_Vchat_SetMicPosStatusNoty						=2193,
	Sub_Vchat_GetRoomInfoExtReq							=2194,
	Sub_Vchat_getRoomInfoExtResp						=2195,
	Sub_Vchat_GroupSettingStatReq						=2196,		  //房间相关设置状态请求
	Sub_Vchat_InviteOnMicReq							=2197,
	Sub_Vchat_GatewayKickoutUserReq                     =2198,		  //gateway检测链接超时后踢出用户
	Sub_Vchat_QryJoinGroupCondReq						=2199,		  //查询加群条件请求
	Sub_Vchat_QryJoinGroupCondResp						=2200,		  //查询加群条件反馈
	Sub_Vchat_ApplyJoinGroupAuthReq						=2201,		  //用户申请进群留言验证请求
	Sub_Vchat_ApplyJoinGroupAuthResp					=2202,		  //用户申请进群留言验证反馈
	Sub_Vchat_HandleJoinGroupAuthReq					=2203,		  //管理员审批进群验证请求
	Sub_Vchat_HandleJoinGroupAuthResp					=2204,		  //管理员审批进群验证反馈
	Sub_Vchat_GroupUserSettingReq						=2205,		  //个人星球设置
	Sub_Vchat_GroupUserSettingResp						=2206,		  //个人星球设置响应
	Sub_Vchat_InviteOnMicNoty							=2208,
	Sub_Vchat_UserAuthStateNoty							=2209,		  //成员认证状态变更
    
	Sub_Vchat_UserAuthReq								= 2210,//
	Sub_Vchat_UserAuthResp								= 2211,//

	Sub_Vchat_GroupOnlineMemberList						= 2212,		//星成员在星列表
	Sub_Vchat_MemberEnterGroup							= 2213,		//星成员进星通知
	Sub_Vchat_MemberExitGroup							= 2214,		//星成员离开通知

	Sub_Vchat_ForbidUserChat							= 2215,//禁言请求
	Sub_Vchat_ForbidUserChatNoty						= 2216,//禁言通知
	
	Sub_Vchat_CourseFinishReq							= 2217,
	Sub_Vchat_CourseFinishNoty							= 2218,

	Sub_Vchat_TipsNoty									= 2219,
	Sub_Vchat_SilenceNoty								= 2220,
	Sub_Vchat_PointLinkNoty								= 2221,
	Sub_Vchat_noticeNoty								= 2222,
	Sub_Vchat_CourseNoty								= 2223,
	Sub_Vchat_ForbidUserChatRsp							= 2224,	//禁言请求返回

	Sub_Vchat_CloseLiveAndCourseNoty					= 2225,//禁用课程、直播间通知

	Sub_Vchat_LiveStart					                = 2226,//直播开始通知
	Sub_Vchat_LiveStop					                = 2227,//直播结束通知

	Sub_Vchat_CourseStartNoty			                = 2228,//课程开始通知
	Sub_Vchat_NewSubCourseNoty                          = 2229,//系列课新子课通知
	Sub_Vchat_NewCourseNoty                             = 2230,//新课（单节课或系列课）通知

    /**************chatsvr 3000-3999*************/
    //private chat
    Sub_Vchat_PrivateMsgReq                             =3000,        //新私聊消息请求
    Sub_Vchat_PrivateMsgRecv                            =3001,        //服务端收到新私聊消息反馈
    Sub_Vchat_PrivateMsgNotify                          =3002,        //新私聊消息提示
    Sub_Vchat_PrivateMsgNotifyRecv                      =3003,        //客户端收到新私聊消息提示反馈
    Sub_Vchat_UnreadPrivateMsgReq                       =3004,        //请求用户未读的私聊消息（登录时使用）
    Sub_Vchat_UnreadPrivateMsgNotify                    =3005,        //用户未读的私聊消息提示
    Sub_Vchat_PrivateMsgHisReq                          =3006,        //请求历史私聊消息
    Sub_Vchat_PrivateMsgHisResp                         =3007,        //历史私聊消息反馈
    
    //group public chat
    Sub_Vchat_GroupMsgReq                               =3010,        //发起新群组消息
    Sub_Vchat_GroupMsgRecv                              =3011,        //服务端收到新群组消息反馈
    Sub_Vchat_GroupMsgNotify                            =3012,        //新群组消息提示
    Sub_Vchat_GroupMsgNotifyRecv                        =3013,        //客户端收到新群组消息提示反馈
    Sub_Vchat_UnreadGroupMsgReq                         =3014,        //请求用户未读的群聊消息
    Sub_Vchat_UnreadGroupMsgNotify                      =3015,        //用户未读的群聊消息提示
    Sub_Vchat_GroupMsgHisReq                            =3016,        //请求历史群组消息
    Sub_Vchat_GroupMsgHisResp                           =3017,        //历史群组消息反馈
	Sub_Vchat_GroupUnreadMsgEnd							=3018,		  //群 未读群聊/私聊消息发送完成


    
    //group private chat
    Sub_Vchat_GroupPrivateMsgReq                        =3020,        //发起新群组私聊消息
    Sub_Vchat_GroupPrivateMsgRecv                       =3021,        //服务端收到新群组私聊消息反馈
    Sub_Vchat_GroupPrivateMsgNotify                     =3022,        //新群组私聊消息提示
    Sub_Vchat_GroupPrivateMsgNotifyRecv                 =3023,        //客户端收到新群组私聊消息提示反馈
    Sub_Vchat_UnreadGroupPrivateMsgNotify               =3025,        //用户未读的群组私聊消息提示
    Sub_Vchat_GroupPrivateMsgHisReq                     =3026,        //请求群组历史私聊消息
    Sub_Vchat_GroupPrivateMsgHisResp                    =3027,        //历史群组私聊消息反馈

	Sub_Vchat_GroupOnlookerChatReq						=3030,		  //围观群众发言请求
	Sub_Vchat_GroupOnlookerChatNotify					=3031,		  //围观群众发言提醒
	Sub_Vchat_UnreadGPAssistMsgNotify					=3039,		  //未读群助手提示
	Sub_Vchat_GPAssistMsgNotify							=3040,		  //新群助手提示
	Sub_Vchat_GPAssistMsgNotifyRecv						=3041,		  //客户端收到新群助手消息提示反馈
	Sub_Vchat_GPAssistMsgStateReq						=3042,		  //客户端同步群助手的审核状态请求
	Sub_Vchat_GPAssistMsgStateResp						=3043,		  //客户端同步群助手的审核状态反馈
	Sub_Vchat_UnreadGPAssistMsgReq						=3044,		  //用户未读的群助手消息提示请求
	Sub_Vchat_GPAssistMsgHisReq							=3045,		  //用户查询群助手消息列表请求
	Sub_Vchat_GPAssistMsgHisResp						=3046,		  //用户查询群助手消息列表反馈
	Sub_Vchat_GPAssistMsgStateNotify					=3047,		  //群助手消息审核状态变更
	Sub_Vchat_GroupInvestUserReq						=3048,		  //邀请用户上传图片或视频认证请求
	Sub_Vchat_GroupInvestUserResp						=3049,		  //邀请用户反馈

	Sub_Vchat_GroupPChatQualiticationReq				=3051,		  //用户群私聊资格查询
	Sub_Vchat_GroupPChatQualiticationResp				=3052,		  //用户群私聊资格应答
	Sub_Vchat_DeleteChatMsgHis							=3053,		  //删除聊天历史请求
	Sub_Vchat_DeleteChatMsgHisResp						=3054,		  //删除聊天历史响应

	Sub_Vchat_PPTPicChangeNotify						=3055,		  //PPT 图片变更通知
    /**************chatsvr 3000-3999*************/

	/**************consumesvr 4000-4999**********/
	Sub_Vchat_SendRedPacketReq							=4000,		  //用户发送红包请求
	Sub_Vchat_SendRedPacketResp							=4001,		  //用户发送红包反馈
	Sub_Vchat_CatchRedPacketReq							=4002,		  //用户抢红包请求
	Sub_Vchat_CatchRedPacketResp						=4003,		  //用户抢红包反馈
	Sub_Vchat_TakeRedPacketReq							=4004,		  //用户领取红包请求
	Sub_Vchat_TakeRedPacketResp							=4005,		  //用户领取红包反馈
	Sub_Vchat_QryRedPacketInfoReq						=4006,        //查询红包信息(包括领取情况)                                                                                    
	Sub_Vchat_RedPacketInfo								=4007,        //红包详细信息
	Sub_Vchat_WeekCharmNotify							=4009,        //魅力周榜推送
	Sub_Vchat_WeekContributeNotify						=4010,        //贡献周榜推送
	Sub_Vchat_QryWeekListReq							=4011,        //贡献周榜查询
	Sub_Vchat_QryRedPacketReq							=4012,		  //根据类型查询查询红包
	Sub_Vchat_QryRedPacketRsp							=4013,		  //返回红包查询列表

	Sub_Vchat_SponsorTreasureReq						=4021,		//发起夺宝
	Sub_Vchat_SponsorTreasureResp						=4022,		//发起夺宝反馈
	Sub_Vchat_BuyTreasure								=4023,		//参与夺宝
	Sub_Vchat_BuyTreasureResp							=4024,		//夺宝反馈
	Sub_Vchat_QryMyTreasureDetail						=4025,		//请求我的夺宝详情
	Sub_Vchat_MyTreasureDetail							=4026,		//我的夺宝详情
	Sub_Vchat_QryGroupTreasureList						=4027,		//查询群夺宝列表
	Sub_Vchat_GroupTreasureList							=4028,		//群夺宝列表反馈
	Sub_Vchat_QryMyTreasureList							=4029,		//查询我的夺宝列表
	Sub_Vchat_MyTreasureList							=4030,		//我的夺宝列表反馈
	Sub_Vchat_TreasureInfoNotify						=4031,		//宝物状态变更通知
	Sub_Vchat_QryTreasureInfo							=4032,		//查询宝物信息
	Sub_Vchat_QryTreasureInfoResp						=4033,		//查询宝物信息反馈
	Sub_Vchat_SendPressStampReq							= 4041,			  //用户盖印章请求
	Sub_Vchat_SendPressStampResp						= 4042,			  //用户盖印章反馈
	Sub_Vchat_QryStampReq								= 4043,			  //查询印章
	Sub_Vchat_QryStampRsp								= 4044,			  //查询印章返回
	/**************consumesvr 4000-4999**********/


	/**************pushmsg 5000-5199**********/
	Sub_Vchat_SysNoticeMsgNotify						=5000,		  //系统消息提示
	Sub_Vchat_UnreadSysNoticeMsgNotify					=5001,		  //未读系统消息提示
	Sub_Vchat_SysNoticeMsgNotifyRecv					=5002,		  //客户端收到系统通知消息反馈
	Sub_VChat_UserNoticeMsgNotify						=5003,		  //用户相关通知消息提示
	Sub_VChat_UnreadUserNoticeMsgNotify					=5004,		  //用户相关未读通知提示
	Sub_VChat_UserNoticeMsgNotifyRecv					=5005,		  //客户端收到用户通知消息反馈
	Sub_Vchat_UnreadNoticeMsgReq						=5006,		  //请求用户未读的通知消息（包含个人）
	/**************pushmsg 5000-5199**********/

    //10194和10197消息要空着,前面有定义
	//订阅服务器使用的协议段 20000-21000
	Sub_Vchat_TeacherOnMicReq                           =20000,       //讲师上麦通知订阅用户请求
	Sub_Vchat_PrivateVipNoty                            =20001,       //私人订制通知订阅用户请求

	//登录服务器相关协议段 21001-22000
	Sub_Vchat_UserQuanxianBegin                         =21001,       //用户权限数据
	Sub_Vchat_UserQuanxianLst                           =21002,       //
	Sub_Vchat_UserQuanxianEnd                           =21003,       //
	Sub_Vchat_logonFinished                             =21004,       //登录完成无数据
	Sub_VChat_QuanxianId2ListResp                       =21005,       //权限id数据
	Sub_VChat_QuanxianAction2ListBegin                  =21006,       //权限操作数据
	Sub_VChat_QuanxianAction2ListResp                   =21007,       //
	Sub_VChat_QuanxianAction2ListFinished               =21008,       //
	Sub_Vchat_UserExitMessage_Req                       =21009,       //用户退出软件的请求
	Sub_Vchat_UserExitMessage_Resp                      =21010,       //用户退出软件的响应
	Sub_Vchat_ClientCloseSocket_Req                     =21013,       //client close socket(gate send)
	Sub_Vchat_logonReq5                                 =21015,       //login
	Sub_Vchat_logonErr2                                 =21016,       //登陆失败
	Sub_Vchat_logonSuccess2                             =21017,       //登陆成功
	Sub_Vchat_logonTokenReq                             =21018,       //user token request
	Sub_Vchat_logonTokenNotify                          =21019,       //user token notify
    Sub_Vchat_SetUserPwdReq                             =21026,       //设置用户密码
    Sub_Vchat_SetUserPwdResp                            =21027,       //
    Sub_Vchat_RoomMicStateList                          =21029,       //房间麦状态列表
    Sub_Vchat_SendPushTokenReq                          =21032,       //上传设备推送token

    //节点服务器协议段 23001-24000
    Sub_Vchat_NoticeServerStart = 23001,     //通知节点服务器新服务器启动
    Sub_Vchat_NoticeServerAdd = 23002,       //通知节点服务器新程序正常服务，节点服务器可通知各关联进程
    Sub_Vchat_NoticeServerStop = 23003,      //通知节点服务器程序停止服务,节点服务器可知道各关联进程
    Sub_Vchat_NoticeServerRemove = 23004,    //通知节点服务器不再监听服务
    Sub_Vchat_NoticeServerList = 23005,      //通知服务器关联的服务器列表

	//媒体数据服务器 24001-25000
	Sub_Vchat_NoticeRoomOnMic = 24001,       //讲师上麦通知媒体服务器

	//桥接服务器协议段25001-26000
	Sub_Vchat_LogonLog=25001,				 //用户登录记录登记
	Sub_Vchat_LogoutLog=25002,				 //用户登出记录登记
	Sub_Vchat_JoinRoomRecord=25003,			 //用户加入房间登记
	Sub_Vchat_OnMicMediaMGR = 25004,		 //房间上麦媒体通知

    //PHP协议段  26001-27000
    Sub_Vchat_RoomMgrNoty               = 26001,          //PHP房间管理通知
	Sub_Vchat_BlockUser                 = 26002,          //PHP封杀用户通知
    Sub_Vchat_HistoricalRecordUpdate    = 26003,          //历史战绩
	Sub_Vchat_HandOutUpdate             = 26004,          //讲师讲义
    Sub_Vchat_SyllabusUpDate            = 26006,          //课程表推送
	Sub_Vchat_InvestDataNoty			= 26007,          //投资内参数据推送
	Sub_Vchat_RoomBroadCast				= 26008,		  //PHP配置房间广播(当前999用)
	Sub_Vchat_VIPCourseChangeNoty		= 26009,		  //VIP课程

	//report服务器协议段  27001-28000
	Sub_Vchat_RoomOnline_timespan		= 27001,

	//timesvr服务器协议段 28001-29000
	Sub_Vchat_JPushConfigNoty			= 28001,
    Sub_Vchat_CallFansPushDataReq       = 28002,

	//robot服务器协议段 29001-30000
	Sub_Vchat_SOFTBOT_JOINROOM			= 29001,          //机器人加入房间
	Sub_Vchat_SOFTBOT_EXITROOM			= 29002,          //机器人退出房间

	//usermgrsvr服务器协议段 30001-31000
	Sub_Vchat_PushStockNotify			= 30001,          //股票收益广播推送

	//consumesvr服务器协议段 31001-32000
	Sub_Vchat_BuyVipNotify				= 31001,          //VIP开通动画广播推送
	Sub_Vchat_BuyPtCourseReq			= 31002,		  //购买白金课程请求
	Sub_Vchat_BuyPtCourseResp			= 31003,		  //购买白金课程响应
	Sub_Vchat_BuyPtCourseNoty			= 31004,		  //购买白金课程飘窗广播

	//syncsvr 服务器协议段 32001-33000
	Sub_SyncData_req					= 32001,
	Sub_SyncData_resp					= 32002,
	Sub_SyncData_Last_resp				= 32003,

	//认证
	Sub_Vchat_Auth_Req					= 50000,
	Sub_Vchat_Auth_Resp					= 50001,
	Sub_Vchat_Auth_Pass					= 50002,
	Sub_Vchat_Room_Member 				= 51000,
	//gateway协议段 60000-65535
	Sub_Vchat_SubCmdReq                 = 60000,            //客户端订阅指定命令字的请求
	Sub_Vchat_RegistDevReq				= 60001,			//设备连接登录请求
	Sub_Vchat_SubCmdLstReq				= 60002,			//客户端订阅多个命令字的请求
};

typedef enum {
	e_logonsvr_type			= 1,
	e_roomsvr_type			= 3,
	e_roomadapter_type		= 4,
	e_marketquotations_type = 5,
	e_consumesvr_type		= 6,
	e_goldtrade_type		= 8,
	e_usermgrsvr_type		= 9,
	e_reportsvr_type		= 10,
	e_mediamgrsvr_type		= 11,
	e_timesvr_type			= 12,
	e_tppushsvr_type		= 13,
	e_gateway_type			= 14,
	e_bridgesvr_type		= 15,
	e_syncsvr_type			= 16,
	e_pushmsg_type			= 17,
	e_stocknotice_type		= 18,
}e_SvrType;

enum {
	Sub_Json_Msg_Push				= 10001,		//小米、iso消息通知
};

//packet use
enum {
	en_msgbuffersize     = 1024*64,
	en_msgmaxsize        = 1024*32,
	en_checkactivetime   = 5,	//unit:s
	en_checkkeeplivetime = 20,	//unit:s
	en_checkconntimeout	 = 5	//unit:s
};

enum {
	MSG_TYPE_TIMER			= 1050,
	MSG_TYPE_CLIENT_CONNECT	= 1051,
	MSG_TYPE_SERVER_CONNECT = 1052,
	MSG_TYPE_CLOSE 			= 1053,
	MSG_TYPE_ERROR 			= 1054,
};

//0-PC,1-Android,2-IOS,3-web
enum {
	e_PC_devtype       = 0,
	e_Android_devtype  = 1,
	e_IOS_devtype      = 2,
	e_WEB_devtype      = 3
};

//server like:usermgr\consumesvr use these type to push msg to client
enum {
	e_configsvr_noty	= 1,
	e_clientlog_noty	= 2,
	e_clientupdate_noty = 3,
	e_refreshmoney_noty = 4,
	e_baywindow_noty	= 5,
	e_roomgroup_noty	= 6,
	e_teacher_mic_noty	= 7,
	e_profess_view_noty = 8,
	e_new_mail_noty		= 9
};

typedef enum NodeMgrServerStatus
{
    NodeMgrServerStatusNew = 0,
    NodeMgrServerStatusNormal = 1,
    NodeMgrServerStatusStop = 2,
}e_NodeMgrServerStatus;

typedef enum MediaConnectActionType
{
	Connect_You = 1,
	Disconnect_You = 3,
}e_MediaConnectActionType;

typedef enum NoticeDevType
{
    e_Notice_PC        = 0,
    e_Notice_Android   = 1,
    e_Notice_IOS       = 2,
    e_Notice_Web       = 3,
    e_Notice_AllType   = 4,
	e_Notice_WinPhone  = 5,
}e_NoticeDevType;

typedef enum enum_AlarmLevel
{
    alarm_level_general = 0,
    alarm_level_major,
    alarm_level_fatal
}e_AlarmLevel;

typedef enum enum_PtCourseState
{
	e_NotPostState     = 0,
	e_PostState        = 1,
	e_NotStartState    = 2,
	e_GotoClassState   = 3,
	e_CloseState       = 4
}e_PtCourseState;

typedef enum 
{
	SYNC_INIT,
	SYNC_SEMI_READY,
	SYNC_READY
}REDIS_SYNC_STATUS;

//角色身份 0：游客 10：正式成员 20：嘉宾  30：管理员  40：群主
typedef enum
{
	e_VisitorRole		= 1,
	e_MemberRole		= 10,
	e_GuestRole			= 20,
	e_AdminRole			= 30,
	e_OwnerRole			= 40
}USER_ROLE_TYPE;

typedef enum
{
	e_DefaultGender		= 0,
	e_MaleGender		= 1,
	e_FemaleGender		= 2
}USER_GENDER_TYPE;

typedef enum
{
	MSGTYPE_KICKGROUP	= 0,
	MSGTYPE_LEVEL_CHG	= 1,
	MSGTYPE_PCHAT_STATE	= 2,
	MSGTYPE_VIDEO_AUTH	= 3,
	MSGTYPE_MSG_AUTH	= 4,
	MSGTYPE_INVEST_PHOTO	= 5,
	MSGTYPE_INVEST_VIEWAUTH	= 6,
	MSGTYPE_REDPACK_BALANCE = 7,
}e_MsgSvrType;

//same as e_AssistMsgType in ChatSvr.proto
enum
{
	ASSISTMSG_SYSTEM	= 0,
	ASSISTMSG_VIEW		= 1,
	ASSISTMSG_AGREE		= 2,
};

typedef enum 
{
	// 1 邀请 2同意 3拒绝 4超时拒绝 5连接成功 6挂断 7取消邀请 8被邀请 9强制挂断
	INVT_MIC_INVITE     = 1,
	INVT_MIC_AGREE      = 2,
	INVT_MIC_REJECT     = 3,
	INVT_MIC_TIMEOUT    = 4,
	INVT_MIC_CONNECTED  = 5,
	INVT_MIC_HANGUP     = 6,
	INVT_MIC_CANCEL     = 7,
	INVT_MIC_BE_INVITED = 8,
	INVT_MIC_FORCE_HANGUP = 9
}e_InviteMicOptType;

//same as e_JoinGroupAuthType in TCommonIf.thrift
typedef enum
{
	Join_NoNeedAuth		= 0,
	Join_RedPacket		= 1,
	Join_VideoAuth		= 2,
	Join_MsgAuth		= 3,
	Join_Forbidden		= 4
}e_JoinGroupAuth;

#define MULTICAST_TYPE 6666	//多播
#define BROADCAST_TYPE 9999
#define BROADCAST_ROOM_TYPE 8888
#define BROADCAST_ROOM_ROLETYPE 7777
#define BROADCAST_ROOM_GENDER 2222
#define CAST_CLIENTS_ON_ONE_SVR 5555
#define CAST_CLIENTS_SUBSCRIBE 4444
#define CAST_APP_VERSION 3333

//----------------------------------------------------------
#pragma pack(1)

//4 bytes
typedef struct tag_CMDClientHello
{
	uint8 param1;
	uint8 param2;
	uint8 param3;
	uint8 param4;
}CMDClientHello_t;

//6 bytes
typedef struct tag_CMDGateHello
{
	uint8 param1;
	uint8 param2;
	uint8 param3;
	uint8 param4;
	uint16 gateid;
}CMDGateHello_t;

//6 bytes
typedef struct tag_CMDRoomsvrHello
{
	uint8 param1;
	uint8 param2;
	uint8 param3;
	uint8 param4;
	uint16 svrid;
}CMDRoomsvrHello_t;

//4 bytes
typedef struct tag_CMDClientPing
{
	uint32 userid;        //用户id
	uint32 roomid;        //房间id
}CMDClientPing_t;

typedef struct tag_CMDSetExecQueryReq
{
	uint32 userid;
    int32  textlen;
	char   content[0];
}CMDSetExecQueryReq_t;

typedef struct tag_CMDSetExecQueryResp
{
   uint32 userid;
   int32  errorid;
}CMDSetExecQueryResp_t;

typedef struct tag_CMDGetDBInfoReq
{
   uint32 userid;
}CMDGetDBInfoReq_t;

typedef struct tag_CMDGetDBInfoResp
{
	uint32 userid;
	int32  dbport;
	char szServer[32];
	char szdbname[32];
	char szdbuser[32];
	char szdbuserpwd[32];
}CMDGetDBInfoResp_t;

typedef struct tag_CmdServerAuthInfo
{
	char sz1[64];
	char sz2[64];
	char sz3[64];
	char sz4[64];
	char sz5[64];
}CmdServerAuthInfo_t;

//128 bytes
typedef struct tag_CMDClientHello_2
{
	char encrytionContent[16];  //加密后的内容
	char encrytionKey[16];
}CMDClientHello_t_2;

typedef struct tag_CMDNoticeServerInfo
{
    int servertype;
    char szIp[32];
    int port;
}CMDNoticeServerInfo;

typedef struct tag_CMDNoticeServerList
{
    int num;
    char content[0];        //CMDNoticeServerInfo ...
}CMDNoticeServerList;

typedef struct _tag_AndroidParam
{
	int vcbid;
	int userid;
	int ngender;
	int viplevel;
	int roomlevel;
	int carid;
	char szuseralias[NAMELEN];
	char szcarname[NAMELEN];
	int lifetime;
	int temp_maxaddnum;  //临时变量,占位后用
}AndroidParam_t;

#pragma pack()

#endif  //__MESSAGE_VCHAT_HH_20130715__


