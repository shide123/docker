#include "WebgateApp.h"
#include <stdio.h>
#include "clienthandler.h"
#include "CLogThread.h"
#include "CClientItem.h"
#include "CClientLogic.h"
#include "message_vchat.h"
#include "timer.h"
#include "boost/bind.hpp"
#include "JsonCmdConfigMgr.h"

CWebgateApp::CWebgateApp()
{
	CGateBase::setGateType(GATE_TYPE_WEBGATE);
}

CWebgateApp::~CWebgateApp()
{
	if (m_pCmdConfigMgr)
	{
		delete m_pCmdConfigMgr;
		m_pCmdConfigMgr = NULL;
	}

	unInitNode();
}

bool CWebgateApp::onWorkerStart(Application* worker)
{
	LOG_PRINT(log_info, "onWorkerStart worker id:%d.", worker->m_id);
	
	loadSelfConfig();

	CGateBase::m_ClientMgr.init();

	m_usGateId = m_config.getInt("self", "gateid");
	if (!m_usGateId)
	{
		LOG_PRINT(log_error, "start failed!config file has no gateid of worker id:%d.", worker->m_id);
		killAll();
		return false;
	}

	m_usGateId += m_id;

	initNode();
	return true;
}

bool CWebgateApp::onWorkerStop(Application* worker)
{
	LOG_PRINT(log_info, "onWorkerStop worker id:%d.", worker->m_id);
	return true;
}

bool CWebgateApp::onConnect(clienthandler_ptr client_conn)
{
	if (!client_conn)
	{
		LOG_PRINT(log_error, "onConnect fail,connection is null.");
		return false;
	}

	LOG_PRINT(log_info, "connection onConnect,client connid:%u.", client_conn->getconnid());
	if (!client_conn->user_data)
	{
		client_conn->user_data = new CClientItem();
		CClientItem * item = (CClientItem *)client_conn->user_data;
		item->m_mobile = e_WEB_devtype;
		item->m_timeid = Timer::getInstance()->add(en_checkactivetime, boost::bind(&CWebgateApp::onTimer, this, client_conn), TIMES_UNLIMIT);
	}

	client_conn->getProtocol()->setParseFunc(&CWebgateApp::ParseMessage);

	return true;
}

bool CWebgateApp::onMessage(task_proc_data* message)
{
	if (!message)
	{
		LOG_PRINT(log_error, "onMessage fail,message is null.");
		return false;
	}

	if (message->connection)
	{
		if (message->connection->user_data)
		{
			CClientLogic cClientLogic(message->connection);
			return cClientLogic.handle_message(message->pdata, message->datalen);
		}
		else
		{
			LOG_PRINT(log_error, "client connect user data is null.client connid:%u,%s:%u.", message->connection->getconnid(), message->connection->getremote_ip(), message->connection->getremote_port());
			return true;
		}
	}
	else
	{
		LOG_PRINT(log_error, "onMessage fail,message connection is null.");
		return true;
	}
}

bool CWebgateApp::onWrite(clienthandler_ptr connection, const char *pdata, unsigned int datalen, SL_ByteBuffer &outBuff)
{
	if (!connection || NULL == pdata || 0 == datalen)
	{
		return false;
	}
	return (((CJsonCmdConfigMgr *)m_pCmdConfigMgr)->parse_msg_to_json(pdata, datalen, outBuff));
}

bool CWebgateApp::onClose(clienthandler_ptr connection)
{
	if (connection)
	{
		LOG_PRINT(log_info, "connection onClose,client connid:%u.", connection->getconnid());

		//post user logout
		if (connection->user_data)
		{
			CClientItem * pItem = (CClientItem *)connection->user_data;
			CGateBase::m_SvrConnMgr.post_user_logout(connection->user_id, pItem->m_mobile, pItem->m_last_login_time, connection->getconnid(), pItem->m_uuid);

			m_ClientMgr.del_client(connection);

			LOG_PRINT(log_info, "delete timer:%d of client connid:%u.", pItem->m_timeid, connection->getconnid());
			Timer::getInstance()->del(pItem->m_timeid);
			
			delete pItem;
			connection->user_data = NULL;
		}
	}
	return true;
}

