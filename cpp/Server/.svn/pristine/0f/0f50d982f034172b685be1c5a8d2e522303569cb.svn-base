/*
 * CSvrEvent.cpp
 *
 *  Created on: Aug 22, 2016
 *      Author: testtest
 */

#include "PushmsgLogic.h"
#include "redis_def.h"
#include "msgcommapi.h"
#include "AppDelegate.h"
#include "ChatSvr.pb.h"
#include "RoomSvr.pb.h"

redisMgr 						* PushmsgLogic::m_pRedisMgr;
PushmsgLogic::PushmsgLogic()
{
	m_last_printstackinfotime = 0;
	m_redisThreadNum = 1;
	m_nRedisPort = 0;
	m_pRedisMgr		= NULL;
}

PushmsgLogic::~PushmsgLogic()
{
}

bool PushmsgLogic::onConnect(clienthandler_ptr connection)
{
	if (connection)
	{
		LOG_PRINT(log_info, "recv new connection %s:%d connid:%u", connection->getremote_ip(), connection->getremote_port(), connection->getconnid());

	}
	return true;
}

bool PushmsgLogic::onMessage(task_proc_data * message)
{
	COM_MSG_HEADER* in_msg = (COM_MSG_HEADER*) message->pdata;
	if (in_msg->subcmd == Sub_Vchat_ClientHello && in_msg->length == sizeof(COM_MSG_HEADER) + sizeof(CMDGateHello_t))
	{
		message->connection->bSayHello = true;
		CMDGateHello_t* pHelloMsg = (CMDGateHello_t *) in_msg->content;
		if (pHelloMsg->param1 == 12 && pHelloMsg->param2 == 8 && pHelloMsg->param3 == 7)
		{
			if (pHelloMsg->param4 == e_gateway_type)
			{
				//onMessage每增加一个处理的命令字，都要对应命令字添加到hello_response函数内的命令字列表，注册到gateway
				//hello_response(message); 	//注册gateway此服务处理的子命令字

				uint32 connid = message->connection->getconnid();
				int gateid = pHelloMsg->gateid;
				if (gateid)
				{
					m_GateConnMgr.addConn(message->connection);
					LOG_PRINT(log_info, "recv gateway hello %s:%d connect,conn_id=%u, gateid=%u", message->connection->getremote_ip(),
							message->connection->getremote_port(), connid, gateid);
				}

				if (!AppInstance()->m_id)
				{
					register_cmd_msg(message->connection);
				}
			}
		}
		return 0;
	}

	if (Sub_Vchat_ClientPing == in_msg->subcmd)
	{
		if (in_msg->maincmd == MDM_Vchat_WEB && message->connection)
		{
			message->connection->write_message((char *)in_msg, in_msg->length);
		}
		else
		{
			//ping msg from gate
			handle_ping_msg(message);
		}
		return 0;
	}

	switch(in_msg->subcmd)
	{
	case Sub_Vchat_SysNoticeMsgNotifyRecv:
		proc_NoticeMsgNotifyRecv(*message, false);
		break;
	case Sub_VChat_UserNoticeMsgNotifyRecv:
		proc_NoticeMsgNotifyRecv(*message, true);
		break;
	case Sub_Vchat_UnreadNoticeMsgReq:
		proc_unreadNoticeMsgReq(*message);
		break;
	default:
		LOG_PRINT(log_warning, "have not handled such sub cmd!maincmd:%u,subcmd:%u.", in_msg->maincmd, in_msg->subcmd);
		printf("pid:%d have not handled such sub cmd!maincmd:%u,subcmd:%u.", getpid(), in_msg->maincmd, in_msg->subcmd);
		break;
	}

	return true;
}

