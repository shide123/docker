#ifndef SOCKETLITE_SOCKET_CONECTION_H
#define SOCKETLITE_SOCKET_CONECTION_H

#include "SL_Config.h"
#include "SL_ByteBuffer.h"
#include "SL_Sync_Mutex.h"
#include "SL_Queue.h"
#include "SL_Socket_SendControl_HandlerManager.h"
#include <vector>

class SL_Socket_SendControl_Interface
{
public:
    enum CONNECTION_STATUS
    {
        STATUS_UNKNOW       = -1,           //未知
        STATUS_CLOSE        = 0,            //连接关闭
        STATUS_OPEN         = 1,            //连接正常
        STATUS_CLOSE_WAIT   = 2             //连接等待关闭
    };

    // These enumerations are used to describe when messages are delivered.
    enum MessagePriority
    {
        // Used by SocketLite to send above-high priority messages.
        SYSTEM_PRIORITY = 0,

        // High priority messages are send before medium priority messages.
        HIGH_PRIORITY,

        // Medium priority messages are send before low priority messages.
        MEDIUM_PRIORITY,   

        // Low priority messages are only sent when no other messages are waiting.
        LOW_PRIORITY,

        // 内部使用
        NUMBER_OF_PRIORITIES
    };

    enum WRITE_RETURN_VALUE
    {
        WRITE_RETURN_NOT_INITIAL    = -3,   //没初始化
        WRITE_RETURN_STATUS_INVALID = -2,   //表示Socket状态失效
        WRITE_RETURN_SEND_FAIL      = -1,   //表示发送失败(Socket已失效)
        WRITE_RETURN_NOT_DATA       = 0,    //表示没有数据可发送 
        WRITE_RETURN_SEND_SUCCESS   = 1,    //表示发送成功
        WRITE_RETURN_SEND_PART      = 2     //表示发送出部分数据(包含0长度)
    };

    SL_Socket_SendControl_Interface() 
    {
    }

    virtual ~SL_Socket_SendControl_Interface() 
    {
    }

    virtual int put_data(const char *buf, int len, MessagePriority priority=MEDIUM_PRIORITY) = 0;

private:
    virtual int init_control() = 0;
    virtual int clear_control() = 0;
    virtual WRITE_RETURN_VALUE write_data(SL_IOVEC *iov, int iovcnt) = 0;

protected:
    ulong   last_updatetime_;               //最后更新连接时间(收到/发送成功数据后更新)
    ulong   last_failtime_;                 //最后发送失败时间(发送失败后更新)
    int8    current_status_;                //当前状态(CONNECTION_STATUS类型)
  
    SL_Socket_SendControl_HandlerManager *handle_manager_;
    friend class SL_Socket_SendControl_HandlerManager;
};

template <typename TSocketHandler, typename TByteBuffer, typename TSyncMutex>
class SL_Socket_SendControl_Handler : public TSocketHandler, public SL_Socket_SendControl_Interface
{
public:
    SL_Socket_SendControl_Handler()
        : pop_bytebuffer_(NULL)
    {
    }

    virtual ~SL_Socket_SendControl_Handler()
    {
        if (pop_bytebuffer_ != NULL)
        {
            delete []pop_bytebuffer_;
            pop_bytebuffer_ = NULL;
        }
    }

    virtual int do_open()
    {
        return 0;
    }

    virtual int do_close()
    {
        return 0;
    }

    virtual int do_read(const char *buf, int len)
    {
        return 0;
    }

    int handle_open(SL_SOCKET fd, SL_Socket_Source *socket_source, SL_Socket_Runner *socket_runner)
    {
        TSocketHandler::handle_open(fd, socket_source, socket_runner);
        system_priority_queue_.clear();
        high_priority_queue_.clear();
        medium_priority_queue_.clear();
        low_priority_queue_.clear();
        last_updatetime_ = SL_Socket_CommonAPI::util_process_clock_ms();
        last_failtime_   = 0;
        current_status_  = STATUS_OPEN;
        return do_open();
    }

