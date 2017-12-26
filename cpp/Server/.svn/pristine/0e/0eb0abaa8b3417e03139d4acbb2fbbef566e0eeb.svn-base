#include "CUserGroupinfo.h"
#include "json/json.h"
#include "message_vchat.h"
#include "macro_define.h"
#include "errcode.h"
#include "utils.h"
#include "DBTools.h"
#include "comm_struct.h"

redis_map_map CUserGroupinfo::m_usergroupinfo;
redis_map_set CUserGroupinfo::m_usergroup_set;

CUserGroupinfo::CUserGroupinfo()
{
}

int CUserGroupinfo::init(redisMgr * pRedisMgr)
{
	if (!pRedisMgr)
	{
		LOG_PRINT(log_error, "redisMgr is null.init failed.");
		return -1;
	}

	m_usergroupinfo.init(DB_USERGROUP_DB, KEY_SET_GROUPID":", pRedisMgr);
	m_usergroup_set.init(DB_USERGROUP_DB, KEY_USER_GROUPS":", pRedisMgr);
	return 0;
}

bool CUserGroupinfo::find(unsigned int userid, unsigned int groupid)
{
	if (!userid || !groupid)
	{
		LOG_PRINT(log_error, "input error, userid:%u,groupid:%u.", userid, groupid);
		return false;
	}

	LOG_PRINT(log_debug, "[find user exist]userid:%u,groupid:%u.", userid, groupid);
	return m_usergroupinfo.find(groupid, userid);
}

bool CUserGroupinfo::isMember(unsigned int userid, unsigned int groupid)
{
	if (!userid || !groupid)
	{
		LOG_PRINT(log_error, "isMember input error,userid:%u,groupid:%u.", userid, groupid);
		return false;
	}

	if (find(userid, groupid))
	{
		return true;
	}
	else
	{
		bool bMember = false;
		loadUserGroupInfo2Redis(userid, groupid, &bMember);
		return bMember;
	}
}

bool CUserGroupinfo::loadUserGroupInfo2Redis(unsigned int userid, unsigned int groupid, bool * isMember/*= NULL*/)
{
//	if (!userid || !groupid)
//	{
//		LOG_PRINT(log_error, "loadUserGroupInfo2Redis input error,userid:%u,groupid:%u.", userid, groupid);
//		return false;
//	}
//
//	StUserGroupInfo oUserGroupInfo;
//	int ret = CDBSink().get_userGroupInfo_DB(userid, groupid, oUserGroupInfo);
//	if (ret != 0)
//	{
//		LOG_PRINT(log_error, "get_userGroupInfo_DB error,userid:%u,groupid:%u.", userid, groupid);
//		return false;
//	}
//	else
//	{
//		if (isMember)
//		{
//			*isMember = true;
//		}
//		std::map<std::string, std::string > oFieldValueMap;
//		oFieldValueMap[CFieldName::USER_ROLE_TYPE]				= bitTostring(oUserGroupInfo.roleType);
//		oFieldValueMap[CFieldName::USER_GAIN_PERCENT]			= bitTostring(oUserGroupInfo.gainPercent);
//		IF_METHOD_FALSE_RETURN_EX(insert(userid, groupid, oFieldValueMap), false, "loadUserGroupInfo2Redis fail,userid:%u,groupid:%u.", userid, groupid);
//		return true;
//	}
}

int CUserGroupinfo::getUserRoleType(unsigned int userid, unsigned int groupid, unsigned int & roleType)
{
	roleType = e_VisitorRole;
	if (!userid || !groupid)
	{
		LOG_PRINT(log_error, "input error,userid:%u,groupid:%u.", userid, groupid);
		return -1;
	}

	int ret = 0;
	do 
	{
		std::string strRoleType = "";
		ret = get(userid, groupid, CFieldName::USER_ROLE_TYPE, strRoleType);
		if (ret == 0 && !strRoleType.empty())
		{
			roleType = atoi(strRoleType.c_str());
			return 0;
		}

	} while (0);

	LOG_PRINT(log_warning, "redis do not have this user in group.userid:%u,groupid:%u.", userid, groupid);
	return 0;
}


