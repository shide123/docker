#ifndef CONNECHANDLER_H
#define CONNECHANDLER_H
#include "sockethandler.h"

class redisOpt;

class connecthandler : public boost::enable_shared_from_this<connecthandler>,
		public sockethandler
{
public:
    connecthandler(boost::asio::io_service& io_service);
    ~connecthandler();

    void start_connect(const char *addr, u_int16_t port, server *svr);
    void write_message(SL_ByteBuffer& message, bool bforced = false);
    void write_message(char* pdata, int datalen, bool bforced = false);

    void send_hello();
    void report_sync_status();
    void Req_sync_full_data();
    void syncsvr_notify(CMDRoomsvrNotify_t &data);
    redisOpt *getRedis() const { return pRemote_Redis; }

public:
    void addclientid(u_int64_t client_connid);
    void delclientid(u_int64_t client_connid);
	void setgateid(uint16 idgate) { m_ngateid = idgate; }

protected:    
    void release();
    void stop();
    int handle_message(const char* pdata, int msglen);
    void handle_connect(const boost::system::error_code& e);
    void handle_timeout(const boost::system::error_code& e);
    void handle_read(const boost::system::error_code& e, std::size_t bytes_transferred);
    void send_message(SL_ByteBuffer& message, bool bforced);
    void handle_write(const boost::system::error_code& e,  std::size_t bytes_transferred);
    int connect();
    void clear_data();

protected:
    int lastupdate;
    time_t m_nlasttime_printstack;
    boost::asio::io_service& io_service_;
    tcp::endpoint endpoint_;
	unsigned int last_activetime_;
	unsigned int begin_connecttime_;
	string 		remote_codis_ip;
	uint16_t 	remote_codis_port;
	redisOpt	*pRemote_Redis;
	uint64_t	last_sync_log_index;
	time_t		last_sync_log_time;

    SL_Thread<SL_Sync_ThreadMutex>  auto_connect_thread;
private:
	boost::mutex conn_mutex_;
	uint16 m_ngateid; 
	vector<u_int64_t> vec_client_connid;
	int disconn_report_times;
};

typedef boost::shared_ptr<connecthandler> connecthandler_ptr;
#endif // CONNECHANDLER_H