int PushmsgLogic::hello_response(task_proc_data *message)
{
	COM_MSG_HEADER* in_msg = (COM_MSG_HEADER*)message->pdata;
	char szcmd[128];
	char buf[128] = {0};

	//sprintf(szcmd, "svr_type=%d&cmdlist=10001,10002,10003,10004,10005,10006,10007,10008", e_pushmsg_type);
	if (m_strCmdList.empty())
	{
		sprintf(szcmd, "svr_type=%d&cmdlist=%d,%d,%d", e_pushmsg_type, Sub_Vchat_SysNoticeMsgNotifyRecv, Sub_VChat_UserNoticeMsgNotifyRecv, Sub_Vchat_UnreadNoticeMsgReq);
	}
	else
	{
		sprintf(szcmd, "svr_type=%d&cmdlist=%s", e_pushmsg_type, m_strCmdList.c_str());
	}
	LOG_PRINT(log_info, "%s", szcmd);
	COM_MSG_HEADER* out_msg = (COM_MSG_HEADER*) buf;
	memcpy(out_msg, in_msg, sizeof(COM_MSG_HEADER));
	out_msg->subcmd = Sub_Vchat_ClientHelloResp;
	int* psize = (int*)out_msg->content;
	*psize = strlen(szcmd);
	memcpy(out_msg->content + sizeof(int), szcmd, *psize);
	out_msg->length = SIZE_IVM_HEADER + sizeof(int) + *psize;

	message->connection->write_message(buf, out_msg->length);

	return 0;
}


bool PushmsgLogic::onClose(clienthandler_ptr connection)
{
	if (connection.get())
	{
		m_GateConnMgr.delConn(connection);
		LOG_PRINT(log_info, "connection onClose, %s:%d connid:%u.", connection->getremote_ip(), connection->getremote_port(), connection->getconnid());
		if (connection->user_data)
		{
			//delete [] (char*)connection->user_data;
			//connection->user_data = NULL;
		}
	}
	return true;
}

bool PushmsgLogic::onError(clienthandler_ptr connection, int code, std::string msg)
{
	if (code == boost::asio::error::eof)
	{
	}
	else
	{
	}

	return true;
}
bool PushmsgLogic::loadConfig(Config & config)
{
	//redis配置信息
	m_strRedisHost = config.getString("redis", "ip");
	m_nRedisPort = config.getInt("redis", "port");
	m_sRedisPass = config.getString("redis", "password");
    int redisThreadNum = config.getInt("redis", "threadnum");
    if (redisThreadNum > 0)
    	m_redisThreadNum = redisThreadNum;
    LOG_PRINT(log_debug, "redis config:%s:%d %s %d", m_strRedisHost.c_str(), m_nRedisPort, m_sRedisPass.c_str(), m_redisThreadNum);

    m_strCmdList = config.getString("cmd", "cmdlist");
	return true;
}

bool PushmsgLogic::init(Config & config)
{
	m_pRedisMgr = new redisMgr(m_strRedisHost.c_str(), m_nRedisPort, m_sRedisPass.c_str(), m_redisThreadNum);
    return true;
}

void PushmsgLogic::handle_ping_msg(task_proc_data * message)
{
	if (message && message->connection && message->pdata)
	{
		COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *)message->pdata;

		SL_ByteBuffer outbuf(SIZE_IVM_HEADER + sizeof(CMDClientPingResp_t));

		COM_MSG_HEADER * pmsgheader = (COM_MSG_HEADER *)outbuf.buffer();
		memcpy(pmsgheader, in_msg, SIZE_IVM_HEADER);
		pmsgheader->subcmd = Sub_Vchat_ClientPingResp;

		CMDClientPingResp_t * rsp = (CMDClientPingResp_t *)(pmsgheader->content);
		CMDClientPing_t * req = (CMDClientPing_t *)in_msg->content;
		rsp->userid = req->userid;
		rsp->roomid = req->roomid;

		pmsgheader->length = SIZE_IVM_HEADER + sizeof(CMDClientPingResp_t);
		outbuf.data_end(pmsgheader->length);
		message->connection->write_message(outbuf, true);
	}
}

