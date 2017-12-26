#ifndef __CMD_VCHAT_HH_20110409__
#define __CMD_VCHAT_HH_20110409__

#include "yc_datatypes.h"
#include "message_vchat.h"
#include <time.h>

#define MAXSTARSIZE		  16  //周星等最大数目限制
 
//-----------------------------------------------------------
#pragma pack(1)
//登录请求消息

typedef struct tag_CMDUserLogonReq4
{
	uint32 nmessageid;      //message id
	char   cloginid[32];    //[0]-游客登陆
	uint32 nversion;        //本地版本号?
	uint32 nmask;           //标示,用于客户端验证是不是自己发出的resp,
	char   cuserpwd[PWDLEN];    //当bPwdType为0时,登录密码,游客登录不需要密码,长度与将来的md5兼容,当bPwdType为1时,这个是短信验证码,当bPwdType为2时,这个是用户token
	char   cSerial[64];		//uuid
	char   cMacAddr[IPADDRLEN]; //mac地址
	char   cIpAddr[IPADDRLEN];  //ip地址
	byte   bPwdType;        //0-用户密码,1-cuserpwd为短信验证码,2-cuserpwd为用户token
	byte   nmobile;         //0-PC,1-Android,2-IOS,3-web
	char   devicemodel[32];     // 设备型号
	char   deviceos[16];        // 设备系统
}CMDUserLogonReq4_t;

//Logon from other platform
typedef struct tag_CMDUserLogonReq5
{
	uint32 nmessageid;          //message id
	uint32 userid;              //local user id,as before
	char   openid[48];          //open platform id
	char   opentoken[64];       //open platform token
	uint32 platformType;        //platform type,1-QQ;2-weibo;3-wechat;4-token
	uint32 nversion;            //本地版本号?
	uint32 nmask;               //标示,用于客户端验证是不是自己发出的resp
	char   cSerial[64];         //uuid
	char   cMacAddr[IPADDRLEN]; //mac地址
	char   cIpAddr[IPADDRLEN];  //ip地址
	byte   nimstate;            //IM状态:如隐身登录
	byte   nmobile;             //0-PC,1-Android,2-IOS,3-web
	char   devicemodel[32];     // 设备型号
	char   deviceos[16];        // 设备系统
}CMDUserLogonReq5_t;

//登录错误消息
//4 bytes
typedef struct tag_CMDUserLogonErr
{
	uint32 errid;       //根据id,客户端本地判断错误,包括版本错误(需要升级),封杀
	uint32 data1;       //参数1
	uint32 data2;       //参数2
}CMDUserLogonErr_t;

//登录错误消息
typedef struct tag_CMDUserLogonErr2
{
	uint32 nmessageid;  //message id
	uint32 errid;       //根据id,客户端本地判断错误,包括版本错误(需要升级),封杀
	uint32 data1;       //参数1
	uint32 data2;       //参数2
}CMDUserLogonErr2_t;

//用户权限响应消息(10 bytes)
typedef struct tag_CMDUserQuanxianInfo
{
	uint8 qxid;
	uint8 qxtype;
	uint32 srclevel;
	uint32 tolevel;
}CMDUserQuanxianInfo_t;

//登录成功响应消息
//71 bytess
typedef struct tag_CMDUserLogonSuccess
{
	int64 nk;                    //金币
	int64 nb;                    //礼物积分,可以换金币或兑换RMB
	int64 nd;                    //游戏豆
	uint32 nmask;                 //标志位, 用于客户端验证是不是自己发出的resp,
	uint32 userid;                //本号
	uint32 langid;                //靓号id
	uint32 langidexptime;         //靓号id到期时间
	uint32 servertime;            //服务器时间,显示客户端,用于抢星之类的时间查看
	uint32 version;               //服务器版本号,用在在信令相同的情况下登陆成功发回服务器DB中的版本号
	uint32 headid;                //用户头像id
	byte   viplevel;              //会员等级(可能是临时等级)
	byte   yiyuanlevel;           //艺员等级,如
	byte   shoufulevel;           //守护者等级
	byte   zhonglevel;            //终生等级,叠加数字(1~150)
	byte   caifulevel;            //财富等级
	byte   lastmonthcostlevel;     //上月消费排行
	byte   thismonthcostlevel;     //本月消费排行
	byte   thismonthcostgrade;    //本月累计消费等级
	byte   ngender;               //性别
	byte   blangidexp;            //靓号是否过期
	byte   bxiaoshou;             //是不是销售标志
	char   cuseralias[NAMELEN];   //呢称
}CMDUserLogonSuccess_t;

typedef struct tag_CMDUserLogonSuccess2
{
	uint32 nmessageid;            //message id, 用于客户端验证是不是自己发出的resp
	uint32 userid;                //本号
	char   headadd[64];           //用户头像地址
	byte   userlevel;             //用户等级（1是游客，2是普通用户）
	byte   viplevel;              //vip等级
	uint32 vipexpire_time;        //vip到期时间
	byte   ngender;               //性别
	char   cuseralias[NAMELEN];   //呢称
	byte   haspwd;				  //是否有密码
	byte   bloginSource;          //登陆平台，0是本平台，1是第三方平台
	char   cTel[32];			  //电话号码
	char   sessiontoken[33];	  //user表里的code
	char   validtime[32];		  //user表里的code_time
	int64  coin;                  //用户的平台币
}CMDUserLogonSuccess2_t;

//设置用户资料
typedef struct tag_CMDSetUserProfileReq
{
	uint32 userid;                 //用户ID
	uint32 headid;                 //用户头像ID
	byte   ngender;                //性别
	char   cbirthday[BIRTHLEN];    //生日
	char   cuseralias[NAMELEN];    //用户昵称	
	char   province[16];           //省份
	char   city[16];               //城市
    int16  introducelen;           //个人签名长度
	char   content[0];             //消息内容，格式：个人签名
}CMDSetUserProfileReq_t;

typedef struct tag_CMDSetUserProfileResp
{
	uint32 userid;
	int32  errorid;    //错误代码
}CMDSetUserProfileResp_t;

//设置用户密码
typedef struct tag_CMDSetUserPwdReq
{
	uint32 userid;         //用户id
	uint32 vcbid;          //0房间外操作
	char   pwdtype;        //密码类型，1-用户登录密码,2-用户银行密码
	char   oldpwd[PWDLEN];     //旧密码
	char   newpwd[PWDLEN];     //新密码
}CMDSetUserPwdReq_t;

typedef struct tag_CMDSetUserPwdResp
{
	uint32 userid;
	uint32 vcbid;
	int    errorid;        //错误代码, 0-无错误
	char   pwdtype;
	char   cnewpwd[PWDLEN];     //设置成功的新密码
}CMDSetUserPwdResp_t;

//请求房间组列表消息
//12...-bytes
typedef struct tag_CMDRoomGroupListReq
{
	uint32 userid;               //请求者的id
}CMDRooGroupListReq_t;

//房间组列表的数据体 (已经排序好的,即parent排序[+子排序] 一共二级排序,见高校导航树php说明)
//29 + n bytes
typedef struct tag_CMDRoomGroupItem
{ 
	uint32 grouid;                //组id
	uint32 parentid;              //父id,0-没有
	int32  usernum;               //用户数目
	uint32 textcolor;             //文字颜色
	byte   reserve_1;     
	byte   showusernum;           //是否显示人数
	byte   urllength;             //内容长度
	byte   bfontbold;             //文字是否是粗体
	char   cgroupname[NAMELEN];   //组名字
	char   clienticonname[NAMELEN];  //图标名称
	char   content[0];            //URL内容
}CMDRoomGroupItem_t;

//房间组状态消息
typedef struct tag_CMDRoomGroupStatus
{
	uint32 grouid;                //组id
	uint32 usernum;               //组人数
}CMDRoomGroupStatus_t;

//请求房间列表消息
//4 bytes
typedef struct tag_CMDRoomListReq
{
	uint32 userid;               //请求者的id
	uint32 ntype;                //-1,指定vcb_id列表,>0,就是房间组id
	uint32 nvcbcount;
	char   content[0];          //vcb_id列表
}CMDRoomListReq_t;

//房间列表的数据体
//112 byte -- 不再使用,依然使用web的方式代替
typedef struct tag_CMDRoomItem
{
	uint32 roomid;            //房间id
	uint32 creatorid;         //房间创建者id
	uint32 groupid;           //房间组id, 这里不适合使用多组有同一个房间的方式,如果是多组有同一个房间,应该单独有个组/房间对应表
	uint32 flag;              //
	char   cname[256];         //房间名称
	char   croompic[256];      //房间图片
	char   croomaddr[GATEADDRLEN];       //只有一个roomaddr,房间网关地址
}CMDRoomItem_t;

//加入房间请求
//281 bytes
typedef struct tag_CMDJoinRoomReq
{
	uint32 userid;         //用户id,可能是靓号id,可能是游客号码
	uint32 vcbid;          //房间id
	uint32 devtype;      //0:PC端 1:安卓 2:IOS 3:WEB
	uint32 time;
	uint32 crc32;
	uint32 coremessagever;     //客户端内核版本
	char   cuserpwd[PWDLEN];   //用户密码,没有就是游客
	char   croompwd[PWDLEN];   //房间密码,可能有
	char   cSerial[64];    //uuid
	char   cMacAddr[IPADDRLEN];   //客户端mac地址
	char   cIpAddr[IPADDRLEN];	  //客户端ip地址
	byte   bloginSource;         //local 99 login or other platform login:0-local;1-other platform
	byte   reserve1;
	byte   reserve2;
}CMDJoinRoomReq_t;

//在gate端请求加入房间(主要解决roomsvr断开的问题)
typedef struct tag_CMDGateJoinRoomReq
{
	uint32 userid;         		  //用户id
	uint32 vcbid;          		  //房间id
	char   cIpAddr[IPADDRLEN];	  //客户端ip地址
	char   uuid[64];    		  //uuid
	byte   devtype;				  //客户端类型	//0:PC端 1:安卓 2:IOS 3:WEB
	uint32 	  micuserid;		  //上麦用户
	char   	  micstate;  		//麦克状态,注意下面的麦状态标志
	char      micindex;
}CMDGateJoinRoomReq_t;

typedef struct tag_CMDGateJoinRoomResp
{
	uint32 userid;         		  //用户id
	uint32 vcbid;          		  //房间id
	int    errorid;				  //错误id (0:成功,否则失败)
}CMDGateJoinRoomResp_t;

//攻城消息内容
typedef struct tag_SiegeInfo
{
	uint32	vcbid;
	uint32	srcid;				//发送者id
	uint32	toid;				//接收者id
	uint32	giftid;				//礼物id
	uint32	count;				//礼物数目
	uint32	time;				//发起时间
	char	srcalias[NAMELEN];	    //发送者昵称
	char	toalias[NAMELEN];	    //接收者昵称
}SiegeInfo_t;


//加入房间响应 332 + 88bytes (420byte)
typedef struct tag_CMDJoinRoomResp
{
	uint32 userid;       //用户id
	uint32 vcbid;        //房间id
	byte   roomtype;     //房间类型
	byte   busepwd;      //是否需要房间密码
	byte   bIsCollectRoom;  //是否被该用户收藏
	byte   devtype;    //设备类型 PC端 1:安卓 2:IOS 3:WEB
	uint16 seats;        //总人数
	uint32 groupid;        //组id, 可以用来做区长判断?
	uint32 runstate;       //房间 管理状态
	uint32 creatorid;      //房主
	uint32 op1id;          //副房主
	uint32 op2id;        
	uint32 op3id;
	uint32 op4id;
	uint32 inroomstate;  //加入房间后在房间状态，目前只有使用到隐身状态解析.
	int64 nk;                //用户余额,nk
	int64 nb;                //用户余额,nb
	int64 nlotterypool;      //幸运奖池彩金
	int32 nchestnum;         //用户现有宝箱个数
	char   cname[NAMELEN];         //房间名字
	char   cmediaaddr[MEDIAADDRLEN];   //媒体服务器地址
	char   cpwd[PWDLEN];     
	uint64 npopularity_;		   //房间访问人气(人次)
	uint32 ncollect_times;		   //房间收藏次数(粉丝数)
}CMDJoinRoomResp_t;

//加入房间错误
typedef struct tag_CMDJoinRoomErr
{
	uint32 userid;
	uint32 vcbid;
	uint32 errid;
	uint32 data1;
	uint32 data2;
}CMDJoinRoomErr_t;

//房间人员列表的数据体
typedef struct tag_CMDRoomUserInfo
{
	uint32 	userid;                //用户id,可能是靓号id
	uint32 	vcbid;                 //房间id
	byte   	userlevel;              //会员等级(游客、注册用户、管理员等)
	byte   	userviplevel;			//vip等级（黄金vip，白金vip）
	byte   	gender;              //性别,
	byte   	devtype;            //设备类型
	char   	pubmicindex;           //公麦位置
	byte   	roomlevel;             //房间等级
	byte   	usertype;              //用户类型: 普通 机器人
	uint32 	cometime;				//进入房间时间
	char 	chead[64 + 1];			//用户头像
	uint32 	userstate;             //用户状态,组合标志
	int32  	bforbidchat;			//禁言
	char   	useralias[NAMELEN];   //呢称
}CMDRoomUserInfo_t;


//房间公麦状态
typedef struct tag_CMDRoomPubMicState
{
	int16   micid;         //从0开始(0-第一个公麦),和本地公麦用户的micid对应
	int16   mictimetype;   //
    uint32  userid;        //该麦的用户id,可能为0(机器人id)
	int16   userlefttime;  //麦用户剩余时间,
}CMDRoomPubMicState_t;


//房间聊天请求和通知的数据体,目前无跨平台的大喇叭(客户大多数只有一个平台)
typedef struct tag_CMDRoomChatMsg
{
  uint32 vcbid;      //roomId
  uint32 tovcbid;    //new
  uint32 srcid;
  uint32 toid;
  byte srcuserlevel;  //用户的user level,对广播消息有用(前缀图标),管理员\普通用户\讲师等等
  byte touserlevel;   //目标用户的user level,管理员\普通用户\讲师等等
  byte srcuserviplevel;  //用户的vip level
  byte touserviplevel;   //目标用户的vip level

  //msgtype定义:
  //0-文字聊天消息,1-房间内广播消息,2-欢迎词消息(收到此消息不会再自动回复)，3-小喇叭消息(聊天),4-礼物喇叭消息,5-塞子,
  //8-宝箱获取广播喇叭消息,10-获得幸运奖消息(给小喇叭窗口),11-自动回复消息(收到此消息不会再自动回复)
  //13-彩色文字(提出区别的目的:该数据只在公麦区显示),14－隔房间小纸条消息(给自己私聊窗口),15-房间内私聊消息 16-礼物感谢
  //20-系统广播消息
  byte msgtype;      //私聊类型也在放这里
  uint16 textlen;    //聊天内容长度
  char   srcalias[NAMELEN];
  char   toalias[NAMELEN];
  char   vcbname[NAMELEN];
  char   tocbname[NAMELEN];//new
  char   content[0];  //聊天内容
}CMDRoomChatMsg_t;


//聊天应答报文
typedef struct tag_CMDRoomChatMsgResp
{
	uint32 vcbid;      //roomId
	uint32 tovcbid;		//new
	byte msgtype;      //私聊类型也在放这里
	uint16 textlen;    //聊天内容长度
}CMDRoomChatMsgResp_t;




//房间公告的数据体
typedef struct tag_CMDRoomNotice
{
	uint32 vcbid;     //roomId
	byte   index;     //房间公告索引idx:0~2(0,1,2)
	uint16 textlen;   //房间公告长度
	char   content[0];
}CMDRoomNotice_t;

//房间公告的数据体,带发送者信息
typedef struct tag_CMDRoomNoticeNoty
{
	uint32 vcbid;     //roomId
	uint32 userid;     //userId
	char   useralias[NAMELEN];
	byte   index;     //房间公告索引idx:0~2(0,1,2)
	uint16 textlen;   //房间公告长度
	char   content[0];
}CMDRoomNoticeNoty_t;

//系统公告的数据体
//系统公告消息,注意单独使用，如中奖的提示小喇叭,赠送一定数额的礼物提示
typedef struct tag_CMDSysCastNotice
{
	byte msgtype;   //公告类型
	byte reserve;
	uint16 textlen;  //文字长度
	char content[0];
}CMDSysCastNotice_t;
//msgtype:
//1-幸运礼物中大奖提示,2-玩游戏大赢钱提示, 3-一次送超过金额的送礼提示, 4-准备播放烟花提示
//5-其他系统公告消息(系统公告)

//房间管理人员列表的数据体
typedef struct tag_CMDRoomManagerInfo
{
	uint32 vcbid;
	uint16 num;        //管理员(正管)数目
	uint32 members[0];
}CMDRoomManagerInfo_t;

