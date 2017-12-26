#ifndef __USER_GROUP_INFO_HH__
#define __USER_GROUP_INFO_HH__
#include <set>
#include "yc_datatypes.h"
#include "redisMgr.h"
#include "redis_map_map.h"
#include "redis_map_set.h"
#include "UserObj.h"
#include "DBTools.h"
#include "DBSink.h"

typedef std::map<std::string, std::string> FIELD_VALUE_MAP;
class IRedisInterface
{
public:
	virtual bool getData() = 0 ;
	virtual bool setData() = 0 ;
};
template<typename T1,typename T2 = redis_map_map>
class CRedisMap:IRedisInterface
{
public:
	CRedisMap(){}
	virtual ~CRedisMap(){}
	static bool init(redisMgr * pRedisMgr);
private:
	T2 m_t2;
};
class CUserGroupinfo
{
public:
    CUserGroupinfo();

	static int init(redisMgr * pRedisMgr);
	static bool find(unsigned int userid, unsigned int groupid);
	static bool loadUserGroupInfo2Redis(unsigned int userid, unsigned int groupid, bool * isMember = NULL);
	static bool isMember(unsigned int userid, unsigned int groupid);
	static int getUserRoleType(unsigned int userid, unsigned int groupid, unsigned int & roleType);
	static int getUserRoleType(unsigned int userid, unsigned int groupid);
	static bool setUserRoleType(unsigned int userid, unsigned int groupid, unsigned int roleType);
	static bool erase(unsigned int userid, unsigned int groupid);
	static bool getUserGroupIDs(unsigned int userid, std::set<unsigned int> & oGroupIDs);
	static bool getall(unsigned int groupid, unsigned int userid, FIELD_VALUE_MAP & field_value_map);
	static int getall(unsigned int groupid, const std::set<std::string> & fieldSet, std::map<unsigned int, FIELD_VALUE_MAP > & user_value_map);
	template<class Type>
	static bool isValueChanged(unsigned int userid, unsigned int groupid, const std::string field, Type value);
	template<class Type>
	static bool modUserGroupInfo(unsigned int userid, unsigned int groupid, const std::string field, Type value, bool modDB = true);
	static bool modUserGroupInfo(unsigned int userid, unsigned int groupid, const FIELD_VALUE_MAP &mValues, bool modDB = true);
	static bool chkUserRoleBeyond(unsigned int userid, unsigned int groupid, unsigned int roleType);
	static bool getAllGroupMembers(unsigned int groupid, std::vector<unsigned int> &vMembers);
	static bool getAllGroupMembers(unsigned int groupid, std::set<unsigned int> &vMembers);
	static unsigned int getGroupMemberNum(unsigned int groupid);
private:
	static int get(unsigned int userid, unsigned int groupid, const std::string & field, std::string & value);
	static bool insert(unsigned int userid, unsigned int groupid, const std::string field, const std::string value);
	static bool insert(unsigned int userid, unsigned int groupid, const FIELD_VALUE_MAP & field_value_map);
	static redis_map_map m_usergroupinfo;
	static redis_map_set m_usergroup_set;

};

template<class Type>
bool CUserGroupinfo::isValueChanged(unsigned int userid, unsigned int groupid, const std::string field, Type value)
{
	std::string oldValue;
	if (get(userid, groupid, field, oldValue) == 0 && oldValue == bitTostring(value))
	{
		LOG_PRINT(log_info, "group[%u] user[%u] field[%s] value[%s], modification is the same as old value", groupid, userid, field.c_str(), oldValue.c_str());
		return false;
	}

	return true;
}
template<class Type>
bool CUserGroupinfo::modUserGroupInfo(unsigned int userid, unsigned int groupid, const std::string field, Type value, bool modDB /*= true*/)
{
	if (modDB && !CDBSink().updGroupMemberInfo(groupid, userid, field, value))
	{
		return false;
	}

	if (find(userid, groupid))
	{
		return CUserGroupinfo::insert(userid, groupid, field, bitTostring(value));
	}
	else
	{
		return loadUserGroupInfo2Redis(userid, groupid);
	}
}

#endif //__USER_GROUP_INFO_HH__
