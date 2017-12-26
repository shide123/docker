# TChatSvrIf.thrift

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

struct TChatMsg
{
    1: i32 srcUId;   //消息发起者userid
    2: i32 dstUId;   //聊天对象userid
    3: byte msgType;   //消息类型 0文字, 1图片, 2语音, 3视频, 4红包, 5群助手，6OGod团队，7提示消息
    4: string content;   //聊天内容  
}

struct TChatAuditMsg
{
    1: i32 srcUId;   //消息发起者userid
    2: i32 dstUId;   //聊天对象userid
    3: byte msgType;   //消息类型 0文字, 1图片, 2语音, 3视频, 4红包, 5群助手，6OGod团队，7提示消息
    4: string content;   //聊天内容  
	5: i32 groupId;   //房间id
	6: i32 msgId;   //msgid
	7: i32  mastermsgId ;//父评论id
	8: i32  notifyType; //通知类型 1 审核通过;2 回复评论
	9: i32  updateId;//如果是在管理后台回复评论需要填该字段
}
struct TChatMsgExtra
{
	1: bool isPrivate = false;
	2: bool pushOnline = false;
	3: bool memberOnly = false;
}

struct TGroupAssistMsg
{
	1: i32 userID;			//加入群是入群者 踢出是被踢者 等级变更是被调整者
	2: i32 groupID;			//群ID
	3: i32 svrType;			//业务类型  0.kick out group 1.level change 2.pchat switch 3.video auth 4.msg auth
	4: bool svrSwitch;		//功能开关
	5: bool svrLevelUp;		//功能等级提升/降低
	6: i32 svrLevel;		//功能等级
	7: i32 state;			//0.不需要处理 1.待处理 2.非管理者同意 3.管理者同意 4.拒绝
	8: string url;			//url
	9: i32 msgType;			//msgType 0.系统类型  1.查看类型  2.同意or拒绝类型
	10:string msg;			//留言内容
	11:i32 authID;			//认证ID,对应mc_authentication的id
	12:i32 investuserID;	//邀请者ID
	13:i32 packetID;		//红包id
	14:i32 balance;			//红包余额
}

struct TPPTPicInfo
{
   1: i32 rank;			//图片排序号
   2: string picId;		//图片id
   3: i32 groupId;		//课程直播间id
   4: string picUrl;	//图片URL
}
############################################################################

service TChatSvr
{
    bool procGroupChatReq(1: i32 groupid, 2: TChatMsg chatMsg, 3: TChatMsgExtra extra),
	#10 表示成员
    bool procUserJoinGroupNotify(1: i32 userid, 2: i32 groupid, 3: i32 roletype = 10),

	#增加群助手消息
	i32 proc_addGroupAssistMsg(1:TGroupAssistMsg assistMsg, 2:bool bNotyAdmin = true, 3:bool bNotyUser = true);
	#处理群助手消息
	bool handle_approveGroupAssistMsg(1: i32 handle_userid, 2: i32 groupid, 3: i32 msgid, 4: bool agree, 5: i32 apply_userid);
	bool handle_voice_callback(1:string inputkey,2:i32 code,3:string key);
	bool handle_keyword_action(1:i32 action,2:string value,3:string replace);

	#操作ppt图片,1 增加图片,须带上排序号;2 删除图片, 3 修改图片
	bool proc_optPPTPic(1:list<TPPTPicInfo> vecPicId,2:i32 optType);

	#评论审核通知、后台回复评论
	bool proc_commentAudit(1:TChatAuditMsg tTChatMsg);
}
