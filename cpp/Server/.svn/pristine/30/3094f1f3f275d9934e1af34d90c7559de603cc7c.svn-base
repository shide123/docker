#ifndef SOCKETLITE_SOCKET_EPOLL_RUNNER_H
#define SOCKETLITE_SOCKET_EPOLL_RUNNER_H

#include "SL_Config.h"
#include "SL_ByteBuffer.h"
#include "SL_Socket_CommonAPI.h"
#include "SL_Socket_Runner.h"
#include "SL_Socket_TcpServer.h"
#include "SL_Socket_TcpServer_Handler.h"
#include "SL_Sync_Mutex.h"
#include "SL_Thread.h"

#ifdef SOCKETLITE_OS_LINUX
#include <sys/epoll.h>

template <class TSyncMutex>
class SOCKETLITE_API SL_Socket_Epoll_Runner : public SL_Socket_Runner
{
public:
	SL_Socket_Epoll_Runner(int max_events=1024, int epoll_mode=ET_MODE)
		: max_events_(max_events)
        , current_handle_size_(0)
		, epoll_mode_(epoll_mode)
		, events_(NULL)
	{
        set_buffer_size();
	}

	~SL_Socket_Epoll_Runner()
	{
		close();
	}

    int set_buffer_size(int recv_buffer_size=16384, int send_buffer_size=16384)
    {
        recv_buffer_.reserve(recv_buffer_size);
        return 0;
    }

    int get_recv_buffer_size() const
    {
        return recv_buffer_.buffer_size();
    }

    char* get_recv_buffer()
    {
        return recv_buffer_.buffer();
    }

	int open(int event_mask=SL_Socket_Handler::READ_EVENT_MASK, int max_size=100000, int max_wait_time=200, int thread_number=0)
	{
        close();
		if (max_size <= 0)
		{
		    return -1;
		}

        epoll_handle_ = epoll_create(max_size);
        if (epoll_handle_ < 0)
        {
            return -2;
        }
        if (max_events_ > max_size)
        {
            max_events_ = max_size;
        }
        events_         = new epoll_event[max_events_];
        event_mask_     = event_mask;
        max_size_       = max_size;
        max_wait_time_  = max_wait_time;
        return 0;
	}

	int close()
	{
        current_handle_size_ = 0;
        if (NULL != events_)
        {
            delete []events_;
            events_ = NULL;
        }
        ::close(epoll_handle_);
        if (event_loop_thread_.get_running())
        {
            event_loop_thread_.stop();
            event_loop_thread_.join();
        }
		return 0;
	}

	int add_handle(SL_Socket_Handler *socket_handler, int event_mask)
	{
        if (socket_handler->socket_ < 0)
        {
            return -1;
        }
		if (!(event_mask & SL_Socket_Handler::ALL_EVENT_MASK & event_mask_))
		{
		    return -2;
		}

        mutex_.lock();
        if (socket_handler->event_mask_ != SL_Socket_Handler::NULL_EVENT_MASK)
        {
            mutex_.unlock();
            return -3;
        }
        if (current_handle_size_ >= max_size_)
        {
            mutex_.unlock();
            return -4;
        }
        struct epoll_event ev;
        memset(&ev, 0, sizeof(epoll_event));
        ev.data.ptr = socket_handler;
        if (epoll_mode_ == ET_MODE)
        {
            ev.events = EPOLLET;
        }
        int temp_event_mask = SL_Socket_Handler::NULL_EVENT_MASK;
        if (event_mask & SL_Socket_Handler::READ_EVENT_MASK & event_mask_)
        {
            ev.events |= EPOLLIN;
            temp_event_mask |= SL_Socket_Handler::READ_EVENT_MASK;
        }
        if (event_mask & SL_Socket_Handler::WRITE_EVENT_MASK & event_mask_)
        {
            ev.events |= EPOLLOUT;
            temp_event_mask |= SL_Socket_Handler::WRITE_EVENT_MASK;
        }    
        if (event_mask & SL_Socket_Handler::EXCEPT_EVENT_MASK & event_mask_)
        {    
            ev.events |= EPOLLERR;
            temp_event_mask |= SL_Socket_Handler::EXCEPT_EVENT_MASK;
        }
        if (epoll_ctl(epoll_handle_, EPOLL_CTL_ADD, socket_handler->socket_, &ev) < 0)
        {
            mutex_.unlock();
            return -5;
        }
        socket_handler->event_mask_ = temp_event_mask;
        ++current_handle_size_;
        mutex_.unlock();
		return 0;
	}

	int del_handle(SL_Socket_Handler *socket_handler)
	{
        if (socket_handler->socket_ < 0)
        {
            return -1;
        }

        mutex_.lock();
        if (SL_Socket_Handler::NULL_EVENT_MASK == socket_handler->event_mask_)
        {
            mutex_.unlock();
            return -2;
        }
        struct epoll_event ev;
        memset(&ev, 0, sizeof(epoll_event));
        ev.events   = socket_handler->event_mask_;
        ev.data.ptr = socket_handler;
        if (epoll_ctl(epoll_handle_, EPOLL_CTL_DEL, socket_handler->socket_, &ev) < 0)
        {
            mutex_.unlock();
            return -3;
        }
        socket_handler->event_mask_ = SL_Socket_Handler::NULL_EVENT_MASK;
        --current_handle_size_;
        mutex_.unlock();

        //作一些清理工作
        SL_Socket_Handler *source_handler = socket_handler->socket_source_->get_socket_handler();
        if (socket_handler->handle_close() >= 0)
        {
            SL_Socket_CommonAPI::socket_close(socket_handler->socket_);
            socket_handler->socket_ = SL_INVALID_SOCKET;
            if (source_handler != socket_handler)
            {
                ((SL_Socket_TcpServer_Handler*)source_handler)->delete_handler(socket_handler);
            }
            else
            {
                source_handler->socket_source_->handle_disconnect();
            }
        }
        else 
        {
            SL_Socket_CommonAPI::socket_close(socket_handler->socket_);
            socket_handler->socket_ = SL_INVALID_SOCKET;
            if (source_handler == socket_handler)
            {
                source_handler->socket_source_->handle_disconnect();
            }
        }
		return 0;
	}

