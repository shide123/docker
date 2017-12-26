#ifndef TIMER_H_
#define TIMER_H_
#include <string.h>
#include <errno.h>

#include "message_helper.h"
#include "cmd_nodemgr.h"
#include "CLogThread.h"
#include "node_client_impl.h"

#include "epoll_socket.h"


std::string m_svrname="";//"httpproxy";
int m_hostport=3456;

std::string m_hostip="123.103.74.8";


char isRegister=false;


int encode_com_header(COM_MSG_HEADER *header, char *writeptr, int *wlen)
{

	uint32_t offset = 0;

	memcpy(writeptr + offset, &header->length, sizeof(header->length));

	offset += sizeof(header->length);
	memcpy(writeptr + offset, &header->version, sizeof(header->version));

	offset += sizeof(header->version);
	memcpy(writeptr + offset, &header->checkcode, sizeof(header->checkcode));


	offset += sizeof(header->checkcode);
	memcpy(writeptr + offset, &header->maincmd, sizeof(header->maincmd));


	offset += sizeof(header->maincmd);
	memcpy(writeptr + offset, &header->subcmd, sizeof(header->subcmd));


	offset += sizeof(header->subcmd);
	*wlen = (int)offset;

	return 0;

}

int encode_register_body(CMDMultiCardRegisterServer_t *body, char *writeptr, int *wlen)
{

	uint32_t offset = 0;

	memcpy(writeptr + offset, &body->servername, sizeof(body->servername));

	offset += sizeof(body->servername);
	memcpy(writeptr + offset, &body->ip, sizeof(body->ip));

	offset += sizeof(body->ip);
	memcpy(writeptr + offset, &body->port, sizeof(body->port));


	offset += sizeof(body->port);
	*wlen = (int)offset;

	return 0;

}


int encode_subscribe_body(CMDSubscribeServer_t *body, char *writeptr, int *wlen)
{

	uint32_t offset = 0;

	memcpy(writeptr + offset, &body->servername, sizeof(body->servername));

	offset += sizeof(body->servername);
	memcpy(writeptr + offset, &body->servernamelen, sizeof(body->servernamelen));

	offset += sizeof(body->servernamelen);
	memcpy(writeptr + offset, &body->servernames, body->servernamelen);


	offset += body->servernamelen;
	*wlen = (int)offset;

	return 0;

}


void register_node()
{
	if (isRegister)
	{
		//return;
	}
	
	char buf[SIZE_IVM_HEADER + sizeof(CMDMultiCardRegisterServer_t)];
	memset(buf,0,SIZE_IVM_HEADER + sizeof(CMDMultiCardRegisterServer_t));
	/*CMDMultiCardRegisterServer_t * req = 
		GetAndFillHeader<CMDMultiCardRegisterServer_t>(buf, 
		MDM_Version_Value, 0, 
		MDM_Vchat_Nodemgr, 
		Sub_NodeMgr_MultiCardRegisterServerReq);

	int32 cpylen = std::min((int32)SERVER_NAME_LEN, (int32)m_svrname.size());
	strncpy(req->servername-4, m_svrname.c_str(), cpylen);
	req->servername[cpylen] = '\0';
	printf("cpylen=%d,req->servername=%s\n",cpylen,req->servername);

	memset(req->ip, 0, sizeof(req->ip));
	if (!m_hostip.empty()) {
		int32 cpylen = std::min((int32)128 - 1, (int32)m_hostip.size());
		memcpy(req->ip-4, m_hostip.c_str(), cpylen);
	}

	req->port       = m_hostport;*/


	int wheadlen=0;
	COM_MSG_HEADER header;
    header.length= sizeof(COM_MSG_HEADER) + sizeof(CMDMultiCardRegisterServer_t);
	header.version=MDM_Version_Value;
	header.checkcode=0;
	header.maincmd=MDM_Vchat_Nodemgr;
	header.subcmd=Sub_NodeMgr_MultiCardRegisterServerReq;
	encode_com_header(&header,buf,&wheadlen);


	int wbodylen=0;
	CMDMultiCardRegisterServer_t req;
	memset(&req,0,sizeof(req));

	strncpy(req.servername,m_svrname.c_str(),m_svrname.size());
	strncpy(req.ip,m_hostip.c_str(),m_hostip.size());
	req.port=m_hostport;

	encode_register_body(&req,buf+wheadlen,&wbodylen);

	COM_MSG_HEADER *lastheader=(COM_MSG_HEADER *)buf;
	lastheader->length=wheadlen+wbodylen;

	printf("register, port:%d, length:%d.wheadlen=%d, wbodylen=%d \n", req.port, GetMessageLength(buf),wheadlen,wbodylen);
	printf("register_node,node fd=%d\n",EpollSocket::_nodefd);


	EpollSocket::tcp_send(EpollSocket::get_nodefd(),buf,wheadlen+wbodylen);



	return ;
}