int32 PushmsgLogic::proc_sendSysNoticeMsg(const ::TNoticeMsg& msg, const std::vector< ::TCondition> & lstCondition, const bool onlineOnly)
{
	int ret = ERR_CODE_SUCCESS;
	unsigned long notice_id = 0;
	uint32 tNow = time(NULL);
	if (!onlineOnly)
	{
		CDBSink sink;
		std::string strSql = stringFormat(
			" insert into mc_notice_msg(type, content, action, create_time, end_time) values(%d, '%s', '%s', FROM_UNIXTIME(%u), %s)", 
			msg.type, 
			msg.content.c_str(), 
			msg.action.c_str(), 
			tNow,
			msg.endTime ? stringFormat("FROM_UNIXTIME(%u)", msg.endTime).c_str() : "DEFAULT");

		do 
		{
			sink.transBegin();
			if (!sink.run_sql(strSql.c_str()))
			{
				ret = ERR_CODE_FAILED_DB;
				break;
			}
			if (notice_id = sink.getInsertId())
			{
				int value_id = 0;
				for (int i = 0; i < lstCondition.size(); i++)
				{
					if (!dealNoticeConditionDB(sink, notice_id, lstCondition[i], value_id))
					{
						ret = ERR_CODE_FAILED_DB;
						break;
					}
				}
				if (ret != ERR_CODE_SUCCESS)
					break;
			}
			else
			{
				ret = ERR_CODE_FAILED_DB;
				break;
			}

		} while (0);
		if (ERR_CODE_SUCCESS == ret)
		{
			sink.transCommit();
		}
		else
		{
			sink.transRollBack();
		}
	}

	if (ERR_CODE_SUCCESS == ret)
	{
		//TODO....
		CMDSysNoticeMsgNotify notify;
		CMDNoticeMsg *pMsg = notify.mutable_msg();
		pMsg->set_msgid(notice_id);
		pMsg->set_type(msg.type);
		pMsg->set_content(msg.content);
		pMsg->set_action(msg.action);
		pMsg->set_time(tNow);
		pMsg->set_endtime(msg.endTime);

		int nProtoLen = notify.ByteSize();
		int nLength = nProtoLen + SIZE_IVM_HEAD_TOTAL;
		SL_ByteBuffer buff(nLength);
		CMsgComm::Build_COM_MSG_HEADER(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_SysNoticeMsgNotify, nLength);
		DEF_IVM_CLIENTGATE(pGateMask, buff.buffer());
		notify.SerializeToArray(buff.buffer()+SIZE_IVM_HEAD_TOTAL, nProtoLen);
		if (lstCondition.empty())
		{
			CMsgComm::Build_BroadCastOnLine_Gate(pGateMask, e_Notice_AllType);
		}
		else if (lstCondition.size() == 1)
		{
			::TCondition cond = lstCondition[0];
			if (SYSMSG_COND_TYPE::USER == cond.type)
			{
				CMsgComm::Build_BroadCastUser_Gate(pGateMask, atoi(cond.value.c_str()));
			}
		}
		m_GateConnMgr.sendMsg(buff.buffer(), nLength);
	}

	return ret;
}


int32 PushmsgLogic::proc_sendUserNoticeMsg(uint32 userid, const ::TNoticeMsg& msg)
{
	int ret = ERR_CODE_SUCCESS;
	time_t tNow = time(0);
	std::string strSql;
	strSql = stringFormat(" insert into mc_user_notice_msg(user_id, type, content, action, create_time) values(%u, %d, '%s', '%s', FROM_UNIXTIME(%u)) ", userid, msg.type, msg.content.c_str(), msg.action.c_str(), tNow);

	CDBSink sink;
	do 
	{
		if (!sink.run_sql(strSql.c_str()) || sink.getAffectedRow() == 0)
		{
			ret = ERR_CODE_FAILED_DB;
			break;
		}

		unsigned long msgid = sink.getInsertId();
		CMDUserNoticeMsgNotify notify;
		notify.set_userid(userid);
		CMDNoticeMsg *pMsg = notify.mutable_msg();
		pMsg->set_msgid(msgid);
		pMsg->set_type(msg.type);
		pMsg->set_content(msg.content);
		pMsg->set_action(msg.action);
		pMsg->set_time(tNow);
		pMsg->set_endtime(msg.endTime);

		int nProtoLen = notify.ByteSize();
		int nLength = nProtoLen + SIZE_IVM_HEAD_TOTAL;
		SL_ByteBuffer buff(nLength);
		CMsgComm::Build_COM_MSG_HEADER(buff.buffer(), MDM_Vchat_Room, Sub_VChat_UserNoticeMsgNotify, nLength);
		DEF_IVM_CLIENTGATE(pGateMask, buff.buffer());
		CMsgComm::Build_BroadCastUser_Gate(pGateMask, userid);
		notify.SerializeToArray(buff.buffer()+SIZE_IVM_HEAD_TOTAL, nProtoLen);
		m_GateConnMgr.sendMsg(buff.buffer(), nLength);
	} while (0);

	return ret;		
}

