#include "node_impl.h"

#include "message_helper.h"
#include "cmd_nodemgr.h"
#include "dns_resolver.h"
#include "CLogThread.h"

int CNode::s_nId = 0;

CNode::CNode(key_type key, svr_name_type & svr_name, service_type & service,
        IWatcher & watcher, CDNSResolver & resolver)
    : INode(key, svr_name, service, watcher)
    , CTcpBase(service, ++ s_nId)
    , m_register(false)
    , m_resolver(resolver)
{
}

CNode::~CNode(void)
{
    stop();
}

//INode
void CNode::start(const ip_type & ip, const port_type & port)
{
    start();
    m_nodeport  = port;
    m_nodeip    = ip;

    reconnect();
    //m_resolver.addTask(key(), m_nodeip, get_ptr<CNode>());
    //CTcpBase::connect(ip.c_str(), port);
}

void CNode::connect(ip_type ip)
{
    if (ip.empty()) {
        return;
    }

    m_service.post(
        boost::bind(
            &CNode::connect_, 
            get_ptr<CNode>(),
            ip
        )
    );
}

void CNode::connect_(ip_type ip)
{
	LOG_PRINT(log_debug, "connect, status:%d!", m_nStatus);
	
    if (m_nStatus == en_status_transfer) {
		LOG_PRINT(log_info, "connect, but status is transfer!");
        return;
    }

    doClose();
    CTcpBase::connect(ip.c_str(), m_nodeport);
}

void CNode::reconnect()
{
    m_bStop     	= false;
	m_bNeedClose	= false;
	LOG_PRINT(log_info, "[%s], reconnect!", key().c_str());
    m_resolver.addTask(key(), m_nodeip, get_ptr<CNode>());
}

void CNode::stop(void)
{
	CTcpBase::doClose();
    CTcpBase::stop();
}

void CNode::subscribe(std::string & svrlst)
{
    if (svrlst.empty()) {
        return;
    }

    char buf[4096];
    
    CMDSubscribeServer_t * req = GetAndFillHeader<CMDSubscribeServer_t>(buf, 
        MDM_Version_Value, 
        0, 
        MDM_Vchat_Nodemgr, 
        Sub_NodeMgr_SubscribeServerReq);

    int32 cpylen = std::min((int32)SERVER_NAME_LEN, (int32)m_svrname.size());
    strncpy(req->servername, m_svrname.c_str(), cpylen);
    req->servername[cpylen] = '\0';

    req->servernamelen  = svrlst.size() + 1;
    strncpy(req->servernames, svrlst.c_str(), 
        svrlst.size());
    req->servernames[svrlst.size()] = '\0';

    COM_MSG_HEADER * header = GetObject<COM_MSG_HEADER>(buf);
    header->length  = GetMessageLength(buf) + svrlst.size() + 1;

    writeMessage(buf, GetMessageLength(buf));
}

//CTcpBase

void CNode::start()
{
    CTcpBase::start(10, false);
}

