#ifndef SERVER_H
#define SERVER_H

#include "clienthandler.h"
#include "io_service_pool.h"

class clienthandler;

class server
{
public:
    server(u_int16_t port, std::size_t io_service_pool_size = 1);
    ~server();

    void run();
    u_int64_t newconnid() {
    	boost::mutex::scoped_lock lock(connid_mutex_);
    	return ++connid;
    }
    void setgateclient(uint16_t gateid, clienthandler *pgateclient);
    clienthandler* getgateclient(uint16_t gateid);
    uint16 getsvrid() const { return svrid; }
    void setsvrid(uint16 id) { svrid = id; }
protected:
    void start_accept();
    void handle_accept(clienthandler_ptr session, const boost::system::error_code& e);
    void handle_stop();

public:
    boost::mutex client_mutex_;
    typedef map <uint16, clienthandler*> GATE_CLIENT_MAP; //map:[gateid, clienthandler*]
    GATE_CLIENT_MAP gateclientmap;
protected:
    io_service_pool io_service_pool_;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::mutex connid_mutex_;
    u_int16_t port_;
    int sockfd;
    int totalconns;
    u_int64_t connid;
    uint16_t svrid;

};

#endif // SERVER_H
