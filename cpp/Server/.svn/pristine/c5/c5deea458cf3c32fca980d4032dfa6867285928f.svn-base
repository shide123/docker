#ifndef SOCKETLITE_SOCKET_UDPSOURCE_H
#define SOCKETLITE_SOCKET_UDPSOURCE_H

#include "SL_Config.h"
#include "SL_Socket_Source.h"
#include "SL_Socket_Runner.h"
#include "SL_Socket_INET_Addr.h"
#include "SL_Thread.h"
#include "SL_Log.h"

class SL_Socket_UdpSource_Handler;
class SL_Socket_UdpSource : public SL_Socket_Source
{
public:
    SL_Socket_UdpSource();
    virtual ~SL_Socket_UdpSource();

    inline SL_Socket_UdpSource_Handler* get_socket_handler()
    {
        return (SL_Socket_UdpSource_Handler*)socket_handler_;
    }

	inline SL_Socket_INET_Addr& get_addr()
	{
		return local_addr_;
	}

    int set_config(bool is_recv_thread=false, uint recvbuffer_size=1024);
    int set_interface(SL_Socket_UdpSource_Handler *udpsource_handler, SL_Socket_Runner *socket_runner, SL_Log *log);

	int open(ushort local_port=0, const char *local_name=NULL, bool is_bind_localaddr=true, bool ipv6=false);
    int close();

private:
#ifdef SOCKETLITE_OS_WINDOWS
    static unsigned int WINAPI recv_proc(void *arg);
#else
    static void* recv_proc(void *arg);
#endif

protected:
    SL_Socket_INET_Addr local_addr_;
    bool                is_recv_thread_;
    SL_Thread<SL_Sync_ThreadMutex> recv_thread_;
};

#endif
