
#ifndef __CONNECT_MGR_H_
#define __CONNECT_MGR_H_

#include <list>
#include <set>
#include <stdlib.h>
#include <boost/thread/mutex.hpp>
#include <boost/smart_ptr/detail/spinlock.hpp>
#include "clienthandler.h"
#include "yc_datatypes.h"

typedef std::map<uint64, CConnection_ptr> TCPCLIENT_MAP;

class ConnectionMgr
{
public:
	ConnectionMgr(void);
	virtual ~ConnectionMgr(void);

	void addConnection(string svrname, CConnection_ptr connection);
	bool delConnection(string svrname, string ip, int port);
	bool chkConnection(string svrname, string ip, int port);
	CConnection_ptr getConnection(const string & svrname, string ip, int port);
		
private:
	void addConnection(int svr_type, CConnection_ptr connection);
	bool delConnection(int svr_type, string ip, int port) ;
	bool chkConnection(int svr_type, uint64 key);
	bool chkConnection(int svr_type, string ip, int port);
	uint64 getKey(string ip, uint16 port);

    //key:svr_type,value:tcp client set.
    std::map<int, TCPCLIENT_MAP > svr_sessions_;
};

#endif  //__CONNECT_MGR_H_


