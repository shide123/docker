#include "GatewayApp.h"
#include <stdio.h>
#include "clienthandler.h"
#include "CLogThread.h"
#include "CClientItem.h"
#include "CClientLogic.h"
#include "message_vchat.h"
#include "timer.h"
#include "utils.h"
#include "boost/bind.hpp"

GatewayApp::GatewayApp()
{
}

GatewayApp::~GatewayApp()
{
	if (m_pCmdConfigMgr)
	{
		delete m_pCmdConfigMgr;
		m_pCmdConfigMgr = NULL;
	}

	unInitNode();
}

bool GatewayApp::onWorkerStart(Application * worker)
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

bool GatewayApp::onWorkerStop(Application * worker)
{
	LOG_PRINT(log_info, "onWorkerStop worker id:%d.", worker->m_id);
	return true;
}

bool GatewayApp::onConnect(clienthandler_ptr client_conn)
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
		item->m_timeid = Timer::getInstance()->add(en_checkactivetime, boost::bind(&GatewayApp::onTimer, this, client_conn), TIMES_UNLIMIT);
	}

	return true;
}

bool GatewayApp::onMessage(task_proc_data * message)
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
			CClientLogic event(message->connection);
			return event.handle_message(message->pdata, message->datalen);
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

bool GatewayApp::onClose(clienthandler_ptr connection)
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

GatewayApp * GatewayApp::getInstance()
{
	return (GatewayApp *)Application::getInstance();
}

void GatewayApp::loadSelfConfig()
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
	m_pCmdConfigMgr = new CCmdConfigMgr(getPath(PATH_TYPE_CONFIG), getProcName());
	m_pCmdConfigMgr->loadCmdCheckConfig();
}

bool GatewayApp::onError(clienthandler_ptr connection, int code, std::string msg)
{
	if (!connection || !connection->user_data || !connection->isconnected())
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

void GatewayApp::onTimer(clienthandler_ptr connection)
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
			LOG_PRINT(log_warning, "client has not sent packet for %u seconds,close it.[userid:%u,roomid:%u,client connid:%u,%s:%u,"
				"now_time:%u,last_active_time:%u,actual timeout:%u]",\
				m_uClientTimeout, connection->user_id, connection->room_id, connection->getconnid(), connection->getremote_ip(), connection->getremote_port(),
				now_time, pClientitem->m_last_active_time, now_time - pClientitem->m_last_active_time);
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
