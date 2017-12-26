/*********************************************************************
 *  Copyright 2016  by 99letou.
 *  All right reserved.
 *
 *  功能：用户信息管理
 *
 *  Edit History:
 *
 *    2016/02/23  - Created  by Laimingming.
 *    2016/04/20  - Moded  by Zhangshaochen for add cast the pkt to a userlist.
 */

#include "CUserMgr.h"
#include "CLogThread.h"
#include "timestamp.h"
#include "message_vchat.h"
#include "clienthandler.h"
#include "sstream"
#include "DBTools.h"
#include "CFieldName.h"
#include "CUserBasicInfo.h"
#include "CUserGroupinfo.h"

CUserMgr::CUserMgr()
{
    m_connUserMap.clear();
}

CUserMgr::~CUserMgr()
{
}

int CUserMgr::init(redisMgr * pRedisMgr)
{
	if (!pRedisMgr)
	{
		LOG_PRINT(log_error, "redisMgr is null.init failed.");
		return -1;
	}

	m_redis_userinfo_map.init(DB_USER_MGR, KEY_HASH_USER_INFO":", pRedisMgr);
	return 0;
}

void CUserMgr::resetRedisDB()
{
	m_redis_userinfo_map.cleandb(DB_USER_MGR);
}

//返回0说明该用户信息是初次加入登陆
//返回1说明该用户信息是再次加入登陆，connOldObj返回旧的连接，userOldObj返回旧的用户。
LOGIN_RESULT CUserMgr::userLogin(const CConnInfo & connObj, const CUserinfo & userObj, CConnInfo & connOldObj, CUserinfo & userOldObj)
{	
	CConnInfo connMemObj;
	CUserinfo userMemObj;
	int ret = getUserOnlineInfo(userObj.getUserid(), connMemObj, userMemObj);
	if (ret < 0)
	{
		LOG_PRINT(log_error, "get user online failed.userid:%u.", userObj.getUserid());
		return eFail;
	}
    else if (ret == 0)
    {
        //用户首次加入
		setUserOnlineInfo(connObj, userObj);
		m_connUserMap[connObj].insert(userObj);

        return eFirstLogin;
    }
    else
    {
        //缓存里已有该用户ID，说明该用户是用相同终端类型(PC\手机\Web)登录，逻辑按首次加入来需踢人
        //比较登录时间和connID
        if (userMemObj.getLoginTime() == userObj.getLoginTime() && userMemObj.getConnID() == userObj.getConnID() && connMemObj == connObj)
        {
            char sztime[128] = { 0 };
            toStringTimestamp3(userObj.getLoginTime(), sztime);
			LOG_PRINT(log_normal, "[CUserMgr]user(%u,%d,%s(%u),gateway:%s,%u)is the same,so not need to handle.", \
                userObj.getUserid(), (int)userObj.getDevType(), sztime, (unsigned int)userMemObj.getLoginTime(), connObj.getIpaddress().c_str(), connObj.getPort());
            return eDefault;
        }
        else if (userMemObj.getLoginTime() <= userObj.getLoginTime())
        {
            //缓存里的登陆时间是较早时间,即:缓存里的登陆时间 < 输入用户的登陆时间
			//或者存在毫秒的差异但是秒数是一样的,但connID或connObj跟缓存里的不一致
            userOldObj = userMemObj;
			connOldObj = connMemObj;

            //把老对象的链接找到并且删除老对象
            CONN_USER_MAP::iterator iterConn = m_connUserMap.find(connMemObj);
			if (iterConn != m_connUserMap.end())
			{
				iterConn->second.erase(userOldObj);
				if (iterConn->second.empty())
				{
					m_connUserMap.erase(connMemObj);
				}
			}

            //找新对象的链接并插入新对象
			m_connUserMap[connObj].insert(userObj);

            //刷新redis为新的对象
			setUserOnlineInfo(connObj, userObj);
        }
        else
        {
            //缓存里的登陆时间 > 输入用户的登陆时间,说明当前输入的时间还早,不需要更新缓存,直接返回
            userOldObj = userObj;
            connOldObj = connObj;
        }
		return eRelogin;
    }
}

