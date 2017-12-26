/*
 * redisdataHelper.cpp
 *
 *  Created on: Aug 15, 2016
 *      Author: shuisheng
 */
#include "redisdataHelper.h"


void JsonToString(Json::Value &jval, string &value)
{
	if (jval.isInt()) {
		int val = jval.asInt();
		bitTostring(val, value);
	}
	else if (jval.isUInt()) {
		uint val = jval.asUInt();
		bitTostring(val, value);
	}
	else if (jval.isInt64()) {
		int64 val = jval.asInt64();
		bitTostring(val, value);
	}
	else if (jval.isUInt64()) {
		uint64 val = jval.asInt64();
		bitTostring(val, value);
	}
	else if (jval.isBool() || jval.isString()) {
		value = jval.asString();
	}
	else if (jval.isArray()) {
		value = Json::FastWriter().write(jval);
	}
}

void JsonToVec(Json::Value &jval, vector<string> &names, vector<string> &values)
{
	Json::Value::Members m = jval.getMemberNames();
	names.clear();
	names = m;
	string str;
	for (size_t i=0; i<m.size(); i++) {
		JsonToString(jval[m[i]], str);
		values.push_back(str);
	}
}

void JsonToMap(Json::Value &jval, map<string, string> &mValues)
{
	Json::Value::Members m = jval.getMemberNames();
	string str;
	for (size_t i=0; i<m.size(); i++) {
		JsonToString(jval[m[i]], str);
		mValues[m[i]] = str;
	}
}
/*
void redisdataHelper::setDataHeader(uint8_t action, uint8_t msgid, uint16_t fromsvrid, uint32_t nflags, const char* key, uint8_t datatype, uint8_t db)
{
	head["datatype"] 	= datatype;
	head["action"] 		= action;
	head["msgid"] 		= msgid;
	head["flags"] 		= nflags;
	head["fromsvrid"] 	= fromsvrid;
	head["redisdb"] 	= db;

	if (key && *key != '\0')
		head["key"] = key;
}

int redisdataHelper::writeRedisMsg(redisOpt *pRedisMSG, vector<string> &vecfield, vector<string> &vecvalue)
{
	if (0 == vecfield.size() || vecfield.size() != vecvalue.size() )
		return -1;

	body.clear();
	for (size_t i=0; i<vecfield.size(); i++) {
		body[vecfield[i]] = vecvalue[i];
	}
	root["head"] = head;
	root["body"] = body;

	Json::FastWriter writer;
	string out = writer.write(root);
	LOG_PRINT(log_debug, "out: %s", out.c_str());

	int ret = pRedisMSG->redis_writeMsg(out.c_str(), out.size());
	if (ret < 0)
		LOG_PRINT(log_error, "failed to write redis msg: %s", out.c_str());
	return ret;
}

int redisdataHelper::writeRedisMsg(redisOpt *pRedisMSG, map<string, string> &map_field_value)
{
	if (0 == map_field_value.size())
		return -1;

	vector<string> vecfield, vecvalue;
	map<string, string>::iterator it = map_field_value.begin();
	for (; it != map_field_value.end(); it++) {
		vecfield.push_back(it->first);
		vecvalue.push_back(it->second);
	}

	return writeRedisMsg(pRedisMSG, vecfield, vecvalue);
}

*/