CWebgateApp *CWebgateApp::getInstance()
{
	return (CWebgateApp *)Application::getInstance();
}

bool CWebgateApp::ParseMessage(const char *pdata, int len, SL_ByteBuffer &out)
{
	CJsonCmdConfigMgr *pJsonCmdConfigMgr = (CJsonCmdConfigMgr *)getInstance()->m_pCmdConfigMgr;
	return pJsonCmdConfigMgr->parse_json_to_msg(pdata, len, out);
}

void CWebgateApp::loadSelfConfig()
{
	unsigned int nalarmqueuesize = m_config.getInt("self", "alarmqueuesize");
	unsigned int clienttimeout = m_config.getInt("self", "client_timeout");
	unsigned int on_mic_clienttimeout = m_config.getInt("self", "on_mic_client_timeout");
	unsigned int servertimeout = m_config.getInt("self", "server_timeout");

	if(nalarmqueuesize)
	{
		m_uAlarmQueueSize = nalarmqueuesize;
	}

	if (clienttimeout)
	{
		m_uClientTimeout = clienttimeout;
	}

	if (on_mic_clienttimeout)
	{
		m_uOnMicClientTimeout = on_mic_clienttimeout;
	}

	if (servertimeout)
	{
		m_uServerTimeout = servertimeout;
	}

	//load cmd config
	CJsonCmdConfigMgr * pJsonCmdConfigMgr = new CJsonCmdConfigMgr(getPath(PATH_TYPE_CONFIG), getProcName());
	pJsonCmdConfigMgr->loadCmdCheckConfig();
	pJsonCmdConfigMgr->loadCmdParamConfig();
	m_pCmdConfigMgr = (CCmdConfigMgr *) pJsonCmdConfigMgr;
}

bool CWebgateApp::onError(clienthandler_ptr connection, int code, std::string msg)
{
	if (!connection || !connection->user_data)
	{
		return true;
	}

	CClientLogic event(connection);
	if (code == boost::asio::error::eof)
	{
		event.notify_svr_clientexit();
	}
	else
	{
		event.notify_svr_exceptexit();
	}

	return true;
}

void CWebgateApp::onTimer(clienthandler_ptr connection)
{
	if (connection)
	{
		CClientLogic event(connection);
		unsigned int msg_size = connection->getsendqueue_size();
		if (msg_size > m_uAlarmQueueSize)
		{
			LOG_PRINT(log_info, "[message_size]client-session queue-size is %u larger than %u,close it.[userid:%u,client connid:%u,%s:%u]",\
				msg_size, m_uAlarmQueueSize, connection->user_id, connection->getconnid(), connection->getremote_ip(), connection->getremote_port());
			event.notify_svr_clientexit();
			connection->stop();
			return;
		}

		bool timeout = false;
		time_t now_time = time(0);
		CClientItem * pClientitem = (CClientItem *)connection->user_data;
		if(pClientitem && 0 == pClientitem->m_micstate && (now_time - pClientitem->m_last_active_time > m_uClientTimeout))
		{
			LOG_PRINT(log_warning, "client has not sent packet for %u seconds,close it.[userid:%u,roomid:%u,client connid:%u,%s:%u]",\
				m_uClientTimeout, connection->user_id, connection->room_id, connection->getconnid(), connection->getremote_ip(), connection->getremote_port());
			timeout = true;
		}

		if(pClientitem && 1 == pClientitem->m_micstate && (now_time - pClientitem->m_last_active_time > m_uOnMicClientTimeout))
		{
			LOG_PRINT(log_warning, "on mic client has not sent packet for %u seconds,close it.[userid:%u,roomid:%u,client connid:%u,%s:%u]",\
				m_uOnMicClientTimeout, connection->user_id, connection->room_id, connection->getconnid(), connection->getremote_ip(), connection->getremote_port());
			timeout = true;
		}

		if (timeout)
		{
			event.notify_svr_clienttimeout();
			connection->stop();
			return;
		}
	}
}
