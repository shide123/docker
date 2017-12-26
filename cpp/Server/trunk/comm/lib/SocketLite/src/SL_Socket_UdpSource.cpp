#include "SL_Socket_UdpSource.h"
#include "SL_Socket_UdpSource_Handler.h"
#include "SL_Socket_CommonAPI.h"
#include "SL_ByteBuffer.h"

SL_Socket_UdpSource::SL_Socket_UdpSource()
{
    set_config();
}

SL_Socket_UdpSource::~SL_Socket_UdpSource()
{
    close();
}

int SL_Socket_UdpSource::set_config(bool is_recv_thread, uint recvbuffer_size)
{
    is_recv_thread_  = is_recv_thread;
    recvbuffer_size_ = recvbuffer_size;
    return 0;
}

int SL_Socket_UdpSource::set_interface(SL_Socket_UdpSource_Handler *udpsource_handler, SL_Socket_Runner *socket_runner, SL_Log *log)
{
    socket_handler_ = udpsource_handler;
    socket_runner_  = socket_runner;
    log_            = log;
    return 0;
}

int SL_Socket_UdpSource::open(ushort local_port, const char *local_name, bool is_bind_localaddr, bool is_ipv6)
{
	int res = 0;
    int address_family = is_ipv6 ? AF_INET6:AF_INET;
	SL_SOCKET fd = SL_Socket_CommonAPI::socket_open(address_family, SOCK_DGRAM, IPPROTO_UDP);
	if (SL_INVALID_SOCKET == fd)
    {
		return -1;
    }
	if (socket_handler_->handle_open(fd, this, socket_runner_) < 0)
	{
		res = -2;
		goto error_exit;
	}
    if (is_bind_localaddr)
    {
	    res = local_addr_.set(local_name, local_port, is_ipv6);
	    if (res < 0)
        {
		    return -3;
        }
        SL_Socket_CommonAPI::socket_set_reuseaddr(fd, 1);
        res = SL_Socket_CommonAPI::socket_bind(fd, local_addr_.get_addr(), local_addr_.get_addr_size());
	    if (res < 0)
	    {
		    res = -4;
		    goto error_exit;
	    }
        if (is_recv_thread_)
        {
            SL_Socket_CommonAPI::socket_set_block(fd, true);
            if (recv_thread_.start(recv_proc, this) < 0)
            {
		        res = -5;
		        goto error_exit;
            }
            return 0;
        }
    }
    if (socket_runner_ != NULL)
    {
        is_recv_thread_ = false;
        return socket_runner_->add_handle(socket_handler_, SL_Socket_Handler::READ_EVENT_MASK);
    }
    return 0;

error_exit:
	socket_handler_->set_socket(SL_INVALID_SOCKET);
	SL_Socket_CommonAPI::socket_close(fd);

    return 0;
}

int SL_Socket_UdpSource::close()
{
    if (socket_handler_ != NULL)
    {
        if (is_recv_thread_)
        {
            recv_thread_.stop();
        }
        else
        {
            socket_runner_->remove_handle(socket_handler_);
        }
        SL_Socket_CommonAPI::socket_close(socket_handler_->get_socket());
        socket_handler_->set_socket(SL_INVALID_SOCKET);
	    socket_handler_->handle_close();
    }
    return 0;
}

#ifdef SOCKETLITE_OS_WINDOWS
unsigned int WINAPI SL_Socket_UdpSource::recv_proc(void *arg)
#else
void* SL_Socket_UdpSource::recv_proc(void *arg)
#endif
{
    SL_Socket_UdpSource *udpsource = (SL_Socket_UdpSource*)arg;
    SL_Socket_UdpSource_Handler *udpsource_handler = udpsource->get_socket_handler();
    SL_SOCKET udp_fd = udpsource->get_socket_handler()->get_socket();

    SL_Socket_INET_Addr sl_addr(udpsource->get_addr().is_ipv6());
    sockaddr      *addr    = sl_addr.get_addr(); 
    int           addrlen  = sl_addr.get_addr_size();

    int           raw_buf_size = udpsource->get_recvbuffer_size();
    SL_ByteBuffer buf(raw_buf_size);
    char          *raw_buf = buf.buffer();

    int res;
    int error;
	while (1)
	{
        if (!udpsource->recv_thread_.get_running())
        {
            break;
        }
        res = SL_Socket_CommonAPI::socket_recvfrom(udp_fd, raw_buf, raw_buf_size, 0, addr, &addrlen, NULL, &error);
        if (res > 0)
        {
            udpsource_handler->handle_read(raw_buf, res, sl_addr);
        }
    }
    return 0;
}
