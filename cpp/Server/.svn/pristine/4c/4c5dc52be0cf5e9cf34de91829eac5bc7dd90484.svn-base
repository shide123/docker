#ifndef _MY_SOCKET_TCPSERVER_HANDLER
#define _MY_SOCKET_TCPSERVER_HANDLER

#include "SL_Socket_TcpServer_Handler.h"
#include "SL_ObjectPool.h"
#include "SL_Sync_Mutex.h"
#include "SL_Socket_Iocp_IOChannel.h"

class My_Socket_Handler : public SL_Socket_Iocp_IOChannel
{
public:
    int handle_read(const char *msg, int len)
    {
        struct MSG
        {
            int len;
            char msgcontent[100];
        };
        MSG *msg_real = (MSG*)msg;
        printf("socket:%d, msg: %s\n", socket_, msg_real->msgcontent);
        return 0;
    }

    int do_close()
    {
        printf("socket:%d connect close\n", socket_);
        return -1;
    }
};
class My_Socket_TcpServer_Handler:
    public SL_Socket_TcpServer_Handler_Impl<My_Socket_Handler,SL_ObjectPool_SimpleEx<My_Socket_Handler,SL_Sync_ThreadMutex> >
{
public:
    My_Socket_TcpServer_Handler(SL_ObjectPool_SimpleEx<My_Socket_Handler,SL_Sync_ThreadMutex> *obj_pool)
        : SL_Socket_TcpServer_Handler_Impl<My_Socket_Handler,SL_ObjectPool_SimpleEx<My_Socket_Handler,SL_Sync_ThreadMutex> >(obj_pool)
    {
    };
    My_Socket_TcpServer_Handler()
    {
    };

    //virtual int handle_accept();

};

#endif