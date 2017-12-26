#include "CClientManager.h"
#include "CClientItem.h"
#include "timer.h"
#include "GateBase.h"

#include <boost/bind.hpp>

CClientManager::CClientManager():
m_nTimerId(0)
{
}

CClientManager::~CClientManager()
{
	Timer::getInstance()->del(m_nTimerId);
}

void CClientManager::init()
{
	m_nTimerId = Timer::getInstance()->add(10, boost::bind(&CClientManager::check_req_time, this), TIMES_UNLIMIT);
}

void CClientManager::del_client(clienthandler_ptr connection)
{
    if (!connection)
    {
        return;
    }

	//boost::mutex::scoped_lock lock(client_session_mutex_);

	CClientItem * pClientItem = (CClientItem *)connection->user_data;
	char sBuf[128] = {0};
	sprintf(sBuf, "%d_%d_%d", connection->user_id, pClientItem->m_mobile, pClientItem->m_last_login_time);
	std::string strKey = std::string(sBuf);

	std::map<std::string, unsigned int>::iterator iter_map = m_mapuser.find(strKey);
	if (iter_map != m_mapuser.end() && iter_map->second == connection->getconnid())
	{
		//in same second,client may login again and fresh map value to new connID,so need to check connID when delete.
		m_mapuser.erase(iter_map);
		LOG_PRINT(log_debug, "CClientManager::del_client()[client connid:%u,%s],user_map size:%u.", connection->getconnid(), strKey.c_str(), m_mapuser.size());
	}
	
	std::map<unsigned int, std::set<unsigned int> >::iterator iter = m_user_connid_map.find(connection->user_id);
	if (iter != m_user_connid_map.end())
	{
		iter->second.erase(connection->getconnid());
		if (iter->second.empty())
		{
			m_user_connid_map.erase(iter);
		}
	}

	session_map_.erase(connection->getconnid());
	m_connid_time_map.erase(connection->getconnid());
	CGateBase::m_RoomClientMgr.del_user_connid(connection->room_id, connection->user_id, connection->getconnid());
	CGateBase::m_SvrConnMgr.del_conn_inf(connection->getconnid());
	LOG_PRINT(log_debug, "CClientManager::del_client()[client connid:%u,%s],session_map size=[%u].", connection->getconnid(), strKey.c_str(), session_map_.size());

	//for group member need to clean all groupid.
	std::set<unsigned int> groupIDSet;
	pClientItem->getAllGroupIDs(groupIDSet);
	std::set<unsigned int>::iterator iter_set = groupIDSet.begin();
	for (; iter_set != groupIDSet.end(); ++iter_set)
	{
		LOG_PRINT(log_debug, "user may be group member,so need to clean map res.client connid:%u,userid:%u,groupid:%u.", connection->getconnid(), connection->user_id, *iter_set);
		CGateBase::m_RoomClientMgr.del_user_connid(*iter_set, connection->user_id, connection->getconnid());
	}
}

void CClientManager::update_client_map(clienthandler_ptr connection)
{
	//boost::mutex::scoped_lock lock(client_session_mutex_);
	if (connection)
	{
		session_map_[connection->getconnid()] = connection;
		LOG_PRINT(log_debug, "CClientManager::update_client_map() session[client connid:%u], total-size=[%u].", connection->getconnid(), session_map_.size());
	}
}

clienthandler_ptr CClientManager::find_client_map(unsigned int connect_id)
{
	clienthandler_ptr connection;
	//boost::mutex::scoped_lock lock(client_session_mutex_);
	std::map<unsigned int, clienthandler_ptr >::iterator iter = session_map_.find(connect_id);
	if (iter != session_map_.end())
	{
		connection = iter->second;
	}

	return connection;
}

//send msg to client connection by connID,0 means success, others means fail.
int CClientManager::send_msg_to_client_by_connID(unsigned int connect_id, SL_ByteBuffer & buff, e_NoticeDevType e_dev_type/* = e_Notice_AllType */)
{
	clienthandler_ptr connection;
	//boost::mutex::scoped_lock lock(client_session_mutex_);
	connection = find_client_map(connect_id);
	if (connection && connection->user_data)
	{
		CClientItem * item = (CClientItem *)connection->user_data;
		if (e_Notice_AllType == e_dev_type || e_dev_type == item->m_mobile)
		{
			connection->write_message(buff.data(), buff.data_size());
			return 0;
		}
	}

	return -1;
}

