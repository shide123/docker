#ifndef __COMM_STRUCT_DEFINE_H__
#define __COMM_STRUCT_DEFINE_H__

#include "yc_datatypes.h"
#include <string>
#include <vector>

typedef struct tag_RedPacket
{
	unsigned int packetID;
	unsigned int groupID;
	unsigned int srcUserID;
	unsigned int dstUserID;
	bool isPrivate;
	unsigned int packetType;
	unsigned int rangeType;
	unsigned int packetNum;
	unsigned int takeNum;
	unsigned int createTime;
	unsigned int invalidTime;
	unsigned long long fixPerMoney;
	unsigned long long packetMoney;
	unsigned long long takeMoney;
	unsigned int moneyState;
	std::string strMessage;
	unsigned int isRecycle;
	unsigned int finishTime;
	unsigned int rangeGender;
	unsigned int fixTime;
	unsigned int fixType;
	unsigned int diffTime;
	double longitude;
	double latitude;
	unsigned int luckyNum;
}stRedPacketInfo;

typedef struct tag_ContributeValue
{
	unsigned int userID;
	unsigned int groupID;
	unsigned int total_contribute;
	unsigned int near_week_contribute;	//最近7天
}stContributeValue;

typedef struct tag_RedPakcetLog
{
	uint32 packetId;
	uint32 userId;
	unsigned long long money;
	uint32 time;
}stRedPacketLog;

typedef struct tag_GroupInfo
{
	unsigned int groupid;
	unsigned int showid;
	std::string name;
	std::string headaddr;
	unsigned int muteSet;
	unsigned int roleType;
	unsigned int memberCount;
	unsigned int visitCount;
}stGroupInfo;

typedef struct tag_RoomFullInfo
{
	uint32 ncourseid;
	uint32 nuser_id;							//群主
	uint32 nlive_id;
	std::string class_name;
	std::string teacher_name;
	std::string teacher_info;
	std::string img;
	std::string src_img;
	uint32 nprice;
	uint32 nlevel;
	std::string strpwd;
	std::string invite_code;
	std::string strremark;
	std::string tags;
	std::string goal;
	std::string dest_user;
	std::string requirements;
	uint32 nbegin_time;
	uint32 nend_time;
	uint32 nstatus;
	uint32 nopenstatus;
	uint32 npublish_time;
	uint32 ncreate_time;
	uint32 nupdate_time;
	uint32 talk_status;
	uint32 virtual_num;
	uint32 form;
}stRoomFullInfo;

typedef struct tag_StRoomNotice
{
	tag_StRoomNotice()
	{
		m_groupID = 0;
		m_strRoomNotice = "";
		m_uBroadUserid = 0;
		m_uBroadTime = 0;
	}

	uint32		m_groupID;				//群ID
	std::string m_strRoomNotice;		//公告信息
	uint32		m_uBroadUserid;			//公告发布者userid
	uint32		m_uBroadTime;			//公告发布时间
}StRoomNotice_t;

typedef struct tag_StUserGroupInfo
{
	uint32		userID;					//用户ID
	uint32 		groupID;
	uint32		roleType;				//角色
	uint32 		noDisturbing;
	uint32 		gainPercent;
	uint32 		pchatThreshold;
	uint32 		authState;
	std::string videoUrl;
	uint32 		is_daka;
}StUserGroupInfo;

typedef struct tag_StUserBaseAndGroupInfo
{
	std::string userAlias;				//用户昵称
	std::string userHead;				//用户头像
	uint32		userGender;				//用户性别
	StUserGroupInfo	userGroupInfo;
}StUserBaseAndGroupInfo;

typedef struct tag_StGPAssistMsg
{
	tag_StGPAssistMsg()
	{
		userID = 0;
		groupID = 0;
		msgId = 0;
		msgSvrType = 0;
		msgSvrSwitch = false;
		msgSvrLevelUp = false;
		msgSvrLevel = 0;
		msgState = 0;
		msgTime = 0;
		url = "";
		msgType = 0;
		relate_msgid = 0;
		userContext = "";
		authID = 0;
		investUserID = 0;
		packetid = 0;
		balance = 0;
	}

	uint32 userID;						//用户id
	uint32 groupID;						//群id
	uint32 msgId;						//msgid
	uint32 msgSvrType;					//功能类型 e_MsgSvrType
	bool   msgSvrSwitch;				//功能打开或关闭
	bool   msgSvrLevelUp;				//功能等级提升或降低 1为提升 0为降低
	uint32 msgSvrLevel;					//功能等级
	uint32 msgState;					//消息状态
	uint32 msgTime;						//消息时间
	std::string url;					//url
	uint32 msgType;						//消息类型(0-系统消息 1-特殊消息)
	uint32 relate_msgid;				//相关联的消息ID(比如审核通过的消息,这里填的是提交审核的消息ID)
	std::string userContext;			//留言
	uint32 authID;						//认证ID,对应mc_authentication的id
	uint32 investUserID;				//邀请者ID
	std::string investUserAlias;		//邀请者昵称
	uint32 packetid;					//红包id
	uint32 balance;						//红包余额
}StGPAssistMsg_t;

