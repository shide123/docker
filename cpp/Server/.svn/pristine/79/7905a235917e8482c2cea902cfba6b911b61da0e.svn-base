#ifndef __ROOMCLIENTMGR_H__
#define __ROOMCLIENTMGR_H__

#include <set>
#include <map>

typedef std::set<unsigned int> CONNECT_SET;
typedef std::map<unsigned int, CONNECT_SET > USER_CONNECT_MAP;

class CRoomClientMgr
{
public:
	CRoomClientMgr();

	~CRoomClientMgr();

	void add_user_connid(unsigned int roomid, unsigned int userid, unsigned int connectid);

	void del_user_connid(unsigned int roomid, unsigned int userid, unsigned int connectid);

	void get_one_user_connids(unsigned int roomid, unsigned int userid, CONNECT_SET & connectid_set);

	void del_one_user_connid(unsigned int roomid, unsigned int userid);

	void get_user_connids(unsigned int roomid, std::set<unsigned int> & connectid_set);

	void get_all_user_connids(std::set<unsigned int> & connectid_set);

	//boost::mutex room_mutex_;

	//key:roomid, value: user connect id map
	std::map<unsigned int, USER_CONNECT_MAP > m_room_all_connect;
};

#endif //__ROOMCLIENTMGR_H__
