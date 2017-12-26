#include "clienthandler.h"
#include "tcpserver.h"
#include "Application.h"
#include "ProtocolsBase.h"
#include "CryptoHandler.h"

clienthandler::clienthandler(boost::asio::io_service & ioservice, std::string protocol, IWorkerEvent * event,tcpserver* svr)
	: socket_(ioservice)
	, deadline_(ioservice)
	, writetimer(ioservice)
	, session_status_(0)
	, svrid(0)
	, lastalivetime(time(0))
	, bSayHello(false)
	, last_activetime_(time(0))
	, disconn_report_times(0)
	, user_id(0)
	, room_id(0)
	, user_data(NULL)
	, m_workerEvent(event)
	, recv_buf_remainlen_(0)
	, connected(false)
	, m_connid(0)
	, remote_port(0)
	, serv(svr)
	, m_ngateid(0)
	, m_swtich(0)
	, m_force_stop(false)
{
	m_protocol = ProtocolsBase::getProtocolByName(protocol);
	m_bAuthPass = !Application::getInstance()->m_bAuth;
}

clienthandler::~clienthandler()
{
	LOG_PRINT(log_debug, "~clienthandler client connid:%u,%s:%u.", m_connid, remote_ip, remote_port);
	if (m_protocol)
	{
		delete m_protocol;
		m_protocol = NULL;
	}
}

void clienthandler::sendAuthmsg()
{
	srand((unsigned)time(NULL));
	int v = rand();
	char rdm[32] = {0};
	sprintf(rdm,"%d",v);
	cryptohandler::md5hash(rdm,m_skey);
	std::transform(m_skey.begin(),m_skey.end(),m_skey.begin(),::tolower);
//	transKey(m_skey,m_transkey);
	COM_MSG_AUTH_REQ req = {};
	strncpy(req.content, m_skey.c_str(), sizeof(req.content) - 1);
	m_skey = req.content;
	char szBuf[1024] = {0};
	int len = build_NetMsg_Client(szBuf,1024,MDM_Vchat_Login, Sub_Vchat_Auth_Req,  &req, sizeof(req));
	write_message(szBuf,len);
	LOG_PRINT(log_debug, "sendauthmsg key[%s] to connid:%u,%s:%u\n", m_skey.c_str(), m_connid, remote_ip, remote_port);
}

void clienthandler::sendAuthPassmsg()
{
	char szBuf[1024] = {0};
	string str = "";
	int len = build_NetMsg_Client(szBuf,1024,MDM_Vchat_Login, Sub_Vchat_Auth_Pass, (void*)str.c_str(), str.length());
	write_message(szBuf,len);
	LOG_PRINT(log_debug, "sendAuthPassmsg to connid:%u,%s:%u\n", m_connid, remote_ip, remote_port);
}

//only s = md5
void clienthandler::transKey(const string& s,string& d)
{
	for(int i  = 0 ; i < s.length() ; i++)
	{
		if(s[i]>=48 && s[i] <= 57)
			d += mask[s[i]-48];
		else if (s[i] >= 97 && s[i] <= 102)
			d += mask[s[i]-87];
	}
}

int clienthandler::open()
{
	writetimer.expires_from_now(boost::posix_time::milliseconds(Application::m_interval_time));
	writetimer.async_wait(boost::bind(&clienthandler::handle_write_loop, shared_from_this(), boost::asio::placeholders::error));
	printf("interval:%d\n", Application::m_interval_time);
	connected = true;
	m_connid = tcpserver::getconnid();
	tcpserver::setconnhandler(m_connid, shared_from_this());
	remote_port = 0;
	bSayHello = false;

	tcp::no_delay option(true);
	socket_.set_option(option);

	char * recv_buffer = recv_buf_ + recv_buf_remainlen_;
	std::size_t rev_buffer_size = en_msgbuffersize - recv_buf_remainlen_;

	if(remote_port == 0)
	{
		SL_Socket_INET_Addr::get_ip_remote_s(socket_.native_handle(), remote_ip, 30, &remote_port);
		LOG_PRINT(log_info, "receive client connid:%u,%s:%u connection", m_connid, remote_ip, remote_port);
	}

	if(m_workerEvent)
	{
		addTaskMessage(TASK_MSGTYPE_LOCAL, MSG_TYPE_SERVER_CONNECT);
	}

	socket_.async_read_some(boost::asio::buffer((void *)recv_buffer, rev_buffer_size),
		boost::bind(&clienthandler::handle_read, shared_from_this(),
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));

	lastalivetime = time(0);
	m_protocol->sendAuthMsgIfNeed(shared_from_this());
    return 0;
}

