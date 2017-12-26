
#ifndef __USERMGREVENT_H__
#define __USERMGREVENT_H__

#include "ProtocolsBase.h"
#include "Config.h"
#include "CUserinfo.h"
#include "CUserMgr.h"
#include "CConnInfo.h"
#include "redisMgr.h"
#include "ClientConnectionMgr.h"
#include "DBConnection.h"
#include "CUserGroupMgr.h"

class UsermgrEvent: public IWorkerEvent
{
public:
	UsermgrEvent();
	virtual ~UsermgrEvent();

	virtual bool onConnect(clienthandler_ptr connection);
	virtual bool onMessage(task_proc_data * message);
	virtual bool onError(clienthandler_ptr connection, int code, std::string msg);
	virtual bool onClose(clienthandler_ptr connection);

	bool onTimer();
	bool loadConfig(Config & config);
	bool init(Config & config);
	void setWorkID(unsigned int workid);

public:
	redisMgr				* m_pRedisMgr;
	CClientConnectionMgr	m_clientConnMgr;

private:
	int proc_userlogin_notify(task_proc_data * message);
	int proc_userlogout_notify(task_proc_data * message);
	void proc_micstate_transfer(task_proc_data * message);
	std::string changeBirth2StarSign(const std::string & strBirth);
	void proc_user_sysparam_config(unsigned int userid, DEVTYPE termtype, unsigned int conn_id, const CConnInfo & gateObj);
	void proc_user_relategrouplst(unsigned int userid, unsigned int conn_id, const CConnInfo & gateObj);
	void notifyRelateGroupSetting(unsigned int groupID, unsigned int conn_id, const CConnInfo & gateObj);
	
	int handle_usermodinfo_req(task_proc_data * message);
	void handle_operFriendshipReq(task_proc_data * message);
	void handle_qryUserAttentionList(task_proc_data * message);
	void handle_hello_msg(task_proc_data * message);
	void handle_ping_msg(task_proc_data * message);
	void register_cmd_msg(clienthandler_ptr client);
	void handele_userLocationInfo(task_proc_data * message);
	unsigned int	m_workid;
    time_t			m_last_printstackinfotime;
	std::list<std::string>	m_cmdrange_lst;
	std::string				m_cmdlist;
	
    //Redis
	string 			m_strRedisHost;
	uint16_t 		m_nRedisPort;
	string 			m_sRedisPass;
	int 			m_redisThreadNum;
    CUserMgr 		m_usermgr;
    Geohash 		m_geohash;
};

#endif /* __USERMGREVENT_H__ */
