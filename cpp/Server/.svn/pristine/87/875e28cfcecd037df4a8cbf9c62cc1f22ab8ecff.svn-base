#ifndef _NODE_H__
#define _NODE_H__

#include <boost/asio/io_service.hpp>
#include <string>

#include "yc_datatypes.h"

typedef std::string key_type;
class INode
{
public:
    typedef boost::asio::io_service service_type;
    typedef std::string             ip_type;
    typedef int                     port_type;
    typedef std::string             svr_name_type;

public:
    class IWatcher
    {
    public:
        enum en_watch_event
        {
            en_svr_add = 0,
            en_svr_rem,
            en_svr_full,
        };
        
    public:
        virtual ~IWatcher(void) {}

        virtual void onConnected(const key_type & key) = 0;

        virtual void onDisConnected(const key_type & key) = 0;

        virtual void onNodeEvent(const key_type & key, 
			INode::svr_name_type & svrname,
            int32 ip, int16 port, const std::string & desc,
            INode::IWatcher::en_watch_event event) = 0;
    };

public:
    static INode * CreateNode(key_type key, svr_name_type & svr_name, 
        service_type & service, IWatcher & watcher);
    
public:
    INode(key_type key, svr_name_type & svr_name, service_type & service,
        IWatcher & watcher) 
        : m_key(key)
        , m_svrname(svr_name)
        , m_service(service)
        , m_watcher(watcher)
        , m_hostip()
        , m_hostport(0)
    {}
    
    virtual ~INode(void) {}

    virtual void start(const ip_type & ip, const port_type & port) = 0;
    
    virtual void stop(void) = 0;

    virtual void subscribe(std::string & svrlst) = 0;

    const key_type & key() const { return m_key; }  

    void setHostInfo(const ip_type & ip, const port_type & port)
    {
        m_hostip    = ip;
        m_hostport  = port;
    }

	void set_desc(const std::string & desc) 
	{
		m_desc = desc;
	}

private:
    const key_type  m_key;
    
protected:
    svr_name_type   m_svrname;
    service_type &  m_service;
    IWatcher &      m_watcher;
    ip_type         m_hostip;
    port_type       m_hostport;
	std::string		m_desc;
};

#endif
