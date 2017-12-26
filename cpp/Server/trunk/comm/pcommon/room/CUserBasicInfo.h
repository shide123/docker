#ifndef __USER_BASIC_INFO_HH__
#define __USER_BASIC_INFO_HH__
#include <set>
#include "yc_datatypes.h"
#include "table2.h"
#include "redisMgr.h"
#include "redis_map_map.h"
#include "UserObj.h"
#include "comm_struct.h"

typedef std::map<std::string, std::string> FIELD_VALUE_MAP;
const std::string updateurl = "http://test.talk.99cj.com.cn/UpdateAccessToken/update";
typedef struct tag_UserBasicInfo
{
	tag_UserBasicInfo():userid(0),nGender(){}
	unsigned int userid;
	std::string strAlias;	//昵称
	std::string strHead;	//头像
	int nGender;	//性别
	std::string code;
	unsigned int code_time;
	int inroomstate;
} stUserBasicInfo;

class CUserBasicInfo
{
public:
    CUserBasicInfo();

	static int init(redisMgr * pRedisMgr);
	static int getUserGender(unsigned int userid);
	static std::string getUserAlias(unsigned int userid);
	static std::string getUserHeadAddr(unsigned int userid);
	static bool getUserHeadAndAlias(unsigned int userid, std::string & strHead, std::string & strAlias);
	static bool getUserInfo(unsigned int userid, std::map<std::string, std::string> & oFieldValueMap);
	static bool getUserInfo(unsigned int userid, const std::string & fieldName, std::string & value);
	static bool getUserInfo(unsigned int userid, CUserObj & oUserObj);
	static bool getUserInfo(unsigned int userid, stUserBasicInfo &info);
	static bool setUserInfo(unsigned int userid, std::map<std::string, std::string> & oFieldValueMap);
	static int loadUserBasic2Redis(unsigned int userid);
private:
	static bool find(unsigned int userid);
	static int loadUserBasic2Redis(unsigned int userid, StUserFullInfo_t & oBasicInfo);
	static void genUserBasic2RedisMap(const StUserFullInfo_t & oBasicInfo, std::map<std::string, std::string> & oFieldValueMap);
	static void genRedisMap2UserObj(std::map<std::string, std::string> & oFieldValueMap, CUserObj & oUserObj);
	static redis_map_map m_user_hashmap;
};
class CGlobalInfo
{
public:
	CGlobalInfo();
	static int init(redisMgr * pRedisMgr);
	static int getGzhtoken(std::string& token,unsigned int& tokenTime);
private:
	static redis_map_map m_Global_hashmap;
};
#endif //__USER_BASIC_INFO_HH__