void CClientManager::setuserconn(unsigned int userid, byte nmobile, unsigned int nlogintime, unsigned int idconn)
{
	char sBuf[128] = {0};
	sprintf(sBuf, "%d_%d_%d", userid, nmobile, nlogintime);
	std::string strKey = std::string(sBuf);

    //boost::mutex::scoped_lock lock(client_session_mutex_);
    std::map<std::string, unsigned int>::iterator iter = m_mapuser.find(strKey);
    if (iter != m_mapuser.end())
    {
        iter->second = idconn;
    }
    else
    {
        m_mapuser.insert(std::make_pair(strKey, idconn));
    }

	std::map<unsigned int, std::set<unsigned int> >::iterator iter_map = m_user_connid_map.find(userid);
	if (iter_map != m_user_connid_map.end())
	{
		iter_map->second.insert(idconn);
	}
	else
	{
		std::set<unsigned int> connid_set;
		connid_set.insert(idconn);
		m_user_connid_map.insert(std::make_pair(userid, connid_set));
	}
}

clienthandler_ptr CClientManager::find_user_conn(unsigned int iduser, byte nmoblie, unsigned int nlogintime)
{
	clienthandler_ptr connection;
	//boost::mutex::scoped_lock lock(client_session_mutex_);

	char sBuf[128] = {0};
	sprintf(sBuf, "%d_%d_%d", iduser, nmoblie, nlogintime);
	std::string strKey = std::string(sBuf);

	std::map<std::string, unsigned int>::iterator iter_user = m_mapuser.find(strKey);
	if (iter_user != m_mapuser.end())
	{
		unsigned int connect_id = iter_user->second;
		connection = find_client_map(connect_id);
	}
	else
	{
		LOG_PRINT(log_warning, "find_user_conn() user[%s] not found", sBuf);
	}
	return connection;
}

void CClientManager::getallconnptr(std::vector<clienthandler_ptr> & vecconn)
{
	vecconn.resize(session_map_.size());
	//boost::mutex::scoped_lock lock(client_session_mutex_);
	std::map<unsigned int, clienthandler_ptr >::iterator iter = session_map_.begin();
	for(int i = 0; iter != session_map_.end(); ++iter)
	{
		vecconn[i++] = iter->second;
	}
}

void CClientManager::get_user_all_connects(unsigned int userid, std::set<unsigned int > & conn_setID)
{
	conn_setID.clear();
	std::map<unsigned int, std::set<unsigned int> >::iterator iter_map = m_user_connid_map.find(userid);
	if (iter_map != m_user_connid_map.end())
	{
		conn_setID.insert(iter_map->second.begin(), iter_map->second.end());
	}
}

