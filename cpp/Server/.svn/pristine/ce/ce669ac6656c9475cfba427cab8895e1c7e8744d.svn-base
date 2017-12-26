#ifndef __GATEEVENT_H__
#define __GATEEVENT_H__
#include "ProtocolsBase.h"
class CGateEvent :
	public IWorkerEvent
{
public:
	CGateEvent();
	virtual ~CGateEvent();

	bool onConnect(clienthandler_ptr client_conn);
	bool onMessage(task_proc_data* message);
	bool onClose(clienthandler_ptr connection);
	bool onError(clienthandler_ptr connection, int code, std::string msg);
private:

	void dispatchMessage(task_proc_data* task_data);
	void onTimer(clienthandler_ptr connection);
	void register_cmd_msg(clienthandler_ptr client);
	void handle_ping_msg(task_proc_data * task_node);

};

#endif //__GATEEVENT_H__