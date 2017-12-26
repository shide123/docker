#ifndef SOCKETLITE_SOCKET_CONECTIONMGR_H
#define SOCKETLITE_SOCKET_CONECTIONMGR_H

#include "SL_Thread.h"
#include <list>

//���⣺��α���ģ����SL_Socket_SendControl_Handler<>��ָ��?
//�������1:
//      �����д��SL_Socket_Handler���ָ�룬
//      ͨ��dynamic_cast<SL_Socket_SendControl_Interface*>(handler), 
//      ��dynamic_cast���������ͳ����Ч�ʣ��ڸ���c++�ı������϶������,һ����˵,
//      ʹ�õ�dynamic_cast�Ĳ���Ч�ʽ���15%�����Ǻ������ġ����������ĳ����ܹ�����dynamic_cast���ǾͲ�Ҫ��
//std::list<SL_Socket_Handler*>   sendcontrol_list_;
//�������2:
//      �����д��<SL_Socket_Handler*, SL_Socket_SendControl_Interface*>�Ľṹ��
//      ���ڴ�ռ��Ҫ��Щ(sizeof(void*)

class SL_Socket_SendControl_Interface;
class SL_Socket_SendControl_HandlerManager
{
public:
    SL_Socket_SendControl_HandlerManager();
    virtual ~SL_Socket_SendControl_HandlerManager();

    int open(int thread_number=1, int send_block_size=4096, int iovec_count=10, int keepalive_time=0, int send_delaytime=100, int scan_sleeptime=10);
    int close();
    int add_handler(SL_Socket_Handler *handler, SL_Socket_SendControl_Interface *sendcontrol);
    int add_handler(SL_Socket_Handler *handler);
    int event_loop(int wait_time=50);

protected:
    virtual int assign_thread();

private:
    SL_IOVEC    *iovec_;
    uint        iovec_count_;       //ϵͳ����writev����iovec����ĸ���
    uint        thread_number_;     //�߳���(��<=0,��ʾ������ר�ŷ����߳�)
    uint        send_block_size_;   //�������ݿ�Ĵ�С(ÿ����һ�η���һ��)
    uint        keepalive_time_;    //keepalive���
    uint        send_delaytime_;    //������ʱʱ��(ms)
    uint        scan_sleeptime_;    //ɨ��һ���sleepʱ��(ms)

    typedef std::pair<SL_Socket_Handler*, SL_Socket_SendControl_Interface*> ITEM_SENDCONTROL;
    struct SendThread
    {
        void   *parent;            
        uint    index;                  //�߳����̼߳���������
        uint    list_size;

        SL_Sync_ThreadMutex             mutex;
        SL_Thread<SL_Sync_NullMutex>    thread;
        std::list<ITEM_SENDCONTROL>     handler_list;
    };
    std::vector<SendThread*> send_threads_;

#ifdef SOCKETLITE_OS_WINDOWS
    static unsigned int WINAPI send_proc(void *arg);
#else
    static void* send_proc(void *arg);
#endif

    template <typename TSocketHandler, typename TByteBuffer, typename TSyncMutex> friend class SL_Socket_SendControl_Handler;
};

#endif
