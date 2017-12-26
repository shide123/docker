#include <stdlib.h>
#include <time.h>

#include "UserObj.h"
#include "yc_defines.h"
#include "redis_def.h"
#include "CUserGroupinfo.h"
#include "CUserBasicInfo.h"


redis_map_map CUserObj::m_onlineInfo;
redis_map_map CUserObj::m_userRoomInfo;
CUserObj::CUserObj()
{
	reset();
}

CUserObj::CUserObj(const vector<string> & fields, const vector<string> & vals)
{
	reset();

	findvalue(nuserid_, "userid" ,  fields,vals);
	findvalue(user_level, "user_level" , fields,vals);
	findvalue(age, "age" , fields, vals );
	findvalue(gender, "gender", fields, vals);
	findvalue(head_add, "head_add", fields, vals);
	findvalue(name, "name", fields, vals);
	findvalue(alias, "alias", fields, vals);
	findvalue(login_name, "login_name", fields, vals);
	findvalue(password, "password", fields, vals);
	findvalue(email, "email", fields, vals);
	findvalue(qq, "qq", fields, vals);
	findvalue(id_card, "id_card", fields, vals);
	findvalue(birthday, "birthday", fields, vals);
	findvalue(register_ip, "register_ip", fields, vals);
	findvalue(register_date, "register_date", fields, vals);
	findvalue(last_login_ip, "last_login_ip", fields, vals);
	findvalue(last_login_mac, "last_login_mac", fields,vals);
	findvalue(last_login_time, "last_login_time", fields, vals);
	findvalue(tel, "tel", fields, vals);
	findvalue(code, "code", fields, vals);
	findvalue(code_time, "code_time", fields, vals);
	findvalue(freeze, "freeze", fields, vals);
	findvalue(is_recommend, "is_recommend", fields, vals);
	findvalue(tag, "tag", fields, vals);
	findvalue(intro, "intro", fields, vals);
	findvalue(addr, "addr", fields, vals);
	findvalue(is_daka, "is_daka", fields, vals);

	findvalue(ngateid, "gateid", fields, vals);
	findvalue(pGateObj_, "gateobj", fields, vals);
	findvalue(pGateObjId_, "gateobjid", fields, vals);
	findvalue(pConnId_, "gateconnid", fields, vals);
	findvalue(inroomstate_, "inroomstate", fields, vals);
	findvalue(cometime_, "cometime", fields, vals);
//	cometime_ = time(0);
}

CUserObj::CUserObj(const std::map<std::string, std::string> & field_values)
{
	reset();

	findvalue(nuserid_, "userid" ,  field_values);
	findvalue(user_level, "user_level" , field_values);
	findvalue(age, "age" , field_values );
	findvalue(gender, "gender", field_values);
	findvalue(head_add, "head_add", field_values);
	findvalue(name, "name", field_values);
	findvalue(alias, "alias", field_values);
	findvalue(login_name, "login_name", field_values);
	findvalue(password, "password", field_values);
	findvalue(email, "email", field_values);
	findvalue(qq, "qq", field_values);
	findvalue(id_card, "id_card", field_values);
	findvalue(birthday, "birthday", field_values);
	findvalue(register_ip, "register_ip", field_values);
	findvalue(register_date, "register_date", field_values);
	findvalue(last_login_ip, "last_login_ip", field_values);
	findvalue(last_login_mac, "last_login_mac", field_values);
	findvalue(last_login_time, "last_login_time", field_values);
	findvalue(tel, "tel", field_values);
	findvalue(code, "code", field_values);
	findvalue(code_time, "code_time", field_values);
	findvalue(freeze, "freeze", field_values);
	findvalue(is_recommend, "is_recommend", field_values);
	findvalue(tag, "tag", field_values);
	findvalue(intro, "intro", field_values);
	findvalue(addr, "addr", field_values);
	findvalue(is_daka, "is_daka", field_values);

	findvalue(ngateid, "gateid", field_values);
	findvalue(pGateObj_, "gateobj", field_values);
	findvalue(pGateObjId_, "gateobjid", field_values);
	findvalue(pConnId_, "gateconnid", field_values);
	findvalue(inroomstate_, "inroomstate", field_values);
	findvalue(cometime_, "cometime", field_values);
//	cometime_ = time(0);
}

CUserObj::~CUserObj()
{
}

