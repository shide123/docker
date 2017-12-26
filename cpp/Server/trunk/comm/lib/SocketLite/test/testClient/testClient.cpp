// testClient.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <time.h>

#include "../../src/SL_Config.h"
#include "../../src/SL_Socket_Select_Runner.h"
#include "../../src/SL_Socket_Epoll_Runner.h"
#include "../../src/SL_Log.h"
#include "../../src/SL_Socket_TcpServer.h"
#include "../../src/SL_Socket_TcpClient.h"
#include "../../src/SL_Sync_Mutex.h"
#include "../../src/SL_Sync_Condition.h"

#include "Client_Handler.h"

int _tmain(int argc, _TCHAR* argv[])
{
    SL_Socket_CommonAPI::socket_init(2,2);

    SL_Log log;
    log.init(SL_Log::LOG_MODE_TO_PRINT, 5, "log", "client", "log");
    SL_Socket_Select_Runner<SL_Sync_ThreadMutex> runner;
    runner.open(SL_Socket_Handler::READ_EVENT_MASK|SL_Socket_Handler::WRITE_EVENT_MASK);

    Client_Handler client_handler[1000];
    SL_Socket_TcpClient tcpclient[1000];
    
    for (int i=0; i<1000; i++)
    {
        tcpclient[i].set_interface(&client_handler[i],&runner,&log);
        tcpclient[i].set_config(200, false);
        tcpclient[i].add_serveraddr("127.0.0.1",5000);
        //tcpclient[i].add_serveraddr("127.0.0.1",1200);    
        //tcpclient[i].add_serveraddr("192.168.1.125",1100);
        tcpclient[i].open();
    }
    printf("connect success count:%d\n", Client_Handler::connect_success_);
    while (1)
    {
        runner.event_loop(50);
    };
	
    //SL_Sync_ThreadMutex mutex1;
    //SL_Sync_Cond<SL_Sync_ThreadMutex> cond(mutex1);
    //cond.wait();
    //clock_t t = clock();

    return 0;
}