int CUserGroupinfo::getUserRoleType(unsigned int userid, unsigned int groupid)
{
	int roleType = e_VisitorRole;
	if (!userid || !groupid)
	{
		LOG_PRINT(log_error, "input error,userid:%u,groupid:%u.", userid, groupid);
		return roleType;
	}

	std::string strRoleType = "";
	int ret = get(userid, groupid, CFieldName::USER_ROLE_TYPE, strRoleType);
	if (ret == 0 && !strRoleType.empty())
	{
		roleType = atoi(strRoleType.c_str());
	}

	return roleType;
}

bool CUserGroupinfo::setUserRoleType(unsigned int userid, unsigned int groupid, unsigned int roleType)
{
	if (find(userid, groupid))
	{
		return CUserGroupinfo::insert(userid, groupid, CFieldName::USER_ROLE_TYPE, bitTostring(roleType));
	}
	else
	{
		loadUserGroupInfo2Redis(userid, groupid);
		return CUserGroupinfo::insert(userid, groupid, CFieldName::USER_ROLE_TYPE, bitTostring(roleType));
	}
}

bool CUserGroupinfo::insert(unsigned int userid, unsigned int groupid, const std::string field, const std::string value)
{
	if (!userid || !groupid)
	{
		LOG_PRINT(log_error, "input error, userid:%u,groupid:%u.", userid, groupid);
		return false;
	}

	LOG_PRINT(log_debug, "[insert user]userid:%u,groupid:%u,field:%s,value:%s.", userid, groupid, field.c_str(), value.c_str());
	bool ret = true;
	std::string json_value = "";
	if (m_usergroupinfo.find(groupid, userid, json_value))
	{
		Json::Reader reader(Json::Features::strictMode());
		Json::Value root;
		if (reader.parse(json_value, root)) 
		{
			root[field] = Json::Value(value);
			
			Json::FastWriter fast_writer;
			std::string strJson = fast_writer.write(root);
			m_usergroupinfo.insert(groupid, userid, strJson);
		}
		else
		{
			LOG_PRINT(log_error, "[insert user fail]userid:%u,groupid:%u,field:%s,value:%s,json_value:%s.", \
				userid, groupid, field.c_str(), value.c_str(), json_value.c_str());
			ret = false;
		}
	}
	else
	{
		Json::Value root;
		root[CFieldName::USER_USERID] = Json::Value(bitTostring(userid));
		root[CFieldName::USER_GROUPID] = Json::Value(bitTostring(groupid));
		root[field] = Json::Value(value);

		Json::FastWriter fast_writer;
		std::string strJson = fast_writer.write(root);
		m_usergroupinfo.insert(groupid, userid, strJson);
	}

	if (!m_usergroup_set.count(userid, groupid))
	{
		LOG_PRINT(log_debug, "userid:%u first join groupid:%u.", userid, groupid);
		m_usergroup_set.insert(userid, groupid);
	}

	return ret;
}

bool CUserGroupinfo::insert(unsigned int userid, unsigned int groupid, const FIELD_VALUE_MAP & field_value_map)
{
	if (!userid || !groupid)
	{
		LOG_PRINT(log_error, "input error, userid:%u,groupid:%u.", userid, groupid);
		return false;
	}

	LOG_PRINT(log_debug, "[insert user]userid:%u,groupid:%u,field_value_size:%u.", userid, groupid, field_value_map.size());
	bool ret = true;
	std::string json_value = "";
	if (m_usergroupinfo.find(groupid, userid, json_value))
	{
		Json::Reader reader(Json::Features::strictMode());
		Json::Value root;
		if (reader.parse(json_value, root)) 
		{
			FIELD_VALUE_MAP::const_iterator iter_map = field_value_map.begin();
			for (; iter_map != field_value_map.end(); ++iter_map)
			{
				root[iter_map->first] = Json::Value(iter_map->second);
			}

			Json::FastWriter fast_writer;
			std::string strJson = fast_writer.write(root);
            LOG_PRINT(log_debug, "userid:%u,groupid:%u,json string:%s.", userid, groupid, strJson.c_str());
			m_usergroupinfo.insert(groupid, userid, strJson);
		}
		else
		{
			LOG_PRINT(log_error, "[insert user fail]userid:%u,groupid:%u,field_value_size:%u,json_value:%s.", \
				userid, groupid, field_value_map.size(), json_value.c_str());
			ret = false;
		}
	}
	else
	{
		Json::Value root;
		root[CFieldName::USER_USERID] = Json::Value(bitTostring(userid));
		root[CFieldName::USER_GROUPID] = Json::Value(bitTostring(groupid));
		FIELD_VALUE_MAP::const_iterator iter_map = field_value_map.begin();
		for (; iter_map != field_value_map.end(); ++iter_map)
		{
			root[iter_map->first] = Json::Value(iter_map->second);
		}

		Json::FastWriter fast_writer;
		std::string strJson = fast_writer.write(root);
		m_usergroupinfo.insert(groupid, userid, strJson);
	}

	if (!m_usergroup_set.count(userid, groupid))
	{
		LOG_PRINT(log_debug, "userid:%u first join groupid:%u.", userid, groupid);
		m_usergroup_set.insert(userid, groupid);
	}

	return ret;
}

