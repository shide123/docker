# TCommonIf.thrift
namespace php Thrift.ThriftStub.RPC_Group_Push
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

enum E_THRIFT_GROUP_SETTING
{
	#围观功能
	VISIT_SWITCH  = 1, 					#是否允许围观
	KICK_ALL_VISITOR = 2,				#是否踢出所有游客
	
	#入群验证
	MALE_JOIN_AUTH = 3,					#男进群验证方式
	MALE_JOIN_REDPACKET_NUM = 4,		#男生群红包数量
	MALE_JOIN_REDPACKET_AMOUNT = 5,		#男生入群红包总额
	FEMALE_JOIN_AUTH = 6,				#女生进群验证方式
	FEMALE_JOIN_REDPACKET_NUM = 7,		#女生进群红包数量
	FEMALE_JOIN_REDPACKET_AMOUNT = 8,	#女生入群红包总额

	#贡献榜
	CONTRIBUTION_LIST_SWITCH = 9,		#贡献榜开关

	#语音功能
	VOICE_CHAT_SWITCH = 10,				#语音聊天开关
	ALLOW_VISITOR_ON_MIC = 11,			#是否允许游客上麦

	#分成功能
	GAIN_RANGE_TYPE = 12,				#分成范围
	GAIN_PERCENT = 13,					#分成比例

	#群私聊功能
	PRIVATE_CHAT_SWITCH = 14,			#私聊开关
	PCHAT_MIN_THRESHOLD_MALE = 15,		#男生最低私聊门槛
	PCHAT_MIN_THRESHOLD_FEMALE =16,		#女生最低私聊门槛

	GAIN_SWITCH = 17,					#分成开关
	GAIN_GROUP = 18,					#群利润百分比（来自群主收入)
}

enum e_INGROUP_TYPE
{
	Member	= 1;	//成员
	Visitor	= 2;	//游客
}

//action means: 0--visitor 1--member
//inway = e_JoinGroupAuthType
struct TJoinGroup
{
	1: i32	userid;
	2: i32	groupid;
	3: byte	inway;
	4: byte	action;
}

struct TUserGroupInfo
{
	1: i32		userID;
	2: string	userAlias;
	3: string	userHead;
	4: i32		groupID;
	5: i32		roleType;
}

enum e_JoinGroupAuthType
{
	NoNeedAuth = 0, //不需要验证
	RedPacket  = 1, //发红包进群   
	VideoAuth  = 2, //视频认证进群
	MsgAuth    = 3, //留言申请
	Forbidden  = 4, //禁止进群
}

struct TJoinGroupCondition
{
	1: e_JoinGroupAuthType type;	//验证方式
	2: i16	redPacketNum;	//红包数量（type == RedPacket 时必须使用）
	3: i32	redPacketAmount;	//红包总额（type == RedPacket 时必须使用），实际金币*100
}

enum SYSMSG_COND_TYPE
{
	USER		= 1,	//用户id
	TELNUM		= 2,	//手机号码
	APPVERSION	= 3,	//客户端版本
}

struct TCondition
{
	1: SYSMSG_COND_TYPE	type;	//条件类型：1 用户id，2 手机号码，3 客户端版本
	2: string		oper;	//条件运算符：等于(=)/大于(>)/小于(<)/大于等于(>=)/小于等于(<=)/包含(in)
	3: string		value;	//等于(=)/大于(>)/小于(<)/大于等于(>=)/小于等于(<=) 时使用
	4: set<string>	values;	//包含(in) 时使用
}

struct TNoticeMsg
{
	1: i16		    type;		//通知类型：1 系统公告，2 活动公告，3 版本通知，10 充值成功
	2: string		content;	//通知标题
	3: string		action;		//客户端点击动作
	4: i32			endTime;	//结束时间
}

############################################################################
