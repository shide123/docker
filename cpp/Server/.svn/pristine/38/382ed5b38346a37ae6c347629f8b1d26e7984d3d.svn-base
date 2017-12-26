
#include "ConnectionMgr.h"
#include <iostream>
#include "CLogThread.h"
#include "GateBase.h"

ConnectionMgr::ConnectionMgr()
{
	svr_sessions_.clear();
}

ConnectionMgr::~ConnectionMgr()
{

}

void ConnectionMgr::addConnection(string strname, CConnection_ptr connection)
{
	addConnection(CGateBase::change_svrname_to_type(strname), connection);
}

//增加服务器的链接
void ConnectionMgr::addConnection(int svr_type, CConnection_ptr connection)
{
	if (!connection)
	{
		return ;
	}
	clienthandler_ptr conn = connection->get_connecthandle();
	if (!conn)
	{
		return ;
	}
	uint64 key = getKey(conn->getremote_ip(), conn->getremote_port());

	if (chkConnection(svr_type, key))
	{
		LOG_PRINT(log_warning, "connection[%s:%d] is exists", conn->getremote_ip(), conn->getremote_port());
		return;
	}

    std::map<int, TCPCLIENT_MAP >::iterator iter = svr_sessions_.find(svr_type);
    if (iter != svr_sessions_.end())
    {
        iter->second.insert(std::make_pair(key, connection));
    }
    else
    {
        TCPCLIENT_MAP tcpset;
        tcpset.insert(std::make_pair(key, connection));
        svr_sessions_.insert(std::make_pair(svr_type, tcpset));
    }
}

bool ConnectionMgr::chkConnection(int svr_type, string ip, int port)
{
	return chkConnection(svr_type, getKey(ip, port));
}

bool ConnectionMgr::chkConnection(int svr_type, uint64 key)
{
	std::map<int, TCPCLIENT_MAP >::iterator iter_sess = svr_sessions_.find(svr_type);

	return iter_sess != svr_sessions_.end() && iter_sess->second.find(key) != iter_sess->second.end();
}

bool ConnectionMgr::chkConnection(string svrname, string ip, int port)
{
	return chkConnection(CGateBase::change_svrname_to_type(svrname), ip, port);
}

CConnection_ptr ConnectionMgr::getConnection(const string & svrname, string ip, int port)
{
	CConnection_ptr ret;
	unsigned int svr_type = CGateBase::change_svrname_to_type(svrname);
	uint64 key = getKey(ip, port);

	std::map<int, TCPCLIENT_MAP >::iterator iter = svr_sessions_.find(svr_type);
	if (iter != svr_sessions_.end())
	{
		TCPCLIENT_MAP::iterator iter_map = iter->second.find(key);
		if (iter_map != iter->second.end())
		{
			ret = iter_map->second;
		}
	}

	return ret;
}

bool ConnectionMgr::delConnection(string strname, string ip, int port)
{
	return delConnection(CGateBase::change_svrname_to_type(strname), ip, port);
}

bool ConnectionMgr::delConnection(int svr_type, string ip, int port)
{
	if (!chkConnection(svr_type, ip, port))
	{
		LOG_PRINT(log_warning, "connection[%s:%d] is not exists", ip.c_str(), port);
		return false;
	}

	CConnection_ptr pSvrConnPtr;
	std::map<int, TCPCLIENT_MAP >::iterator iter_sess = svr_sessions_.find(svr_type);
    if (iter_sess != svr_sessions_.end())
    {
		TCPCLIENT_MAP & tcpset = iter_sess->second;
		TCPCLIENT_MAP::iterator iter_set = tcpset.find(getKey(ip, port));
		if (iter_set != tcpset.end())
		{
			iter_set->second->get_connecthandle()->stop();
			tcpset.erase(iter_set);
			return true;				
        }
    }
    return false;
}

uint64 ConnectionMgr::getKey(string ip, uint16 port)
{
	uint64 uKey = ((uint32)inet_addr(ip.c_str())) & 0xFFFFFFFF;
	uKey |= (((uint64)port) << 32);

	return uKey;
}

