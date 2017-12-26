#pragma once

#include <set>
#include "clienthandler.h"

class CConnectionMgr
{
public:
	CConnectionMgr(void);
	virtual ~CConnectionMgr(void);

	void addConn(clienthandler_ptr client);

	void delConn(clienthandler_ptr client);

	bool sendMsg(const char *pData, size_t len);

	bool sendMsgToGate(const char *pData, size_t len, uint32 gateid = 0);

private:
	typedef std::set<clienthandler_ptr> CONN_SET;
	typedef CONN_SET::iterator CONN_ITERATOR;
	std::set<clienthandler_ptr> m_sConns;
};