typedef struct tag_SystemMsg
{
	uint32 msgid;
	int8 msgType;
	std::string content;
	std::string action;
	uint32 createTime;
	uint32 endTime;
}StNoticeMsg_t;

typedef struct tag_Condition
{
	int8 type;
	std::string oper;
	std::string value;
}StCondition_t;

typedef struct tag_SysConfig
{
	tag_SysConfig()
	{
		paramName = "";
		paramValue = "";
	}

	std::string paramName;
	std::string paramValue;
}SysConfig_t;

typedef struct tag_StUserFullInfo
{
	tag_StUserFullInfo()
	{
		userid = 0;
		userlevel = 0;
		age = 0;
		gender = 0;
		strHead = "";
		strAlias = "";
		nfreeze = 0;
		strBirthday = "";
		strTel = "";
	}

	unsigned int userid;
	unsigned int userlevel;
	int age;
	int gender;
	std::string strHead;
	std::string strAlias;
	int nfreeze;
	std::string strBirthday;
	std::string strTel;
	std::string code;
	std::string code_time;
	unsigned int invite_id;
}StUserFullInfo_t;
typedef struct App2Key_tag
{
	App2Key_tag()
	{
		appid = "";
		key = "";
	}
	std::string appid;
	std::string key;
}App2Key_t;
typedef std::vector<App2Key_t> VAppId2Key;

typedef struct BD_Mod_t
{
    std::string apikey;	        //string 	是 	应用的api key,用于标识应用。
    uint timestamp;	        	//uint 	    是 	用户发起请求时的unix时间戳。本次请求签名的有效时间为该时间戳向后10分钟。
    std::string sign; 	        //string 	是 	调用参数签名值，与apikey成对出现。用于防止请求内容被篡改, 生成方法请参考。
    uint expires;           	//uint 	    否 	用户指定本次请求签名的失效时间。格式为unix时间戳形式，用于防止 replay 型攻击。为保证防止 replay攻击算法的正确有效，请保证客户端系统时间正确。
    uint device_type;	    	//uint  	否 	当一个应用同时支持多个设备平台类型（比如：Android和iOS），请务必设置该参数。其余情况可不设置。具体请参见：device_type参数使用说明，android:3,ios:4
}BD_Mod;

typedef struct Push_Batch_Device_Mod_tag:public BD_Mod
{
    std::string channel_ids; 	//string 	是 	一组channel_id（最多为一万个）组成的json数组字符串 	对应一批设备
    uint msg_type;      		//number 	否 	取值如下：0：消息；1：通知。默认为0 	消息类型
    std::string msg;         	//string 	是 	详情见消息/通知数据格式 	消息内容，json格式
    uint msg_expires;   		//number 	否 	0~604800(86400*7)，默认为5小时(18000秒) 	相对于当前时间的消息过期时间，单位为秒
    std::string topic_id;    	//string 	是 	字母、数字及下划线组成，长度限制为1~128 	分类主题名称
}Push_Batch_Device_Mod;

typedef struct tipUserinfo
{
	uint32 srcid;
	std::string srcname;
	std::string srchead;
	int amount;
	std::string gifname;
	std::string picname;
	bool operator == (const tipUserinfo & info)
	{
		return info.srcname == srcname;
	}
};

typedef struct _pptPicInfo
{
	uint32 iGroupId;
	uint32 iRank;
	std::string sPicId;
	std::string sPicUrl;
	
}pptPicInfo;

struct commentAudit
{
	int iUid;
	int	iGroupid;
	std::string sContent;
	int iMsgid;
	int iPmsgid;
	int iMsgtype;

	int iStatus;

};
#endif //__COMM_STRUCT_DEFINE_H__
