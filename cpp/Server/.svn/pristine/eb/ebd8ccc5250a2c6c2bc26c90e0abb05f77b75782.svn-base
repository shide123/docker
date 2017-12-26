#ifndef SOCKETLITE_SOCKET_TCPCLIENT_H
#define SOCKETLITE_SOCKET_TCPCLIENT_H

#include <vector>
#include <string>
#include "SL_Config.h"
#include "SL_Socket_Source.h"
#include "SL_Socket_INET_Addr.h"

class SL_Socket_TcpClient : public SL_Socket_Source
{
public:
	SL_Socket_TcpClient()
        : local_port_(0)
        , is_bind_localaddr_(false)
        , connect_serveraddr_pos_(0)
        , connect_timeout_(15000)
        , is_connected_(false)
        , is_autoconnect_(false)
    {
        serveraddr_list_.reserve(4);
        set_config();
    }

	~SL_Socket_TcpClient()
    {
        close();
		clear();
    }

	int open(const char *server_name, ushort server_port, const char *local_name=NULL, ushort local_port=0, bool is_bind_localaddr=false, bool is_ipv6=false);
    int open();
	int close(bool bclearaddr=true);
	void clear();
    int connect();

    int send(const char *buf, int len, int flags);
    int recv(char *buf, int len, int flags);

    int set_config(ulong connect_timeout = 15000,
                   bool  is_autoconnect  = false,
                   bool  is_addhandler   = true,
                   uint  recvbuffer_size = 4096,
                   uint  msgbuffer_size  = 4096, 
                   uint8 msglen_bytes    = 4,
                   uint8 msg_byteorder   = 0);

    int set_interface(SL_Socket_Handler *tcpclient_handler, SL_Socket_Runner *socket_runner, SL_Log *log);

    int add_serveraddr(const char *server_name, ushort server_port, bool is_ipv6=false)
    {
        SL_Socket_INET_Addr server_addr;
        server_addr.set(server_name, server_port, is_ipv6);
	    serveraddr_list_.push_back(server_addr);
        return 0;
    }

    int remove_serveraddr(int pos)
    {
        if ((pos < 0) || (pos > (int)serveraddr_list_.size()-1))
        {
            return -1;
        }
        serveraddr_list_.erase(serveraddr_list_.begin()+pos);
        return 0;
    }

	int remove_serveraddr()
	{
		serveraddr_list_.clear();
		return 0;
	}

    int get_serveraddr_count()
    {
        return (int)serveraddr_list_.size();
    }

    int get_connect_serveraddr_pos()
    {
        return connect_serveraddr_pos_;
    }

    SL_Socket_INET_Addr* get_serveraddr(int pos)
    {
        if ((pos < 0) || (pos > (int)serveraddr_list_.size()-1))
        {
            return NULL;
        }
        return &serveraddr_list_[pos];
    }

    bool get_is_connected()
    {
        return is_connected_;
    }

    bool get_is_autoconnect()
    {
        return is_autoconnect_;
    }

    void set_autoconnect(bool is_autoconnect)
    {
        is_autoconnect_ = is_autoconnect;

    }

protected:
    int handle_disconnect()
    {
        is_connected_ = false;
        return 0;
    }

protected:
    std::vector<SL_Socket_INET_Addr> serveraddr_list_;
    std::string     local_name_;
    ushort          local_port_;
    bool            is_bind_localaddr_;

    uint            connect_serveraddr_pos_;
    ulong           connect_timeout_;
    volatile bool   is_connected_;                  //连接状态(fase:失败,true:成功)
    volatile bool   is_autoconnect_;                //是否自动重连
};

#endif
