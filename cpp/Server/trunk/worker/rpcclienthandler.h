#ifndef rpcclienthandler_H
#define rpcclienthandler_H

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
using boost::asio::ip::tcp;
#include "message_vchat.h"
#include "io_service_pool.h"
#include <thrift/TProcessor.h>
#include "thrift/transport/TSocket.h"
#include "thrift/protocol/TBinaryProtocol.h"
#include "thrift/transport/TBufferTransports.h"
#include "thrift/transport/THttpTransport.h"
#include "thrift/transport/THttpServer.h"
using namespace apache::thrift;
using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;

class rpcclienthandler : public boost::enable_shared_from_this<rpcclienthandler>
{
public:
	rpcclienthandler(boost::asio::io_service & ioservice,TProcessor* processor);
	virtual ~rpcclienthandler();
	int open( int fd);
protected:
	void close();
	void handle_read(const boost::system::error_code & e, std::size_t bytes_transferred);
	int parsePkg();
	void transition();
private:
//	int sfd;
	TProcessor* m_processor;
	boost::shared_ptr<TSocket> t_socket;
	boost::shared_ptr<TMemoryBuffer> inputMemoryBuffer;
	//
	boost::shared_ptr<TTransport> inputtransports;
	boost::shared_ptr<TBinaryProtocol> inputprotocol;
	//
	boost::shared_ptr<TTransport> outputtransports;
	boost::shared_ptr<TBinaryProtocol> outputprotocol;
	/// Transport to read from

	boost::asio::io_service & ioservice_;
	char recv_buf_[en_msgbuffersize];
	int  recv_buf_remainlen_;
public:
	tcp::socket socket_;
};

typedef boost::shared_ptr<rpcclienthandler> rpcclienthandler_ptr;

#endif // clienthandler_H
