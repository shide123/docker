#include "clienthandler.h"
#include "server.h"
#include "SyncServerApplication.h"
#include <stdint.h>

clienthandler::clienthandler(boost::asio::io_service& ioservice)
	: sockethandler(ioservice)
	, userid(0)
	, roomid(0)
	, svrid(0)
	, last_update_logindex(UINT64_MAX)
	, remote_sync_status(SYNC_INIT)
	, bSayHello(false)
	, lastalivetime(time(0))
{
}

clienthandler::~clienthandler()
{
	LOG_PRINT(log_debug, "clienthandler::~clienthandler");
}

int clienthandler::open(server* fromsvr, int fd, u_int64_t connid)
{
    connected = true;
    serv = fromsvr;
    connid_ = connid;
    sockethandler::setconnhandler(connid, this);
    sfd = fd;
    remote_port = 0;
    bSayHello = false;

	tcp::no_delay option(true);
	socket_.set_option(option);

	char* recv_buffer = recv_buf_ + recv_buf_remainlen_;
	std::size_t rev_buffer_size = en_msgbuffersize - recv_buf_remainlen_;

	socket_.async_read_some(boost::asio::buffer((void*)recv_buffer, rev_buffer_size),
		boost::bind(&clienthandler::handle_read, shared_from_this(),
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));

	deadline_.expires_from_now(boost::posix_time::seconds(1));
	deadline_.async_wait(boost::bind(&clienthandler::check_deadline, shared_from_this(), boost::asio::placeholders::error));

	lastalivetime =time(0);

    return 0;
}

