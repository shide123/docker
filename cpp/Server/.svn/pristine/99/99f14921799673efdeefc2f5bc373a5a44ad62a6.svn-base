#ifndef SOCKETLITE_SOCKET_TCPSERVER_HANDLER_H
#define SOCKETLITE_SOCKET_TCPSERVER_HANDLER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	#pragma once
#endif

#include "SL_Socket_CommonAPI.h"
#include "SL_Socket_Handler.h"
#include "SL_Socket_INET_Addr.h"
#include "SL_ObjectPool.h"
#include "SL_Sync_Mutex.h"
#include "SL_Socket_TcpServer.h"

class SL_Socket_TcpServer_Handler : public SL_Socket_Handler
{
public:
    SL_Socket_TcpServer_Handler() 
    {
    }

    virtual ~SL_Socket_TcpServer_Handler() 
    {
    }

    virtual int handle_close() 
    { 
        return -1; 
    }

    virtual int handle_accept() 
    { 
        return 0; 
    }

    virtual int handle_accept(SL_SOCKET fd, SL_Socket_INET_Addr &sl_addr) 
    { 
        return 0; 
    }

    virtual void* new_handler() 
    { 
        return NULL; 
    }

    virtual void  delete_handler(void *handler) 
    { 
        return; 
    }

protected:
	int handle_event(int event_mask)
    {
        return handle_accept();
    }
};

template <typename THandler, typename TObjectPool=SL_ObjectPool_SimpleEx<THandler,SL_Sync_ThreadMutex> >
class SL_Socket_TcpServer_Handler_Impl : public SL_Socket_TcpServer_Handler
{
public:
    SL_Socket_TcpServer_Handler_Impl(TObjectPool *object_pool)
        : object_pool_(object_pool)
    {
    }

    SL_Socket_TcpServer_Handler_Impl()
    {
    }

    virtual ~SL_Socket_TcpServer_Handler_Impl()
    {
    }

    void set_object_pool(TObjectPool *object_pool)
    {
        object_pool_ = object_pool;
    }

    int handle_close()
    { 
        return -1;
    }

    virtual int do_accept(SL_SOCKET fd, SL_Socket_INET_Addr &sl_addr)
    {
        return 0;
    }

    int handle_accept()
    { 
        SL_Socket_TcpServer *tcpserver = (SL_Socket_TcpServer*)socket_source_;
        SL_Socket_INET_Addr sl_addr(tcpserver->get_addr().is_ipv6());
        int addrlen = sl_addr.get_addr_size();

        SL_SOCKET fd = SL_Socket_CommonAPI::socket_accept(socket_, sl_addr.get_addr(), &addrlen);
        if (fd != SL_INVALID_SOCKET)
        {
            if (do_accept(fd, sl_addr) < 0)
            {
                SL_Socket_CommonAPI::socket_close(fd);
                return 0;
            }
            THandler* handler = object_pool_->get_object();
            if (handler != NULL)
            {
                if (handler->handle_open(fd, socket_source_, socket_runner_) < 0)
                {
                    object_pool_->put_object(handler);
                    SL_Socket_CommonAPI::socket_close(fd);
                    return 0;
                }
                if (socket_source_->get_is_addhander())
                {
                    if (socket_source_->get_socket_runner()->add_handle(handler, SL_Socket_Handler::READ_EVENT_MASK) < 0)
                    {
                        object_pool_->put_object(handler);
                        SL_Socket_CommonAPI::socket_close(fd);
                    }
                }
            }
            else
            {
                SL_Socket_CommonAPI::socket_close(fd);
            }
        }
        return 0;
    }

    int handle_accept(SL_SOCKET fd, SL_Socket_INET_Addr &sl_addr)
    { 
        if (do_accept(fd, sl_addr) < 0)
        {
            return -1;
        }
        THandler* handler = object_pool_->get_object();
        if (NULL != handler)
        {
            if (handler->handle_open(fd, socket_source_, socket_runner_) < 0)
            {
                object_pool_->put_object(handler);
                return -2;
            }
            if (socket_source_->get_is_addhander())
            {
                if (socket_source_->get_socket_runner()->add_handle(handler, SL_Socket_Handler::READ_EVENT_MASK) < 0)
                {
                    object_pool_->put_object(handler);
                    return -3;
                }
            }
        }
        else
        {
            return -4;
        }
        return 0;
    }

    void* new_handler()
    { 
        return object_pool_->get_object();
    }

    void delete_handler(void *handler)
    { 
        return object_pool_->put_object((THandler*)handler);
    }

private:
    TObjectPool *object_pool_;
};

#endif
