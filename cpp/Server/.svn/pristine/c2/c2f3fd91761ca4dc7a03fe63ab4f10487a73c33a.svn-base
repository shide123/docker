#include "RoomClientMgr.h"
#include "CLogThread.h"

CRoomClientMgr::CRoomClientMgr()
{
	m_room_all_connect.clear();
}

CRoomClientMgr::~CRoomClientMgr()
{
	m_room_all_connect.clear();
}

void CRoomClientMgr::add_user_connid(unsigned int roomid, unsigned int userid, unsigned int connectid)
{
	if (!roomid || !connectid)
	{
		return;
	}

	//boost::mutex::scoped_lock lock(room_mutex_);

	std::map<unsigned int, USER_CONNECT_MAP >::iterator iter_room = m_room_all_connect.find(roomid);
	if (m_room_all_connect.end() == iter_room)
	{
		CONNECT_SET connects;
		connects.insert(connectid);
		m_room_all_connect[roomid][userid] = connects;
	}
	else
	{
		iter_room->second[userid].insert(connectid);
	}

	LOG_PRINT(log_debug, "add_user_connid:roomid:%u,userid:%u,client connid:%u.", roomid, userid, connectid);
}

void CRoomClientMgr::del_user_connid(unsigned int roomid, unsigned int userid, unsigned int connectid)
{
	if (!roomid || !connectid)
	{
		return;
	}

	//boost::mutex::scoped_lock lock(room_mutex_);

	std::map<unsigned int, USER_CONNECT_MAP >::iterator iter_room = m_room_all_connect.find(roomid);
	if (m_room_all_connect.end() != iter_room)
	{
		USER_CONNECT_MAP::iterator iter_user = iter_room->second.find(userid);
		if (iter_room->second.end() != iter_user)
		{
			iter_user->second.erase(connectid);
			LOG_PRINT(log_debug, "del_user_connid:roomid:%u,userid:%u,client connid:%u.", roomid, userid, connectid);
			if (iter_user->second.empty())
			{
				iter_room->second.erase(userid);
			}
		}

		if (iter_room->second.empty())
		{
			m_room_all_connect.erase(roomid);
		}
	}
}

void CRoomClientMgr::get_one_user_connids(unsigned int roomid, unsigned int userid, CONNECT_SET & connectid_set)
{
	if (!roomid || !userid)
	{
		return;
	}

	std::map<unsigned int, USER_CONNECT_MAP >::iterator iter_room = m_room_all_connect.find(roomid);
	if (m_room_all_connect.end() != iter_room)
	{
		USER_CONNECT_MAP::iterator iter_user = iter_room->second.find(userid);
		if (iter_room->second.end() != iter_user)
		{
			connectid_set.insert(iter_user->second.begin(), iter_user->second.end());
		}
	}
}

void CRoomClientMgr::del_one_user_connid(unsigned int roomid, unsigned int userid)
{
	if (!roomid || !userid)
	{
		return;
	}

	std::map<unsigned int, USER_CONNECT_MAP >::iterator iter_room = m_room_all_connect.find(roomid);
	if (m_room_all_connect.end() != iter_room)
	{
		iter_room->second.erase(userid);

		if (iter_room->second.empty())
		{
			m_room_all_connect.erase(roomid);
		}
	}
}

void CRoomClientMgr::get_user_connids(unsigned int roomid, std::set<unsigned int> & connectid_set)
{
	connectid_set.clear();
	if (!roomid)
	{
		return;
	}

	//boost::mutex::scoped_lock lock(room_mutex_);
	std::map<unsigned int, USER_CONNECT_MAP >::iterator iter_room = m_room_all_connect.find(roomid);
	if (m_room_all_connect.end() != iter_room)
	{
		USER_CONNECT_MAP & user_map = iter_room->second;
		USER_CONNECT_MAP::iterator iter_user = user_map.begin();
		for (; iter_user != user_map.end(); ++iter_user)
		{
			connectid_set.insert(iter_user->second.begin(), iter_user->second.end());
		}
	}
}

void CRoomClientMgr::get_all_user_connids(std::set<unsigned int> & connectid_set)
{
	connectid_set.clear();

	//boost::mutex::scoped_lock lock(room_mutex_);
	std::map<unsigned int, USER_CONNECT_MAP >::iterator iter_room = m_room_all_connect.begin();
	for (; iter_room != m_room_all_connect.end(); ++iter_room)
	{
		USER_CONNECT_MAP & user_map = iter_room->second;
		USER_CONNECT_MAP::iterator iter_user = user_map.begin();
		for (; iter_user != user_map.end(); ++iter_user)
		{
			connectid_set.insert(iter_user->second.begin(), iter_user->second.end());
		}
	}
}