void CClientManager::get_room_user_conn_by_roletype_filter(std::set<clienthandler_ptr> &sConn, unsigned int roomid, unsigned int minRole, unsigned int maxRole /*= 0*/, e_NoticeDevType e_dev_type/* = e_Notice_AllType*/)
{
	//get set of user connect id.
	CONNECT_SET connidSet;
	CGateBase::m_RoomClientMgr.get_user_connids(roomid, connidSet);
	if (connidSet.empty())
	{
		LOG_PRINT(log_warning, "[broadcast_room_all_connects]roomid:%u has no client connection id.", roomid);
		return;
	}

	unsigned int count = 0;
	unsigned int logcount = 0;
	std::list<std::string> userlog_lst;
	std::string userlog = "";

	//get clienthandler to send msg
	for (CONNECT_SET::iterator iter_set = connidSet.begin(); iter_set != connidSet.end(); ++iter_set)
	{
		unsigned int connect_id = *iter_set;

		clienthandler_ptr connection = find_client_map(connect_id);
		if (connection && connection->user_data)
		{
			CClientItem * item = (CClientItem *)connection->user_data;
			if (e_Notice_AllType != e_dev_type && e_dev_type != item->m_mobile)
			{
				continue;
			}
			unsigned int roleType = 0;
			item->getGroupRole(roomid, roleType);
			if (roleType < minRole || (maxRole && maxRole < roleType))
			{
				continue;
			}
			sConn.insert(connection);
			++logcount;
			++count;
			char cID[LEN32] = {0};
			sprintf(cID, "%u,", connection->user_id);
			userlog += cID;
			if (logcount >= 150)
			{
				userlog_lst.push_back(userlog);
				logcount = 0;
				userlog.clear();
			}
		}	
	}
	if (userlog.length())
		userlog_lst.push_back(userlog);

	std::list<std::string>::iterator iter = userlog_lst.begin();
	for (; iter != userlog_lst.end(); ++iter)
	{
		LOG_PRINT(log_debug, "[get_room_user_conn_by_filter]roomid:%u,minRole:%u,maxRole:%u,dev_type:%d,user list:%s.", roomid, minRole, maxRole, (int)e_dev_type, iter->c_str());
	}

	LOG_PRINT(log_debug, "[get_room_user_conn_by_filter]roomid:%u,minRole:%u,maxRole:%u,dev_type:%d,user count:%u.", roomid, minRole, maxRole, (int)e_dev_type, count);
}

void CClientManager::save_user_invite_mic(unsigned int userid, unsigned int runnerID, unsigned int touserID, int sessionid)
{
	std::set<unsigned int> user_conn_set;

	{
		//boost::mutex::scoped_lock lock(client_session_mutex_);
		std::map<unsigned int, std::set<unsigned int> >::iterator iter_map = m_user_connid_map.find(userid);
		if (iter_map != m_user_connid_map.end())
		{
			user_conn_set.insert(iter_map->second.begin(), iter_map->second.end());
		}
	}

	if (user_conn_set.empty())
	{
		return;
	}

	std::set<unsigned int>::iterator iter_set = user_conn_set.begin();
	for (; iter_set != user_conn_set.end(); ++iter_set)
	{
		unsigned int connect_id = *iter_set;

		//boost::mutex::scoped_lock lock(client_session_mutex_);

		clienthandler_ptr connection = find_client_map(connect_id);
		if (connection && connection->user_data)
		{
			CClientItem * item = (CClientItem *)connection->user_data;
			item->fill_invite_mic_state(runnerID, touserID, sessionid);
			LOG_PRINT(log_debug, "[save invite on mic state]userid:%u,runnerid:%u,touserid:%u,sessionid:%d.", userid, runnerID, touserID, sessionid);
		}
	}
}

void CClientManager::reset_user_invite_mic(unsigned int userid)
{
	std::set<unsigned int> user_conn_set;

	{
		//boost::mutex::scoped_lock lock(client_session_mutex_);
		std::map<unsigned int, std::set<unsigned int> >::iterator iter_map = m_user_connid_map.find(userid);
		if (iter_map != m_user_connid_map.end())
		{
			user_conn_set.insert(iter_map->second.begin(), iter_map->second.end());
		}
	}

	if (user_conn_set.empty())
	{
		return;
	}

	std::set<unsigned int>::iterator iter_set = user_conn_set.begin();
	for (; iter_set != user_conn_set.end(); ++iter_set)
	{
		unsigned int connect_id = *iter_set;

		//boost::mutex::scoped_lock lock(client_session_mutex_);

		clienthandler_ptr connection = find_client_map(connect_id);
		if (connection && connection->user_data)
		{
			CClientItem * item = (CClientItem *)connection->user_data;
			if (userid == item->m_invite_mic_runid || userid == item->m_invite_mic_toid)
			{
				item->reset_invite_mic_state();
				LOG_PRINT(log_debug, "[reset invite mic state]userid:%u.", userid);
			}
		}
	}
}

