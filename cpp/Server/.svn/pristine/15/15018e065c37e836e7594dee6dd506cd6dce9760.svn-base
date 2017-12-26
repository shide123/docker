#include "SL_Thread_Group.h"

SL_Thread_Group::SL_Thread_Group()
    : running_(false)
{
}

SL_Thread_Group::~SL_Thread_Group()
{
    clear();
}

int SL_Thread_Group::clear()
{
    for(std::vector<SL_Thread<SL_Sync_NullMutex>* >::iterator iter=threads_.begin(); iter!=threads_.end(); ++iter)
    {
        delete *iter;
    }
    threads_.clear();
    return 0;
}

int SL_Thread_Group::start(SL_ThreadProc threadproc, void *arg, int max_thread_num, int init_thread_num)
{
    if (threadproc == NULL)
    {
        return -1;
    }

    clear();
    if (max_thread_num <= 0)
        max_thread_num = 1;
    if (init_thread_num <= 0)
        init_thread_num = 0;
    if (init_thread_num > max_thread_num)
        init_thread_num = max_thread_num;
    threads_.reserve(max_thread_num);
    threadproc_ = threadproc;
    arg_ = arg;

    running_ = true;
    SL_Thread<SL_Sync_NullMutex> *thread;
    for (int i=0; i<init_thread_num; ++i)
    {
        thread = new SL_Thread<SL_Sync_NullMutex>();
        thread->start(threadproc, arg);
        threads_.push_back(thread);
    }
    return 0;
}

int SL_Thread_Group::stop()
{
    running_ = false;
    for(std::vector<SL_Thread<SL_Sync_NullMutex>* >::iterator iter=threads_.begin(); iter!=threads_.end(); ++iter)
    {
        (*iter)->stop();
    }
    return 0;
}

int SL_Thread_Group::join()
{
    for(std::vector<SL_Thread<SL_Sync_NullMutex>* >::iterator iter=threads_.begin(); iter!=threads_.end(); ++iter)
    {
        (*iter)->join();
    }
    return 0;
}

int SL_Thread_Group::exit()
{
    for(std::vector<SL_Thread<SL_Sync_NullMutex>* >::iterator iter=threads_.begin(); iter!=threads_.end(); ++iter)
    {
        (*iter)->exit();
    }
    return 0;
}
