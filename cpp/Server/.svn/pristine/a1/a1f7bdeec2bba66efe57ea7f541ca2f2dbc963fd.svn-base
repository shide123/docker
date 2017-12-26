//#include "StdAfx.h"
#include "My_App.h"
#include "IMDefine.h"
#include "IMLog.h"
#include "SL_Socket_CommonAPI.h"

SL_Log My_App::m_log;
SL_Socket_TcpServer My_App::m_tcpserver;

#ifdef SOCKETLITE_OS_WINDOWS
    SL_Socket_Select_Runner<SL_Sync_ThreadMutex> My_App::m_runner;
#else
    SL_Socket_Epoll_Runner<SL_Sync_ThreadMutex>  My_App::m_runner;
#endif
My_Socket_TcpServer_Handler My_App::m_server_handler;
SL_ObjectPool_SimpleEx<My_Socket_Handler,SL_Sync_ThreadMutex> My_App::m_obj_pool;
My_Task My_App::m_task;

My_App::My_App(void)
{
}

My_App::~My_App(void)
{
}

int My_App::init()
{
    m_task.open(5, 10000);
    m_log.init(SL_Log::LOG_MODE_TO_PRINT, 5, "log", "test_server", "log");
    m_obj_pool.init();

    //m_runner.open(SL_Socket_Handler::ALL_EVENT_MASK,FD_SETSIZE, 100, 1);
    //m_server_handler.set_object_pool(&m_obj_pool);
    //m_tcpserver.set_interface(&m_server_handler, &m_runner, &m_log);
    //m_tcpserver.set_config(true, true);
    //m_tcpserver.open(1100, 200);

    uint32 transid = 1;
    while (1)
    {
        //m_runner.event_loop(50);
        SL_ByteBuffer buf(IM_MSGHEAD_FULL_LEN);
        buf.data_end(IM_MSGHEAD_FULL_LEN);
        IM_MSG_FULL *im_msg = (IM_MSG_FULL*)buf.data();
        im_msg->length  = IM_MSGHEAD_FULL_LEN;
        im_msg->command = IM_CMD_KEEP_ALIVE;
        im_msg->fromid  = transid;
        im_msg->transid = transid;
        im_msg->param0  = 0;
        im_msg->param1  = 0;
        im_msg->param2  = 0;
        im_msg->param3  = 0;
        im_msg->protocolver = 1;
        m_task.putq(buf);

        transid++;
        SL_Socket_CommonAPI::util_sleep(1);
    };
    return 0;
}
