#include "node_client_impl.h"
#include "CLogThread.h"
#include "dns_resolver.h"
#include "alarm.h"

#include <sys/types.h>
#include <ifaddrs.h>

//default alarm server: 127.16.41.96
#define ALARM_DEFAULT_IP	\
	((127 << 0) | (16 << 8) | (41 << 16) | (96 << 24))
#define ALARM_DEFAULT_PORT	7601	
#define ALARM_DEFAULT_INTL	60
#define LOCAL_IP			("192.168.")

#define SAFE_DELETE(ptr)    \
    do {                    \
        if (ptr != NULL) {  \
            delete ptr;     \
            ptr = NULL;     \
        }                   \
    } while (0)
    

CNodeClient::CNodeClient(svr_name_type & svr, service_type & service,
        INodeNotify * notify, const ip_type & ip, const port_type & port,
        std::string & desc)
    : INodeClient(svr, service, notify, ip, port, desc)
    , m_timer(service)
    , m_valid_nodes()
    , m_nodes_map()
    , m_desc_map()
    , m_enable(true)
    , m_node2svrmap()
    , m_id2cntmap()
    , m_id2svrmap()
    , m_subscribes()
    , m_notifyout()
    , m_timerinfo()
    , m_resolver(new CDNSResolver())
    , m_alarm_map()
    , m_alarm_ip(u322ip(ALARM_DEFAULT_IP))
    , m_alarm_port(ALARM_DEFAULT_PORT)
    , m_alarm_intval(ALARM_DEFAULT_INTL)
{
	struct ifaddrs * addrs = NULL, * faddr = NULL;
	void * ptr = NULL;

	std::string ips = "";
	if (getifaddrs(&addrs) == 0) {
		faddr = addrs;
		while (addrs != NULL) {
			if (addrs->ifa_addr->sa_family == AF_INET) {
				if (strcmp(addrs->ifa_name, "lo")) {
					ptr = &((struct sockaddr_in *)addrs->ifa_addr)->sin_addr;
					char addressBuffer[INET_ADDRSTRLEN];
					inet_ntop(AF_INET, ptr, addressBuffer, INET_ADDRSTRLEN);

					if (strncmp(addressBuffer, LOCAL_IP, strlen(LOCAL_IP)) != 0) {
						ips.append(addressBuffer).append(",");
					}
				}
			}
			addrs = addrs->ifa_next;
		}

		if (faddr != NULL) {
			freeifaddrs(faddr);
		}
	}
	printf("host ip:%s, alarmip:%s\n", ips.c_str(), m_alarm_ip.c_str());
	LOG_PRINT(log_info, "host ip:%s", ips.c_str());
	
	if (ips.size() > 0) {
		m_ip	= ips;
	}

	m_timer.expires_from_now(boost::posix_time::seconds(5));
	m_timer.async_wait(
        boost::bind(&CNodeClient::onTimeout, 
        this,
		boost::asio::placeholders::error));
}

CNodeClient::~CNodeClient(void)
{   
    boost::system::error_code e;
	m_timer.cancel(e);
    
    for (key_node_map::iterator it = m_nodes_map.begin(); 
		it != m_nodes_map.end(); ++it) {
		node_ptr node = it->second;
		if (node.get() != NULL) {
	        it->second->stop();
	        it->second.reset();
		}
    }
    
    m_valid_nodes.clear();
    m_nodes_map.clear();
    m_node2svrmap.clear();
    m_id2cntmap.clear();
    m_id2svrmap.clear();
    m_subscribes.clear();
    m_notifyout.clear();

	SAFE_DELETE(m_resolver);

	key_alarm_map::iterator it = m_alarm_map.begin();
	while (it != m_alarm_map.end()) {
		SAFE_DELETE(it->second);
		++ it;
	}
	m_alarm_map.clear();
}

bool CNodeClient::set_address(const char * addresses)
{
	if (addresses == NULL) {
		return false;
	}
	
	std::vector<std::string> addrs;
	std::string straddrs = addresses;
	LOG_PRINT(log_info, "node addr:%s", straddrs.c_str());
	printf("node addr:%s\n", addresses);
	
	if (straddrs.empty()) {
		return false;
	}
	
	SplitAndTrim(straddrs.c_str(), ';', addrs);
	bool ret = false;

	for (uint32 i = 0; i < addrs.size(); i++) {
		printf("addr:%s\n", addrs[i].c_str());
		
		if (addrs[i].size() > 0) {
			std::vector<std::string> addr; 
			SplitAndTrim(addrs[i].c_str(), ':', addr);

			if (addr.size() == 2) {
				std::string nodeip(addr[0]);
				int32		nodeport = atoi(addr[1].c_str());

				if (nodeport > 0) {
					m_service.post(
				        boost::bind(
				            &CNodeClient::add_node, 
				            this,
				            nodeip,
				            nodeport
				        )
				    );
					
					ret = true;
				}
			}
		}
	}
	return ret;
}

