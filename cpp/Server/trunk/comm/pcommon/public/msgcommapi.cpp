#include "msgcommapi.h"
#include <sstream>

char * CMsgComm::Build_COM_MSG_HEADER(char * pPkt, int checkcode, int version, int maincmd, int subcmd, int pPktLen)
{
	COM_MSG_HEADER * pOutMsg = (COM_MSG_HEADER *)pPkt;
	pOutMsg->checkcode = checkcode;
	pOutMsg->version = version;
	pOutMsg->maincmd = maincmd;
	pOutMsg->subcmd = subcmd;
	pOutMsg->length = pPktLen;
	return pPkt + sizeof(COM_MSG_HEADER);
}

char * CMsgComm::Build_COM_MSG_HEADER(char * pPkt, int maincmd, int subcmd, int pPktLen /*= 0*/)
{
	COM_MSG_HEADER * pOutMsg = (COM_MSG_HEADER *)pPkt;
	pOutMsg->checkcode = CHECKCODE;
	pOutMsg->version = MDM_Version_Value;
	pOutMsg->maincmd = maincmd;
	pOutMsg->subcmd = subcmd;
	pOutMsg->length = pPktLen;
	return pPkt + sizeof(COM_MSG_HEADER);
}

int CMsgComm::Build_NetMsg(char * szBuf, unsigned int nBufLen, unsigned int mainCmdId, unsigned int subCmdId, ClientGateMask_t ** ppGateMask, void * pData, unsigned int pDataLen)
{
	if(!szBuf || !pData)
	{
		LOG_PRINT(log_error, "(szBuf==0 || pData==0).");
		return -1;
	}

	int nMsgLen = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + pDataLen;
	if(nBufLen <= nMsgLen)
	{
		LOG_PRINT(log_error, "(nBufLen <= nMsgLen).");
		return -1;
	}

	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)szBuf;
	pHead->version = MDM_Version_Value;
	pHead->checkcode = CHECKCODE;
	pHead->maincmd = mainCmdId;
	pHead->subcmd = subCmdId;
	ClientGateMask_t * pClientGate = (ClientGateMask_t *)(pHead->content);
	memset(pClientGate, 0, sizeof(ClientGateMask_t));
	void * pContent = (void *)(pHead->content + SIZE_IVM_CLIENTGATE);
	memcpy(pContent, pData, pDataLen);
	pHead->length = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + pDataLen;
	(*ppGateMask) = pClientGate;
	return pHead->length;
}

void CMsgComm::Build_BroadCastRoomGate(ClientGateMask_t * pGateMask, e_NoticeDevType e_dev_type, unsigned int roomid, unsigned int except_connid, bool inGroupOnly/* = false*/)
{
	pGateMask->param1 = e_dev_type;
	pGateMask->param2 = except_connid;
	pGateMask->param3 = BROADCAST_ROOM_TYPE;
	pGateMask->param4 = BROADCAST_ROOM_TYPE;
	pGateMask->param5 = roomid;
	pGateMask->param6 = (inGroupOnly ? 1: 0);
}

void CMsgComm::Build_BroadCastRoomRoleType(ClientGateMask_t * pGateMask, unsigned int roomid, unsigned int minRole, unsigned int maxRole, e_NoticeDevType e_dev_type /*= e_Notice_AllType*/, unsigned int userid /*= 0*/)
{
	unsigned long long param = minRole;
	param = (param << 32) | maxRole;

	pGateMask->param1 = e_dev_type;
	pGateMask->param2 = roomid;
	pGateMask->param3 = BROADCAST_ROOM_ROLETYPE;
	pGateMask->param4 = BROADCAST_ROOM_ROLETYPE;
	pGateMask->param5 = param;
	pGateMask->param6 = userid;
}

void CMsgComm::Build_BroadCastRoomGender_Gate(ClientGateMask_t * pGateMask, unsigned int roomid, unsigned int gender)
{
	pGateMask->param1 = 0;
	pGateMask->param2 = roomid;
	pGateMask->param3 = BROADCAST_ROOM_GENDER;
	pGateMask->param4 = BROADCAST_ROOM_GENDER;
	pGateMask->param5 = gender;
	pGateMask->param6 = 0;
}

void CMsgComm::Build_BroadCastOnLine_Gate(ClientGateMask_t * pGateMask, e_NoticeDevType e_dev_type, unsigned int except_connid)
{
	pGateMask->param1 = e_dev_type;
	pGateMask->param2 = except_connid;
	pGateMask->param3 = BROADCAST_TYPE;
	pGateMask->param4 = BROADCAST_TYPE;
	pGateMask->param5 = 0;
	pGateMask->param6 = 0;
}

void CMsgComm::Build_MultiCastOnLine_Gate(ClientGateMask_t * pGateMask, e_NoticeDevType e_dev_type, unsigned int except_connid)
{
	pGateMask->param1 = e_dev_type;
	pGateMask->param2 = except_connid;
	pGateMask->param3 = MULTICAST_TYPE;
	pGateMask->param4 = MULTICAST_TYPE;
	pGateMask->param5 = 0;
	pGateMask->param6 = 0;
}

