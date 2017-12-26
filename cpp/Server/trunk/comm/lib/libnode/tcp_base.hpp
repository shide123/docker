#ifndef __TCP_BASE_H__
#define __TCP_BASE_H__

#include <deque>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/asio/deadline_timer_service.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "SL_ByteBuffer.h"

typedef boost::asio::io_service      tcp_service;
typedef boost::asio::ip::tcp::socket tcp_socket;
typedef boost::asio::deadline_timer  tcp_timer;

class CTcpBase
	: public boost::enable_shared_from_this<CTcpBase>
	, private boost::noncopyable
{
public:
	explicit CTcpBase(tcp_service & io_service, size_t connId);
    
	virtual ~CTcpBase();

	virtual void start() = 0;
	void stop();

	void connect(const char* szip, int port);
	void connect();
	void close();
	tcp_socket & socket();
	bool IsConnected();
    size_t getConnectionId() { return m_uConnectionId; }
    time_t getLastActiveTime() {return m_uLastActiveTime; }
    const std::string & getRemoteIp() { return m_strRemoteIp; }
    int getRemotePort() { return m_nRemotePort; }
	void writeMessage(const SL_ByteBuffer& message);
	void writeMessage(const char* pdata, int datalen);

    virtual int handleMessage(const char* pdata, int msglen) = 0;
    virtual void onConnect();
    virtual bool onTimeout();
    virtual bool onAlarm(size_t queuesize);
    virtual void onClose();
    virtual void onReadDone(size_t size);
    virtual void onWriteDone(size_t size);
    
    template<typename T>
    boost::shared_ptr<T> get_ptr()
    {
        return boost::dynamic_pointer_cast<T>(shared_from_this());
    }

    size_t sendQueueSize();
    bool isSendQueueEmpty();
    bool waitQueueEmpty();
    void setWaitQueueFlag(bool flag);

    void clearData(void);
    
protected:
    void start(ssize_t timeo, bool passive = true);
    
	void sendMessage(SL_ByteBuffer& message);

	void handleConnect(const boost::system::error_code& e);
	void handleTimeout(const boost::system::error_code& e);
	void handleRead(const boost::system::error_code& e,
		std::size_t byte_transferred);
	void handleWrite(const boost::system::error_code& e,
		std::size_t byte_transferred);

	int parseMessage();
    
	void doClose();

public:
	enum {
		en_status_disconnected  = 0,
		en_status_connecting    = 1,
		en_status_connected     = 2,
		en_status_transfer      = 3,
	};

protected:
    typedef std::deque<SL_ByteBuffer> taskqueue_t;
	enum {
		en_msgbuffersize    = 1024 * 64,
		en_msgmaxsize       = 1024 * 32,
	};
    
	tcp_service &   m_cService;
	tcp_socket      m_cSocket;
	tcp_timer       m_cTimer;
    
	//read message buffer
	char            m_szRecvBuffer[en_msgbuffersize];
	int             m_nRecvUsedLen;
    
	//send message queue
	taskqueue_t     m_dequeSendMessage;
	boost::mutex    m_mutexSendMessage;

    bool            m_bStop;
    bool            m_bNeedClose;
    
    size_t          m_uConnectionId;
	time_t          m_uLastActiveTime;
	time_t          m_uBeginConnectTime;
    size_t          m_uTimeo;

    std::string     m_strRemoteIp;
    int             m_nRemotePort;
    
    int             m_nStatus;
    bool            m_bWaitSendQueueEmpty;
};

typedef boost::shared_ptr<CTcpBase>		tcp_ptr;
#endif //__TCP_BASE_H__

