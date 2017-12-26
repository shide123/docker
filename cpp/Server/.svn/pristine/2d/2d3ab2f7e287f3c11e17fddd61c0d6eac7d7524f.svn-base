#ifndef SOCKETLITE_SOCKET_UDPSOURCE_HANDLER_H
#define SOCKETLITE_SOCKET_UDPSOURCE_HANDLER_H

#include "SL_Socket_CommonAPI.h"
#include "SL_Socket_Handler.h"
#include "SL_Socket_INET_Addr.h"
#include "SL_Socket_UdpSource.h"
#include "SL_ByteBuffer.h"

class SL_Socket_UdpSource_Handler : public SL_Socket_Handler
{
public:
    SL_Socket_UdpSource_Handler() 
    {
    }

    virtual ~SL_Socket_UdpSource_Handler() 
    {
    }

    int write_data(const char *buf, int len, SL_Socket_INET_Addr &sl_addr)
    {
        return SL_Socket_CommonAPI::socket_sendto(socket_, buf, len, 0, sl_addr.get_addr(), sl_addr.get_addr_size());
    }

    virtual int handle_read(const char *buf, int len, SL_Socket_INET_Addr &sl_addr) 
    { 
        return 0; 
    }

protected:
	int handle_event(int event_mask)
    {
        SL_Socket_UdpSource *udpsource = (SL_Socket_UdpSource*)socket_source_;
        SL_Socket_INET_Addr sl_addr(udpsource->get_addr().is_ipv6());

        sockaddr    *addr        = sl_addr.get_addr();
        int         addrlen      = sl_addr.get_addr_size();

        int         raw_buf_size = socket_runner_->get_recv_buffer_size();
        char        *raw_buf     = socket_runner_->get_recv_buffer();

        int res;
        int error;
        res = SL_Socket_CommonAPI::socket_recvfrom(socket_, raw_buf, raw_buf_size, 0, addr, &addrlen, NULL, &error);
        if (res > 0)
        {
            handle_read(raw_buf, res, sl_addr);
        }
        return 0;
    }

};

#endif