    int handle_close()
    {
        if (current_status_ != STATUS_CLOSE)
        {
            do_close();
            mutex_.lock();
            current_status_  = STATUS_CLOSE;
            last_updatetime_ = 0;
            last_failtime_   = 0;
            system_priority_queue_.clear();
            high_priority_queue_.clear();
            medium_priority_queue_.clear();
            low_priority_queue_.clear();
            mutex_.unlock();
        }
        return -1;
    }

    int handle_read(const char *buf, int len)
    {
        last_updatetime_ = SL_Socket_CommonAPI::util_process_clock_ms();
        return do_read(buf, len);
    }

    int put_data(TByteBuffer &buf, MessagePriority priority=MEDIUM_PRIORITY)
    {
        mutex_.lock();
        switch (priority)
        {
        case SYSTEM_PRIORITY:
            system_priority_queue_.push_back_i(buf);
            break;
        case HIGH_PRIORITY:
            high_priority_queue_.push_back_i(buf);
            break;
        case MEDIUM_PRIORITY:
            medium_priority_queue_.push_back_i(buf);
            break;
        case LOW_PRIORITY:
            low_priority_queue_.push_back_i(buf);
            break;
        default:
            mutex_.unlock();
            return -1;
        }
        mutex_.unlock();
        return 0;
    }

    int put_data(const char *buf, int len, MessagePriority priority=MEDIUM_PRIORITY)
    {
        TByteBuffer tbuf(len);
        tbuf.write(buf, len);
        return put_data(tbuf, priority);
    }

    size_t get_sendqueue_size() const
    {
        return (system_priority_queue_.size()+high_priority_queue_.size()+medium_priority_queue_.size()+low_priority_queue_.size());
    }

private:
    int init_control()
    {
        if (NULL == pop_bytebuffer_)
        {
            pop_bytebuffer_ = new TByteBuffer[handle_manager_->iovec_count_];
        }
        if (NULL == pop_bytebuffer_)
        {
            return -1;
        }
        return 0;
    }

    int clear_control()
    {
        for (int i=0; i<handle_manager_->iovec_count_; ++i)
        {
            pop_bytebuffer_[i].clear();
        }
        return 0;
    }