int  CNode::handleMessage(const char* pdata, int msglen)
{
    if (pdata == NULL || msglen == 0) {
        return 0;
    }

    //printf("msg len:%d\n", msglen);
    if (msglen < (int)SIZE_IVM_HEADER) {
        return -1;
    }

    int maincmd = GetMainCmd(pdata);
    int subcmd  = GetSubCmd(pdata);

    //printf("maincmd:%d, subcmd:%d\n", maincmd, subcmd);
    if (maincmd != MDM_Vchat_Nodemgr) {
        return 0;
    }

    m_uLastActiveTime   = time(NULL);

    if (!m_register && subcmd == Sub_NodeMgr_RegisterServerResp) {
        CMDRegisterServerResp_t * resp = 
            GetSubCmdObject<CMDRegisterServerResp_t>(pdata);
        if (resp->res == 0 && m_hostport == resp->port) {
            m_register  = true;
            m_watcher.onConnected(key());
            return 0;
        }
    }

    if (!m_register) {
        return 0;
    }

    switch (subcmd) {
        case Sub_NodeMgr_SubscribeServerResp: {
            CMDSubscribeServerResp_t * resp 
                = GetSubCmdObject<CMDSubscribeServerResp_t>(pdata);

            CMDSubscribeServerInfo_t * svr = NULL;

            {
                std::string svrname("fullpull");
				std::string desc;
                int32 ip    = 0;
                int16 port  = 0;
				
                m_watcher.onNodeEvent(key(), svrname, ip, port, desc,
                    IWatcher::en_svr_full);
            }
                
            for (uint32 i = 0; i < resp->num; i++) {
                svr = (CMDSubscribeServerInfo_t *)
                    (resp->content + i * sizeof(CMDSubscribeServerInfo_t));

                svr_name_type   svrname(svr->servername);
				std::string		desc("");
                
                m_watcher.onNodeEvent(key(), svrname, svr->ip, svr->port, desc,
                     IWatcher::en_svr_add);
            }
            break;
        }
		case Sub_NodeMgr_SubscribeServerV2Resp: {
            CMDSubscribeServerResp_t * resp 
                = GetSubCmdObject<CMDSubscribeServerResp_t>(pdata);

			LOG_PRINT(log_info, "receive length:%d", msglen);
			
            CMDSubscribeServerInfoV2_t * svr = NULL;

            {
                std::string svrname("fullpullv2");
				std::string desc;
                int32 ip    = 0;
                int16 port  = 0;
				
                m_watcher.onNodeEvent(key(), svrname, ip, port, desc,
                    IWatcher::en_svr_full);
            }

			char * baseaddr = resp->content;
			int offset 		= 0;
			
            for (uint32 i = 0; i < resp->num; i++) {
                svr = (CMDSubscribeServerInfoV2_t *)
                    (baseaddr + offset);

                svr_name_type   svrname(svr->servername);
				std::string		desc(GET_DESC_DATA(svr), GET_DESC_LEN(svr));
				offset += sizeof(CMDSubscribeServerInfoV2_t) + desc.size();
                
                m_watcher.onNodeEvent(key(), svrname, svr->ip, svr->port, desc,
                     IWatcher::en_svr_add);
            }
            break;
        }
		case Sub_NodeMgr_SubscribeServerAdd: {
			CMDNoticeServerAdd_t * svr 
                = GetSubCmdObject<CMDNoticeServerAdd_t>(pdata);

            svr_name_type   svrname(svr->servername);
			std::string		desc("");
            
            m_watcher.onNodeEvent(key(), svrname, svr->ip, svr->port, desc,
                     IWatcher::en_svr_add);
            break;
		}
        case Sub_NodeMgr_SubscribeServerAddV2: {
            CMDNoticeServerAddV2_t * svr 
                = GetSubCmdObject<CMDNoticeServerAddV2_t>(pdata);

            svr_name_type   svrname(svr->servername);
			std::string		desc(GET_DESC_DATA(svr), GET_DESC_LEN(svr));
            
            m_watcher.onNodeEvent(key(), svrname, svr->ip, svr->port, desc,
                     IWatcher::en_svr_add);
            break;
        }
        case Sub_NodeMgr_SubscribeServerRemove: {
            CMDNoticeServerRemove_t * svr 
                = GetSubCmdObject<CMDNoticeServerRemove_t>(pdata);

            svr_name_type   svrname(svr->servername);
			std::string		desc("");
            
            m_watcher.onNodeEvent(key(), svrname, svr->ip, svr->port, desc,
                     IWatcher::en_svr_rem);
            break;
        }
        case Sub_NodeMgr_Echo: {
            writeMessage(pdata, msglen);
            break;
        }
        default:
            break;
    }
    return 0;
}

void CNode::onConnect()
{
    registersvr();
}

bool CNode::onTimeout()
{
    time_t now  = time(NULL);
    //printf("timeout, close, now:%d, last:%d\n", now, m_uLastActiveTime);

    if (now - m_uLastActiveTime > 30 || (m_nStatus != en_status_transfer)) {
        doClose();
        reconnect();
    } else if (m_nStatus == en_status_transfer) {
    	m_register ? ping() : registersvr();
    }

    return true;
}

void CNode::onClose()
{
    m_register = false;
	if (!getRemoteIp().empty()) {
    	m_watcher.onDisConnected(key());
	}
    m_resolver.cancelTask(key());
}

bool CNode::onAlarm(size_t queuesize)
{
    return true;
}

void CNode::registersvr()
{
	//registersvr_v1();
	if (m_desc.empty()) {
		registersvr_v2();
	} else {
		registersvr_v3();
	}
}

