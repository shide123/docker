#ifndef _MY_SOCKET_TCPSERVER_HANDLER
#define _MY_SOCKET_TCPSERVER_HANDLER

#include "SL_Socket_TcpServer_Handler.h"
#include "SL_ObjectPool.h"
#include "SL_Sync_Mutex.h"
#include "SL_Socket_IOChannel.h"
#include "SL_Sync_Mutex.h"

class My_Socket_Handler:public SL_Socket_IOChannel
{
public:
	int handle_open(SL_SOCKET fd, SL_Socket_Source *socket_source, SL_Socket_Runner *socket_runner);
	virtual int handle_close();
    virtual int process_message(const char *msg, int len);
private:    
    SL_Sync_ThreadMutex m_mtx;

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