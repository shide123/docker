/*
 *  Description: This is for manage user info
 */

#ifndef CUSERMGR_H_
#define CUSERMGR_H_

#include "CUserinfo.h"
#include "CConnInfo.h"
#include <boost/thread/mutex.hpp>
#include <map>
#include <set>
#include <vector>
#include "yc_datatypes.h"
#include "utils.h"
#include "redis_map_map.h"
#include "ClientConnectionMgr.h"
#include "UserMgrSvr.pb.h"
#include "comm_struct.h"

typedef std::set<CUserinfo> USER_SET;
typedef std::map<CConnInfo, USER_SET> CONN_USER_MAP;

typedef enum
{
	eFail		= 0,
    eDefault	= 1,
    eFirstLogin = 2,          //说明该用户信息是初次加入登陆
    eRelogin	= 3           //说明该用户信息是再次加入登陆,且原有的connObj跟新的不同
}LOGIN_RESULT;

class UsermgrEvent;
class CUserMgr
{
public:

    CUserMgr();

    ~CUserMgr();

	int init(redisMgr * pRedisMgr);

	void resetRedisDB();

	LOGIN_RESULT userLogin(const CConnInfo & connObj, const CUserinfo & userObj, CConnInfo & connOldObj, CUserinfo & userOldObj);

	void findUser_termtype(unsigned int userid, byte termtype, CONN_USER_MAP & conn_map);

    void delUser(const CConnInfo & connObj, const CUserinfo & userObj);

    void delAllUser(const std::string & strIP, unsigned int iPort);

	int modUserInfo(unsigned int userid, const std::map<std::string, std::string> & oFieldValueMap, std::set<unsigned int> & groupIDSet);

	int getUserOnlineInfo(unsigned int userid, CConnInfo & connObj, CUserinfo & userObj);

private:
	bool setUserOnlineInfo(const CConnInfo & connObj, const CUserinfo & userObj);

	CONN_USER_MAP m_connUserMap;

	redis_map_map m_redis_userinfo_map;
};

#endif /* CUSERMGR_H_ */
