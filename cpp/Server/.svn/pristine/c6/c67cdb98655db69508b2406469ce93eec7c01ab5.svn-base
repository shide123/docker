
//io_service_pool.hpp

#ifndef __IO_SERVICE_POOL__HH_20150606__
#define __IO_SERVICE_POOL__HH_20150606__

#include <boost/asio.hpp>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

//a pool of io_service objects.
class io_service_pool : private boost::noncopyable
{
public:
	//Construct the io_service pool.
	explicit io_service_pool(std::size_t pool_size);
	~io_service_pool();
	void run();
	void loop();
	void stop();
	boost::asio::io_service& get_io_service(int index = -1);
	std::size_t size(){return io_services_.size();}
	void fork_notify();
private:
	typedef boost::shared_ptr<boost::asio::io_service> io_service_ptr;
	typedef boost::shared_ptr<boost::asio::io_service::work> work_ptr;
	std::vector<boost::shared_ptr<boost::thread> > threads;

	std::vector<io_service_ptr> io_services_;
	std::vector<work_ptr> works_;

	boost::mutex io_service_mutex_;
	std::size_t next_io_service_;

};

#endif //__IO_SERVICE_POOL__HH_20150606__

