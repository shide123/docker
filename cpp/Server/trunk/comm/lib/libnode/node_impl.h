#ifndef _NODE_IMPL_H__
#define _NODE_IMPL_H__

#include "node.h"
#include "tcp_base.hpp"

class CDNSResolver;

class CNode
    : public INode
    , public CTcpBase
{
public:
    CNode(key_type key, svr_name_type & svr_name, service_type & service,
        IWatcher & watcher, CDNSResolver & resolver);
    
    virtual ~CNode(void);

    //INode
    virtual void start(const ip_type & ip, const port_type & port);
    virtual void stop(void);
    virtual void subscribe(std::string & svrlst);

    //CTcpBase
    virtual void start();
    virtual int  handleMessage(const char* pdata, int msglen);
    virtual void onConnect();
    virtual bool onTimeout();
    virtual void onClose();
    virtual bool onAlarm(size_t queuesize);

    void connect(ip_type ip);
    void connect_(ip_type ip);
    void reconnect();
private:
    void registersvr_v1();   
	void registersvr_v2();
	void registersvr_v3();

	void registersvr();
	
    void ping();
    int32 ip2i32(const ip_type & ip);
    ip_type i322ip(int32 ip);

private:
    bool m_register;
    CDNSResolver &  m_resolver;
    ip_type         m_nodeip;
    int32           m_nodeport;

	static int s_nId;
};

#endif
