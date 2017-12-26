#include "CUserGroupMgr.h"
#include "CLogThread.h"
#include "utils.h"
#include "errcode.h"
#include "UserObj.h"
#include "CUserBasicInfo.h"
#include "CUserGroupinfo.h"
#include "DBSink.h"

CRoomManager * CUserGroupMgr::m_roommgr = NULL;

CUserGroupMgr::CUserGroupMgr()
{
}

CUserGroupMgr::~CUserGroupMgr()
{
}

int CUserGroupMgr::init(redisMgr * pRedisMgr)
{
	if (!pRedisMgr)
	{
		LOG_PRINT(log_error, "redisMgr is null.init failed.");
		return -1;
	}

	CUserGroupinfo::init(pRedisMgr);
	CUserBasicInfo::init(pRedisMgr);
	return 0;
}

void CUserGroupMgr::setRoomManager(CRoomManager * pRoomMgr)
{
	if (pRoomMgr)
	{
		m_roommgr = pRoomMgr;
	}
}

int CUserGroupMgr::loadMember2Redis(unsigned int groupID)
{
	LOG_PRINT(log_info, "load group:%u members to redis begin.", groupID);

	std::list<stJoinGroupInfo> oGroupMemLst;
	CDBSink().getGroupMemebr_DB(groupID, oGroupMemLst);
	std::list<stJoinGroupInfo>::iterator iter = oGroupMemLst.begin();
	for (; iter != oGroupMemLst.end(); ++iter)
	{
		addUserGroupRedis(*iter);
	}

	LOG_PRINT(log_info, "load group:%u member to redis end.", groupID);
	return 0;
}

int CUserGroupMgr::procUserJoinGroup(const stJoinGroupInfo & oJoinObj)
{
	if (oJoinObj.role_type == e_VisitorRole)
	{
		LOG_PRINT(log_info, "user:%u groupid:%u roletype is vistor,do not handle it.", oJoinObj.userID, oJoinObj.groupID);
		return 0;
	}

	if (CUserGroupinfo::find(oJoinObj.userID, oJoinObj.groupID))
	{
		LOG_PRINT(log_info, "userid:%u has joined this groupid:%u before, so do not need to handle it.", oJoinObj.userID, oJoinObj.groupID);
		return 0;
	}

	if (!CDBSink().addUserGroup_DB(oJoinObj))
	{
		LOG_PRINT(log_error, "DB add user in group failed.userid:%u,groupid:%u.", oJoinObj.userID, oJoinObj.groupID);
		return -1;
	}

	return addUserGroupRedis(oJoinObj);
}

int CUserGroupMgr::procMemberQuitGroup(unsigned int userid, unsigned int groupid)
{
	LOG_PRINT(log_info, "[member quit group]userid:%u,groupid:%u.", userid, groupid);
	if (!CDBSink().delUserGroup_DB(userid, groupid))
	{
		LOG_PRINT(log_error, "DB del user in group failed.userid:%u,groupid:%u.", userid, groupid);
		return -1;
	}

	if (!CUserGroupinfo::find(userid, groupid))
	{
		LOG_PRINT(log_warning, "[member quit group]userid:%u is not in groupid:%u.not need to handle redis data.", userid, groupid);
		return 0;
	}

	return delUserGroupRedis(userid, groupid);
}

int CUserGroupMgr::addUserGroupRedis(const stJoinGroupInfo & oJoinObj)
{
	LOG_PRINT(log_info, "addUserGroupRedis.userid:%u,groupid:%u,roletype:%d.", oJoinObj.userID, oJoinObj.groupID, (int)oJoinObj.role_type);
	if (CUserGroupinfo::find(oJoinObj.userID, oJoinObj.groupID))
	{
		LOG_PRINT(log_warning, "userid:%u has joined this groupid:%u before, do not need to handle it.", oJoinObj.userID, oJoinObj.groupID);
		return 0;
	}
	else
	{
		LOG_PRINT(log_info, "userid:%u join this groupid:%u at the first time.", oJoinObj.userID, oJoinObj.groupID);

		IF_METHOD_FALSE_RETURN_EX(CUserGroupinfo::loadUserGroupInfo2Redis(oJoinObj.userID, oJoinObj.groupID), -1, \
			"userid:%u add info to groupid:%u failed in redis.", oJoinObj.userID, oJoinObj.groupID);

		freshMemberNum(oJoinObj.groupID, oJoinObj.userID, true);
		return 1;
	}
}

