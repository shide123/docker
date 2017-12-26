
//io_service_pool.cpp

#include "io_service_pool.h"
#include <stdexcept>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

io_service_pool::io_service_pool(std::size_t pool_size)
:next_io_service_(0)
{
	if(pool_size==0)
		throw std::runtime_error("io_service_pool size is 0");
	for(std::size_t i=0; i<pool_size;i++)
	{
		io_service_ptr io_service(new boost::asio::io_service);
		work_ptr work(new boost::asio::io_service::work(*io_service));
		io_services_.push_back(io_service);
		works_.push_back(work);
	}
}

io_service_pool::~io_service_pool()
{
	loop();
}

void io_service_pool::run()
{

	for(std::size_t i=0;i<io_services_.size(); ++i)
	{
		boost::shared_ptr<boost::thread> thread(new boost::thread(
			boost::bind(&boost::asio::io_service::run, io_services_[i])));
		threads.push_back(thread);
	}

//	wait for all threads in the pool to exit
//	for(std::size_t i=0; i<threads.size(); ++i)
//		threads[i]->join();
}

void io_service_pool::loop()
{
	for(std::size_t i=0; i<threads.size(); ++i)
		threads[i]->join();
}

void io_service_pool::stop()
{
	//Explicitly stop all io_services
	for(std::size_t i=0;i<io_services_.size(); ++i)
		io_services_[i]->stop();
}

boost::asio::io_service& io_service_pool::get_io_service(/*int& index*/)
{
	boost::asio::io_service& io_service =*io_services_[next_io_service_];
//	index = next_io_service_;
	++next_io_service_;
	if(next_io_service_ == io_services_.size())
		next_io_service_=0;
	return io_service;
}
void io_service_pool::fork_notify()
{
	for(std::size_t i=0;i<io_services_.size(); i++)
	{
		io_services_[i]->notify_fork(boost::asio::io_service::fork_child);
	}
}