//赠送礼物请求和通知的数据体
typedef struct tag_CMDTradeGiftRecord
{
    uint64 nk;	
	uint32 vcbid;     //roomId
	uint32 srcid;
	uint32 toid;
	uint32 tovcbid;    //接受对象所在的房间
	uint32 totype;     //接受对象类型:0-普通用户,1-所有用户,2-所有注册用户,3-所有会员,4-所有管理,5-所有主持
	uint32 giftid;     //礼物id
	uint32 giftnum;    //赠送数目
	uint32 dtime;      //发起时间
	uint32 oldnum;     //下发时填写,上次数目, 下发跑道礼物列表时0(null)
	char   flyid;      //跑道ID,-1没有
	char   srcvcbname[NAMELEN];     //消费者所在的房间名称
	char   tovcbname[NAMELEN];      //接受者所在的房间名称
	char   srcalias[NAMELEN];
	char   toalias[NAMELEN];
	char   sztext[GIFTTEXTLEN];   //定义40,实际使用最多18个汉字或英文（最多占36个长度)
}CMDTradeGiftRecord_t;


//赠送礼物的响应
typedef struct tag_CMDTradeGiftResp
{
	uint32 vcbid;     //roomId
	uint32 tovcbid;    //接受对象所在的房间
	uint32 giftid;     //礼物id
	uint32 giftnum;    //赠送数目
}CMDTradeGiftResp_t;

//赠送礼物的错误响应
typedef struct tag_CMDTradeGiftErr
{
	int nerrid;    //错误编号
}CMDTradeGiftErr_t;

//鲜花赠送消息
//见消息定义说明,一种特殊的应用 (同平台,同房间内应用),不可用在月票上
typedef struct tag_CMDTradeFlowerRecord
{
	uint32 vcbid;
	uint32 srcid;
	uint32 toid;
	uint32 giftid;    //鲜花礼物id
	uint32 sendnum;   //数目,默认为1
	uint32 allnum;    //总数,收到后更新客户端
	char   srcalias[NAMELEN];
	char   toalias[NAMELEN];
} CMDTradeFlowerRecord_t;

//用户退出房间请求和通知的数据体
typedef struct tag_CMDUserExitRoomInfo
{
	uint32 userid;
	uint32 vcbid;
}CMDUserExitRoomInfo_t;

typedef struct tag_CMDUserExitRoomInfo_ext
{
	uint32 userid;
	uint32 vcbid;
	uint16 textlen;     // the length of content
	char   content[0];   //user ip and gate ip
}CMDUserExitRoomInfo_ext_t;

//用户退出房间请求和通知的数据体
typedef struct tag_CMDUserExceptExitRoomInfo
{
	uint32 userid;
	uint32 vcbid;
}CMDUserExceptExitRoomInfo_t;

typedef struct tag_CMDUserExceptExitRoomInfo_ext
{
	uint32 userid;
	uint32 vcbid;
	uint16 textlen;     // the length of content
	char   content[0];   //user ip and gate ip
}CMDUserExceptExitRoomInfo_ext_t;

//踢人请求和通知消息的数据体
typedef struct tag_CMDUserKickoutRoomInfo
{
	uint32 vcbid;
	uint32 srcid;
	uint32 toid;
	int32  resonid; 
	byte   mins;         //提出时间id,自输入(0~255分钟)
}CMDUserKickoutRoomInfo_t;

typedef struct tag_CMDUserKickoutRoomInfo_ext
{
	uint32 vcbid;
	uint32 srcid;
	uint32 toid;
	int32  resonid;
	byte   mins;         //提出时间id,自输入(0~255分钟)
	uint16 textlen;     // the length of content
	char   content[0];   //user ip and gate ip
}CMDUserKickoutRoomInfo_ext_t;

//用户帐户消息(应答,通知)
typedef struct tag_CMDUserAccountInfo
{
	uint32 vcbid;   //用户所在的房间
	uint32 userid;
	int64 nk;       //
	int64 nb;
	uint32 dtime;   //时间
}CMDUserAccountInfo_t;

//封杀用户信息(req, notify)
typedef struct tag_CMDThrowUserInfo
{
	uint32 vcbid;
	uint32 runnerid;
	uint32 toid;
	byte   viplevel;       //(roomsvr填写)
	byte   nscopeid;       //封杀范围 :1-房间,2-全站
	byte   ntimeid;        //封杀时长
	byte   nreasionid;     //封杀理由
	char   szip[IPADDRLEN];       //(roomsvr填写)
	char   szserial[IPADDRLEN];   //(roomsvr填写)
}CMDThrowUserInfo_t;

typedef struct tag_CMDThrowUserInfoResp
{
	uint32 vcbid;
	int32 errorid;
}CMDThrowUserInfoResp_t;

//麦克状态结构体
//用户上、下麦时使用,2013.09.10 使用新的方式,可能是直接抱上麦的,可能是从麦时上报上麦的,那么此次抱麦消耗不消耗麦时(删除排麦id)?
// 或者,流程统一,如果在排麦模式下(房间打开允许排麦),抱用户就是把用户放到排麦列表中的第一个(等待自动转换?)
// 或者,设置, 排麦模式下就没有抱麦功能? 
typedef struct tag_CMDUserMicState
{
	uint32 vcbid;
	uint32 runid;
	uint32 toid;      //上麦用户
	int32  giftid;    //使用礼物id
	int32  giftnum;   //礼物num
	char   micstate;  //麦克状态,注意下面的麦状态标志
	char   micindex;  //麦克序号(notify中有数据),新修改,上麦时可以指定麦序(公麦时0~2), -1:不指定
	char   optype;    //操作类型,注意: 加入了该字段后就表示这个信令不是简单的上麦了
	//gcz++ 要新加参数,是不是从排麦中(第几个micid, 0-表示不是, 1..n 就是第几个麦id) 转成公麦的(如果是newmic=公麦)
	char   reserve11; //空白
}CMDUserMicState_t;

//设备状态结构体
//开启、关闭麦克风
typedef struct tag_CMDUserDevState
{
	uint32 vcbid;
	uint32 userid;
	byte   audiostate;  //设备状态 0-不处理, 1-声音输入静音 (用户禁止share),2-声音未静音(正常)
	byte   videostate;  //设备状态,0-不处理, 3-无视频设备 ,2-有视频设备(正常)， 1-视频关了(有视频设备,但发送者禁止发送数据)(用户禁止share)
	uint32 userinroomstate;  //最新合集状态(客户端根据需要采用)
}CMDUserDevState_t;

//用户昵称更新 (req,resp,noty都使用该结构)
typedef struct tag_CMDUserAliasState
{
	uint32 vcbid;
	uint32 userid;
	char   headurl[URLLEN];
	char   alias[NAMELEN];
}CMDUserAliasState_t;

//用户权限操作 (req, notify)
//16 Byte
typedef struct tag_CMDUserPriority
{
	uint32 vcbid;       //房间id
	uint32 runnerid;    //操作人员id
	uint32 userid;      //被操作人员id
	byte   action;		//动作 （1->加 2->卸）
	byte   priority;	//用户权限（1->管理员，2->临时管理员）
}CMDUserPriority_t;

typedef struct tag_CMDSetUserPriorityResp
{
	uint32 vcbid;
	int32 errorid;
}CMDSetUserPriorityResp_t;


//查看用户IP请求
typedef struct tag_CMDSeeUserIpReq
{
	uint32 vcbid;
	uint32 runid;
	uint32 toid;
}CMDSeeUserIpReq_t;

//查看用户IP应答
typedef struct tag_CMDSeeUserIpResp
{
	uint32 vcbid;
	uint32 runid;
	uint32 userid;
	//char  ip[IPADDRLEN];
	uint16 textlen;          //描述信息长度
	uint16 reserve;          //保留
	char  content[0];       //地址内容
}CMDSeeUserIpResp_t;

// 音视频请求消息
typedef struct tag_CMDTransMediaInfo
{
	uint32 vcbid;
	uint32 srcid;
	uint32 toid;
	byte   action;  //动作：1表示请求打开对方音视频，3表示请求关闭对方音视频。
	byte   vvflag;  //vv标志
}CMDTransMediaInfo_t;

// 房间信息消息,下发时使用
typedef struct tag_CMDRoomBaseInfo
{
	uint32 vcbid;     //只读
	uint32 groupid;   //房间组ID,只读
	byte   level;     //只读
	byte   busepwd;   //
	uint16 seats;     
	uint32 creatorid;
	uint32 op1id;
	uint32 op2id;
	uint32 op3id;
	uint32 op4id;
	uint32 opstate;   //使用bit,一共可以放置32个,第1位:是否允许悄悄话? 
	//                         第2位:是否允许仍塞子，上传和广播时均是混合结果数据.
	char   cname[NAMELEN]; //房间名
	char   cpwd[PWDLEN];  //密码(可能有)
}CMDRoomBaseInfo_t;

// 单独 修改房间状态消息(notify)
typedef struct tag_CMDRoomOpState
{
	uint32 vcbid;
	uint32 opstate;  //使用bit,一共可以放置32个,第1位:是否允许悄悄话? 
	//                         第2位:是否允许仍塞子，上传和广播时均是混合结果数据.
}CMDRoomOpState_t;

//修改房间媒体URL请求消息
typedef struct tag_CMDRoomMediaInfo
{
	uint32 vcbid;
	uint32 userid;
	char caddr[MEDIAADDRLEN];  //媒体服务器URL
}CMDRoomMediaInfo_t;


//设置公麦状态的请求消息
typedef struct tag_CMDChangePubMicStateReq
{
	uint32 vcbid;
	uint32 runnerid;   //操作人员id
	byte   micid;      //公麦id
	byte   optype;     //操作类型:1-延迟当前用户麦时间,2-设置麦属性
	int16  param1;     //麦时类型 或延迟时间(分)
}CMDChangePubMicStateReq_t;

//设置公麦状态的响应/错误消息
typedef struct tag_CMDChangePubMicStateResp
{
	uint32  vcbid;
	int32   errorid;    //错误代码
}CMDChangePubMicStateResp_t;

//设置公麦状态的通知消息
typedef struct tag_CMDChangePubMicStateNoty
{
	uint32  vcbid;
	uint32  runnerid;      //操作人员id
	byte    micid;
	byte    optype;        //同请求的操作类型
	int16   param1;        //麦时类型 或延迟时间(分) 
	uint32  userid;        //该麦的用户id,可能为0
	int16   userlefttime;  //麦用户剩余时间,
}CMDChangePubMicStateNoty_t;

//gch++
typedef struct tag_CMDUpWaitMic
{
	uint32  vcbid;
	uint32  ruunerid;
	uint32  touser;
	int32   nmicindex;   //-1,默认(插麦到最后一个);1-同时插入到第一个
}CMDUpWaitMic_t;

//设置排序用户index的请求
typedef struct tag_CMDOperateWaitMic
{
	uint32  vcbid;
	uint32  ruunerid;
	uint32  userid;
	int16   micid;      //该用户的第几个麦序
	int     optype;     //操作类型: -3,清除所有麦序?,-2 删除该用户的所有麦序,-1,删除该麦序,1-up,2-down,3-top,4-button
}CMDOperateWaitMic_t;

//设置排序用户index的响应/错误消息
typedef struct tag_CMDChangeWaitMicIndexResp
{
	uint32  vcbid;
	int32   errorid;     //错误代码
}CMDChangeWaitMicIndexResp_t;

//设置排序用户index的通知消息
typedef struct tag_CMDChangeWaitMicIndexNoty
{
	uint32 vcbid;
	uint32 ruunerid;
	uint32 userid;
	int16  micid;      //该用户的第几个麦序
	int    optype;     //操作类型: -3,清除所有人所有麦序?,-2 删除该用户的所有麦序,-1,删除该麦序,1-up,2-down,3-top,4-button
}CMDChangeWaitMicIndexNoty_t;



//夺用户公麦的请求消息
typedef struct tag_CMDLootUserMicReq
{
	uint32 vcbid;
	uint32 runnerid;
	uint32 userid;
	int16  micid;     //该用户所在的公麦id
}CMDLootUserMicReq_t;

//夺用户公麦的响应/错误消息
typedef struct tag_CMDLootUserMicResp
{
	uint32 vcbid;
	int32  errorid;    //错误代码
}CMDLootUserMicResp_t;

//夺用户公麦的通知消息
typedef struct tag_CMDLootUserMicNoty
{
	uint32 vcbid;
	uint32 runnerid;
	uint32 userid;
	int16  micid;   
}CMDLootUserMicNoty_t;

//设置房间信息的请求消息
typedef struct tag_CMDSetRoomInfoReq
{
	uint32 vcbid;   //只读
	uint32 runnerid;
	uint32 creatorid;
	uint32 op1id;
	uint32 op2id;
	uint32 op3id;
	uint32 op4id;
	int    busepwd; //是否启用密码
	char   cname[NAMELEN]; //房间名
	char   cpwd[PWDLEN];  //密码(可能有)
}CMDSetRoomInfoReq_t;

typedef struct tag_CMDSetRoomInfoReq_v2
{
	uint32 vcbid;             //只读
	uint32 runnerid;
	//进入房间设置
	int8 nallowjoinmode;
	//房间聊天设置
	int8 ncloseroom;
	int8 nclosepubchat;
	int8 nclosecolorbar;
	int8 nclosefreemic;
	uint8 noptstat;			//房间管理状态设置, bit0 : 1禁止观众录像
	int8 ncloseprvchat;
	char   cname[NAMELEN];  //房间名
	char   cpwd[PWDLEN];    //密码(可能有)
}CMDSetRoomInfoReq_v2_t;

//设置房间信息的应答消息
typedef struct tag_CMDSetRoomInfoResp
{
	uint32 vcbid;
	int32  errorid;    //错误代码
}CMDSetRoomInfoResp_t;

//设置房间运行状态/属性的请求消息
typedef struct tag_CMDSetRoomOPStatusReq
{
	uint32 vcbid;
	uint32 runnerid;
	uint32 opstatus;
}CMDSetRoomOPStatusReq_t;

//设置房间运行状态/属性的应答消息
typedef struct tag_CMDSetRoomOPStatusResp
{
	uint32 vcbid;
	int32  errorid;    //错误代码
}CMDSetRoomOPStatusResp_t;

//设置房间公告的请求消息
typedef struct tag_CMDSetRoomNoticeReq
{
	uint32 vcbid;
	uint32 ruunerid;
	byte   index;       //房间公告索引idx:0~2(0,1,2)
	uint16 textlen;     //房间公告长度
	char   content[0];   //文字内容
}CMDSetRoomNoticeReq_t;

//设置房间公告的应答消息
typedef struct tag_CMDSetRoomNoticeResp
{
	uint32 vcbid;
	int32  errorid;    //错误代码
}CMDSetRoomNoticeResp_t;


//盖章请求和被盖章通知/广播的数据体
typedef struct tag_CMDSendUserSeal
{
	uint32 userid;
	uint32 vcbid;
	uint32 toid;
	uint16 sealid;   
	uint16 sealtime; 
}CMDSendUserSeal_t;

typedef struct tag_CMDSendUserSealErr
{
	uint32 userid;
	uint32 vcbid;
	int32  errid;
}CMDSendUserSealErr_t;

//禁言请求消息和通知/广播的数据体
typedef struct tag_CMDForbidUserChat
{
	uint32 vcbid;
	uint32 srcid;
	uint32 toid;
	uint32 ttime;   //禁言时长
	byte   action;   //动作：1禁言 0解禁
}CMDForbidUserChat_t;

//把房间加入Favorite
typedef struct tag_CMDFavoriteRoomReq
{
	uint32 vcbid;
	uint32 userid;
	int    actionid;  //1:收藏, -1:取消收藏
}CMDFavoriteRoomReq_t;

typedef struct tag_CMDFavoriteRoomResp
{
	int32  errorid;    //错误代码
	uint32 vcbid;
	int  actionid; //动作,同收藏房间
}CMDFavoriteRoomResp_t;

//中奖礼物通知消息
//该消息只限制同房间内广播,因此不需要srcalias,由于最多可能20次中奖,因此有重复的中奖率(如10倍中了2次),
//因此使用下面的内容结构,最后是变长的中奖数据。
typedef struct tag_CMDLotteryGiftNotice
{
	uint32 vcbid;   //房间id
	uint32 srcid;   //userid
	uint16 giftid;  //礼物id
	uint16 noddsnum;   //后续的中奖记录数目
	char content[0];   //中奖内容:[int16|int16]... 奖率|次数
}CMDLotteryGiftNotice_t;

//中奖爆炸礼物通知消息
typedef struct tag_CMDBoomGiftNotice
{
	uint32 vcbid;
	uint32 srcid;
	uint32 giftid; //礼物id 
	int    beishu; //倍数
	uint64 winmoney;  //爆炸中奖总额
}CMDBoomGiftNotice_t;

//幸运奖池通知消息
typedef struct tag_CMDLotteryPoolInfo
{
	uint64 nlotterypool;
}CMDLotteryPoolInfo_t;

//捡烟花加钱请求消息
typedef struct tag_CMDTradeFireworksReq
{
	uint32 vcbid;
	uint32 srcid;
	uint16 giftid;   
	uint16 giftnum;
	uint16 sendtype;   //1.大烟花,2.小烟花
	char   srcalias[NAMELEN]; 
}CMDTradeFireworksReq_t;

//捡烟的通知消息
typedef struct tag_CMDTradeFireworksNotify
{ 
	uint32 vcbid;
	uint32 srcid;
	uint16 giftid;   
	uint16 giftnum;
	uint16 sendtype;   //1.大烟花,2.小烟花
	char   srcalias[NAMELEN]; 
}CMDTradeFireworksNotify_t;

