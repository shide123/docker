# TPushMsgIf.thrift

enum e_PushMsgType {  
	PrivateChat,  		#私聊
	GroupChat,  		#群聊
	GroupPrivateChat,	#群私聊
}  


struct TChatMsgPush
{
	1: e_PushMsgType msg_type;	//消息类型
	2: i32 srcuid;			//消息发出者userid
	3: string title;		//标题
	4: string description;	//描述
	5: i64 create_time;		//时间
	6: string notify_id;	//如果通知栏要显示多条推送消息，需要针对不同的消息设置不同的notify_id（小米有效）
	7: bool include_muted = false;	//免打扰的用户也要推送
	8: list<i32> userids;	//推送用户id，私聊、群私聊、@人必须传
}

############################################################################

service TpPushSvr 
{
	########################################################################
	# function    ：proc_notifyChatMsg                                     #
	# description ：第三方离线消息推送                                     #
	# input param ：msg  推送消息详情                                      #
	# input param ：groupid 群id，非群消息则为0                            #
	########################################################################
	void proc_notifyChatMsg(1: TChatMsgPush msg, 2: i32 groupid = 0, 3: bool notify_offline = false),
}
