#ifndef SOCKETLITE_SOCKET_TCPCLIENT_AUTOCONNECT_H
#define SOCKETLITE_SOCKET_TCPCLIENT_AUTOCONNECT_H

#include <set>
#include "SL_Config.h"
#include "SL_Thread.h"
#include "SL_Socket_TcpClient.h"

class SOCKETLITE_API SL_Socket_TcpClient_AutoConnect
{
public:
    SL_Socket_TcpClient_AutoConnect();
    ~SL_Socket_TcpClient_AutoConnect();
    int open(uint check_interval=5000);
    int close();

    int add_tcpclient(SL_Socket_TcpClient *tcpclient);
    int del_tcpclient(SL_Socket_TcpClient *tcpclient);
	int have_tcpclient(SL_Socket_TcpClient *tcpclient);

private:
#ifdef SOCKETLITE_OS_WINDOWS
    static unsigned int WINAPI autoconnect_proc(void *arg);
#else
    static void* autoconnect_proc(void *arg);
#endif

    SL_Sync_ThreadMutex             mutex_;
    std::set<SL_Socket_TcpClient*>  tcpclient_set_;
    SL_Thread<SL_Sync_ThreadMutex>  autoconnect_thread_;
    uint                            check_interval_;
};

#endif
