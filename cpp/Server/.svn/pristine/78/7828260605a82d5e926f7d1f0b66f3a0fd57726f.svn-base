#ifndef SOCKETLITE_SOCKET_POLL_RUNNER_H
#define SOCKETLITE_SOCKET_POLL_RUNNER_H

#include "SL_Config.h"
#include "SL_Socket_Runner.h"
#include "SL_Socket_CommonAPI.h"
#include "SL_Socket_TcpServer.h"
#include "SL_Socket_TcpServer_Handler.h"
#include "SL_Sync_Mutex.h"
#include "SL_Thread.h"
#include "SL_ByteBuffer.h"
#include <algorithm>
#include <map>
#include <vector>

#ifndef SOCKETLITE_OS_WINDOWS
#include <poll.h>

template <typename TSyncMutex>
class SOCKETLITE_API SL_Socket_Poll_Runner : public SL_Socket_Runner 
{
protected:
    inline short to_pollevent_i(int event_mask)
    {
        int poll_event = SL_Socket_Handler::NULL_EVENT_MASK;
        if (event_mask & SL_Socket_Handler::READ_EVENT_MASK)
        {
            poll_event |= POLLIN;
        }
        if (event_mask & SL_Socket_Handler::WRITE_EVENT_MASK)
        {
            poll_event |= POLLOUT;
        }
        if (event_mask & SL_Socket_Handler::EXCEPT_EVENT_MASK)
        {
            poll_event |= POLLIN;
        }
        return poll_event;
    }

public:
    SL_Socket_Poll_Runner()
        : socket_pollfds_(NULL)
    {
        temp_handlers_active_   = &temp_handlers_1_;
        temp_handlers_standby_  = &temp_handlers_2_;
        set_buffer_size();
    }

    ~SL_Socket_Poll_Runner()
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

    int open(int event_mask=SL_Socket_Handler::READ_EVENT_MASK, int max_size=1024, int max_wait_time=200, int thread_number=0)
    {
        close();
        if (max_size <= 0)
        {
            return -1;
        }

        event_mask_     = event_mask;
        max_size_       = max_size;
        max_wait_time_  = max_wait_time;    

        temp_handlers_1_.clear();
        temp_handlers_2_.clear();
        socket_handlers_.clear();
        socket_handlers_.reserve(max_size*2);
        socket_pollfds_ = new pollfd[max_size];
        return 0;
    }

