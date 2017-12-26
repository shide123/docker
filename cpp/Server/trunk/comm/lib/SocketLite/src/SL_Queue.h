#ifndef SOCKETLITE_QUEUE_H
#define SOCKETLITE_QUEUE_H

#include "SL_DataType.h"
#include "SL_Sync_Guard.h"

#include <list>
#include <deque>

template <typename T, typename TSyncMutex>
class SL_Queue
{
public:
    inline SL_Queue()
        : max_size_(10000)
        , current_size_(0)
    {
    }

    inline ~SL_Queue()
    {
        clear();
    }

    inline int init(size_t max_size = 10000)
    {
        if (max_size < 1)
        {
            max_size_ = 1;
        }
        else
        {
            max_size_ = max_size;
        }
        clear();
        return 0;
    }

    inline int clear()
    {
        mutex_.lock();
        queue_.clear();
        current_size_ = 0;
        mutex_.unlock();
        return 0;
    }

    inline int push_front_i(const T &node)
    {
        queue_.push_front(node);
        ++current_size_;
        return (int)current_size_;
    }

    inline int push_front_i(const T node[], uint node_count, uint &push_node_count)
    {
        for (int i=0; i<node_count; ++i)
        {
            queue_.push_front(node[i]);
        }
        push_node_count = node_count;
        current_size_  += node_count;
        return current_size_;
    }

    inline int push_back_i(const T &node)
    {
        queue_.push_back(node);
        ++current_size_;
        return (int)current_size_;
    }

    inline int push_back_i(const T node[], uint node_count, uint &push_node_count)
    {
        for (int i=0; i<node_count; ++i)
        {
            queue_.push_back(node[i]);
        }
        push_node_count = node_count;
        current_size_  += node_count;
        return current_size_;
    }

    inline int pop_front_i(T &node)
    {
        if (current_size_ > 0)
        {
            node = queue_.front();
            queue_.pop_front();
            --current_size_;
            return (int)current_size_;
        }
        return -1;
    }

    inline int pop_front_i(T node[], uint node_count, uint &pop_node_count)
    {
        pop_node_count = ((node_count<current_size_) ? node_count : current_size_);
        if (pop_node_count <= 0)
        {
            return -1;
        }
        for (uint i=0; i<pop_node_count; ++i)
        {
            node[i] = queue_.front();
            queue_.pop_front();
        }
        current_size_ -= pop_node_count;
        return (int)current_size_;
    }

    inline int pop_back_i(T &node)
    {
        if (current_size_ > 0)
        {
            node = queue_.back();
            queue_.pop_back();
            --current_size_;
            return current_size_;
        }
        return -1;
    }

    inline int pop_back_i(T node[], uint node_count, uint &pop_node_count)
    {
        pop_node_count = ((node_count<current_size_) ? node_count : current_size_);
        if (pop_node_count <= 0)
        {
            return -1;
        }
        for (int i=0; i<pop_node_count; ++i)
        {
            node[i] = queue_.back();
            queue_.pop_back();
        }
        current_size_ -= pop_node_count;
        return current_size_;
    }

    inline int push_front(const T &node)
    {
        mutex_.lock();
        int res = push_front_i(node);
        mutex_.unlock();
        return res;
    }

    inline int push_front(const T node[], uint node_count, uint &push_node_count)
    {
        mutex_.lock();
        int res = push_front_i(node, node_count, push_node_count);
        mutex_.unlock();
        return res;
    }

    inline int push_back(const T &node)
    {
        mutex_.lock();
        int res = push_back_i(node);
        mutex_.unlock();
        return res;
    }

    inline int push_back(const T node[], uint node_count, uint &push_node_count)
    {
        mutex_.lock();
        int res = push_back_i(node, node_count, push_node_count);
        mutex_.unlock();
        return res;
    }

    inline int pop_front(T &node)
    {
        if (current_size_ == 0)
        {
            return -1;
        }
        mutex_.lock();      
        int res = pop_front_i(node);
        mutex_.unlock();
        return res;
    }

    inline int pop_front(T node[], uint node_count, uint &pop_node_count)
    {
        if (current_size_ == 0)
        {
            return -1;
        }
        mutex_.lock();
        int res = pop_front_i(node, node_count, pop_node_count);
        mutex_.unlock();
        return res;
    }

    inline int pop_back(T &node)
    {
        if (current_size_ == 0)
        {
            return -1;
        }
        mutex_.lock();
        int res = pop_back_i(node);
        mutex_.unlock();
        return res;
    }

    inline int pop_back(T node[], uint node_count, uint &pop_node_count)
    {
        if (current_size_ == 0)
        {
            return -1;
        }
        mutex_.lock();
        int res = pop_back_i(node, node_count, pop_node_count);
        mutex_.unlock();
        return res;
    }

    inline size_t size() const
    {
        return current_size_;
    }

    inline bool empty() const
    {
        return (current_size_ == 0);
    }

    inline T& front()
    {
        return queue_.front();
    }

    inline T& back()
    {
        return queue_.back();
    }

	inline TSyncMutex& mutex()
	{
		return mutex_;
	}

private:
    size_t          max_size_;
    size_t          current_size_;  //加入的原因是因为std::list.size()只保证为时间效率为O(N)不是O(1)

    TSyncMutex      mutex_;
    std::list<T>    queue_;
    //std::deque<T>   queue_;
};

#endif
