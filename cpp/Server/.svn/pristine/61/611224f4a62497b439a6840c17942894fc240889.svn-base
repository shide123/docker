/*
 * CSvrEvent.h
 *
 *  Created on: Aug 22, 2016
 *      Author: testtest
 */

#ifndef __CSVREVENT_H__
#define __CSVREVENT_H__

#include "ProtocolsBase.h"
#include "SL_ByteBuffer.h"
#include "message_comm.h"
#include "message_vchat.h"
#include "CLogThread.h"
#include "CSvrItem.h"
#include "timer.h"
#include "clienthandler.h"

class CSvrEvent: public IWorkerEvent
{
public:
	CSvrEvent();
	virtual ~CSvrEvent();

	virtual bool onMessage(task_proc_data * message);
	virtual bool onConnect(clienthandler_ptr connection);
	virtual bool onClose(clienthandler_ptr connection);

protected:
	virtual void afterConnect(clienthandler_ptr svr_conn){};
	virtual bool onTimer(clienthandler_ptr svr_conn);

	virtual void send_hello_msg(clienthandler_ptr svr_conn);
	virtual void send_ping_msg(clienthandler_ptr svr_conn);

	virtual int handle_message(const char * msg, int len, clienthandler_ptr svr_conn);
	virtual int handle_logic_msg(const char * msg, int len, clienthandler_ptr svr_conn);
	void handle_register_msg(const char * msg, int len, clienthandler_ptr svr_conn);
	virtual void handle_pingrsp_msg(const char * msg, int len, clienthandler_ptr svr_conn);
	void handle_broadcast_user_msg(const char * msg, int len, clienthandler_ptr svr_conn);
	void handle_multicast_user_msg(const char * msg, int len, clienthandler_ptr svr_conn);
	void handle_broadcast_room_msg(const char * msg, int len, clienthandler_ptr svr_conn);
	void handle_broadcast_room_roletype(const char * msg, int len, clienthandler_ptr svr_conn);
	void handle_broadcast_app_version(const char * msg, int len, clienthandler_ptr svr_conn);
	void handle_broadcast_subscribe_msg(const char * msg, int len, clienthandler_ptr svr_conn);
	void handle_cast_clients_on_one_svr(const char * msg, int len, clienthandler_ptr svr_conn);
	void handle_broadcast_room_gender_msg(const char* msg, int len, clienthandler_ptr svr_conn);

	virtual void post_close_process(clienthandler_ptr svr_conn){};
	void del_req_check_by_connid(clienthandler_ptr svr_conn);
	void changeGroupRoleType(unsigned int userid, unsigned int groupid, unsigned int new_roleType);
	void cleanKickoutUserRes(unsigned int connid, unsigned int roomid, unsigned int touserid);
	void handleMicState(unsigned int toUserID, unsigned int runUserID, unsigned int groupID, int micIndex, int micState);
	void handleInviteOnMicState(unsigned int runnerID, unsigned int touserID, int optype, int sessionid);
	void print_specail_cmd(const char * msg, int len, clienthandler_ptr svr_conn);

	//DATA
	unsigned int conn_id(clienthandler_ptr svr_conn){ return svr_conn->getconnid(); }
	const char * get_remote_ip(clienthandler_ptr svr_conn){ return svr_conn->getremote_ip(); }
	ushort get_remote_port(clienthandler_ptr svr_conn){ return svr_conn->getremote_port(); }
	std::string get_svr_name(clienthandler_ptr svr_conn);
	void set_pingtime(clienthandler_ptr svr_conn);
	time_t get_pingtime(clienthandler_ptr svr_conn);
};

#endif /* __CSVREVENT_H__ */
