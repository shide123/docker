/*
 * CSvrEvent.cpp
 *
 *  Created on: Aug 22, 2016
 *      Author: testtest
 */

#include "LogonEvent.h"
#include "clienthandler.h"
#include "DBConnection.h"
#include "errcode.h"
#include "GlobalSetting.h"
#include "msgcommapi.h"
#include "LogonSvrApp.h"

LogonEvent::LogonEvent(unsigned int work_id)
{
	m_cmdrange_lst.clear();
	m_cmdlist = "";
	m_id = work_id;
}

LogonEvent::~LogonEvent()
{
}

bool LogonEvent::onConnect(clienthandler_ptr connection)
{
    if (connection)
    {
        LOG_PRINT(log_info, "recv new connection %s:%d", connection->getremote_ip(), connection->getremote_port());
    }
    return true;
}

bool LogonEvent::onMessage(task_proc_data * message)
{
	COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *) message->pdata;

    ulong ntime_begin = 0;
    ulong ntime_end = 0;
    if(in_msg->subcmd == Sub_Vchat_ClientHello)
    {
        CMDClientHello_t * pHelloMsg = (CMDClientHello_t *)(in_msg->content);
        if(pHelloMsg->param1 == 12 && pHelloMsg->param2 == 8 && pHelloMsg->param3 == 7)
        {
            if (!message->connection->bSayHello)
			{
				if (e_gateway_type == pHelloMsg->param4)
				{
					LOG_PRINT(log_info, "[connid=%u,remote=%s:%d]recv gateway hello message", message->connection->getconnid(), message->connection->getremote_ip(), message->connection->getremote_port());
					
					if (!m_id)
					{
						register_cmd_msg(message->connection);
					}
				}
				else
				{
					LOG_PRINT(log_warning, "recv hello from unknown server, conn_id=%u,remote=%s:%d", message->connection->getconnid(), message->connection->getremote_ip(), message->connection->getremote_port());
					return false;
				}
            }
            message->connection->bSayHello = true;
        }
        return true;
    }

    if(!message->connection->bSayHello)
    {
        LOG_PRINT(log_error, "[clientobject=%llu,connid=%llu]this client connect has not sent hello message", (uint64)message->connection.get(), message->connection->getconnid());
        return true;
    }

    if (in_msg->subcmd == Sub_Vchat_ClientPing)
    {
		handle_ping_msg(message);
        return true;
    }
	
    switch(in_msg->subcmd)
    {
    case Sub_Vchat_logonReq5:
        {
            ntime_begin = SL_Socket_CommonAPI::util_process_clock_ms();

            CGlobalSetting::m_app->m_logon_mgr.proc_logonreq5(message);

            ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();
            LOG_PRINT(log_info, "[cost]proc_logonreq5 ms = %u.", ntime_end - ntime_begin);
        }
        break;
    case Sub_Vchat_logonTokenReq:
        {
            CGlobalSetting::m_app->m_logon_mgr.proc_synusertoken_req(message);
        }
        break;
	case Sub_Vchat_ClientCloseSocket_Req:
		{
			//the client close socket
			CGlobalSetting::m_app->m_logon_mgr.proc_client_closeSocket_req(message);
		}
		break;
	case Sub_Vchat_UserExitMessage_Req:
		{
			CGlobalSetting::m_app->m_logon_mgr.proc_client_getExitSoftMessage_req(message);
		}
		break;
    default:
        {
            LOG_PRINT(log_error, "unknown cmd!maincmd:%u,subcmd:%u.", in_msg->maincmd, in_msg->subcmd);
        }
        break;
    }

	return true;
}

void LogonEvent::register_cmd_msg(clienthandler_ptr client)
{
	if (m_cmdrange_lst.empty() && m_cmdlist.empty())
	{
		return;
	}

	char szBuf[512] = {0};
	int msglen = CMsgComm::Build_RegisterCmd_Msg(szBuf, sizeof(szBuf), e_logonsvr_type, m_cmdlist, m_cmdrange_lst);
	if (msglen > 0)
	{
		client->write_message(szBuf, msglen);
	}
}

bool LogonEvent::onClose(clienthandler_ptr connection)
{
	if (connection.get())
	{
		LOG_PRINT(log_info, "connection onClose, %s:%d connid:%u.", connection->getremote_ip(), connection->getremote_port(), connection->getconnid());
		if (connection->user_data)
		{
			delete connection->user_data;
			connection->user_data = NULL;
		}
	}
	return true;
}

bool LogonEvent::onError(clienthandler_ptr connection, int code, std::string msg)
{
	if (code == boost::asio::error::eof)
	{
		//notify_svr_clientexit();
	}
	else
	{
		//notify_svr_exceptexit();
	}

	return true;
}
bool LogonEvent::onTimer()
{
	return true;
}

bool LogonEvent::loadConfig()
{
	return true;
}

bool LogonEvent::init()
{
	std::string strAppName = CLogonSvrApp::getInstance()->getProcName();
	std::string strConfigPath = CLogonSvrApp::getInstance()->getPath(PATH_TYPE_CONFIG);
	std::string configfile = strConfigPath + strAppName + ".conf";

	Config config;
	if(!config.loadfile(configfile))
	{
		LOG_PRINT(log_error, "load config file failed.file:%s.", configfile.c_str());
		return false;
	}

	m_cmdlist = config.getString("cmd", "cmdlist");

	m_cmdrange_lst.clear();
	int i = 1;
	while (1)
	{
		char cmdrange[LEN128] = {0};
		sprintf(cmdrange, "cmdrange_%d", i);
		std::string strcmdrange = config.getString("cmd", cmdrange);
		if (strcmdrange.empty())
		{
			break;
		}

		m_cmdrange_lst.push_back(std::string(cmdrange) + "=" + strcmdrange);
		++i;
	}

	return true;
}

void LogonEvent::handle_ping_msg(task_proc_data * task_node)
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
	}
}
