#ifndef RPCSERVER_H
#define RPCSERVER_H

#include "rpcclienthandler.h"
#include "io_service_pool.h"
#include "ProtocolsBase.h"
#include <sys/sem.h>
#include <sys/shm.h>
#include <thrift/TProcessor.h>
#include "thrift/transport/TSocket.h"
#include "thrift/protocol/TBinaryProtocol.h"
#include "thrift/transport/TBufferTransports.h"
using namespace apache::thrift;
using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;
class rpcserver
{
public:
	rpcserver();
	void setProcessor(TProcessor* processor);
	void init(u_int16_t port);
	void fork_notify();
	void start();
private:
	TProcessor* m_processor;
	int port_;
protected:
    void handle_accept(rpcclienthandler_ptr session, const boost::system::error_code& e);
    void start_accept();
    io_service_pool io_service_pool_;
    boost::asio::ip::tcp::acceptor acceptor_;

};

#endif // RPCSERVER_H
