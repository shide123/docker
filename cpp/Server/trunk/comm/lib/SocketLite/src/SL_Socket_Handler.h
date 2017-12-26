#ifndef SOCKETLITE_SOCKET_HANDLER_H
#define SOCKETLITE_SOCKET_HANDLER_H

#include "SL_Config.h"
#include "SL_Socket_CommonAPI.h"

class SL_Socket_Source;
class SL_Socket_Runner;
template <typename TSyncMutex> class SL_Socket_Select_Runner;
template <typename TSyncMutex> class SL_Socket_Poll_Runner;
template <typename TSyncMutex> class SL_Socket_Epoll_Runner;
template <typename TSyncMutex> class SL_Socket_Iocp_Runner;
template <typename TSyncMutex> class SL_Socket_kqueue_Runner;
template <typename TSocketRunner> class SL_Socket_Runner_Group;

class SL_Socket_Handler
{
public:
	enum
	{
		NULL_EVENT_MASK     = 0x00000000,
		READ_EVENT_MASK     = 0x00000001,
		WRITE_EVENT_MASK    = 0x00000002,
		EXCEPT_EVENT_MASK   = 0x00000004,
		TIMEOUT_EVENT_MASK  = 0x00000008,
		ALL_EVENT_MASK      = READ_EVENT_MASK | WRITE_EVENT_MASK | EXCEPT_EVENT_MASK  | TIMEOUT_EVENT_MASK
	};

	inline SL_Socket_Handler()
        : socket_(SL_INVALID_SOCKET)
		, socket_source_(NULL)
		, socket_runner_(NULL)
		, event_mask_(NULL_EVENT_MASK)
        , runner_pos_(-1)
	{
	}

	virtual ~SL_Socket_Handler() 
    {
    }

	inline SL_SOCKET get_socket() const
    {
	    return socket_;
	}

	inline void set_socket(SL_SOCKET fd)
    {
        socket_ = fd;
	}

    inline void set_socket_source(SL_Socket_Source *socket_source)
    {
        socket_source_ = socket_source;
    }

    inline void set_socket_runner(SL_Socket_Runner *socket_runner)
    {
        socket_runner_ = socket_runner;
    }

	inline SL_Socket_Source* get_socket_source() const
	{
		return socket_source_;
	}

	inline SL_Socket_Runner* get_socket_runner() const
	{
	    return socket_runner_;
	}

    virtual int handle_open(SL_SOCKET fd, SL_Socket_Source *socket_source, SL_Socket_Runner *socket_runner)
    {
        socket_ = fd;
        socket_source_ = socket_source;
        socket_runner_ = socket_runner;
        SL_Socket_CommonAPI::socket_set_block(fd, false);
        return 0;
    }

    virtual int handle_close() 
    { 
        return 0; 
    }

	virtual int handle_event(int event_mask) 
    { 
        return 0;
    }

    virtual int handle_read()
    { 
        return 0; 
    }

    virtual int handle_read(const char *buf, int len) 
    { 
        return 0; 
    }

    virtual int handle_write()
    { 
        return 0; 
    }

    //tcpclient连接前，预处理事件
    virtual int handle_connecting() 
    { 
        return 0; 
    }

    //tcpclient连接成功后，通知事件
    virtual int handle_connect() 
    { 
        return 0; 
    }

	//tcpclient连接失败，处理事件
	virtual int handle_connecterr()
	{
		return 0;
	}

    int write_data(const char *buf, int len);
protected:
	SL_SOCKET		    socket_;
	SL_Socket_Source    *socket_source_;
	SL_Socket_Runner    *socket_runner_;

private:
    int                 event_mask_;    //事件码(上层不能修改)
	int                 runner_pos_;    //在socket_runner_中位置(上层不能修改)

	template <typename TSyncMutex> friend class SL_Socket_Select_Runner;
    template <typename TSyncMutex> friend class SL_Socket_Poll_Runner;
	template <typename TSyncMutex> friend class SL_Socket_Epoll_Runner;
    template <typename TSyncMutex> friend class SL_Socket_Iocp_Runner;
    template <typename TSyncMutex> friend class SL_Socket_kqueue_Runner;
    template <typename TSocketRunner> friend class SL_Socket_Runner_Group;
};

#endif
