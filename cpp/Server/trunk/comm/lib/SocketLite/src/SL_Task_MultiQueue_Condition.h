#ifndef SOCKETLITE_TASK_MULTIQUEUE_CONDITION_H
#define SOCKETLITE_TASK_MULTIQUEUE_CONDITION_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	#pragma once
#endif
#include "SL_Config.h"
#include "SL_Random.h"
#include "SL_Sync_Condition.h"
#include "SL_Thread.h"
#include "SL_Queue.h"
#include <vector>

//可保证处理顺序的SL_Task
template <typename T>
class SL_Task_MultiQueue_Condition
{
public:
    SL_Task_MultiQueue_Condition() 
    {
    }

    virtual ~SL_Task_MultiQueue_Condition()
    {
        close();
    }

    int open(int thread_number = 4, int queue_max_size = 10000, int batch_node_count = 10)
	{
        if (thread_number < 1)
        {
            thread_number = 1;
        }
        batch_node_count_ = batch_node_count;
        work_threads_.reserve(thread_number);

        WorkThread *work_thread;
        for (int i=0; i<thread_number; ++i)
        {
            work_thread = new WorkThread;        
            work_thread->parent = this;
            work_thread->index  = i;
            work_thread->loadfactor = 0;
            work_thread->queue.init(queue_max_size);
            work_thread->thread.start(svc_run, work_thread);
            work_threads_.push_back(work_thread);
        }
        return 0;
	}
    int close()
    {
        WorkThread *work_thread;
        typename std::vector<WorkThread*>::iterator iter = work_threads_.begin();
        for (; iter!=work_threads_.end(); ++iter)
        {
            work_thread = *iter;
            work_thread->thread.stop();            
            work_thread->condition.signal();
            work_thread->thread.join();
            work_thread->queue.clear();
            delete work_thread;
        }
        work_threads_.clear();
        return 0;
    }

    virtual int assign_thread()
    {
        int thread_size = work_threads_.size();
        if (thread_size == 1)
        {
            return 0;
        }

        //分配给负荷最小的线程,从而整体上使每个线程负荷平均
        int res_index = 0;
        int min_queue_size = work_threads_[0]->queue.size();
        int queue_size;
        for (int i=1; i<thread_size; ++i)
        {
            queue_size = work_threads_[i]->queue.size();
            if (queue_size < min_queue_size)
            {
                min_queue_size = queue_size;
                res_index = i;
            }
        }
        return res_index;
    }

    inline int getq(T &t, int thread_index)
    {
        if (thread_index < 0)
        {
            return -1;
        }
        return work_threads_[index]->queue.pop_front(t);
    }

    inline int putq(const T &t, int thread_index=-1)
    {//1)若成功返回工作线程index 2)若失败返回负数
        if (thread_index < 0)
        {
            thread_index = assign_thread();
            if (thread_index < 0)
            {
                return -1;
            }
        }
        else
        {
            if (thread_index > work_threads_.size())
            {
                return -2;
            }
        }

        int res = work_threads_[thread_index]->queue.push_back(t);
        if (res == 1)
        {//队列为空
            work_threads_[thread_index]->condition.signal();
        }
        else if (res < 0)
        {
            return -1;
        }
        return thread_index;
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
        typename SL_Task_MultiQueue_Condition<T>::WorkThread *work_thread = (typename SL_Task_MultiQueue_Condition<T>::WorkThread *)arg;
        SL_Task_MultiQueue_Condition<T> *task = (SL_Task_MultiQueue_Condition<T> *)work_thread->parent;
        if (task->init_svc_run() < 0)
        {
            return 0;
        }

        uint i = 0;
        uint pop_node_count = 0;
        bool change_svc_data = false;
        void *svc_data = task->svc_data();
        SL_Sync_ThreadMutex &mutex = work_thread->queue.mutex();
        T *node =  new T[task->batch_node_count_];

		while (1)
		{
            if (!work_thread->thread.get_running())
            {
                delete []node;
                task->fini_svc_run();
                return 0;
            }

            mutex.lock();
			while (work_thread->queue.pop_front_i(node, task->batch_node_count_, pop_node_count) < 0)
            {
                work_thread->condition.wait(&mutex);
                if (!work_thread->thread.get_running())
                {
                    mutex.unlock();
                    delete []node;
                    task->fini_svc_run();
                    return 0;
                }
            }
            mutex.unlock();

            for (i=0; i<pop_node_count; ++i)
            {
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
    struct WorkThread
    {
        void    *parent;            
        uint    index;              //线程在线程集合中索引
        uint    loadfactor;         //负载因子

        SL_Sync_Condition condition;
        SL_Thread<SL_Sync_NullMutex> thread;
        SL_Queue<T, SL_Sync_ThreadMutex> queue;
    };

    std::vector<WorkThread*> work_threads_;
    uint batch_node_count_;

};

#endif