void CUserMgr::findUser_termtype(unsigned int userid, byte termtype, CONN_USER_MAP & conn_map)
{
    conn_map.clear();
    LOG_PRINT(log_normal, "find userid:%u,termtype:%d.", userid, (int)termtype);

	CConnInfo connObj;
	CUserinfo userObj;
	int ret = getUserOnlineInfo(userid, connObj, userObj);
	if (ret <= 0)
	{
		LOG_PRINT(log_warning, "cannot find this userid:%u.", userid);
		return;
	}

    DEVTYPE devtype = userObj.getDevType();
    time_t logintime = userObj.getLoginTime();
    char sztime[128] = { 0 };
    toStringTimestamp3(logintime, sztime);

    if (e_Notice_AllType != termtype && devtype != termtype)
    {
        LOG_PRINT(log_normal, "userObj(%u,%d,%s):devtype is not equal input(%d).",userid, (int)devtype, sztime, (int)termtype);
        return;
    }

    LOG_PRINT(log_normal, "find out this userObj(%u,%d,%s(%u))", userid, (int)devtype, sztime, (unsigned int)logintime);
	conn_map[connObj].insert(userObj);
}

void CUserMgr::delUser(const CConnInfo & connObj, const CUserinfo & userObj)
{
	unsigned int userid = userObj.getUserid();
	time_t logintime = userObj.getLoginTime();
	DEVTYPE devtype = userObj.getDevType();
	uint32 connID = userObj.getConnID();
	char sztime[128] = {0};
	toStringTimestamp3(logintime, sztime);

	std::string strIP = connObj.getIpaddress();
	unsigned int iPort = connObj.getPort();

	do
	{
		CConnInfo connMemObj;
		CUserinfo userMemObj;
		int ret = getUserOnlineInfo(userObj.getUserid(), connMemObj, userMemObj);
		if (ret <= 0)
		{
			break;
		}

		if (connMemObj == connObj && userMemObj == userObj && userMemObj.getConnID() == connID)
		{
			//delete user in redis
			m_redis_userinfo_map.erase(userid);
			LOG_PRINT(log_info, "del user in redis.user(%u,%d,%s(%u),connid:%u.gateway:%s,%u) logout.", \
				userid, (int)devtype, sztime, (unsigned int)logintime, connID, strIP.c_str(), iPort);
		}

	}while(0);

    CONN_USER_MAP::iterator iterMap = m_connUserMap.find(connObj);
    if (m_connUserMap.end() != iterMap)
    {
        std::set<CUserinfo>::iterator iterSet = iterMap->second.find(userObj);
        if (iterMap->second.end() != iterSet && connID == iterSet->getConnID())
        {
            iterMap->second.erase(userObj);
			LOG_PRINT(log_info, "del user in connUserMap.user(%u,%d,%s(%u),connid:%u.gateway:%s,%u) logout.", \
				userid, (int)devtype, sztime, (unsigned int)logintime, connID, strIP.c_str(), iPort);
        }
    }
}

void CUserMgr::delAllUser(const std::string & strIP, unsigned int iPort)
{
	LOG_PRINT(log_warning, "[CUserMgr]delAllUser(gateway:%s:%u).", strIP.c_str(), iPort);

    CConnInfo connObj(strIP, iPort);

    CONN_USER_MAP::iterator iterConn = m_connUserMap.find(connObj);
    if (m_connUserMap.end() == iterConn)
    {
        return;
    }

    std::set<CUserinfo> & allUser = iterConn->second;
    std::set<CUserinfo>::iterator iterSet = allUser.begin();
    for (; allUser.end() != iterSet; ++iterSet)
    {
        const CUserinfo & delUserObj = *iterSet;
        
		unsigned int userid = delUserObj.getUserid();
		time_t logintime = delUserObj.getLoginTime();
		DEVTYPE devtype = delUserObj.getDevType();
		uint32 connID = delUserObj.getConnID();
		char sztime[128] = {0};
		toStringTimestamp3(logintime, sztime);

		do
		{
			CConnInfo connMemObj;
			CUserinfo userMemObj;
			int ret = getUserOnlineInfo(delUserObj.getUserid(), connMemObj, userMemObj);
			if (ret <= 0)
			{
				break;
			}

			if (connMemObj == connObj && userMemObj == delUserObj && userMemObj.getConnID() == connID)
			{
				//delete user in redis
				m_redis_userinfo_map.erase(userid);
				LOG_PRINT(log_info, "delAllUser in redis.user(%u,%d,%s(%u),connid:%u.gateway:%s,%u).", \
					userid, (int)devtype, sztime, (unsigned int)logintime, connID, strIP.c_str(), iPort);
			}

		}while(0);
    }
    
    m_connUserMap.erase(connObj);
}

