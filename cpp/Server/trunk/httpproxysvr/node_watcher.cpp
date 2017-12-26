#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include "node_watcher.h"
//#include "simple_log.h"
#include "message_helper.h"
#include "cmd_nodemgr.h"

#include "CLogThread.h"

#include "node_client_impl.h"

boost::mutex    NodeEpollWatcher::m_mutex;
std::map<string,map<string,ServerIpInfo> > NodeEpollWatcher::m_sSubRpcServer;

int NodeEpollWatcher::on_readable(int &epollfd, epoll_event &event) 
{
	printf("NodeEpollWatcher::on_readable\n");
	EpollContext *epoll_context = (EpollContext *) event.data.ptr;
	int fd = epoll_context->fd;

	/*int buffer_size = SS_READ_BUFFER_SIZE;
	char read_buffer[buffer_size];
	memset(read_buffer, 0, buffer_size);*/

	int read_size = recv(fd, m_szRecvBuffer+m_nRecvLeftLen, en_msgbuffersize, 0);
	printf("NodeEpollWatcher::read_size=%d\n",read_size);
	if (read_size == -1 && errno == EINTR) {
		return READ_CONTINUE; 
	} 
	if (read_size == -1 /* io err*/|| read_size == 0 /* close */) 
	{
		return READ_CLOSE;
	}
	
	//LOG_DEBUG("read success which read size:%d", read_size);
	printf("NodeEpollWatcher read success which read size:%d\n", read_size);
	LOG_PRINT(log_info, "NodeEpollWatcher  read success which read size:%d!", read_size);
	m_nRecvLeftLen+=read_size;

	char* p=m_szRecvBuffer;
	while(m_nRecvLeftLen > 4)
	{
		int msglen=*((int*)p);
		if(msglen <= 0||msglen > en_msgbuffersize)
		{
			LOG_PRINT(log_error, "invalid msg len:%d!", msglen);
			m_nRecvLeftLen=0;
			return READ_CLOSE;
		}
		else if(m_nRecvLeftLen < msglen)
		{
			break;
		}
		else 
		{
			if(handleMessage(fd,p, msglen)==-1)
			{
				LOG_PRINT(log_error, "handleMessage failed!");
				m_nRecvLeftLen=0;
				return -1;
			}
			m_nRecvLeftLen -= msglen;
			p += msglen;
		}
	}
	LOG_PRINT(log_info, "NodeEpollWatcher  rm_nRecvLeftLen size:%d!", m_nRecvLeftLen);
	memmove(m_szRecvBuffer,p,m_nRecvLeftLen);


	return READ_CONTINUE;
}



int NodeEpollWatcher::on_close(EpollContext &epoll_context) 
{
	if (epoll_context.ptr == NULL) 
	{
		return 0;
	}
	
	return 0;
}


uint64  NodeEpollWatcher::getKey(const int32 & ip, const int32 & port)
{
	uint64 key = ip & 0xFFFFFFFF;
	key |= ((((uint64)port) & 0xFFFF) << 32);

	return key;
}

int32   NodeEpollWatcher::getip(uint64 key)
{
	return (key & 0xFFFFFFFF);
}

int32   NodeEpollWatcher::getport(uint64 key)
{
	return (key >> 32) & 0xFFFF;
}


std::string NodeEpollWatcher::u322ip(const uint32 ip)
{
	if(ip == 0) {
		return "";
	}

	struct in_addr addr;
	memcpy(&addr, &ip, 4);
	return std::string(::inet_ntoa(addr));
}

template<typename ServerInfo_t> int NodeEpollWatcher::getIpPort (ServerInfo_t *svr,std::string &sIp,int &iPort)
{
	uint64 skey = getKey(svr->ip, svr->port);
	int32   iip = getip(skey);
	iPort   = getport(skey);
	sIp     = u322ip(iip);

	return 0;
}

