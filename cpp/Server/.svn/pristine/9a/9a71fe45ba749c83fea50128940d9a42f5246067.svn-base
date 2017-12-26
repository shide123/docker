#ifndef MY_APP_H
#define MY_APP_H

#include "my_tcpserver_handler.h"
#include "My_Task.h"
#include "SL_Socket_Select_Runner.h"
#include "SL_Socket_Epoll_Runner.h"
#include "SL_Socket_Runner_Group.h"
#include "SL_Socket_TcpServer.h"
#include "SL_Log.h"

class My_App
{
private:
    My_App(void);
    ~My_App(void);
   
public:
    static int init();
public:
    static SL_Log               m_log;
    static SL_Socket_TcpServer  m_tcpserver;
#ifdef SOCKETLITE_OS_WINDOWS
    static SL_Socket_Select_Runner<SL_Sync_ThreadMutex> m_runner;
#else
    static SL_Socket_Epoll_Runner<SL_Sync_ThreadMutex>  m_runner;
#endif
    static My_Socket_TcpServer_Handler                  m_server_handler;
    static SL_ObjectPool_SimpleEx<My_Socket_Handler,SL_Sync_ThreadMutex>  m_obj_pool;

    static My_Task  m_task;
};

#endif