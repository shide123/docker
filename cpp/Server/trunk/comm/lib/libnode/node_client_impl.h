#ifndef _NODE_CLIENT_IMPL_H__
#define _NODE_CLIENT_IMPL_H__

#include "node_client.h"
#include "node_impl.h"

#include <boost/asio/deadline_timer.hpp>
#include <map>
#include <set>

class IAlarm;
class CDNSResolver;

class CNodeClient 
    : public INodeClient
    , public INode::IWatcher
{
public:
    typedef boost::asio::deadline_timer     timer_type;
    typedef boost::shared_ptr<CNode>        node_ptr;
    typedef std::map<uint64, node_ptr>      u64_node_map;

    typedef std::map<uint64, uint64>        u64_u64_map;
    typedef std::map<uint64, uint32>        u64_u32_map;
    typedef std::map<uint64, u64_u32_map>   u64_u64_u32_map;
    typedef std::map<uint64, svr_name_type> u64_svr_map;
    typedef std::set<std::string>           string_set;
    typedef std::vector<std::string>        string_vector;

    typedef std::string                     key_type;
    typedef std::map<key_type, node_ptr>    key_node_map;
    typedef std::map<key_type, uint32>      key_u32_map;
    typedef std::map<key_type, u64_u32_map> key_u64_u32_map;
	typedef std::map<uint64, std::string> 	u64_str_map;
	typedef std::map<key_type, u64_str_map>	key_u64_str_map;
	typedef std::map<key_type, IAlarm *>	key_alarm_map;
    
public:
    CNodeClient(svr_name_type & svr, service_type & service,
        INodeNotify * notify, const ip_type & ip, const port_type & port,
        std::string & desc);
    
    virtual ~CNodeClient(void);

	//INodeClient
	virtual bool set_address(const char * addresses);
	virtual void connect(); 
	virtual void disconnect();
	virtual void set_alarm_param(const char * ip, int port, int intval);
    virtual void subscribe(const char * svr);

    //INode::IWatcher
    virtual void onConnected(const key_type & key);
    virtual void onDisConnected(const key_type & key);
    virtual void onNodeEvent(const key_type & key, INode::svr_name_type & svrname,
            int32 ip, int16 port, const std::string & desc,
            INode::IWatcher::en_watch_event event);

    void    add_node(const ip_type & ip, port_type & port);
	void 	connect_op(bool enable);
	void	connect_node();
	void 	clear_node();
    void    subscribe_(const svr_name_type & svr);
	void 	set_alarm_param_(std::string ip, int port, int intval);
    
private:
    uint64  getKey(const ip_type & ip, const port_type & port);
    uint64  getKey(const int32 & ip, const port_type & port);
    int32   getip(uint64 key);
    int32   getport(uint64 key);
    
    uint32  ip2u32(const ip_type & ip);
    ip_type u322ip(const uint32 ip);

    void SplitAndTrim(const std::string & str, char delim, 
        string_vector & vec);

    std::string getSubscribeSvrs();

    void    notifyIfNeed(uint64 skey);
    
    void    onTimeout(const boost::system::error_code& e);

    void    clearNode(const key_type & nkey, bool notify);

    key_type    mknode_key(const ip_type & ip, port_type & port);

	void alarm(const key_type & key);
private:
    enum {
        en_invalid  = 0,
        en_valid    = 1,
    };

    struct STimerInfo
    {
        time_t  last_subscribe;
        time_t  last_check_valid;
    };
private:
    timer_type      m_timer;
    key_u32_map     m_valid_nodes;  //connected node.
    key_node_map    m_nodes_map;    //node key -> node
    key_u64_str_map	m_desc_map;
    bool			m_enable;
    
    key_u64_u32_map m_node2svrmap;  //node key -> svr set.
    u64_u32_map     m_id2cntmap;    //svr key  -> svr cnt.
    u64_svr_map     m_id2svrmap;    //svr key  -> svr name
    string_set      m_subscribes;
    u64_u32_map     m_notifyout;
    STimerInfo      m_timerinfo;
    CDNSResolver*   m_resolver;
	key_alarm_map	m_alarm_map;
	
	ip_type					m_alarm_ip;
	port_type				m_alarm_port;
	int						m_alarm_intval;
};

#endif//_NODE_CLIENT_IMPL_H__
