#ifndef SL_TSS_H
#define SL_TSS_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	#pragma once
#endif
#include "SL_Config.h"

#if defined(SOCKETLITE_OS_WINDOWS)
#define SL_TLS_OUT_OF_INDEXES TLS_OUT_OF_INDEXES 

template <typename T>
class SL_TSS
{
public:
    SL_TSS()
        : tss_key_(SL_TLS_OUT_OF_INDEXES)
    {
        init();
    }

    ~SL_TSS()
    {
        clear();
    }

    int init()
    {
        if (tss_key_ == SL_TLS_OUT_OF_INDEXES)
        {
            tss_key_ = ::TlsAlloc();
            if (tss_key_ == SL_TLS_OUT_OF_INDEXES)
            {
                return -1;
            }
        }
        return 0;
    }

    int clear()
    {
        if (tss_key_ != SL_TLS_OUT_OF_INDEXES)
        {
            ::TlsFree(tss_key_);
            tss_key_ = SL_TLS_OUT_OF_INDEXES;
        }
        return 0;
    }

    // Get the value.
    T* get() const
    {
        return static_cast<T*>(::TlsGetValue(tss_key_));
    }

    T* operator->() const
    {
        return get();
    }

    operator T*() const
    {
        return get();
    }

    // Set the value.
    void set(T* value)
    {
        ::TlsSetValue(tss_key_, value);
    }

    void operator= (T* value)
    {
        set(value);
    }

private:
    DWORD   tss_key_;
};

#elif defined(SOCKETLITE_HAVE_PTHREADS)
#include <pthread.h>
#define SL_TLS_OUT_OF_INDEXES -1 

template <typename T>
class SL_TSS
{    
public:
    SL_TSS()
        : tss_key_(SL_TLS_OUT_OF_INDEXES)
    {
        init();
    }

    ~SL_TSS()
    {
        clear();
    }

    int init()
    {
        if (tss_key_ == SL_TLS_OUT_OF_INDEXES)
        {
            int rc = ::pthread_key_create(&tss_key_, 0);
            if (rc != 0)
            {
                tss_key_ = SL_TLS_OUT_OF_INDEXES;
                return -1;
            }
        }
        return 0;
    }

    int clear()
    {
        if (tss_key_ != SL_TLS_OUT_OF_INDEXES)
        {
            ::pthread_key_delete(tss_key_);
            tss_key_ = SL_TLS_OUT_OF_INDEXES;
        }
        return 0;
    }

    // Get the value.
    T* get() const
    {
        return static_cast<T*>(::pthread_getspecific(tss_key_));
    }

    T* operator->() const
    {
        return get();
    }

    operator T*() const
    {
        return get();
    }

    // Set the value.
    void set(T* value)
    {
        ::pthread_setspecific(tss_key_, value);
    }

    void operator= (T* value)
    {
        set(value);
    }

private:
    pthread_key_t   tss_key_;
};

#endif  //SOCKETLITE_OS_WINDOWS

#endif  //SL_TSS_H
