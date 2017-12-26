
#ifndef __USERMGREVENT_H__
#define __USERMGREVENT_H__

#include "ProtocolsBase.h"
#include "Config.h"
#include "bridgeMgr.h"
#include "redisMgr.h"
#include "DBConnection.h"

class LogonEvent: public IWorkerEvent
{
public:
	LogonEvent(unsigned int work_id);
	virtual ~LogonEvent();

	virtual bool onConnect(clienthandler_ptr connection);
	virtual bool onMessage(task_proc_data * message);
	void register_cmd_msg(clienthandler_ptr client);
	virtual bool onError(clienthandler_ptr connection, int code, std::string msg);
	virtual bool onClose(clienthandler_ptr connection);

	bool onTimer();
	bool loadConfig();
	bool init();

    int proc_setuserprofile_req(task_proc_data * task_node);
    int proc_setusermoreinfo_req(task_proc_data * task_node);
    int proc_setuserpwd_req(task_proc_data * task_node);
    int proc_logonreq4(task_proc_data * task_node);

private:
	void handle_ping_msg(task_proc_data * task_node);

	std::list<std::string> m_cmdrange_lst;
	std::string m_cmdlist;
	unsigned int m_id;
};

#endif /* __USERMGREVENT_H__ */
