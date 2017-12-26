
#ifndef __CLIENTCONNECTIONMGR_HH_
#define __CLIENTCONNECTIONMGR_HH_

#include <list>
#include <set>
#include <stdlib.h>
#include <boost/thread/mutex.hpp>
#include <boost/smart_ptr/detail/spinlock.hpp>
#include "clienthandler.h"
#include "CConnInfo.h"

typedef std::map<CConnInfo, clienthandler_ptr> CLIENT_CONN_MAP;
typedef std::map<int, clienthandler_ptr> GATEID_CLIENT_MAP;

class CClientConnectionMgr
{
public:
	
    CClientConnectionMgr(void);

	virtual ~CClientConnectionMgr(void);

	void addClientConn(clienthandler_ptr connection, const std::string & strIP, int iPort);

	void delClientConn(const std::string & strIP, int iPort);

    int getClientConn(const CConnInfo & connInfo, clienthandler_ptr & pClient);
	int getClientConnByGateid(const CConnInfo & connInfo, clienthandler_ptr & pClient);

    void getAllClientconn(std::list<clienthandler_ptr> & tcpclient_lst);
    
    void castAllClientConn(char * pdata, int datalen, bool bforced = false);
        
    bool checkClientExist(const std::string & strIP, int iPort);
  
	void printConnInfo();

private:

	CLIENT_CONN_MAP m_mapClientConns;
	GATEID_CLIENT_MAP m_mapGateidClients;
};

#endif  //__CLIENTCONNECTIONMGR_HH_


