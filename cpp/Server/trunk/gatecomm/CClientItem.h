/*
 * CClientItem.h
 *
 *  Created on: 2016.8.20
 *      Author: laimingming
 */

#ifndef __CCLIENTITEM_H__
#define __CCLIENTITEM_H__

#include <map>
#include <set>
#include <string>
#include "yc_datatypes.h"

struct CClientItem
{
public:

	CClientItem();

	~CClientItem();

	//last login time
	unsigned int m_last_login_time;

	//last active time
	unsigned int m_last_active_time; // TODO: time_t or unsigned unsigned int ??

	//client login devtype:0-PC,1-Android,2-IOS
	unsigned char m_mobile;

	//mark this client session has logon or not.
	bool m_blogon;

	//mark this client session has exit room or not.
	bool m_exitroom;

	//roomsvr mic state need this
	unsigned int m_micRunUserid;

	//roomsvr mic state need this.
	unsigned int m_micToUserid;

	//roomsvr mic state need this.
	unsigned int m_micGroupID;

	//roomsvr mic state need this.
	int m_micindex;

	//roomsvr mic state need this.
	int m_micstate;

	//invite on mic need this
	unsigned int m_invite_mic_runid;

	//invite on mic need this
	unsigned int m_invite_mic_toid;

	//invite on mic need this
	int m_invite_mic_sessionid;

	//uuid
	std::string m_uuid;

	//platform type:1-QQ,2-weibo,3-weixin
	unsigned char m_platformType;

	//loginid
	char m_cloginid[LEN32];

	//client version
	unsigned int m_nversion;
	char sAppVersion[LEN32];

	//timeid
	int m_timeid;

	//valid session
	bool m_valid;

	//user gender
	unsigned int m_gender;

	//groupid,roletype map
	std::map<unsigned int, unsigned int > m_groupRoleMap;

	typedef std::set<unsigned int> SET_MSG_PUSH;

	void set_pushmsg_pass(unsigned int msgid, bool bcanpass);
	bool get_pushmsg_pass(unsigned int msgid);
	void insert_problem_svr_conn(unsigned int svr_type, unsigned int svr_connID);

	unsigned int find_problem_svr_conn(unsigned int svr_type);

	void fill_mic_state(unsigned int runUserID, unsigned int toUserID, int micIndex, int micState, unsigned int groupID);
	void reset_mic_state();
	void fill_invite_mic_state(unsigned int runnerID, unsigned int touserID, int sessionid);
	void reset_invite_mic_state();

	void setGroupRole(unsigned int groupid, unsigned int roleType);
	bool getGroupRole(unsigned int groupid, unsigned int & roleType);
	bool removeGroupRole(unsigned int groupid);
	void getAllGroupIDs(std::set<unsigned int> & groupIDSet);

protected:
	SET_MSG_PUSH m_set_pushmsg_pass;

private:
	//key:svr_type,value:server connID which has problem
	std::map<unsigned int, unsigned int> m_problem_svr_conn_map;
};

#endif /* __CCLIENTITEM_H__ */
