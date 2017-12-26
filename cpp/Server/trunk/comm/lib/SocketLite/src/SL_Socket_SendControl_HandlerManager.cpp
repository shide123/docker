#include "SL_Socket_TcpServer_Handler.h"
#include "SL_Socket_SendControl_Handler.h"
#include "SL_Socket_SendControl_HandlerManager.h"

SL_Socket_SendControl_HandlerManager::SL_Socket_SendControl_HandlerManager()
    : iovec_(NULL)
    , iovec_count_(10)
    , send_block_size_(4096)
{
}

SL_Socket_SendControl_HandlerManager::~SL_Socket_SendControl_HandlerManager()
{
    close();
}

int SL_Socket_SendControl_HandlerManager::open(int thread_number, 
                                               int send_block_size,
                                               int iovec_count, 
                                               int keepalive_time, 
                                               int send_delaytime, 
                                               int scan_sleeptime)
{
    close();

    thread_number_   = thread_number;
    send_block_size_ = send_block_size;
    iovec_count_     = iovec_count;
    keepalive_time_  = keepalive_time;
    send_delaytime_  = send_delaytime;
    scan_sleeptime_  = scan_sleeptime;
    if (thread_number > 0)
    {
        send_threads_.reserve(thread_number);
        iovec_ = NULL;
        SendThread *send_thread;
        for (int i=0; i<thread_number; ++i)
        {
            send_thread             = new SendThread;
            send_thread->parent     = this;
            send_thread->index      = i;
            send_thread->list_size  = 0;
            send_thread->thread.start(send_proc, send_thread);
            send_threads_.push_back(send_thread);
        }
    }
    else
    {
        iovec_ = new SL_IOVEC[iovec_count];
        SendThread *send_thread;
        send_thread             = new SendThread;
        send_thread->parent     = this;
        send_thread->index      = 0;
        send_thread->list_size  = 0;
        send_threads_.push_back(send_thread);
    }
    return 0;
}

int SL_Socket_SendControl_HandlerManager::close()
{
    if (NULL != iovec_)
    {
        delete []iovec_;
        iovec_ = NULL;
    }
    SendThread *send_thread;
    std::vector<SendThread*>::iterator iter = send_threads_.begin();
    if (thread_number_ > 0)
    {
        for (; iter!=send_threads_.end(); ++iter)
        {
            send_thread = *iter;
            send_thread->thread.stop();
            send_thread->thread.join();
            send_thread->handler_list.clear();
            delete send_thread;
        }
    }
    else
    {
        for (; iter!=send_threads_.end(); ++iter)
        {
            send_thread = *iter;
            send_thread->handler_list.clear();
            delete send_thread;
        }
    }
    send_threads_.clear();
    return 0;
}

int SL_Socket_SendControl_HandlerManager::assign_thread()
{
    int thread_size = send_threads_.size();
    if (thread_size == 1)
    {
        return 0;
    }

    //分配给负荷最小的线程,从而整体上使每个线程负荷平均
    int res_index = 0;
    int min_list_size = send_threads_[0]->list_size;
    int list_size;
    for (int i=1; i<thread_size; ++i)
    {
        list_size = send_threads_[i]->list_size;
        if (list_size < min_list_size)
        {
            min_list_size = list_size;
            res_index = i;
        }
    }
    return res_index;
}

int SL_Socket_SendControl_HandlerManager::add_handler(SL_Socket_Handler *handler, SL_Socket_SendControl_Interface *sendcontrol)
{
    sendcontrol->handle_manager_ = this;
    sendcontrol->init_control();
    ITEM_SENDCONTROL item_sendcontrol(handler, sendcontrol);
    int thread_index = assign_thread();
    SendThread *send_thread = send_threads_[thread_index];
    send_thread->mutex.lock();
    ++send_thread->list_size;
    send_thread->handler_list.push_back(item_sendcontrol);
    send_thread->mutex.unlock();
    return 0;
}

int SL_Socket_SendControl_HandlerManager::add_handler(SL_Socket_Handler *handler)
{
    SL_Socket_SendControl_Interface *sendcontrol = dynamic_cast<SL_Socket_SendControl_Interface*>(handler);
    return add_handler(handler, sendcontrol);
}