void clienthandler::handle_read(const boost::system::error_code & e, std::size_t bytes_transferred)
{
	if(!e) {

		if (m_force_stop)
		{
			close();
			LOG_PRINT(log_info, "force_stop is true, do not continue, connid[%u], server[%s:%u]", m_connid, remote_ip, remote_port);
			return;
		}

		if (!connected){
			LOG_PRINT(log_error, "[clientobject:%x,client connid:%u,%s:%u,userid:%u]client is disconnected.", this, m_connid, remote_ip, remote_port, user_id);
			close();
			return;
		}

		recv_buf_remainlen_ += bytes_transferred;

		int ret = m_protocol->decode(recv_buf_, recv_buf_remainlen_, shared_from_this());
		if(-1 == ret)
		{
			LOG_PRINT(log_error, "[clientobject:%x,client connid:%u,remoteip:%s,port:%u,userid:%u]parse message failed!close socket",
					this, m_connid, remote_ip, remote_port, user_id);

			onError(-1, "parse message failed");
			close();
			return;
		}
		else if (-2 == ret)
		{
			LOG_PRINT(log_error, "[clientobject:%x,client connid:%u,remoteip:%s,port:%u,userid:%u]receive close signal!close socket",
				this, m_connid, remote_ip, remote_port, user_id);
			onError(boost::asio::error::eof, "receive socket close signal");
			close();
			return;
		}

		lastalivetime = time(0);

		//continue to receive data.
		char * recv_buffer = recv_buf_ + recv_buf_remainlen_;
		std::size_t rev_buffer_size = en_msgbuffersize - recv_buf_remainlen_;
		socket_.async_read_some(boost::asio::buffer((void *)recv_buffer, rev_buffer_size),
			boost::bind(&clienthandler::handle_read, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}
	else if(e)
	{
		LOG_PRINT(log_error, "[clientobject:%x,client connid:%u,remoteip:%s,port:%u,userid:%u][%s]",
				this, m_connid, remote_ip, remote_port, user_id, boost::system::system_error(e).what());

		if(e != boost::asio::error::try_again && e != boost::asio::error::would_block && e != boost::asio::error::operation_aborted)
		{
			onError(e.value(), boost::system::system_error(e).what());
			close();
		}
	}
}


void clienthandler::release()
{
	socket_.get_io_service().post(boost::bind(&clienthandler::close, shared_from_this()));
}

int clienthandler::build_NetMsg_Svr(char* szBuf,int nBufLen,int mainCmdId, int subCmdId, void* pData, int pDataLen)
{
	if(szBuf==0 || pData==0)
	{
		LOG_PRINT(log_error, "szBuf==0 || pData==0 ");
		return -1;
	}
	int nMsgLen = SIZE_IVM_HEADER + pDataLen;
	if(nBufLen <= nMsgLen)
	{
		LOG_PRINT(log_error, "nBufLen <= nMsgLen ");
		return -1;
	}

	COM_MSG_HEADER* pHead=(COM_MSG_HEADER*)szBuf;
	pHead->version=MDM_Version_Value;
	pHead->checkcode=CHECKCODE;
	pHead->maincmd=mainCmdId;
	pHead->subcmd=subCmdId;
	ClientGateMask_t* pClientGate=(ClientGateMask_t*)(pHead->content);
	memset(pClientGate,0,sizeof(ClientGateMask_t));
	pClientGate->param1 = (uint64)this;
	pClientGate->param2 = (uint64)m_connid;
	void* pContent=(void*)(pHead->content + SIZE_IVM_CLIENTGATE);
	memcpy(pContent, pData, pDataLen);
	pHead->length= SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + pDataLen;

	return pHead->length;
}

int clienthandler::build_NetMsg_Client(char* szBuf,int nBufLen,int mainCmdId, int subCmdId, void* pData, int pDataLen)
{
	if(szBuf==0 || pData==0)
	{
		LOG_PRINT(log_error, "szBuf==0 || pData==0.");
		return -1;
	}
	int nMsgLen = SIZE_IVM_HEADER + pDataLen;
	if(nBufLen <= nMsgLen)
	{
		LOG_PRINT(log_error, "nBufLen <= nMsgLen.");
		return -1;
	}

	COM_MSG_HEADER* pHead = (COM_MSG_HEADER*)szBuf;
	pHead->version=MDM_Version_Value;
	pHead->checkcode=CHECKCODE;
	pHead->maincmd=mainCmdId;
	pHead->subcmd=subCmdId;
	void* pContent=(void*)(pHead->content);
	memcpy(pContent, pData, pDataLen);
	pHead->length= SIZE_IVM_HEADER + pDataLen;

	return pHead->length;
}

time_t clienthandler::getLastalivetime() const
{
	return lastalivetime;
}

void clienthandler::stop()
{
	socket_.get_io_service().post(boost::bind(&clienthandler::cancel, shared_from_this()));
}

void clienthandler::cancel()
{
	m_force_stop = true;
	boost::system::error_code e;
	deadline_.cancel(e);
	if (e)
	{
		LOG_PRINT(log_error, "cancel timer error: %s, server[%s:%u] connid[%u]", boost::system::system_error(e).what(), remote_ip, remote_port, m_connid);
	}
	close();
}

void clienthandler::start_connect(const char *addr, u_int16_t port)
{
	if(session_status_ == en_session_status_ready||session_status_ == en_session_status_connected)
		return;
	m_force_stop = false;
	session_status_ = en_session_status_ready;
	strcpy(remote_ip, addr);
	remote_port = port;

	boost::asio::ip::address addr1 =boost::asio::ip::address_v4::from_string(addr);
	endpoint_.address(addr1);
	endpoint_.port(port);

	LOG_PRINT(log_info, "Begin connect server[%s:%d]...", remote_ip, remote_port);

	deadline_.expires_from_now(boost::posix_time::seconds(en_checkconntimeout));
	deadline_.async_wait(boost::bind(&clienthandler::handle_timeout, shared_from_this(), boost::asio::placeholders::error));

	recv_buf_remainlen_ = 0;
	socket_.async_connect(endpoint_, boost::bind(&clienthandler::handle_connect, shared_from_this(), boost::asio::placeholders::error));

	m_bAuthPass = true;
	printf("start_connect server[%s:%d]\n", remote_ip, remote_port);
}

void clienthandler::handle_connect(const boost::system::error_code& e)
{
	if(!e)
	{
		if (m_force_stop)
		{
			close();
			LOG_PRINT(log_info, "force_stop is true, do not continue, connid[%u], server[%s:%u]", m_connid, remote_ip, remote_port);
			return;
		}
		writetimer.expires_from_now(boost::posix_time::milliseconds(Application::m_interval_time));
		writetimer.async_wait(boost::bind(&clienthandler::handle_write_loop, shared_from_this(), boost::asio::placeholders::error));

		session_status_ = en_session_status_connected;
		m_connid = tcpserver::getconnid();

		clear_data();
		last_activetime_ = time(0);
		disconn_report_times = 0;

		LOG_PRINT(log_info, "connect to server[%s:%u] connid[%u]! queue-size(%d).", remote_ip, remote_port, m_connid, m_send_queue.size());
		boost::asio::ip::tcp::no_delay option(true);
		socket_.set_option(option);

		connected = true;

		if(m_workerEvent)
		{
			addTaskMessage(TASK_MSGTYPE_LOCAL, MSG_TYPE_CLIENT_CONNECT);
		}

		//start recv data...
		char* recv_buffer = recv_buf_+recv_buf_remainlen_;
		std::size_t rev_buffer_size = en_msgbuffersize - recv_buf_remainlen_;

		socket_.async_read_some(boost::asio::buffer((void*)recv_buffer, rev_buffer_size),
			boost::bind(&clienthandler::handle_read, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
		tcpserver::setconnhandler(m_connid, shared_from_this());

		lastalivetime = time(0);
	}
	else
	{
		if (disconn_report_times < 3)
			LOG_PRINT(log_error, "%s, server[%s:%u] connid[%u]", boost::system::system_error(e).what(), remote_ip, remote_port, m_connid);
	}
}
void clienthandler::handle_timeout(const boost::system::error_code& e)
{
	if(!e)
	{
		if (m_force_stop)
		{
			close();
			LOG_PRINT(log_info, "force_stop is true, do not continue, connid[%u], server[%s:%u]", m_connid, remote_ip, remote_port);
			return;
		}

		if(session_status_ == en_session_status_needclose || session_status_ == en_session_status_ready)
		{
			if(!m_swtich)
			{
				if(disconn_report_times == 3)
				{
					if(m_workerEvent)
					{
						onError(1,"connect err");
					}
				}

				if (disconn_report_times < 3)
				{
					CAlarmNotify::sendAlarmNoty(e_all_notitype, e_network_conn, Application::getInstance()->m_strProcName, "Connect Error",\
						"Room,Yunwei", "Disconnect Server[m_id:%d]:[%s:%d]", Application::getInstance()->m_id, remote_ip, remote_port);
				}

				LOG_PRINT(log_error, "server is disconnected,then try connect server[%s:%d] connid[%u].", remote_ip, remote_port, m_connid);

				connect();
			}
		}

		time_t now = time(0);

		if (now - last_activetime_ >= 20){
			last_activetime_ = now;
			LOG_PRINT(log_debug, "server[%s:%d] connid[%u] queue-size:%u .", remote_ip, remote_port, m_connid, m_send_queue.size());
		}

		deadline_.expires_from_now(boost::posix_time::seconds(en_checkconntimeout));
		deadline_.async_wait(boost::bind(&clienthandler::handle_timeout, shared_from_this(), boost::asio::placeholders::error));
	}
	else
	{
		LOG_PRINT(log_error, "%s, server[%s:%u] connid[%u]", boost::system::system_error(e).what(), remote_ip, remote_port, m_connid);
	}
}
int clienthandler::connect()
{
	if (disconn_report_times < 3){
		LOG_PRINT(log_info, "Begin connect server[%s:%d] connid[%u]...", remote_ip, remote_port, m_connid);
		disconn_report_times++;
	}
	
	m_force_stop = false;
	recv_buf_remainlen_ =0;
	socket_.async_connect(endpoint_, boost::bind(&clienthandler::handle_connect, shared_from_this(), boost::asio::placeholders::error));

	return 0;
}
void clienthandler::clear_data()
{
	boost::mutex::scoped_lock lock(send_messages_mutex_);
	int ncount = m_send_queue.size();

	if(ncount > 0)
		m_send_queue.clear();
}

void clienthandler::close()
{
	if (!connected)
	{
		return ;
	}

	LOG_PRINT(log_error, "[clientobject:%x,client connid:%u,remote_ip:%s,remote_port:%u,userid:%u]closing connection",
		this, m_connid, remote_ip, remote_port, user_id);

	connected = false;
	if(m_workerEvent)
	{
		addTaskMessage(TASK_MSGTYPE_LOCAL, MSG_TYPE_CLOSE);
	}

	tcpserver::eraseconnhandler(m_connid);

	if(serv)
	{
		serv->client_release();
		printf("notify_one conid:%d\n",getconnid());
	}

	boost::system::error_code e;
	writetimer.cancel(e);

	{
		boost::mutex::scoped_lock lock(send_messages_mutex_);
		m_send_queue.clear();
	}

	{
		boost::mutex::scoped_lock locker(messagelstmtx);
		m_vbuffer.clear();
	}
	socket_.close(e);

	bSayHello = false;
	session_status_ = en_session_status_needclose;
}

void clienthandler::write_message(SL_ByteBuffer& message, bool bforced)
{
	write_message(message.data(),message.data_size(),  bforced);
}
void clienthandler::write_message(const char* pdata, int datalen, bool bforced)
{
	SL_ByteBuffer realBuff;
	SL_ByteBuffer encodebuffer;
	realBuff.write(pdata,datalen);
	if (!m_workerEvent->onWrite(shared_from_this(), pdata, datalen, realBuff))
	{
		m_protocol->encode((char*)pdata,datalen,encodebuffer);
	}
	else
	{
		if(realBuff.data_end() == 0)
			realBuff.write(pdata,datalen);
		m_protocol->encode(realBuff.buffer(),realBuff.data_end(),encodebuffer);
	}

	if (bforced)
	{
		socket_.get_io_service().post(boost::bind(&clienthandler::send_message, shared_from_this(), encodebuffer, bforced));
	}
	else
	{
		boost::mutex::scoped_lock locker(messagelstmtx);
		m_vbuffer.push_back(encodebuffer);
	}
}
void clienthandler::send_message(SL_ByteBuffer& message, bool bforced)
{
	if(connected || bforced)
	{
		boost::mutex::scoped_lock lock(send_messages_mutex_);

		bool write_in_progress =!m_send_queue.empty();
		m_send_queue.push_back(message);
		if(!write_in_progress && connected) //first async-write data
		{
			SL_ByteBuffer* pslbuf=&(m_send_queue.front());
			boost::asio::async_write(socket_,
				boost::asio::buffer(pslbuf->buffer(), pslbuf->data_end()),
				boost::bind(&clienthandler::handle_write, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		}
	}
}
void clienthandler::handle_write(const boost::system::error_code& e, std::size_t bytes_transferred)
{
	if(!e)
	{
		if (m_force_stop)
		{
			close();
			LOG_PRINT(log_info, "force_stop is true, do not continue, connid[%u], server[%s:%u]", m_connid, remote_ip, remote_port);
			return;
		}

		if (!connected){
			LOG_PRINT(log_error, "client-session is stopped,not continue,queue-size:%u.[client connid:%u,%s:%d]",
					m_send_queue.size(), getconnid(), remote_ip, remote_port);
			return;
		}

		if(m_send_queue.size() > 5000)
		{
			CAlarmNotify::sendAlarmNoty(e_all_notitype, e_msgqueue, Application::getInstance()->m_strProcName, "message Queue",
					"Room,Yunwei", "clienthandle send msg queue above Max,client:%s:%d.", remote_ip, remote_port);
			LOG_PRINT(log_warning, "clienthandle send msg queue above Max,client:%s:%d.", remote_ip, remote_port);
		}

		boost::mutex::scoped_lock lock(send_messages_mutex_);
		if (!m_send_queue.empty())
		{
			m_send_queue.pop_front();
		}
		
		if(!m_send_queue.empty() && connected)  //needclose_ wait all message send-out
		{
			SL_ByteBuffer* pslbuf = &(m_send_queue.front());
			boost::asio::async_write(socket_,
				boost::asio::buffer(pslbuf->buffer(), pslbuf->data_end()),
				boost::bind(&clienthandler::handle_write, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		}
		else if(!connected)
		{
			LOG_PRINT(log_error, "client-session is stopped,not continue,queue-size:%u.[client connid:%u,%s:%d]",
					m_send_queue.size(), m_connid, remote_ip, remote_port);
		}
	}
	else
	{
		LOG_PRINT(log_error, "[clientobject:%x,client connid:%u,remoteip:%s,port:%u][%s]",
				this, m_connid, remote_ip, remote_port, boost::system::system_error(e).what());
		if(e != boost::asio::error::operation_aborted)
		{
			if(m_workerEvent)
				onError(e.value(), boost::system::system_error(e).what());
			close();
		}
	}
}

void clienthandler::handle_write_loop(const boost::system::error_code& e)
{
	if(!e)
	{
		SL_ByteBuffer buff ;
		boost::mutex::scoped_lock locker(messagelstmtx);
		if(!m_vbuffer.empty())
		{
			for(int i = 0 ; i < m_vbuffer.size() ; i++)
			{
				buff.write(m_vbuffer[i].buffer(), m_vbuffer[i].data_end());
			}
			m_vbuffer.clear();
			socket_.get_io_service().post(boost::bind(&clienthandler::send_message, shared_from_this(), buff, false));
		}

		writetimer.expires_from_now(boost::posix_time::milliseconds(Application::m_interval_time));
		writetimer.async_wait(boost::bind(&clienthandler::handle_write_loop, shared_from_this(), boost::asio::placeholders::error));
	}
}

void clienthandler::addclientid(unsigned int client_connid)
{
	boost::mutex::scoped_lock lock(conn_mutex_);
	m_client_connid_set.insert(client_connid);
}

void clienthandler::delclientid(unsigned int client_connid)
{
	boost::mutex::scoped_lock lock(conn_mutex_);
	set<unsigned int>::iterator iter = m_client_connid_set.find(client_connid);
	if (m_client_connid_set.end() != iter)
	{
		m_client_connid_set.erase(iter);
	}
}

unsigned int clienthandler::getclientid_size()
{
	boost::mutex::scoped_lock lock(conn_mutex_);
	return m_client_connid_set.size();
}

void clienthandler::getallclientid(std::set<unsigned int> & clientid_set)
{
	clientid_set.clear();
	boost::mutex::scoped_lock lock(conn_mutex_);
	if (!m_client_connid_set.empty())
	{
		clientid_set.insert(m_client_connid_set.begin(), m_client_connid_set.end());
	}
}

void clienthandler::resperrinf(COM_MSG_HEADER * reqHead, ClientGateMask_t * pMask, uint16 errcode)
{
	if (reqHead == NULL || pMask == NULL)
		return ;

	char szOutBuf[128] = {0};
	COM_MSG_HEADER * pOutMsg=(COM_MSG_HEADER*)szOutBuf;
	pOutMsg->version = MDM_Version_Value;
	pOutMsg->checkcode = reqHead->checkcode;
	pOutMsg->maincmd = reqHead->maincmd;
	pOutMsg->subcmd = Sub_Vchat_Resp_ErrCode;
	pOutMsg->reqid = reqHead->reqid;

	memcpy(pOutMsg->content, pMask, SIZE_IVM_CLIENTGATE); 	

	CMDErrCodeResp_t * message = (CMDErrCodeResp_t*)(pOutMsg->content + SIZE_IVM_CLIENTGATE);
	memset(message, 0, sizeof(CMDErrCodeResp_t));
	message->errmaincmd = reqHead->maincmd;
	message->errsubcmd = reqHead->subcmd;
	message->errcode = errcode;

	pOutMsg->length = SIZE_IVM_TOTAL(CMDErrCodeResp_t);
	write_message(szOutBuf, pOutMsg->length);

	return;
}

void clienthandler::setRemoteSvrIp(const char* ip,unsigned short port)
{
	m_swtich = 1;//begin setting
	release();//close socket;

	//set connecting
	disconn_report_times = 0;
	strcpy(remote_ip, ip);
	remote_port = port;

	boost::asio::ip::address addr1 =boost::asio::ip::address_v4::from_string(ip);
	endpoint_.address(addr1);
	endpoint_.port(port);
	m_swtich = 0;//end setting
	LOG_PRINT(log_error, "[remoteip:%s,port:%u]status:[%d]", remote_ip, remote_port, session_status_);
}

void clienthandler::addTaskMessage(int msgType, int subType, int code/* = 0*/, const std::string &msg/* = std::string()*/)
{
	int datalen = sizeof(task_proc_param) + msg.length() + 1;
	task_proc_data* task_data = new task_proc_data;
	task_proc_param *param = (task_proc_param *)new char[datalen];
	param->message = subType;
	param->param1 = code;
	param->param2 = 1;
	if (msg.size())
	{
		strncpy(param->msg, msg.c_str(), msg.length());
		param->msg[msg.length()] = 0;
	}
	task_data->msgtye=msgType;
	task_data->pdata = (char *)param;
	task_data->datalen = datalen;
	task_data->connection = shared_from_this();
	Application::getInstance()->add_message(task_data);
}

void clienthandler::onError(int code, const std::string &msg /*= std::string()*/)
{
	addTaskMessage(TASK_MSGTYPE_LOCAL, MSG_TYPE_ERROR, code, msg);
}

CConnection::CConnection(const string protocolname)
{
	m_connecthandler_Ptr.reset(new clienthandler(Application::get_io_service(),protocolname,NULL));
}

CConnection::~CConnection()
{

}

void CConnection::setRemoteSvrIp(const char* ip,unsigned short port)
{
	m_connecthandler_Ptr->setRemoteSvrIp(ip, port);
}
void CConnection::connect(const char * ip, u_int16_t port)
{
	m_connecthandler_Ptr->start_connect(ip, port);
}
void CConnection::stop()
{
	m_connecthandler_Ptr->stop();
}
