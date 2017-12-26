//#include "my_tcpserver_handler.h"
//#include "../../src/SL_Socket_Select_Runner.h"
//#include "../../src/SL_Socket_Epoll_Runner.h"
//#include "../../src/SL_Socket_Runner_Group.h"
//#include "../../src/SL_Socket_TcpServer.h"
//#include "../../src/SL_Log.h"

#include "My_App.h"
#include "../../src/SL_Socket_UdpSource.h"
#include "My_UdpHandler.h"

int main(int argc, char ** argv)
{
    //SL_Socket_CommonAPI::socket_init(2,2);
    SL_Log log;
    log.init(SL_Log::LOG_MODE_TO_PRINT,5,"log","test_server","log");

    //SL_Socket_Select_Runner<SL_Sync_ThreadMutex> my_runner;
    ////SL_Socket_Runner_Group<SL_Socket_Select_Runner<SL_Sync_ThreadMutex> > my_runner;
    ////my_runner.init(3);

    //SL_ObjectPool_Simple<My_Socket_Handler,SL_Sync_ThreadMutex> my_obj_pool;
    //my_obj_pool.init();

    //My_Socket_TcpServer_Handler my_server_handle(&my_obj_pool);

    //SL_Socket_TcpServer my_tcpserver;
    //my_runner.open(SL_Socket_Handler::ALL_EVENT_MASK,FD_SETSIZE, 100, 1);
    //my_tcpserver.set_interface(&my_server_handle,&my_runner, &log);
    //my_tcpserver.set_config(true, true);
    //my_tcpserver.open(1100, 200);
    //while (1)
    //{
    //    my_runner.event_loop(50);
    //};

    //my_runner.run_event_loop(50);
    //my_runner.thread_wait();
    //my_runner.close();
    //my_runner.fini();

    SL_Socket_CommonAPI::socket_init(2, 2);
    //My_App::init();

    SL_Socket_Select_Runner<SL_Sync_NullMutex> my_runner;
    SL_Socket_UdpSource my_udpsource;
    My_UdpHandler my_udphandler;

    my_udpsource.set_config(false, 1024);
    my_udpsource.set_interface(&my_udphandler,&my_runner, &log);
    my_runner.open();
    my_udpsource.open(0);

    SL_Socket_INET_Addr sl_addr;
    sl_addr.set("127.0.0.1",1000);
    my_runner.thread_event_loop(100);
    while (1)
    {
        Sleep(100);
        my_udphandler.write_message("hello ÕÅ±£Ôª", 13, sl_addr);
        //my_runner.event_loop();
    }
    my_runner.thread_wait();
    my_udpsource.close();

    return 0;
}
