#include "tcp_base.hpp"

#include <time.h>
#include <assert.h>
#include "CLogThread.h"
#include "message_comm.h"

CTcpBase::CTcpBase(tcp_service & io_service, size_t connId)
    : m_cService(io_service)
    , m_cSocket(io_service)
    , m_cTimer(io_service)
    , m_nRecvUsedLen(0)
    , m_dequeSendMessage()
    , m_mutexSendMessage()
    , m_bStop(true)
    , m_bNeedClose(true)
    , m_uConnectionId(connId)
    , m_uLastActiveTime(time(0))
    , m_uBeginConnectTime(0)
    , m_uTimeo(1)
    , m_strRemoteIp()
    , m_nRemotePort(0)
    , m_nStatus(en_status_disconnected)
    , m_bWaitSendQueueEmpty(false)
{
    LOG_PRINT(log_debug, "construct CTcpBase");
}

CTcpBase::~CTcpBase()
{
    LOG_PRINT(log_debug, "destruct CTcpBase");

    //fix memory leak.
    clearData();
}

void CTcpBase::start(ssize_t timeo, bool passive/* = true*/)
{
    m_uTimeo            = timeo;
    if (passive) {
        try  {
    		m_strRemoteIp = m_cSocket.remote_endpoint().address().to_string();
    		m_nRemotePort = m_cSocket.remote_endpoint().port();
    	}  catch(boost::system::system_error &ec) {
    		LOG_PRINT(log_error, "ERROR!!!! get remote ip/port error!!!");
    	}

        LOG_PRINT(log_info, "start client:[%d][%s:%d]", 
            m_uConnectionId, m_strRemoteIp.c_str(), m_nRemotePort);

        boost::asio::ip::tcp::no_delay option(true);
        m_cSocket.set_option(option);
#if 0
        int                 keepIdle = 6;
        int                 keepInterval = 5;
        int                 keepCount = 3;
        setsockopt(m_cSocket.native(), SOL_TCP, TCP_KEEPIDLE, (void *)&keepIdle, sizeof(keepIdle));
        setsockopt(m_cSocket.native(), SOL_TCP,TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
        setsockopt(m_cSocket.native(),SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));
#endif        
        m_nRecvUsedLen  = 0;
        m_nStatus       = en_status_connected;
        m_bStop         = false;
        m_bNeedClose    = false;

        m_cSocket.async_read_some(
            boost::asio::buffer((void *)m_szRecvBuffer, en_msgbuffersize),
    		boost::bind(&CTcpBase::handleRead, 
    		    shared_from_this(),
    		    boost::asio::placeholders::error,
    		    boost::asio::placeholders::bytes_transferred));
    }
    
	m_cTimer.expires_from_now(boost::posix_time::seconds(m_uTimeo));
	m_cTimer.async_wait(boost::bind(&CTcpBase::handleTimeout, 
        shared_from_this(),
		boost::asio::placeholders::error));
}

void CTcpBase::stop()
{
	boost::system::error_code e;
	m_cTimer.cancel(e);
}

void CTcpBase::connect(const char* szip, int port)
{
	if(m_nStatus != en_status_disconnected) {
        LOG_PRINT(log_info, "connId:%d, socket to server[%s:%d] status:%d",
            m_uConnectionId, m_strRemoteIp.c_str(), m_nRemotePort, m_nStatus);
		return;
    }

    m_strRemoteIp   = szip;
    m_nRemotePort   = port;
    m_bStop         = false;
    m_bNeedClose    = false;

    boost::asio::ip::tcp::endpoint endpoint;
	boost::asio::ip::address addr 
        = boost::asio::ip::address_v4::from_string(m_strRemoteIp);
	endpoint.address(addr);
	endpoint.port(m_nRemotePort);

	LOG_PRINT(log_error, "connId:%d, begin connect server[%s:%d]...",
		m_uConnectionId, m_strRemoteIp.c_str(), m_nRemotePort);

	m_nRecvUsedLen = 0;
    
	m_cSocket.async_connect(endpoint,
		boost::bind(&CTcpBase::handleConnect, 
		    shared_from_this(), 
		    boost::asio::placeholders::error));
    
	m_nStatus           = en_status_connecting;
	m_uBeginConnectTime = time(0);
}