void CMsgComm::Build_BroadCastSubscribeGate(ClientGateMask_t * pGateMask, e_NoticeDevType e_dev_type)
{
	pGateMask->param1 = e_dev_type;
	pGateMask->param2 = 0;
	pGateMask->param3 = CAST_CLIENTS_SUBSCRIBE;
	pGateMask->param4 = CAST_CLIENTS_SUBSCRIBE;
	pGateMask->param5 = 0;
	pGateMask->param6 = 0;
}

void CMsgComm::Build_BroadCastUser_Gate(ClientGateMask_t * pGateMask, unsigned int userid, e_NoticeDevType e_dev_type /*= e_Notice_AllType*/, unsigned int except_connid /*= 0*/)
{
	pGateMask->param1 = e_dev_type;
	pGateMask->param2 = except_connid;
	pGateMask->param3 = BROADCAST_TYPE;
	pGateMask->param4 = BROADCAST_TYPE;
	pGateMask->param5 = userid;
	pGateMask->param6 = 0;
}

void CMsgComm::Build_BroadCastUsersOnOneSvr_Gate(ClientGateMask_t * pGateMask, e_NoticeDevType e_dev_type /*= e_Notice_AllType*/, unsigned int except_connid /*= 0*/)
{
	pGateMask->param1 = e_dev_type;
	pGateMask->param2 = except_connid;
	pGateMask->param3 = CAST_CLIENTS_ON_ONE_SVR;
	pGateMask->param4 = CAST_CLIENTS_ON_ONE_SVR;
	pGateMask->param5 = 0;
	pGateMask->param6 = 0;
}

void CMsgComm::Build_BroadCastUsersOnOneSvr_Gate(ClientGateMask_t * pGateMask, unsigned int roomid, e_NoticeDevType e_dev_type /*= e_Notice_AllType*/, unsigned int except_connid /*= 0*/)
{
	pGateMask->param1 = e_dev_type;
	pGateMask->param2 = except_connid;
	pGateMask->param3 = CAST_CLIENTS_ON_ONE_SVR;
	pGateMask->param4 = CAST_CLIENTS_ON_ONE_SVR;
	pGateMask->param5 = roomid;
	pGateMask->param6 = 0;
}

void CMsgComm::Build_BroadCastAppVersion_Gate(ClientGateMask_t * pGateMask, std::string strVersion, e_NoticeDevType e_dev_type /*= e_Notice_AllType*/)
{
	pGateMask->param1 = e_dev_type;
	pGateMask->param2 = 0;
	pGateMask->param3 = CAST_APP_VERSION;
	pGateMask->param4 = CAST_APP_VERSION;
	pGateMask->param5 = convVersionStringToU64(strVersion);
	pGateMask->param6 = 0;
}

int CMsgComm::Build_RegisterCmd_Msg(char * szBuf, unsigned int nBufLen, e_SvrType e_svr_type, const std::string & cmdlist, const std::list<std::string> & cmdrange_lst)
{
	if (cmdrange_lst.empty() && cmdlist.empty())
	{
		LOG_PRINT(log_warning, "this server has no cmd to register.");
		return -1;
	}

	std::stringstream str;
	str << "svr_type=" << e_svr_type;
	
	if (!cmdlist.empty())
	{
		str << "&cmdlist=" << cmdlist;
	}
	
	std::list<std::string>::const_iterator iter = cmdrange_lst.begin();
	for (; iter != cmdrange_lst.end(); ++iter)
	{
		str << "&" << *iter;
	}

	unsigned int msglen = SIZE_IVM_HEADER + sizeof(int) + str.str().size();
	LOG_PRINT(log_info, "[Build_RegisterCmd_Msg]cmd:%s,msglen:%u.", str.str().c_str(), msglen);

	if (nBufLen < msglen)
	{
		LOG_PRINT(log_error, "buf len %u is less than %u.", nBufLen, msglen);
		return -1;
	}

	COM_MSG_HEADER * out_msg = (COM_MSG_HEADER *)szBuf;
	out_msg->checkcode = CHECKCODE;
	out_msg->subcmd = Sub_Vchat_ClientHelloResp;
	out_msg->length = msglen;
	int * psize = (int *)out_msg->content;
	*psize = strlen(str.str().c_str());
	memcpy(out_msg->content + sizeof(int), str.str().c_str(), *psize);

	return msglen;
}

bool CMsgComm::convSvrMsgToClientMsg(const char *szBuf, unsigned int nBufLen, SL_ByteBuffer &buff)
{
	if (!szBuf || !nBufLen || nBufLen < SIZE_IVM_HEAD_TOTAL)
	{
		return false;
	}
	buff.resize(nBufLen - SIZE_IVM_CLIENTGATE);
	buff.reset();
	buff.write(szBuf, SIZE_IVM_HEADER);
	buff.write(szBuf + SIZE_IVM_HEAD_TOTAL, nBufLen - SIZE_IVM_HEAD_TOTAL);
	((COM_MSG_HEADER *)buff.buffer())->length -= SIZE_IVM_CLIENTGATE;
	
	return true;
}
