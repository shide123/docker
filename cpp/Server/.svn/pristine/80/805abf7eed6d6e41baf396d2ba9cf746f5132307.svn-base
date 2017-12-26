#ifndef SOCKETLITE_SYNC_MACROS_H
#define SOCKETLITE_SYNC_MACROS_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	#pragma once
#endif
#include "SL_Config.h"

#if defined(SOCKETLITE_OS_WINDOWS)
	typedef CRITICAL_SECTION				SL_SYNC_MUTEX_T;
	typedef SL_HANDLE						SL_SYNC_CONDTIION_T;
	#define SL_SYNC_THREADMUTEX(x)			::CRITICAL_SECTION(x)
	#define SL_SYNC_THREADMUTEX_INIT(x)		::InitializeCriticalSection(x)
	#define SL_SYNC_THREADMUTEX_DESTROY(x)	::DeleteCriticalSection(x)
	#define SL_SYNC_THREADMUTEX_LOCK(x)		::EnterCriticalSection(x)
	#define SL_SYNC_THREADMUTEX_UNLOCK(x)	::LeaveCriticalSection(x)
	#define SL_SYNC_THREADMUTEX_TRYLOCK(x)  ::TryEnterCriticalSection(x)
	#define SL_SYNC_THREADMUTEX_TRYLOCK_RETURN(x)	\
		BOOL result = ::TryEnterCriticalSection(x);	\
		if (FALSE == result)						\
			return false;							\
		else										\
			return true

#elif defined(SOCKETLITE_HAVE_PTHREADS)
	typedef pthread_mutex_t					SL_SYNC_MUTEX_T;
	typedef pthread_cond_t					SL_SYNC_CONDTIION_T;
	#define SL_SYNC_THREADMUTEX(x)			pthread_mutex_t(x)
	//#define SL_SYNC_THREADMUTEX_INIT(x)		pthread_mutex_init(x, 0)
    #define SL_SYNC_THREADMUTEX_INIT(x)    do { pthread_mutexattr_t    attr; \
	                                            pthread_mutexattr_init(&attr); \
												pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP); \
												pthread_mutex_init(x, &attr);\
												pthread_mutexattr_destroy(&attr); }while(0) 
	#define SL_SYNC_THREADMUTEX_DESTROY(x)	pthread_mutex_destroy(x)
	#define SL_SYNC_THREADMUTEX_LOCK(x)		pthread_mutex_lock(x)
	#define SL_SYNC_THREADMUTEX_UNLOCK(x)	pthread_mutex_unlock(x)
	#define SL_SYNC_THREADMUTEX_TRYLOCK(x)  pthread_mutex_trylock(x)
	#define SL_SYNC_THREADMUTEX_TRYLOCK_RETURN(x)	\
		int rc = pthread_mutex_trylock(x);			\
		return (rc == 0)

#endif

#endif //SOCKETLITE_SYNC_MACROS_H