void clienthandler::handle_read(const boost::system::error_code& e,	std::size_t bytes_transferred)
{
	if(!e) {
		if (!connected){
			LOG_PRINT(log_error, "[clientobject=%x, connid=%u, socket=%d] client is disconnected.",	this, connid_, sfd);
			release();
			return;
		}
		try {
			if(remote_port == 0){
				SL_Socket_INET_Addr::get_ip_remote_s(socket_.native_handle(), remote_ip, 30, &remote_port);
				LOG_PRINT(log_error, "recv client %s:%u connnection", remote_ip, remote_port);
			}
		}
		catch(boost::system::system_error &ec) {
			LOG_PRINT(log_error, "[clientobject=%x, connid=%u, socket=%d] get remote ip and port error!!! then close", this, connid_, sfd);
			release();
			return;
		}

		recv_buf_remainlen_ += bytes_transferred;
		if(parse_message() == -1)
		{
			LOG_PRINT(log_error, "[clientobject=%x, connid=%u, socket=%d remoteip=%s, port=%u] Parse message failed! close socket",
					this, connid_, sfd, remote_ip, remote_port);
			release();
			return;
		}

		//continue to recv data.
		char* recv_buffer = recv_buf_ + recv_buf_remainlen_;
		std::size_t rev_buffer_size = en_msgbuffersize - recv_buf_remainlen_;
		socket_.async_read_some(boost::asio::buffer((void*)recv_buffer, rev_buffer_size),
			boost::bind(&clienthandler::handle_read, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}
	else if(e)
	{
		LOG_PRINT(log_error, "[clientobject=%x, connid=%u, socket=%d remoteip=%s, port=%u] [%s]",
				this, connid_, sfd, remote_ip, remote_port, boost::system::system_error(e).what());

		if(e != boost::asio::error::try_again && e != boost::asio::error::would_block && e != boost::asio::error::operation_aborted)
		{
			release();
		}
	}
}

int clienthandler::handle_message(const char* pdata, int msglen)
{
    if (NULL == pdata || msglen == 0)
        return 0;

    lastalivetime = time(0);
    COM_MSG_INDEX_HEADER* in_msg = (COM_MSG_INDEX_HEADER*)pdata;
	
	if (handle_msgsend_timeout(pdata) < 0)
		return 0;
	
   	if (in_msg->version != MDM_Version_Value)
   	{
   		LOG_PRINT(log_error, "Packet version checks faild! socket:%d,[%s:%d]", sfd, remote_ip, remote_port);
   		return 0;
   	}
   	if(in_msg->length !=msglen)
   	{
   		LOG_PRINT(log_error, "Packet length checks faild! socket:%d,[%s:%d]", sfd, remote_ip, remote_port);
   		return 0;
   	}

   	//先处理hello消息
   	if(in_msg->maincmd == MDM_Vchat_Redis_Sync && in_msg->subcmd == Sub_Vchat_ClientHello)
   	{
   		CMDGateHello_t* pHelloMsg = (CMDGateHello_t *)in_msg->content;
   		if(pHelloMsg->param1 == 12 &&
   			pHelloMsg->param2 == 8 &&
   			pHelloMsg->param3 == 7 &&
   			pHelloMsg->param4 == e_syncsvr_type)
   		{
   			bSayHello = true;
   			svrid = pHelloMsg->gateid;
   			serv->setgateclient(svrid, this);
   			LOG_PRINT(log_info, "recv client hello message,conn_id=%u", connid_);
   			remote_sync_status = SYNC_INIT;
   			write_message(pdata, msglen);
   		}
   		return 0;
   	}
   	//之后hello后才是合法的消息,其他消息不处理(等待链接活动过期自动被删除)
    if (false == bSayHello){
    	LOG_PRINT(log_error, "[clientobject=%x, connid=%u, socket=%d] need client hello message.", this, connid_, sfd);
    	return 0;
    }

    if(in_msg->maincmd == MDM_Vchat_Redis_Sync && Sub_Vchat_RedisSync_Report_Status == in_msg->subcmd ){
    	remote_sync_status = (REDIS_SYNC_STATUS)in_msg->content[0];
    	if (SYNC_READY == remote_sync_status && appInstance->get_sync_master_ip().empty()) {
    		appInstance->set_sync_master_ip(getremote_ip());
    		LOG_PRINT(log_info, "remote server[%s:%d] remote_sync_status: %d", getremote_ip(), getremote_port(), remote_sync_status);
    	}
    	write_message(pdata, msglen);
    	return 0;
    }

   	lastalivetime = SL_Socket_CommonAPI::util_time();
   	if (in_msg->subcmd == Sub_SyncData_req)
   		last_update_logindex = appInstance->curr_synclog_seq();

   	//其余消息使用task处理
	int newsize = sizeof(MSG_PROC_TASK_NODE) + msglen;
	SL_ByteBuffer buf(newsize);
   	char* pwritebuf = buf.buffer();
   	if (!pwritebuf){
   		LOG_PRINT(log_fatal, "Failed to allocate memory");
   		return 0;
   	}
   	MSG_PROC_TASK_NODE *task_node=(MSG_PROC_TASK_NODE *)pwritebuf;
   	task_node->task_type = TASK_TYPE_REDISSYNC_MSG;
   	task_node->wparam = 0;
   	task_node->lparam = 0;
   	task_node->from_connectPtr = this;      //设置连接对象指针
   	task_node->connid = connid_;
   	task_node->pdata = 0;
   	pwritebuf += sizeof(MSG_PROC_TASK_NODE);
   	memcpy(pwritebuf,pdata,msglen);
   	task_node->pdata=pwritebuf;
   	buf.data_end(newsize);

   	appInstance->m_online_task.putq(buf);

   	return 0;
}

void clienthandler::write_message(SL_ByteBuffer& slByte)
{
	socket_.get_io_service().post(boost::bind(&clienthandler::send_message, shared_from_this(), slByte));
	handle_msgsend_record(slByte.data());
}

void clienthandler::write_message(const char* pdata, int datalen)
{
	SL_ByteBuffer buffer(datalen);
	buffer.write(pdata, datalen);
	socket_.get_io_service().post(boost::bind(&clienthandler::send_message, shared_from_this(), buffer));
	handle_msgsend_record(pdata);
}

void clienthandler::handle_write(const boost::system::error_code& e, std::size_t bytes_transferred)
{
	if(!e)
	{
		if (!connected){
			LOG_PRINT(log_error, "client-session is stopped,not continue,queue-size=%d.[id:%u,%s:%d]",
					m_send_queue.size(), getconnid(), remote_ip, remote_port);
			return;
		}
		if (!bytes_transferred)
			LOG_PRINT(log_error, "bytes_transferred:%u", bytes_transferred);

		boost::mutex::scoped_lock lock(send_messages_mutex_);
		m_send_queue.pop_front();
		if(!m_send_queue.empty() && connected)  //needclose_ wait all message send-out
		{
			SL_ByteBuffer* pslbuf =&(m_send_queue.front());
			boost::asio::async_write(socket_,
				boost::asio::buffer(pslbuf->buffer(), pslbuf->data_end()),
				boost::bind(&clienthandler::handle_write, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		}
		else if(!connected)
		{
			LOG_PRINT(log_error, "client-session is stopped,not continue,queue-size=%d.[id:%u,%s:%d]",
					m_send_queue.size(), connid_, remote_ip, remote_port);
		}
	}
	else
	{
		LOG_PRINT(log_error, "[connid=%u,socket=%d,error:%d,%s:%u] %s",	connid_, sfd, e.value(), remote_ip, remote_port,
				boost::system::system_error(e).what());
		if(e != boost::asio::error::try_again && e != boost::asio::error::would_block)
		{
			release();
		}
	}
}

void clienthandler::send_message(SL_ByteBuffer& slbuf)
{
	if(!connected)
		return;

	boost::mutex::scoped_lock lock(send_messages_mutex_);

	bool write_in_progress =!m_send_queue.empty();
	m_send_queue.push_back(slbuf);
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

void clienthandler::check_deadline(const boost::system::error_code& e)
{
	if(!e)
	{
	    handle_msgsend_timeout(0);

		time_t tNow = time(0);
		if (tNow - lastalivetime > 60){
			LOG_PRINT(log_error, "[clientobject=%x, connid=%u, socket=%d, userid=%d,roomid=%d]client idle time out",
					this, connid_, sfd, userid, roomid);

			release();
			return;
		}

		if (connected){
			deadline_.expires_from_now(boost::posix_time::seconds(en_checkconntimeout)); //最小间隔 en_checkactivetime 检查
			deadline_.async_wait(boost::bind(&clienthandler::check_deadline, shared_from_this(), boost::asio::placeholders::error));
		}
	}
	else if(e) //error
	{
		LOG_PRINT(log_error, "[%s],queue-size=%d.[id:%u,%s:%d]",
				boost::system::system_error(e).what(), m_send_queue.size(), getconnid(), remote_ip, remote_port);
	}
}

void clienthandler::release()
{
	LOG_PRINT(log_error, "[clientobject=%x, connid=%u, socket=%d, remote_ip=%s, remote_port=%u] closing sync connection",
			this, connid_, socket_.native_handle(), remote_ip, remote_port);
	CAlarmNotify::sendAlarmNoty(e_all_notitype, e_data_sync, "syncsvr", "sync connection occur error", "Yunwei,Room",
			"sync connection occur error, will interrupt [%s:%u]", remote_ip, remote_port);

    serv->setgateclient(svrid, NULL);
    close();
    userid = 0;
    roomid = 0;

    handle_msgsend_timeout(0);
}

