#include "CUserBasicInfo.h"
#include "json/json.h"
#include "CLogThread.h"
#include "errcode.h"
#include "message_vchat.h"
#include "macro_define.h"
#include "UserObj.h"
#include "DBTools.h"
#include "DBSink.h"
#include "HttpClient.h"
redis_map_map CUserBasicInfo::m_user_hashmap;
redis_map_map CGlobalInfo::m_Global_hashmap;
CUserBasicInfo::CUserBasicInfo()
{
}

int CUserBasicInfo::init(redisMgr * pRedisMgr)
{
	if (!pRedisMgr)
	{
		LOG_PRINT(log_error, "redisMgr is null.init failed.");
		return -1;
	}
	
	m_user_hashmap.init(DB_USERBASIC_DB, KEY_HASH_USER_INFO":", pRedisMgr);
	return 0;
}

bool CUserBasicInfo::find(unsigned int userid)
{
	return m_user_hashmap.exist(userid);
}

int CUserBasicInfo::getUserGender(unsigned int userid)
{
	if (!userid)
	{
		LOG_PRINT(log_error, "getUserGender failed,because input userid is 0.");
		return -1;
	}

	std::string strGender = "";
	do 
	{
		IF_METHOD_FALSE_BREAK_EX(m_user_hashmap.get(userid, CFieldName::USER_GENDER, strGender), "fail to get user gender from redis.userid:%u.", userid);
		if (!strGender.empty())
		{
			return atoi(strGender.c_str());
		}
		else
		{
			LOG_PRINT(log_warning, "[user basic info]gender is missing in redis, need to reload user basic info from db to redis.userid:%u.", userid);
		}

	} while (0);

	StUserFullInfo_t oBasicInfo;
	if (ERR_CODE_FAILED_USERNOTFOUND != loadUserBasic2Redis(userid, oBasicInfo))
	{
		return oBasicInfo.gender;
	}
	else
	{
		return -1;
	}	
}

std::string CUserBasicInfo::getUserAlias(unsigned int userid)
{
	if (!userid)
	{
		LOG_PRINT(log_error, "getUserAlias failed,because input userid is 0.");
		return "";
	}

	std::string strAlias = "";
	do 
	{
		IF_METHOD_FALSE_BREAK_EX(m_user_hashmap.get(userid, CFieldName::USER_ALIAS, strAlias), "fail to get user alias from redis.userid:%u.", userid);
		if (!strAlias.empty())
		{
			return strAlias;
		}
		else
		{
			LOG_PRINT(log_warning, "[user basic info]alias is missing in redis, need to reload user basic info from db to redis.userid:%u.", userid);
		}

	} while (0);

	StUserFullInfo_t oBasicInfo;
	loadUserBasic2Redis(userid, oBasicInfo);
	return oBasicInfo.strAlias;
}

std::string CUserBasicInfo::getUserHeadAddr(unsigned int userid)
{
	if (!userid)
	{
		LOG_PRINT(log_error, "getUserHeadAddr failed,because input userid is 0.");
		return "";
	}

	std::string strHeadAddr = "";
	do 
	{
		IF_METHOD_FALSE_BREAK_EX(m_user_hashmap.get(userid, CFieldName::USER_HEAD, strHeadAddr), "fail to get user headaddr from redis.userid:%u.", userid);
		if (!strHeadAddr.empty())
		{
			return strHeadAddr;
		}
		else
		{
			LOG_PRINT(log_warning, "[user basic info]headaddr is missing in redis, need to reload user basic info from db to redis.userid:%u.", userid);
		}

	} while (0);

	StUserFullInfo_t oBasicInfo;
	loadUserBasic2Redis(userid, oBasicInfo);
	return oBasicInfo.strHead;
}

bool CUserBasicInfo::getUserHeadAndAlias(unsigned int userid, std::string & strHead, std::string & strAlias)
{
	if (!userid)
	{
		return false;
	}

	strHead = "";
	strAlias = "";
	CUserObj oUserObj;
	if (getUserInfo(userid, oUserObj))
	{
		strHead = oUserObj.headpic_;
		strAlias = oUserObj.calias_;
		return true;
	}
	else
	{
		return false;
	}
}