void CUserObj::reset()
{
	nuserid_ = 0;
	user_level = 0;
	age = 0;
	gender = 0;
	head_add = "";
	name = "";
	alias = "";
	login_name = "";
	password = "";
	email = "";
	qq = "";
	id_card = "";
	id_card = "";
	birthday = 0;
	register_ip = "";
	register_date = 0;
	last_login_ip = "";
	last_login_mac = "";
	last_login_time = "";
	tel = "";
	code = "";
	code_time = "";
	freeze = 0;
	is_recommend = 0;
	tag = "";
	intro = "";
	addr = "";
	is_daka = 0;
}

string CUserObj::getRedisKey()
{
	char ckey[32];
	sprintf(ckey, "user:%u", nuserid_);
	return string(ckey);
}

string CUserObj::getRedisFields()
{
	return CFieldName::USER_FIELDS;
}

void CUserObj::getRedisValues(Json::Value &jOut, const string &fields)
{
	Json::Value jval;
	jval["userid"] = nuserid_;
	jval["user_level"] = user_level;
	jval["age"] = age;
	jval["gender"] = gender;
	jval["head_add"] = head_add;
	jval["name"] = name;
	jval["alias"] = alias;
	jval["login_name"] = login_name;
	jval["password"] = password;
	jval["email"] = email;
	jval["qq"] = qq;
	jval["id_card"] = id_card;
	jval["birthday"] = birthday;
	jval["register_ip"] = register_ip;
	jval["register_date"] = register_date;
	jval["last_login_ip"] = last_login_ip;
	jval["last_login_mac"] = last_login_mac;
	jval["last_login_time"] = last_login_time;
	jval["tel"] = tel;
	jval["code"] = code;
	jval["code_time"] = code_time;
	jval["freeze"] = freeze;
	jval["is_recommend"] = is_recommend;
	jval["tag"] = tag;
	jval["intro"] = intro;
	jval["addr"] = addr;
	jval["is_daka"] = is_daka;

	jval["gateid"] = ngateid;
	jval["gateobj"] = pGateObj_;
	jval["gateobjid"] = pGateObjId_;
	jval["gateconnid"] = pConnId_;
	jval["inroomstate"] = inroomstate_;
	jval["cometime"] = cometime_;

	std::stringstream os;
	os << fields;
	std::string field;
	while(!os.eof())
	{
		field.clear();
		os >> field;
		if (field.empty())
			continue;

		if (jval.isMember(field))
		{
			jOut[field] = jval[field];
		}
	}
}

