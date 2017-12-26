#include "GateEvent.h"
#include <boost/bind.hpp>
#include "ChatServerApp.h"
#include "msgcommapi.h"
#include "errcode.h"
#include "GroupAssistLogic.h"

CGateEvent::CGateEvent()
{
}

CGateEvent::~CGateEvent()
{
}

bool CGateEvent::onConnect(clienthandler_ptr client_conn)
{
	if (!client_conn)
	{
		LOG_PRINT(log_error, "onConnect fail,connection is null.");
		return false;
	}

	LOG_PRINT(log_info, "connection onConnect,client connid:%u.", client_conn->getconnid());
	int *pTimeId = new int;
	*pTimeId = Timer::getInstance()->add(en_checkactivetime, boost::bind(&CGateEvent::onTimer, this, client_conn), TIMES_UNLIMIT);
	client_conn->user_data = pTimeId;
	return true;
}

bool CGateEvent::onMessage(task_proc_data* message)
{
	if (!message)
	{
		LOG_PRINT(log_error, "onMessage failed, message is null.");
		return false;
	}

	clienthandler_ptr client = message->connection;
	if (!client)
	{
		LOG_PRINT(log_error, "onMessage failed, message->connection is null.");
		return true;
	}

	if (NULL == message->pdata || message->datalen == 0)
		return false;

	COM_MSG_HEADER* in_msg = (COM_MSG_HEADER*)message->pdata;
	if (in_msg->version != MDM_Version_Value)
	{
		LOG_PRINT(log_error, "Packet version checks faild! connid:%u,[%s:%d]", client->getconnid(), client->getremote_ip(), client->getremote_port());
		return false;
	}

	if (in_msg->length != message->datalen)
	{
		LOG_PRINT(log_error, "Packet length checks faild! connid:%u,[%s:%d]", client->getconnid(), client->getremote_ip(), client->getremote_port());
		return false;
	}

	//�ȴ���hello��Ϣ
	if (in_msg->subcmd == Sub_Vchat_ClientHello)
	{
		CMDGateHello_t* pHelloMsg = (CMDGateHello_t *)in_msg->content;
		if (pHelloMsg->param1 == 12 &&
			pHelloMsg->param2 == 8 &&
			pHelloMsg->param3 == 7)
		{
			client->bSayHello = true;
			unsigned int connid = client->getconnid();
			int gateid = pHelloMsg->gateid;
			client->setgateid(gateid);
			LOG_PRINT(log_info, "recv hello message,client:%s:%u,client connid:%u,gateid:%u,svr_type:%d.", client->getremote_ip(), client->getremote_port(), connid, gateid, (int)pHelloMsg->param4);

			if (e_gateway_type == pHelloMsg->param4)
			{
				if (gateid)
				{
					AppInstance()->m_GateConnMgr.addConn(client);
					LOG_PRINT(log_info, "save gate conn to memory,client:%s:%u,client connid:%u,gateid:%u", client->getremote_ip(), client->getremote_port(), connid, gateid);
				}

				if (!AppInstance()->m_id)
				{
					register_cmd_msg(client);
				}
			}

			if (e_bridgesvr_type == pHelloMsg->param4)
			{
				LOG_PRINT(log_info, "recv bridgesvr hello message,client:%s:%u,client connid:%u,gateid:%u", client->getremote_ip(), client->getremote_port(), connid, gateid);
				//m_RoomManager->m_bridgeMgr.insert(client);
			}

		}
		return true;
	}

	//֮��hello����ǺϷ�����Ϣ
	if (false == client->bSayHello)
	{
		LOG_PRINT(log_error, "[clientobject=%llu, connid=%llu, connid=%u] need client hello message.",
			(uint64)client.get(), client->getconnid(), client->getconnid());
		return true;
	}

	client->lastalivetime = time(NULL);

	if (in_msg->subcmd == Sub_Vchat_ClientPing)
	{
		handle_ping_msg(message);
		return true;
	}

	dispatchMessage(message);

	return true;
}

bool CGateEvent::onClose(clienthandler_ptr connection)
{
	if (connection)
	{
		AppInstance()->m_GateConnMgr.delConn(connection);
		//m_RoomManager->m_bridgeMgr.erase(connection);
		LOG_PRINT(log_info, "connection onClose,client connid:%u.", connection->getconnid());

		if (connection->user_data)
		{
			Timer::getInstance()->del(*((int*)connection->user_data));
			delete (int *)connection->user_data;
			connection->user_data = NULL;
		}
	}
	return true;
}

bool CGateEvent::onError(clienthandler_ptr connection, int code, std::string msg)
{
	return true;
}

