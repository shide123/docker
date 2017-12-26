#ifndef __TABLE2_H
#define __TABLE2_H

#include <string.h>
//#pragma pack(1)
//#pragma pack()
//sprintf输出long long使用格式符%ld，LONGLONG则是%I64d。

/*
字符串转64位
linux下
long long strtol("1234", NULL, 10 );

windows 下
__int64 _atoi64("123456");
因此可以做如下宏定义：
#ifndef WIN32
#define _atoi64(val)     strtoll(val, NULL, 10)
#endif 
*/
//---

#include <time.h>
#include <stdint.h>

#ifdef _MSC_VER
typedef __int64				int64;
#else
typedef long long			int64;
#endif
//---

//#define PWDLEN			  32  //密码长度
//#define MD5PWDLEN         33  //md5密码长度,固定32+1

#pragma pack(1)

typedef struct tag_DDUserLangID
{
	int nid;
	int nuserid;
	int nlangid;
	char davadate[32];
	char dexpdate[32];
}DDUserLangID_t;

typedef struct tag_DDUserFullInfo
{
	int nuserid;
	int nuserlevel;
	long long nk;
	long long nb;
	long long nd;
	long long nkdeposit;
	char nage;
	char ngender;
	char chead[64 + 1];
	char cname[32 + 1];
	char calias[64 + 1];
	char cloginname[32 + 1];
	char cpassword[64 + 1];
	char cemail[64 + 1];	//邮箱
	char cqq[32 + 1];		//QQ
	char ctel[32 + 1];		//手机
	int nfreeze;
    char cbirthday[32 + 1];	//生日 format:2015-12-21
}DDUserFullInfo_t;

typedef struct tag_DDUserPwdInfo
{
	int userid;
	char cpassword[33];
	char crepasswd[33];
	char cbankpwd[33];
}DDUserPwdInfo_t;

typedef struct tag_DDUserBaseInfo
{
	int userid;
	int headid;
	int gender;
	char calias[64];
}DDUserBaseInfo_t;

typedef struct tag_DDUserFreezeInfo
{
	int nuserid;
	char dfreezedate[32];
	char dfreezedate2[32];
}DDUserFreezeInfo_t;

typedef struct tag_DDRoomGroupItem
{
    int groupid;
	int glevel;
	int parentid;
	int bshowflag;
	int bshownum;   //是否显示人数
	int sortid;
	int ncolor;
	int bfontbold;
	char cname[32];
	char ciconname[32];
	char curl[128];
}DDRoomGroupItem_t;

typedef struct tag_DDRoomGroupUserNum
{
	int grouid;
	int usernum;
}DDRoomGroupUserNum_t;

typedef struct tag_DDRoomFullInfo
{
   int roomid;
   int xingjiid;
   int groupid;
   int attrid;
   int seats;
   int visible;
   int creatorid;
   int op1id;
   int op2id;
   int op3id;
   int op4id;
   int mcuid;
   //int  managers[200];  //gcz -- 不再使用
   int busepwd;
   char cname[256];
   char cpassword[256];
   char croompic[256];
   char cmedia[128];
   int ntextteacherid_;
   uint64_t npopularity_;
   int viplevel;
   int roomtype;
   uint64_t activetime;
   int optstat;
   uint32_t platformid;
   uint32_t maxclientnum;
}DDRoomFullInfo_t;

typedef struct tag_DDRoomBaseInfo
{
	int roomid;
	int xingjiid;
	int groupid;
	int attrid;
	int seats;
	int visible;
	int creatorid;
	int op1id;
	int op2id;
	int op3id;
	int op4id;
	int busepwd;
	char cname[256];
	char cpassword[256];
	char croompic[256];
	char areaid[32];
	int roomtype;
	int optstat;
}DDRoomBaseInfo_t;

typedef struct tag_DDRoomNoticeInfo
{
	int roomid;
	char cbroad1[2048];
	char cbroad2[2048];
	char cbroad3[2048];
	char cbroad4[2048];
}DDRoomNoticeInfo_t;

typedef struct tag_DDRoomBroadInfo
{
	int roomid;
	int userid;
	char useralias[32];
	int index;
	char cbroad[2048];
	int ntime;
}DDRoomBroadInfo_t;

typedef struct tag_DDRoomMediaInfo
{
   int roomid;
   char cmedia[128];
   char areaid[5];
}DDRoomMediaInfo_t;