int SL_Socket_SendControl_HandlerManager::event_loop(int wait_time)
{
    SL_Socket_Handler *source_handler;
    SL_Socket_Handler *handler;
    SL_Socket_SendControl_Interface *sendcontrol;
    int write_success_times = 0;

    SendThread *send_thread = send_threads_[0];
    std::list<ITEM_SENDCONTROL>::iterator iter_handler = send_thread->handler_list.begin();
    while (iter_handler != send_thread->handler_list.end())
    {
        handler     = (*iter_handler).first;
        sendcontrol = (*iter_handler).second;

        //发送数据
        if (sendcontrol->write_data(iovec_, iovec_count_) == SL_Socket_SendControl_Interface::WRITE_RETURN_SEND_SUCCESS)
        {
            ++write_success_times;
        }

        if (sendcontrol->current_status_ == SL_Socket_SendControl_Interface::STATUS_CLOSE)
        {//连接已关闭
            sendcontrol->clear_control();
            send_thread->mutex.lock();
            --send_thread->list_size;
            iter_handler = send_thread->handler_list.erase(iter_handler);
            send_thread->mutex.unlock();

            //回收连接对象
            source_handler = handler->get_socket_source()->get_socket_handler();
            if (source_handler != handler)
            {//source_handler(tcpserver和tcpclient本身的handler)不可回收
                ((SL_Socket_TcpServer_Handler*)source_handler)->delete_handler(handler);
            }
        }
        else if (sendcontrol->current_status_ == SL_Socket_SendControl_Interface::STATUS_CLOSE_WAIT)
        {//强制关闭连接
            ++iter_handler;
            sendcontrol->current_status_ = SL_Socket_SendControl_Interface::STATUS_UNKNOW;
            handler->get_socket_runner()->del_handle(handler);
        }
        else
        {
            ++iter_handler;
            if ( 
                (keepalive_time_ > 0) && 
                ((SL_Socket_CommonAPI::util_process_clock_ms()-sendcontrol->last_updatetime_) > keepalive_time_)
               )
            {//防止死连接
                handler->get_socket_runner()->del_handle(handler);
            }
        }
    }
    return write_success_times;
}

#ifdef SOCKETLITE_OS_WINDOWS
unsigned int WINAPI SL_Socket_SendControl_HandlerManager::send_proc(void *arg)
#else
void* SL_Socket_SendControl_HandlerManager::send_proc(void *arg)
#endif
{
    SL_Socket_SendControl_HandlerManager::SendThread *send_thread = (SL_Socket_SendControl_HandlerManager::SendThread*)arg;
    SL_Socket_SendControl_HandlerManager *handler_manger = (SL_Socket_SendControl_HandlerManager*)send_thread->parent;

    SL_Socket_Handler *source_handler;
    SL_Socket_Handler *handler;
    SL_Socket_SendControl_Interface *sendcontrol;
    std::list<ITEM_SENDCONTROL>::iterator iter_handler; 

    SL_IOVEC *iovec = new SL_IOVEC[handler_manger->iovec_count_];
    int write_success_times;

#ifdef SOCKETLITE_DEBUG
    ulong starttime;
#endif

    while (1)
    {
        if (!send_thread->thread.get_running())
        {
            delete []iovec;
            return 0;
        }
        write_success_times = 0;

#ifdef SOCKETLITE_DEBUG
        starttime = SL_Socket_CommonAPI::util_process_clock_ms();
#endif
        iter_handler = send_thread->handler_list.begin();
        while (iter_handler != send_thread->handler_list.end())
        {
            handler     = (*iter_handler).first;
            sendcontrol = (*iter_handler).second;

            //发送数据
            if (sendcontrol->write_data(iovec, handler_manger->iovec_count_) == SL_Socket_SendControl_Interface::WRITE_RETURN_SEND_SUCCESS)
            {
                ++write_success_times;
            }

            if (sendcontrol->current_status_ == SL_Socket_SendControl_Interface::STATUS_CLOSE)
            {//连接已关闭
                sendcontrol->clear_control();
                send_thread->mutex.lock();
                --send_thread->list_size;
                iter_handler = send_thread->handler_list.erase(iter_handler);
                send_thread->mutex.unlock();

                //回收连接对象
                source_handler = handler->get_socket_source()->get_socket_handler();
                if (source_handler != handler)
                {//source_handler(tcpserver和tcpclient本身的handler)不可回收
                    ((SL_Socket_TcpServer_Handler*)source_handler)->delete_handler(handler);
                }
            }
            else if (sendcontrol->current_status_ == SL_Socket_SendControl_Interface::STATUS_CLOSE_WAIT)
            {//强制关闭
                ++iter_handler;
                sendcontrol->current_status_ = SL_Socket_SendControl_Interface::STATUS_UNKNOW;
                handler->get_socket_runner()->del_handle(handler);
            }
            else
            {
                ++iter_handler;
                if ( 
                    (handler_manger->keepalive_time_> 0) && 
                    ((SL_Socket_CommonAPI::util_process_clock_ms()-sendcontrol->last_updatetime_) > handler_manger->keepalive_time_)
                    )
                {//防止死连接
                    handler->get_socket_runner()->del_handle(handler);
                }
            }
        }

        if (write_success_times == 0)
        {
            SL_Socket_CommonAPI::util_sleep(handler_manger->scan_sleeptime_);
        }
#ifdef SOCKETLITE_DEBUG
        else
        {
            printf("scan handler_list size: %ld, write_success_times: %ld, pass_time: %ld ms\n", 
                send_thread->handler_list.size(), 
                write_success_times, 
                SL_Socket_CommonAPI::util_process_clock_ms()-starttime);
        }
#endif
    }

    delete []iovec;
    return 0;
}
