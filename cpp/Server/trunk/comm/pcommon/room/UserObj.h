#pragma once

#include "yc_datatypes.h"
#include "json/json.h"
#include "clienthandler.h"
#include "redisMgr.h"
#include "redisOpt.h"
#include "redis_map_map.h"
#include "CFieldName.h"

typedef enum _enum_UserType 
{
	USERTYPE_NORMALUSER	= 0,	//普通用户
	USERTYPE_NPC		= 1,	//挂机npc（机器人，转播员）
    USERTYPE_MICNPC		= 2,	//守麦员
	USERTYPE_SOFTBOT	= 3,	//软件机器人
}e_UserType;

class CUserObj;
typedef boost::shared_ptr<CUserObj>  UserObject_Ref;
class CUserObj
{
public:
	CUserObj();
	CUserObj(const vector<string> & fields, const vector<string> & vals);
	CUserObj(const std::map<std::string, std::string> & field_values);
	virtual ~CUserObj();
	static void init(redisMgr *pRedisMgr)
	{
		m_userRoomInfo.init(0, KEY_HASH_USER_INFO":", pRedisMgr);
		m_onlineInfo.init(1, KEY_HASH_USER_INFO":", pRedisMgr);
	}
	void reset();
    string getRedisKey();
    string getRedisFields();
    void getRedisValues(Json::Value &jOut, const string &fields);
	void setUserInfo2Redis(redisOpt *opt, std::string fields = "");
	bool updateOnlineInfo();
	bool isNormalUser();
	bool isZhuanboUser();
	static UserObject_Ref findUserOnline(uint32 userid);

public:
	uint32 nuserid_;
	uint32 user_level;
	uint32 age;
	uint32 gender;
	std::string head_add;
	std::string name;
	std::string alias;
	std::string login_name;
	std::string password;
	std::string email;
	std::string qq;
	std::string id_card;
	uint32 birthday;
	std::string register_ip;
	uint32 register_date;
	std::string last_login_ip;
	std::string last_login_mac;
	std::string last_login_time;
	std::string tel;
	std::string code;
	std::string code_time;
	uint32 freeze;
	uint32 is_recommend;
	std::string tag;
	std::string intro;
	std::string addr;
	uint32 is_daka;


	int 	m_OnMicSec;
	bool 	bIsMember;
	uint32 	nprivate_userid;
	uint32	nprivate_time;
	uint32 	noptype;
	uint32 	reqid;
//    uint32  nuserid_;				//用户id
	uint32	ngroupid;				//群id
	uint16_t   nsvrid;				//roomsvr id
	uint16_t   ngateid;				//gateway id
	byte  ngender_;
	byte  nage_;
	byte  nuserlevel_;				//角色身份 0：游客 10：正式成员 20：嘉宾  30：管理员  40：群主
	byte  nuserviplevel_;			//vip用户等级（黄金vip，白金vip 等)
	byte  ndevtype;					//登陆的设备类型(0:PC端 1:安卓 2:IOS 3:WEB)

    byte  ncuronmictype_;			//当前在麦的类型,0-不在麦,1-公麦,2-私麦, 用来计算麦时
	char  ncurpublicmicindex_;		//当前在公麦的位置
	byte  ninroomlevel_;			//当前在房间内等级

	uint32 nstarttime_pubmic_;		//在公麦的开始时间
	uint32 nstarttime_mic_;        
	uint32 nwillendtime_pubmic_;	//在公麦的应该结束时间 (如果是限时麦) ,0 不限时

	uint32 inroomstate_;			//在房间状态
	uint32 cometime_;				//用户进来时间

	uint32 nstarflag_;
	uint32 nactivityflag_;
	int32  bForbidInviteUpMic_;		//禁止抱麦
	int32  bForbidChat_;			//禁言

	e_UserType type_;				//用户类型

	//连接器信息
	clienthandler_ptr pConn_;
	uint32	pConnId_;				//这里指的是gate-connid
	uint64	pGateObj_;				//在gate上面的标示1
	uint64	pGateObjId_;			//在gate上面的标示2
	bool	bExceptExit;			//是否异常退出了

	string calias_;         		//昵称
	string headpic_;
	string macserial_;
	string uuid_;
	string ipaddr_;
	string areaid;					//区域id
    string mediaip;					//媒体服务器ip和端口
    string gateip;					//网关服务器ip和端口

private:
	void getRedisValuesMap(std::map<std::string, std::string> &mValues, const string fields);
	void loadStaticInfo();
private:
	static redis_map_map m_onlineInfo;
	static redis_map_map m_userRoomInfo;
};

//typedef boost::shared_ptr<CUserObj>  UserObject_Ref;