bool CUserBasicInfo::getUserInfo(unsigned int userid, std::map<std::string, std::string> & oFieldValueMap)
{
	oFieldValueMap.clear();
	if (!userid)
	{
		return false;
	}

	do 
	{
		IF_METHOD_FALSE_BREAK_EX(m_user_hashmap.getall(userid, oFieldValueMap), "fail to get user all info from redis.userid:%u.", userid);
		if (!oFieldValueMap.empty())
		{
			return true;
		}
		else
		{
			LOG_PRINT(log_warning, "[user basic info]user info is missing in redis, need to reload user basic info from db to redis.userid:%u.", userid);
		}

	} while (0);

	StUserFullInfo_t oBasicInfo;
	if (ERR_CODE_FAILED_USERNOTFOUND != loadUserBasic2Redis(userid, oBasicInfo))
	{
		genUserBasic2RedisMap(oBasicInfo, oFieldValueMap);
		return true;
	}
	else
	{
		LOG_PRINT(log_error, "loadUserBasic2Redis(userid, oBasicInfo) failed");
		return false;
	}
}

bool CUserBasicInfo::getUserInfo(unsigned int userid, CUserObj & oUserObj)
{
	if (!userid)
	{
		LOG_PRINT(log_error, "getUserInfo input error,userid is 0");
		return false;
	}

	std::map<std::string, std::string> oFieldValueMap;
	if (getUserInfo(userid, oFieldValueMap))
	{
		genRedisMap2UserObj(oFieldValueMap, oUserObj);
		return true;
	}
	else
	{
		LOG_PRINT(log_error, "getUserInfo(userid, oFieldValueMap) failed");
		return false;
	}
}

bool CUserBasicInfo::getUserInfo(unsigned int userid, const std::string & fieldName, std::string & value)
{
	if (!userid)
	{
		LOG_PRINT(log_error, "getUserInfo input error,userid is 0,field:%s.", fieldName.c_str());
		return false;
	}

	if (!find(userid))
	{
		IF_METHOD_NOTZERO_RETURN_EX(loadUserBasic2Redis(userid), false, "set user info fail.userid:%u.", userid);
	}

	return m_user_hashmap.get(userid, fieldName, value);
}

bool CUserBasicInfo::getUserInfo(unsigned int userid, stUserBasicInfo &info)
{
	CUserObj oUserObj;
	if (getUserInfo(userid, oUserObj))
	{
		info.userid = userid;
		info.strHead = oUserObj.headpic_;
		info.strAlias = oUserObj.calias_;
		info.nGender = oUserObj.ngender_;
		//LOG_PRINT(log_info, "tmp_info: headpic_[%s], calias_[%s]", oUserObj.headpic_.c_str(), oUserObj.calias_.c_str());
		return true;
	}
	else
	{
		LOG_PRINT(log_error, "getUserInfo(userid, oUserObj) failed")
		return false;
	}
}

bool CUserBasicInfo::setUserInfo(unsigned int userid, std::map<std::string, std::string> & oFieldValueMap)
{
	if (!userid)
	{
		LOG_PRINT(log_error, "setUserInfo input error,userid is 0");
		return false;
	}

	if (!find(userid))
	{
		IF_METHOD_NOTZERO_RETURN_EX(loadUserBasic2Redis(userid), false, "set user info fail.userid:%u.", userid);
	}

	std::map<std::string, std::string> oRedisFieldValue;
	std::map<std::string, std::string>::iterator iter_map = oFieldValueMap.begin();
	for (; iter_map != oFieldValueMap.end(); ++iter_map)
	{
		if (std::string::npos != CFieldName::USER_FIELDS.find(iter_map->first.c_str()))
		{
			oRedisFieldValue[iter_map->first] = iter_map->second;
		}
	}

	return m_user_hashmap.insert(userid, oRedisFieldValue);
}

int CUserBasicInfo::loadUserBasic2Redis(unsigned int userid, StUserFullInfo_t & oBasicInfo)
{
	if (!userid)
	{
		LOG_PRINT(log_error, "load user info input error,userid is 0");
		return ERR_CODE_FAILED_USERNOTFOUND;
	}

	IF_METHOD_NOTZERO_RETURN_EX(CDBSink().get_user_fullinfo_DB(oBasicInfo, userid), ERR_CODE_FAILED_USERNOTFOUND, "get user info fail from DB.userid:%u", userid);
	//LOG_PRINT(log_info, "tmp_info: oBasicInfo.strAlias = %s after CDBSink().get_user_fullinfo_DB()", oBasicInfo.strAlias.c_str());
	std::map<std::string, std::string> oFieldValueMap;
	genUserBasic2RedisMap(oBasicInfo, oFieldValueMap);
	IF_METHOD_FALSE_RETURN_EX(m_user_hashmap.insert(userid, oFieldValueMap), ERR_CODE_FAILED_REDIS, "insert user info to redis fail.userid:%u", userid);

	//IF_METHOD_FALSE_RETURN_EX(m_user_hashmap.expireat(userid, 60*2), ERR_CODE_FAILED_REDIS, "expireat user info to redis fail.userid:%u", userid);
	
	return 0;
}