typedef struct tag_CMDTradeFireworksErr
{ 
	uint32 vcbid;
	uint32 userid;
	uint32 giftid;
	int32  errid;
}CMDTradeFireworksErr_t;


//银行存取款
typedef struct tag_CMDMoneyAndPointOp
{
	uint32 vcbid;
	uint32 srcid; 
	uint32 touserid;   //3-用到
	int64 data;        //金额/积分数目
	uint8 optype;      //1 银行存款 2 银行取款   3 转账  4 积分兑换金币
}CMDMoneyAndPointOp_t;

typedef struct tag_CMDSetRoomWaitMicMaxNumLimit
{
	uint32 vcbid;
	uint32 runnerid; 
	uint32 maxwaitmicuser; //最大排麦人数
	uint32 maxuserwaitmic; //每人最多排麦次数
}CMDSetRoomWaitMicMaxNumLimit_t;

typedef struct tag_CMDSetForbidInviteUpMic
{
	uint32 vcbid;
	uint32 userid;
	int32  reserve;
}CMDSetForbidInviteUpMic_t;

typedef struct tag_PropsFlashPlayTaskItem
{
	short nTaskType;
	short nArg;
}PropsFlashPlayTaskItem_t;//, *PPropsFlashPlayTaskItem_t;

//查询房间存在消息
typedef struct tag_CMDQueryVcbExistReq
{
	uint32 vcbid;
	uint32 userid;
	uint32 queryvcbid;   //要查询的vcbid
}CMDQueryVcbExistReq_t;

typedef struct tag_CMDQueryVcbExistResp
{
	int32 errorid;  //0-没有错误
	uint32 vcbid;
	uint32 userid;
	uint32 queryvcbid;
	char   cqueryvcbname[NAMELEN];
}CMDQueryVcbExistResp_t;

//查看用户是否存在消息
typedef struct tag_CMDQueryUserExistReq
{
	uint32 vcbid;   
	uint32 userid; 
	uint32 queryuserid;  //要查询的userid
	uint32 specvcbid;   //要在指定的房间查询
}CMDQueryUserExistReq_t;

typedef struct tag_CMDQueryUserExistResp
{
	int32 errorid;  //0-没有错误
	uint32 vcbid;   
	uint32 userid;
	uint32 queryuserid;
	uint32 specvcbid; 
	byte   queryuserviplevel;
	char   cspecvcbname[NAMELEN];  //如果指定房间
	char   cqueryuseralias[NAMELEN];
}CMDQueryUserExistResp_t;

typedef struct tag_CMDOpenChestReq
{
	uint32 vcbid;
	uint32 userid;
	int32  openresult_type;   //开奖类型, 0-单开, 1-全开
}CMDOpenChestReq_t;

typedef struct tag_CMDOpenChestResp
{
	int32 errorid;   //错误 !=0
	uint32 vcbid;
	uint32 userid;
	int32 usedchestnum;   //使用掉的宝箱
	int32 remainchestnum;  //剩余的宝箱
	int32 openresult_type;   //开奖类型 - 对应请求中数据
	int32 openresult_0;      //单次奖项的数据idx
	int32 openresult_1[7];   //7个奖项的次数,注意,特等奖只会产生一次
	int64 poolvalue;         //剩余奖池数目
	int64 tedengvalue;       //特等奖结果
}CMDOpenChestResp_t;

typedef struct tag_CMDMobZhuboInfo
{
	uint32 vcbid;
	uint32 userid;
	char   alias[NAMELEN];
	char   headurl[URLLEN];
}CMDMobZhuboInfo_t;

//3.3 gch++ 用户财富消费排行等级实时更新
typedef struct tag_CMDUserCaifuCostLevelInfo
{
	uint32 userid;
	uint32 vcbid;
	int32  ncaifulevel;
	int32  nlastmonthcostlevel;    //上月消费排行
	int32  nthismonthcostlevel;    //本月消费排行
	int32  nthismonthcostgrade;    //本月累计消费等级
}CMDUserCaifuCostLevelInfo_t;

//gch++ 用户详细信息
typedef struct tag_CMDUserMoreInfo
{
	int userid;
	byte  birthday_day;
	byte  birthday_month;
	byte  gender;
	byte  bloodgroup;
	int16 birthday_year;
	char  country[NAMELEN];
	char  province[NAMELEN];
	char  city[NAMELEN];
	byte  moodlength;
	byte  explainlength;
	char content[0];
}CMDUserMoreInfo_t;

typedef struct tag_CMDSetUserMoreInfoResp
{
	uint32 userid;
	int32  errorid;    //错误代码
}CMDSetUserMoreInfoResp_t;

typedef struct tag_CMDQueryUserMoreInfo
{
	uint32 srcid;
	uint32 vcbid;
	uint32 toid;
	int32  errorid;    //错误代码
}CMDQueryUserMoreInfo_t;

typedef struct tag_CMDQuanxianId2Item
{
	int16 levelid;  //可(-)
	int16 quanxianid;  //可(-)
	uint8 quanxianprio;
	uint16 sortid;
	uint8 sortprio;
}CMDQuanxianId2Item_t;

typedef struct tag_CMDQuanxianAction2Item
{
	uint16 actionid;
	int8 actiontype;
	int16 srcid;
	int16 toid;
}CMDQuanxianAction2Item_t;

typedef struct tag_CMDCloseGateObjectReq
{
	uint64 object;
	uint64 objectid;
}CMDCloseGateObjectReq_t;

typedef struct tag_CMDCloseRoomNoty
{
	uint32 vcbid;
	char closereason[URLLEN];
}CMDCloseRoomNoty_t;

typedef struct tag_CMDClientPingResp
{
	uint32 userid;        //用户id
	uint32 roomid;        //房间id
}CMDClientPingResp_t;

typedef struct tag_CMDQueryRoomGateAddrReq
{
	uint32 userid;
	uint32 roomid;
	uint32 flags;    //自定义传输参数，返回带入
}CMDQueryRoomGateAddrReq_t;

typedef struct tag_CMDQueryRoomGateAddrResp
{
	uint32 errorid;   //错误id
	uint32 userid;
	uint32 roomid;
	uint32 flags;    //自定义传输参数
	int16 textlen;   //地址长度变长
	char content[0];
}CMDQueryRoomGateAddrResp_t;

typedef struct tag_CMDSetUserHideStateReq
{
	uint32 userid;
	uint32 vcbid;
	int32  hidestate;    //1-toHide, 2-tounHide 
}CMDSetUserHideStateReq_t;

typedef struct tag_CMDSetUserHideStateResp
{
	uint32 errorid;   //错误id
}CMDSetUserHideStateResp_t;

typedef struct tag_CMDSetUserHideStateNoty
{
	uint32 userid;
	uint32 vcbid;
	uint32 inroomstate;  //最终状态
}CMDSetUserHideStateNoty_t;

typedef struct tag_CMDUserAddChestNumNoty
{
	uint32 userid;
	uint32 vcbid;
	uint32 addchestnum; //新增宝箱数目
	uint32 totalchestnum; //共有宝箱
}CMDUserAddChestNumNoty_t;

//中奖倍数次数
typedef struct tag_JiangCiShu
{
	int beishu;
	int count;
}JiangCiShu_t;

//增加密友通知
typedef struct tag_CMDAddClosedFriendNotify
{
	uint32 userid;
	uint32 vcbid;
}CMDAddClosedFriendNotify_t;

// 关键字屏蔽
typedef struct tag_AdKeywordInfo
{
	int		naction;				//1-增加 2-modify 3-删除
	int		ntype;					//广告类型
	int		nrunerid;				//操作人Id
	char	createtime[32];			//创建时间
	char	keyword[64];			//关键词
	char 	replace[64];			//replace
}CMDAdKeywordInfo_t;


// 关键字操作请求
typedef struct tag_CMDAdKeywordsReq{
	int num;
    char keywod[0];
}CMDAdKeywordsReq_t;

//关键字操作回应
typedef struct tag_CMDAdKeywordsResp{
	int errid;				// 0 代表成功，否则失败
	uint32 userid;	
}CMDAdKeywordsResp_t;

//关键字操作广播(也作为用户加入房间后，客户端第一次获取关键字列表的请求）
typedef struct tag_CMDAdKeywordsNotify{	
	int num;
    char keywod[0];
}CMDAdKeywordsNotify_t;

//讲师请求打分
typedef struct tag_CMDTeacherScoreReq
{
    uint32 teacher_userid;             //讲师ID
    char  teacheralias[NAMELEN]; //讲师呢称
    uint32  vcbid;                //所在房间id
    int64 data1;                  //备用字段1
    char   data2[NAMELEN];        //备用字段2
}CMDTeacherScoreReq_t;

//讲师请求打分结果
typedef struct tag_CMDTeacherScoreResp
{
    int    type;                  //操作是否成功
    uint32 teacher_userid;             //讲师ID
    char   teacheralias[NAMELEN]; //讲师呢称
    int    vcbid;                 //房间id，主房间或子房间
}CMDTeacherScoreResp_t;

//讲师打分记录
typedef struct tag_CMDTeacherScoreRecordReq
{
    uint32 teacher_userid;             //讲师ID
    char   teacheralias[NAMELEN]; //讲师呢称
    uint32 userid;                //打分人ID
    char   alias[NAMELEN];        //打分人呢称
    byte   usertype;              //讲师类型: 0-普通 1-机器人
    uint32 score;                 //分数
    char   logtime[NAMELEN];      //打分时间
    uint32  vcbid;                //所在房间id
    int64 data1;                  //备用字段1
    int64 data2;                  //备用字段2
    int64 data3;                  //备用字段3
    char   data4[NAMELEN];        //备用字段4
    char   data5[NAMELEN];        //备用字段5
}CMDTeacherScoreRecordReq_t;

//讲师打分结果
typedef struct tag_CMDTeacherScoreRecordResp
{
    uint32 teacher_userid;             //讲师ID
    char   teacheralias[NAMELEN]; //讲师呢称
    int    type;                  //操作是否成功
}CMDTeacherScoreRecordResp_t;

//机器人对应讲师ID通知
typedef struct tag_CMDRoborTeacherIdNoty
{
    uint32 vcbid;	//房间id
    uint32 roborid;//机器人id
    uint32 teacherid;             //讲师ID
    char   teacheralias[NAMELEN]; //讲师呢称
}CMDRoborTeacherIdNoty_t;

//讲师忠实度周版请求
typedef struct tag_CMDTeacherGiftListReq
{
	uint32 vcbid;					//房间id
	uint32 teacherid;				//讲师ID
}CMDTeacherGiftListReq_t;

//讲师忠实度周版响应
typedef struct tag_CMDTeacherGiftListResp
{
	uint32 seqid;                   //把排名也发过去
	uint32 vcbid;					//房间id
	uint32 teacherid;				//讲师ID
	char useralias[NAMELEN];	    //用户昵称
	uint64 t_num;					//忠实度值
}CMDTeacherGiftListResp_t;

//用户进入房间或者讲师上麦，传送一个房间和子房间ID
typedef struct tag_CMDRoomAndSubRoomIdNoty
{
	uint32 roomid;                 //主房间ID
	uint32 subroomid;              //子房间ID
}CMDRoomAndSubRoomIdNoty_t;

typedef struct tag_CMDTeacherAvarageScoreNoty
{
	uint32 teacherid;
	uint32 roomid;
	float avarage_score;
	char data1[NAMELEN]; //备用字段1
	uint32 data2; //备用字段2
}CMDTeacherAvarageScoreNoty_t;

// for roomsvr notification
typedef enum {
	UNKNOWN_ACTION = -1,
	USER_JOIN = 0,
	USER_LEFT = 1,
	USER_UPDATE = 2,
	MIC_UPDATE = 3,
    HVALS_INIT          = 5,   
	HVALS_INSERT        = 6,
	HVALS_FIND          = 7,
	HVALS_DELETE        = 8,
	ROOM_INFO_UPDATE    = 9,
	PHP_ROOM_MGR        = 10,
	KEY_WORD            = 11,
	BASIC_SYNC          = 12,
	PTCOURSE_MGR		= 13,
	USERINFO_LOAD		= 14,
}ROOM_ACTION_NOTIFY;

typedef struct tag_CMDRoomsvrNotify
{
	uint32 vcbid;
	uint32 userid;
	byte   action; //the value from ACTION_NOTIFY type
}CMDRoomsvrNotify_t;

typedef struct tag_CMDAddrNotify
{
	char ip[32];
	uint16 port;
}CMDAddrNotify_t;

//用户退出程序给出的提示
typedef struct tag_CMDExitAlertReq
{
	int32 userid;                      //用户ID
}CMDExitAlertReq_t;

typedef struct tag_CMDExitAlertResp
{
	int32 userid;
	char email[32];                 //邮箱
	char qq[32];                    //qq号码
	char tel[32];                   //手机号码
	int32 hit_gold_egg_time;        //当天内砸蛋的次数
	int32 data1;
	int32 data2;
	int32 data3;
	char data4[32];
	char data5[32];
}CMDExitAlertResp_t;

//用户安全信息查询请求
typedef struct tag_CMDSecureInfoReq
{
	int32 userid;                      //用户ID
}CMDSecureInfoReq_t;

//用户安全信息查询响应
typedef struct tag_CMDSecureInfoResp
{
	char email[32];                 //邮箱
	char qq[32];                    //qq号码
	char tel[32];                   //手机号码
	int32 remindtime;                 //已提醒次数
	int32 data1;
	int32 data2;
	int32 data3;
	char data4[32];
	char data5[32];
	char data6[32];
}CMDSecureInfoResp_t;

//报告媒体服务器
typedef struct tag_CMDReportMediaGateReq
{
    uint32 vcbid;
    uint32 userid;
    uint16 textlen;          //下面内容的长度
    char  content[0];       //ip:port的形式，分号作为分隔符，用“|”竖号分隔网关服务器地址和媒体服务器地址
}CMDReportMediaGateReq_t;
//报告媒体服务器回应
typedef struct tag_CMDReportMediaGateResp
{
    uint32 vcbid;
    uint32 userid;
    int errid;
}CMDReportMediaGateResp_t;

//web端砸完金蛋的通知消息
typedef struct tag_CMDHitGoldEggWebNoty
{
    uint32 vcbid;
    uint32 userid;
}CMDHitGoldEggWebNoty_t;
//砸完金蛋通知客户端
typedef struct tag_CMDHitGoldEggClientNoty
{
    uint32 vcbid;
    uint32 userid;
    uint64 money;
}CMDHitGoldEggClientNoty_t;

//用户对讲师的评分响应
typedef struct tag_CMDUserScoreNoty
{
    uint32 vcbid;					//房间号
    uint32 teacherid;				//讲师ID
    int32 score;					//用户对讲师的评分
	int32 userid;					//用户ID
}CMDUserScoreNoty_t;

//重设客户端的连接信息（用于gate与roomsvr断开时）
typedef struct tag_CMDResetConnInfo
{
    uint32 vcbid;					//房间号
	int32 userid;					//用户ID
}CMDResetConnInfo_t;

typedef struct tag_CMDUserOnlineBaseInfoNoty
{
	//0=>手机号，1=>用户名，2=>微信，3=>微博，4=>QQ',
	uint32 userid;
	uint32 sessionid;
	uint32 devicetype;
	char   cIpAddr[IPADDRLEN];		//ip地址
	uint32 logontime;				//登陆
	uint32 exittime;				//离开时间
	byte platformType;        //platform type,for example:1-QQ,2-weibo,3-weixin
	char   cloginid[NAMELEN];
	char   uuid[URLLEN];			//uuid
}CMDUserOnlineBaseInfoNoty_t;

typedef struct tag_CMDLogonStasticsReq
{
	uint32 userid;
	uint32 device_type;
	char   cIpAddr[IPADDRLEN];  //ip地址
}LogonStasticsReq_t;

//终端登录信息
typedef struct tag_CMDLogonClientInf
{
	uint32 m_userid;
	byte   m_bmobile;
	uint32 m_logontime;
	uint32 m_connid;
}CMDLogonClientInf_t;

typedef struct tag_CMDClientExistNot
{
    uint32 userid;             //用户ID
	byte m_ntype;                //退出类型0 重复登录
}CMDClientExistNot_t;

////////////////////////////////////////
//信箱小红点提醒（服务器主动推送）
typedef struct tag_CMDMessageNoty
{
	int32 userid;					//用户ID
}CMDMessageNoty_t;

//信箱未读记录数提醒
typedef struct tag_CMDMessageUnreadRes
{
	int32 userid;				      //用户ID
	int8  teacherflag;                //是否讲师（0-不是，1-是）
	int32 chatcount;                  //互动回复未读记录数
	int32 viewcount;                  //观点回复未读记录数
	int32 answercount;                //问答提醒未读记录数
	int32 syscount;                   //系统信息未读记录数
}CMDMessageUnreadRes_t;

//查看邮箱（不同分类请求用同一个消息类型及结构）
typedef struct tag_CMDHallMessageReq
{
	uint32 userid;                 //用户ID
	int8   teacherflag;            //是否讲师（0-不是，1-是）
	int16  type;                   //分类：1.互动回复（11.收到的互动（用户）；12.发出的互动（讲师）；）；
							   	   //2.观点回复（21.收到的观点回复；22.发出的观点评论；）；
							   	   //3.问答提醒（31. 未回答提问；32.已回答提问；）；
							   	   //4.我的关注（41.我的粉丝；42.我的关注；43.明日预测）；
							   	   //5.系统信息；
	int64  messageid;              //请求得到的最大消息ID，第一次为0
	int32  startIndex;             //起始索引
	int16  count;                  //请求记录数
}CMDHallMessageReq_t;

