
#include "ClientConnectionMgr.h"
#include <iostream>
#include "CLogThread.h"

CClientConnectionMgr::CClientConnectionMgr()
{
	m_mapClientConns.clear();
}

CClientConnectionMgr::~CClientConnectionMgr()
{

}

bool CClientConnectionMgr::checkClientExist(const std::string & strIP, int iPort)
{
	CConnInfo oConn(strIP, iPort);
    if (m_mapClientConns.end() == m_mapClientConns.find(oConn))
    {
        return false;
    } 
    else
    {
        return true;
    }
}

//增加与gateway服务器的链接
void CClientConnectionMgr::addClientConn(clienthandler_ptr connection, const std::string & strIP, int iPort)
{
    LOG_PRINT(log_info, "add gateway|webgate %s:%u to m_mapClientConns", strIP.c_str(), iPort);
	CConnInfo oConn(strIP, iPort);
    m_mapClientConns.insert(std::make_pair(oConn, connection));
	m_mapGateidClients.insert(std::make_pair(connection->getgateid(), connection));
}

//删除与gateway服务器的链接
void CClientConnectionMgr::delClientConn(const std::string & strIP, int iPort)
{
	LOG_PRINT(log_info, "delClientConn %s:%u", strIP.c_str(), iPort);
	CConnInfo oConn(strIP, iPort);

    CLIENT_CONN_MAP::iterator iter = m_mapClientConns.find(oConn);
    if (iter != m_mapClientConns.end())
    {
    	LOG_PRINT(log_info, "del gateway|webgate %s:%u gateid:%d from m_mapClientConns", strIP.c_str(), iPort, (int)iter->second->getgateid());
		m_mapGateidClients.erase(iter->second->getgateid());
        m_mapClientConns.erase(oConn);
    }
}

//打印连接信息
void  CClientConnectionMgr::printConnInfo()
{
	CLIENT_CONN_MAP::iterator iterMap = m_mapClientConns.begin();
	for (; iterMap != m_mapClientConns.end(); ++iterMap)
	{
		clienthandler_ptr pClientConn = iterMap->second;
		//判断链接是否正常
        if (!pClientConn->isconnected())
		    continue;

		LOG_PRINT(log_debug, "print_conn_inf[%s:%u] ", pClientConn->getremote_ip(), pClientConn->getremote_port());
	}
}

//获取指定gateway服务器连接
int CClientConnectionMgr::getClientConn(const CConnInfo & connInfo, clienthandler_ptr & pClient)
{
    CLIENT_CONN_MAP::iterator iter = m_mapClientConns.find(connInfo);
    if (iter != m_mapClientConns.end())
    {
        pClient = iter->second;
        return 0;
    }
    else
    {
        return -1;
    }
}

//获取指定gateway服务器连接
int CClientConnectionMgr::getClientConnByGateid(const CConnInfo & connInfo, clienthandler_ptr & pClient)
{
    GATEID_CLIENT_MAP::iterator iter = m_mapGateidClients.find(connInfo.getGateid());
    if (iter != m_mapGateidClients.end())
    {
        pClient = iter->second;
        return 0;
    }
    else
    {
        return -1;
    }
}

void CClientConnectionMgr::getAllClientconn(std::list<clienthandler_ptr> & clientconn_lst)
{
	clientconn_lst.clear();
    CLIENT_CONN_MAP::iterator iterMap = m_mapClientConns.begin();
    for (; iterMap != m_mapClientConns.end(); ++iterMap)
    {
    	clientconn_lst.push_back(iterMap->second);
    }
}

void CClientConnectionMgr::castAllClientConn(char * pdata, int datalen, bool bforced)
{
    CLIENT_CONN_MAP::iterator iterMap = m_mapClientConns.begin();
    for (; iterMap != m_mapClientConns.end(); ++iterMap)
    {
		if (iterMap->second && iterMap->second->isconnected())
			iterMap->second->write_message(pdata, datalen, bforced);
    }
}
