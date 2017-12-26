#ifndef SOCKETLITE_SYNC_SPINMUTEX_H
#define SOCKETLITE_SYNC_SPINMUTEX_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
    #pragma once
#endif
#include "SL_Sync_Macros.h"

#ifdef SOCKETLITE_OS_WINDOWS
class SL_Sync_SpinMutex
{
public:
    inline SL_Sync_SpinMutex()
        : spinmutex_(0)
    {         
    }

    inline ~SL_Sync_SpinMutex()
    {
    }

    inline void lock()
    {
        while (::InterlockedCompareExchange(&spinmutex_, 1, 0) != 0) 
        {
            ::Sleep(0);
        }
    }

    inline void unlock()
    { 
        ::InterlockedExchange((LONG*)&spinmutex_, 0);
    }

    inline bool trylock()
    {
        return (::InterlockedCompareExchange(&spinmutex_, 1, 0) == 0);
    }

    inline long handle()
    {
        return spinmutex_;
    }

private:
    SL_Sync_SpinMutex(const SL_Sync_SpinMutex &);
    void operator=(const SL_Sync_SpinMutex &);

    long spinmutex_;
};
#else
class SL_Sync_SpinMutex
{
public:
    inline SL_Sync_SpinMutex(int pshared=0)
    { 
        pthread_spin_init(&spinmutex_, pshared);
    }

    inline ~SL_Sync_SpinMutex()
    { 
        pthread_spin_destroy(&spinmutex_);
    }

    inline void lock()
    { 
        pthread_spin_lock(&spinmutex_);
    }

    inline void unlock()
    { 
        pthread_spin_unlock(&spinmutex_);
    }

    inline bool trylock()
    {
        return (pthread_spin_trylock(&spinmutex_) == 0);
    }

    inline pthread_spinlock_t* handle()
    {
        return &spinmutex_;
    }

private:
    SL_Sync_SpinMutex(const SL_Sync_SpinMutex &);
    void operator=(const SL_Sync_SpinMutex &);

    pthread_spinlock_t spinmutex_;
};
#endif

#endif
