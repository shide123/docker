#ifndef SOCKETLITE_THREAD_GROUP_H
#define SOCKETLITE_THREAD_GROUP_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	#pragma once
#endif
#include "SL_Config.h"
#include "SL_Thread.h"
#include <vector>

class SOCKETLITE_API SL_Thread_Group
{
public:
	SL_Thread_Group();
	~SL_Thread_Group();
    int start(SL_ThreadProc threadproc, void *arg, int max_thread_num, int init_thread_num);
    int stop();
    int join();
    int clear();
    int exit();

    inline int get_thread_count() const
    {
        return (int)threads_.size();
    }

	inline bool get_running() const
    {
        return running_;
    }

private:
    //int set_threadnum(int thread_number);
    //int add_thread(int thread_number);
    //int del_thread(int thread_number);

protected:
    bool            running_;
    void            *arg_;
    SL_ThreadProc   threadproc_;
    std::vector<SL_Thread<SL_Sync_NullMutex>* > threads_;
};

#endif
