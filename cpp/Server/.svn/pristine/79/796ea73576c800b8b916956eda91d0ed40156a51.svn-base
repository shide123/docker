
#ifndef __LOCK_DEF_H_20060109__
#define __LOCK_DEF_H_20060109__

#ifndef WIN32
#include <pthread.h>
#include <semaphore.h>
#else
#if defined(USE_PTHREAD)
#include <pthread.h>
#endif
#include <Windows.h>

#endif

#if defined(WIN32) && !defined (USE_PTHREAD)                             
#define LOCK_INIT(a)     InitializeCriticalSection(a)                
#define LOCK(a)          EnterCriticalSection(a)
#define UNLOCK(a)        LeaveCriticalSection(a)
#define LOCK_DESTROY(a)  DeleteCriticalSection (a)
#define SLEEPX(a)        Sleep(a)
#define LOCK_T	CRITICAL_SECTION
#define THREAD_T HANDLE

#define SEM_T    HANDLE
#define SEM_INIT(a)     do {a = CreateSemaphore(NULL, 0, 9999, "");} while (0);
#define SEM_POST(a)     ReleaseSemaphore(a, 1, 0)
#define SEM_WAIT(a)     WaitForSingleObject(a, INFINITE)
#define SEM_DESTROY(a)  CloseHandle(a)

#else    
#define LOCK_INIT(a)     do { pthread_mutexattr_t    attr; \
							 pthread_mutexattr_init(&attr); \
							 pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP); \
							 pthread_mutex_init(a, &attr);\
							 pthread_mutexattr_destroy(&attr); }while(0)                                        
#define LOCK(a)          pthread_mutex_lock(a)
#define UNLOCK(a)        pthread_mutex_unlock(a)
#define LOCK_DESTROY(a)  pthread_mutex_destroy(a)
#define SLEEPX(a)        usleep(a*1000)
#define LOCK_T	pthread_mutex_t
#define THREAD_T pthread_t

#define SEM_T           sem_t
#define SEM_INIT(a)     sem_init(&a, 1, 0);
#define SEM_POST(a)     sem_post(&a)
#define SEM_WAIT(a)     sem_wait(&a)
#define SEM_DESTROY(a)  sem_destroy(&a)


#endif   




#endif



