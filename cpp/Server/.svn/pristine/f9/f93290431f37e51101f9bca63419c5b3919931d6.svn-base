#ifndef SOCKETLITE_SHAREDPTR_H
#define SOCKETLITE_SHAREDPTR_H
#include <algorithm>
#include "SL_Sync_SpinMutex.h"

template<typename TLOCK>
class SL_Refcount
{
public:
    SL_Refcount()
        : refcount_(0)
    {
    }

    SL_Refcount(long refcount)
        : refcount_(refcount)
    {
    }

    ~SL_Refcount()
    {
    }

    //++操作符
    SL_Refcount& operator ++() 
    {
        lock_.lock();
        ++refcount_;
        lock_.unlock();
        return *this;
    }

    //--操作符
    long operator --()
    {
        long ret;
        lock_.lock();
        --refcount_;
        ret = refcount_;
        lock_.unlock();
        return ret;
    }

    long refcount() const
    {
        return refcount_;
    }

private:
    long    refcount_;
    TLOCK   lock_;
};

template<typename T, typename TRefcount = SL_Refcount<SL_Sync_SpinMutex> >
class SL_SharedPtr
{
public:
    //默认构造
    SL_SharedPtr()
        : ptr_(NULL)
        , refcount_(NULL)
    {
    }

    //构造
    explicit SL_SharedPtr(T *p)
        : ptr_(p)
        , refcount_(new TRefcount(1))
    {
    }

    //拷贝构造
    SL_SharedPtr(const SL_SharedPtr<T, TRefcount> &b)
        : ptr_(b.ptr_)
        , refcount_(b.refcount_)
    { 
        if (NULL != refcount_)
        {
            ++*refcount_;
        }
    }

    //析构
    ~SL_SharedPtr()
    {
        release();
    }

    //=操作符
    SL_SharedPtr<T, TRefcount>& operator= (const SL_SharedPtr<T, TRefcount> &b) 
    {
        if (this != &b) 
        {
            release();
            ptr_ = b.ptr_;
            refcount_ = b.refcount_;
            ++*refcount_;
        }
        return *this;
    }

    T* get() const
    {
        return ptr_;
    }

    T* operator->() const
    {
        return ptr_;
    }

    T& operator*() const
    {
        return *ptr_;
    }

    long use_count() const
    {//return use count
        if (NULL != refcount_)
        {
            return refcount_->refcount();
        }
        return 0;
    }

    bool unique() const
    {//return true if no other shared_ptr object owns this resource
        return (use_count() == 1);
    }

    void swap(SL_SharedPtr<T, TRefcount> &b)
    {//never throws
        std::swap(ptr_, b.ptr_);
        std::swap(refcount_, b.refcount_);
    }

    void reset()
    {
        SL_SharedPtr().swap(*this);
    }
    
    void reset(T *p)
    {
        SL_SharedPtr(p).swap(*this);
    }

private:
    void release() 
    {
        if (NULL == refcount_)
        {
            return; 
        }
        if ((--*refcount_) <= 0) 
        {
            delete refcount_;
            refcount_ = NULL;
            if (NULL != ptr_ )
            {
                delete ptr_;
                ptr_ = NULL;
            }
        }
    }

private:
    T           *ptr_;
    TRefcount   *refcount_;
};

#endif