bool CUserMgr::setUserOnlineInfo(const CConnInfo & connObj, const CUserinfo & userObj)
{
	unsigned int userid = userObj.getUserid();
	
	std::map<std::string, std::string> oFieldValueMap;
	//user info
	oFieldValueMap.insert(std::make_pair(CUserinfo::DEV_TYPE, bitTostring((unsigned int)userObj.getDevType())));
	oFieldValueMap.insert(std::make_pair(CUserinfo::LOGIN_TIME, bitTostring((unsigned int)userObj.getLoginTime())));
	oFieldValueMap.insert(std::make_pair(CUserinfo::GATE_CONNID, bitTostring(userObj.getConnID())));

	//connect info
	oFieldValueMap.insert(std::make_pair(CConnInfo::GATE_IP, connObj.getIpaddress()));
	oFieldValueMap.insert(std::make_pair(CConnInfo::GATE_PORT, bitTostring(connObj.getPort())));
	oFieldValueMap.insert(std::make_pair(CConnInfo::GATE_ID, bitTostring(connObj.getGateid())));

	m_redis_userinfo_map.insert(userid, oFieldValueMap);
}

int CUserMgr::getUserOnlineInfo(unsigned int userid, CConnInfo & connObj, CUserinfo & userObj)
{
	std::set<std::string> field_name_set;
	field_name_set.insert(std::string(CConnInfo::GATE_IP));
	field_name_set.insert(std::string(CConnInfo::GATE_PORT));
	field_name_set.insert(std::string(CConnInfo::GATE_ID));
	field_name_set.insert(std::string(CUserinfo::DEV_TYPE));
	field_name_set.insert(std::string(CUserinfo::LOGIN_TIME));
	field_name_set.insert(std::string(CUserinfo::GATE_CONNID));

	std::map<std::string, std::string> field_value_map;
	bool ret = m_redis_userinfo_map.getall(userid, field_value_map);
	if (!ret)
	{
		LOG_PRINT(log_error, "hgetall user on line info fail from redis.userid:%u.", userid);
		return -1;
	}

	if (field_value_map.empty())
	{
		LOG_PRINT(log_info, "userid:%u do not have online info in redis.", userid);
		return 0;
	}

	std::set<std::string>::iterator iter = field_name_set.begin();
	for (; iter != field_name_set.end(); ++iter)
	{
		std::string value = "";
		std::string key = *iter;
		
		std::map<std::string, std::string>::iterator iter_map = field_value_map.find(key);
		if (field_value_map.end() == iter_map)
		{
			LOG_PRINT(log_warning, "cannot find userid:%u field name:%s value.", userid, key.c_str());
			continue;
		}
		
		value = iter_map->second;

		if (strcmp(key.c_str(), CConnInfo::GATE_IP) == 0)
		{
			connObj.setIpaddress(value);
		}
		else if (strcmp(key.c_str(), CConnInfo::GATE_PORT) == 0)
		{
			connObj.setPort(atoi(value.c_str()));
		}
		else if (strcmp(key.c_str(), CConnInfo::GATE_ID) == 0)
		{
			connObj.setGateid(atoi(value.c_str()));
		}
		else if (strcmp(key.c_str(), CUserinfo::GATE_CONNID) == 0)
		{
			userObj.setConnID(atoi(value.c_str()));
		}
		else if (strcmp(key.c_str(), CUserinfo::DEV_TYPE) == 0)
		{
			userObj.setDevType((DEVTYPE)atoi(value.c_str()));
		}
		else if (strcmp(key.c_str(), CUserinfo::LOGIN_TIME) == 0)
		{
			userObj.setLoginTime((time_t)atoi(value.c_str()));
		}
	}
	
	userObj.setUserid(userid);
	return 1;
}

int CUserMgr::modUserInfo(unsigned int userid, const std::map<std::string, std::string> & oFieldValueMap, std::set<unsigned int> & groupIDSet)
{
	if (!userid || oFieldValueMap.empty())
	{
		LOG_PRINT(log_error, "modify user info error,userid is 0.field_value_map size:%u.", oFieldValueMap.size());
		return -1;
	}

	int ret = 0;
	do 
	{
		if (!CDBSink().update_userinfo_DB(userid, oFieldValueMap))
		{
			LOG_PRINT(log_error, "userid:%u modify user info fail in DB.", userid);
			ret = -1;
			break;
		}

		//update user basic info in redis
		std::map<std::string, std::string> oRedisFieldValue;
		std::map<std::string, std::string>::const_iterator iter_map = oFieldValueMap.begin();
		for (; iter_map != oFieldValueMap.end(); ++iter_map)
		{
			if (std::string::npos != CFieldName::USER_FIELDS.find(iter_map->first.c_str()))
			{
				oRedisFieldValue[iter_map->first] = iter_map->second;
			}
		}

		if (!oRedisFieldValue.empty())
		{
			CUserBasicInfo::setUserInfo(userid, oRedisFieldValue);
			CUserGroupinfo::getUserGroupIDs(userid, groupIDSet);
		}

	} while (0);

	if (ret == 0 && !groupIDSet.empty())
	{
		LOG_PRINT(log_info, "userid:%u change some key values successfully,need to notify group members.", userid);
	}

	return ret;
}
