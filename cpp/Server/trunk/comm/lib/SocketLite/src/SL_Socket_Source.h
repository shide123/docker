#ifndef SOCKETLITE_SOCKET_SOURCE_H
#define SOCKETLITE_SOCKET_SOURCE_H
#include "SL_Socket_Runner.h"
#include "SL_Log.h"

class SOCKETLITE_API SL_Socket_Source
{
public:
    SL_Socket_Source()
        : recvbuffer_size_(16384)
        , msgbuffer_size_(4096)
        , msglen_bytes_(4)
        , msg_byteorder_(0)
        , is_addhandler_(true)
		, socket_handler_(0)
		, socket_runner_(0)
		, log_(0)
    {
    }

    virtual ~SL_Socket_Source() 
    {
    }

    //SL_TcpClient需要的特定函数
    virtual int handle_disconnect() 
    { 
        return 0; 
    }
    virtual int send(const char *buf, int len, int flag) 
    { 
        return 0; 
    }
    virtual int recv(char *buf, int len, int flag) 
    { 
        return 0; 
    }

	inline SL_Socket_Handler* get_socket_handler()
	{
	    return socket_handler_;
	}

    inline SL_Socket_Runner* get_socket_runner()
    {
        return socket_runner_;
    }

    inline SL_Log* get_log()
    {
        return log_;
    }

    inline uint get_recvbuffer_size() const
    {
        return recvbuffer_size_;
    }

    inline uint get_msgbuffer_size() const
    {
        return msgbuffer_size_;
    }

    inline uint8 get_msglen_bytes() const
    {
        return msglen_bytes_;
    }

    inline uint8 get_msg_byteorder() const
    {
        return msg_byteorder_;
    }

    inline bool get_is_addhander() const
    {
        return is_addhandler_;
    }

protected:
    uint                recvbuffer_size_;
    uint                msgbuffer_size_;
    uint8               msglen_bytes_;      //消息长度域所占字节数
    uint8               msg_byteorder_;     //消息的字节顺序(0:host-byte,1:big endian(network-byte) 2:little endian)

    bool                is_addhandler_;     //新连接是否加入Socket_Runner中，进行事件分派

	SL_Socket_Handler   *socket_handler_;
	SL_Socket_Runner    *socket_runner_;
    SL_Log              *log_;
};

#endif
