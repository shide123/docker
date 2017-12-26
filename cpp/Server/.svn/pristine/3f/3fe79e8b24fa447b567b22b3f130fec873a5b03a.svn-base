#ifndef SOCKETLITE_SOCKET_TCPSERVER_H
#define SOCKETLITE_SOCKET_TCPSERVER_H

#include "SL_Config.h"
#include "SL_Socket_INET_Addr.h"
#include "SL_Socket_Source.h"
#include "SL_Socket_Runner.h"
#include "SL_Thread_Group.h"
#include "SL_Log.h"

class SL_Socket_TcpServer_Handler;
class SL_Socket_TcpServer : public SL_Socket_Source
{
public:
    SL_Socket_TcpServer()
    {
        set_config();
    }

	~SL_Socket_TcpServer()
    { 
        close();
    }

    inline SL_Socket_TcpServer_Handler* get_socket_handler()
    {
        return (SL_Socket_TcpServer_Handler*)socket_handler_;
    }

	inline SL_Socket_INET_Addr& get_addr()
	{
		return local_addr_;
	}

    int set_config(ushort accept_thread_num = 1,
                   bool   is_addhandler     = true,
                   uint   maxconnect_num    = 100000,
                   uint   recvbuffer_size   = 4096,
                   uint   msgbuffer_size    = 4096,
                   uint8  msglen_bytes      = 4,
                   uint8  msg_byteorder     = 0);
    int set_interface(SL_Socket_TcpServer_Handler *tcpserver_handler, SL_Socket_Runner *socket_runner, SL_Log *log);

	int open(ushort local_port, int backlog=100, const char *local_name=NULL, bool is_ipv6=false);
    int close();

private:
#ifdef SOCKETLITE_OS_WINDOWS
    static unsigned int WINAPI accept_proc(void *arg);
#else
    static void* accept_proc(void *arg);
#endif

protected:
    SL_Socket_INET_Addr local_addr_;

    uint                maxconnect_num_;
    ushort              accept_thread_num_;
    SL_Thread_Group     accept_thread_group_;
};

#endif
