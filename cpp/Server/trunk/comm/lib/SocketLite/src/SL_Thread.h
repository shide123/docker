#ifndef SOCKETLITE_THREAD_H
#define SOCKETLITE_THREAD_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	#pragma once
#endif
#include "SL_Sync_Mutex.h"
#include "SL_Sync_Guard.h"

#if defined(SOCKETLITE_OS_WINDOWS)
#include <process.h>
typedef uint	SL_THREAD_ID;
typedef HANDLE	SL_THREAD_HANDLE;

typedef unsigned int (__stdcall *SL_ThreadProc)(void *arg);
template <typename TSyncMutex = SL_Sync_NullMutex>
class SL_Thread
{
public:
	SL_Thread()
		: thread_id_(0)
		, thread_handle_(NULL)
		, started_(false)
		, running_(false)
	{
	}

	~SL_Thread()
	{        
	}

	inline int clear()
	{
		mutex_.lock();
		if (started_)
		{
            if (running_)
            {
			    running_ = false;
			    this->join();
			    ::CloseHandle(thread_handle_);
            }
            thread_handle_  = NULL;
            started_        = false;
		}
        mutex_.unlock();
		return 0;
	}

	inline void selfclear()
	{
		running_=false;
		thread_handle_= NULL;
		started_= false;
	}

	int start(SL_ThreadProc threadproc, void *arg)
	{
		mutex_.lock();
		if (started_)
        {
		    mutex_.unlock();
			return 1;
        }

		started_ = true;
		running_ = true;
		thread_handle_ = reinterpret_cast<SL_THREAD_HANDLE>(::_beginthreadex(NULL, 0, threadproc, arg, 0, &thread_id_));
		if (NULL == thread_handle_)
        {
		    started_ = false;
		    running_ = false;
		    mutex_.unlock();
			return -1;
        }
		mutex_.unlock();
		return 0;
	}

	inline void stop()
	{
		mutex_.lock();
		running_ = false;
		mutex_.unlock();
	}

	inline void join()
	{
        ::WaitForSingleObject(thread_handle_, INFINITE);
	}

	inline int cancel()
	{
		int res = ::TerminateThread(thread_handle_,0) ? 0:-1;
		return res;
	}

	inline int kill()
	{
		return this->cancel();
	}

    inline int exit(unsigned int exit_code=0)
    {
        ::_endthreadex(exit_code);
        return 0;
    }

	inline bool get_running() const
	{
		return running_;
	}

	inline uint thread_id() const
	{
		return thread_id_;
	}

	inline SL_HANDLE* handle() const
	{
		return &thread_handle_;
	}

	inline TSyncMutex& mutex()
	{
	    return mutex_;
	}

	static void self(SL_THREAD_HANDLE &handle)
	{
		handle = ::GetCurrentThread();
	}

	static SL_THREAD_ID self()
	{
		return ::GetCurrentThreadId();
	}

	static void sleep(uint timeout)
	{//timeout: ms
		::Sleep(timeout);
	}

protected:
	SL_THREAD_ID		thread_id_;
	SL_THREAD_HANDLE    thread_handle_;
	bool                started_;
	bool                running_;

	TSyncMutex          mutex_;
};

#elif defined(SOCKETLITE_HAVE_PTHREADS)
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
typedef pthread_t	SL_THREAD_ID;
typedef pthread_t	SL_THREAD_HANDLE;
typedef void* (*SL_ThreadProc)(void *arg);

template <typename TSyncMutex = SL_Sync_NullMutex>
class SL_Thread
{
public:

	SL_Thread()
		: thread_id_(0)
		, thread_handle_(0)
		, started_(false)
		, running_(false)
	{
	}

	virtual ~SL_Thread()
	{
	}

	inline int clear()
	{
		mutex_.lock();
		if (started_)
		{
            if (running_)
            {
			    running_ = false;
                this->join();
            }
            thread_handle_  = 0;
            started_        = false;
		}
		mutex_.unlock();
		return 0;
	}

	inline void selfclear()
	{
		running_=false;
		thread_handle_= 0;
		started_= false;
	}

	int start(SL_ThreadProc threadproc, void *arg)
	{
		mutex_.lock();
		if (started_)
        {
		    mutex_.unlock();
			return 1;
        }

		started_ = true;
		running_ = true;
		int res = ::pthread_create(&thread_handle_, NULL, threadproc, arg);
		if (res != 0)
        {
		    started_ = false;
		    running_ = false;
		    mutex_.unlock();
			return -1;
        }
		mutex_.unlock();
		return 0;
	}

	inline void stop()
	{
		mutex_.lock();
		running_ = false;
		mutex_.unlock();
	}
	inline void join()
	{
		::pthread_join(thread_handle_, NULL);
	}

	inline int cancel()
	{
		return ::pthread_cancel(thread_handle_);
	}

	inline int kill()
	{
		return ::pthread_kill(thread_handle_, 0);
	}

    inline int exit(unsigned int exit_code=0)
    {
        ::pthread_exit(NULL);
        return 0;
    }

    inline uint thread_id() const
    {
        return thread_id_;
    }

	inline bool get_running() const
	{
        return running_;
	}

	inline SL_HANDLE& handle()
	{
		return thread_handle_;
	}

	inline TSyncMutex& mutex()
	{
		return mutex_;
	}

	static void self(SL_THREAD_HANDLE &handle)
	{
		handle = ::pthread_self();
	}

	static SL_THREAD_ID self()
	{
		return ::pthread_self();
	}

	static void sleep(uint timeout)
	{//timeout: ms

        //方法一
        usleep(timeout*1000);

        //方法二
        //struct timespec ts;
        //ts.tv_sec	 = timeout/1000;
        //ts.tv_nsec = (timeout%1000)*1000000L;
        //nanosleep(&ts, 0);
	}

protected:
	SL_THREAD_ID		thread_id_;
	SL_THREAD_HANDLE	thread_handle_;
	bool                started_;
	bool                running_;

	TSyncMutex          mutex_;
};

#endif  //SOCKETLITE_OS_WINDOWS

#endif  //SOCKETLITE_THREAD_H
