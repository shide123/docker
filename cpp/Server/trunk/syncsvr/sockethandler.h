#ifndef SOCKETHANDLER_H
#define SOCKETHANDLER_H

#include <map>
#include <queue>
using namespace std;
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
using boost::asio::ip::tcp;
using boost::asio::deadline_timer;

#include "SL_Headers.h"
#include "CLogThread.h"
#include "cmd_vchat.h"

#ifndef UINT64_MAX
#define UINT64_MAX 18446744073709551615ULL
#endif

typedef struct {
	uint16 subcmd;
	struct timeval tv;
}MSG_SEND_TIME;


class server;
class sockethandler
{
public:
    sockethandler(boost::asio::io_service& ioservice);
    virtual ~sockethandler();

    bool isconnected() const { return connected; }
    bool isactived() const { return actived; }
    int socketid() const { return sfd; }
    void setconnid(uint32_t connid) { connid_ = connid; }
    uint32_t getconnid() const { return connid_; }
    const char* getremote_ip() const { return remote_ip; }
    ushort getremote_port() const { return remote_port; }
    static void setconnhandler(uint64_t connid, sockethandler* handler);
    static sockethandler* getconnhandler(uint64_t connid);
protected:
    int parse_message();
    virtual int handle_message(const char* pdata, int msglen) { return 0; }
    virtual void close();
    virtual int handle_msgsend_timeout(const char* pdata);
    virtual void handle_msgsend_record(const char* pdata);
public:
    typedef map <uint32_t, sockethandler*> CONN_MAP;
    static CONN_MAP conns;
    static SL_Sync_ThreadMutex conns_Mutex;
    tcp::socket socket_;
    deadline_timer deadline_;
    enum {ERR_READ = 1, ERR_WRITE, ERR_PACKET};
protected:
	enum {
		en_msgbuffersize     = 1024*64,
		en_msgmaxsize        = 1024*32,
		en_checkactivetime   = 5,  //s
		en_checkkeeplivetime = 20,  //s
		en_checkconntimeout = 1	//s
	};
	time_t ntime_msgsend_timeout;

    int sfd;
	//read message buffer.
	char recv_buf_[en_msgbuffersize];
	int  recv_buf_remainlen_;

	std::deque<SL_ByteBuffer > m_send_queue;
    boost::mutex send_messages_mutex_;
    int 		   msgpacklen;

    bool connected;
    bool actived;
    uint32_t connid_;
    char    remote_ip[30];
    ushort  remote_port;
    server  *serv;
    map<uint32,  MSG_SEND_TIME> mapMsgsend;
    boost::mutex map_msgsend_lock;
};

#endif // SOCKETHANDLER_H