//查看互动回复，查看问答提醒
typedef struct tag_CMDInteractRes
{
	int16  type;                   //分类：互动回复（11.收到的互动（用户）；12.发出的互动（讲师）；）； 问答提醒（31. 未回答提问；32.已回答提问；）；
	uint32 userid;                 //请求用户ID
	uint32 touserid;               //互动用户ID
	char   touseralias[NAMELEN];   //互动用户昵称
	uint32 touserheadid;           //互动用户头像
	int64  messageid;              //消息ID（用于回复时找到对应的记录）
	int16  sortextlen;             //源内容长度
	int16  destextlen;             //回复内容长度，type=3时值为0表示未回答的提问，否则表示已回答的提问；
	uint64 messagetime;            //回复时间(yyyymmddhhmmss)
    int8   commentstype;           //客户端类型 0:PC端 1:安卓 2:IOS  3:WEB
	char   content[0];             //消息内容，格式：源内容+回复内容
}CMDInteractRes_t;

//查看观点回复
typedef struct tag_CMDViewShowRes
{
	int16  type;                   //分类： 观点回复（21.收到的观点回复；22.发出的观点评论；）；
	uint32 userid;                 //请求用户ID
	uint32 touserid;               //互动用户ID
	char   useralias[NAMELEN];     //互动用户昵称
	uint32 userheadid;             //互动用户头像
	int64  commentid;              //评论ID（用于回复时找到对应的记录）
	int16  viewTitlelen;           //观点标题长度
	int16  viewtextlen;            //观点内容长度
	int16  srctextlen;             //评论内容长度
	int16  replytextlen;           //回复内容长度，对于用户值为0表示发出的观点评论，否则表示收到的观点回复；
				       	   	   	   //对于讲师值为0表示收到的观点评论，否则表示发出的观点回复;
	uint64 restime;                //回复时间
	int8   commentstype;           //客户端类型 0:PC端 1:安卓 2:IOS  3:WEB
	char   content[0];             //消息内容，格式：观点标题+观点内容+评论内容+回复内容
}CMDViewShowRes_t;

//查看我的粉丝
typedef struct tag_CMDTeacherFansRes
{
	uint32 teacherid;              //讲师ID
	uint32 userid;                 //用户ID
	char   useralias[NAMELEN];     //用户昵称
	uint32 userheadid;             //用户头像
}CMDTeacherFansRes_t;

//查看我的关注（已关注讲师）
typedef struct tag_CMDInterestRes
{
	uint32 userid;                 //用户ID
	uint32 teacherid;              //讲师ID
	char   teacheralias[NAMELEN];  //讲师昵称
	uint32 teacherheadid;          //讲师头像
	int16  levellen;               //讲师等级长度
	int16  labellen;               //讲师标签长度
	int16  introducelen;           //讲师简介长度
	char   content[0];             //消息内容，格式：讲师等级+讲师标签+讲师简介
}CMDInterestRes_t;

//查看我的关注（无关注讲师）
typedef struct tag_CMDUnInterestRes
{
	uint32 userid;                 //用户ID
	uint32 teacherid;              //讲师ID
	char   teacheralias[NAMELEN];  //讲师昵称
	uint32 teacherheadid;          //讲师头像
	int16  levellen;               //讲师等级长度
	int16  labellen;               //讲师标签长度
	int16  goodatlen;              //讲师擅长领域长度
	int64  answers;                //已回答问题的数目
	char   content[0];             //消息内容，格式：讲师等级+讲师标签+讲师擅长领域
}CMDUnInterestRes_t;

//查看明日预测（已关注的讲师）
typedef struct tag_CMDTextLivePointListRes
{
	uint32 userid;                 //用户ID
	uint32 teacherid;              //讲师ID
	char   teacheralias[NAMELEN];  //讲师昵称
	uint32 teacherheadid;          //讲师头像
	int64  messageid;              //消息ID
	int16  livetype;               //文字直播类型：1-纯文字；2-文字+链接；3-文字+图片;
	int16  textlen;                //消息长度
	int8   commentstype;           //客户端类型 0:PC端 1:安卓 2:IOS  3:WEB
	uint64 messagetime;            //发送时间(yyyymmddhhmmss)
	char   content[0];             //消息内容
}CMDTextLivePointListRes_t;

//讲师回复（包含观点回复和回答提问）
typedef struct tag_CMDViewAnswerReq
{
	uint32 fromid;              //发出人
	uint32 toid;                //接收人
	int16  type;                //分类：1.互动回复；2.观点回复；3.问答提醒；（互动无回复）
	int64  messageid;           //消息ID
	int16  textlen;             //观点回复长度
	int8   commentstype;        //客户端类型 0:PC端 1:安卓 2:IOS  3:WEB
	char   content[0];          //观点回复内容
}CMDViewAnswerReq_t;

//讲师回复响应
typedef struct tag_CMDViewAnswerRes
{
	int32 userid;				    //用户ID
	int16 type;                     //分类：1.互动回复；2.观点回复；3.问答提醒；
	int64 messageid;                //创建成功的对应消息ID
	int16 result;                   //回复是否成功：0失败，1成功；
}CMDViewAnswerRes_t;

//关注（无关注讲师时返回所有讲师列表，点击关注）
typedef struct tag_CMDInterestForReq
{
	uint32 userid;                 //用户ID
	uint32 teacherid;              //讲师ID
	int16  optype;                 //操作类型：1是关注，2是取消关注
}CMDInterestForReq_t;

//返回讲师的粉丝总数
typedef struct tag_CMDFansCountReq
{
    uint32 teacherid;              //讲师ID
}CMDFansCountReq_t;

typedef struct tag_CMDFansCountRes
{
    uint32 teacherid;              //讲师ID
    uint64 fansCount;              //粉丝总数
}CMDFansCountRes_t;

//关注响应
typedef struct tag_CMDInterestForRes
{
	int32 userid;				  //用户ID
	int16 result;                 //回复是否成功：0失败，1成功；
}CMDInterestForRes_t;

//加入房间成功后推送讲师信息请求
typedef struct tag_CMDTextRoomTeacherReq
{
	uint32 vcbid;                  //房间ID
	uint32 userid;                 //用户ID
}CMDTextRoomTeacherReq_t;

//加入房间成功后推送讲师信息响应
typedef struct tag_CMDTextRoomTeacherNoty
{
	uint32 vcbid;                  //房间ID
	uint32 userid;                 //用户ID
	uint32 teacherid;              //讲师ID
	char   teacheralias[NAMELEN];  //讲师昵称
	uint32 headid;                 //讲师头像ID
	int16  levellen;               //讲师等级长度
	int16  labellen;               //讲师标签长度
	int16  goodatlen;              //讲师擅长领域长度
	int16  introducelen;           //讲师简介长度
	int64  historymoods;           //直播人气数
	int64  fans;                   //粉丝数
    int64  zans;                   //点赞数
	int64  todaymoods;             //今日人气数
	int64  historyLives;           //直播历史数
	int16  liveflag;               //是否直播中（0-不在线；1-在线）
	int16  fansflag;               //是否已经关注讲师（0-未关注；1-已关注）
	int16  bstudent;               //是否已经拜师（0-未拜师；1-已拜师）
	int32   rTeacherLiveRoom;			//是否视频直播中
	char   content[0];             //消息内容，格式：讲师等级+讲师标签+讲师擅长领域（多个以分号分隔）+讲师简介
}CMDTextRoomTeacherNoty_t;

//加载直播记录请求
typedef struct tag_CMDTextRoomLiveListReq
{
	uint32 vcbid;                  //房间ID
	uint32 userid;                 //用户ID
	uint32 teacherid;              //讲师ID
	int16  type;                   //类型：1-文字直播；2-直播重点；3-明日预测（已关注的用户可查看）；4-观点；5-所有秘籍；6-已购买秘籍；
	int64  messageid;              //上一次请求得到的最小消息ID，第一次为0
    int32  count;                  //获取多少条记录
}CMDTextRoomLiveListReq_t;

//加载直播记录响应
typedef struct tag_CMDTextRoomLiveListNoty
{
	uint32 vcbid;                  //房间ID
	uint32 userid;                 //用户ID
	uint32 teacherid;              //讲师ID
	uint32 srcuserid;              //互动用户ID
	char   srcuseralias[NAMELEN];  //互动用户昵称
	int64  messageid;              //消息ID
	int16  pointflag;              //是否直播重点:0-否；1-是；
	int16  forecastflag;           //是否明日预测:0-否；1-是；
	int16  livetype;               //文字直播类型：1-纯文字；2-文字+链接；3-文字+图片；4-互动回复；5-观点动态；6-个人秘籍动态；
	int64  viewid;     			   //观点ID(5-观点动态用)/秘籍ID(6-个人秘籍动态用)
	int16  textlen;                //消息长度
	int16  destextlen;             //互动回复长度
	uint64 messagetime;            //发送时间(yyyymmddhhmmss)
    int64  zans;                   //点赞数
    int8   commentstype;		   //客户端类型 0:PC端 1:安卓 2:IOS 3:WEB
	char   content[0];             //消息内容，格式：消息内容（发送的直播是内容，发送的观点是标题，互动的是源内容）+互动回复内容
}CMDTextRoomLiveListNoty_t;

//加入房间成功后推送直播重点和明日预测记录
typedef struct tag_CMDTextRoomLivePointNoty
{
	uint32 vcbid;                  //房间ID
	uint32 userid;                 //用户ID
	uint32 teacherid;              //讲师ID
	int64  messageid;              //消息ID
	int16  livetype;               //文字直播类型：1-纯文字；2-文字+链接；3-文字+图片;
	int16  textlen;                //消息长度
	uint64 messagetime;            //发送时间(yyyymmddhhmmss)
    int64  zans;                   //点赞数
    int8   commentstype;		   //客户端类型 0:PC端 1:安卓 2:IOS 3:WEB
	char   content[0];             //消息内容
}CMDTextRoomLivePointNoty_t;

//讲师发送文字直播请求
typedef struct tag_CMDTextRoomLiveMessageReq
{
	uint32 vcbid;                  //房间ID
	uint32 teacherid;              //讲师ID
	int16  pointflag;              //是否直播重点:0-否；1-是；
	int16  forecastflag;           //是否明日预测:0-否；1-是；
	int16  livetype;               //文字直播类型：1-纯文字；2-文字+链接；3-文字+图片；
	int16  textlen;                //直播消息长度
	uint64 messagetime;            //发送时间(yyyymmddhhmmss)
    int8   commentstype;		   //客户端类型 0:PC端 1:安卓 2:IOS 3:WEB
	char   content[0];             //直播消息内容
}CMDTextRoomLiveMessageReq_t;

//讲师发送文字直播响应
typedef struct tag_CMDTextRoomLiveMessageRes
{
	uint32 vcbid;                  //房间ID
	uint32 teacherid;              //讲师ID
	int64  messageid;              //消息ID
	int16  pointflag;              //是否直播重点:0-否；1-是；
	int16  forecastflag;           //是否明日预测:0-否；1-是；
	int16  livetype;               //文字直播类型：1-纯文字；2-文字+链接；3-文字+图片；
	int16  textlen;                //直播消息长度
	uint64 messagetime;            //发送时间(yyyymmddhhmmss)
    int8   commentstype;		   //客户端类型 0:PC端 1:安卓 2:IOS 3:WEB
	char   content[0];             //直播消息内容
}CMDTextRoomLiveMessageRes_t;

//用户点击关注请求
typedef struct tag_CMDTextRoomInterestForReq
{
	uint32 vcbid;                  //房间ID
	uint32 userid;                 //用户ID
	uint32 teacherid;              //讲师ID
    int16  optype;                 //操作类型：1-关注  2-取消关注
}CMDTextRoomInterestForReq_t;

//用户点击关注响应
typedef struct tag_CMDTextRoomInterestForRes
{
	uint32 vcbid;                  //房间ID
	uint32 userid;                 //用户ID
	int16  result;                 //回复是否成功：0失败；1成功；2已关注；
    uint32 teacherid;              //讲师ID
    int16  optype;                 //操作类型：1是关注，2是取消关注
}CMDTextRoomInterestForRes_t;

//查询用户商品请求
typedef struct tag_CMDTextRoomGetUserGoodStatusReq
{
	uint32 userid;               //用户ID
	uint32 salerid;              //老师ID
	uint32 type;                 //类型
	uint32 goodclassid;          //商品类型ID
}CMDTextRoomGetUserGoodStatusReq_t;

typedef struct tag_CMDTextRoomGetUserGoodStatusRes
{
	uint32 userid;              //用户ID
	uint32 salerid;             //老师ID
	uint32 num;                 //数量
	int16  result;							//0 成功 1 error
}CMDTextRoomGetUserGoodStatusRes_t;

//用户点击提问请求
typedef struct tag_CMDTextRoomQuestionReq
{
	uint32 vcbid;                  //房间ID
	uint32 teacherid;              //老师ID
	uint32 userid;                 //用户ID
	int16  stocklen;               //个股名称长度
	int16  textlen;                //问题描述长度
	int8   commentstype;		   		 //客户端类型 0:PC端 1:安卓 2:IOS 3:WEB
	uint64 messagetime;            //发送时间(yyyymmddhhmmss)
	int8	 isfree;								 //免费提问还是付费提问
	char   content[0];             //消息内容，格式：个股名称+问题描述
}CMDTextRoomQuestionReq_t;

//用户提问响应
typedef struct tag_CMDTextRoomQuestionRes
{
	uint32 vcbid;                  //房间ID
	uint32 userid;                 //请求用户ID
	int16  result;                 //操作是否成功：0失败，1成功；
	int64  messageid;              //消息ID（通用接口，失败时为0）
	uint64 nk;										 //用户余额/或剩余免费提问次数
} CMDTextRoomQuestionRes_t;

//用户操作响应
typedef struct tag_CMDTextRoomLiveActionRes
{
	uint32 vcbid;                  //房间ID
	uint32 userid;                 //请求用户ID
	int16  result;                 //操作是否成功：0失败，1成功；
	int64  messageid;              //消息ID（通用接口，失败时为0）
}CMDTextRoomLiveActionRes_t;

//用户点赞请求
typedef struct tag_CMDTextRoomZanForReq
{
	uint32 vcbid;                  //房间ID
	uint32 userid;                 //用户ID
	int64  messageid;              //消息ID
}CMDTextRoomZanForReq_t;

//用户点赞响应
typedef struct tag_CMDTextRoomZanForRes
{
	uint32 vcbid;                  //房间ID
	uint32 userid;                 //用户ID
	int16  result;                 //回复是否成功：0失败，1成功；
	int64  messageid;              //消息ID
	int64  recordzans;             //消息点赞数
	int64  totalzans;              //总点赞数
}CMDTextRoomZanForRes_t;

//聊天请求
typedef struct tag_CMDRoomLiveChatReq
{
	uint32 vcbid;                  //房间ID
	uint32 srcid;                  //讲话人ID
	uint32 toid;                   //用户ID
	byte   msgtype;                //私聊类型也在放这里
	uint64 messagetime;            //发送时间(yyyymmddhhmmss)
    uint16 textlen;                //聊天内容长度
    int8   commentstype;		   //客户端类型 0:PC端 1:安卓 2:IOS 3:WEB
	char   content[0];             //聊天内容
}CMDRoomLiveChatReq_t;

//聊天响应
typedef struct tag_CMDTextRoomLiveChatRes
{
	uint32 vcbid;                  //房间ID
	uint32 srcid;                  //讲话人ID
	char   srcalias[NAMELEN];      //讲话人昵称
	uint32 srcheadid;              //讲话人头像
	uint32 toid;                   //用户ID
	char   toalias[NAMELEN];       //用户昵称
	uint32 toheadid;               //用户头像
	byte   msgtype;                //私聊类型也在放这里
	uint64 messagetime;            //发送时间(yyyymmddhhmmss)
	uint16 textlen;                //聊天内容长度
    int8   commentstype;		   //客户端类型 0:PC端 1:安卓 2:IOS 3:WEB
	char   content[0];             //聊天内容
}CMDTextRoomLiveChatRes_t;

//聊天回复(互动)请求
typedef struct tag_CMDTextLiveChatReplyReq
{
	uint32 vcbid;                  //房间ID
	uint32 fromid;                 //回复人ID
	uint32 toid;                   //被回复人ID
	uint64 messagetime;            //互动时间(yyyymmddhhmmss)
	uint16 reqtextlen;             //源消息内容长度
	uint16 restextlen;             //回复内容长度
	int8  liveflag;                //是否发布到直播：0-否；1-是；
    	int8   commentstype;	       //客户端类型 0:PC端 1:安卓 2:IOS 3:WEB
	char   content[0];             //消息内容，格式：源消息内容 + 回复内容
}CMDTextLiveChatReplyReq_t;

