/*
 * redisdataHelper.h
 *
 *  Created on: Aug 15, 2016
 *      Author: shuisheng
 */

#ifndef REDIS_REDISDATAHELPER_H_
#define REDIS_REDISDATAHELPER_H_

#include "json/json.h"
#include "utils.h"
#include "redisOpt.h"

template <typename T>
void redisMsg_ObjectUpdate(string &key, string &fields, Json::Value &body, T* p)
{
	if (NULL == p)
		return;

	key = p->getRedisKey();
	if (fields.empty())
		fields = p->getRedisFields();

	body.clear();
	p->getRedisValues(body, fields);
}

void JsonToString(Json::Value &jval, string &value);
void JsonToVec(Json::Value &jval, vector<string> &names, vector<string> &values);
/*
class redisdataHelper
{
public:
	redisdataHelper() {};
	virtual ~redisdataHelper() {};

public:
	void setDataHeader(uint8_t action, uint8_t msgid, uint16_t fromsvrid, uint32_t nflags, const char* key = NULL,
			uint8_t datatype = REDIS_HASH_CMD, uint8_t db = 0);

	int writeRedisMsg(redisOpt *pRedisMSG, vector<string> &vecfield, vector<string> &vecvalue);
	int writeRedisMsg(redisOpt *pRedisMSG, map<string, string> &map_field_value);

	template<typename T>
	int writeRedisMsg(redisOpt *pRedisMSG, T* p, const char *pfield = NULL)
	{
		if (!p) {
			LOG_PRINT(log_error, "pUserObj must not be NULL");
			return -1;
		}

		body.clear();

		string key, fields, values;
		if (pfield && *pfield != '\0')
			fields = pfield;

		redisMsg_ObjectUpdate(key, fields, body, p);
		head["key"] = key;

		root["head"] = head;
		root["body"] = body;

		Json::FastWriter writer;
		string out = writer.write(root);
		LOG_PRINT(log_info, "out: %s", out.c_str());

		int ret = pRedisMSG->redis_writeMsg(out.c_str(), out.size());
		if (ret < 0)
			LOG_PRINT(log_error, "[room:%u,user:%u]failed to write redis msg for key:%s", key.c_str());
		return ret;
	}
protected:
	Json::Value root;
	Json::Value head;
	Json::Value body;
};
*/

#endif /* REDIS_REDISDATAHELPER_H_ */
