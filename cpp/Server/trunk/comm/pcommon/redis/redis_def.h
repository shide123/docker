/*
 * redis_def.h
 *
 *  Created on: 2016年2月2日
 *      Author: shuisheng
 */

#ifndef REDIS_DEF_H_
#define REDIS_DEF_H_

#include "utils.h"

#define KEY_SET_GATE_SVR "gate"
#define KEY_SET_ROOMIDS	"roomids"
#define KEY_HASH_ROOM_INFO "room"
#define KEY_SET_ROOM_USERIDS "room_userids"
#define KEY_HASH_ROOM_USER_SETTING "room_user_setting"
#define KEY_HASH_ROOM_GROUP_CHAT_MSG "room_chat_msg"
#define KEY_HASH_ROOM_MGR "roommgr_info"
#define KEY_HASH_USER_INFO "user"
#define KEY_HASH_GLOBAL_INFO "global"
#define KEY_HASH_GROUPMSG_INFO "group_msg"
#define KEY_HASH_KEYWORD_INFO "keyword"
#define KEY_HASH_GROUPID_NUM "groupid_num"
#define KEY_SET_GROUPID "groupid"
#define KEY_USER_GROUPS "user_groups"
#define KEY_HASH_USER_ROOM_ID "user_room"
#define KEY_ROOM_MSG "room_msg_tt"
#define KEY_HASH_TEAMTOPMONEY "team_top_money"
#define KEY_SERVER_SET "99cjserverset"
#define KEY_VISTOR_INFO "vistor"
#define KEY_REDPACKET "redpacket"
#define KEY_REDPACKET_TAKEUSERS "redpacket_take_users"
#define KEY_REDPACKET_SENDUSER "redpacket_send_userid"
#define KEY_REDPAKCET_IDS "redpacket_ids"
#define KEY_SET_INGROUP "ingroup"

#define KEY_HASH_COMMENT_AUDIT_SWITCH   "comment_audit_status"

#define CHANNEL_ROOM_MSG  "room_msg"  //消息结构 (svrid roommsgid userid roomid [...])
#define CHANNEL_GATE_DISCONN_MSG  "room_gate_msg"  //消息结构(svrid gateid)
#define CHANNEL_REDIS_SYNC  "redis_sync"
#define REDIS_DELIMITER  "{卍卐△▽}"
//#define REDIS_DELIMITER  ","
#define BLANK  "#"

enum {
    DB_ROOM			= 0,
    DB_USER_MGR		= 1,
    DB_USERGROUP_DB	= 2,
	DB_USERBASIC_DB	= 3,
	DB_VISITORID	= 4,
	DB_VIPMGR		= 5,
	DB_REDPACKET	= 6,
	DB_CHATRELA		= 7
};

enum {
	REDIS_UNKNOWN_CMD = -1,
	REDIS_KEY_CMD = 0,
	REDIS_STRING_CMD = 1,
	REDIS_HASH_CMD = 2,
	REDIS_SET_CMD = 3,
	REDIS_LIST_CMD = 4,
};

enum {
	REDIS_NOACTION,
	REDIS_ADD,
	REDIS_REM,
	REDIS_UPDATE,
	REDIS_INCRBY,
	REDIS_DECRBY,
};

enum {
	WRITE_LOCAL_REDIS 	= 0x00000001,
	WRITE_REMOTE_REDIS 	= 0x00000002,
	PUBLISH_LOCAL		= 0x00000004,
	PUBLISH_REMOTE		= 0x00000008,
	SYNC_REDIS_MSG		= 0x00000010,
};

template<typename T>
inline void findvalue(T &value, const char* field, const string &fields, const string &values)
{
	vector<string> vfields = strToVec(fields, ' ');
	vector<string> vValues = strToVec(values, REDIS_DELIMITER);
	int index = -1;
	for (size_t i=0; i<vfields.size(); i++) {
		if (vfields[i] == field) {	index = i;	break;	}
	}
	if (index != -1)
		stringTobit(value, vValues[index]);
}

template<typename T>
inline void findvalue(T &value, const char* field, const string &fields, const vector<string> &values)
{
	vector<string> vfields = strToVec(fields, ' ');
	vector<string> vValues = values;
	int index = -1;
	for (size_t i=0; i<vfields.size(); i++) {
		if (vfields[i] == field) {	index = i;	break;	}
	}
	if (index != -1)
		stringTobit(value, vValues[index]);
}

template<typename T>
inline void findvalue(T &value, const char* field, const vector<string> &fields, const vector<string> &values)
{
	vector<string> vValues = values;
	int index = -1;
	for (size_t i=0; i<fields.size(); i++) {
		if (fields[i] == field) {	index = i;	break;	}
	}
	if (index != -1)
		stringTobit(value, vValues[index]);
}

template<typename T>
inline void findvalue(T &value, const char* field, const map<string, string> & oFieldValueMap)
{
	map<string, string>::const_iterator iter_map = oFieldValueMap.find(string(field));
	if (iter_map != oFieldValueMap.end())
	{
		string m_value = iter_map->second;
		stringTobit(value, m_value);
	}	
}

template<typename T>
inline void setvalue(T &value, const char* field,  map<string, string> & oFieldValueMap)
{
	oFieldValueMap[field] = bitTostring(value);
}

#endif /* REDIS_DEF_H_ */