void CTcpBase::connect()
{
	if(m_nStatus != en_status_disconnected) {
        LOG_PRINT(log_info, "connId:%d, socket to server[%s:%d] status:%d",
            m_uConnectionId, m_strRemoteIp.c_str(), m_nRemotePort, m_nStatus);
		return;
    }
    
	if(m_strRemoteIp.empty() || m_nRemotePort == 0) {
		LOG_PRINT(log_error, "ip/port(%d) invalid!", m_nRemotePort);
		return;
	}

	LOG_PRINT(log_error, "connId:%d, begin connect server[%s:%d]...",
		m_uConnectionId, m_strRemoteIp.c_str(), m_nRemotePort);

	m_nRecvUsedLen = 0;
    m_bStop         = false;
    m_bNeedClose    = false;

    boost::asio::ip::tcp::endpoint endpoint;
	boost::asio::ip::address addr 
        = boost::asio::ip::address_v4::from_string(m_strRemoteIp);
	endpoint.address(addr);
	endpoint.port(m_nRemotePort);
    
	m_cSocket.async_connect(endpoint,
		boost::bind(&CTcpBase::handleConnect, 
		    shared_from_this(), 
		    boost::asio::placeholders::error));
	m_nStatus           = en_status_connecting;
	m_uBeginConnectTime = time(0);
}

void CTcpBase::close()
{
	m_cService.post(boost::bind(&CTcpBase::doClose, 
        shared_from_this()));
}

tcp_socket & CTcpBase::socket()
{
    return m_cSocket;
}

void CTcpBase::writeMessage(const SL_ByteBuffer& message)
{
    m_cService.post(
        boost::bind(&CTcpBase::sendMessage, 
            shared_from_this(), message));
}

void CTcpBase::writeMessage(const char* pdata, int datalen)
{
	SL_ByteBuffer message(datalen);
	message.write(pdata, datalen);
    m_cService.post(
        boost::bind(&CTcpBase::sendMessage, 
            shared_from_this(), message));
}

size_t CTcpBase::sendQueueSize()
{
     boost::mutex::scoped_lock lock(m_mutexSendMessage);
     return m_dequeSendMessage.size();
}

bool CTcpBase::isSendQueueEmpty()
{
    boost::mutex::scoped_lock lock(m_mutexSendMessage);
    return m_dequeSendMessage.empty();
}

bool CTcpBase::waitQueueEmpty()
{
    return m_bWaitSendQueueEmpty;
}

void CTcpBase::setWaitQueueFlag(bool flag)
{
    m_bWaitSendQueueEmpty = flag;
}

void CTcpBase::sendMessage(SL_ByteBuffer& message)
{
    if (m_bStop || m_bNeedClose || 
        (m_nStatus != en_status_transfer && m_nStatus != en_status_connected)) {
        return;
    }

    boost::mutex::scoped_lock lock(m_mutexSendMessage);

    if (!onAlarm(m_dequeSendMessage.size())) {
        LOG_PRINT(log_error, "connId:%d, alarm, and close socket!", m_uConnectionId);
        close();
        return;
    }

    bool write_in_progress = !m_dequeSendMessage.empty();
	m_dequeSendMessage.push_back(message);
    
	if(!write_in_progress) {
	
		SL_ByteBuffer* pslbuf=&(m_dequeSendMessage.front());
		boost::asio::async_write(
            m_cSocket,
			boost::asio::buffer(pslbuf->buffer(), pslbuf->data_end()),
			boost::bind(&CTcpBase::handleWrite, 
			    shared_from_this(),
			    boost::asio::placeholders::error,
			    boost::asio::placeholders::bytes_transferred));
	}
}

