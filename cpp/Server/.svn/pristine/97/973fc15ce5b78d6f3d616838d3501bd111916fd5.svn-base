
#ifndef __PUSHMSGLOGIC_H__
#define __PUSHMSGLOGIC_H__

#include "ProtocolsBase.h"
#include "Config.h"
#include "DBConnection.h"
#include "bridgeMgr.h"
#include "redisMgr.h"
#include "ConnectionMgr.h"
#include "TCommonIf_types.h"
#include "DBSink.h"
#include "GateConnMgr.h"

//通知条件类型
enum e_NoticeConditionType
{
	e_NoticeUser		= 1,	//通知指定用户
	e_NoticeTel			= 2,	//通知指定手机号
	e_NoticeOS			= 3,	//通知指定系统(0: PC, 1: Android, 2: IOS, 3: WEB)
	e_NoticeAppVersion	= 4,	//通知指定客户端版本号
};

typedef enum {
	e_ActionGroup = 1,			/*启动房间 房间状态为1*/
	e_ModGroup    = 2,			/*修改房间 房间状态为2*/
}e_GroupMgrAction;

typedef std::set<uint32> 		CONNID_SET;
class PushmsgLogic: public IWorkerEvent
{
public:

	PushmsgLogic();
	virtual ~PushmsgLogic();

	virtual bool onConnect(clienthandler_ptr connection);
	virtual bool onMessage(task_proc_data * message);
	virtual bool onError(clienthandler_ptr connection, int code, std::string msg);
	virtual bool onClose(clienthandler_ptr connection);

	int hello_response(task_proc_data *message);
	bool loadConfig(Config & config);
	bool init(Config & config);
	void handle_ping_msg(task_proc_data * message);
	int32 proc_sendSysNoticeMsg(const ::TNoticeMsg& msg, const std::vector< ::TCondition> & lstCondition, const bool onlineOnly);
	int32 proc_sendUserNoticeMsg(uint32 userid, const ::TNoticeMsg& msg);
	void proc_handleAuthState(uint32 userid, uint32 groupid, uint32 authState);

private:
	int32 getBroadCastType(const std::vector< ::TCondition> & lstCondition);
	bool dealNoticeConditionDB(CDBSink &sink, uint32 notice_id, ::TCondition condition, int &value_id);
	void proc_NoticeMsgNotifyRecv(task_proc_data &task_data, bool isUser = false);
	void proc_unreadNoticeMsgReq(task_proc_data &task_data);
	void register_cmd_msg(clienthandler_ptr client);
public:
	CGateConnMgr m_GateConnMgr;
	static redisMgr 				* m_pRedisMgr;

private:

	time_t							m_last_printstackinfotime;

	string 							m_strCmdList;

	//Redis
	string 			m_strRedisHost;
	uint16_t 		m_nRedisPort;
	string 			m_sRedisPass;
	int 			m_redisThreadNum;


};

#endif /* __PUSHMSGLOGIC_H__ */