    int close()
    {
        if (NULL != socket_pollfds_)
        {
            delete []socket_pollfds_;
            socket_pollfds_ = NULL;
        }
        temp_handlers_1_.clear();
        temp_handlers_2_.clear();
        socket_handlers_.clear();

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
        if (socket_handlers_.size() >= (max_size_-temp_handlers_standby_->size()))
        {
            mutex_.unlock();
            return -4;
        }
        int temp_event_mask = SL_Socket_Handler::NULL_EVENT_MASK;
        if (event_mask & SL_Socket_Handler::READ_EVENT_MASK & event_mask_)
        {
            temp_event_mask |= SL_Socket_Handler::READ_EVENT_MASK;
        }
        if (event_mask & SL_Socket_Handler::WRITE_EVENT_MASK & event_mask_)
        {
            temp_event_mask |= SL_Socket_Handler::WRITE_EVENT_MASK;
        }
        if (event_mask & SL_Socket_Handler::EXCEPT_EVENT_MASK & event_mask_)
        {
            temp_event_mask |= SL_Socket_Handler::EXCEPT_EVENT_MASK;
        }
        socket_handler->event_mask_ = temp_event_mask;
        (*temp_handlers_standby_)[socket_handler] = ADD_HANDLE;
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
        socket_handler->event_mask_ = SL_Socket_Handler::NULL_EVENT_MASK;
        (*temp_handlers_standby_)[socket_handler] = DEL_HANDLE;
        mutex_.unlock();
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
        socket_handler->event_mask_ = SL_Socket_Handler::NULL_EVENT_MASK;
        (*temp_handlers_standby_)[socket_handler] = REMOVE_HANDLE;
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
        int temp_event_mask = SL_Socket_Handler::NULL_EVENT_MASK;
        if (socket_handler->event_mask_ & SL_Socket_Handler::READ_EVENT_MASK)
        {
            if (event_mask & SL_Socket_Handler::READ_EVENT_MASK & event_mask_)
            {
                temp_event_mask |= SL_Socket_Handler::READ_EVENT_MASK;
            }
        }
        else if (event_mask & SL_Socket_Handler::READ_EVENT_MASK & event_mask_)
        {
            temp_event_mask |= SL_Socket_Handler::READ_EVENT_MASK;
        }
        if (socket_handler->event_mask_ & SL_Socket_Handler::WRITE_EVENT_MASK)
        {
            if (event_mask & SL_Socket_Handler::WRITE_EVENT_MASK & event_mask_)
            {
                temp_event_mask |= SL_Socket_Handler::WRITE_EVENT_MASK;
            }
        }
        else if (event_mask & SL_Socket_Handler::WRITE_EVENT_MASK & event_mask_)
        {
            temp_event_mask |= SL_Socket_Handler::WRITE_EVENT_MASK;
        }
        if (socket_handler->event_mask_ & SL_Socket_Handler::EXCEPT_EVENT_MASK)
        {
            if (event_mask & SL_Socket_Handler::EXCEPT_EVENT_MASK & event_mask_)
            {
                temp_event_mask |= SL_Socket_Handler::EXCEPT_EVENT_MASK;
            }
        }
        else if (event_mask & SL_Socket_Handler::EXCEPT_EVENT_MASK & event_mask_)
        {
            temp_event_mask |= SL_Socket_Handler::EXCEPT_EVENT_MASK;
        }
        socket_handler->event_mask_ = temp_event_mask;
        (*temp_handlers_standby_)[socket_handler] = SET_EVENT_MASK;
        mutex_.unlock();
        return 0;
    }

    size_t get_handler_size() const
    {
        return socket_handlers_.size();
    }