void CGateEvent::dispatchMessage(task_proc_data* task_data)
{
	COM_MSG_HEADER* in_msg = (COM_MSG_HEADER*)task_data->pdata;

	switch (in_msg->subcmd)
	{
	case Sub_Vchat_PrivateMsgReq:
		CChatLogic::procPrivateMsgReq(*task_data);
		break;
	case Sub_Vchat_PrivateMsgNotifyRecv:
		CChatLogic::procPrivateMsgNotifyRecv(*task_data);
		break;
	case Sub_Vchat_UnreadPrivateMsgReq:
		CChatLogic::procUnreadPrivateMsgReq(*task_data);
		break;
	case Sub_Vchat_PrivateMsgHisReq:
		CChatLogic::procPrivateMsgHisReq(*task_data);
		break;
	case Sub_Vchat_GroupMsgReq:
		{
			unsigned int ntime_begin = 0, ntime_end = 0;
			LOG_PRINT(log_info, "++++++ Sub_Vchat_GroupMsgReq.");
			ntime_begin = SL_Socket_CommonAPI::util_process_clock_ms();
			CChatLogic::procGroupMsgReq(*task_data);
			ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();
			LOG_PRINT(log_info, "------ Sub_Vchat_GroupMsgReq ms = %d.", ntime_end - ntime_begin);
		}
		break;
	case Sub_Vchat_GroupMsgNotifyRecv:
		CChatLogic::procGroupMsgNotifyRecv(*task_data);
		break;
	case Sub_Vchat_UnreadGroupMsgReq:
		CChatLogic::procUnreadGroupMsgReq(*task_data);
		break;
	case Sub_Vchat_GroupMsgHisReq:
		{
			unsigned int ntime_begin = 0, ntime_end = 0;
			LOG_PRINT(log_info, "++++++ Sub_Vchat_GroupMsgHisReq.");
			ntime_begin = SL_Socket_CommonAPI::util_process_clock_ms();
			CChatLogic::procGroupMsgHisReq(*task_data);
			ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();
			LOG_PRINT(log_info, "------ Sub_Vchat_GroupMsgHisReq ms = %d.", ntime_end - ntime_begin);
		}
		break;
	case Sub_Vchat_GroupPrivateMsgReq:
		{
			unsigned int ntime_begin = 0, ntime_end = 0;
			LOG_PRINT(log_info, "++++++ Sub_Vchat_GroupPrivateMsgReq.");
			ntime_begin = SL_Socket_CommonAPI::util_process_clock_ms();
			CChatLogic::procGroupPMsgReq(*task_data);
			ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();
			LOG_PRINT(log_info, "------ Sub_Vchat_GroupPrivateMsgReq ms = %d.", ntime_end - ntime_begin);
		}
		break;
	case Sub_Vchat_GroupPrivateMsgNotifyRecv:
		CChatLogic::procGroupPMsgNotifyRecv(*task_data);
		break;
	case Sub_Vchat_GroupPrivateMsgHisReq:
		{
			unsigned int ntime_begin = 0, ntime_end = 0;
			LOG_PRINT(log_info, "++++++ Sub_Vchat_GroupPrivateMsgHisReq.");
			ntime_begin = SL_Socket_CommonAPI::util_process_clock_ms();
			CChatLogic::procGroupPMsgHisReq(*task_data);
			ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();
			LOG_PRINT(log_info, "------ Sub_Vchat_GroupPrivateMsgHisReq ms = %d.", ntime_end - ntime_begin);
		}
		break;
	case Sub_Vchat_GroupOnlookerChatReq:
		CChatLogic::procGroupOnlookerChatReq(*task_data);
		break;
	case Sub_Vchat_DeleteChatMsgHis:
		CChatLogic::procDeleteChatMsgHis(*task_data);
		break;
//	case Sub_Vchat_GroupPrivatePermissionReq:
//		CChatLogic::procGroupPrivatePermissionReq(*task_data);
//		break;
//	case Sub_Vchat_GroupPChatPermissionReq:
//		CChatLogic::procGroupPChatPermissionReq(*task_data);
//		break;
//	case Sub_Vchat_ModifyGroupPChatPermissionReq:
//		CChatLogic::procModifyGroupPChatPermissionReq(*task_data);
//		break;
	case Sub_Vchat_UnreadGPAssistMsgReq:
		CGroupAssistLogic::procUnreadMsgReq(*task_data);
		break;
	case Sub_Vchat_GPAssistMsgNotifyRecv:
		CGroupAssistLogic::procMsgNotifyRecv(*task_data);
		break;
	case Sub_Vchat_GPAssistMsgStateReq:
		CGroupAssistLogic::procSyncTaskStatusReq(*task_data);
		break;
	case Sub_Vchat_GPAssistMsgHisReq:
		CGroupAssistLogic::procAssistMsgHisReq(*task_data);
		break;
	case Sub_Vchat_GroupPChatQualiticationReq:
		CChatLogic::procGroupPChatQualiticationReq(*task_data);
		break;
	case Sub_Vchat_GroupInvestUserReq:
		CGroupAssistLogic::procInvestMsgReq(*task_data);
		break;
	default:
		{
			ClientGateMask_t* pGateMask = (ClientGateMask_t*)in_msg->content;
			LOG_PRINT(log_warning, "unknown request subcmd:%d", in_msg->subcmd);
			task_data->connection->resperrinf(in_msg, pGateMask, ERR_CODE_FAILED_UNKNONMESSAGETYPE);
			break;
		}
	}
}

void CGateEvent::onTimer(clienthandler_ptr connection)
{
	if (connection)
	{
		unsigned int msg_size = connection->getsendqueue_size();
		if (msg_size > AppInstance()->m_uAlarmQueueSize)
		{
			LOG_PRINT(log_info, "[message_size]client-session queue-size is %u larger than %u,close it.[userid:%u,client connid:%u,%s:%u]", 
				msg_size, AppInstance()->m_uAlarmQueueSize, connection->user_id, connection->getconnid(), connection->getremote_ip(), connection->getremote_port());
			connection->stop();
			return;
		}
	}
}

void CGateEvent::register_cmd_msg(clienthandler_ptr client)
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

void CGateEvent::handle_ping_msg(task_proc_data * task_node)
{
	if (task_node && task_node->connection && task_node->pdata)
	{
		COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *)task_node->pdata;

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
		task_node->connection->write_message(outbuf, true);

		LOG_PRINT(log_info, "send Sub_Vchat_ClientPingResp to gate");

	}
}
