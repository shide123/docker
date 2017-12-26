#include <assert.h>
#include <string>
#include <sys/wait.h>
#include "ProtocolsBase.h"
#include "Application.h"
#include "Frame.h"
#include "Websocket.h"
#include "DCL.h"
#include "Http.h"
#include "CryptoHandler.h"
using namespace std;
ProtocolsBase* ProtocolsBase::app = 0;
ProtocolsBase::ProtocolsBase(void)
{
	app = this;
	parse_message = NULL;
}


ProtocolsBase::~ProtocolsBase(void)
{
	//
}

ProtocolsBase* ProtocolsBase::getProtocolByName(string protocolname)
{
	transform(protocolname.begin(), protocolname.end(), protocolname.begin(), ::tolower);
	ProtocolsBase* protocol = 0;
	if(protocolname=="websocket")
	{
		protocol = new Websocket();
	}
	else if(protocolname=="frame")
	{
		protocol = new Frame();
	}
	else if(protocolname=="dcl")
	{
		protocol = new DCL();
	}
	else if(protocolname=="http")
	{
		protocol = new Http();
	}
	else
	{
		protocol = new ProtocolsBase();
	}
	return  protocol;
}

ProtocolsBase* ProtocolsBase::getInstance()
{
	//static Application m_netMonitorInstance;
	assert(app);
	return app;
}

int ProtocolsBase::handle_message(const char* pdata, int msglen,clienthandler_ptr conn)
{
	if (NULL == pdata || msglen == 0)
	        return 0;

	const char *pMessage = pdata;
	int nMsgLen = msglen;
	SL_ByteBuffer buff;
	if (parse_message)
	{
		if (parse_message(pdata, msglen, buff))
		{
			pMessage = buff.buffer();
			nMsgLen = buff.buffer_size();
		}
		else
		{
			return -1;
		}
	}

	if(conn->m_bAuthPass)
	{
		task_proc_data* task_data = new task_proc_data;
		task_data->msgtye=TASK_MSGTYPE_NETMSG;
		task_data->pdata =new char[nMsgLen+1];
		memset(task_data->pdata,0,nMsgLen+1);
		task_data->datalen =nMsgLen;
		memcpy(task_data->pdata, pMessage, nMsgLen);
		task_data->connection =conn;
		Application::getInstance()->add_message(task_data);
	}else
	{
		COM_MSG_HEADER* head = (COM_MSG_HEADER *)pMessage;
		if(head->maincmd == MDM_Vchat_Login)
		{
			switch(head->subcmd)
			{
				case Sub_Vchat_Auth_Resp:
				{
					string transkey = "";
					COM_MSG_AUTH_RESP* auth = (COM_MSG_AUTH_RESP*)head->content;
					conn->transKey(auth->ckey,transkey);
					LOG_PRINT(log_debug, "auth->ckey:%s,transkey:%s!  connid:%u,%s:%u\n",auth->ckey,transkey.c_str(), conn->getconnid(), conn->getremote_ip(), conn->getremote_port());
					conn->transKey(conn->m_skey,conn->m_transkey);
					LOG_PRINT(log_debug, "conn->skey:%s,transkey:%s!  connid:%u,%s:%u\n",conn->m_skey.c_str(),conn->m_transkey.c_str(), conn->getconnid(), conn->getremote_ip(), conn->getremote_port());
					string key = transkey + conn->m_transkey;
					LOG_PRINT(log_debug, "key:%s!  connid:%u,%s:%u\n",key.c_str(), conn->getconnid(), conn->getremote_ip(), conn->getremote_port());
					string tkey = "";
					cryptohandler::md5hash(key,tkey);
					std::transform(tkey.begin(),tkey.end(),tkey.begin(),::tolower);
					LOG_PRINT(log_debug, "md5:%s,content:%s connid:%u,%s:%u\n",tkey.c_str(),auth->content, conn->getconnid(), conn->getremote_ip(), conn->getremote_port());

					if(tkey == auth->content)
					{
						conn->m_bAuthPass = true;
						conn->sendAuthPassmsg();
						LOG_PRINT(log_debug, "auth pass! connid:%u,%s:%u\n", conn->getconnid(), conn->getremote_ip(), conn->getremote_port());
						return 1;
					}
				}
			}
		}
		LOG_PRINT(log_error, "auth fail! connid:%u,%s:%u.maincmd:%u,subcmd:%u.", conn->getconnid(), conn->getremote_ip(), conn->getremote_port(), head->maincmd, head->subcmd);
		conn->release();
	}
	//remain in queue,handle by circle
//	if(m_workerEvent)
//		m_workerEvent->onMessage(task_data);
//	delete[] task_data->pdata;
	return 0;
}

void ProtocolsBase::setParseFunc(parse_msg_func func)
{
	parse_message = func;
}

int ProtocolsBase::decode(char* recv_buf_, int &recv_buf_remainlen_,clienthandler_ptr conn)
{
	char* p = recv_buf_;
	handle_message(p, recv_buf_remainlen_,conn);
	recv_buf_remainlen_ -= recv_buf_remainlen_;
	p += recv_buf_remainlen_;
	if(p != recv_buf_ && recv_buf_remainlen_ > 0){
		memmove(recv_buf_, p, recv_buf_remainlen_);
	}
	return 0;
}
void ProtocolsBase::encode(char* data, int len,SL_ByteBuffer& message)
{
	message.write(data, len);
}

void ProtocolsBase::sendAuthMsgIfNeed(clienthandler_ptr conn)
{
	if(Application::getInstance()->m_bAuth)
		conn->sendAuthmsg();
}

