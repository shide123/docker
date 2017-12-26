#include "server.h"
#include "clienthandler.h"
#include "CLogThread.h"

server::server(u_int16_t port, std::size_t io_service_pool_size)
    : io_service_pool_(io_service_pool_size)
	, acceptor_(io_service_pool_.get_io_service())
	, port_(port)
    , sockfd(-1)
    , totalconns(0)
	, connid(0)
	, svrid(0)
{
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

	LOG_PRINT(log_info, "syncsvr start listen[%s:%d]", endpoint.address().to_string().c_str(), endpoint.port());

	start_accept();
}

server::~server()
{
    port_ = 0;

    close(sockfd);
    sockfd = -1;    
}

void server::run()
{
	io_service_pool_.run();
	io_service_pool_.loop();
}

void server::start_accept()
{
	clienthandler_ptr conn(new clienthandler(io_service_pool_.get_io_service()));
	acceptor_.async_accept(conn->socket_, boost::bind(&server::handle_accept, this, conn, boost::asio::placeholders::error));
}

void server::handle_accept(clienthandler_ptr session, const boost::system::error_code& e)
{
	if(!e)
	{
	    if (session.get()){
	    	session->open(this, session->socket_.native_handle(), newconnid());
	        totalconns++;
	        LOG_PRINT(log_info, "toatal connections number: %d", totalconns);
	    }
	}
	else
	{
		LOG_PRINT(log_error, "%s",boost::system::system_error(e).what());
	}

	start_accept();
}

void server::handle_stop()
{
	io_service_pool_.stop();
}

void server::setgateclient(uint16_t gateid, clienthandler *pgateclient)
{
	boost::mutex::scoped_lock(client_mutex_);
	gateclientmap[gateid] = pgateclient;

}

clienthandler* server::getgateclient(uint16_t gateid)
{
	boost::mutex::scoped_lock(client_mutex_);
	return gateclientmap[gateid];
}
