#pragma once

#include "SL_Socket_SendControl_Handler.h"
#include "SL_Socket_TcpServer_Handler.h"
#include "SL_ObjectPool.h"
#include "SL_Sync_Mutex.h"
#include "SL_Socket_Iocp_Message_Handler.h"
#include "SL_Socket_SendControl_HandlerManager.h"

SL_Socket_SendControl_HandlerManager g_sendcontrol_handlermgr;

class My_SendControl_Handler : public SL_Socket_SendControl_Handler<SL_Socket_Iocp_Message_Handler, SL_ByteBuffer, SL_Sync_Mutex>
{
public:
    int do_open()
    {
        g_sendcontrol_handlermgr.add_handler(this, this);
        return 0;
    }
    //int handle_read(const char *msg, int len)
    //{
    //    struct MSG
    //    {
    //        int len;
    //        char msgcontent[100];
    //    };
    //    MSG *msg_real = (MSG*)msg;
    //    printf("socket:%d, msg: %s\n", socket_, msg_real->msgcontent);
    //    return 0;
    //}
    int do_read(const char *msg, int len)
    {
        struct MSG
        {
            int len;
            char msgcontent[100];
        };
        MSG *msg_real = (MSG*)msg;
        //printf("socket:%d, msg: %s\n", socket_, msg_real->msgcontent);
        put_buffer(msg, len);
        return 0;
    }
    int do_close()
    {
        printf("socket:%d connect close\n", socket_);
        return 0;
    }

};

class My_SendControl_TcpServer_Handler:
    public SL_Socket_TcpServer_Handler_Impl<My_SendControl_Handler, SL_ObjectPool_SimpleEx<My_SendControl_Handler, SL_Sync_ThreadMutex> >
{
public:

    //virtual int handle_accept();

};