    int remove_handle(SL_Socket_Handler *socket_handler)
    {
        if (socket_handler->socket_ < 0)
        {
            return -1;
        }

        mutex_.lock();
        if (SL_Socket_Handler::NULL_EVENT_MASK == socket_handler->event_mask_)
        {
            mutex_.unlock();
            return -2;
        }
        struct epoll_event ev;
        memset(&ev, 0, sizeof(epoll_event));
        ev.events   = socket_handler->event_mask_;
        ev.data.ptr = socket_handler;
        if (epoll_ctl(epoll_handle_, EPOLL_CTL_DEL, socket_handler->socket_, &ev) < 0)
        {
            mutex_.unlock();
            return -3;
        }
        socket_handler->event_mask_ = SL_Socket_Handler::NULL_EVENT_MASK;
        --current_handle_size_;
        mutex_.unlock();
        return 0;
    }

	int set_event_mask(SL_Socket_Handler *socket_handler, int event_mask)
	{
        if (socket_handler->socket_ < 0)
        {
            return -1;
        }
        if (!(event_mask & SL_Socket_Handler::ALL_EVENT_MASK & event_mask_))
        {
            return -2;
        }

        mutex_.lock();
        struct epoll_event ev;
        memset(&ev, 0, sizeof(epoll_event));
        ev.data.ptr = socket_handler;
        if (epoll_mode_ == ET_MODE)
        {
            ev.events = EPOLLET;
        }
        int temp_event_mask = SL_Socket_Handler::NULL_EVENT_MASK;
        if (event_mask & SL_Socket_Handler::READ_EVENT_MASK & event_mask_)
        {
            ev.events |= EPOLLIN;
            temp_event_mask |= SL_Socket_Handler::READ_EVENT_MASK;
        }
        if (event_mask & SL_Socket_Handler::WRITE_EVENT_MASK & event_mask_)
        {
            ev.events |= EPOLLOUT;
            temp_event_mask |= SL_Socket_Handler::WRITE_EVENT_MASK;
        }    
        if (event_mask & SL_Socket_Handler::EXCEPT_EVENT_MASK & event_mask_)
        {    
            ev.events |= EPOLLERR;
            temp_event_mask |= SL_Socket_Handler::EXCEPT_EVENT_MASK;
        }
        if (epoll_ctl(epoll_handle_, EPOLL_CTL_MOD, socket_handler->socket_, &ev) < 0)
        {
            mutex_.unlock();
            return -3;
        }
        socket_handler->event_mask_ = temp_event_mask;
        mutex_.unlock();
        return 0;
    }

    size_t get_handler_size() const
    {
        return current_handle_size_;
    }

    int event_loop(int wait_time=50)
	{
		if (wait_time > max_wait_time_)
        {
			wait_time = max_wait_time_;
        }
		int res = epoll_wait(epoll_handle_, events_, max_events_, wait_time);
        if (res > 0)
        {
		    int event_mask;
            SL_Socket_Handler *socket_handler;
		    for (int i=0; i<res; ++i)
		    {       
                event_mask = SL_Socket_Handler::NULL_EVENT_MASK;
			    socket_handler = (SL_Socket_Handler*)events_[i].data.ptr;
			    if (events_[i].events & EPOLLIN)
                {
				    event_mask |= SL_Socket_Handler::READ_EVENT_MASK;
                }
			    if (events_[i].events & EPOLLOUT)
                {
				    event_mask |= SL_Socket_Handler::WRITE_EVENT_MASK;
                }
			    if (events_[i].events & EPOLLERR)
                {
			        event_mask |= SL_Socket_Handler::EXCEPT_EVENT_MASK;
                }
			    if (socket_handler->handle_event(event_mask) < 0)
			    {
			        del_handle(socket_handler);
			    }
		    }
        }
	    return res;
	}

    int event_dispatch()
    {
        return event_loop(-1);
    }

    int thread_event_loop(int wait_time=50)
    { 
        wait_time_ = wait_time;
        return event_loop_thread_.start(event_loop_proc, this);
    }

    int thread_wait()
    {
        event_loop_thread_.join();
        return 0;
    }

    int thread_stop() 
    { 
        event_loop_thread_.stop();
        return 0;
    }

    static void* event_loop_proc(void *arg)
    {
		SL_Socket_Epoll_Runner<TSyncMutex> *runner =(SL_Socket_Epoll_Runner<TSyncMutex>*)arg;
	    while (1)
	    {
            if (!runner->event_loop_thread_.get_running())
            {
                break;
            }
            runner->event_loop(runner->wait_time_);
        }
        return 0;
    }

private:
    enum
    {
        LT_MODE = 1,
        ET_MODE = 2
    };

    int     event_mask_;
    int     max_size_;
    int     max_events_;
    int     max_wait_time_;
    int     wait_time_;
    size_t  current_handle_size_;

    int     epoll_mode_;
    int     epoll_handle_;
    struct  epoll_event *events_;

    TSyncMutex                      mutex_;
    SL_Thread<SL_Sync_ThreadMutex>  event_loop_thread_;
    SL_ByteBuffer                   recv_buffer_;    
};

#endif

#endif