void CClientManager::broadcast_all_client(byte termtype, SL_ByteBuffer & buff, unsigned int except_connid)
{
    //boost::mutex::scoped_lock lock(client_session_mutex_);

    int count = 0;
    std::map<unsigned int, clienthandler_ptr >::iterator iter = session_map_.begin();
    for (; iter != session_map_.end(); ++iter)
    {
		clienthandler_ptr connection = iter->second;
        if (connection && connection->user_data)
        {
			/*
			if (connection->getconnid() == except_connid)
			{
				continue;
			}
			*/

			CClientItem * item = (CClientItem *)connection->user_data;
            if (e_Notice_AllType == termtype || termtype == item->m_mobile)
            {
                connection->write_message(buff.data(), buff.data_size());
                ++count;
				LOG_PRINT(log_debug, "[broadcast_all_client]userid:%u,client connid:%u,%s:%d,dev_type:%d,uuid:%s.", \
					connection->user_id, connection->getconnid(), connection->getremote_ip(), connection->getremote_port(), (int)item->m_mobile, item->m_uuid.c_str());
            }
        }
    }

	LOG_PRINT(log_info, "[Broadcast all client]result has sent nmobile:%u,except_connid:%u,users count:%u.", termtype, except_connid, count);
}

void CClientManager::broadcast_all_subscribe_client(e_NoticeDevType termtype, unsigned int subcmd, SL_ByteBuffer & buff)
{
    int count = 0;
    std::map<unsigned int, clienthandler_ptr >::iterator iter = session_map_.begin();
    for (; iter != session_map_.end(); ++iter)
    {
		clienthandler_ptr connection = iter->second;
        if (connection && connection->user_data)
        {
			CClientItem * item = (CClientItem *)connection->user_data;
			//client is not at subscribe this cmd, skip to cast msg;
			COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *)buff.data();
			if (false == item->get_pushmsg_pass(in_msg->subcmd))
			{
				continue;
			}

            if (e_Notice_AllType == termtype || termtype == item->m_mobile)
            {
                connection->write_message(buff.data(), buff.data_size());
                ++count;
				LOG_PRINT(log_debug, "[Broadcast all subscribe client]userid:%u,client connid:%u,%s:%d,dev_type:%d,uuid:%s.", \
					connection->user_id, connection->getconnid(), connection->getremote_ip(), connection->getremote_port(), (int)item->m_mobile, item->m_uuid.c_str());
            }
        }
    }

	LOG_PRINT(log_info, "[Broadcast all subscribe client]result has sent nmobile:%d,users count:%u,subcmd:%u.", (int)termtype, count, subcmd);
}

void CClientManager::broadcast_user_subscribe_client(unsigned int userid, e_NoticeDevType termtype, unsigned int subcmd, SL_ByteBuffer & buff)
{
	std::set<unsigned int> user_conn_set;

	{
		std::map<unsigned int, std::set<unsigned int> >::iterator iter_map = m_user_connid_map.find(userid);
		if (iter_map != m_user_connid_map.end())
		{
			user_conn_set.insert(iter_map->second.begin(), iter_map->second.end());
		}
	}

	if (user_conn_set.empty())
	{
		return;
	}

	int count = 0;

	std::set<unsigned int>::iterator iter_set = user_conn_set.begin();
	for (; iter_set != user_conn_set.end(); ++iter_set)
	{
		unsigned int connect_id = *iter_set;

		clienthandler_ptr connection = find_client_map(connect_id);
		if (connection && connection->user_data)
		{
			CClientItem * item = (CClientItem *)connection->user_data;
			//client is not at subscribe this cmd, skip to cast msg;
			COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *)buff.data();
			if (false == item->get_pushmsg_pass(in_msg->subcmd))
			{
				continue;
			}

			if (e_Notice_AllType == termtype || termtype == item->m_mobile)
			{
				connection->write_message(buff.data(), buff.data_size());
				++count;
				LOG_PRINT(log_debug, "[Broadcast user subscribe client]userid:%u,client connid:%u,%s:%d,dev_type:%d,uuid:%s.", \
					connection->user_id, connection->getconnid(), connection->getremote_ip(), connection->getremote_port(), (int)item->m_mobile, item->m_uuid.c_str());
			}
		}
	}

	LOG_PRINT(log_info, "[Broadcast user subscribe client]result has sent userid:%u,noty_dev_type:%d,connect count:%u,subcmd:%u.", userid, (int)termtype, count, subcmd);
}

