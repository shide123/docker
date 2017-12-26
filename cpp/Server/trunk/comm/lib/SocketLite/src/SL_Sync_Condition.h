#ifndef SOCKETLITE_SYNC_CONDITION_H
#define SOCKETLITE_SYNC_CONDITION_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	#pragma once
#endif
#include "SL_Config.h"
#include "SL_Sync_Mutex.h"
#include "SL_Sync_Macros.h"

#if defined(SOCKETLITE_OS_WINDOWS)
class SL_Sync_Condition
{
private:
	//= Prevent assignment and initialization.
	// Disallow copying and assignment.
    SL_Sync_Condition(const SL_Sync_Condition &);
    void operator= (const SL_Sync_Condition &);
public:
	SL_Sync_Condition()
	{
        condition_ = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	~SL_Sync_Condition()
	{
	    CloseHandle(condition_);
	}

	inline int signal()
	{
		if (SetEvent(condition_))
        {
		    return 0;
        }
		return -1;
	}

    inline int broadcast()
    {
        return signal();
    }

	inline int wait()
    {
        DWORD res = WaitForSingleObject(condition_, INFINITE);
		if (res == WAIT_OBJECT_0)
		{
	        return 0;
		}
        return -1;
    }

    template <typename TSyncMutex>
	inline int wait(TSyncMutex *mutex)
	{
        mutex->unlock();
        DWORD res = WaitForSingleObject(condition_, INFINITE);
		if (res == WAIT_OBJECT_0)
		{
            mutex->lock();
	        return 0;
		}
        mutex->lock();
        return -1;
	}

	inline int timed_wait(ulong timeout) 
	{
        DWORD res = WaitForSingleObject(condition_, timeout);
		if (res == WAIT_TIMEOUT)
		{
            return -1;
        }
        else if (res == WAIT_OBJECT_0)
        {
            return 0;
        }
		return -2;
	}

    template <typename TSyncMutex>
	inline int timed_wait(TSyncMutex *mutex, ulong timeout) 
	{
        mutex->unlock();
        DWORD res = WaitForSingleObject(condition_, timeout);
        int res = -1;
		if (res == WAIT_TIMEOUT)
		{
            res = -1;
        }
        else if (res == WAIT_OBJECT_0)
        {
            res = 0;
        }
        mutex->lock();
		return res;
	}

	inline SL_SYNC_CONDTIION_T* handle()
	{
	    return &condition_;
	}

private:
	SL_SYNC_CONDTIION_T condition_;
};

#elif defined(SOCKETLITE_HAVE_PTHREADS)
#include <sys/time.h>
class SL_Sync_Condition
{
private:
	//= Prevent assignment and initialization.
	// Disallow copying and assignment.
    SL_Sync_Condition(const SL_Sync_Condition &);
    void operator= (const SL_Sync_Condition &);
public:
	SL_Sync_Condition()
	{
		pthread_cond_init(&condition_, 0);
	}

	~SL_Sync_Condition()
	{
		pthread_cond_destroy(&condition_);
	}

	inline int signal()
	{
		int res = pthread_cond_signal(&condition_);
		if (res == 0)
        {
			return 0;
        }
		return -1;
	}

    inline int broadcast()
    {
		int res = pthread_cond_broadcast(&condition_);
		if (res == 0)
        {
			return 0;
        }
		return -1;
    }
/*
	inline int wait()
	{
		int res = pthread_cond_wait(&condition_, NULL);
		if (res == 0)
		{
			return 0;
		}
		return -1;
	}
*/
    template <typename TSyncMutex>
	inline int wait(TSyncMutex *mutex)
	{
		int res = pthread_cond_wait(&condition_, mutex->handle());
		if (res == 0)
		{
			return 0;
		}
		return -1;
	}
/*
	inline int timed_wait(ulong timeout) 
    {
		int res = -1;
		if (timeout > 0)
		{
			timeval  nowtime;
			timespec abstime;
			gettimeofday(&nowtime, NULL);
			abstime.tv_sec	= timeout/1000 + nowtime.tv_sec;
			abstime.tv_nsec	= (timeout%1000)*1000000 + nowtime.tv_usec*1000;
			res = pthread_cond_timedwait(&condition_, NULL, &abstime);
		}
		else
		{
			res = pthread_cond_wait(&condition_, NULL);
		}
		if (res == 0)
        {
		    return 0;
        }
		else if (res == ETIMEDOUT) 
		{
			return -1;
		}
		return -2;
	}
*/
    template <typename TSyncMutex>
	inline int timed_wait(TSyncMutex *mutex, ulong timeout) 
    {
		int res = -1;
		if (timeout > 0)
		{
			timeval  nowtime;
			timespec abstime;
			gettimeofday(&nowtime, NULL);
			abstime.tv_sec	= timeout/1000 + nowtime.tv_sec;
			abstime.tv_nsec	= (timeout%1000)*1000000 + nowtime.tv_usec*1000;
			res = pthread_cond_timedwait(&condition_, mutex->handle(), &abstime);
		}
		else
		{
			res = pthread_cond_wait(&condition_, mutex->handle());
		}
		if (res == 0)
        {
		    return 0;
        }
		else if (res == ETIMEDOUT) 
		{
			return -1;
		}
		return -2;
	}

	inline SL_SYNC_CONDTIION_T* handle()
	{
		return &condition_;
	}

private:
	SL_SYNC_CONDTIION_T condition_;
};

#endif

#endif
