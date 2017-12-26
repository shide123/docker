#ifndef SOCKETLITE_SOCKET_CONECTIONMGR_H
#define SOCKETLITE_SOCKET_CONECTIONMGR_H

#include "SL_Thread.h"
#include <list>

//问题：如何保存模板类SL_Socket_SendControl_Handler<>的指针?
//解决方法1:
//      集合中存放SL_Socket_Handler类的指针，
//      通过dynamic_cast<SL_Socket_SendControl_Interface*>(handler), 
//      但dynamic_cast会显著降低程序的效率，在各种c++的编译器上都是如此,一般来说,
//      使用到dynamic_cast的部分效率降低15%以上是很正常的。所以如果你的程序能够不用dynamic_cast，那就不要用
//std::list<SL_Socket_Handler*>   sendcontrol_list_;
//解决方法2:
//      集合中存放<SL_Socket_Handler*, SL_Socket_SendControl_Interface*>的结构体
//      但内存占用要大些(sizeof(void*)

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
    uint        iovec_count_;       //系统调用writev函数iovec数组的个数
    uint        thread_number_;     //线程数(若<=0,表示不启动专门发送线程)
    uint        send_block_size_;   //发送数据块的大小(每遍历一次发送一块)
    uint        keepalive_time_;    //keepalive间隔
    uint        send_delaytime_;    //发送延时时间(ms)
    uint        scan_sleeptime_;    //扫描一遍后，sleep时间(ms)

    typedef std::pair<SL_Socket_Handler*, SL_Socket_SendControl_Interface*> ITEM_SENDCONTROL;
    struct SendThread
    {
        void   *parent;            
        uint    index;                  //线程在线程集合中索引
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
