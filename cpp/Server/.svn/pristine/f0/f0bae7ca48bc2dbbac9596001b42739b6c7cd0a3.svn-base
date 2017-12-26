
#ifndef __SVR_SESSION_MANAGER_HH_
#define __SVR_SESSION_MANAGER_HH_

#include <list>
#include <set>
#include <stdlib.h>
#include <boost/thread/mutex.hpp>
#include <boost/smart_ptr/detail/spinlock.hpp>
#include "clienthandler.h"

typedef std::set<CConnection_ptr> TCPCLIENT_SET;
typedef std::map<int, unsigned int> SVRTYPE_CONNID_MAP;

class ConnectionMgr
{
public:
	ConnectionMgr(void);
	virtual ~ConnectionMgr(void);

	void addConnection(int svr_type, CConnection_ptr connection);
	CConnection_ptr getConnection(int svr_type) ;
	bool delConnection(int svr_type, string ip, int port);
		
private:

    //key:svr_type,value:tcp client set.
    std::map<int, TCPCLIENT_SET > svr_sessions_;
	
    //key:client connection id, value:<svr_type,server connect id>
    std::map<unsigned int, SVRTYPE_CONNID_MAP > session_map_;
};

#endif  //__SVR_SESSION_MANAGER_HH_