typedef struct tag_DDRoomAreaMediaInfo
{
	int roomid;
	char cmedia[128];
	int areaid;
}DDRoomAreaMediaInfo_t;

typedef struct tag_DDRoomManagerInfo
{
   int roomid;
   int  managers[200];
}DDRoomManagerInfo_t;

typedef struct tag_DDFlyGiftInfo
{
	int nid;
   int nvcbid;
   int nsrcid;
   int nbuddyid;
   int ngiftid;
   int ncount;
   int nanonymous;
   int nsrcplatid;
   int ntoplatid;
   char ctext[40];
   char dsenttime[40];
   char csrcalias[32];
   char ctoalias[32];
   //char csrcvcbname[32];
}DDFlyGiftInfo_t;

typedef struct tag_DDGiftLotteryPro {   //幸运礼物中奖率分布
	int nid;
	int ngiftid;
	int64 njiner;
	int winpro[10];  //概率分布奖金数
}DDGiftLotteryPro_t;

typedef struct tag_DDXingyunrenPro {
	int nid;
    int nuserid;
	int njctype;
	int nchi;
	int nshu;
	char dtime[40];
}DDXingyunrenPro_t;


typedef struct tag_DDQuanxianAction {
	int qxid;
	int qxtype;
	int srclevel;
	int tolevel;
}DDQuanxianAction_t;

//封杀信息
typedef struct tag_DDViolationInfo {
   int vcbid;
   int runnerid;
   int userid;
   int lefttime;
   int reasontype;
   int scopetype;
}DDViolationInfo_t;

//登陆日记信息
typedef struct tag_DDUserLoginLog {
	int userid;
	int devtype;
	char szip[32];
	char szmac[32];
    char szserial[64];
	char   devicemodel[32];     // 设备型号
	char   deviceos[16];        // 设备系统
}DDUserLoginLog_t;

//讲师打分记录
typedef struct tag_CMDTeacherScoreRecord2
{
	int teacher_userid;             //讲师ID
	char   teacheralias[32]; //讲师呢称
	int userid;                //打分人ID
	char   alias[32];        //打分人呢称
	int score;                 //分数
	char   logtime[32];      //打分时间
	int  vcbid;                //所在房间id
	//int64  roommrgid;            //所在大区id(考虑到一个房间可属于多个大区，不能直接统计，去掉)
	int64 data1;                  //备用字段1
	int64 data2;                  //备用字段2
	int64 data3;                  //备用字段3
	char   data4[32];        //备用字段4
	char   data5[32];        //备用字段5
}CMDTeacherScoreRecord_t2;

//系统公告信息
typedef struct tag_DDSysCastNews {
    int nid;
	int ntype;
	int nisoneoff;
	char cnewstext[1024];
}DDSysCastNews_t;

//烟花日志信息
typedef struct tag_DDFireworksLogTab {
	int userid; //本记录所有者
	int srcuserid; //送
	int dstuserid; //收
	int vcbid;
	int giftid; 
	int giftnum;  //烟花数量
	int64 nk_out;   //支出
	int64 nb_out;   //支出
	int64 nk_in;    //收到
	int64 nb_in;    //收到
	int64 nk;       //送完/接受完烟花收帐上k币数
	int64 nb;       //送完/接受完烟花收帐上b币数
}DDFireworksLogTab_t;

//盖章日志
typedef struct tag_DDSendSealLogTab{
	int  userid;
	int  touserid;
	int  vcbid;
	int  sealid;
	int64 nk_out;
	int64 nk;
	int64 nb;
}DDSendSealLogTab_t;

//用户开宝箱信息
typedef struct tag_DDUserOpenChestStatInfo {
	int userid;
	int totalnum;
	int64 totalcost;
	int64 totalwinval;
	int todaynum;
}DDUserOpenChestStatInfo_t;

typedef struct tag_DDUserLoginRoomLogTab
{
    int  userid;
	int  vcbid;
	char ip[20];
	char mac[40];
	char logintime[40];
	char logoutime[40];
}DDUserLoginRoomLogTab_t;

typedef struct tag_DDUserOnMicLogTab
{
	int userid;
	int vcbid;
	int mictype;
	int npay;
	char upmictime[40];
	char downmictime[40];
}DDUserOnMicLogTab_t;

typedef struct tbviewsiegelog{
	int nvcbid;
	int nsrcid;
	int nbuddyid;
	int ngiftid;
	int ncount;
	int nsenttime;
	char srcalias[32];
	char toalias[32];
}DDviewsiegelog_t;

