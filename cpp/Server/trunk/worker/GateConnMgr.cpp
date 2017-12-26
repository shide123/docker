﻿#include "GateConnMgr.h"

CGateConnMgr::CGateConnMgr(void)
{
}

CGateConnMgr::~CGateConnMgr(void)
{
}

void CGateConnMgr::addConn(clienthandler_ptr client)
{
	m_sConns.insert(client);
}

void CGateConnMgr::delConn(clienthandler_ptr client)
{
	m_sConns.erase(client);
}

bool CGateConnMgr::sendMsg(const char *pData, size_t len)
{
	if (NULL == pData || 0 == len)
		return false;

	bool ret = false;
	for (CONN_ITERATOR it = m_sConns.begin(); it != m_sConns.end(); it++)
	{
		clienthandler_ptr conn = *it;
		if (conn && conn->isconnected())
		{
			conn->write_message(pData, len);
			ret = true;
		}
	}

	return ret;
}

bool CGateConnMgr::sendMsgToGate(const char *pData, size_t len, uint32 gateid /*= 0*/)
{
	if (0 == gateid)
	{
		return sendMsg(pData, len);
	}

	if (NULL == pData || 0 == len)
		return false;

	for (CONN_ITERATOR it = m_sConns.begin(); it != m_sConns.end(); it++)
	{
		clienthandler_ptr conn = *it;
		if (conn && conn->isconnected() && gateid == conn->getgateid())
		{
			conn->write_message(pData, len);
			return true;
		}
	}
	
	return false;
}
