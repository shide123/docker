/*
 * CUsermgrEvent.cpp
 *
 *  Created on: Aug 22, 2016
 *      Author: testtest
 */

#include "CUsermgrEvent.h"
#include "GateBase.h"

CUsermgrEvent::CUsermgrEvent()
{
}

CUsermgrEvent::~CUsermgrEvent()
{
}

void CUsermgrEvent::afterConnect(clienthandler_ptr svr_conn)
{
	if (!svr_conn)
	{
		return;
	}

//	LOG_PRINT(log_info, "after connect usermgr:%s:%u.", get_remote_ip(svr_conn), get_remote_port(svr_conn));
//	CGateBase::m_SvrConnMgr.post_all_user_login(svr_conn);
	return;
}