void CNodeClient::add_node(const ip_type & ip, port_type & port)
{
	key_type key = mknode_key(ip, port);
    m_nodes_map[key];	//add empty ptr.
}

void CNodeClient::connect()
{
	LOG_PRINT(log_info, "node client, connect!");
	m_service.post(
        boost::bind(
            &CNodeClient::connect_op, 
            this,
            true
        )
    );
} 

void CNodeClient::disconnect()
{
	LOG_PRINT(log_info, "node client, disconnect!");
	m_service.post(
        boost::bind(
            &CNodeClient::connect_op, 
            this,
            false
        )
    );
}

void CNodeClient::connect_op(bool enable)
{
	m_enable = enable;

	if (m_enable) {
		connect_node();
	} else {
		clear_node();
	}
}

void CNodeClient::connect_node()
{
	key_node_map::iterator it = m_nodes_map.begin();
	for ( ; it != m_nodes_map.end(); ++ it) {
		node_ptr node 	= it->second;
		key_type key 	= it->first;

		if (node.get() != NULL) {
			LOG_PRINT(log_warning, "key:%s already create???", key.c_str());
			continue;
		}

		std::vector<std::string> vec;
		SplitAndTrim(key.c_str(), ':', vec);
		if (vec.size() != 2) {	//should not occured!
			LOG_PRINT(log_warning, "key:%s invald???", key.c_str());
			continue;
		}

		node_ptr pnode(new CNode(key, m_svr_name, m_service, *this, *m_resolver));
	    pnode->setHostInfo(m_ip, m_port);
		pnode->set_desc(m_desc);
	    
	    m_nodes_map[key]  = pnode;
	    pnode->start(vec[0], atoi(vec[1].c_str()));

		it->second = pnode;
	}
}

void CNodeClient::clear_node()
{
	key_node_map::iterator it;
    for (it = m_nodes_map.begin(); it != m_nodes_map.end(); ++it) {
        it->second->stop();
        it->second.reset();
    }
}

void CNodeClient::set_alarm_param(const char * ip, int port, int intval)
{
	if (ip == NULL || strlen(ip) == 0 || port == 0) {
		return;
	}
	
	LOG_PRINT(log_info, "alarm info, ip:%s, port:%d, intval:%d",
		ip, port, intval);
	std::string ip_(ip);
	
	m_service.post(
        boost::bind(
            &CNodeClient::set_alarm_param_, 
            this,
            ip_,
            port,
            intval
        )
    );
}

void CNodeClient::set_alarm_param_(std::string ip, int port, int intval)
{
	if (ip == m_alarm_ip && port == m_alarm_port && intval == m_alarm_intval) {
		return;
	}

	key_alarm_map::iterator it = m_alarm_map.begin();
	while (it != m_alarm_map.end()) {
		SAFE_DELETE(it->second);
		++ it;
	}
	m_alarm_map.clear();

	m_alarm_ip		= ip;
	m_alarm_port	= port;
	m_alarm_intval	= intval;
}
	
void CNodeClient::subscribe(const char * svr)
{
	if (svr == NULL || strlen(svr) == 0) {
		return;
	}
	
    m_service.post(
        boost::bind(
            &CNodeClient::subscribe_, 
            this,
            svr
        )
    );
}

void CNodeClient::subscribe_(const svr_name_type & svr)
{
    if (svr.empty()) {
        return;
    }
    
    string_vector svrs;
    SplitAndTrim(svr, ',', svrs);

    for (uint32 i = 0; i < svrs.size(); i++) {
        if (!svrs[i].empty()) {
            m_subscribes.insert(svrs[i]);
        }
    }
}

//INode::IWatcher
void CNodeClient::onConnected(const key_type & key)
{
    printf("nodeclient[%s], connect!\n", key.c_str());
    LOG_PRINT(log_info, "nodeclient[%s] connect!", key.c_str());
    m_valid_nodes[key] = en_valid;
    
    std::string svrs = getSubscribeSvrs();
    if (svrs.empty()) {
        return;
    }
    //printf("subscribe, %s!\n", svrs.c_str());

    node_ptr pnode = m_nodes_map[key];
    if (pnode) {
        pnode->subscribe(svrs);
    }
}