void subscribe_server()
{
	char buf[SIZE_IVM_HEADER + sizeof(CMDSubscribeServer_t)];
	memset(buf,0,SIZE_IVM_HEADER + sizeof(CMDSubscribeServer_t));


	int wheadlen=0;
	COM_MSG_HEADER header;
	header.length= sizeof(COM_MSG_HEADER) + sizeof(CMDSubscribeServer_t);
	header.version=MDM_Version_Value;
	header.checkcode=0;
	header.maincmd=MDM_Vchat_Nodemgr;
	header.subcmd=Sub_NodeMgr_SubscribeServerReq;
	encode_com_header(&header,buf,&wheadlen);


	int wbodylen=0;
	CMDSubscribeServer_t req;
	memset(&req,0,sizeof(req));

	strncpy(req.servername,m_svrname.c_str(),m_svrname.size());
	req.servernamelen=strlen("roomsvr_ql_rpc,chatsvr_ql_rpc,");
	strncpy(req.servernames,"roomsvr_ql_rpc,chatsvr_ql_rpc,",req.servernamelen);

	encode_subscribe_body(&req,buf+wheadlen,&wbodylen);

	COM_MSG_HEADER *lastheader=(COM_MSG_HEADER *)buf;
	lastheader->length=wheadlen+wbodylen;

	printf("subscribe_server,  length:%d.wheadlen=%d, wbodylen=%d \n", GetMessageLength(buf),wheadlen,wbodylen);
	printf("subscribe_server,node fd=%d\n",EpollSocket::get_nodefd());


	int ret=EpollSocket::tcp_send(EpollSocket::get_nodefd(),buf,wheadlen+wbodylen);
	if (ret==-2)
	{
		LOG_PRINT(log_info,"timer,this adr: %x\n", EpollSocket::get_node_epoll_adr());
		EpollSocket* es = (EpollSocket *)EpollSocket::get_node_epoll_adr();
		LOG_PRINT(log_info,"timer reconnect node...\n");
		es->reconnect_node();
		
	}
	

	return ;
}



void subscribe_server_v2()
{

	char buf[4096];
	memset(buf,0,sizeof(buf));

	CMDSubscribeServer_t * req = GetAndFillHeader<CMDSubscribeServer_t>(buf, 
		MDM_Version_Value, 
		0, 
		MDM_Vchat_Nodemgr, 
		Sub_NodeMgr_SubscribeServerReq);

	int32 cpylen = std::min((int32)SERVER_NAME_LEN, (int32)m_svrname.size());
	strncpy(req->servername, m_svrname.c_str(), cpylen);
	req->servername[cpylen] = '\0';

	std::string svrlst="roomsvr_ql_rpc,";

	printf("size=%zd\n",svrlst.size());

	req->servernamelen  = svrlst.size() + 1;
	strncpy(req->servernames, svrlst.c_str(), 
		svrlst.size());
	req->servernames[svrlst.size()] = '\0';

	COM_MSG_HEADER * header = GetObject<COM_MSG_HEADER>(buf);
	header->length  = GetMessageLength(buf) + svrlst.size() + 1;

	EpollSocket::tcp_send(EpollSocket::get_nodefd(),buf,GetMessageLength(buf));

	return ;
}


void on_timer(const boost::system::error_code& e, boost::asio::deadline_timer& t, int &nodefd) 
{
	//register_node();

	
	subscribe_server();


	if(!e)  
	{  
		//count++;
		t.expires_from_now(boost::posix_time::seconds(2));  
		t.async_wait(boost::bind(on_timer,   
			boost::asio::placeholders::error,  
			boost::ref(t),  
			boost::ref(nodefd)));  

	}   

	return ;
}


#endif