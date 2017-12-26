#ifndef __USE_GROUP_MGR_H__
#define __USE_GROUP_MGR_H__

#include "redisMgr.h"
#include "redis_map_map.h"
#include "DBTools.h"
#include "RoomManager.h"

class CUserGroupMgr
{
public:
	CUserGroupMgr();
	virtual ~CUserGroupMgr();

	static int init(redisMgr * pRedisMgr);
	static void setRoomManager(CRoomManager * pRoomMgr);
	static int loadMember2Redis(unsigned int groupID);
	static int procUserJoinGroup(const stJoinGroupInfo & oJoinObj);
	static int procMemberQuitGroup(unsigned int userid, unsigned int groupid);
	static int modUserRoleType(unsigned int userid, unsigned int groupid, unsigned int roletype);

private:
	static int addUserGroupRedis(const stJoinGroupInfo & oJoinObj);
	static int delUserGroupRedis(unsigned int userid, unsigned int groupid);
	static void freshMemberNum(unsigned int groupid, unsigned int userid, bool bAdd);

	static CRoomManager * m_roommgr;
};

#endif //__USE_GROUP_MGR_H__