typedef struct tag_tbusernknboplog{
	int nvcbid;
	int nsrcid;
	int ntoid;
	int noptype;
	int64 nmoney;
	int64 nkbefore;
	int64 nbbefore;
	int64 nkdepositbefore;
	int64 nkafter;
	int64 nbafter;
	int64 nkdepositafter;
	int64 nkbef2;
	int64 nbbef2;
	int64 nkaft2;
	int64 nbaft2;
	char  dtime[40];
}DDUsernknbOplog_t;

typedef struct tag_updatemgeonlinetime{
	int nvcbid;
    int nuserid;
	int mgrtype;
	int intervaltime;
}DDUpdateMgrOnlineTime_t;

//座驾信息
typedef struct tag_usercarinfo {
	int ncarid;
	char carname[32];
}DDUserCarInfo_t;

//房间命令(目前只使用机器人部分)
typedef struct tag_vcbcommand {
	int nvcbid;
	int cmdtype;
	char cmdtext[128];
	char dtime[40];
}DDvcbcommand_t;

//转播机器人消息
typedef struct tag_zhaunborobotinfo {
	int nid;
	int nvcbid;
	int npubmicidx;
	int nrobotid;
	int nrobotviplevel;
	int nrobotvcbid;
	int nstatus;
	int nuserid;
}DDzhaunborobotinfo_t;

//系统广播
typedef struct tag_updatebroadinfo{
	int nid;				
	int ntype;			//范围	1-全站，2-大区
	int ngroupid;		//大区id
	int ntiming;		//定时发送 0-关闭，1-开启
	int ntimestamp;		//时间间隔（以秒为单位）
	char ccreator[16];	//创建人
	char dtime[32];		//创建时间
	char dstarttime[32];//定时发送开启时间 0-立即发送
	char dendtime[32];	//定时发送结束时间 0-立即结束
	char ccontent[1024];//广播内容
}DDupdatebroadinfo_t;

//房间管理操作
typedef struct tag_updateblackuserinfo_t
{
	int		scope;		//1-房间 2-全站
	int		vcbid;		//房间id 0-全站
	int		userid;		//用户id
	int		runnerid;	//操作人id 0-未知
	char	reason[256];	//禁言或加入黑名单理由
}DDupdateblackuserinfo_t;

//资讯信息
typedef struct tag_loadbroadcastinfo_t
{
	char	cabstract[256];	//摘要
	char	clink[256];		//链接
}DDloadbroadcastinfo_t;

//在线时长信息
typedef struct tag_DDOnLineTimeInfo
{	
	int				userid;			//用户id
	unsigned int	onlinecount;	//在线数量	
	unsigned int	starttime;		//上线时间	以秒为单位
	unsigned int	lastupdatetime;	//上一次更新在线时长的时间
	unsigned int	onlinetime;		//在线时长
}DDOnLineTimeInfo_t;

//关键字信息
typedef struct 
{
	int		naction;				//0-刷新 1-增加 2-删除 
	int		ntype;					//广告类型
	int		nrunerid;				//操作人Id
	char	createtime[32];			//创建时间
	char	keyword[64];			//关键词
}DDAdKeywordInfo_t;

typedef struct tag_CMDTeacherThisWeekFans
{
	char alias[32];          //用户昵称
	int64 totalmoney;        //积分总额
}CMDTeacherThisWeekFans_t;

//用户安全信息(邮箱、QQ、电话号码、已提醒次数)
typedef struct tag_CMDSecureInfo
{
	char email[50];         //用户邮箱
	char qq[15];            //用户QQ
	char phonenumber[15];   //电话号码
	int remindtimes;        //已提醒次数
}CMDSecureInfo_t;

typedef struct _tagQuanxianActionItem2
{
	int quanxianActionId;
	char quanxianName[64];
	int actionType;
	int srcId;
	int toId;
}QuanxianActionItem_t2;

typedef struct _tagQuanxianIdItem2
{
	int levelId;
	char levelName[64];
	int quanxianId;
	int quanxianPrio;
	int sortId;
	int sortPrio;
}QuanxianIdItem_t2;

//用户对讲师的评分响应
typedef struct tag_CMDUserScoreNoty2
{
    int vcbid;					//房间号
    int teacherid;				//讲师ID
    int score;					//用户对讲师的评分
	int userid;					//用户ID
}UserScoreList_t;

