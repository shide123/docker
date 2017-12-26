#include "dns_resolver.h"
#include "CLogThread.h"

CDNSResolver::CDNSResolver()
    : m_service()
    , m_pthread(NULL)
    , m_lock()
    , m_def_task()
{
    std::string dnsname("");

    m_def_task.reset(new STask());
    m_def_task->dnsname = "";
    m_def_task->timer.reset(new boost_timer(m_service));

    m_alltasks[m_def_task] = "";
        
    m_def_task->timer->expires_from_now(boost::posix_time::seconds(60 * 60 * 24));
	m_def_task->timer->async_wait(
        boost::bind(&CDNSResolver::onTimeout, 
        this,
		boost::asio::placeholders::error,
		m_def_task));
    m_pthread   = new boost::thread(boost::bind(&boost::asio::io_service::run, 
                                                &m_service));
}
    
CDNSResolver::~CDNSResolver()
{
    m_service.stop();
    m_pthread->join();
    delete m_pthread;
}

void CDNSResolver::addTask(key_type key, std::string dnsname, node_ptr node)
{   
    //printf("add task:%s!\n", key.c_str());
	LOG_PRINT(log_info, "add task:%s", key.c_str());
    boost::mutex::scoped_lock lock(m_lock);

    task_type task(new STask());
    
    task->dnsname   = dnsname;
    task->node      = node;
    task->timer.reset(new boost_timer(m_service));
    
    m_taskmap[key]      = task;
    m_alltasks[task]    = key;
    
    task->timer->expires_from_now(boost::posix_time::seconds(0));
	task->timer->async_wait(
        boost::bind(&CDNSResolver::onTimeout, 
        this,
		boost::asio::placeholders::error,
		task));
}

void CDNSResolver::cancelTask(key_type key)
{
    boost::mutex::scoped_lock lock(m_lock);

    task_type task = m_taskmap[key];
    m_taskmap.erase(key);
    
    //printf("cancel task:%s!\n", key.c_str());

    if (task.get() != NULL) {
		LOG_PRINT(log_info, "remove task:%s", key.c_str());
        m_alltasks.erase(task);
        boost::system::error_code e;
    	task->timer->cancel(e);
    }
}

void CDNSResolver::onTimeout(const boost::system::error_code& e,
    task_type task)
{
    task_timer timer    = task->timer;
    node_ptr node       = task->node;
    
    if (task->dnsname.empty()) {
        timer->expires_from_now(boost::posix_time::seconds(60 * 60 * 24));
    	timer->async_wait(
            boost::bind(&CDNSResolver::onTimeout, 
            this,
    		boost::asio::placeholders::error,
    		task));
        return;
    }

    std::string ip = do_resolver(task->dnsname);

    {
        boost::mutex::scoped_lock lock(m_lock);
        
        if (m_alltasks.find(task) == m_alltasks.end()) {
            return;
        }
        
        key_type key = m_alltasks[task];
        m_alltasks.erase(task);
        m_taskmap.erase(key);
    }
    
    //printf("resolver task:%s!\n", node->key().c_str());
    LOG_PRINT(log_info, "resolver task:%s, ip:%s!", 
    	node->key().c_str(), ip.c_str());
	
    node->connect(ip);
}

std::string CDNSResolver::do_resolver(std::string dnsname)
{
    std::string ip("");

    typedef boost::asio::ip::tcp tns_tcp;

	try {
	    tns_tcp::resolver resolver(m_service);
	    tns_tcp::resolver::query query(dnsname, "0");
	    tns_tcp::resolver::iterator endpoint_iter = resolver.resolve(query);
	    tns_tcp::resolver::iterator end;  
	    tns_tcp::endpoint endpoint;

	    if (endpoint_iter != end)  {
	        endpoint = *endpoint_iter;
	        boost::asio::ip::address address = endpoint.address();
	        ip = address.to_string();
	    }
	} catch(std::exception& e) {
        LOG_PRINT(log_error, "resolve hostname(%s) catch error..", dnsname.c_str());
    }
    //printf("resolve ip:%s!\n", ip.c_str());
    LOG_PRINT(log_info, "resolve ip:%s!", ip.c_str());

    return ip;
}