void PushmsgLogic::proc_handleAuthState(uint32 userid, uint32 groupid, uint32 authState)
{
	if (!userid || !groupid)
	{
		return;
	}

	CMDAuthStateNoty notify;
	notify.set_userid(userid);
	notify.set_groupid(groupid);
	notify.set_authstate(authState);

	int nProtoLen = notify.ByteSize();
	int nLength = nProtoLen + SIZE_IVM_HEAD_TOTAL;

	SL_ByteBuffer buff(nLength);
	CMsgComm::Build_COM_MSG_HEADER(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_UserAuthStateNoty, nLength);
	DEF_IVM_CLIENTGATE(pGateMask, buff.buffer());
	notify.SerializeToArray(buff.buffer() + SIZE_IVM_HEAD_TOTAL, nProtoLen);
	CMsgComm::Build_BroadCastRoomGate(pGateMask, e_Notice_AllType, groupid);	
	m_GateConnMgr.sendMsg(buff.buffer(), nLength);
}

int32 PushmsgLogic::getBroadCastType(const std::vector< ::TCondition> & lstCondition)
{
	LOG_PRINT(log_info, "condition size: %u", lstCondition.size());
	if (lstCondition.size() > 2)
		return -1;

	bool toUser = false, toTel = false, toOS = false, toAppVersion = false;
	std::stringstream strStream;
	for (int i = 0; i < lstCondition.size(); i++)
	{
		const ::TCondition &cond = lstCondition[i];
		cond.printTo(strStream);
		LOG_PRINT(log_info, "condition[%d]: {%s}.", i, strStream.str().c_str());

		switch (lstCondition[i].type)
		{
		case e_NoticeUser:
			toUser = true;	
			break;
		case e_NoticeTel:
			toTel = true;
			if (cond.oper != "=" && cond.oper != "in")
				return -1;
			break;
		case e_NoticeOS:
			toOS = true;
			break;
		case e_NoticeAppVersion:
			toAppVersion = true;
			break;
		default:
			return -1;
		}

	}

	//TODO......
	return 0;
}

bool PushmsgLogic::dealNoticeConditionDB(CDBSink &sink, uint32 notice_id, ::TCondition condition, int &value_id)
{
	std::stringstream sql;
	std::string strSql;
	if ("in" == condition.oper || "IN" == condition.oper)
	{
		if (condition.values.empty())
			return true;

		strSql = stringFormat(
			"insert into mc_notice_condition(notice_id, condition_type, condition_oper, condition_value) values(%u, %d, '%s', '%d') ",
			notice_id, condition.type, condition.oper.c_str(), value_id);

		if (!sink.run_sql(strSql.c_str()))
			return false;

		std::stringstream sqlValues;
		sql << "insert into mc_notice_condition_value(notice_id, value_id, value) values";
		for (std::set<std::string>::iterator iter = condition.values.begin(); iter != condition.values.end(); iter++)
		{
			if (iter != condition.values.begin())
			{
				sqlValues << ',';
			}
			sqlValues << "(" << notice_id << ',' << value_id << ",'" << iter->c_str() << "')";
		}
		sql << sqlValues.str();
		if (!sink.run_sql(sql.str().c_str()))
			return false;
		value_id++;
	}
	else
	{
		strSql = stringFormat(
			"insert into mc_notice_condition(notice_id, condition_type, condition_oper, condition_value) values(%u, %d, '%s', '%s') ",
			notice_id, condition.type, condition.oper.c_str(), condition.value.c_str());

			if (!sink.run_sql(strSql.c_str()))
				return false;
	}

	return true;
}


void PushmsgLogic::proc_NoticeMsgNotifyRecv(task_proc_data &task_data, bool isUser/* = false*/)
{
	CMDNoticeMsgNotifyRecv recv;
	if (!CMsgComm::ParseProtoMessage(task_data.pdata, task_data.datalen, recv))
	{
		task_data.resperrinf(ERR_CODE_FAILED_PACKAGEERROR);
		LOG_PRINT(log_error, "parse %s error, isUser[%d].", recv.GetTypeName().c_str(), isUser);
		return;
	}

	LOG_PRINT(log_info, "user[%u] has recv msgid[%u], isUser[%d]", recv.userid(), recv.msgid(), isUser)
	if (!recv.userid() || !recv.msgid())
	{
		task_data.resperrinf(ERR_CODE_INVALID_PARAMETER);
		return;
	}

	CDBSink().updReadNoticeMsgIdOfUser(recv.userid(), recv.msgid(), isUser);
}