int CUserGroupMgr::delUserGroupRedis(unsigned int userid, unsigned int groupid)
{
	LOG_PRINT(log_info, "delUserGroupRedis.userid:%u,groupid:%u.", userid, groupid);
	if (CUserGroupinfo::find(userid, groupid))
	{
		IF_METHOD_FALSE_RETURN_EX(CUserGroupinfo::erase(userid, groupid), -1, "del user redis data fail.userid:%u,groupid:%u.", userid, groupid);

		freshMemberNum(groupid, userid, false);		
	}

	return 0;
}

int CUserGroupMgr::modUserRoleType(unsigned int userid, unsigned int groupid, unsigned int roletype)
{
	if (!userid || !groupid)
	{
		LOG_PRINT(log_error, "modUserRoleType input error,userid:%u,groupid:%u.", userid, groupid);
		return -1;
	}

	IF_METHOD_FALSE_RETURN_EX(CDBSink().updateUserRoletype_DB(userid, groupid, roletype), -1, "update user:%u groupid:%u roletype:%u failed in DB.", userid, groupid, roletype);

	if (CUserGroupinfo::find(userid, groupid))
	{
		unsigned int oldRoleType = e_VisitorRole;
		IF_METHOD_NOTZERO_RETURN(CUserGroupinfo::getUserRoleType(userid, groupid, oldRoleType), -1);
		if (oldRoleType == roletype)
		{
			LOG_PRINT(log_info, "userid:%u groupid:%u has same roletype:%u not need to handle it.", userid, groupid, roletype);
			return 0;
		}
		else if (roletype == e_VisitorRole)
		{
			LOG_PRINT(log_info, "userid:%u degrade from member to visitor in groupid:%u,old roletype:%u.", userid, groupid, oldRoleType);
			delUserGroupRedis(userid, groupid);
			return 1;
		}
		else
		{
			LOG_PRINT(log_info, "userid:%u change roletype in groupid:%u,old roletype:%u,new roletype:%u.", userid, groupid, oldRoleType, roletype);
			IF_METHOD_FALSE_RETURN_EX(CUserGroupinfo::setUserRoleType(userid, groupid, roletype), -1, "fresh userid:%u groupid:%u role type:%u to redis failed.", userid, groupid, roletype);
			return 1;
		}
	}
	else
	{
		if (roletype != e_VisitorRole)
		{
			LOG_PRINT(log_info, "userid:%u upgrade from visitor to member in groupid:%u,new roletype:%u.", userid, groupid, roletype);
			stJoinGroupInfo oJoinObj = {0};
			oJoinObj.userID = userid;
			oJoinObj.groupID = groupid;
			oJoinObj.role_type = roletype;
			return CUserGroupMgr::addUserGroupRedis(oJoinObj);
		}
		else
		{
			LOG_PRINT(log_info, "userid:%u is groupid:%u visitor and change to visitor roletype.so not need to handle it.", userid, groupid);
			return 0;
		}
	}
}

void CUserGroupMgr::freshMemberNum(unsigned int groupid, unsigned int userid, bool bAdd)
{
	if (!groupid || !userid || !m_roommgr)
	{
		LOG_PRINT(log_error, "not need to fresh redis group number.userid:%u,groupid:%u.", userid, groupid);
		return;
	}

	m_roommgr->updateGroupUserCount(groupid, userid, bAdd);
}