static time_t call_time = 0;

void CClientManager::broadcast_room_all_connects(int cmd, unsigned int roomid, unsigned int own_id, SL_ByteBuffer & buff, e_NoticeDevType e_dev_type, bool castInGroupOnly/* = false*/)
{
	//get set of user connect id.
	CONNECT_SET connidSet;
	CGateBase::m_RoomClientMgr.get_user_connids(roomid, connidSet);
	if (connidSet.empty())
	{
		LOG_PRINT(log_warning, "[broadcast_room_all_connects]cmd:%d,roomid:%u has no client connection id.", cmd, roomid);
		return;
	}

	unsigned int count = 0;

	unsigned int logcount = 0;
	std::list<std::string> userlog_lst;
	std::string userlog = "";

	//get clienthandler to send msg
	CONNECT_SET::iterator iter_set = connidSet.begin();
	for (; iter_set != connidSet.end(); ++iter_set)
	{
		unsigned int connect_id = *iter_set;

		/*
		if (connect_id == own_id)
		{
			continue;
		}
		*/

		//boost::mutex::scoped_lock lock(client_session_mutex_);
		clienthandler_ptr connection = find_client_map(connect_id);
		if (connection && connection->user_data)
		{
			//cast user in group now
			if (castInGroupOnly && connection->room_id != roomid)
			{
				continue;
			}

			CClientItem * item = (CClientItem *)connection->user_data;
			if (e_Notice_AllType == e_dev_type || e_dev_type == item->m_mobile)
			{
				connection->write_message(buff.data(), buff.data_size());
				++count;

				
				++logcount;
				char cID[LEN32] = {0};
				sprintf(cID, "%u,", connection->user_id);
				if (logcount < 150)
				{
					userlog += cID;
				}
				else
				{
					userlog_lst.push_back(userlog);
					userlog = cID;
					logcount = 1;
				}
				
			}
		}
	}

	
	if (!userlog.empty())
	{
		userlog_lst.push_back(userlog);
	}
	

	static unsigned int callcout = 0;
	static unsigned int sendcount = 0;
	static unsigned int sendmsglen = 0;
	++callcout;
	sendcount += count;
	sendmsglen += count * buff.data_size();

	if(time(NULL) - call_time > 10)
	{
		LOG_PRINT(log_info, "Broadcast call times:%u,send count:%u,sum of msg len:%u.", callcout, sendcount, sendmsglen);
		callcout = 0;
		sendcount = 0;
		sendmsglen = 0;
		call_time = time(NULL);
	}

	LOG_PRINT(log_info, "[Broadcast room all connets]result has sent cmd:%d,roomid:%u,own_connid:%u,noty_dev_type:%d,users connect count:%u.", cmd, roomid, own_id, (int)e_dev_type, count);

	
	std::list<std::string>::iterator iter = userlog_lst.begin();
	for (; iter != userlog_lst.end(); ++iter)
	{
		LOG_PRINT(log_debug, "[Broadcast room all connets]result has sent cmd:%d,roomid:%u,user list:%s.", cmd, roomid, iter->c_str());
	}
	
}

void CClientManager::broadcast_all_room_all_connects(unsigned int own_id, SL_ByteBuffer & buff, e_NoticeDevType e_dev_type)
{
	//get set of user connect id.
	std::set<unsigned int> user_conn_set;
	CGateBase::m_RoomClientMgr.get_all_user_connids(user_conn_set);
	if (user_conn_set.empty())
	{
		return;
	}
	
	int count = 0;
	//get clienthandler to send msg
	std::set<unsigned int>::iterator iter_set = user_conn_set.begin();
	for (; iter_set != user_conn_set.end(); ++iter_set)
	{
		unsigned int connect_id = *iter_set;

		/*
		if (connect_id == own_id)
		{
			continue;
		}
		*/

		//boost::mutex::scoped_lock lock(client_session_mutex_);
		clienthandler_ptr connection = find_client_map(connect_id);
		if (connection && connection->user_data)
		{
			CClientItem * item = (CClientItem *)connection->user_data;
			if (e_Notice_AllType == e_dev_type || e_dev_type == item->m_mobile)
			{
				connection->write_message(buff.data(), buff.data_size());
				++count;
			}
		}
	}

	LOG_PRINT(log_info, "[Broadcast all room all connets]result has sent own_connid:%u,noty_dev_type:%d,users connect count:%u.", own_id, (int)e_dev_type, count);
}