void CUserObj::getRedisValuesMap(std::map<std::string, std::string> &mValues, const string fields)
{
	if (fields.find("userid") != string::npos)
		mValues["userid"] = bitTostring(nuserid_);
	if (fields.find("user_level") != string::npos)
		mValues["user_level"] = bitTostring(user_level);
	if (fields.find("age") != string::npos)
		mValues["age"] = bitTostring(age);
	if (fields.find("gender") != string::npos)
		mValues["gender"] = bitTostring(gender);
	if (fields.find("head_add") != string::npos)
		mValues["head_add"] = bitTostring(head_add);
	if (fields.find("name") != string::npos)
		mValues["name"] = bitTostring(name);
	if (fields.find("alias") != string::npos)
		mValues["alias"] = bitTostring(alias);
	if (fields.find("login_name") != string::npos)
		mValues["login_name"] = bitTostring(login_name);
	if (fields.find("password") != string::npos)
		mValues["password"] = bitTostring(password);
	if (fields.find("email") != string::npos)
		mValues["email"] = bitTostring(email);
	if (fields.find("qq") != string::npos)
		mValues["qq"] = bitTostring(qq);
	if (fields.find("id_card") != string::npos)
		mValues["id_card"] = bitTostring(id_card);
	if (fields.find("birthday") != string::npos)
		mValues["birthday"] = bitTostring(birthday);
	if (fields.find("register_ip") != string::npos)
		mValues["register_ip"] = bitTostring(register_ip);
	if (fields.find("register_date") != string::npos)
		mValues["register_date"] = bitTostring(register_date);
	if (fields.find("last_login_ip") != string::npos)
		mValues["last_login_ip"] = bitTostring(last_login_ip);
	if (fields.find("last_login_mac") != string::npos)
		mValues["last_login_mac"] = bitTostring(last_login_mac);
	if (fields.find("last_login_time") != string::npos)
		mValues["last_login_time"] = bitTostring(last_login_time);
	if (fields.find("tel") != string::npos)
		mValues["tel"] = bitTostring(tel);
	if (fields.find("code") != string::npos)
		mValues["code"] = bitTostring(code);
	if (fields.find("code_time") != string::npos)
		mValues["code_time"] = bitTostring(code_time);
	if (fields.find("freeze") != string::npos)
		mValues["freeze"] = bitTostring(freeze);
	if (fields.find("is_recommend") != string::npos)
		mValues["is_recommend"] = bitTostring(is_recommend);
	if (fields.find("tag") != string::npos)
		mValues["tag"] = bitTostring(tag);
	if (fields.find("intro") != string::npos)
		mValues["intro"] = bitTostring(intro);
	if (fields.find("addr") != string::npos)
		mValues["addr"] = bitTostring(addr);
	if (fields.find("is_daka") != string::npos)
		mValues["is_daka"] = bitTostring(is_daka);

	if (fields.find("gateid") != string::npos)
		mValues["gateid"] = bitTostring(ngateid);
	if (fields.find("gateobj") != string::npos)
		mValues["gateobj"] = bitTostring(pGateObj_);
	if (fields.find("gateobjid") != string::npos)
		mValues["gateobjid"] = bitTostring(pGateObjId_);
	if (fields.find("gateconnid") != string::npos)
		mValues["gateconnid"] = bitTostring(pConnId_);
	if (fields.find("inroomstate") != string::npos)
		mValues["inroomstate"] = bitTostring(inroomstate_);
	if (fields.find("cometime") != string::npos)
		mValues["cometime"] = bitTostring(cometime_);
//	std::stringstream os;
//	os << fields;
//	std::string field;
//	while(!os.eof())
//	{
//		field.clear();
//		os >> field;
//		if (field.empty())
//			continue;
//
//		if (mInfos.find(field) != mInfos.end())
//		{
//			mValues[field] = mInfos[field];
//		}
//	}
}       

void CUserObj::setUserInfo2Redis(redisOpt *opt, std::string fields/* = ""*/)
{
	if (opt)
	{
		if (fields.empty())
		{
			fields = getRedisFields();
		}
	
		std::map<std::string, std::string> mValues;
		getRedisValuesMap(mValues, fields);
		if (opt->redis_hmset(getRedisKey().c_str(), mValues) < 0)
		{
			LOG_PRINT(log_info, "setUserInfo2Redis [userid=%u] failed to write redis msg..", nuserid_);
		}
	}
}

bool CUserObj::updateOnlineInfo()
{
	if (!m_onlineInfo.exist(nuserid_))
	{
		LOG_PRINT(log_warning, "user[%u] is already offline", nuserid_);
		return false;
	}
	std::map<std::string, std::string> mValues;
	getRedisValuesMap(mValues, "private_userid optype private_time");
	if (!m_onlineInfo.insert(nuserid_, mValues))
	{
		return false;
	}

	return true;
}

bool CUserObj::isNormalUser()
{
	return (type_ == USERTYPE_NORMALUSER);
}

bool CUserObj::isZhuanboUser()
{
	return (type_ == USERTYPE_NPC || type_ == USERTYPE_MICNPC);
}

UserObject_Ref CUserObj::findUserOnline(uint32 userid)
{
	UserObject_Ref pUserRef;
	if (!m_onlineInfo.exist(userid))
		return pUserRef;

	map<string, string> vecValues;
	m_userRoomInfo.getall(userid, vecValues);

	pUserRef.reset(new CUserObj(vecValues));
	pUserRef->nuserid_ = userid;
	pUserRef->loadStaticInfo();
	return pUserRef;
}


void CUserObj::loadStaticInfo()
{
	if (nuserid_)
	{
		CUserBasicInfo::getUserInfo(nuserid_, *this);
		if (ngroupid)
		{
			nuserlevel_ = CUserGroupinfo::getUserRoleType(nuserid_, ngroupid);
		}

		std::map<std::string, std::string> mValues;
		m_onlineInfo.getall(nuserid_, mValues);
		findvalue(nprivate_time, "private_time" , mValues);
		findvalue(nprivate_userid, "private_userid" , mValues);
		findvalue(noptype, "optype" , mValues);
	}
}