//聊天回复(互动)响应
typedef struct tag_CMDTextLiveChatReplyRes
{
	uint32 vcbid;                  //房间ID
	uint32 fromid;                 //回复人ID
	char   fromalias[NAMELEN];     //回复人昵称
	uint32 fromheadid;             //回复人头像
	uint32 toid;                   //被回复人ID
	char   toalias[NAMELEN];       //被回复人昵称
	uint32 toheadid;               //被回复人头像
	uint64 messagetime;            //互动时间(yyyymmddhhmmss)
	uint16 reqtextlen;             //源消息内容长度
	uint16 restextlen;             //回复内容长度
	int8   liveflag;               //是否发布到直播：0-否；1-是；
    	int8   commentstype;	       //客户端类型 0:PC端 1:安卓 2:IOS 3:WEB
	int64  messageid;              //消息ID
	char   content[0];             //消息内容，格式：源消息内容 + 回复内容
}CMDTextLiveChatReplyRes_t;

//点击查看观点类型请求
typedef struct tag_CMDTextRoomLiveViewGroupReq
{
	uint32 vcbid;                  //房间ID
	uint32 userid;                 //用户ID
    uint32 teacherid;              //讲师ID
}CMDTextRoomLiveViewGroupReq_t;

//观点分类响应
typedef struct tag_CMDTextRoomViewGroupRes
{
	uint32 vcbid;                  //房间ID
	uint32 userid;                 //用户ID
    uint32 teacherid;              //讲师ID
	int32  viewtypeid;             //观点类型ID
	int32  totalcount;             //总观点数
	int16  viewtypelen;            //观点类型名称长度
	char   viewtypename[0];        //观点类型名称
}CMDTextRoomViewGroupRes_t;

//点击观点类型分类请求
typedef struct tag_CMDTextRoomLiveViewListReq
{
	uint32 vcbid;                  //房间ID
	uint32 userid;                 //用户ID
	uint32 teacherid;              //讲师ID
	int32  viewtypeid;             //观点类型ID，如果为0，则不分类返回
	int64  messageid;              //消息ID，为请求的观点概述列表的起点
	int32  startIndex;             //起始索引
	uint32 count;                  //观点概述的条数
}CMDTextRoomLiveViewListReq_t;

//点击观点类型分类请求(手机版本)，仅限20个
typedef struct tag_CMDTextRoomViewListReq_mobile
{
    char    uuid[16];               //唯一标识头
    uint32  vcbid;                  //房间ID
    uint32  userid;                 //用户ID
    uint32  teacherid;              //讲师ID
    int32   viewtypeid;             //观点类型ID，如果为0，则不分类返回
    int64   messageid;              //消息ID，为请求的观点概述列表的起点
    int32   startIndex;             //起始索引
    uint32  count;                  //观点概述的条数
}CMDTextRoomViewListReq_mobile;

//观点列表响应
typedef struct tag_CMDTextRoomLiveViewRes
{
	uint32 vcbid;                  //房间ID
	uint32 userid;                 //用户ID
	int32  viewtypeid;             //观点类型ID
	int64  viewid;                 //消息ID
	int16  livetype;               //文字直播类型：1-纯文字；2-文字+链接；3-文字+图片；
	int16  viewTitlelen;           //观点标题长度
	int16  viewtextlen;            //观点内容长度
	uint64 messagetime;            //发送时间(yyyymmddhhmmss)
	int64  looks;                  //浏览次数
    int64  zans;                   //点赞数
	int64  comments;               //评论数
	int64  flowers;                //送花数
    int8   commentstype;		   //客户端类型 0:PC端 1:安卓 2:IOS 3:WEB
	char   content[0];             //消息内容，格式：观点标题+观点内容
}CMDTextRoomLiveViewRes_t;

//查看观点详情请求
typedef struct tag_CMDTextRoomLiveViewDetailReq{

	uint32 vcbid;                  //房间ID
	uint32 userid;                 //用户ID
	int64  viewid;                 //观点ID
	int64  messageid;              //评论ID，为请求的观点概述列表的起点,从0起始计数 为零会同时发送观点详情包和评论列表，不为零则只发送评论列表
	int64  startcommentpos;        //起始索引
	uint32 count;                  //加载的评论条数
	int8   type;                   //请求类型，1观点详情，2观点评论，3观点详情+评论
}CMDTextRoomLiveViewDetailReq_t;

//查看评论详细信息
typedef struct tag_CMDTextRoomViewInfoRes
{
	uint32 vcbid;                  	//房间ID
	uint32 userid;                 	//请求用户ID
	int64  viewid;			        //观点ID
	int64  commentid;				//评论ID
	uint32 viewuserid;              //评论用户ID
	char   useralias[NAMELEN];		//评论的用户昵称，不对应上面的用户ID
	int16  textlen;                	//评论长度
	uint64 messagetime;            	//发送时间(yyyymmddhhmmss)
	int64  srcinteractid;			//源评论ID（回复评论内容时需要填写），0则代表没有
	char   srcuseralias[NAMELEN];  	//源评论的用户昵称，没有则为空
    int8   commentstype;			//客户端类型 0:PC端 1:安卓 2:IOS 3:WEB
	char   content[0];            	//评论内容
}CMDTextRoomViewInfoRes_t;


//讲师修改观点类型分类请求
typedef struct tag_CMDTextRoomViewTypeReq
{
	uint32 vcbid;                  //房间ID
	uint32 teacherid;              //请求用户ID
	int16  actiontypeid;           //操作类型ID：1-新增；2-修改；3-删除（需要删除分类下所有观点后才可操作）；
	int32  viewtypeid;             //观点类型ID（新增时为0）
	char   viewtypename[NAMELEN];  //观点类型名称
}CMDTextRoomViewTypeReq_t;

//讲师修改和新增观点类型分类响应
typedef struct tag_CMDTextRoomViewTypeRes
{
	uint32 vcbid;                  //房间ID
	uint32 teacherid;              //请求用户ID
	int16  result;                 //操作是否成功：0失败，1成功；
	int32  viewtypeid;             //观点类型ID（通用接口，失败时为0）
}CMDTextRoomViewTypeRes_t;

//讲师发布观点或修改观点请求
typedef struct tag_CMDTextRoomViewMessageReq
{
	uint32 vcbid;                  //房间ID
	uint32 userid;                 //用户ID
	int64  messageid;              //消息ID（修改时填入，新增时为0）
	int16  viewtype;               //文字直播类型：1-纯文字；2-文字+链接；3-文字+图片；
	int16  titlelen;               //观点标题长度
	int16  textlen;                //观点内容长度
	uint64 messagetime;            //发送时间(yyyymmddhhmmss)
    int8   commentstype;		   //客户端类型 0:PC端 1:安卓 2:IOS 3:WEB
    char   content[0];             //观点格式：观点标题+观点内容
}CMDTextRoomViewMessageReq_t;

//讲师发布观点或修改观点响应
typedef struct tag_CMDTextRoomViewMessageReqRes
{
	uint32 userid;                 //用户ID
	int64  messageid;              //消息ID（修改时填入，新增时为0）
	int16  titlelen;               //观点标题长度
	uint64 messagetime;            //发送时间(yyyymmddhhmmss)
	char   content[0];             //观点格式：观点标题
}CMDTextRoomViewMessageReqRes_t;

//讲师删除观点请求
typedef struct tag_CMDTextRoomViewDeleteReq
{
	uint32 vcbid;                  	//房间ID
	uint32 userid;                 	//用户ID
	int64  viewid;              		//观点类型ID
}CMDTextRoomViewDeleteReq_t;

//讲师删除观点响应
typedef struct tag_CMDTextRoomViewDeleteRes
{
	uint32 vcbid;                  	//房间ID
	uint32 userid;                 	//用户ID
	int64  viewid;              	//消息ID
	int16  result;                  //操作是否成功：0失败，1成功；
}CMDTextRoomViewDeleteRes_t;

//讲师通过PHP页面发布观点或修改观点或删除观点请求
typedef struct tag_CMDTextRoomViewPHPReq
{
	uint32 vcbid;                  //房间ID
	uint32 teacherid;              //请求用户ID
	int64  messageid;              //消息ID
    	int64  businessid;             //观点ID
	int8   viewtype;               //操作类型：1-新增；2-修改；3-删除；
	int16  titlelen;               //观点标题长度
	int16  textlen;                //观点简介长度
	uint64 messagetime;            //发送时间(yyyymmddhhmmss)
    	int8   commentstype;	       //客户端类型 0:PC端 1:安卓 2:IOS 3:WEB
    	char   content[0];             //格式：观点标题+观点简介
}CMDTextRoomViewPHPReq_t;

//讲师通过PHP页面发布观点或修改观点或删除观点响应
typedef struct tag_CMDTextRoomViewPHPRes
{
	uint32 vcbid;                  //房间ID
	uint32 teacherid;              //请求用户ID
	int64  messageid;              //消息ID
    	int64  businessid;             //观点ID
	int8   viewtype;               //操作类型：1-新增；2-修改；3-删除；
	int16  titlelen;               //观点标题长度
	int16  textlen;                //观点简介长度
	uint64 messagetime;            //发送时间(yyyymmddhhmmss)
    	int8   commentstype;	       //客户端类型 0:PC端 1:安卓 2:IOS 3:WEB
    	char   content[0];             //格式：观点标题+观点简介
}CMDTextRoomViewPHPRes_t;

//观点评论详细页送花请求
typedef struct tag_CMDTextLiveViewFlowerReq
{
	uint32 vcbid;                  //房间ID
	uint32 userid;                 //用户ID
	int64  messageid;              //消息ID
	int32  count;                  //送多少朵
}CMDTextLiveViewFlowerReq_t;

//观点评论详细页送花响应
typedef struct tag_CMDTextLiveViewFlowerRes
{
	uint32 vcbid;                  //房间ID
	uint32 userid;                 //用户ID
	int16  result;                 //回复是否成功：0失败，1成功；
	int64  messageid;              //消息ID
	int64  recordflowers;          //总送花数
}CMDTextLiveViewFlowerRes_t;

//对观点进行评论
typedef struct tag_CMDTextRoomViewCommentReq
{
	uint32 vcbid;                  //房间ID
	uint32 fromid;                 //评论人ID
	uint32 toid;                   //被评论人ID
	int64  messageid;              //消息ID
	int16  textlen;                //评论长度
	uint64 messagetime;            //发送时间(yyyymmddhhmmss)
	int8   commentstype;		   //客户端类型 0:PC端 1:安卓 2:IOS 3:WEB
	int64  srcinteractid;          //如果是对评论产生评论，需要填写这字段
	char   content[0];             //评论内容
}CMDTextRoomViewCommentReq_t;

//点击直播历史（可分页请求展示）请求
typedef struct tag_CMDTextLiveHistoryListReq
{
	uint32 vcbid;                  //房间ID
	uint32 userid;                 //用户ID
	uint32 teacherid;              //讲师ID
    int32  count;                  //获取多少条记录
	int16  fromIndex;              //从第几条开始取
	int16  toIndex;                //到第几条结束
	int32  fromdate;               //从哪一天(年月日)开始
	uint8  bInc;                   //是否升序（0降序,1升序）
}CMDTextLiveHistoryListReq_t;

//点击直播历史（可分页请求展示）响应
typedef struct tag_CMDTextLiveHistoryListRes
{
	uint32 vcbid;                  //房间ID
	uint32 userid;                 //用户ID
	uint32 teacherid;              //讲师ID
	uint32 datetime;               //日期(yyyymmdd)
	uint64 beginTime;              //一天内直播第一条记录的时间(yyyymmddhhmmss)
	uint64 endTime;                //一天内最后一条记录的时间(yyyymmddhhmmss)
	uint32 renQi;                  //当天人气
	uint32 cAnswer;                //回答问题的条数
	uint32 totalCount;             //当天直播记录总数
}CMDTextLiveHistoryListRes_t;

//请求某一天的直播记录列表（可分页请求展示）请求
typedef struct tag_CMDTextLiveHistoryDaylyReq
{
	uint32 vcbid;                  //房间ID
	uint32 userid;                 //用户ID
	uint32 teacherid;              //讲师ID
	int16  type;                   //类型：1-文字直播；2-直播重点；3-明日预测（已关注的用户可查看）；4-观点；
	int64  messageid;              //请求得到的最大消息ID，第一次为0
    int32  count;                  //获取多少条记录
    int32  startindex;             //起始索引
    uint32 datetime;               //哪一天的记录(yyyymmdd)
}CMDTextLiveHistoryDaylyReq_t;

//讲师进入房间广播消息
typedef struct tag_CMDTeacherComeNotify
{
	int64  recordzans;             //房间总点赞数
}CMDTeacherComeNotify_t;

//列表消息头(手机版本)，仅限20个
typedef struct tag_CMDTextRoomList_mobile
{
    char    uuid[16];               //唯一标识头
}CMDTextRoomList_mobile;

//房间推送系统公告
typedef struct tag_CMDSyscast
{
	unsigned char newType;  //0 一次性新闻 1    
	long long   nid;        //记录ID	
    char   title[32];    
	char   content[512]; 
}CMDSyscast;

//问答回复-个人信箱
typedef struct tag_CMDAnswerRes
{
    int16   type;                   //分类：互动回复（11.收到的互动（用户）；12.发出的互动（讲师）；）； 问答提醒（31. 未回答提问；32.已回答提问；）； 
	uint32  userid;                 //发送该请求的用户ID
    uint32  touserid;               //对端用户ID
	char    touseralias[NAMELEN];   //互动用户昵称
	uint32  touserheadid;           //互动用户头像
    int64   messageid;              //消息ID（用于回复时找到对应的记录）
	int16   answerlen;             //回复内容长度
	int16   stokeidlen;             //股票ID长度
    int16   questionlen;       //问题长度
	uint64  messagetime;      //回复时间(yyyymmddhhmmss)
    int8    commentstype;           //客户端类型 0:PC端 1:安卓 2:IOS  3:WEB
	char    content[0];             //消息内容，格式：回答+股票ID+问题
}CMDAnswerRes_t;

typedef struct tag_CMDSessionTokenReq
{
	uint32  userid;
}CMDSessionTokenReq_t;

typedef struct tag_CMDSessionTokenResp
{
	uint32  userid;
	char    sessiontoken[33];
    char    validtime[32];
}CMDSessionTokenResp_t;

//0:Both,1:Email,2:SMS
typedef enum {
    e_all_notitype    = 0,
    e_email_notitype  = 1,  
    e_sms_notitype    = 2,
    e_wechat_notitype = 3
}E_NOTICE_TYPE;

typedef enum {
    e_db_connect      = 0,
    e_configfile      = 1,
    e_msgqueue        = 2,
    e_network_conn    = 3,
	e_redis_connect   = 4,
	e_logic           = 5,
	e_data_sync		  = 6,
}E_ALARM_TYPE;

typedef struct tag_CmdAlarmNotify
{
    E_NOTICE_TYPE type;       //notice type
    E_ALARM_TYPE  alarmtype;  //alarm type
    char  groups[128];        //GroupName,split by ',' example:Test,Develop
    char  title[64];          //Title, now email need.
    char  content[512];       //content
}CmdAlarmNotify;

typedef struct tag_CmdPushGateMask
{
    uint32 userid;      //用户ID，广播用户填0
    byte termtype;      //登陆类型（0：PC，1：Android，2：IOS，3：Web，4：所有类型）
    byte type;          //业务类型：
                        //1. 配置服务器推送;
                        //2. 通知客户端打印日志;
                        //3. 客户端升级推送;
                        //4. 砸金蛋;
    byte needresp;      //是否需要确认
    uint16 validtime;   //有效时长
    byte versionflag;   //0：所有，1：等于，2：大于等于，3：小于等于
    uint32 version;     //版本号，0代表没有版本限制
    uint16 length;      //推送业务内容长度，比如砸金蛋结构体长度
    char content[0];    //推送业务内容，比如砸金蛋结构体内容
}CPushGateMask;


//1. 拜师信息请求
struct GetBeTeacherInfoNornalUserReq
{
	uint32  teacherid; //当前想拜讲师ID
};

struct GetBeTeacherInfoTeacherReq
{
	
};

typedef struct tag_CMDGetBeTeacherInfoReq
{
	uint8   bTeacherTag; //是否讲师
	uint32  userid; //请求该信息的用户ID
	uint32  vcbid; //房间ID
	union 
	{
		GetBeTeacherInfoNornalUserReq norlmalUserReq;
		GetBeTeacherInfoTeacherReq teacherReq;
	}userReqParam;
}CMDGetBeTeacherInfoReq_t;

//2. 拜师信息请求返回
struct GetBeTeacherInfoNornalUserRespHead
{
	uint32  teacherid; //当前想拜讲师ID
	uint32  cStudent; //当前讲师徒弟总数
	uint8   bStudent; //您是否他的徒弟
};

struct GetBeTeacherInfoTeacherRespHead
{
	
};

typedef struct tag_CMDGetBeTeacherInfoRespHead
{
	int32 userid;   //请求者ID
    int32 price_30;
	int32 price_90;
    union
    {
		GetBeTeacherInfoNornalUserRespHead normaluserHead;
		GetBeTeacherInfoTeacherRespHead	teacherHead;
    }userHeadParam;
}CMDGetBeTeacherInfoRespHead_t;

struct GetBeTeacherInfoNornalUserRespItem
{
	int32 teacherid;
	char  teacherAlias[NAMELEN];
	int32 cQuery;//剩余免费问股次数
	int32 cViewFlowers;//剩余免费观点献花次数
};

