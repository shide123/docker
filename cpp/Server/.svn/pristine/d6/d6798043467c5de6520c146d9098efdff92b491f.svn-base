#include "SL_Socket_TcpServer.h"
#include "SL_Socket_TcpServer_Handler.h"
#include "SL_Socket_CommonAPI.h"

int SL_Socket_TcpServer::set_config(ushort accept_thread_num,
                                    bool   is_addhandler,
                                    uint   maxconnect_num,
                                    uint   recvbuffer_size,
                                    uint   msgbuffer_size,
                                    uint8  msglen_bytes,
                                    uint8  msg_byteorder)
{
    accept_thread_num_  = accept_thread_num;
    is_addhandler_      = is_addhandler;
    maxconnect_num_     = maxconnect_num;
    recvbuffer_size_    = recvbuffer_size;
    msgbuffer_size_     = msgbuffer_size;
    msglen_bytes_       = msglen_bytes;
    msg_byteorder_      = msg_byteorder;
    return 0;
}

int SL_Socket_TcpServer::set_interface(SL_Socket_TcpServer_Handler *tcpserver_handler, SL_Socket_Runner *socket_runner, SL_Log *log)
{
    socket_handler_ = tcpserver_handler;
    socket_runner_  = socket_runner;
    log_            = log;
    return 0;
}

int SL_Socket_TcpServer::open(ushort local_port, int backlog, const char *local_name, bool is_ipv6)
{
	int res = local_addr_.set(local_name, local_port, is_ipv6);
	if (res < 0)
    {
		return -1;
    }
	printf("1\n");
	int address_family = is_ipv6 ? AF_INET6:AF_INET;
	SL_SOCKET fd = SL_Socket_CommonAPI::socket_open(address_family, SOCK_STREAM, IPPROTO_TCP);
	if (SL_INVALID_SOCKET == fd)
    {
		return -2;
    }
	printf("1\n");
	if (socket_handler_->handle_open(fd, this, socket_runner_) < 0)
	{
		res = -3;
		goto error_exit;
	}
printf("1\n");
    SL_Socket_CommonAPI::socket_set_reuseaddr(fd, 1);
	res = SL_Socket_CommonAPI::socket_bind(fd, local_addr_.get_addr(), local_addr_.get_addr_size());
	if (res < 0)
	{
		res = -4;
		goto error_exit;
	}
	printf("1\n");
	res = SL_Socket_CommonAPI::socket_listen(fd, backlog);
	if (res < 0)
	{
		res = -5;
		goto error_exit;
	}
	printf("1\n");
    if (accept_thread_num_ > 0)
    {
        SL_Socket_CommonAPI::socket_set_block(fd, true);
        if (accept_thread_group_.start(accept_proc, this, accept_thread_num_, accept_thread_num_) < 0)
        {
		    res = -6;
		    goto error_exit;
        }
        return 0;
    }
	printf("1\n");
    return socket_runner_->add_handle(socket_handler_, SL_Socket_Handler::READ_EVENT_MASK);

error_exit:
	socket_handler_->set_socket(SL_INVALID_SOCKET);
	SL_Socket_CommonAPI::socket_close(fd);
	return res;

}

int SL_Socket_TcpServer::close()
{
    if (socket_handler_ != NULL)
    {
        if (accept_thread_num_ > 0)
        {
            accept_thread_group_.stop();
        }
	    SL_Socket_CommonAPI::socket_close(socket_handler_->get_socket());
	    socket_handler_->set_socket(SL_INVALID_SOCKET);
        socket_handler_->handle_close();
    }
	return 0;
}

#ifdef SOCKETLITE_OS_WINDOWS
unsigned int WINAPI SL_Socket_TcpServer::accept_proc(void *arg)
#else
void* SL_Socket_TcpServer::accept_proc(void *arg)
#endif
{
    SL_Socket_TcpServer *tcpserver = (SL_Socket_TcpServer*)arg;
    SL_Socket_INET_Addr sl_addr(tcpserver->get_addr().is_ipv6());
    sockaddr *addr = sl_addr.get_addr();     
    int addrlen = sl_addr.get_addr_size();

    SL_SOCKET client_socket;
    SL_SOCKET listen_fd = tcpserver->socket_handler_->get_socket();

	while (1)
	{
        if (!tcpserver->accept_thread_group_.get_running())
        {
            break;
        }
        client_socket = SL_Socket_CommonAPI::socket_accept(listen_fd, addr, &addrlen);
		if (SL_INVALID_SOCKET == client_socket)
		{
			continue;
		}
        if (tcpserver->get_socket_handler()->handle_accept(client_socket, sl_addr) < 0)
        {
            SL_Socket_CommonAPI::socket_close(client_socket);
        }
    }
    return 0;
}
