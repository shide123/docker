#include "connecthandler.h"
#include "SyncServerApplication.h"
#include "clienthandler.h"
#include "server.h"
#include "connmanager.h"



#define GP_MAX_SENDQUEUE_SIZE 6000

connecthandler::connecthandler(boost::asio::io_service& io_service)
    : sockethandler(io_service)
	, m_nlasttime_printstack(0)
	, io_service_(io_service)
	, pRemote_Redis(NULL)
	, last_sync_log_index(UINT64_MAX)
	, disconn_report_times(0)
{
    actived = true;
	m_ngateid = 0;
}

connecthandler::~connecthandler()
{    
	connected = false;
}

void connecthandler::stop()
{
	boost::system::error_code e;
	deadline_.cancel(e);
}

void connecthandler::start_connect(const char *addr, u_int16_t port, server *svr)
{
    strcpy(remote_ip, addr);
    remote_port = port;
    serv = svr;

	boost::asio::ip::address addr1 =boost::asio::ip::address_v4::from_string(addr);
	endpoint_.address(addr1);
	endpoint_.port(port);

	deadline_.expires_from_now(boost::posix_time::seconds(en_checkconntimeout));
	deadline_.async_wait(boost::bind(&connecthandler::handle_timeout, shared_from_this(), boost::asio::placeholders::error));
	LOG_PRINT(log_info, "Begin connect server[%s:%d]...", endpoint_.address().to_string().c_str(), endpoint_.port());

	recv_buf_remainlen_ =0;
	socket_.async_connect(endpoint_, boost::bind(&connecthandler::handle_connect, shared_from_this(), boost::asio::placeholders::error));
	begin_connecttime_ =time(0);

}

int connecthandler::connect()
{
	if (disconn_report_times < 3){
		LOG_PRINT(log_info, "Begin connect server[%s:%d]...", endpoint_.address().to_string().c_str(), endpoint_.port());
		disconn_report_times++;
	}

	recv_buf_remainlen_ =0;
	socket_.async_connect(endpoint_, boost::bind(&connecthandler::handle_connect, shared_from_this(), boost::asio::placeholders::error));

	begin_connecttime_ =time(0);
	return 0;
}

void connecthandler::handle_connect(const boost::system::error_code& e)
{
	if(!e)
	{
	    connected = true;
	    if (serv){
	        connid_ = serv->newconnid();
	        sockethandler::setconnhandler(connid_, this);
	    }
	    clear_data();
	    last_activetime_ = time(0);
	    disconn_report_times = 0;

	    appInstance->m_pConnMgr->add_svr_conn(shared_from_this());

	    LOG_PRINT(log_info, "connect to sync node:%s:%u! queue-size(%d).", remote_ip, remote_port, m_send_queue.size());
		boost::asio::ip::tcp::no_delay option(true);
		socket_.set_option(option);
//	    struct timeval timeout={1, 0};//3s
//	    int ret = setsockopt(socket_.native_handle(), SOL_SOCKET,SO_SNDTIMEO, &timeout, sizeof(timeout));

		//first send hello message
		send_hello();
		report_sync_status();
        
		//start recv data...
		char* recv_buffer = recv_buf_+recv_buf_remainlen_;
		std::size_t rev_buffer_size = en_msgbuffersize - recv_buf_remainlen_;

		socket_.async_read_some(boost::asio::buffer((void*)recv_buffer, rev_buffer_size),
			boost::bind(&connecthandler::handle_read, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));

		deadline_.expires_from_now(boost::posix_time::seconds(en_checkconntimeout));
		deadline_.async_wait(boost::bind(&connecthandler::handle_timeout, shared_from_this(), boost::asio::placeholders::error));
	}
	else
	{
		if (disconn_report_times < 3)
			LOG_PRINT(log_error, "%s", boost::system::system_error(e).what());
	}
}

void connecthandler::handle_timeout(const boost::system::error_code& e)
{
	if(!e)
	{
		if(!connected)
		{
			if (disconn_report_times < 3){
				LOG_PRINT(log_error, "Server is disconnected, then try connect.");
			}
			connect();
		}
		time_t now = time(0);

		if (connected){
			report_sync_status();
			Req_sync_full_data();
		}

		if (now - last_activetime_ >= 20){
			last_activetime_ = now;
			LOG_PRINT(log_debug, "queue-size: %u .", m_send_queue.size());
		}

		deadline_.expires_from_now(boost::posix_time::seconds(en_checkconntimeout));
		deadline_.async_wait(boost::bind(&connecthandler::handle_timeout, shared_from_this(), boost::asio::placeholders::error));
	}
	else
	{
		LOG_PRINT(log_error, "%s", boost::system::system_error(e).what());
	}
}