bool CUserGroupinfo::erase(unsigned int userid, unsigned int groupid)
{
	if (!userid || !groupid)
	{
		LOG_PRINT(log_error, "input error, userid:%u,groupid:%u.", userid, groupid);
		return false;
	}

	LOG_PRINT(log_debug, "[erase user]userid:%u,groupid:%u.", userid, groupid);
	IF_METHOD_FALSE_RETURN_EX(m_usergroupinfo.erase(groupid, userid), false, "[erase user fail in redis]userid:%u,groupid:%u.", userid, groupid);
	IF_METHOD_FALSE_RETURN_EX(m_usergroup_set.erase(userid, groupid), false, "[erase user fail in redis]userid:%u,groupid:%u.", userid, groupid);
	return true;
}

bool CUserGroupinfo::getUserGroupIDs(unsigned int userid, std::set<unsigned int> & oGroupIDs)
{
	oGroupIDs.clear();
	if (!userid)
	{
		LOG_PRINT(log_error, "input error, userid is 0.", userid);
		return false;
	}

	std::vector<std::string> vGroupID;
	IF_METHOD_FALSE_RETURN_EX(m_usergroup_set.find(userid, vGroupID), false, "[getUserGroupIDs fail from redis]userid:%u.", userid);
	if (!vGroupID.empty())
	{
		for (int i = 0; i < vGroupID.size(); ++i)
		{
			oGroupIDs.insert(atoi(vGroupID[i].c_str()));
		}
	}
	LOG_PRINT(log_debug, "[getUserGroupIDs]userid:%u,group number:%u.", userid, oGroupIDs.size());
	return true;
}

int CUserGroupinfo::get(unsigned int userid, unsigned int groupid, const std::string & field, std::string & value)
{
	if (!userid || !groupid)
	{
		LOG_PRINT(log_error, "input error, userid:%u,groupid:%u.", userid, groupid);
		return -1;
	}

	int ret = 0;
	LOG_PRINT(log_debug, "[get user]userid:%u,groupid:%u,field:%s.", userid, groupid, field.c_str());
	std::string json_value = "";
	if (m_usergroupinfo.find(groupid, userid, json_value))
	{
		Json::Reader reader(Json::Features::strictMode());
		Json::Value root;
		if (reader.parse(json_value, root)) 
		{
			if (root.isMember(field))
			{
				value = root[field].asString();
			}
			else
			{
				LOG_PRINT(log_error, "[get user fail]userid:%u,groupid:%u do not have field:%s,json_value:%s.", \
					userid, groupid, field.c_str(), json_value.c_str());
				ret = -2;
			}
		}
		else
		{
			LOG_PRINT(log_error, "[get user fail]userid:%u,groupid:%u,field:%s,json_value:%s.", \
				userid, groupid, field.c_str(), json_value.c_str());
			ret = -2;
		}
	}
	else
	{
		LOG_PRINT(log_error, "[get user]not find userid:%u in groupid:%u.", userid, groupid);
		ret = ERR_CODE_USER_NOT_IN_GROUP;
	}

	return ret;
}