void PushmsgLogic::proc_unreadNoticeMsgReq(task_proc_data &task_data)
{
	CMDUnreadNoticeMsgReq req;
	if (!CMsgComm::ParseProtoMessage(task_data.pdata, task_data.datalen, req))
	{
		task_data.resperrinf(ERR_CODE_FAILED_PACKAGEERROR);
		LOG_PRINT(log_error, "parse %s error.", req.GetTypeName().c_str());
		return;
	}

	if (!req.userid())
	{
		task_data.resperrinf(ERR_CODE_INVALID_PARAMETER);
		return;
	}
	CDBSink sink;
	std::vector<StNoticeMsg_t> vList;
	if (sink.getUnreadSysNoticeMsgList(req.userid(), vList) > 0)
	{
		CMDUnreadSysNoticeMsgNotify notify;
		for (int i = 0; i < vList.size(); i++)
		{
			if (sink.chkUnreadSysNoticeMsgCondition(vList[i].msgid, req.userid(), "", req.osplatform(), req.appversion()))
			{
				CMDNoticeMsg *pMsg = notify.add_msgs();
				pMsg->set_msgid(vList[i].msgid);
				pMsg->set_type(vList[i].msgType);
				pMsg->set_content(vList[i].content);
				pMsg->set_action(vList[i].action);
				pMsg->set_time(vList[i].createTime);
				pMsg->set_endtime(vList[i].endTime);
			}
		}
		if (notify.msgs_size() > 0)
		{
			task_data.respProtobuf(notify, Sub_Vchat_UnreadSysNoticeMsgNotify);
			LOG_PRINT(log_info, "SysNoticeMsgNotify to user[%u], size[%u].", req.userid(), notify.msgs_size());
		}

		vList.clear();
	}
	
	if (sink.getUnreadUserNoticeMsgList(req.userid(), vList) > 0)
	{
		CMDUnreadUserNoticeMsgNotify notify;
		notify.set_userid(req.userid());
		for (int i = 0; i < vList.size(); i++)
		{
			CMDNoticeMsg *pMsg = notify.add_msgs();
			pMsg->set_msgid(vList[i].msgid);
			pMsg->set_type(vList[i].msgType);
			pMsg->set_content(vList[i].content);
			pMsg->set_action(vList[i].action);
			pMsg->set_time(vList[i].createTime);
		}

		if (notify.msgs_size() > 0)
		{
			task_data.respProtobuf(notify, Sub_VChat_UnreadUserNoticeMsgNotify);
			LOG_PRINT(log_info, "UserNoticeMsgNotify to user[%u], size[%u].", req.userid(), notify.msgs_size());
		}

	}
}

void PushmsgLogic::register_cmd_msg(clienthandler_ptr client)
{
	std::string cmdlist = AppInstance()->m_config.getString("cmd", "cmdlist");

	std::list<std::string> cmdrange_lst;
	cmdrange_lst.clear();

	int i = 1;
	while (1)
	{
		char cmdrange[LEN128] = { 0 };
		sprintf(cmdrange, "cmdrange_%d", i);
		std::string strcmdrange = AppInstance()->m_config.getString("cmd", cmdrange);
		if (strcmdrange.empty())
		{
			break;
		}

		cmdrange_lst.push_back(std::string(cmdrange) + "=" + strcmdrange);
		++i;
	}

	if (cmdrange_lst.empty() && cmdlist.empty())
	{
		LOG_PRINT(log_info, "[Register cmd]this server has no cmd to register.");
		return;
	}

	char szBuf[512] = { 0 };
	int msglen = CMsgComm::Build_RegisterCmd_Msg(szBuf, sizeof(szBuf), e_roomadapter_type, cmdlist, cmdrange_lst);
	if (msglen > 0)
	{
		client->write_message(szBuf, msglen);
	}
}
