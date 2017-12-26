// testIocpServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "SL_Socket_Select_Runner.h"
#include "SL_Socket_Epoll_Runner.h"
#include "SL_Socket_Iocp_Runner.h"
#include "SL_Socket_Iocp_IOChannel.h"
#include "SL_Socket_Runner_Group.h"
#include "SL_Socket_TcpServer.h"
#include "SL_Log.h"

#include "my_tcpserver_handler.h"
#include "my_sendcontrol_handler.h"

int _tmain(int argc, _TCHAR* argv[])
{
    //ulong start_time = SL_Socket_CommonAPI::util_process_clock_ms();
    //long double x = 1e8;
    //while (x>0)
    //{
    //    --x;
    //}
    //printf("tack time:%ld ms", SL_Socket_CommonAPI::util_process_clock_ms()-start_time);

    SL_Socket_CommonAPI::socket_init(2, 2);
    SL_Socket_Iocp_Runner<SL_Sync_ThreadMutex> iocp_runner1;

    SL_Log log;
    log.init(SL_Log::LOG_MODE_TO_PRINT,5,"log","test_iocp_server","log");

    SL_ObjectPool_SimpleEx<My_SendControl_Handler, SL_Sync_ThreadMutex> my_obj_pool;
    my_obj_pool.init(1000, 1000, 10);
    My_SendControl_TcpServer_Handler my_server_handle;
    my_server_handle.set_object_pool(&my_obj_pool);

    SL_Socket_TcpServer my_tcpserver;
    my_tcpserver.set_interface(&my_server_handle, &iocp_runner1, &log);
    my_tcpserver.set_config(10, true, 1024, 4096, 1024, 4, 0);

    g_sendcontrol_handlermgr.open(1, 10, 4096, 0, 100, 10);
    iocp_runner1.open(SL_Socket_Handler::ALL_EVENT_MASK, 10000, 2000, 6);
    my_tcpserver.open(5000, 50);

    iocp_runner1.thread_wait();
    while (1)
    {
        SL_Socket_CommonAPI::util_sleep(100);
    }
    iocp_runner1.close();

	return 0;
}

