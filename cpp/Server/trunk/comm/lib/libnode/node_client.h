#ifndef _NODE_CLIENT_H__
#define _NODE_CLIENT_H__

#include <string>
#include <boost/asio/io_service.hpp>

class INodeNotify
{
public:
    enum en_node_event
    {
        en_svr_add  = 0,
        en_svr_rem,
    };
        
    virtual ~INodeNotify(void) {};

	/*
	 * callback define. call by nodeclient.
	 *
	 * @svrname		subscribe service name.
	 * @ip			server ip.
	 * @port		server port which server bind.
	 * @event		add or remove
	 * @desc		description.
	 */
    virtual void onNodeEvent(
		const std::string & svrname, 
		const std::string & ip, 
        int port, 
        en_node_event event,
        std::string desc = "") = 0;
};

class INodeClient
{
public:
    typedef boost::asio::io_service service_type;
    typedef std::string             ip_type;
    typedef int                     port_type;
    typedef std::string             svr_name_type;

public:
    static INodeClient * CreateObject(const char * svr, 
		boost::asio::io_service & service,
        INodeNotify * notify, const char * ip, int port, 
        std::string desc = "");

public:
    INodeClient(svr_name_type & svr, service_type & service,
        INodeNotify * notify, const ip_type & ip, const port_type & port,
        std::string & desc) 
        : m_svr_name(svr)
        , m_service(service) 
        , m_notify(notify)
        , m_ip(ip)
        , m_port(port)
        , m_desc(desc)
    {}
    
    virtual ~INodeClient(void) {}

	/*
	 * set node server addresses!
	 * address format: ip1:port1[;ip2:port2]
	 *
	 * @addresses	node server addresses.
	 * 
	 * @return		if has one valid ip&port, return true.
	 * 				otherwise return false.
	 */
	virtual bool set_address(const char * addresses) = 0;

	/*
	 * begin connect node server.
	 */
	virtual void connect() = 0; 

	/*
	 * disconnect node.
	 */
	virtual void disconnect() = 0;

	/*
	 * set alarm parameters.
	 * @ip		alarm server ip
	 * @port	alarm server bind port.
	 * @intval	alarm interval. avoid too frequence
	 */
	virtual void set_alarm_param(const char * ip, int port, int intval) = 0;

	/*
	 * subscribe server info from node server.
	 * format: "name1,name2[,name3]", such as "logonsvr,roomsvr"
	 * @svr		svrname subscribe.
	 */
    virtual void subscribe(const char * svr) = 0;

protected:
    svr_name_type           m_svr_name;
    service_type &          m_service;
    INodeNotify *           m_notify;
    ip_type                 m_ip;
    port_type               m_port;
	std::string				m_desc;
};

#endif//_NODE_CLIENT_H__