void CTcpBase::handleConnect(const boost::system::error_code& e)
{
	if(!e)
	{
		LOG_PRINT(log_info, "connId:%d, connect Server[%s:%d] OK! clear old queue-size(%d)", 
            m_uConnectionId, m_strRemoteIp.c_str(), m_nRemotePort, m_dequeSendMessage.size());
        
		clearData();

        if (m_bStop || m_bNeedClose) {
            LOG_PRINT(log_info, "connId:%d, connect Server[%s:%d] OK! but need stop!",
                m_uConnectionId, m_strRemoteIp.c_str(), m_nRemotePort);
            return;
        }

		boost::asio::ip::tcp::no_delay option(true);
		m_cSocket.set_option(option);

        m_uLastActiveTime   = time(NULL);
        m_nStatus           = en_status_connected;
        
        onConnect();
        
        m_nStatus           = en_status_transfer;

		//start recv data...
		char* recv_buffer = m_szRecvBuffer + m_nRecvUsedLen;
		std::size_t rev_buffer_size = en_msgbuffersize - m_nRecvUsedLen;

		m_cSocket.async_read_some(
            boost::asio::buffer((void*)recv_buffer, rev_buffer_size),
			boost::bind(&CTcpBase::handleRead, 
			    shared_from_this(),
			    boost::asio::placeholders::error,
			    boost::asio::placeholders::bytes_transferred));
	}
	else
	{
		LOG_PRINT(log_error,"connId:%d, error:%s", 
            m_uConnectionId, boost::system::system_error(e).what());
	}
}

void CTcpBase::handleTimeout(const boost::system::error_code& e)
{
    //LOG_PRINT(log_debug, "handle timeout socket:[%d][%s:%d]", 
    //        m_uConnectionId, m_strRemoteIp.c_str(), m_nRemotePort);
    if (!e) {
        
        if (!onTimeout()) {
            LOG_PRINT(log_info, "connId:%d, timeout handle return false, timer stop!",
                m_uConnectionId);
            return;
        }

        if (m_bStop || m_bNeedClose) {
            LOG_PRINT(log_info, "connId:%d, timeout, but stop flag set!",
                m_uConnectionId);
            return;
        }

        m_cTimer.expires_from_now(boost::posix_time::seconds(m_uTimeo));
	    m_cTimer.async_wait(
            boost::bind(
                &CTcpBase::handleTimeout, 
                shared_from_this(),
    		    boost::asio::placeholders::error));
    } else {
        LOG_PRINT(log_error, "connId:%d, error:%s", 
            m_uConnectionId, boost::system::system_error(e).what());
    }
}

void CTcpBase::handleRead(const boost::system::error_code& e,
							 std::size_t bytes_transferred)
{
	if(!e)
	{
	    if (m_bStop || m_bNeedClose) {
            LOG_PRINT(log_info, "connId:%d, [%s:%d]stop flag set !!!!!",
                m_uConnectionId, m_strRemoteIp.c_str(), m_nRemotePort);
            return;
        }
        
		if(bytes_transferred == 0) {
			LOG_PRINT(log_error, "connId:%d, [%s:%d]recv data-len=0 !!!!!",
                m_uConnectionId, m_strRemoteIp.c_str(), m_nRemotePort);
			close();
			return;
		}
        
        onReadDone(bytes_transferred);
        
		m_nRecvUsedLen += bytes_transferred;
		if(parseMessage() == -1) {
		
            LOG_PRINT(log_error, "connId:%d, [%s:%d]parse message failed! close socket !",
                m_uConnectionId, m_strRemoteIp.c_str(), m_nRemotePort);
			close();
			return;
		}
         
		if(m_nStatus == en_status_transfer || m_nStatus == en_status_connected) {
		
			//start recv data...
			char* recv_buffer = m_szRecvBuffer + m_nRecvUsedLen;
			std::size_t rev_buffer_size = en_msgbuffersize - m_nRecvUsedLen;

			m_cSocket.async_read_some(
                boost::asio::buffer((void*)recv_buffer, rev_buffer_size),
				    boost::bind(&CTcpBase::handleRead, 
				    shared_from_this(),
				    boost::asio::placeholders::error,
				    boost::asio::placeholders::bytes_transferred));
		}
	}
	else
	{
		LOG_PRINT(log_error,"connId:%d, error:%s", 
            m_uConnectionId, boost::system::system_error(e).what());
		if(e != boost::asio::error::operation_aborted) //boost::asio::error::eof
			close();
	}
}