//每一条用户对讲师评分记录
typedef struct tag_ScoreValue
{
	int score_value;
}ScoreValue_t;

//用户会员信息
typedef struct tag_CUserVip
{
	unsigned int userID;		//用户ID
	unsigned int platformID;	//服务号ID
	int groupID;				//会员组ID
	unsigned int deadLineTime;	//过期时间
}CUserVip_t;

///////////////////////////////////////////////////////////
//登入直播间记录
typedef struct tag_CMDTextRoomUserInfo
{
	int userid;                    //用户ID
	int vcbid;                     //房间ID
	int logtype;	               //客户端类型 0:PC端 1:安卓 2:IOS  3:WEB
}CMDTextRoomUserInfo_t;

//讲师信息
typedef struct tag_CMDTextRoomTeacherNoty2
{
	int   vcbid;                     //房间ID
	int   teacherid;                 //讲师ID
	char  teacheralias[32];       //讲师昵称
	int   headid;                   //讲师头像ID
	char  clevel[64];               //讲师等级
	char  clabel[128];             //讲师标签
	char  cgoodat[512];            //讲师擅长领域
	char  cintroduce[512];         //讲师简介
    long  anserCount;              //讲师已回答问题数
}CMDTextRoomTeacherNoty_t2;

//加载直播记录请求
typedef struct tag_CMDTextRoomLiveListReq2
{
	int vcbid;                           //房间ID
	int userid;                          //用户ID
	int teacherid;                       //讲师ID
	int  type;                           //类型：1-文字直播；2-直播重点；3-明日预测（已关注的用户可查看）；4-观点；
	int64  messageid;                    //上一次请求得到的最小消息ID，第一次为0
    int  count;                          //获取多少条记录
}CMDTextRoomLiveListReq_t2;

//加载直播记录响应
typedef struct tag_CMDTextRoomLiveListNoty2
{
	int vcbid;                           //房间ID
	int userid;                          //用户ID
	int teacherid;                       //讲师ID
	int srcuserid;                       //互动用户ID
	char   srcuseralias[32];             //互动用户昵称
	long long  messageid;                    //消息ID
	int  pointflag;                      //是否直播重点:0-否；1-是；
	int  forecastflag;                   //是否明日预测:0-否；1-是；
	int  livetype;                       //文字直播类型：1-纯文字；2-文字+链接；3-文字+图片；4-互动回复；5-观点动态；6-个人秘籍动态；
	long long  viewid;			             //观点ID(5-观点动态用)
	char  srctext[1024];                  //消息内容（发送的直播是内容，发送的观点是标题，互动的是源内容）
	char  destext[1024];                  //互动回复内容
	long long messagetime;                   //发送时间
    int64  zans;                         //点赞数
	char  commentstype;		             //客户端类型 0:PC端 1:安卓 2:IOS 3:WEB
}CMDTextRoomLiveListNoty_t2;

//直播重点/明日预测
typedef struct tag_CMDTextRoomLivePointNoty2
{
	int vcbid;                           //房间ID
	int userid;                          //用户ID
	int  type;                           //类型：1-文字直播；2-直播重点；3-明日预测（已关注的用户可查看）；4-观点；
	int teacherid;                       //讲师ID
	long long  messageid;                //消息ID
	int  livetype;                       //文字直播类型：1-纯文字；2-文字+链接；3-文字+图片；4-互动回复；5-观点动态；6-个人秘籍动态；
	char  content[1024];                 //消息内容
	long long messagetime;               //发送时间
    	int64  zans;                         //点赞数
	char  commentstype;		             //客户端类型 0:PC端 1:安卓 2:IOS 3:WEB
}CMDTextRoomLivePointNoty_t2;

//讲师发送文字直播请求
typedef struct tag_CMDTextRoomLiveMessageReq2
{
	int vcbid;                           //房间ID
	int teacherid;                       //讲师ID
	int  pointflag;                      //是否直播重点:0-否；1-是；
	int  forecastflag;                   //是否明日预测:0-否；1-是；
	int  livetype;                       //文字直播类型：1-纯文字；2-文字+链接；3-文字+图片；
	char   content[1024];                //直播消息内容
	int64 messagetime;                   //发送时间
	int   commentstype;		     //客户端类型 0:PC端 1:安卓 2:IOS 3:WEB
}CMDTextRoomLiveMessageReq_t2;