int CUserBasicInfo::loadUserBasic2Redis(unsigned int userid)
{
	if (!userid)
	{
		LOG_PRINT(log_error, "load user info input error,userid is 0");
		return ERR_CODE_FAILED_USERNOTFOUND;
	}

	StUserFullInfo_t oBasicInfo;
	return loadUserBasic2Redis(userid, oBasicInfo);
}

void CUserBasicInfo::genUserBasic2RedisMap(const StUserFullInfo_t & oBasicInfo, std::map<std::string, std::string> & oFieldValueMap)
{
	oFieldValueMap.insert(make_pair(CFieldName::USER_ALIAS,	oBasicInfo.strAlias));
	oFieldValueMap.insert(make_pair(CFieldName::USER_HEAD,	oBasicInfo.strHead));
	oFieldValueMap.insert(make_pair(CFieldName::USER_GENDER,	bitTostring(oBasicInfo.gender)));
	oFieldValueMap.insert(make_pair(CFieldName::USER_CODE,	bitTostring(oBasicInfo.code)));
	oFieldValueMap.insert(make_pair(CFieldName::USER_CODE_TIME,	bitTostring(oBasicInfo.code_time)));
}

void CUserBasicInfo::genRedisMap2UserObj(std::map<std::string, std::string> & oFieldValueMap, CUserObj & oUserObj)
{
	std::map<std::string, std::string>::const_iterator iter_map = oFieldValueMap.find(CFieldName::USER_ALIAS);
	if (oFieldValueMap.end() != iter_map)
	{
		oUserObj.calias_ = iter_map->second;
	}

	iter_map = oFieldValueMap.find(CFieldName::USER_HEAD);
	if (oFieldValueMap.end() != iter_map)
	{
		oUserObj.headpic_ = iter_map->second;
	}

	iter_map = oFieldValueMap.find(CFieldName::USER_GENDER);
	if (oFieldValueMap.end() != iter_map)
	{
		oUserObj.ngender_ = (byte)(atoi(iter_map->second.c_str()));
	}
	iter_map = oFieldValueMap.find(CFieldName::USER_CODE);
	if (oFieldValueMap.end() != iter_map)
	{
		oUserObj.code = iter_map->second.c_str();
	}
	iter_map = oFieldValueMap.find(CFieldName::USER_CODE_TIME);
	if (oFieldValueMap.end() != iter_map)
	{
		oUserObj.code_time = iter_map->second.c_str();
	}
}
CGlobalInfo::CGlobalInfo()
{
}

int CGlobalInfo::init(redisMgr * pRedisMgr)
{
	if (!pRedisMgr)
	{
		LOG_PRINT(log_error, "redisMgr is null.init failed.");
		return -1;
	}

	m_Global_hashmap.init(DB_ROOM, KEY_HASH_GLOBAL_INFO, pRedisMgr);
	return 0;
}
int CGlobalInfo::getGzhtoken(std::string& token,unsigned int& tokenTime)
{
	std::string ttime;
	std::string strresp;
	do
	{
		CDBSink().get_token_DB(token,tokenTime);
//		IF_METHOD_FALSE_BREAK_EX(m_Global_hashmap.get(1, CFieldName::TOKEN, token), "fail to getGzhtoken from redis");
//		IF_METHOD_FALSE_BREAK_EX(m_Global_hashmap.get(1, CFieldName::TOKEN_TIME, ttime), "fail to getGzhtoken from redis");
		if (!ttime.empty() && !token.empty())
		{
			unsigned int t1 = time(0) ;
			int t2 = atoi(ttime.c_str()) - t1;
			int t3 = 0.5*60*60;
			if( t2 < t3)
			{
				CHttpClient httpClient;
				httpClient.Post(/*"http://test.talk.99cj.com.cn/UpdateAccessToken/update"*/CDBSink().qrySysParameterString("UPDATE_ACCESS_TOKEN").c_str(), "", strresp);//http://wechat.99cj.com.cn/UpdateAccessToken/update
			}else
			{
				tokenTime = atoi(ttime.c_str());
				return 1;
			}
		}
		else
		{
			LOG_PRINT(log_warning, "[user basic info]getGzhtoken from db to redis.");
		}

	} while (0);

	CDBSink().get_token_DB(token,tokenTime);

//	std::map<std::string, std::string> oFieldValueMap;
//	oFieldValueMap[CFieldName::TOKEN] = token;
//	oFieldValueMap[CFieldName::TOKEN_TIME] = bitTostring(tokenTime);
//	m_Global_hashmap.insert(1,oFieldValueMap);
	return 1;
}
