/*
 * CRoomEvent.h
 *
 *  Created on: Aug 22, 2016
 *      Author: testtest
 */

#ifndef __CROOMEVENT_H__
#define __CROOMEVENT_H__

#include "CSvrEvent.h"

class CRoomEvent: public CSvrEvent
{
public:
	CRoomEvent();
	virtual ~CRoomEvent();

protected:
	int handle_logic_msg(const char * msg, int len, clienthandler_ptr svr_conn);
	virtual void post_close_process(clienthandler_ptr svr_conn);
	void client_rejoinroom(clienthandler_ptr svr_conn);
	void notify_svr_exitroom(unsigned int roomid, unsigned int userid, ClientGateMask_t * pGate, clienthandler_ptr svr_conn);
};

#endif /* __CROOMEVENT_H__ */
