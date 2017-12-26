#ifndef _DNS_RESOLVER_H_
#define _DNS_RESOLVER_H_

#include <map>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "node_impl.h"

class CDNSResolver
{
private:
    typedef boost::asio::deadline_timer     boost_timer;
    typedef boost::shared_ptr<boost_timer>  task_timer;
    typedef std::string                     dns_type;
    typedef boost::shared_ptr<CNode>        node_ptr;
    typedef boost::asio::io_service         task_service;
    typedef boost::mutex                    task_lock;
    typedef boost::thread                   task_thread;
    
    struct STask
    {
        dns_type    dnsname;
        node_ptr    node;
        task_timer  timer;
    };

    typedef boost::shared_ptr<STask>        task_type;
    typedef std::map<key_type, task_type>   task_map;
    typedef std::map<task_type, key_type>   task_key_map;
    
public:
    CDNSResolver();
    
    ~CDNSResolver();

    void addTask(key_type key, std::string dnsname, node_ptr node);

    void cancelTask(key_type key);

    void onTimeout(const boost::system::error_code& e,
        task_type task);

private:
    std::string do_resolver(std::string dnsname);
    
private:
    task_service    m_service;
    task_thread *   m_pthread;
    task_lock       m_lock;
    task_map        m_taskmap;
    task_type       m_def_task;
    task_key_map    m_alltasks;
};

#endif