    int event_loop(int wait_time=50)
    {
        SL_Socket_Handler *socket_handler;
        bool is_change = false;

        mutex_.lock();
        if (!temp_handlers_standby_->empty())
        {//交换active/standby指针(采用std::swap函数)
            std::swap(temp_handlers_standby_, temp_handlers_active_);
            is_change = true;
        }
        mutex_.unlock();

        if (is_change)
        {
            SL_Socket_Handler *end_handler;
            SL_Socket_Handler *source_handler;
            for (TEMP_SOCKET_HANDLES::iterator iter=temp_handlers_active_->begin(); iter!=temp_handlers_active_->end(); ++iter)
            {
                socket_handler = iter->first;
                if (iter->second == ADD_HANDLE)
                {
                    if (socket_handler->runner_pos_ < 0)
                    {
                        socket_handlers_.push_back(socket_handler);
                        socket_handler->runner_pos_ = socket_handlers_.size()-1;
                    }
                    socket_pollfds_[socket_handler->runner_pos_].fd      = socket_handler->socket_;
                    socket_pollfds_[socket_handler->runner_pos_].events  = to_pollevent_i(socket_handler->event_mask_);
                    socket_pollfds_[socket_handler->runner_pos_].revents = 0;
                }
                else if (iter->second == DEL_HANDLE)
                {
                    if (socket_handler->runner_pos_ < 0)
                    {
                        continue;
                    }
                    if (socket_handlers_.size() > 0)
                    {
                        if (socket_handler->runner_pos_ != (socket_handlers_.size()-1))
                        {
                            end_handler = socket_handlers_[socket_handlers_.size()-1];
                            socket_pollfds_[socket_handler->runner_pos_].fd = end_handler->socket_;
                            socket_pollfds_[socket_handler->runner_pos_].events = to_pollevent_i(end_handler->event_mask_);
                            end_handler->runner_pos_ = socket_handler->runner_pos_;
                            socket_handlers_[socket_handler->runner_pos_] = end_handler;
                        }
                        socket_handlers_.pop_back();
                    }
                    socket_handler->runner_pos_ = -1;
                    if (socket_handler->handle_close() >= 0)
                    {
                        source_handler = socket_handler->socket_source_->get_socket_handler();
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
                }
                else if (iter->second == REMOVE_HANDLE)
                {
                    if (socket_handler->runner_pos_ < 0)
                    {
                        continue;
                    }
                    if (socket_handlers_.size() > 0)
                    {
                        if (socket_handler->runner_pos_ != (socket_handlers_.size()-1))
                        {
                            end_handler = socket_handlers_[socket_handlers_.size()-1];
                            socket_pollfds_[socket_handler->runner_pos_].fd = end_handler->socket_;
                            socket_pollfds_[socket_handler->runner_pos_].events = to_pollevent_i(end_handler->event_mask_);
                            end_handler->runner_pos_ = socket_handler->runner_pos_;
                            socket_handlers_[socket_handler->runner_pos_] = end_handler;
                        }
                        socket_handlers_.pop_back();
                    }
                    socket_handler->runner_pos_ = -1;
                }
                else if (iter->second == SET_EVENT_MASK)
                {
                    socket_pollfds_[socket_handler->runner_pos_].fd = socket_handler->socket_;
                    socket_pollfds_[socket_handler->runner_pos_].events = to_pollevent_i(socket_handler->event_mask_);
                }
            }
            temp_handlers_active_->clear();
        }

        if (wait_time > max_wait_time_)
        {
            wait_time = max_wait_time_;
        }
        uint socket_pollfds_size = socket_handlers_.size();
        int res = poll(socket_pollfds_, socket_pollfds_size, wait_time);
        if (res > 0)
        {
            int  event_mask;
            for (uint i=0; i<socket_pollfds_size; ++i)
            {
                event_mask = SL_Socket_Handler::NULL_EVENT_MASK;
                if (socket_pollfds_[i].revents & POLLIN)
                {
                    event_mask |= SL_Socket_Handler::READ_EVENT_MASK;
                }
                if (socket_pollfds_[i].revents & POLLOUT)
                {
                    event_mask |= SL_Socket_Handler::WRITE_EVENT_MASK;
                }
                if (socket_pollfds_[i].revents & (POLLERR|POLLHUP)
                {
                    event_mask |= SL_Socket_Handler::EXCEPT_EVENT_MASK;
                }
                if (event_mask > SL_Socket_Handler::NULL_EVENT_MASK)
                {
                    if (socket_handlers_[i]->handle_event(event_mask) < 0)
                    {
                        del_handle(socket_handler);
                    }
                    --res;
                    if (res <= 0)
                    {
                        break;
                    }
                }
            }
        }
        return res;
    }

    size_t get_handler_size() const
    {
        return socket_handlers_.size();
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
        SL_Socket_Poll_Runner<TSyncMutex> *runner = (SL_Socket_Poll_Runner<TSyncMutex>*)arg;
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
    typedef std::vector<SL_Socket_Handler*>   SOCKET_HANDLES;
    typedef std::map<SL_Socket_Handler*, int> TEMP_SOCKET_HANDLES;
    enum
    {
        ADD_HANDLE      = 1,
        DEL_HANDLE      = 2,
        REMOVE_HANDLE   = 3,
        SET_EVENT_MASK  = 4
    };

    int event_mask_;
    int max_size_;
    int max_wait_time_;
    int wait_time_;

    TSyncMutex                      mutex_;
    SL_Thread<SL_Sync_ThreadMutex>  event_loop_thread_;
    SOCKET_HANDLES                  socket_handlers_;
    struct pollfd                   *socket_pollfds_;

    TEMP_SOCKET_HANDLES             temp_handlers_1_;
    TEMP_SOCKET_HANDLES             temp_handlers_2_;
    TEMP_SOCKET_HANDLES             *temp_handlers_active_;
    TEMP_SOCKET_HANDLES             *temp_handlers_standby_;

    SL_ByteBuffer                   recv_buffer_;
};

#endif

#endif