void CClientManager::broadcast_room_gender_connects(unsigned int roomid, SL_ByteBuffer & buff, USER_GENDER_TYPE gender)
{
	//get set of user connect id.
	CONNECT_SET connidSet;
	CGateBase::m_RoomClientMgr.get_user_connids(roomid, connidSet);
	if (connidSet.empty())
	{
		LOG_PRINT(log_warning, "[broadcast_room_gender_connects]roomid:%u has no client connection id.", roomid);
		return;
	}

	unsigned int count = 0;

	unsigned int logcount = 0;
	std::list<std::string> userlog_lst;
	std::string userlog = "";

	//get clienthandler to send msg
	CONNECT_SET::iterator iter_set = connidSet.begin();
	for (; iter_set != connidSet.end(); ++iter_set)
	{
		unsigned int connect_id = *iter_set;

		//boost::mutex::scoped_lock lock(client_session_mutex_);
		clienthandler_ptr connection = find_client_map(connect_id);
		if (connection && connection->user_data)
		{
			CClientItem * item = (CClientItem *)connection->user_data;
			if (item->m_gender == (unsigned int)gender)
			{
				connection->write_message(buff.data(), buff.data_size());
				++count;

				++logcount;
				char cID[LEN32] = {0};
				sprintf(cID, "%u,", connection->user_id);
				if (logcount < 150)
				{
					userlog += cID;
				}
				else
				{
					userlog_lst.push_back(userlog);
					userlog = cID;
					logcount = 1;
				}
				
			}
		}
	}

	if (!userlog.empty())
	{
		userlog_lst.push_back(userlog);
	}

	LOG_PRINT(log_info, "[Broadcast room gender connets]result has sent roomid:%u,gender:%d,users connect count:%u.", roomid, (int)gender, count);
	std::list<std::string>::iterator iter = userlog_lst.begin();
	for (; iter != userlog_lst.end(); ++iter)
	{
		LOG_PRINT(log_debug, "[Broadcast room gender connets]result has sent roomid:%u,user list:%s.", roomid, iter->c_str());
	}
}

void CClientManager::broadcast_all_room_gender_connects(SL_ByteBuffer & buff, USER_GENDER_TYPE gender)
{
	//get set of user connect id.
	std::set<unsigned int> user_conn_set;
	CGateBase::m_RoomClientMgr.get_all_user_connids(user_conn_set);
	if (user_conn_set.empty())
	{
		return;
	}
	
	int count = 0;
	//get clienthandler to send msg
	std::set<unsigned int>::iterator iter_set = user_conn_set.begin();
	for (; iter_set != user_conn_set.end(); ++iter_set)
	{
		unsigned int connect_id = *iter_set;

		//boost::mutex::scoped_lock lock(client_session_mutex_);
		clienthandler_ptr connection = find_client_map(connect_id);
		if (connection && connection->user_data)
		{
			CClientItem * item = (CClientItem *)connection->user_data;
			if (item->m_gender == (unsigned int)gender)
			{
				connection->write_message(buff.data(), buff.data_size());
				++count;
			}
		}
	}

	LOG_PRINT(log_info, "[Broadcast all room gender connets]result has sent gender:%d,users connect count:%u.", (int)gender, count);
}

