#include "SL_Socket_TcpClient.h"
#include "SL_Socket_CommonAPI.h"

int SL_Socket_TcpClient::set_config(ulong connect_timeout,
                                    bool  is_autoconnect,
                                    bool  is_addhandler,
                                    uint  recvbuffer_size,
                                    uint  msgbuffer_size, 
                                    uint8 msglen_bytes,
                                    uint8 msg_byteorder)
{
    connect_timeout_    = connect_timeout;
    is_autoconnect_     = is_autoconnect;
    is_addhandler_      = is_addhandler;
    recvbuffer_size_    = recvbuffer_size;
    msgbuffer_size_     = msgbuffer_size;
    msglen_bytes_       = msglen_bytes;
    msg_byteorder_      = msg_byteorder;
    return 0;
}

int SL_Socket_TcpClient::set_interface(SL_Socket_Handler *tcpclient_handler, SL_Socket_Runner *socket_runner, SL_Log *log)
{
    socket_handler_ = tcpclient_handler;
    socket_runner_  = socket_runner;
    log_            = log;
    tcpclient_handler->set_socket_source(this);
    tcpclient_handler->set_socket_runner(socket_runner);
    return 0;
}

int SL_Socket_TcpClient::open(const char *server_name, 
                              ushort server_port, 
                              const char *local_name, 
                              ushort local_port, 
                              bool is_bind_localaddr, 
                              bool is_ipv6)
{
    local_name_         = local_name ? local_name:"";
    local_port_         = local_port;
    is_bind_localaddr_  = is_bind_localaddr;
    serveraddr_list_.clear();
    add_serveraddr(server_name, server_port, is_ipv6);
    connect_serveraddr_pos_ = serveraddr_list_.size()-1;
    if (!is_autoconnect_)
    {
        return connect();
    }
    return 0;
}

int SL_Socket_TcpClient::open()
{
    if (!is_autoconnect_)
    {
        return connect();
    }
    return -1;
}

int SL_Socket_TcpClient::close(bool bclearaddr/*=true*/)
{
    is_autoconnect_ = false;
    is_connected_   = false;
    if (socket_handler_ != NULL)
    {
        if (is_addhandler_)
        {
            socket_runner_->remove_handle(socket_handler_);
        }
        SL_Socket_CommonAPI::socket_close(socket_handler_->get_socket());
        socket_handler_->set_socket(SL_INVALID_SOCKET);
    }
	if(bclearaddr)
		serveraddr_list_.clear();
    return 0;
}

void SL_Socket_TcpClient::clear()
{
	serveraddr_list_.clear();
}

int SL_Socket_TcpClient::connect()
{
    if (is_connected_)
    {
        return 0;
    }
    if ( 0 == serveraddr_list_.size())
    {
        return -1;
    }
    if (is_addhandler_)
    {
        socket_runner_->remove_handle(socket_handler_);
    }
    SL_Socket_CommonAPI::socket_close(socket_handler_->get_socket());

    if (connect_serveraddr_pos_ >= serveraddr_list_.size())
    {
        connect_serveraddr_pos_ = 0;
    }
    SL_Socket_INET_Addr &server_addr = serveraddr_list_[connect_serveraddr_pos_];
    ++connect_serveraddr_pos_;

    int address_family = server_addr.is_ipv6() ? AF_INET6:AF_INET;
	SL_SOCKET fd = SL_Socket_CommonAPI::socket_open(address_family, SOCK_STREAM, IPPROTO_TCP);
	if (SL_INVALID_SOCKET == fd)
    {
		printf("socket_open() failed, return -2.\n");
		return -2;
    }

    if (is_bind_localaddr_)
    {
        SL_Socket_INET_Addr local_addr_;
        local_addr_.set(local_name_.c_str(), local_port_);
        SL_Socket_CommonAPI::socket_set_reuseaddr(fd, 1);
        if (SL_Socket_CommonAPI::socket_bind(fd, local_addr_.get_addr(), local_addr_.get_addr_size()) < 0)
        {
            SL_Socket_CommonAPI::socket_close(fd);
            return -3;
        }
    }

    int res = 0;
    if (connect_timeout_ > 0)
    {
        struct timeval t;
        t.tv_sec  = connect_timeout_/1000;
        t.tv_usec = (connect_timeout_%1000)*1000;
        res = SL_Socket_CommonAPI::socket_connect(fd, server_addr.get_addr(), server_addr.get_addr_size(), &t);
    }
    else
    {
        res = SL_Socket_CommonAPI::socket_connect(fd, server_addr.get_addr(), server_addr.get_addr_size());
    }
    if (res < 0)
    {
		printf("socket_connect() failed, res=%d.\n", res);
        SL_Socket_CommonAPI::socket_close(fd);
        return -4;
    }
    else
    {
        socket_handler_->handle_open(fd, this, socket_runner_);
        if (is_addhandler_)
        {
            socket_runner_->add_handle(socket_handler_, SL_Socket_Handler::READ_EVENT_MASK);
        }
        is_connected_ = true;
        socket_handler_->handle_connect();
    }
    return 0;
}

int SL_Socket_TcpClient::send(const char *buf, int len, int flags)
{
    if (!is_connected_)
    {
        return 0;
    }

    int error_id = 0;
    int send_len = SL_Socket_CommonAPI::socket_send(socket_handler_->get_socket(), buf, len, flags, NULL, &error_id); 
    if (send_len <= 0)
    {
        if ( (0 == error_id) || (SL_EAGAIN == error_id)|| (SL_EWOULDBLOCK == error_id) || (SL_IO_PENDING == error_id) || (SL_ENOBUFS == error_id) )
        {//非阻塞模式下正常情况
            return 0;
        }
        is_connected_ = false;
    }
    return send_len;
}

int SL_Socket_TcpClient::recv(char *buf, int len, int flags)
{
    if (!is_connected_)
    {
        return 0;
    }

	int error_id = 0;
	int recv_len = SL_Socket_CommonAPI::socket_recv(socket_handler_->get_socket(),buf, len, flags, NULL, &error_id);
	if (recv_len == 0)
    {
		is_connected_ = false;
	}
	else if (recv_len < 0)
        {//非阻塞模式下正常情况
		if ( (SL_EAGAIN == error_id) || (SL_EWOULDBLOCK == error_id) || (SL_EINTR == error_id) )
		{//非阻塞模式下正常情况
            return 0;
        }
        is_connected_ = false;
    }
    return recv_len;
}
