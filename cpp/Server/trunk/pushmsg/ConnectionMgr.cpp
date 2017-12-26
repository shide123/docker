
#include "ConnectionMgr.h"

#include <iostream>

#include "CLogThread.h"

ConnectionMgr::ConnectionMgr()
{
    session_map_.clear();
	svr_sessions_.clear();
}

ConnectionMgr::~ConnectionMgr()
{

}

//增加服务器的链接
void ConnectionMgr::addConnection(int svr_type, CConnection_ptr connection)
{
    std::map<int, TCPCLIENT_SET >::iterator iter = svr_sessions_.find(svr_type);
    if (iter != svr_sessions_.end())
    {
        iter->second.insert(connection);
    }
    else
    {
        TCPCLIENT_SET tcpset;
        tcpset.insert(connection);
        svr_sessions_.insert(std::make_pair(svr_type, tcpset));
    }
}

CConnection_ptr ConnectionMgr::getConnection(int svr_type)
{
	CConnection_ptr pSvrConnPtr;
	std::map<int, TCPCLIENT_SET >::iterator iter_sess = svr_sessions_.find(svr_type);
    if (iter_sess != svr_sessions_.end())
    {
		TCPCLIENT_SET & tcpset = iter_sess->second;
		TCPCLIENT_SET::iterator iter_set = tcpset.begin();
		for (; iter_set != tcpset.end(); iter_set++)
        {
            pSvrConnPtr = *iter_set;
			if (!pSvrConnPtr)
			{
				continue;
			}

            //check if server connection is ok
            if (!pSvrConnPtr->get_connecthandle()->isconnected())
            {
                continue;
            }

			break;
        }
    }
	else {
		LOG_PRINT(log_error, "cannot find svr_type:%d of server sessions!", svr_type);
	}
	return pSvrConnPtr;
}

bool ConnectionMgr::delConnection(int svr_type, string ip, int port)
{
	CConnection_ptr pSvrConnPtr;
	std::map<int, TCPCLIENT_SET >::iterator iter_sess = svr_sessions_.find(svr_type);
    if (iter_sess != svr_sessions_.end())
    {
		TCPCLIENT_SET & tcpset = iter_sess->second;
		TCPCLIENT_SET::iterator iter_set = tcpset.begin();
		for (; iter_set != tcpset.end(); iter_set++)
        {
            pSvrConnPtr = *iter_set;

            if (ip.compare(pSvrConnPtr->get_connecthandle()->getremote_ip()) == 0
            		&& port == pSvrConnPtr->get_connecthandle()->getremote_port())
            {
				pSvrConnPtr->get_connecthandle()->stop();
                tcpset.erase(iter_set);
            	return true;
            }
        }
    }
    return false;
}

/*************************************************************** 文件结束 *********************************************************************/