void CClientManager::broadcast_user_all_connects(unsigned int userid, SL_ByteBuffer & buff, e_NoticeDevType e_dev_type, unsigned int except_connid)
{
	std::set<unsigned int> user_conn_set;

	{
		//boost::mutex::scoped_lock lock(client_session_mutex_);
		std::map<unsigned int, std::set<unsigned int> >::iterator iter_map = m_user_connid_map.find(userid);
		if (iter_map != m_user_connid_map.end())
		{
			user_conn_set.insert(iter_map->second.begin(), iter_map->second.end());
		}
	}

	if (user_conn_set.empty())
	{
		return;
	}

	int count = 0;

	std::set<unsigned int>::iterator iter_set = user_conn_set.begin();
	for (; iter_set != user_conn_set.end(); ++iter_set)
	{
		unsigned int connect_id = *iter_set;

		/*
		if (connect_id == except_connid)
		{
			continue;
		}
		*/

		//boost::mutex::scoped_lock lock(client_session_mutex_);

		clienthandler_ptr connection = find_client_map(connect_id);
		if (connection && connection->user_data)
		{
			CClientItem * item = (CClientItem *)connection->user_data;
			if (e_Notice_AllType == e_dev_type || e_dev_type == item->m_mobile)
			{
				connection->write_message(buff.data(), buff.data_size());
				++count;
				LOG_PRINT(log_debug, "[broadcast_user_all_connects]userid:%u,client connid:%u,%s:%d,dev_type:%d,uuid:%s.", \
					connection->user_id, connection->getconnid(), connection->getremote_ip(), connection->getremote_port(), (int)item->m_mobile, item->m_uuid.c_str());
			}
		}
	}

	LOG_PRINT(log_info, "[Broadcast user all connects]result has sent userid:%u,except_connid:%u,noty_dev_type:%d,connect count:%u.", userid, except_connid, (int)e_dev_type, count);
}

void CClientManager::insert_req_check(unsigned int connect_id, unsigned int req_cmd, const REQ_SVR_INFO & oInfo)
{
	//boost::mutex::scoped_lock lock(client_session_mutex_);
	std::map<unsigned int, REQ_TIME_MAP >::iterator iter = m_connid_time_map.find(connect_id);
	if (iter != m_connid_time_map.end())
	{
		iter->second[req_cmd] = oInfo;
	}
	else
	{
		REQ_TIME_MAP map;
		map.insert(std::make_pair(req_cmd, oInfo));
		m_connid_time_map.insert(std::make_pair(connect_id, map));
	}
	LOG_PRINT(log_debug, "[insert_req_check]client connid:%u,req_cmd:%u,map size:%u.", connect_id, req_cmd, m_connid_time_map.size());
}

void CClientManager::del_req_check(unsigned int connect_id, unsigned int req_cmd)
{
	if (!connect_id || !req_cmd)
	{
		return;
	}

	//boost::mutex::scoped_lock lock(client_session_mutex_);
	std::map<unsigned int, REQ_TIME_MAP >::iterator iter = m_connid_time_map.find(connect_id);
	if (iter != m_connid_time_map.end())
	{
		iter->second.erase(req_cmd);
		if (iter->second.empty())
		{
			m_connid_time_map.erase(iter);
		}
		LOG_PRINT(log_debug, "[del_req_check]client connid:%u,req_cmd:%u,map size:%u.", connect_id, req_cmd, m_connid_time_map.size());
	}
}

void CClientManager::del_req_check_by_conn_set(const std::set<unsigned int> & client_connid_set, unsigned int req_cmd)
{
	if (client_connid_set.empty() || !req_cmd)
	{
		return;
	}

	std::set<unsigned int>::const_iterator iter = client_connid_set.begin();
	for (; iter != client_connid_set.end(); ++iter)
	{
		del_req_check(*iter, req_cmd);
	}
}

void CClientManager::get_req_check_connid(std::map<unsigned int, REQ_TIME_MAP > & connid_time_map)
{
	connid_time_map.clear();
	//boost::mutex::scoped_lock lock(client_session_mutex_);
	connid_time_map.insert(m_connid_time_map.begin(), m_connid_time_map.end());
}