struct GetBeTeacherInfoTeacherRespItem
{
	int32 studentid;
	char  studentAlias[NAMELEN];
};

typedef struct tag_CMDGetBeTeacherInfoRespItem
{
    int32   nuserid; 
	uint64  starttime; //
	uint64  effecttime; //持续时间
	union
	{
		GetBeTeacherInfoNornalUserRespItem normalUserItem;
		GetBeTeacherInfoTeacherRespItem teacherItem;
	}userItemParam;
}CMDGetBeTeacherInfoRespItem_t;

///拜师请求&&返回
typedef struct tag_CMDBeTeacherReq
{
	uint32  userid; //请求该信息的用户ID
	uint32  teacherid; //所拜的讲师
	uint32  vcbid; //房间ID
	uint8   opMode; //0:30天 1:90天
}CMDBeTeacherReq_t;

typedef struct tag_CMDBeTeacherResp
{
	uint32  userid; //请求该信息的用户ID
	uint8   result; //结果
	uint64  nk;
}CMDBeTeacherResp_t;

//////////////////////////////

typedef struct tag_CMDUserPayReq
{
	uint32	srcid;				//消费者ID，一般是用户ID
	uint32	dstid;				//获得收益的ID，一般是讲师 ID
	uint32	vcbid;  			//房间ID
	uint8   isPackage;		//是否是礼包类型（商品组合销售）
	uint32  goodclassid;	//商品类型ID
	uint32  type;					//类型
	uint32	num;					//消费数量（主要针对观点鲜花，礼物设计。其他一律设1）
} CMDUserPayReq_t;


typedef struct tag_CMDUserPayRes
{
	uint32 userid; //用户ID
	uint64 nk;   	 //用户余额
	uint32 errid;	 //错误原因：0. 操作成功 1.商品不存在 2.余额不足 3.DB error
} CMDUserPayRes_t;

typedef struct tag_CMDGetUserAccountBalanceReq
{
	uint32 userid;         //用户ID
} CMDGetUserAccountBalanceReq_t;

typedef struct tag_CMDGetUserAccountBalanceRes
{
	uint32 userid;  //用户ID
	uint64 nk;   //用户余额
	uint32 errid;	//错误原因：0. 操作成功 1.用户不存在 2.DB error
} CMDGetUserAccountBalanceRes_t;

//个人秘籍大厅列表
typedef struct tag_CMDHallSecretsListResp
{
	int32  secretsid;              //秘籍ID
	char	 srcalias[NAMELEN];      //讲师昵称
	int16  coverlittlelen;         //封面小图名称长度
	int16  titlelen;               //秘籍标题长度
	int16  textlen;                //秘籍简介长度
	uint64 messagetime;            //发送时间(yyyymmdd)
	char  content[0];              //消息内容，格式：封面小图名称+秘籍标题+秘籍简介
}CMDHallSecretsListResp_t;

//获取特权列表
enum PrivilegePackage
{
	BeTeacherPackage = 0,
};

typedef struct tag_CMDGetPackagePrivilegeReq
{
	uint32 userid;         //用户ID
	uint32 packageNum;          //
} CMDGetPackagePrivilegeReq_t;

typedef struct tag_CMDGetPackagePrivilegeResp
{
	uint32 userid;         //用户ID
	uint32 index;          //下标
	char   privilege[256];       //特权描述
} CMDGetPackagePrivilegeResp_t;

//个人秘籍总体信息请求
typedef struct tag_CMDTextRoomSecretsTotalReq
{
	uint32 vcbid;                  //房间ID
	uint32 userid;                 //用户ID
	uint32 teacherid;              //讲师ID
}CMDTextRoomSecretsTotalReq_t;

//个人秘籍总体信息响应
typedef struct tag_CMDTextRoomSecretsTotalResp
{
	uint32 vcbid;                  //房间ID
	uint32 userid;                 //用户ID
	uint32 teacherid;              //讲师ID
	int32  secretsnum;             //秘籍总数
	int32  ownsnum;                //已购买秘籍总数
	int8   bStudent;               //是否徒弟（0-否；1-是）
}CMDTextRoomSecretsTotalResp_t;

//个人秘籍列表消息头
typedef struct tag_CMDTextRoomListHead
{
    char    uuid[16];              //唯一标识头
	uint32 vcbid;                  //房间ID
	uint32 userid;                 //用户ID
	uint32 teacherid;              //讲师ID
}CMDTextRoomListHead_t;

//个人秘籍列表
typedef struct tag_CMDTextRoomSecretsListResp
{
	int32  secretsid;              //秘籍ID
	int16  coverlittlelen;         //封面小图名称长度
	int16  titlelen;               //秘籍标题长度
	int16  textlen;                //秘籍简介长度
	uint64 messagetime;            //时间(yyyymmddhhmmss)
	int32  buynums;                //订阅人数
	int32  prices;                 //单次订阅所需玖玖币
	int8   buyflag;                //是否购买：1-已购买；0-未购买；
	int32  goodsid;                //商品ID
	char  content[0];              //消息内容，格式：封面小图名称+秘籍标题+秘籍简介
}CMDTextRoomSecretsListResp_t;

typedef struct tag_CMDTextRoomSecretsListOwn
{
	int  secretsid;                //秘籍ID
}CMDTextRoomSecretsListOwn_t;

//个人秘籍购买请求
typedef struct tag_CMDTextRoomBuySecretsReq
{
	uint32 vcbid;                  //房间ID
	uint32 userid;                 //用户ID
	uint32 teacherid;              //讲师ID
	int32  secretsid;              //秘籍ID
	int32  goodsid;                //商品ID
}CMDTextRoomBuySecretsReq_t;

//个人秘籍购买响应
typedef struct tag_CMDTextRoomBuySecretsResp
{
	uint32 vcbid;                  //房间ID
	uint32 userid;                 //用户ID
	int32  secretsid;              //秘籍ID
	int16  result;                 //回复是否成功：0. 操作成功 1.商品不存在 2.余额不足 3.DB error
	uint64  nk99;                  //用户账户玖玖币余额（成功时需要刷新客户端余额）
}CMDTextRoomBuySecretsResp_t;

//PHP后台录入个人秘籍后主动通知服务端
typedef struct tag_CMDTextRoomSecretsPHPReq
{
	uint32 vcbid;                  //房间ID
	uint32 teacherid;              //请求用户ID
	int64  messageid;              //消息ID
	int32  businessid;             //秘籍ID
	int8   viewtype;               //操作类型：1-新增；2-修改；3-删除；
	int16  coverlittlelen;         //封面小图名称长度
	int16  titlelen;               //秘籍标题长度
	int16  textlen;                //秘籍简介长度
	uint64 messagetime;            //时间(yyyymmddhhmmss)
	int8   commentstype;	       //客户端类型 0:PC端 1:安卓 2:IOS 3:WEB
	int32  prices;                 //单次订阅所需玖玖币
	int32  goodsid;                //商品ID
	char  content[0];              //消息内容，格式：封面小图名称+秘籍标题+秘籍简介
}CMDTextRoomSecretsPHPReq_t;

//PHP后台录入个人秘籍后主动通知服务端广播给客户端
typedef struct tag_CMDTextRoomSecretsPHPResp
{
	uint32 vcbid;                  //房间ID
	uint32 teacherid;              //请求用户ID
	int64  messageid;              //消息ID
	int32  businessid;             //秘籍ID
	int8   viewtype;               //操作类型：1-新增；2-修改；3-删除；
	int16  coverlittlelen;         //封面小图名称长度
	int16  titlelen;               //秘籍标题长度
	int16  textlen;                //秘籍简介长度
	uint64 messagetime;            //时间(yyyymmddhhmmss)
	int8   commentstype;	       //客户端类型 0:PC端 1:安卓 2:IOS 3:WEB
	int32  prices;                 //单次订阅所需玖玖币
	int32  goodsid;                //商品ID
	char  content[0];              //消息内容，格式：封面小图名称+秘籍标题+秘籍简介
}CMDTextRoomSecretsPHPResp_t;

//系统消息大厅邮箱列表
typedef struct tag_CMDHallSystemInfoListResp
{
	int32  systeminfosid;          //系统消息ID
	int16  titlelen;               //标题长度
	int16  linklen;         			 //链接长度
	int16  textlen;                //内容长度
	uint64 messagetime;            //发送时间(yyyymmdd)
	char  content[0];              //消息内容，格式：标题+链接+内容
}CMDHallSystemInfoListResp_t;

//付费表情列表
typedef struct tag_CMDTextRoomEmoticonListResp
{
	uint32 emoticonID;             //表情ID
	char	 emoticonName[NAMELEN];  //表情名称
	int32  prices;                 //价格
	int8   buyflag;                //是否购买：1-已购买；0-未购买；
}CMDTextRoomEmoticonListResp_t;

//获取用户（包括讲师和普通用户）个人资料
typedef struct tag_CMDGetUserInfoReq
{
	uint32 srcuserid;    //用户ID
	uint32 dstuserid;		 //被查看的用户ID	
} CMDGetUserInfoReq_t;

//讲师个人资料
typedef struct tag_CMDTeacherInfoResp
{
	uint32  teacherid;          //讲师ID
    char    sIcon[64];       //头像信息
	uint32  vcbid;         			//讲师所属文字直播房间ID
	int16   introducelen;       //个人介绍长度
	int16   lablelen;           //个人标签长度
	int16   levellen;						//讲师级别长度（初级分析师...）
	int16   type;								//讲师类别 1-文字直播；2-视频直播；3-文字+视频直播；
	uint64  czans;							//讲师获赞数
	uint64  moods;							//讲师人气数
	uint64  fans;								//讲师粉丝数
	int8  	fansflag;           //是否已经关注讲师（0-未关注；1-已关注）
	int8  	subflag;           //是否已经订阅讲师课程（0-未订阅；1-已订阅）
	char    content[0];         //消息内容，格式：个人介绍+个人标签+讲师级别
}CMDTeacherInfoResp_t;

//普通用户个人资料
typedef struct tag_CMDRoomUserInfoResp
{
	uint32  userid;          //用户ID
	uint32  headid;          //用户头像ID
	uint32  birthday;			   //生日
	int16   introducelen;    //个人介绍长度
	int16   provincelen;     //省份长度
	int16   citylen;				 //城市长度
	char    content[0];      //消息内容，格式：个人介绍+省份+城市
}CMDRoomUserInfoResp_t;

typedef struct tag_CMDUserInfoErr
{
	uint32 userid;  //用户ID
	uint32 errid;   //0 成功（成功不发这个包，1.用户不存在 2.DB error 3.teacherflag 错误 
}CMDUserInfoErr_t;

typedef struct tag_CMDRoomTeacherOnMicReq
{
	uint32  teacherid;      //讲师ID
	uint32  vcbid;          //房间ID
	char	 	alias[NAMELEN]; //讲师昵称
}CMDRoomTeacherOnMicReq_t;

typedef struct tag_CMDRoomTeacherOnMicResp
{
	uint32  teacherid;      //讲师ID
	uint32  vcbid;          //房间ID
	char	alias[NAMELEN]; //讲师昵称
	char	roomname[NAMELEN]; //房间名称
}CMDRoomTeacherOnMicResp_t;

typedef struct tag_CMDRoomTeacherSubscripReq
{
	uint32  teacherid;      //讲师ID
	uint32  userid;          //房间ID
}CMDRoomTeacherSubscripReq_t;

typedef struct tag_CMDRoomTeacherSubscripNotyResp
{
	uint32 userid;  //用户ID
	uint32 errid;   //0.成功 1.已经订阅  2.DB error
}CMDRoomTeacherSubscripNotyResp_t;


typedef struct tag_CMDGetUserMoreInfReq_t
{
	uint32 userid;
}CMDGetUserMoreInfReq_t;

typedef struct tag_CMDGetUserMoreInfResp_t
{
    char   tel[32];                   //手机号码
    char   birth[16];                 //生日
    char   email[40];                 //email
    int16  autographlen;              //个性签名长度
    char   content[0];                //个性签名内容
}CMDGetUserMoreInfResp_t;

//订阅
typedef struct tag_CMDTeacherSubscriptionReq
{
	uint32 nmask;           //标示,用于客户端验证是不是自己发出的resp,
	uint32 userid;
    uint32 teacherid;
	uint8  bSub; // 0 :取消订阅  1: 订阅
}CMDTeacherSubscriptionReq_t;

typedef struct tag_CMDTeacherSubscriptionResp
{
	uint32 nmask;           //标示,用于客户端验证是不是自己发出的resp,
	uint32 userid;
	uint8  errcode;//返回错误码
}CMDTeacherSubscriptionResp_t;

//查询订阅状态
typedef struct tag_CMDTeacherSubscriptionStateQueryReq
{
	uint32 nmask;           //标示,用于客户端验证是不是自己发出的resp,
	uint32 userid;
    uint32 teacherid;
}CMDTeacherSubscriptionStateQueryReq_t;

typedef struct tag_CMDTeacherSubscriptionStateQueryResp
{
	uint32 nmask;           //标示,用于客户端验证是不是自己发出的resp,
	uint32 userid;
	uint8  state;//订阅状态 0:未订阅 1:已订阅
}CMDTeacherSubscriptionStateQueryResp_t;

//该结构体用于roomsvr与textlivesvr通信:讲师上麦通知
typedef struct tag_CMDVideoRoomOnMicNotyAmongServer
{

	uint32 userid;
	uint32 roomid;
	uint8  state;
}CMDVideoRoomOnMicNotyAmongServer_t;

//通知客户端麦状态改变
typedef struct tag_CMDVideoRoomOnMicClientResp
{

	uint32 userid;
	char   useralias[NAMELEN];
	uint32 roomid;	
	char   roomName[NAMELEN];
	uint8  state;
}CMDVideoRoomOnMicClientResp_t;

typedef struct tag_CMDErrCodeResp_t
{
     uint16 errmaincmd;
	 uint16 errsubcmd;
	 uint16 errcode;
}CMDErrCodeResp_t;

// for redis

typedef struct tag_CMDRedisData_t
{
	uint16 length;
	char content[0];
}CMDRedisData_t;

typedef struct tag_RedisKeyInfo_t
{
	char key[64];
	char type[16];
	uint16 length;
	char content[0];
}RedisKeyInfo_t;

//获取房间的转播机器人ID请求
typedef struct tag_CMDOnMicRobertReq
{
  uint32 vcbid;
}CMDOnMicRobertReq_t;

//获取房间的转播机器人ID应答
typedef struct tag_CMDOnMicRobertResp
{
  uint32 vcbid;
  uint32 robertid;
}CMDOnMicRobertResp_t;

//邮箱新消息提示（用户列表）
typedef struct tag_CMDEmailNewMsgNotyList
{
  byte bEmailType;     //1 私人定制 2系统消息 3评论回复 4提问回复
  uint32 messageid;        //消息ID
  uint32  usernum;         //userid num
  char  content[0];        //userid uint32 ...
} CMDEmailNewMsgNotyList_t;

//邮箱新消息提示
typedef struct tag_CMDEmailNewMsgNoty
{
  byte bEmailType;        //1 私人定制 2系统消息 3评论回复 4提问回复
  uint32 messageid;       //消息ID
} CMDEmailNewMsgNoty_t;

//私人订制通知请求
typedef struct tag_CMDPrivateVipNotyReq
{
  uint32 teacherid;     //讲师ID
  uint32 viptype;       //VIP类型
  uint32 messageid;     //消息ID
}CMDPrivateVipNotyReq_t;

//加入房间预处理请求
typedef struct tag_CMDPreJoinRoomReq
{
    uint32 userid;              //用户id,可能是靓号id,可能是游客号码
    uint32 vcbid;               //房间id
}CMDPreJoinRoomReq_t;

//加入房间预处理响应
typedef struct tag_CMDPreJoinRoomResp
{
  uint32 userid;          //用户id,可能是靓号id,可能是游客号码
  uint32 vcbid;           //房间id
  uint8 result;           //右起1位 1房间存在 0房间不存在，2位 1在黑名单 0不在黑名单，3位 1房间人数是否已满 0房间人数未满，4位 1房间有密码 0房间无密码
}CMDPreJoinRoomResp_t;

typedef struct tag_CMDAfterJoinRoomReq
{
	uint32 userid;
	uint32 vcbid;
	uint64 messageid;
	uint8  actionid; 		//0 人为加入房间; 1 断网自动重连
}CMDAfterJoinRoomReq_t;

//私人订制购买请求
typedef struct tag_CMDBuyPrivateVipReq
{
	uint32 userid;          //用户ID
    uint32 teacherid;     	//讲师ID
    uint32 viptype;     	//vip等级ID
}CMDBuyPrivateVipReq_t;

//私人订制购买响应
typedef struct tag_CMDBuyPrivateVipResp
{
	uint32 userid;          //用户ID
	uint32 teacherid;     	//讲师ID
	uint32 viptype;     	//vip等级ID
	uint64 nk;        	    //账户余额
	char teamname[NAMELEN]; //战队名称
	uint32 buytime;			//购买时间
	uint32 expirtiontime;	//过期时间
	byte btel;				//是否绑定手机(1是绑定手机,0是没绑定手机)
}CMDBuyPrivateVipResp_t;