//讲师修改观点类型分类请求
typedef struct tag_CMDTextRoomViewTypeReq2
{
	int teacherid;                       //请求用户ID
	int  viewtypeid;                     //观点类型ID（新增时为0）
	char   viewtypename[32];             //观点类型名称
}CMDTextRoomViewTypeReq_t2;


//返回直播列表
typedef struct tag_TextLiveItem
{
	int vcbid;                  //房间ID
	int userid;                 //用户ID
	int datetime;               //日期：如20160107
	int64 beginTime;              //一天内直播第一条记录的时间
	int64 endTime;                //一天内最后一条记录的时间
	int renQi;                  //当天人气
	int cAnswer;                //回答问题的条数
	int totalCount;             //当天记录总数
}TextLiveItem_t;

typedef struct tag_DDLTeacherViewType
{
    unsigned int roomid;
    unsigned int teacherid;
    unsigned int viewpointType;
    unsigned int totalCount;
    char viewTypeName[200];
}DDLTeacherViewType_t;

typedef struct tag_DDLViewPointHisItem
{
    int64    viewID;                 //观点ID
    int          viewPointType;          //文字直播类型：1-纯文字；2-文字+链接；3-文字+图片；
    unsigned int teacherID;              //讲师ID
    long         timeStamp;              //观点发送时间
    unsigned int cLikes;                 //观点点赞数内容长度	
    unsigned int cFlowers;               //观点送花数
    unsigned int cComments;              //观点评论数
    unsigned int cReads;                 //观点浏览次数
    int          commentstype;           //客户端类型 0:PC端 1:安卓 2:IOS 3:WEB
    char         title[200];             //观点标题	
    char         content[1025];           //观点内容(部分)
}DDLViewPointHisItem_t;

typedef struct tag_CMDTextRoomUserInfo2
{
	int userid;                      //请求用户ID
	int teacherfalg;                 //是否讲师（0-不是，1-是）
	char alias[32];                  //用户昵称
	int  headid;                     //用户头像ID
}CMDTextRoomUserInfo_t2;

typedef struct tag_DDInteractRes
{
	int userid;                 //用户ID
	char   useralias[50];     //用户昵称
    unsigned int userheadid;             //用户头像
	long long  messageid;              //消息ID（用于回复时找到对应的记录）
	long long messagetime;      //回复时间(yyyymmddhhmmss)
    char   commentstype;           //客户端类型 0:PC端 1:安卓 2:IOS  3:WEB
	char   srccontent[1024];             //消息内容，格式：源内容+回复内容
	char   dstcontent[1024];             //消息内容，格式：源内容+回复内容
	char   answer[1024];             
	char   question[1024];             
	char   stoke[1024];   
}DDInteractRes_t;

typedef struct tag_DDLTextLiveForecastItem
{
    unsigned int userid;                 //用户ID
    unsigned int teacherid;              //讲师ID
    char         teacheralias[32];       //讲师昵称
    unsigned int teacherheadid;          //讲师头像
    int64        messageid;              //消息ID
    unsigned int livetype;               //文字直播类型：1-纯文字；2-文字+链接；3-文字+图片;
    unsigned int commentstype;           //客户端类型 0:PC端 1:安卓 2:IOS  3:WEB
    long         messagetime;            //发送时间(yyyymmddhhmmss)
    char         content[1024];          //消息内容
}DDLTextLiveForecastItem_t;

typedef struct tag_CMDHallMessageReq2
{
	int   userid;                  //用户ID
	int   teacherflag;             //是否讲师（0-不是，1-是）
	int   type;                    //分类：1.互动回复（11.收到的互动（用户）；12.发出的互动（讲师）；）；
							   	   //2.观点回复（21.收到的观点回复；22.发出的观点评论；）；
							   	   //3.问答提醒（31. 未回答提问；32.已回答提问；）；
							   	   //4.我的关注（41.我的粉丝；42.我的关注；43.明日预测）；
							   	   //5.系统信息；
	int64 messageid;               //请求得到的最消息ID，第一次为0
	int   startIndex;              //起始索引
	int   count;                   //请求记录数
}CMDHallMessageReq_t2;

typedef struct tag_CMDInteractRes2
{
	int    userid;                 //用户ID
	int    touserid;               //讲师ID
	char   touseralias[32];        //互动用户昵称
	int    touserheadid;           //互动用户头像
	int64  messageid;              //消息ID（用于回复时找到对应的记录）
	char   srccontent[1024];        //源内容
	char   dstcontent[1024];        //回复内容
	int64  messagetime;            //回复时间
    int    commentstype;           //客户端类型 0:PC端 1:安卓 2:IOS
    int    unread;                 //未读标记
}CMDInteractRes_t2;