void CClientManager::check_req_time()
{
	time_t time_now = time(NULL);
	do 
	{
		//key:server info, value:client set which has problem.
		static std::map<std::string, std::set<unsigned int> > svr_client_map;
		svr_client_map.clear();

		std::map<unsigned int, REQ_TIME_MAP > connid_time_map;
		get_req_check_connid(connid_time_map);

		if (connid_time_map.empty())
		{
			alarm_interval_ = 0;
			break;
		}

		std::map<unsigned int, REQ_TIME_MAP >::iterator iter = connid_time_map.begin();
		for (; iter != connid_time_map.end(); ++iter)
		{
			unsigned int client_connid = iter->first;
			REQ_TIME_MAP & req_time_map = iter->second;
			REQ_TIME_MAP::iterator iter_map = req_time_map.begin();
			for (; iter_map != req_time_map.end(); ++iter_map)
			{
				unsigned int req_cmd = iter_map->first;
				unsigned int time_diff = time_now - iter_map->second.req_time;
				std::string svr_name = CGateBase::change_type_to_svrname(iter_map->second.svr_type);

				if (time_diff >= 10*60)
				{
					char lostalarm[512] = {0};
					sprintf(lostalarm, "response of request may lost(client connid:%u request subcmd:%u from gate to %s:%s:%u has no response in %u seconds)", \
						client_connid, req_cmd, svr_name.c_str(), iter_map->second.svr_ip.c_str(), iter_map->second.svr_port, time_diff);
					LOG_PRINT(log_warning, "%s.", lostalarm);
					del_req_check(client_connid, req_cmd);
					continue;
				}

				if (time_diff >= CGateBase::getReqRespCheckTime())
				{
					clienthandler_ptr clientptr = find_client_map(client_connid);
					if (clientptr && clientptr->user_data)
					{
						CClientItem * pItem = (CClientItem *)(clientptr->user_data);
						pItem->insert_problem_svr_conn(iter_map->second.svr_type, iter_map->second.svr_connID);
						LOG_PRINT(log_warning, "save problem server conn of client connid:%u,svr_type:%u,server connID:%u,%s:%u.", \
							client_connid, iter_map->second.svr_type, iter_map->second.svr_connID, iter_map->second.svr_ip.c_str(), iter_map->second.svr_port);
					}

					char alarmlog[512] = {0};
					sprintf(alarmlog, "[port:%d]client connid:%u request subcmd:%u from gate to %s:%s:%u has no response in %u seconds", \
						CGateBase::Port(), client_connid, req_cmd, svr_name.c_str(), iter_map->second.svr_ip.c_str(), iter_map->second.svr_port, time_diff);

					char svr_info[128] = {0};
					sprintf(svr_info, "%s:%s:%u", svr_name.c_str(), iter_map->second.svr_ip.c_str(), iter_map->second.svr_port);
					if (svr_client_map.end() != svr_client_map.find(std::string(svr_info)))
					{
						svr_client_map[svr_info].insert(client_connid);
					}
					else
					{
						std::set<unsigned int> client_set;
						client_set.insert(client_connid);
						svr_client_map.insert(std::make_pair(std::string(svr_info), client_set));
					}
					
					LOG_PRINT(log_warning, "%s,alarm_checktime:%u seconds", alarmlog, CGateBase::getReqRespCheckTime());
				}
			}
		}

		if (!svr_client_map.empty())
		{
			bool alarmflag = false;

			std::map<std::string, std::set<unsigned int> >::iterator iter_svr = svr_client_map.begin();
			for (; iter_svr != svr_client_map.end(); ++iter_svr)
			{
				char countalarm[512] = {0};
				sprintf(countalarm, "[port:%d]there are %u client_conn request from gate to %s has no response in %u seconds", \
					CGateBase::Port(), iter_svr->second.size(), iter_svr->first.c_str(), CGateBase::getReqRespCheckTime());
				LOG_PRINT(log_warning, "%s.", countalarm);
				if (time_now - last_alarm_cmd_time_ >= 60*alarm_interval_)
				{
					CAlarmNotify::sendAlarmNoty(e_wechat_notitype, e_network_conn, CGateBase::ProcName(), "gate alarm", "Yunwei,Room,Usermgr", countalarm);
					alarmflag = true;
				}
			}

			if (alarmflag)
			{
				last_alarm_cmd_time_ = time_now;

				//change the alarm interval time.
				if (!alarm_interval_)
				{
					alarm_interval_ = 1;
				}
				else if (alarm_interval_ < 9)
				{
					alarm_interval_ += 4;
				}
				else
				{
					alarm_interval_ = 1;
				}
			}
		}
	} while (0);
}