void connecthandler::handle_read(const boost::system::error_code& e, std::size_t bytes_transferred)
{
	if(!e) {
		if(bytes_transferred ==0)
		{
			LOG_PRINT(log_error, "[clientobject=%llu, connid=%llu, socket=%d] syncsvr disconnect  (handle_write)", (uint64)this, connid_, sfd);
			release();
			return;
		}
		recv_buf_remainlen_ += bytes_transferred;
		if(parse_message() == -1)
		{
			LOG_PRINT(log_error, "[clientobject=%llu, connid=%llu, socket=%d, recvlen=%d, msglen=%d]parse message failed! msg len is invalid or over than the msg buffer size",
                        (uint64_t)this, connid_, sfd, bytes_transferred, msgpacklen);
            release();
			return;
		}
		if(connected)
		{
			//start recv data...
			char* recv_buffer=recv_buf_+ recv_buf_remainlen_;
			std::size_t rev_buffer_size = en_msgbuffersize - recv_buf_remainlen_;

			socket_.async_read_some(boost::asio::buffer((void*)recv_buffer, rev_buffer_size),
				boost::bind(&connecthandler::handle_read, shared_from_this(),
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		}
	}
	else
	{
		LOG_PRINT(log_error, "%s", boost::system::system_error(e).what());
		if(e != boost::asio::error::operation_aborted)
			release();
	}
}

void connecthandler::write_message(SL_ByteBuffer& message, bool bforced/*=false*/)
{
	io_service_.post(boost::bind(&connecthandler::send_message, shared_from_this(), message, bforced));

	handle_msgsend_record(message.data());
}

void connecthandler::write_message(char* pdata, int datalen, bool bforced/*=false*/)
{
	SL_ByteBuffer buffer(datalen);
	buffer.write(pdata, datalen);
	io_service_.post(boost::bind(&connecthandler::send_message, shared_from_this(), buffer, bforced));

	handle_msgsend_record(pdata);
}

void connecthandler::send_hello()
{
	SL_ByteBuffer outbuf(512);
	COM_MSG_INDEX_HEADER* pmsgheader=(COM_MSG_INDEX_HEADER*)outbuf.buffer();
	pmsgheader->version = 10;
	pmsgheader->checkcode = PACK_ORDINARY;
	pmsgheader->maincmd =MDM_Vchat_Redis_Sync;
	pmsgheader->subcmd =Sub_Vchat_ClientHello;
	pmsgheader->index = appInstance->new_index();
	CMDGateHello_t* preq=(CMDGateHello_t*)pmsgheader->content;
	preq->param1=12;
	preq->param2=8;
	preq->param3=7;
	preq->param4=e_syncsvr_type;
	preq->gateid = this->m_ngateid;
	pmsgheader->length= sizeof(COM_MSG_INDEX_HEADER)+sizeof(CMDGateHello_t);
	outbuf.data_end(pmsgheader->length);
	write_message(outbuf);
}

void connecthandler::report_sync_status()
{
	SL_ByteBuffer outbuf(32);
	COM_MSG_INDEX_HEADER* pmsgheader=(COM_MSG_INDEX_HEADER*)outbuf.buffer();
	pmsgheader->version = 10;
	pmsgheader->checkcode = PACK_ORDINARY;
	pmsgheader->maincmd = MDM_Vchat_Redis_Sync;
	pmsgheader->subcmd = Sub_Vchat_RedisSync_Report_Status;
	pmsgheader->index = appInstance->new_index();
	pmsgheader->length = SIZE_IVM_INDEX_HEADER + sizeof(byte);
	byte status = appInstance->m_sync_status;
	memcpy(pmsgheader->content, &status, sizeof(byte));
	outbuf.data_end(pmsgheader->length);
	write_message(outbuf.data(), outbuf.data_size());
}

void connecthandler::Req_sync_full_data()
{
	SL_ByteBuffer outbuf(32);
	if (!appInstance->m_bsyncing && SYNC_INIT == appInstance->m_sync_status && appInstance->get_sync_master_ip() == remote_ip){
		LOG_PRINT(log_info, "start sync data from sync node[%s:%u]...", remote_ip, remote_port);
		COM_MSG_INDEX_HEADER* pmsgheader=(COM_MSG_INDEX_HEADER*)outbuf.buffer();
		pmsgheader->version = 10;
		pmsgheader->checkcode = PACK_ORDINARY;
		pmsgheader->maincmd = MDM_Vchat_Redis_Sync;
		pmsgheader->subcmd = Sub_SyncData_req;
		pmsgheader->index = appInstance->new_index();
		pmsgheader->length = SIZE_IVM_INDEX_HEADER;
		outbuf.data_end(pmsgheader->length);
		write_message(outbuf.data(), outbuf.data_size());
		last_sync_log_time = time(0);
		appInstance->m_bsyncing = true;
	}
}

void connecthandler::syncsvr_notify(CMDRoomsvrNotify_t &data)
{
	SL_ByteBuffer buf;
	if (!buf.reserve(256)){
		perror("[connecthandler::syncsvr_notify] failed to allocate memory!");
		return;
	}
	COM_MSG_INDEX_HEADER* pHeader = (COM_MSG_INDEX_HEADER*)buf.buffer();
	pHeader->version = 10;
	pHeader->checkcode = PACK_REQ;
	pHeader->maincmd = MDM_Vchat_Redis_Sync;
	pHeader->subcmd = Sub_Vchat_RoomAmong_Notify;
	pHeader->index = appInstance->new_index();
	CMDRoomsvrNotify_t *pMsg = (CMDRoomsvrNotify_t *)pHeader->content;
	memcpy(pMsg, &data, sizeof(CMDRoomsvrNotify_t));
	pHeader->length = SIZE_IVM_INDEX_HEADER + sizeof(CMDRoomsvrNotify_t);
	buf.data_end(pHeader->length);
	write_message(buf, true);
}

void connecthandler::addclientid(u_int64_t client_connid)
{
	boost::mutex::scoped_lock lock(conn_mutex_);
	vec_client_connid.push_back(client_connid);
}

void connecthandler::delclientid(u_int64_t client_connid)
{
	boost::mutex::scoped_lock lock(conn_mutex_);
	vector<u_int64_t>::iterator it = vec_client_connid.begin();
	while(it != vec_client_connid.end()){
		if (*it == client_connid){
			vec_client_connid.erase(it);
			break;
		}
		++it;
	}
}

void connecthandler::send_message(SL_ByteBuffer& message, bool bforced)
{
	if(connected || bforced)
	{
		boost::mutex::scoped_lock lock(send_messages_mutex_);
		bool write_in_progress =!m_send_queue.empty();
		m_send_queue.push_back(message);
		if(!write_in_progress)
		{
			SL_ByteBuffer* pslbuf=&(m_send_queue.front());
			boost::asio::async_write(socket_,
				boost::asio::buffer(pslbuf->buffer(), pslbuf->data_end()),
				boost::bind(&connecthandler::handle_write, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		}
	}
}

void connecthandler::handle_write(const boost::system::error_code& e, std::size_t bytes_transferred)
{
	if(!e)
	{
		boost::mutex::scoped_lock lock(send_messages_mutex_);
		m_send_queue.pop_front();
		if(!m_send_queue.empty() && connected)
		{
			SL_ByteBuffer* pslbuf =&(m_send_queue.front());
			boost::asio::async_write(socket_,
				boost::asio::buffer(pslbuf->buffer(), pslbuf->data_end()),
				boost::bind(&connecthandler::handle_write, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		}
	}
	else
	{
		LOG_PRINT(log_error, "%s", boost::system::system_error(e).what());
		if(e != boost::asio::error::operation_aborted)
			close();
	}
}

int connecthandler::handle_message(const char* pdata, int msglen)
{
    if (NULL == pdata || msglen == 0)
        return 0;

	COM_MSG_INDEX_HEADER* in_msg = (COM_MSG_INDEX_HEADER*)pdata;
	
    if (handle_msgsend_timeout(pdata) < 0)
    	return 0;

	int newsize = sizeof(MSG_PROC_TASK_NODE) + msglen;
	SL_ByteBuffer buf(newsize);
   	char* pwritebuf=buf.buffer();
   	MSG_PROC_TASK_NODE *task_node=(MSG_PROC_TASK_NODE *)pwritebuf;
   	task_node->task_type = TASK_TYPE_REDISSYNC_MSG;
   	task_node->wparam = 0;
   	task_node->lparam = 0;
   	task_node->from_connectPtr = (clienthandler*)this;      //设置connect对象指针
   	task_node->connid = getconnid();
   	task_node->pdata = 0;
   	pwritebuf += sizeof(MSG_PROC_TASK_NODE);
   	memcpy(pwritebuf,pdata,msglen);
   	task_node->pdata=pwritebuf;
   	buf.data_end(newsize);

   	appInstance->m_online_task.putq(buf);
   	return 0;

}

void connecthandler::clear_data()
{
	boost::mutex::scoped_lock lock(send_messages_mutex_);
	int ncount = m_send_queue.size();

	if(ncount > 0)
		m_send_queue.clear();
}


void connecthandler::release()
{
	appInstance->m_pConnMgr->del_conn_svr(shared_from_this());
	LOG_PRINT(log_warning, "[clientobject=%x, connid=%u, socket=%d] [userid=%d, roomid=%d] disconnected with syncsvr",
               this, connid_, socket_.native_handle());
    close();

    handle_msgsend_timeout(0);
}