void CTcpBase::handleWrite(const boost::system::error_code& e,
							  std::size_t bytes_transferred)
{
	if(!e)
	{
	    if (m_bStop || m_bNeedClose) {
            LOG_PRINT(log_info, "connId:%d, [%s:%d]stop flag set !!!!!", 
                m_uConnectionId, m_strRemoteIp.c_str(), m_nRemotePort);
            return;
        }

        onWriteDone(bytes_transferred);
        
		boost::mutex::scoped_lock lock(m_mutexSendMessage);
        
		if(!m_dequeSendMessage.empty()) {
            m_dequeSendMessage.pop_front();
        }
        
		if(!m_dequeSendMessage.empty()) {
		
			SL_ByteBuffer* pslbuf =&(m_dequeSendMessage.front());
			boost::asio::async_write(m_cSocket,
				boost::asio::buffer(pslbuf->buffer(), pslbuf->data_end()),
				boost::bind(&CTcpBase::handleWrite, 
				shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		}
	}
	else
	{
		LOG_PRINT(log_error,"connId:%d, error:%s", 
            m_uConnectionId, boost::system::system_error(e).what());
		if(e != boost::asio::error::operation_aborted)
			close();
	}
}

int CTcpBase::parseMessage()
{
	char* p=m_szRecvBuffer;
	while(m_nRecvUsedLen > 4)
	{
		int msglen=*((int*)p);
		if(msglen <= 0||msglen > en_msgbuffersize)
		{
		    LOG_PRINT(log_error, "conn[%d] invalid msg len:%d!", 
                m_uConnectionId, msglen);
			m_nRecvUsedLen=0;
			return -1;
		}
		else if(m_nRecvUsedLen < msglen)
		{
			break;
		}
		else {
			if(handleMessage(p, msglen)==-1)
			{
			    LOG_PRINT(log_error, "connId[%d] handleMessage failed!", 
                    m_uConnectionId);
				m_nRecvUsedLen=0;
				return -1;
			}
			m_nRecvUsedLen -= msglen;
			p += msglen;
		}
	}
	if(m_nRecvUsedLen>=en_msgmaxsize)
	{
	    LOG_PRINT(log_error, "connId[%d] invalid recv len:%d!",
            m_uConnectionId, m_nRecvUsedLen);
		m_nRecvUsedLen =0;
		return -1;
	}
	if(p != m_szRecvBuffer && m_nRecvUsedLen>0)
	{
		memmove(m_szRecvBuffer, p, m_nRecvUsedLen);
	}
	return 0;
}

void CTcpBase::doClose()
{
    if (!m_bStop) {
        LOG_PRINT(log_info, "close socket:[%d][%s:%d]", 
            m_uConnectionId, m_strRemoteIp.c_str(), m_nRemotePort);
        
        m_nStatus   = en_status_disconnected;
        m_bStop = true;
		boost::system::error_code ec;
		m_cSocket.close(ec);
        onClose();
    }
    clearData();
}

void CTcpBase::clearData()
{
	boost::mutex::scoped_lock lock(m_mutexSendMessage);
	if(m_dequeSendMessage.size() > 0) {
		m_dequeSendMessage.clear();
    }
}

void CTcpBase::onConnect()
{
}

void CTcpBase::onClose()
{
}

void CTcpBase::onReadDone(size_t size)
{
}

void CTcpBase::onWriteDone(size_t size)
{
}

bool CTcpBase::onTimeout()
{
    return false;
}

bool CTcpBase::onAlarm(size_t queuesize)
{
    return false;
}