typedef struct tag_SupAccountS
{
	int    recordid;        //记录ID	
    long long contribution; //消费金额
    int    userid;          //用户ID
    int    supdate;         //体验号时间
    char   alias[32];       //用户昵称
}SupAccountS_t;

typedef struct tag_SysBoardCast
{
    unsigned char newType;  //0 一次性新闻 1    
	long long   nid;        //记录ID	
    char   title[32];    
	char   content[512]; 
}SysBoardCast;

//讲师回复（包含观点回复和回答提问）
typedef struct tag_CMDViewAnswerReq2
{
	int   fromid;              //发出人
	int   toid;                //接收人
	int64 messageid;           //消息ID
	int   commentstype;        //客户端类型 0:PC端 1:安卓 2:IOS  3:WEB
	char  content[1024];       //观点回复内容
}CMDViewAnswerReq_t2;

typedef struct tag_DDL_tb_usernotify
{
    unsigned int  nuserid;      //userid
    unsigned char termtype;     //logintype
    unsigned char notitype;     //notitype
    unsigned char versionflag;  //versionflag
    unsigned int  version;      //version
    unsigned char needresp;     //needresp
    time_t        inserttime;   //inserttime
    unsigned int  validtime;    //validtime
    unsigned char state;        //state
    time_t        pushtime;     //pushtime
}DDLUserNotify;

typedef struct tag_DDL_jpushconfig
{
	unsigned char termtype;          //termtype
	unsigned char pushtype;          //pushtype
	time_t        pushtime;          //pushtime
	time_t        endtime;           //endtime
	unsigned int  interval;          //interval
	char          content[1024+1];   //content
}DDLJPushConfig;

//师徒list
typedef struct tag_ShiTuItem2
{
	int   userid;
	long long startTime;
	long long effectTime;
	char alias[32];
	int  cQuery;
	int  cViewFlowers;
}ShiTuItem_t2;

typedef struct tag_privilegeItem2
{
	int  index;
	char privilege[256];
}PrivilegeItem2_t2;

typedef struct tag_CMDTextRoomSecretsListResp2
{
	int  secretsid;                //秘籍ID
	char  coverlittle[256];        //封面小图名称长度
	char  title[128];              //秘籍标题长度
	char  text[1024];              //秘籍简介长度
	int64  messagetime;            //时间(yyyymmddhhmmss)
	int    buynums;                //订阅人数
	int    prices;                 //单次订阅所需玖玖币
	int   goodsid;                 //商品ID
}CMDTextRoomSecretsListResp_t2;

typedef struct tag_CMDTextRoomSecretsListOwn2
{
	int  secretsid;                //秘籍ID
}CMDTextRoomSecretsListOwn_t2;

typedef struct tag_DDTeacherSubscribeItem
{
    int     userid;                 //userid
}DDTeacherSubscribeItem;

typedef struct tag_DDUservcbFavoriteItem
{
    int     userid;                 //userid
}DDUservcbFavoriteItem;

typedef struct tag_DDZhuanboRobotInfo {
    int nid;
    int nvcbid;
    int npubmicidx;
    int nrobotid;
    int nrobotviplevel;
    int nrobotvcbid;
    int nstatus;
    int nuserid;
}DDZhuanboRobotInfo_t;

typedef struct tag_VistorLogon
{
	  unsigned int     userid;		//userid
	  char             cuuid[64];	//uuid
	  unsigned char    state;		//0:not logon,1:logon
	  int64            logontime;
}VistorLogon_t;

typedef struct tag_DictItem
{
	char group[32];
	char item[32];
	char addvalue[64];
}DictItem_t;

typedef struct tag_PushInfo
{
    uint16_t  pushtype;       //推送类型（1 小米2 华为 3 IOS）
    char    pushtoken[64];
}PushInfo_t;

typedef struct tag_JoinGroupInfo
{
	unsigned int	userID;
	unsigned int	groupID;
	unsigned char	in_way;
	unsigned char	role_type;
	unsigned int	levelID;
}stJoinGroupInfo;

typedef struct tag_GroupMemInfo
{
	char userAlias[150];
	char userHeadAddr[255];
	unsigned char userGender;
	stJoinGroupInfo userGroupInfo;
}stGroupMemInfo;

#pragma pack()

#endif


