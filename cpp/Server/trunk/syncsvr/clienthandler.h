#ifndef clienthandler_H
#define clienthandler_H

#include "sockethandler.h"
#include "message_comm.h"
#include "message_vchat.h"


#define TASK_TYPE_CLIENT_NETMSG     0            //客户端网络消息
#define TASK_TYPE_WEBSERVER_MSG		1			 //来自webserver的消息（比如 砸金蛋）
#define TASK_TYPE_REDISSYNC_MSG		2			 //redis 同步

class server;
class clienthandler : public boost::enable_shared_from_this<clienthandler>,
	public sockethandler
{
public:
    clienthandler(boost::asio::io_service& ioservice);
    ~clienthandler();    

    int open(server* fromsvr, int fd, u_int64_t connid);
    void release();
    void check_deadline(const boost::system::error_code& e);
    void write_message(const char* pdata, int datalen);
    void write_message(SL_ByteBuffer& slByte);

protected:
    void handle_read(const boost::system::error_code& e, std::size_t bytes_transferred);
    int handle_message(const char* pdata, int msglen);
    void handle_write(const boost::system::error_code& e, std::size_t bytes_transferred);
    void send_message(SL_ByteBuffer& slbuf);

public:
    uint32_t userid;
    uint32_t roomid;
    uint16_t svrid;
    uint64_t last_update_logindex;
    REDIS_SYNC_STATUS remote_sync_status;

protected:
    bool bSayHello;
    time_t lastalivetime;

};

typedef boost::shared_ptr<clienthandler> clienthandler_ptr;

//固定格式
struct MSG_PROC_TASK_NODE
{
	void write_message(const char *data, int nlen){
		if (connid){
			sockethandler::conns_Mutex.lock();
			clienthandler* phandler = (clienthandler*)sockethandler::conns[connid];
			if (phandler && phandler == from_connectPtr){
				phandler->write_message(data, nlen);
			}
			else
				LOG_PRINT(log_error, "MSG_PROC_TASK_NODE clienthandler: %llu, connid: %u is release!", from_connectPtr, connid);
			sockethandler::conns_Mutex.unlock();
		}
	}

	uint32 task_type;          //task类型: 0-客户端网络消息,1-centersvr 网络消息,其他见上面描述
    uint32 wparam;             //附加参数1,用于定时器
	uint32 lparam;             //附加参数2,用于定时器
	clienthandler* from_connectPtr; //来源客户端对象(仅限客户端网络消息)
	uint32_t connid;
	char* pdata;
};

#endif // clienthandler_H