void CNodeClient::onDisConnected(const key_type & nkey)
{
    //printf("nodeclient[%s], disconnect!\n", nkey.c_str());
    LOG_PRINT(log_info, "nodeclient[%s] disconnect!", nkey.c_str());
    //clearNode(nkey, true);
	alarm(nkey);
}

void CNodeClient::onNodeEvent(const key_type & nkey, 
		INode::svr_name_type & svrname,
        int32 ip, int16 port, const std::string & desc,
        INode::IWatcher::en_watch_event event)
{
	if (ip == 0 || port == 0) {
		return;
	}
	
    uint64 skey = getKey(ip, port);

    printf("node event[%s], svrname:%s, ip:%s, port:%d, event:%d, desc:%s\n",
        nkey.c_str(), svrname.c_str(), u322ip(ip).c_str(), 
        getport(skey), event, desc.c_str());
//    LOG_PRINT(log_debug, "node event[%s], svrname:%s, ip:%s, port:%d, event:%d",
//        nkey.c_str(), svrname.c_str(), u322ip(ip).c_str(), getport(skey), event);

    switch (event) {
        case INode::IWatcher::en_svr_add:
            if (m_node2svrmap[nkey][skey] == en_invalid) {
                m_node2svrmap[nkey][skey] = en_valid;
                m_id2cntmap[skey] ++;
            }
            m_id2svrmap[skey] 		= svrname;
			m_desc_map[nkey][skey]	= desc;
            break;
        case INode::IWatcher::en_svr_rem:
            if (m_node2svrmap[nkey][skey] == en_valid) {
                m_node2svrmap[nkey][skey] = en_invalid;
                if (m_id2cntmap[skey] > 0) {
                    m_id2cntmap[skey] --;
                }
				m_desc_map[nkey][skey].clear();
            }
            break;
        case INode::IWatcher::en_svr_full:
            clearNode(nkey, false);
            return;
        default:
            break;
    }

    notifyIfNeed(skey);
}
    
uint64  CNodeClient::getKey(const ip_type & ip, const port_type & port)
{
    uint32 uip = ip2u32(ip);
    uint64 key = uip & 0xFFFFFFFF;
    key |= ((((uint64)port) & 0xFFFF) << 32);

    return key;
}

uint64  CNodeClient::getKey(const int32 & ip, const port_type & port)
{
    uint64 key = ip & 0xFFFFFFFF;
    key |= ((((uint64)port) & 0xFFFF) << 32);

    return key;
}

int32   CNodeClient::getip(uint64 key)
{
    return (key & 0xFFFFFFFF);
}

int32   CNodeClient::getport(uint64 key)
{
    return (key >> 32) & 0xFFFF;
}

uint32  CNodeClient::ip2u32(const ip_type & ip)
{
    return inet_addr(ip.c_str());
}

CNodeClient::ip_type CNodeClient::u322ip(const uint32 ip)
{
    if(ip == 0) {
        return "";
   }
    
   struct in_addr addr;
   memcpy(&addr, &ip, 4);
   return std::string(::inet_ntoa(addr));
}

void CNodeClient::SplitAndTrim(const std::string & str, char delim, 
    string_vector & vec)
{
    std::string::size_type begin = 0;
    std::string::size_type pos   = 0;

    while (begin < str.size() && str[begin] == delim && ++ begin);

    while ((pos = str.find(delim, begin)) != std::string::npos) {
        vec.push_back(str.substr(begin, pos - begin));
        begin = pos + 1;

        while (begin < str.size() && str[begin] == delim && ++ begin);
    }
    if (begin < str.size()) {
        pos = str.size();
        vec.push_back(str.substr(begin, pos - begin));
    }
}

std::string CNodeClient::getSubscribeSvrs()
{
    if (m_subscribes.empty()) {
        return std::string();
    }

    std::string svrs("");
    
    for (string_set::iterator it = m_subscribes.begin();
        it != m_subscribes.end(); ++ it) {
        svrs.append(it->c_str()).append(",");
    }

    //printf("get subscribe:%s\n", svrs.c_str());

    return svrs;
}

