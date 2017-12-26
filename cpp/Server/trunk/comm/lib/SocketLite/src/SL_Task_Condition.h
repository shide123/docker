#ifndef SOCKETLITE_TASK_CONDITION_H
#define SOCKETLITE_TASK_CONDITION_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	#pragma once
#endif
#include "SL_Config.h"
#include "SL_Sync_Condition.h"
#include "SL_Thread_Group.h"
#include "SL_Queue.h"

template <typename T>
class SL_Task_Condition
{
public:
    SL_Task_Condition()
    {
    }

    virtual ~SL_Task_Condition()
    {
        close();
    }

    int open(int thread_number = 4, int max_queue_size = 10000, int batch_node_count = 10)
	{
        batch_node_count_ = batch_node_count;
        queue_.init(max_queue_size);
        return thread_group_.start(svc_run, this, thread_number, thread_number);
	}

    int close()
    {
        thread_group_.stop();
        size_t thread_count = thread_group_.get_thread_count();
		for (size_t i=0; i<thread_count; ++i)
        {
            condition_.signal();
        }
        thread_group_.join();
        queue_.clear();
        return 0;
    }

    inline int getq(T &t, int thread_index)
    {
        return queue_.pop_front(t);
    }

    inline int putq(T &t, int thread_index=-1)
    {
        int res = queue_.push_back(t);
        if (res == 1)
        {//队列为空
            condition_.signal();
        }
        else if (res < 0)
        {
            return -1;
        }
        return 0;
    }

    virtual int init_svc_run()
    {
        return 0;
    }

    virtual int fini_svc_run()
    {
        return 0;
    }

    virtual void *svc_data()
    {
        return NULL;
    }

    virtual int svc(T &t, void *svc_data = NULL, bool *change_svc_data = NULL)
    {
        return 0;
    }

private:
#ifdef SOCKETLITE_OS_WINDOWS
	static unsigned int WINAPI svc_run(void *arg)
#else
    static void* svc_run(void *arg)
#endif
	{// Routine that runs the task routine as a daemon thread.
		SL_Task_Condition<T> *task =(SL_Task_Condition<T>*)arg;
        if (task->init_svc_run() < 0)
        {
            return 0;
        }

        uint i = 0;
        uint pop_node_count = 0;
        bool change_svc_data = false;
        void *svc_data = task->svc_data();

        SL_Sync_ThreadMutex &mutex = task->queue_.mutex();
        T *node =  new T[task->batch_node_count_];

		while (1)
		{
            if (!task->thread_group_.get_running())
            {
                delete []node;
                task->fini_svc_run();
                return 0;
            }

            mutex.lock();
			while (task->queue_.pop_front_i(node, task->batch_node_count_, pop_node_count) < 0)
            {
                task->condition_.wait(&mutex);
                if (!task->thread_group_.get_running())
                {
                    mutex.unlock();
                    delete []node;
                    task->fini_svc_run();
                    return 0;
                }
            }
            mutex.unlock();
			
            for (i=0; i<pop_node_count; ++i)
            {//处理消息
			    task->svc(node[i], svc_data, &change_svc_data);
                if (change_svc_data)
                {
                    change_svc_data = false;
                    svc_data = task->svc_data();
                }
            }
		}
        delete []node;
        task->fini_svc_run();
        return 0;
	}

private:
    SL_Sync_Condition   condition_;
    SL_Thread_Group     thread_group_;
    SL_Queue<T, SL_Sync_ThreadMutex> queue_;
    uint                batch_node_count_;

};

#endif