//专家观点消息推送（房间内部广播）
typedef struct tag_CMDExpertNewViewNoty
{
    uint32 nmessageid;      //观点ID
    uint32 nvcbid;          //房间ID
    uint32 teamid;          //战队ID
	char  sName[48];        //专家名称
    char  sIcon[64];       //头像信息
    char  sPublicTime[32];  //发表时间
	uint32 nCommentCnt;     //评论次数
    uint32 nLikeCnt;        //点赞次数
    uint32 nFlowerCnt;      //鲜花数量
	uint16 contlen;         //内容长度
    char   content[0];      //内容
} CMDExpertNewViewNoty_t;

typedef struct tag_CMDTeamTopNReq
{
  uint32 userid;         //用户id,
  uint32 vcbid;          //房间id
}CMDTeamTopNReq_t;

typedef struct tag_CMDTeamTopNResp
{
  uint32 vcbid;          //房间id
  char teamname[NAMELEN];//战队名称
  uint64 giftmoney;      //礼物金币
}CMDTeamTopNResp_t;

//观点赠送礼物请求
typedef struct tag_CMDViewpointTradeGiftReq
{
  uint32 userid;     // 送礼人
  uint32 roomid;     // 送给哪个roomId
  uint32 teamid;     // 送给哪个teamid
  uint32 viewid;     // 观点id
  uint32 giftid;     // 礼物id
  uint32 giftnum;    // 赠送数目
}CMDViewpointTradeGiftReq_t;

//观点赠送礼物通知
typedef struct tag_CMDViewpointTradeGiftNoty
{
  uint32 userid;     // 送礼人
  char useralias[NAMELEN];     // 送礼人昵称
  uint32 roomid;     // 送给哪个roomId
  uint32 teamid;     // 送给哪个teamid
  char teamalias[NAMELEN];     // team昵称
  uint32 viewid;     // 观点id
  uint32 giftid;     // 礼物id
  uint32 giftnum;    // 赠送数目
  uint64 nk;         //账户余额
}CMDViewpointTradeGiftNoty_t;

//提问请求
typedef struct tag_CMDAskQuestionReq
{
  uint32 roomid;
  uint32 userid;
  uint32 teamid;
  char stock[32];
  uint32 questionlen;
  char question[0];
}CMDAskQuestionReq_t;

//提问请求
typedef struct tag_CMDAskQuestionResp
{
  uint64 nk;        //账户余额
  uint32 questionid;  
  uint32 userid;
  uint32 teamid;
  char stock[32];
  uint32 questionlen;
  char question[0];
}CMDAskQuestionResp_t;

//讲师上麦通知媒体服务器
typedef struct tag_CMDSubInfo
{
    uint32 robotid;     //转播表内的robotid
    uint32 subroomid;   //子房间id
}CMDSubInfo_t;

typedef struct tag_CMDNotifyMediaRoomOnMic
{
    uint32 type;        //0 下麦 1 上麦
    uint32 userid;      //讲师Id
    uint32 roomid;      //房间id
    uint32 num;  //子房间数量
    char   content[0];      //CMDSubInfo ...
}CMDOnMicNotifyMedia_t;

typedef struct tag_CMDRoomMgrNoty
{
  uint32 roomid;     //房间ID
  byte  action;	   //操作：1.新增房间 2.修改房间 3.关闭房间
} CMDRoomMgrNoty_t;

typedef struct tag_CMDRoomOnlineTS
{
	uint32 	vcbid;
	uint32 	userid;
	int 	guest; 			// 0:游客 1:注册用户
	byte  	devtype;
	time_t 	joinroom_time;
	uint32  timespan;		// 在线时长
    char    ipaddr[32];
}CMDRoomOnlineTS_t;

//登陆日记信息
typedef struct tag_CMDUserLoginLog 
{
	int		userid;
	int		devtype;
	char	szip[32];
	char	szmac[32];
    char	szserial[64];
	char	devicemodel[32];	//device model
	char	deviceos[16];		//device os
	uint32	time;				//login/offline time
	uint32	loginid;			//loginid
}CMDUserLoginLog_t;

typedef struct tag_CMDTeacherGiftListResp_v2
{
  uint32 seqid;                   //把排名也发过去
  uint32 vcbid;          //房间id
  uint32 teacherid;        //讲师ID
  char useralias[NAMELEN];      //用户昵称
  uint64 t_num;          //忠实度值
  char usericon[URLLEN];        //用户头像（新增）
}CMDTeacherGiftListResp_v2_t;

typedef struct tag_CMDTeacherOnMicCallFans
{
	char    teacher_name[NAMELEN]; 		//讲师名称
	uint32  roomid;          			//房间ID
	char    room_name[NAMELEN];			//房间名称 
	uint32  team_id;     				//战队ID
	char    team_icon [ROOMPICTURELEN];	//战队头像
	uint32  online_num;      			//在线人数
    uint32  usernum;        			//userid num
    char  	content[0];       			//userid uint32 ...
}CMDTeacherOnMicCallFans_t;

//封杀用户请求数据体
typedef struct tag_CMDBlockUserReq
{
	uint32 runnerid;		//操作者id, 目前管理后台固定填10000
	uint32 userid;			//封杀的用户id
	int32  reasonid;		//reason id, ERR_KICKOUT_AD = 600 打广告
}CMDBlockUserReq_t;

typedef struct tag_CMDSoftbotPushJoinRoomEXT
{
	uint32 roomid;
	uint32 num;   			//增加人数数量
	uint8  viplevelnum; 	//vip等级个数
	char  	content[0];	//CMDVipLevelRate_t 列表
}CMDSoftbotJoinRoomEXT_t;

typedef struct tag_CMDVipLevelRate
{
	uint8  viplevel; 		//vip等级
	uint8  vipratio;		//vip占总人数的比例
}CMDVipLevelRate_t;

typedef struct tag_CMDSoftbotPushExitRoomEXT
{
	uint32 roomid;
	uint32 num;   			//减少人数数量
	uint8  viplevelnum; 	//vip等级个数
	char  	content[0];	//CMDVipLevelRate_t 列表
}CMDSoftbotExitRoomEXT_t;

typedef struct tag_CMDSoftbotPushJoinRoom
{
	uint32 roomid;
	uint32 num;   		//数量
	uint8 vipratio;		//vip占比
}CMDSoftbotJoinRoom_t;

typedef struct tag_CMDSoftbotPushExitRoom
{
	uint32 roomid;
	uint32 num;   		//数量
	uint8 vipratio;		//vip占比
}CMDSoftbotExitRoom_t;

typedef struct tag_CMDGetAskOpportunity
{
	uint32 userid;
}CMDGetAskOpportunity_t;

typedef struct tag_CMDVipUserNoty
{
	uint32 userid;
	uint32 teacherid;
	byte   viplevel;
}CMDVipUserNoty_t;

typedef struct tag_CMDHistoricalRecordHeader
{
    uint32 roomid;       //房间id
    int num;		      //包个数
}CMDHistoricalRecordHeader_t;

typedef struct tag_CMDHistoricalRecord
{
    uint32 portfolioid;    //组合id
    char stock[32];        //股票名称,
    uint16 yield;          //收益率（为收益率的10000倍，如99.99%收益率 yield为9999）
    uint16 timelen;        //收益率时长（单位：天）
}CMDHistoricalRecord_t;

typedef struct tag_CMDHistoricalRecordUpdate
{
	uint32  roomid;       //需要更新的房间id
}COMHistoricalRecordUpdate;

//推送问答列表包结构体
typedef struct tag_CMDQuestionAnswerPush
{
	uint32 	num; 		//问答包个数
	char 	content[0];	//包内容
} CMDQuestionAnswerPush_t;

//推送包内容结构体单元
typedef struct tag_CMDQuestionAnswerItem
{
	uint32 	userid;
	char  	useralias[NAMELEN];   	//呢称
	uint32 	teacherid;
	char  	teacheralias[NAMELEN];	//呢称
	uint64 	ndatetime;				//回答时间
	char	stockid[NAMELEN];		//股票编码
	uint32 	nquestlength;			//问题长度
	uint32 	nanswerlength;			//回答长度
	char 	content[0];				//问题+回答
} CMDQuestionAnswerItem_t;

typedef struct tag_CMDAfterLoginReq
{
    uint32 userid;		      //用户ID
}CMDAfterLoginReq_t;

typedef struct tag_CMDRoomMicStateHeader
{
    int num;		      //包个数
}CMDRoomMicStateHeader_t;

typedef struct tag_CMDRoomMicStateRecord
{
    uint32 roomid;      //房间id
    uint32 teacherid;	//在麦讲师id 无人在麦则为0
}CMDRoomMicStateRecord_t;

typedef struct tag_CMDHandOutUpDate
{
    uint32  roomid;         //需要更新的房间id
    uint32  syllabusid;     //课程id
    uint32  handoutid;      //讲义id
    char    createtime[32]; // 发表时间 
    uint32  contentlength;  //讲义内容长度
    uint32  titlelength;    // 讲义标题长度
    uint32  imgurllength;   //讲义图片长度
    uint32  articlelength;  //讲义链接长度
    char    content[0];     //讲义内容+讲义标题+讲义图片+讲义链接
}CMDHandOutUpDate_t;

typedef struct tag_CMDTransKeyReq
{
	uint32  pubkeyLen;           //公钥长度
	char    pubkeyContent[0];    //公钥内容
}CMDTransKeyReq_t;

typedef struct tag_CMDTransKeyResp
{
	uint32  transKeyLen;         //加密后传输密码的长度
	char    transKeyContent[0];  //加密后传输密码的内容
}CMDTransKeyResp_t;

typedef struct tag_CMDVipPresenceNoty
{
	uint32 roomid;	 			//房间id
	uint32 userid;				//用户ID
	char   useralias[NAMELEN];   //呢称
	uint8  viplevel;			//VIP等级
	uint32 viptotal;			//VIP总人数
} CMDVipPresenceNoty_t;

typedef struct tag_GetRoomAssistantReq
{
    uint32 userid;				//用户id
    uint32 roomid;				//房间id
}CMDGetRoomAssistantReq_t;

typedef struct tag_GetRoomAssistantResp
{
    uint32 userid;				//用户id
    uint32 roomid;				//房间id
    uint32 assid;               //助理id,0时没有助手存在
    byte   ismobile;				//上次是否选中查看手机号,1选中,0没选中
    char   wechatid[64];			//微信号
    char   QRcode[100];          //二维码
    char   msg[256];             //消息内容
}CMDGetRoomAssistantResp_t;

typedef struct tag_PrivateMsgStart
{
    uint32 userid;				//用户id
    uint32 roomid;				//房间id
    byte   ismobile;			//是否选中查看手机号,1选中,0没选中
}CMDPrivateMsgStart_t;

typedef struct tag_RoomChatMsgEtr
{
    char   end;			        //结束符\0  兼容旧版客户端按照结束符处理聊天内容
	uint32 timestamp;			//消息时间
    uint32 messageid;           //消息id    自增建
    char   sIcon[100];          //头像信息
}CMDRoomChatMsgEtr_t;

typedef struct tag_CMDPushStockNoty
{
	uint32	roomid;	 			//房间id
	uint8	type;				//文案类型 0-A类，1-B类，2-C类
	uint32	teamid;				//战队id
	uint32	assetid;			//组合id
	uint16	contlen;         	//内容长度
	char   	content[0];       	//内容
} CMDPushStockNoty_t;

typedef struct tag_CMDPushBuyVipNoty
{
	uint32  	roomid;	 				//房间id
	uint32  	teamid;					//战队id
	char  	 	teamalias[NAMELEN];		//战队名称
	uint32 	 	userid;					//用户ID
	char  	 	useralias[NAMELEN];   	//呢称
	uint8  	 	viplevel;				//VIP等级 1~6
	uint32 	 	viptotal;				//VIP总人数
} CMDPushBuyVipNoty_t;

typedef struct tag_CMDPtCourseNoty
{
	uint32	courseid;					//课程id
	uint32	roomid;						//课程房间id
	uint32	teamid;						//战队id
	char	topic[64];					//课程主题
	uint32	begintime;					//课程开始时间
	uint32	endtime;					//课程结束时间
	byte	isbuy;						//该用户是否购买
	byte	isfull;						//该课程是否满额
	byte	state;						//该课程状态（未发布:0,发布:1,未开课:2,开课:3,结束:4）
	char	teacheralias[32];			//主讲老师昵称
	uint16	studentlimit;				//课程学员上限
	uint16	studentnum;           		//课程学员当前值
	uint32	courseprice;				//课程定价
	char	wechatid[255];				//房间微信号
	char	QRcode[100];				//房间二维码
	uint16	textlen;     				//课程简介长度
	char	content[0];   				//课程简介
} CMDPtCourseNoty_t;

typedef struct tag_CMDNewPtCourseNoty
{
	uint32	courseid;					//新课程ID
} CMDNewPtCourseNoty_t;

typedef struct tag_CMDNewPtCourseReq
{
	uint32  userid;						//用户ID
	uint32	courseid;					//新课程ID
} CMDNewPtCourseReq_t;

typedef struct tag_CMDBuyPtCourseReq
{
	uint32 userid;						//用户ID
	uint32 courseid;					//课程ID
} CMDBuyPtCourseReq_t;

typedef struct tag_CMDBuyPtCourseResp
{
	uint32 errid;						//错误码
	uint8  startlimitbuy;				//开课超过的分钟数，不允许购买，用于前台展示
	uint32 userid;						//用户ID
	uint32 courseid;					//课程ID
	uint64 nk;        					//账户余额
} CMDBuyPtCourseResp_t;

typedef struct tag_CMDPtCourseStateNoty
{
	uint32	courseid;					//课程ID
	byte	coursestate;				//该课程状态（未发布:0,发布:1,未开课:2,开课:3,结束:4）
	byte	isfull;						//是否满额
	uint16	studentnum;           		//课程学员当前值
}CMDPtCourseStateNoty_t;

typedef struct tag_CMDBuyPtCourseNoty
{
	uint32	userid;						//用户ID
	char	useralias[32];				//用户昵称
	char	topic[64];					//课程主题
	uint32	begintime;					//课程开始时间
	uint32	endtime;					//课程结束时间
} CMDBuyPtCourseNoty_t;

typedef struct tag_CMDVIPCourseStateNoty
{
	uint32	courseid;					//课程ID
	byte	coursestate;				//该课程状态（开课:3,结束:4）
	uint32	begintime;					//课程开始时间
	uint32	endtime;					//课程结束时间
	char	teachername[LEN32];			//讲师名字
	char	title[LEN64];				//标题
	byte	isvip;						//是否vip课程
}CMDVIPCourseStateNoty_t;

//房间非直播时间通知
typedef struct tag_NonLiveTimeNotify
{
	uint32  roomid;				//房间id
}CMDNonLiveTimeNotify_t;

typedef struct tag_PotentialVIPNoty
{
  uint32 userid;				//用户id
  byte   potential_viplevel;	//潜在VIP等级
}CMDPotentialVIPNoty_t;

//上传设备 pushtoken 
typedef struct tag_CMDSendPushTokenReq
{
    uint32 userid;                 //用户ID
    uint8 action;				   //0解绑， 1绑定
	uint8 devtype;                 //终端类型（1 安卓2 IOS）
    uint8 pushtype;                //推送类型（1 小米2 华为 3 IOS）
    char pushtoken[64];            //推送token
} CMDSendPushTokenReq_t;

typedef struct tag_CMDTeacherOnMicCallFansPush
{
	char    teacher_name[NAMELEN]; 		//讲师名称
	uint32  roomid;          			//房间ID
	char    room_name[NAMELEN];			//房间名称 
	uint32  team_id;     				//战队ID
	char    team_icon [ROOMPICTURELEN];	//战队头像
	uint32  online_num;      			//在线人数
    char  	content[0];                 //CMDPushDataToken_t
}CMDTeacherOnMicCallFansPush_t;

typedef struct tag_CMDPushDataTokenHeader
{
    uint16  num;            //Token num
    char  	content[0];     //char CMDPushDataToken_t[num]
}CMDPushDataTokenHeader_t;

typedef struct tag_CMDPushDataToken
{
    uint16  pushtype;       //推送类型（1 小米2 华为 3 IOS）
    char    pushtoken[65];  //推送Token
}CMDPushDataToken_t;


//vip专区推送提醒（from PHP）
typedef struct tag_CMDVipZoneNotyFromPHP
{
	uint32 teamid;			//战队编号
	uint8 viplevel;			//vip等级
	uint32 servicetype;		//内容类型
	uint32 serviceid;		//内容id
}CMDVipZoneNotyFromPHP_t;

//vip专区推送提醒（to client）
typedef struct tag_CMDVipZoneNoty
{
	uint32 userid;		//用户编号
	uint32 teamid;		//战队编号
	uint32 roomid;		//房间id
	uint32 servicetype;	//内容类型
	uint32 serviceid;	//内容id
}CMDVipZoneNoty_t;

typedef struct tag_CMDFocusNotice
{
	uint32	subjectid;			//订阅号ID
	uint32	releasetime;		//发布时间 
	uint32	length;					//内容长度
	char    content[0];   	//内容
}CMDFocusNotice_t;

typedef struct tag_CMDPushNotice
{
	uint32	type;						//公告类型
	uint32	releasetime;		//发布时间 
	uint32	length;					//内容长度
	char    content[0];   	//内容
}CMDNotice_t;

