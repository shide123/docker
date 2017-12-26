/*
 * CClientItem.cpp
 *
 *  Created on: 2016.8.20
 *      Author: laimingming
 */

#include "CClientItem.h"
#include <time.h>
#include <string.h>

CClientItem::CClientItem()
:m_last_login_time(0),
 m_last_active_time(time(NULL)),
 m_mobile(0),
 m_blogon(false),
 m_exitroom(true),
 m_micRunUserid(0),
 m_micToUserid(0),
 m_micindex(-1),
 m_micstate(0),
 m_micGroupID(0),
 m_platformType(0),
 m_nversion(0),
 m_timeid(0),
 m_invite_mic_runid(0),
 m_invite_mic_toid(0),
 m_invite_mic_sessionid(0),
 m_valid(true),
 m_gender(0)
{
	m_problem_svr_conn_map.clear();
	m_groupRoleMap.clear();
	memset(m_cloginid, 0, sizeof(m_cloginid));
	memset(sAppVersion, 0, sizeof(sAppVersion));
}

CClientItem::~CClientItem()
{
	m_blogon = false;
	m_exitroom = true;
	m_micRunUserid = 0;
	m_micToUserid = 0;
	m_micindex = -1;
	m_micstate = 0;
	m_micGroupID = 0;
	m_problem_svr_conn_map.clear();
	m_groupRoleMap.clear();
	m_invite_mic_runid = 0;
	m_invite_mic_toid = 0;
	m_invite_mic_sessionid = 0;
	m_gender = 0;
}

void CClientItem::set_pushmsg_pass(unsigned int msgid, bool bcanpass)
{
	if (bcanpass)
	{
		m_set_pushmsg_pass.insert(msgid);
	}
	else
	{
		m_set_pushmsg_pass.erase(msgid);
	}
}

bool CClientItem::get_pushmsg_pass(unsigned int msgid)
{
	return m_set_pushmsg_pass.find(msgid) != m_set_pushmsg_pass.end();
}

void CClientItem::insert_problem_svr_conn(unsigned int svr_type, unsigned int svr_connID)
{
	//boost::mutex::scoped_lock lock(svr_problem_mutex);

	std::map<unsigned int, unsigned int>::iterator iter = m_problem_svr_conn_map.find(svr_type);
	if (m_problem_svr_conn_map.end() == iter)
	{
		m_problem_svr_conn_map.insert(std::make_pair(svr_type, svr_connID));
	}
	else if (svr_connID != iter->second)
	{
		iter->second = svr_connID;
	}
}

unsigned int CClientItem::find_problem_svr_conn(unsigned int svr_type)
{
	//boost::mutex::scoped_lock lock(svr_problem_mutex);

	std::map<unsigned int, unsigned int>::iterator iter = m_problem_svr_conn_map.find(svr_type);
	if (m_problem_svr_conn_map.end() != iter)
	{
		return iter->second;
	}
	else
	{
		return 0;
	}
}

void CClientItem::fill_mic_state(unsigned int runUserID, unsigned int toUserID, int micIndex, int micState, unsigned int groupID)
{
	m_micRunUserid = runUserID;
	m_micToUserid = toUserID;
	m_micindex = micIndex;
	m_micstate = micState;
	m_micGroupID = groupID;
}

void CClientItem::reset_mic_state()
{
	m_micRunUserid = 0;
	m_micToUserid = 0;
	m_micindex = -1;
	m_micstate = 0;
	m_micGroupID = 0;
}

void CClientItem::fill_invite_mic_state(unsigned int runnerID, unsigned int touserID, int sessionid)
{
	m_invite_mic_runid = runnerID;
	m_invite_mic_toid = touserID;
	m_invite_mic_sessionid = sessionid;
}

void CClientItem::reset_invite_mic_state()
{
	m_invite_mic_runid = 0;
	m_invite_mic_toid = 0;
	m_invite_mic_sessionid = 0;
}

void CClientItem::setGroupRole(unsigned int groupid, unsigned int roleType)
{
	m_groupRoleMap[groupid] = roleType;
}

bool CClientItem::getGroupRole(unsigned int groupid, unsigned int & roleType)
{
	std::map<unsigned int, unsigned int >::iterator iter_map = m_groupRoleMap.find(groupid);
	if (m_groupRoleMap.end() != iter_map)
	{
		roleType = iter_map->second;
		return true;
	}
	else
	{
		return false;
	}
}

bool CClientItem::removeGroupRole(unsigned int groupid)
{
	std::map<unsigned int, unsigned int >::iterator iter_map = m_groupRoleMap.find(groupid);
	if (m_groupRoleMap.end() != iter_map)
	{
		m_groupRoleMap.erase(iter_map);
		return true;
	}
	else
	{
		return false;
	}
}

void CClientItem::getAllGroupIDs(std::set<unsigned int> & groupIDSet)
{
	groupIDSet.clear();
	std::map<unsigned int, unsigned int >::iterator iter_map = m_groupRoleMap.begin();
	for (; iter_map != m_groupRoleMap.end(); ++iter_map)
	{
		groupIDSet.insert(iter_map->first);
	}
}