int CUserGroupinfo::getall(unsigned int groupid, const std::set<std::string> & fieldSet, std::map<unsigned int, FIELD_VALUE_MAP > & user_value_map)
{
	if (!groupid || fieldSet.empty())
	{
		LOG_PRINT(log_error, "input error, groupid:%u,fields size:%u.", groupid, fieldSet.size());
		return -1;
	}

	std::map<std::string, std::string > user_json_map; 
	if (!m_usergroupinfo.getall(groupid, user_json_map))
	{
		LOG_PRINT(log_error, "getall groupid:%u error.", groupid);
		return -1;
	}

	if (user_json_map.empty())
	{
		LOG_PRINT(log_error, "getall groupid:%u is empty.", groupid);
		return 0;
	}

	std::map<std::string, std::string >::iterator iter_users = user_json_map.begin();
	for (; iter_users != user_json_map.end(); ++iter_users)
	{
		unsigned int userid = atoi(iter_users->first.c_str());
		std::string strJson = iter_users->second;
		if (!strJson.empty())
		{
			Json::Reader reader(Json::Features::strictMode());
			Json::Value root;
			if (reader.parse(strJson, root)) 
			{
				std::set<std::string>::const_iterator iter_set = fieldSet.begin();
				for (; iter_set != fieldSet.end(); ++iter_set)
				{
					if (root.isMember(*iter_set))
					{
						user_value_map[userid][*iter_set] = root[*iter_set].asString();
					}
				}
			}
		}
	}
	return 0;
}

bool CUserGroupinfo::getall(unsigned int groupid, unsigned int userid, FIELD_VALUE_MAP & field_value_map)
{
	if (!userid || !groupid)
		return false;

	std::string json_value = "";
	if (m_usergroupinfo.find(groupid, userid, json_value))
	{
		Json::Reader reader(Json::Features::strictMode());
		Json::Value root;
		if (reader.parse(json_value, root)) 
		{
			Json::Value::Members oMembers = root.getMemberNames();
			Json::Value::Members::iterator iter_v = oMembers.begin();
			for (; iter_v != oMembers.end(); ++iter_v)
			{
				if (root[*iter_v].type() == Json::stringValue)
				{
					field_value_map[*iter_v] = root[*iter_v].asString();
				}
			}

			LOG_PRINT(log_debug, "[get user group info]userid:%u,groupid:%u have field value map size:%u,json_value:%s.", \
				userid, groupid, field_value_map.size(), json_value.c_str());
			return true;
		}
		else
		{
			LOG_PRINT(log_error, "[get user fail]userid:%u,groupid:%u,json_value:%s.", \
				userid, groupid, json_value.c_str());
			return false;
		}
	}
	else
	{
		LOG_PRINT(log_error, "[get user]not find userid:%u in groupid:%u.", userid, groupid);
		return false;
	}
}

bool CUserGroupinfo::chkUserRoleBeyond(unsigned int userid, unsigned int groupid, const unsigned int roleType)
{
	unsigned int realRole = 0;
	if (getUserRoleType(userid, groupid, realRole) == 0)
	{
		return realRole >= roleType;
	}

	return false;
}

bool CUserGroupinfo::getAllGroupMembers(unsigned int groupid, std::vector<unsigned int> &vMembers)
{
	std::vector<std::string> vKeys;
	if (m_usergroupinfo.gethashkeys(groupid, vKeys))
	{
		vMembers.resize(vKeys.size());
		for (int i = 0; i < vKeys.size(); i++)
		{
			vMembers[i] = atoi(vKeys[i].c_str());
		}
		return true;
	}

	return false;
}

bool CUserGroupinfo::getAllGroupMembers(unsigned int groupid, std::set<unsigned int> &vMembers)
{
	std::vector<std::string> vKeys;
	if (m_usergroupinfo.gethashkeys(groupid, vKeys))
	{
		for (int i = 0; i < vKeys.size(); i++)
		{
			vMembers.insert(atoi(vKeys[i].c_str()));
		}
		return true;
	}

	return false;
}

unsigned int CUserGroupinfo::getGroupMemberNum(unsigned int groupid)
{
	return m_usergroupinfo.size(groupid);
}

bool CUserGroupinfo::modUserGroupInfo(unsigned int userid, unsigned int groupid, const FIELD_VALUE_MAP &mValues, bool modDB/* = true*/)
{
	bool ret = false;

	if (modDB)
	{
		ret = CDBSink().updGroupMemberInfo(groupid, userid, mValues);
	}

	if (ret)
	{
		ret = insert(userid, groupid, mValues);
	}

	return ret;
}
