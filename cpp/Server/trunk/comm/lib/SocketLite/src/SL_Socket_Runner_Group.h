#ifndef SOCKETLITE_SOCKET_RUNNER_GROUP_H
#define SOCKETLITE_SOCKET_RUNNER_GROUP_H

#include "SL_Config.h"
#include "SL_Socket_Handler.h"
#include "SL_Socket_Runner.h"
#include "SL_ObjectPool.h"
#include "SL_Sync_Mutex.h"
#include <vector>

template <typename TRunner>
class SL_Socket_Runner_Group : public SL_Socket_Runner
{
public:
    SL_Socket_Runner_Group()
    {
    }

    ~SL_Socket_Runner_Group()
    {
        close();
        clear();
    }

    virtual TRunner* new_runner()
    {
        TRunner *runner;
        runner = new TRunner();
        return runner;
    }

    virtual void delete_runner(TRunner *runner)
    {
        delete runner;
    }

    virtual SL_Socket_Runner* assign_runner()
    {
        size_t runners_size = socket_runners_.size();
        if (runners_size == 1)
        {
            return socket_runners_[0];
        }

        //分配给负荷最小的runner,从而整体上使每个runner负荷平均
        size_t res_index = 0;
        size_t min_handle_size = socket_runners_[0]->get_handler_size();
        size_t handler_size;
        for (size_t i=1; i<runners_size; ++i)
        {
            handler_size = socket_runners_[i]->get_handler_size();
            if (handler_size < min_handle_size)
            {
                min_handle_size = handler_size;
                res_index = i;
            }
        }
        return socket_runners_[res_index];
    }

    int init(uint runner_num=10)
    {
        if (runner_num < 2)
        {
            runner_num = 2;
        }

        socket_runners_.reserve(runner_num);
        TRunner *runner;
        for (int i=0; i<runner_num; ++i)
        {
            runner = new_runner();
            socket_runners_.push_back(runner);
        }
        return 0;
    }

    int clear()
    {
        typename std::vector<TRunner*>::iterator iter = socket_runners_.begin();
        for (; iter!=socket_runners_.end(); ++iter)
        {
            delete_runner(*iter);
        }
        socket_runners_.clear();
        return 0; 
    }

    int set_buffer_size(int recv_buffer_size=16384, int send_buffer_size=16384) 
    { 
        TRunner *runner;
        typename std::vector<TRunner*>::iterator iter = socket_runners_.begin();
        for (; iter!=socket_runners_.end(); ++iter)
        {
            runner = *iter;
            runner->set_buffer_size(recv_buffer_size, send_buffer_size);
        }
        return 0; 
    }

    int open(int event_mask, int max_size, int max_wait_time, int thread_number) 
    { 
        TRunner *runner;
        typename std::vector<TRunner*>::iterator iter = socket_runners_.begin();
        for (; iter!=socket_runners_.end(); ++iter)
        {
            runner = *iter;
            runner->open(event_mask, max_size, max_wait_time, thread_number);
        }
        return 0; 
    }

    int close() 
    { 
        TRunner *runner;
        typename std::vector<TRunner*>::iterator iter = socket_runners_.begin();
        for (; iter!=socket_runners_.end(); ++iter)
        {
            runner = *iter;
            runner->close();
        }
        return 0; 
    }

	int add_handle(SL_Socket_Handler *socket_handler, int event_mask)
    {
        SL_Socket_Runner *runner = assign_runner();
        if (NULL != runner)
        {
            if (runner->add_handle(socket_handler, event_mask) >= 0)
            {
                socket_handler->socket_runner_ = runner;
                return 0;
            }
        }
        return -1;
    }

	int del_handle(SL_Socket_Handler *socket_handler)
    {
        SL_Socket_Runner *handler_runner = socket_handler->get_socket_runner();
        if (this != handler_runner)
        {
            return handler_runner->del_handle(socket_handler);
        }
        return 0;
    }

    int remove_handle(SL_Socket_Handler *socket_handler)
    {
        SL_Socket_Runner *handler_runner = socket_handler->get_socket_runner();
        if (this != handler_runner)
        {
            return handler_runner->remove_handle(socket_handler);
        }
        return 0;
    }

	int set_event_mask(SL_Socket_Handler *socket_handler, int event_mask)
    {
        SL_Socket_Runner *handler_runner = socket_handler->get_socket_runner();
        if (this != handler_runner)
        {
            return handler_runner->remove_handle(socket_handler);
        }
        return 0;
    }

	size_t get_handler_size()
    {
        size_t handler_size = 0;
        TRunner *runner;
        typename std::vector<TRunner*>::iterator iter = socket_runners_.begin();
        for (; iter!=socket_runners_.end(); ++iter)
        {
            runner = *iter;
            handler_size += runner->get_handler_size();
        }
        return handler_size;
    }

	int event_dispatch()
    {
        return event_loop(-1);
    }

	int event_loop(int wait_time)
    {
        TRunner *runner;
        typename std::vector<TRunner*>::iterator iter = socket_runners_.begin();
        for (; iter!=socket_runners_.end(); ++iter)
        {
            runner = *iter;
            runner->event_loop(wait_time);
        }
        return 0; 
    }

	int thread_event_loop(int wait_time)
    {
        TRunner *runner;
        typename std::vector<TRunner*>::iterator iter = socket_runners_.begin();
        for (; iter!=socket_runners_.end(); ++iter)
        {
            runner = *iter;
            runner->thread_event_loop(wait_time);
        }
        return 0;
    }

    int thread_wait() 
    { 
        TRunner *runner;
        typename std::vector<TRunner*>::iterator iter = socket_runners_.begin();
        for (; iter!=socket_runners_.end(); ++iter)
        {
            runner = *iter;
            runner->thread_wait();
        }
        return 0; 
    }

    int thread_stop() 
    { 
        TRunner *runner;
        typename std::vector<TRunner*>::iterator iter = socket_runners_.begin();
        for (; iter!=socket_runners_.end(); ++iter)
        {
            runner = *iter;
            runner->thread_stop();
        }
        return 0; 
    }

protected:
    std::vector<TRunner*> socket_runners_;
};

#endif
