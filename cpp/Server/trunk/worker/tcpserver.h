#ifndef SERVER_H
#define SERVER_H

#include "clienthandler.h"
#include "io_service_pool.h"
#include "ProtocolsBase.h"
#include <sys/sem.h>
#include <sys/shm.h>
class clienthandler;
//template<typename T>
/*
union semun{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};
*/

typedef struct {
	int pid;
	int index;
	int conns;
} PID_CONNS;

class tcpserver
{
public:
    //server(u_int16_t port, std::size_t io_service_pool_size = 1);
    tcpserver();
    ~tcpserver();

	void init(u_int16_t port, int procnum,std::size_t io_service_pool_size = 1,std::string protocol = "",IWorkerEvent* event = NULL,int max_conns = 1000);
	void delshm();
	void start(int index);
    void fork_notify();
    u_int64_t newconnid() {
    	boost::mutex::scoped_lock lock(connid_mutex_);
    	return ++connid;
    }

    uint16 getsvrid() const { return svrid; }
    void setsvrid(uint16 id) { svrid = id; }
	void client_release();

	static void setconnhandler(unsigned int connid, clienthandler_ptr handler);
	static clienthandler_ptr getconnhandler(unsigned int connid);
	static void getallconnhandler(std::list<clienthandler_ptr> & lstOut);
	static unsigned int cast_msg_to_allconn(const char * pdata, unsigned int len);
	static void eraseconnhandler(unsigned int connid);
	static unsigned int getconns_size();
	static unsigned int getconnid();

protected:
    void start_accept();
	void handle_accept(clienthandler_ptr session, const boost::system::error_code& e);
    void handle_stop();
    //void wait(int index);
    //void notify(int index);
    bool initShm(int size);
    int getminindex();
    int getmaxindex();
public:
    boost::condition_variable_any cond_;
    boost::mutex cond_mutex;
	typedef map <uint32_t, clienthandler_ptr> CONN_MAP;

protected:
	static CONN_MAP conns;
	static boost::mutex conns_Mutex;

	static unsigned int connid_;
	static boost::mutex m_condid_mutex;

    io_service_pool io_service_pool_;
    io_service_pool io_service_pool_single;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::mutex connid_mutex_;
    u_int16_t port_;
    int sockfd;
    int totalconns;
    u_int64_t connid;
    uint16_t svrid;
    int m_max_conns;
private:
	std::string			m_strProtocol;
    IWorkerEvent* m_workerEvent;
    //int semid;
    PID_CONNS* pcn;
	int m_shmId;
    int m_procnum;
    int m_index;
};

#endif // SERVER_H