int  NodeEpollWatcher::handleMessage(int fd,const char* pdata, int msglen)
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
	if (maincmd != MDM_Vchat_Nodemgr) 
	{
		return 0;
	}


	switch (subcmd) 
	{
	case Sub_NodeMgr_SubscribeServerResp: 
		{

		printf("%02hhx\n",pdata[0]);
		CMDSubscribeServerResp_t * resp =(CMDSubscribeServerResp_t *)(pdata+sizeof(COM_MSG_HEADER)-4); //GetSubCmdObject<CMDSubscribeServerResp_t>(pdata);

		CMDSubscribeServerInfo_t * svr = NULL;


		printf("sizeof(COM_MSG_HEADER)=%d,resp->num=%u\n",sizeof(COM_MSG_HEADER),resp->num);


		for (uint32 i = 0; i < resp->num; i++) 
		{
			svr = (CMDSubscribeServerInfo_t *)(resp->content + i * sizeof(CMDSubscribeServerInfo_t));

			std::string   svrname(svr->servername);
			std::string		desc("");

			int port=0;
			std::string sIp;
			printf("resp->num=%u,svr.ip=%u,svr.port=%u\n",resp->num,svr->ip,svr->port);

			getIpPort<CMDSubscribeServerInfo_t>(svr,sIp,port);
			printf("sIp=%s,port=%d\n",sIp.c_str(),port);

			onNodeEvent(svrname, sIp, port, INodeNotify::en_svr_add,desc);
		}
		break;
		}
	case Sub_NodeMgr_SubscribeServerV2Resp: {
		CMDSubscribeServerResp_t * resp 
			= GetSubCmdObject<CMDSubscribeServerResp_t>(pdata);

		LOG_PRINT(log_info, "receive length:%d", msglen);

		CMDSubscribeServerInfoV2_t * svr = NULL;
/*
		{
			std::string svrname("fullpullv2");
			std::string desc;
			int32 ip    = 0;
			int16 port  = 0;

			onNodeEvent( svrname, ip, port, desc,
				IWatcher::en_svr_full);
		}*/

		char * baseaddr = resp->content;
		int offset 		= 0;

		for (uint32 i = 0; i < resp->num; i++) 
		{
			svr = (CMDSubscribeServerInfoV2_t *)
				(baseaddr + offset);

			std::string   svrname(svr->servername);
			std::string		desc(GET_DESC_DATA(svr), GET_DESC_LEN(svr));
			offset += sizeof(CMDSubscribeServerInfoV2_t) + desc.size();

			int port=0;
			std::string sIp;
			getIpPort<CMDSubscribeServerInfoV2_t>(svr,sIp,port);
			onNodeEvent( svrname, sIp, port, INodeNotify::en_svr_add,desc);
		}
		break;
		}

	case Sub_NodeMgr_SubscribeServerAdd: 
		{
		CMDNoticeServerAdd_t * svr 
			= GetSubCmdObject<CMDNoticeServerAdd_t>(pdata);

		std::string   svrname(svr->servername);
		std::string		desc("");

		int port=0;
		std::string sIp;
		getIpPort<CMDNoticeServerAdd_t>(svr,sIp,port);
		onNodeEvent( svrname, sIp, port, INodeNotify::en_svr_add,desc);

		break;
		 }

	case Sub_NodeMgr_SubscribeServerAddV2: 
		{
		CMDNoticeServerAddV2_t * svr 
			= GetSubCmdObject<CMDNoticeServerAddV2_t>(pdata);

		std::string   svrname(svr->servername);
		std::string		desc(GET_DESC_DATA(svr), GET_DESC_LEN(svr));

		int port=0;
		std::string sIp;
		getIpPort<CMDNoticeServerAddV2_t>(svr,sIp,port);
		onNodeEvent( svrname, sIp, port, INodeNotify::en_svr_add,desc);

		break;
		
		}
	case Sub_NodeMgr_SubscribeServerRemove: 
		{
		CMDNoticeServerRemove_t * svr 
			= GetSubCmdObject<CMDNoticeServerRemove_t>(pdata);

		std::string   svrname(svr->servername);
		std::string		desc("");

		int port=0;
		std::string sIp;
		getIpPort<CMDNoticeServerRemove_t>(svr,sIp,port);
		onNodeEvent( svrname, sIp, port, INodeNotify::en_svr_rem,desc);
		break;
											}
	case Sub_NodeMgr_Echo:
		{
		  
		  break;
		}
	default:
		break;
	}
	return 0;
}


void NodeEpollWatcher::onNodeEvent(const std::string & svrname, const std::string & ip, int port, INodeNotify::en_node_event event, std::string desc /*= ""*/)
{
	/*
	* @svrname: 	service name, such as "logonsvr", "roomsvr".
	* @ip:			service ip.
	* @port:		service port.
	* @event:		add or remove. connect if add, and disconnect where remove.
	*/
	LOG_PRINT(log_info, "recv node event: svrname[%s] ip[%s] port[%d] event[%d].", svrname.c_str(), ip.c_str(), port, event);
	if (svrname.empty() || ip.empty() || 0 == port)
		return;

	int svrtype = 0;//(e_SvrType)CThriftSvrMgr::change_svrname_to_type(svrname);
	if (event == INodeNotify::en_svr_add)
	{
		ServerIpInfo tServerIpInfo;
		tServerIpInfo.sIp=ip;
		tServerIpInfo.iPort=port;
		tServerIpInfo.sServerName=svrname;

		boost::mutex::scoped_lock lock(m_mutex);
		m_sSubRpcServer[svrname][ip]=tServerIpInfo;
		
	}
	else if (event == INodeNotify::en_svr_rem)
	{
		LOG_PRINT(log_info, "svr_type:%d ip:%s port:%d had closed", svrtype, ip.c_str(), port);
		if (m_sSubRpcServer.find(svrname) != m_sSubRpcServer.end())
		{
			//CThriftSvrMgr::delThriftClient(ip, port, svrtype);
		}
	}
}


bool NodeEpollWatcher::getServerInfoByServerName(const string &sServerName,ServerIpInfo & tServerIpInfo)
{
	boost::mutex::scoped_lock lock(m_mutex);

	std::map<string,map<string,ServerIpInfo> > ::iterator it=m_sSubRpcServer.find(sServerName);
	if (it!=m_sSubRpcServer.end())
	{
		tServerIpInfo=it->second.begin()->second;
		return true;
	}

	return false;
	
}