void    CNodeClient::notifyIfNeed(uint64 skey)
{
	if (m_notify == NULL) {
		return;
	}
	
    int nodecnt     = m_valid_nodes.size();
    int threshold   = (nodecnt >> 1) + 1;

    int32   iip     = getip(skey);
    int32   iport   = getport(skey);
    int32   icnt    = m_id2cntmap[skey];

    if (icnt >= threshold) {
		std::string desc;
		bool first 		= true;
		//bool notdesc 	= true;
		key_type	desc_key;

		key_u32_map::iterator it = m_valid_nodes.begin();
		while (it != m_valid_nodes.end()) {
			const key_type & nkey = it->first;

			if (first) {
				desc = m_desc_map[nkey][skey];
				desc_key	= nkey;
				first = false;
//				LOG_PRINT(log_debug, "desc use. nkey:%s, skey:[%s:%d]", 
//					nkey.c_str(), u322ip(getip(skey)).c_str(), getport(skey));
			}

			const std::string & _desc_ = m_desc_map[nkey][skey];
			if (_desc_ != desc) {
				LOG_PRINT(log_error, "desc error. (%s) not equal nkey:%s, skey:[%s:%d]", 
					desc_key.c_str(), nkey.c_str(), u322ip(getip(skey)).c_str(), 
					getport(skey));
			}
			
			++ it;
		}
		
        if (m_notifyout[skey] == en_invalid) {
			LOG_PRINT(log_debug, "notify event, svrname:%s, ip:%s, port:%d, event:%d", m_id2svrmap[skey].c_str(), u322ip(iip).c_str(), iport, INodeNotify::en_svr_add);
            m_notifyout[skey] = en_valid;
            m_notify->onNodeEvent(m_id2svrmap[skey], u322ip(iip), iport, 
                INodeNotify::en_svr_add);
        }
    } else {
        if (m_notifyout[skey] == en_valid) {
			LOG_PRINT(log_debug, "notify event, svrname:%s, ip:%s, port:%d, event:%d", m_id2svrmap[skey].c_str(), u322ip(iip).c_str(), iport, INodeNotify::en_svr_rem);
            m_notifyout.erase(skey);
            m_notify->onNodeEvent(m_id2svrmap[skey], u322ip(iip), iport, 
                INodeNotify::en_svr_rem);
        }
    }
}

void    CNodeClient::onTimeout(const boost::system::error_code& e)
{
    if (!e) {

		do {
		    time_t now = time(NULL);

		    if (now - m_timerinfo.last_check_valid > 10) {
		        m_timerinfo.last_check_valid    = now;
		        u64_u32_map notifyout = m_notifyout;

		        u64_u32_map::iterator it;
		        for (it = notifyout.begin(); it != notifyout.end(); ++ it) {
		            notifyIfNeed(it->first);
		        }
		    } 

			if (!m_enable) {
				break;
			}

		    if (now - m_timerinfo.last_subscribe > 60) {
		        std::string subscribesvrs = getSubscribeSvrs();
		        //printf("ontimeout, pull all! [%s]\n", subscribesvrs.c_str());

		        key_u32_map::iterator it = m_valid_nodes.begin();
				while (it != m_valid_nodes.end()) {
					key_type key = it->first;
					
					node_ptr node = m_nodes_map[key];
					if (node.get() != NULL) {
						node->subscribe(subscribesvrs);
					}
					
					++ it;
				}
		        m_timerinfo.last_subscribe      = now;
		    }
		} while (0);
        
        m_timer.expires_from_now(boost::posix_time::seconds(5));
	    m_timer.async_wait(
            boost::bind(
                &CNodeClient::onTimeout, 
                this,
    		    boost::asio::placeholders::error));
    }
}

void    CNodeClient::clearNode(const key_type & nkey, bool notify)
{
    u64_u32_map & svrflagmap = m_node2svrmap[nkey];

    if (notify) {
        m_valid_nodes.erase(nkey);
    }

    if (svrflagmap.size() > 0) {
        for (u64_u32_map::iterator it = svrflagmap.begin(); 
            it != svrflagmap.end(); ++ it) {
            if (it->second == en_valid) {
                uint64 skey = it->first;
                m_id2cntmap[skey] --;

                if (notify) {
                    notifyIfNeed(skey);
                }
            }
        }
        svrflagmap.clear();
		m_desc_map[nkey].clear();
    }
}

key_type    CNodeClient::mknode_key(const ip_type & ip, port_type & port)
{
    char buf[100];
    snprintf(buf, 99, "%s:%d", ip.c_str(), port);
    return buf;
}

void CNodeClient::alarm(const key_type & key)
{
	IAlarm * palarm = NULL;
	
	key_alarm_map::iterator it = m_alarm_map.find(key);
	if (it == m_alarm_map.end()) {
		palarm = IAlarm::Create(m_alarm_ip, m_alarm_port, m_alarm_intval, 
			IAlarm::en_conn_tcp);
		palarm->setinfo("nodemgrsvr", "nodemgr", "nodemgrsvr alarm");
		m_alarm_map[key] = palarm;
	} else {
		palarm = it->second;
	}

	if (palarm != NULL) {
		palarm->send("disconnect to node[%s]!", key.c_str());
	}
}