//php消息推送通知
typedef struct tag_MsgPushNotify
{
	uint32	contentLen;		//Json内容长度
	char 	content[0];		//Json格式内容
}MsgPushNotify_t;

//实时盯盘推送数据项
typedef struct tag_MarketEyeItem
{
	uint32 	time;
	char 	content[256];
}MarketEyeItem_t;


////系统消息
//typedef struct tag_SystemMsgItem
//{
//	uint32 	time;	//创建时间
//	char	title[64];
//	char 	content[256];
//	char    activityurl[128];	//活动链接
//	uint32  userid;				//推送到个人或是广播 0：广播，否则发到某个用户
//}SystemMsgItem_t;
//
////客服消息
//typedef struct tag_CustomerServiceMsgItem
//{
//	uint32 	time;	//创建时间
//	char 	content[256];
//	uint32	userid;
//}CustomerServiceMsgItem_t;
//
////个股公告
//typedef struct tag_StockNoticMsgItem
//{
//	uint32 	time;	//创建时间
//	char 	content[256];
//	uint32	userid;
//}StockNoticMsgItem_t;
//
////我的评论
//typedef struct tag_MyCommentMsgItem
//{
//	uint32 	time;			//创建时间
//	uint32 	id;				//评论id
//	uint32	pid;			//评论父id
//	char 	content[256];	//评论内容
//	uint32	userid;			//评论人
//}MyCommentMsgItem_t;

//个股预警设置（设置&查询用）
typedef struct tag_StkWarningSetting
{
	uint32 userid;				//用户id
	char szStkCode[STKCODELEN];	//股票代码
	uint16 switchs;			//开关			开关值		单位		预警类型
	uint32 uTurnoverRate;	//换手率		0x0001		万分之一		0
	uint32 uHighPrice;		//高价预警		0x0002		0.0001元		1
	uint32 uLowPrice;		//低价预警		0x0004		0.0001元		2
	uint32 uDayRise;		//日涨幅		0x0008		万分之一		3
	uint32 uDayFall;		//日跌幅		0x0010		万分之一		4
	uint32 uLargeBuy;		//大笔买入		0x0020		元				5
	uint32 uLargeSell;		//大笔卖出		0x0040		元				6
	uint32 uFastRise;		//急速拉升		0x0080		万分之一		7
	uint32 uFastFall;		//猛烈打压		0x0100		万分之一		8
}StkWarningSetting_t;

//个股预警设置响应
typedef struct tag_SetStkWarningSettingResp
{
	uint32 userid;				//用户id
	char szStkCode[STKCODELEN];	//股票代码
	uint16 retCode;				//错误码：0 成功, 2002 个股预警已达上限
	uint16 addedValue;			//个股预警个数上限使用
}SetStkWarningSettingResp_t;

typedef struct tag_StkWarningSettingEX :StkWarningSetting_t
{
	uint16 oldSwitchs;	//旧开关
}StkWarningSettingEX_t;

//个股预警查询请求（查到返回StkWarningSetting_t）
typedef struct tag_QryStkWarningSetting
{
	uint32 userid;			//用户id
	char szStkCode[STKCODELEN];		//股票代码
}QryStkWarningSetting_t;

//个股预警查询失败响应
typedef struct tag_QryStkWarningSettingErr
{
	uint32 userid;				//用户id
	char szStkCode[STKCODELEN];	//股票代码
}QryStkWarningSettingErr_t;

//个股预警
typedef struct tag_StkWarningNotice
{
	uint32	uTimestamp;		//预警时间戳
	uint32	userid;			//用户id
	uint8	type;			//预警类型
	char	szStkCode[STKCODELEN];	//股票代码
	char	szStkName[STKNAMELEN];	//股票名字
	uint32	uThreshold;		//预警阈值，单位同tag_StkWarningSetting结构体
//	uint32	uRealValue;		//实时 价格 or 日涨跌幅 or 换手率 or 单笔买入卖出金额 or 走势异动幅度
}StkWarningNotice_t;

//个股预警状态列表查询请求
typedef struct tag_QryStkWarningSettingList
{
	uint32 userid;			//用户id
}QryStkWarningSettingList_t;

//单个股票预警状态结构体
typedef struct tag_StkWarningStatus
{
	char	szStkCode[STKCODELEN];	//股票代码
	char	szStkName[STKNAMELEN];	//股票名称
	uint8	status;					//状态: 0 关闭, 1 开启
}StkWarningStatus_t;

//个股预警状态列表查询响应
typedef struct tag_StkWarningSettingList
{
	uint32	userid;			//用户id
	uint8	globalSwitch;	//0 关闭, 1 开启
	uint8	num;			//股票数量
	uint8	lenUnit;		//结构体单位长度
	StkWarningStatus_t list[0];
}StkWarningSettingList_t;

//个股预警状态修改请求
typedef struct tag_ModStkWarningStatus
{
	uint32 userid;		//用户id
	uint8 type;			//操作类型 0 关闭，1 开启，2 删除
	char szStkCode[STKCODELEN];	//股票代码，若是所有个股则所有字符都置0
}ModStkWarningStatus_t;

//个股预警状态修改响应
typedef struct tag_ModStkWarningStatusResp
{
	uint32 userid;		//用户id
	uint8 type;			//操作类型 0 关闭，1 开启，2 删除
	char szStkCode[STKCODELEN];	//股票代码，若是所有个股则所有字符都置0
	uint16 retcode;		//操作结果
}ModStkWarningStatusResp_t;

//市场雷达设置请求
typedef struct tag_StockRadarSetReq
{
	uint32 userid;			//用户id
	uint8  radartype;		//类型:1.A股;2.自选股;3.行业;4.概念;5.地区
	uint16 status;			//开关			开关值		单位
	uint32 uLargeBuy;		//大笔买入		0x0020		元
	uint32 uLargeSell;		//大笔卖出		0x0040		元
	uint32 uFastRise;		//急速拉升		0x0080		万分之一
	uint32 uFastFall;		//猛烈打压		0x0100		万分之一
	char szBoard[32];		//板块代码.若类型是板块,这个是板块代号;若是其余类型则该字段为""
}StockRadarSetReq_t;

//市场雷达设置响应
typedef struct tag_StockRadarSetResp
{
	uint32 userid;			//用户id
	uint8  radartype;		//类型:1.A股;2.自选股;3.行业;4.概念;5.地区
	uint16 status;			//开关值,和请求一样
	uint16 retcode;			//0:成功,2005:市场雷达设置失败
}StockRadarSetResp_t;

//市场雷达查询请求
typedef struct tag_StockRadarQryReq
{
	uint32 userid;			//用户id
}StockRadarQryReq_t;

//市场雷达查询响应
typedef struct tag_StockRadarQryResp
{
	uint32 userid;			//用户id
	uint8  radartype;		//类型:1.A股;2.自选股;3.行业;4.概念;5.地区
	uint16 status;			//开关			开关值		单位
	uint32 uLargeBuy;		//大笔买入		0x0020		元
	uint32 uLargeSell;		//大笔卖出		0x0040		元
	uint32 uFastRise;		//急速拉升		0x0080		万分之一
	uint32 uFastFall;		//猛烈打压		0x0100		万分之一
	char szBoard[32];		//板块代码.若类型是板块,这个是板块代号
	char szBoardName[32];	//板块名称.若类型是板块,这个是板块名称
}StockRadarQryResp_t;

//市场雷达查询失败响应
typedef struct tag_StockRadarQryErr
{
	uint32 userid;			//用户id
	uint16 retcode;			//0:成功,2007:市场雷达查询失败,2008:市场雷达查询为空
}StockRadarQryErr_t;

//市场雷达查询列表请求
typedef struct tag_StockRadarQryLstReq
{
	uint32 userid;			//用户id
}StockRadarQryLstReq_t;

//市场雷达状态结构体
typedef struct tag_StockRadarQryStatus
{
	uint8 radartype;		//类型:1.A股;2.自选股;3.行业;4.概念;5.地区
	uint8 status;			//状态:0 关闭, 1 开启
}StockRadarQryStatus_t;

//市场雷达查询列表响应
typedef struct tag_StockRadarQryLst
{
	uint32	userid;			//用户id
	uint8	globalSwitch;	//总开关状态：0 关闭, 1 开启
	uint8	num;			//列表数量
	uint8	lenUnit;		//结构体单位长度
	StockRadarQryStatus_t list[0];
}StockRadarQryLst_t;

//市场雷达开关修改请求
typedef struct tag_ModStkRadarStatusReq
{
	uint32 userid;			//用户id
	uint8  radartype;		//类型:0.所有;1.A股;2.自选股;3.行业;4.概念;5.地区
	uint8  actiontype;		//操作类型 0:关闭,1:开启,2:删除
}ModStkRadarStatusReq_t;

//市场雷达开关修改响应
typedef struct tag_ModStkRadarStatusResp
{
	uint32 userid;			//用户id
	uint8  radartype;		//类型:0.所有;1.A股;2.自选股;3.行业;4.概念;5.地区
	uint8  actiontype;		//操作类型 0:关闭,1:开启,2:删除
	uint16 retcode;			//0:成功,2006:市场雷达状态设置失败
}ModStkRadarStatusResp_t;

//市场雷达预警
typedef struct tag_StkRadarNotice
{
	uint32	uTimestamp;					//预警时间戳
	uint32	userid;						//用户id
	uint8	warn_type;					//预警类型
	uint8	radar_type;					//雷达类型:1.A股;2.自选股;3.板块
	char	szBoardCode[STKCODELEN];	//板块代码.若类型是板块,这个是板块代号
	char	szBoardName[STKNAMELEN];	//板块名称.若类型是板块,这个是板块名称
	uint32	uThreshold;					//预警阈值,单位同StockRadarSetReq_t结构体
}StkRadarNotice_t;

//自选股配置请求
typedef struct tag_SetOptionStockReq
{
	uint32 userid;				//用户id
	uint8  actiontype;			//操作类型 1:添加,2:删除
	char szStkCode[STKCODELEN];	//股票代码，内容格式：SH000001
}SetOptionStockReq_t;

//自选股配置响应
typedef struct tag_SetOptionStockResp
{
	uint32 userid;				//用户id
	uint8  actiontype;			//操作类型 1:添加,2:删除
	char szStkCode[STKCODELEN];	//股票代码，内容格式：SH000001
	uint16 retcode;				//0:成功,2003:自选股已经存在,2004:自选股操作失败
}SetOptionStockResp_t;

//自选股查询请求
typedef struct tag_QryIsOptionStockReq
{
	uint32 userid;				//用户id
	char szStkCode[STKCODELEN];	//股票代码，内容格式：SH000001
}QryIsOptionStockReq_t;

//自选股查询响应
typedef struct tag_QryIsOptionStockResp
{
	uint32 userid;				//用户id
	char szStkCode[STKCODELEN];	//股票代码，内容格式：SH000001
	uint8  exist;				//0:不是自选股,1:是自选股
}QryIsOptionStockResp_t;

//用户登录提醒gateway -> stocknotice
typedef struct tag_UserLogonNotify
{
	uint32 userid;		//用户id
	uint8  isLogin;		//是否登录
}UserLogonNotify_t;

//查询个股所属板块请求
typedef struct tag_QryStkInfoReq
{
	char szStkCode[STKCODELEN];	//股票代码，内容格式：SH000001
}QryStkInfoReq_t;

//板块信息结构体
typedef struct tag_BlankInfo
{
	char szBlkCode[STKCODELEN];	//板块代码
	char szBlkName[STKNAMELEN];	//板块名称
}BlankInfo_t;

//查询个股所属板块响应
typedef struct tag_QryStkInfoResp
{
	uint16 headlen;				//包头长度，即sizeof(QryStkInfoResp_t)
	char szStkCode[STKCODELEN];	//股票代码，内容格式：SH00001
	uint8	num;				//列表数量
	uint8	lenUnit;			//结构体单位长度
	BlankInfo_t list[0];
}QryStkInfoResp_t;

//客户端订阅指定命令字的请求(客户端发往gateway)
typedef struct tag_SubCmdReq
{
	uint8 action;      //操作1，订阅0，取消订阅
	uint16 subcmd;      //订阅的子命令	
}SubCmdReq_t;

typedef struct tag_RegistReq
{
	uint32	userid;
	uint8	devtype;	//0-PC,1-Android,2-IOS,3-web
}RegistReq_t;

typedef struct tag_SubCmdLstReq
{
	uint32	userid;
	uint8	action;		//1:subscribe  0:dis  subscribe
	uint32	jsonlen;	//subscribe  subcmd  JSON  LEN
	char	json[0];	//subscribe  subcmd  JSON  CMD  LIST:  {cmdlist:[10006]}
}SubCmdLstReq_t;

typedef struct tag_RoomBroadCast
{
	uint32 	roomid;
	uint16 	jsonlen;
	char 	json[0];
}RoomBroadCast_t;


#pragma pack()

//////////////////////////////////////////////////////////////////////////
typedef enum
{
	//mask:0xFF(mic state)
	FT_ROOMUSER_STATUS_PUBLIC_MIC     = 0x00000001,   //公麦状态(任何人可以连接)
	FT_ROOMUSER_STATUS_PRIVE_MIC      = 0x00000002,   //私麦状态(任何人可以连接)
	FT_ROOMUSER_STATUS_SECRET_MIC     = 0x00000004,   //密麦状态(连接时只有好友自动通过连接,其他拒绝,这里现在都要要求验证同意)
	FT_ROOMUSER_STATUS_CHARGE_MIC     = 0x00000010,    //收费状态 (上了收费麦)

	//mask:0xF0
	FT_ROOMUSER_STATUS_IS_TEMPOP      = 0x00000020,    //临时管理 标识
	FT_ROOMUSER_STATUS_IS_PIG         = 0x00000040,    //是否是猪头? 标识
	FT_ROOMUSER_STATUS_IS_FORBID      = 0x00000080,    //被禁言(用户被禁言,假死,不能说话,不能发送小喇叭) 标识,可以被解禁

	//mask:0x0F00(device), 增加新的mic状态:收费密麦 (2012.04.05 by guchengzhi)
	FT_ROOMUSER_STATUS_VIDEOON        = 0x00000100,    //视频打开 (有视频设备,可以发出数据)
	FT_ROOMUSER_STATUS_MICOFF         = 0x00000200,    //麦克关闭 (在麦，但不出声音)
	FT_ROOMUSER_STATUS_VIDEOOFF       = 0x00000400,    //视频关闭 (有视频设备，不允许连接)
	FT_ROOMUSER_STATUS_IS_HIDE        = 0x00000800,    //隐身(在登录时使用名字冗余字段进行初次隐身进入房间操作或默认隐身进场)
	FT_ROOMUSER_STATUS_CHAIRMAN_MIC   = 0x00001000,    //zhuximoshi ()

	//mask: 盟主(0xF000)
	FT_ROOMUSER_STATUS_IS_SIEGE1      = 0x00002000,    //标识
	FT_ROOMUSER_STATUS_IS_SIEGE2      = 0x00004000,

	//mask: 区长(0xF0000)
	FT_ROOMUSER_STATUS_IS_QUZHUANG    = 0x00010000,    //区长

}e_userinroomstate;

typedef enum
{
	FT_ROOMOPSTATUS_CLOSE_PUBCHAT     = 0x00000001,   //关闭公聊/禁止聊天回复
	FT_ROOMOPSTATUS_CLOSE_PRIVATECHAT = 0x00000002,   //关闭私聊/禁止问股
	FT_ROOMOPSTATUS_CLOSE_SAIZI       = 0x00000004,   //关闭塞子
	FT_ROOMOPSTATUS_CLOSE_PUBMIC      = 0x00000010,   //关闭公麦 (不允许上)
	FT_ROOMOPSTATUS_CLOSE_PRIVATEMIC  = 0x00000020,   //关闭私麦 (不允许上)
	FT_ROOMOPSTATUS_OPEN_AUTOPUBMIC   = 0x00000040,   //打开排麦自动上公麦
	FT_ROOMOPSTATUS_OPEN_WAITMIC      = 0x00000080,   //打开排麦功能
	FT_ROOMOPSTATUS_CLOSE_COLORBAR    = 0x00000100,   //屏蔽彩条
	FT_ROOMOPSTATUS_CLOSE_FREEMIC     = 0x00000200,   //自由排麦/禁止点赞
	FT_ROOMOPSTATUS_CLOSE_RECORDVEDIO = 0x00000400,   //用于标识禁止用户录制视频
	FT_ROOMOPSTATUS_CLOSE_ROOM        = 0x00000800,   //关闭聊天室/屏蔽互动聊天
	FT_ROOMOPSTATUS_FREE_ROOM		  = 0x00001000,   //自由模式
	FT_ROOMOPSTATUS_CHAIR_ROOM        = 0x00002000,   //主席模式

}e_roomopstate;

typedef enum
{
	FT_ROOMOPSTAT_RECORDVEDIO     	= 0x01,   //请求管理用户录制视频标识位,bit0:1 禁止 0 允许
}e_roomopstatreq;

typedef enum
{
	FT_SCOPE_ALL           = 0,     //gcz++ 不限
	FT_SCOPE_ROOM          = 1,     //房间 
	FT_SCOPE_GLOBAL        = 2,     //gcz++ 全局
}e_violatioscope;

#endif //__CMD_VCHAT_HH_20110409__