void CNode::registersvr_v1()
{
	char buf[SIZE_IVM_HEADER + sizeof(CMDRegisterServer_t)];
    CMDRegisterServer_t * req = 
        GetAndFillHeader<CMDRegisterServer_t>(buf, 
        MDM_Version_Value, 0, 
        MDM_Vchat_Nodemgr, 
        Sub_NodeMgr_RegisterServerReq);

    int32 cpylen = std::min((int32)SERVER_NAME_LEN, (int32)m_svrname.size());
    strncpy(req->servername, m_svrname.c_str(), cpylen);
    req->servername[cpylen] = '\0';
    
    if (!m_hostip.empty()) {
        req->ip     = ip2i32(m_hostip);
        if (req->ip == (uint32)(-1)) {
            req->ip = 0;
        }
    }
    
    req->port       = m_hostport;
    printf("register, port:%d, length:%d\n", req->port, GetMessageLength(buf));

    writeMessage(buf, GetMessageLength(buf));
}

void CNode::registersvr_v2()
{
    char buf[SIZE_IVM_HEADER + sizeof(CMDMultiCardRegisterServer_t)];
    CMDMultiCardRegisterServer_t * req = 
        GetAndFillHeader<CMDMultiCardRegisterServer_t>(buf, 
        MDM_Version_Value, 0, 
        MDM_Vchat_Nodemgr, 
        Sub_NodeMgr_MultiCardRegisterServerReq);

    int32 cpylen = std::min((int32)SERVER_NAME_LEN, (int32)m_svrname.size());
    strncpy(req->servername, m_svrname.c_str(), cpylen);
    req->servername[cpylen] = '\0';

	memset(req->ip, 0, sizeof(req->ip));
    if (!m_hostip.empty()) {
		int32 cpylen = std::min((int32)128 - 1, (int32)m_hostip.size());
		memcpy(req->ip, m_hostip.c_str(), cpylen);
    }
    
    req->port       = m_hostport;
    printf("register, port:%d, length:%d\n", req->port, GetMessageLength(buf));
	LOG_PRINT(log_debug, "[%s] register, port:%d, length:%d", 
		key().c_str(), req->port, GetMessageLength(buf));

    writeMessage(buf, GetMessageLength(buf));

}


void CNode::registersvr_v3()
{
	int len		= SIZE_IVM_HEADER + sizeof(CMDMultiCardRegisterServerV2_t);
	char * buf 	= NULL;
	len 	   += m_desc.size();
	buf			= (char *)malloc(len);
	
    CMDMultiCardRegisterServerV2_t * req = 
        GetAndFillHeader<CMDMultiCardRegisterServerV2_t>(buf, 
        MDM_Version_Value, 0, 
        MDM_Vchat_Nodemgr, 
        Sub_NodeMgr_MultiCardRegisterServerV2Req);

    int32 cpylen = std::min((int32)SERVER_NAME_LEN, (int32)m_svrname.size());
    strncpy(req->servername, m_svrname.c_str(), cpylen);
    req->servername[cpylen] = '\0';

	memset(req->ip, 0, sizeof(req->ip));
    if (!m_hostip.empty()) {
		int32 cpylen = std::min((int32)128 - 1, (int32)m_hostip.size());
		memcpy(req->ip, m_hostip.c_str(), cpylen);
    }
    
    req->port       = m_hostport;
	
	SET_DESC_LEN(req, m_desc.size());
	SET_DESC_DATA(req, m_desc.data(), m_desc.size());
	printf("[%s] desc:%s\n", key().c_str(), m_desc.c_str());

	COM_MSG_HEADER * header = GetObject<COM_MSG_HEADER>(buf);
	header->length	= len;
	
    printf("register v3, port:%d, length:%d\n", req->port, GetMessageLength(buf));
	LOG_PRINT(log_debug, "[%s] register, port:%d, length:%d, desc:%d", 
		key().c_str(), req->port, GetMessageLength(buf), m_desc.size());

    writeMessage(buf, len);
}

void CNode::ping()
{
//	LOG_PRINT(log_debug, "[%s]->ping!", key().c_str());
	
    char buf[SIZE_IVM_HEADER];
    GetAndFillHeader(buf, 
        MDM_Version_Value, 0, 
        MDM_Vchat_Nodemgr, Sub_NodeMgr_Ping);

    writeMessage(buf, GetMessageLength(buf));
}

int32 CNode::ip2i32(const ip_type & ip)
{
    return inet_addr(ip.c_str());
}

CNode::ip_type CNode::i322ip(int32 ip)
{
    if(ip == 0) {
        return "";
   }
    
   struct in_addr addr;
   memcpy(&addr, &ip, 4);
   return std::string(::inet_ntoa(addr));
}

