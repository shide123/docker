#include "rpcserver.h"
#include "clienthandler.h"
#include "CLogThread.h"
#include "Application.h"
rpcserver::rpcserver()
:io_service_pool_(1)
,acceptor_(io_service_pool_.get_io_service())
{
	m_processor = NULL;
}

void rpcserver::setProcessor(TProcessor* processor)
{
	m_processor = processor;
}

void rpcserver::init(u_int16_t port)
{
	if(port == 0)
		return ;
	port_ = port;
	tcp::resolver resolver(acceptor_.get_io_service());
	char sport[8];
	sprintf(sport, "%d", port);
	tcp::resolver::query query("0.0.0.0", sport);
	tcp::resolver::iterator endpoint_iter = resolver.resolve(query);
	tcp::endpoint endpoint = *endpoint_iter;
	acceptor_.open(endpoint.protocol());
	acceptor_.set_option(tcp::acceptor::reuse_address(true));
	acceptor_.bind(endpoint);
	acceptor_.listen();

}

void rpcserver::fork_notify()
{
	io_service_pool_.fork_notify();
}
void rpcserver::start_accept()
{
	printf("start_accept\n");
	rpcclienthandler_ptr conn(new rpcclienthandler(io_service_pool_.get_io_service(),m_processor));
	acceptor_.async_accept(conn->socket_, boost::bind(&rpcserver::handle_accept, this, conn, boost::asio::placeholders::error));
}
void rpcserver::handle_accept(rpcclienthandler_ptr session, const boost::system::error_code& e)
{
	if(!e)
	{
		if (session.get()){
			printf("handle_accept\n");
			session->open(session->socket_.native_handle());
//			boost::shared_ptr<TSocket> t_socket(new TSocket());
//			t_socket->setSocketFD(session->socket_.native_handle());
//			boost::shared_ptr<TBufferedTransport> t_transports(new TBufferedTransport(t_socket));
//			boost::shared_ptr<TBinaryProtocol> t_protocol(new TBinaryProtocol(t_transports));
		}
	}
	else
	{
		LOG_PRINT(log_error, "%s",boost::system::system_error(e).what());
	}
	io_service_pool_.get_io_service().post(boost::bind(&rpcserver::start_accept, this));
}

void rpcserver::start()
{
	if(port_ == 0)
			return ;
	io_service_pool_.get_io_service().post(boost::bind(&rpcserver::start_accept, this));
	io_service_pool_.run();
}
