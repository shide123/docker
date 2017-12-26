#ifndef __CLIENTLOGIC_H__
#define __CLIENTLOGIC_H__

#include "ProtocolsBase.h"
#include "clienthandler.h"
#include "CClientItem.h"
#include "msgcommapi.h"

class CClientLogic
{
public:
	CClientLogic(clienthandler_ptr handler);
	virtual ~CClientLogic(void);

	bool handle_message(const char * pdata, int msglen);
	void rejoinroom(unsigned int roomid, unsigned int userid);

	void post_user_login(char * rspData, unsigned int rspLen);
	void requestUnreadNoticeMsg();
	void qryUserAttentionList();
	void handle_user_grouplst(char * pRspData, unsigned int rspLen);
	void handle_assist_unreadmsg();
	void handleUserKickOutMsg(char * pRspData, unsigned int rspLen);
	void handleUserTransferMicState(char * pRspData, unsigned int rspLen);
	void notify_svr_clientexit();
	void notify_svr_exceptexit();
	void notify_svr_clienttimeout();
	void notify_svr_downmic();

private:
	int handle_logonsvr_msg(char * data, const std::string & svr_ip, unsigned int svr_port);
	int handle_roomsvr_msg(char * data, const std::string & svr_ip, unsigned int svr_port, unsigned int svr_connID, int svr_type);
	void handle_ping_msg(COM_MSG_HEADER & head);
	void handle_hello_msg(COM_MSG_HEADER & head);
	void handle_regist_dev(COM_MSG_HEADER & head);
	void handle_subscribe_msg(COM_MSG_HEADER & head);
	void handle_subscribe_cmdlst(COM_MSG_HEADER & head);
	void handle_group_unreadmsg(unsigned int groupid, SL_ByteBuffer & buff, unsigned int msgLen);

	int build_netmsg_svr(char * szBuf, int nBufLen, int mainCmdId, int subCmdId, void * pData, int pDataLen);

	void notify_svr_clientclosesocket();
	void notify_svr_exceptexitroom(unsigned int roomid, unsigned int userid);
	void notify_svr_exitroom(unsigned int roomid, unsigned int userid);
	void notify_svr_kickoutroom(unsigned int roomid, unsigned int userid, int reasonid);
	std::string get_distributed_key_msg(char * in_msg);
	void print_specail_cmd(unsigned int subcmd, const std::string & distributed_value, const std::string & svr_ip, unsigned int svr_port);
	void remove_svr_connect_map(char * in_msg);
	void genClientGateMask(ClientGateMask_t *pGateMask);
	template<typename ProtoType>
	void sendRequestToServer(clienthandler_ptr conn, int subcmd, const ProtoType &req, int main_cmd = MDM_Vchat_Room, bool bGateMask = true);

	//DATA
	CClientItem * clientItem(){ return (CClientItem *) m_pClientHandler->user_data; }
	unsigned int user_id(){ return m_pClientHandler->user_id; }
	void set_user_id(unsigned int user_id){ m_pClientHandler->user_id = user_id; }
	unsigned int room_id(){ return m_pClientHandler->room_id; }
	void set_room_id(unsigned int room_id){ m_pClientHandler->room_id = room_id; }
	unsigned int conn_id(){ return m_pClientHandler->getconnid(); }
	const char * get_remote_ip(){ return m_pClientHandler->getremote_ip(); }
	ushort get_remote_port(){ return m_pClientHandler->getremote_port(); }
	void setLastActiveTime();
	void resperrinf(COM_MSG_HEADER *reqHead, uint16 errcode);

private:
	clienthandler_ptr m_pClientHandler;
};

template<typename ProtoType>
void CClientLogic::sendRequestToServer(clienthandler_ptr conn, int subcmd, const ProtoType &req, int main_cmd/* = MDM_Vchat_Room*/, bool bGateMask/* = true*/)
{
	int nHeadLen = (bGateMask ? SIZE_IVM_HEAD_TOTAL: SIZE_IVM_HEADER);
	int nProtoLen = req.ByteSize();
	int nBufLen = nHeadLen + nProtoLen;
	SL_ByteBuffer buff(nBufLen);
	buff.data_end(nBufLen);
	CMsgComm::Build_COM_MSG_HEADER(buff.buffer(), main_cmd, subcmd, nBufLen);
	if (bGateMask)
	{
		DEF_IVM_CLIENTGATE(pGateMask, buff.buffer());
		genClientGateMask(pGateMask);
	}
	req.SerializeToArray(buff.buffer()+nHeadLen, nProtoLen);
	conn->write_message(buff.buffer(), nBufLen);
	
}

#endif //__CLIENTLOGIC_H__