    //发送数据
    //返回值的类型: WRITE_RETURN_VALUE
    WRITE_RETURN_VALUE write_data(SL_IOVEC *iov, int iovcnt)
    {
        if (STATUS_CLOSE == current_status_)
        {
            return WRITE_RETURN_STATUS_INVALID;
        }

        if ( (last_failtime_ == 0) || 
             (SL_Socket_CommonAPI::util_process_clock_ms()-last_failtime_ > handle_manager_->send_delaytime_)
            )
        {
            int i = 0;
            int j = -1;
            uint data_size = 0;
            uint need_send_bytes = handle_manager_->send_block_size_;
            for (i=0; i<iovcnt; ++i)
            {
                data_size = pop_bytebuffer_[i].data_size();
                if (data_size > 0)
                {
                    if (need_send_bytes >= data_size)
                    {
                        ++j;
                        iov[j].iov_len   = data_size;
                        iov[j].iov_base  = pop_bytebuffer_[i].data();
                        need_send_bytes -= data_size;
                        if (need_send_bytes < 1)
                        {
                            break;
                        }
                    }
                    else
                    {
                        ++j;
                        iov[j].iov_len  = need_send_bytes;
                        iov[j].iov_base = pop_bytebuffer_[i].data();
                        need_send_bytes = 0;
                        break;
                    }
                }
            }

            if (j < 0)
            {//取出需要发送的数据
                if (get_sendqueue_size() == 0)
                {
                    return WRITE_RETURN_NOT_DATA;
                }
                uint node_count     = iovcnt;
                uint pop_node_count = 0;
                mutex_.lock();
                system_priority_queue_.pop_front_i(pop_bytebuffer_, node_count, pop_node_count);
                node_count -= pop_node_count;
                if (node_count > 0)
                {
                    high_priority_queue_.pop_front_i(pop_bytebuffer_, node_count, pop_node_count);
                    node_count -= pop_node_count;
                    if (node_count > 0)
                    {
                        medium_priority_queue_.pop_front_i(pop_bytebuffer_, node_count, pop_node_count);
                        node_count -= pop_node_count;
                        if (node_count > 0)
                        {
                            low_priority_queue_.pop_front_i(pop_bytebuffer_, node_count, pop_node_count);
                            node_count -= pop_node_count;
                        }
                    }
                }
                mutex_.unlock();

                if (node_count == iovcnt)
                {
                    return WRITE_RETURN_NOT_DATA;
                }
                for (i=0; i<iovcnt; ++i)
                {
                    data_size = pop_bytebuffer_[i].data_size();
                    if (data_size > 0)
                    {
                        if (need_send_bytes >= data_size)
                        {
                            ++j;
                            iov[j].iov_len  = data_size;
                            iov[j].iov_base = pop_bytebuffer_[i].data();
                            need_send_bytes -= data_size;
                            if (need_send_bytes < 1)
                            {
                                break;
                            }
                        }
                        else
                        {
                            ++j;
                            iov[j].iov_len  = need_send_bytes;
                            iov[j].iov_base = pop_bytebuffer_[i].data();
                            need_send_bytes = 0;
                            break;
                        }
                    }
                }
                if (j < 0)
                {
                    return WRITE_RETURN_NOT_DATA;
                }
            }

            //发送数据
            if (current_status_ == STATUS_CLOSE)
            {
                return WRITE_RETURN_STATUS_INVALID;
            }
            else
            {
                int errorid = 0;
                int res_send_bytes = SL_Socket_CommonAPI::socket_sendv(TSocketHandler::socket_, iov, j+1, 0, NULL, &errorid);
                if (res_send_bytes <= 0)
                {
                    if ((errorid == 0) || (errorid == SL_EWOULDBLOCK) || (errorid == SL_ENOBUFS))
                    {//非阻塞模式下正常情况
                        last_failtime_ = SL_Socket_CommonAPI::util_process_clock_ms();
                        return WRITE_RETURN_SEND_PART;
                    }
                    else
                    {//当前连接已失效
                        current_status_ = STATUS_CLOSE_WAIT;
                    }
                    return WRITE_RETURN_SEND_FAIL;
                }
                else
                {
                    WRITE_RETURN_VALUE res = WRITE_RETURN_SEND_SUCCESS;
                    if ((handle_manager_->send_block_size_-need_send_bytes) == res_send_bytes)
                    {
                        last_failtime_ = 0;
                    }
                    else
                    {
                        res = WRITE_RETURN_SEND_PART;
                        last_failtime_ = SL_Socket_CommonAPI::util_process_clock_ms();
                    }
                    last_updatetime_ = SL_Socket_CommonAPI::util_process_clock_ms();
                    for (i=0; i<iovcnt; ++i)
                    {
                        data_size = pop_bytebuffer_[i].data_size();
                        if (data_size > 0)
                        {
                            if (data_size > res_send_bytes)
                            {
                                pop_bytebuffer_[i].data_begin(pop_bytebuffer_[i].data_begin()+res_send_bytes);
                                break;
                            }
                            else
                            {
                                pop_bytebuffer_[i].clear();
                                res_send_bytes -= data_size;
                                if (res_send_bytes < 1)
                                {
                                    break;
                                }
                            }
                        }
                    }
                    return res;
                }
            }
        }
        return WRITE_RETURN_NOT_DATA;
    }

protected:
    TSyncMutex  mutex_;
    SL_Queue<TByteBuffer, SL_Sync_NullMutex> system_priority_queue_;
    SL_Queue<TByteBuffer, SL_Sync_NullMutex> high_priority_queue_;
    SL_Queue<TByteBuffer, SL_Sync_NullMutex> medium_priority_queue_;
    SL_Queue<TByteBuffer, SL_Sync_NullMutex> low_priority_queue_;
    TByteBuffer *pop_bytebuffer_;
};

#endif
