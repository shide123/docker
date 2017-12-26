/*
 * CRoomEvent.cpp
 *
 *  Created on: Aug 22, 2016
 *      Author: testtest
 */

#include "CRoomEvent.h"
#include "GateBase.h"
#include "CClientLogic.h"
#include "RoomSvr.pb.h"

CRoomEvent::CRoomEvent()
{
}

CRoomEvent::~CRoomEvent()
{
}

int CRoomEvent::handle_logic_msg(const char * msg, int len, clienthandler_ptr svr_conn)
{
	DEF_IVM_HEADER(in_msg, msg);
	DEF_IVM_CLIENTGATE(pGateMask, msg);
	char * pData = (char *)DEF_IVM_DATA(msg);
	int nMsgLen2 = in_msg->length - SIZE_IVM_CLIENTGATE;
	int nDataLen2 = in_msg->length - SIZE_IVM_HEADER - SIZE_IVM_CLIENTGATE;

	//转发消息
	unsigned int client_connid = pGateMask->param2;
	//应该通过 再次 查找 一下该 conn 是否存在
	//直接使用变量进行 判断的原因 是防止 部分野指针 直接调用函数 引起的 奔溃
	clienthandler_ptr pClientConn = CGateBase::m_ClientMgr.find_client_map(client_connid);
	if(!pClientConn)
	{
		LOG_PRINT(log_error, "client fd is closed,client connid:%u,maincmd:%u,subcmd:%u.server:%s:%u.",\
			client_connid, in_msg->maincmd, in_msg->subcmd, get_remote_ip(svr_conn), get_remote_port(svr_conn));

		if (Sub_Vchat_JoinRoomResp == in_msg->subcmd)
		{
			unsigned int rspDataLen = SIZE_IVM_REQUEST(in_msg);
			char * pRes = (char *)(in_msg->content + SIZE_IVM_CLIENTGATE);
			CMDJoinRoomResp rspData;
			rspData.ParseFromArray(pRes, rspDataLen);
			LOG_PRINT(log_warning, "[join room resp]client fd is closed,need to notify exit room.client connid:%u,userid:%u,roomid:%u.server:%s:%u.",\
				client_connid, rspData.userid(), rspData.vcbid(), get_remote_ip(svr_conn), get_remote_port(svr_conn));
			notify_svr_exitroom(rspData.vcbid(), rspData.userid(), pGateMask, svr_conn);
		}

		if (Sub_Vchat_GateJoinRoomResp == in_msg->subcmd)
		{
			unsigned int rspDataLen = SIZE_IVM_REQUEST(in_msg);
			char * pRes = (char *)(in_msg->content + SIZE_IVM_CLIENTGATE);
			CMDGateJoinRoomResp rspData;
			rspData.ParseFromArray(pRes, rspDataLen);
			if (!rspData.errinfo().errid())
			{
				LOG_PRINT(log_warning, "[rejoin room resp]client fd is closed,need to notify exit room.client connid:%u,userid:%u,roomid:%u.server:%s:%u.",\
					client_connid, rspData.userid(), rspData.vcbid(), get_remote_ip(svr_conn), get_remote_port(svr_conn));
				notify_svr_exitroom(rspData.vcbid(), rspData.userid(), pGateMask, svr_conn);
			}
		}

		return 0;
	}

	CClientItem * item = (CClientItem *)pClientConn->user_data;
	if(pClientConn->isconnected() && item)  
	{
		if (CGateBase::m_pCmdConfigMgr->needCheckRspTime(in_msg->subcmd))
		{
			unsigned int req_cmd = CGateBase::m_pCmdConfigMgr->getReqbyRspCmd(in_msg->subcmd);
			CGateBase::m_ClientMgr.del_req_check(client_connid, req_cmd);
		}

		switch(in_msg->subcmd)
		{
		case Sub_Vchat_JoinRoomResp:
			{
				unsigned int rspDataLen = SIZE_IVM_REQUEST(in_msg);
				char * pRes = (char *)(in_msg->content + SIZE_IVM_CLIENTGATE);
				CMDJoinRoomResp rspData;
				rspData.ParseFromArray(pRes, rspDataLen);

				if (item->m_exitroom)
				{
					LOG_PRINT(log_warning, "client exit room but receive join room response,need to notify exit room.client connid:%u,userid:%u,roomid:%u.",\
						client_connid, rspData.userid(), rspData.vcbid());
					notify_svr_exitroom(rspData.vcbid(), rspData.userid(), pGateMask, svr_conn);
					return 0;
				}
				else
				{
					LOG_PRINT(log_normal, "receive join room response success.client connid:%u,userid:%u,roomid:%u.server:%s:%u.",\
						client_connid, rspData.userid(), rspData.vcbid(), get_remote_ip(svr_conn), get_remote_port(svr_conn));

					CGateBase::m_RoomClientMgr.add_user_connid(rspData.vcbid(), rspData.userid(), client_connid);

					LOG_PRINT(log_debug, "[set group roletype]client connid:%u,userid:%u groupid:%u roleType:%u.", client_connid, rspData.userid(), rspData.vcbid(), rspData.userroletype());
					item->setGroupRole(rspData.vcbid(), rspData.userroletype());
				}
			}
			break;
		case Sub_Vchat_GateJoinRoomResp:
			{
				unsigned int rspDataLen = SIZE_IVM_REQUEST(in_msg);
				char * pRes = (char *)(in_msg->content + SIZE_IVM_CLIENTGATE);
				CMDGateJoinRoomResp rspData;
				rspData.ParseFromArray(pRes, rspDataLen);
				if (0 == rspData.errinfo().errid())
				{
					if (item->m_exitroom)
					{
						LOG_PRINT(log_warning, "client exit room but receive rejoin room response,need to notify exit room.client connid:%u,userid:%u,roomid:%u.",\
							client_connid, rspData.userid(), rspData.vcbid());
						notify_svr_exitroom(rspData.vcbid(), rspData.userid(), pGateMask, svr_conn);
					}
					else
					{
						LOG_PRINT(log_normal, "receive rejoin room response success.client connid:%u,userid:%u,roomid:%u.server:%s:%u.",\
							client_connid, pClientConn->user_id, pClientConn->room_id, get_remote_ip(svr_conn), get_remote_port(svr_conn));
					}
				}
				else
				{
					LOG_PRINT(log_warning, "[rejoin room resp fail]error:%d,need to clear data.client connid:%d,userid:%u,roomid:%u.server:%s:%u.",\
						rspData.errinfo().errid(), client_connid, rspData.userid(), rspData.vcbid(), get_remote_ip(svr_conn), get_remote_port(svr_conn));

					unsigned int roleType = e_VisitorRole;
					if (item->getGroupRole(rspData.vcbid(), roleType) && roleType == e_VisitorRole)
					{
						CGateBase::m_RoomClientMgr.del_user_connid(rspData.vcbid(), rspData.userid(), client_connid);
						item->removeGroupRole(rspData.vcbid());
					}

					if (pClientConn->room_id == rspData.vcbid())
					{
						pClientConn->room_id = 0;
					}
				}
				return 0;
			}
			break;
		case Sub_Vchat_RoomKickoutUserNoty:
			{
				unsigned int rspDataLen = SIZE_IVM_REQUEST(in_msg);
				char * pRes = (char *)(in_msg->content + SIZE_IVM_CLIENTGATE);
				CMDUserKickoutRoomInfo rspData;
				rspData.ParseFromArray(pRes, rspDataLen);
				CGateBase::m_RoomClientMgr.del_user_connid(rspData.vcbid(), rspData.toid(), client_connid);
				item->removeGroupRole(rspData.vcbid());
				if (pClientConn->room_id == rspData.vcbid())
				{
					pClientConn->room_id = 0;
					LOG_PRINT(log_debug, "[kick out user]need to reset roomid in user connection.client connid:%u,userid:%u,roomid:%u.", \
						client_connid, pClientConn->user_id, pClientConn->room_id);
				}
			}
			break;
		case Sub_Vchat_SetMicStateNotify:
			{
				CMDUserMicState rspData;
				rspData.ParseFromArray(pData, nDataLen2);
				if (rspData.errinfo().errid() != 0)
				{
					LOG_PRINT(log_error, "[set mic state fail]errCode:%d,runnid:%u,touserid:%u,groupid:%u,micindex:%d,mic state:%d,client connid:%u.", \
						rspData.errinfo().errid(), rspData.runid().userid(), rspData.toid().userid(), rspData.vcbid(), rspData.micindex(), rspData.micstate(), client_connid);
				}
			}
			break;
		case Sub_Vchat_JoinRoomErr:
			{
				CMDJoinRoomErr err;
				err.ParseFromArray(pData, nDataLen2);
				if (err.vcbid() == pClientConn->room_id)
					pClientConn->room_id = 0;
			}
			break;
		default:
			break;
		}

		//transfer msg to client
		SL_ByteBuffer buff(nMsgLen2);
		COM_MSG_HEADER * pOutMsg = (COM_MSG_HEADER *)buff.buffer();
		memcpy(pOutMsg, in_msg, SIZE_IVM_HEADER);
		memcpy(pOutMsg->content, pData, nDataLen2);

		pOutMsg->length = nMsgLen2;
		buff.data_end(pOutMsg->length);
		pClientConn->write_message(buff);
	}

	return 0;
}

void CRoomEvent::post_close_process(clienthandler_ptr svr_conn)
{
	if (e_roomsvr_type == svr_conn->getsvrtype())
	{
		client_rejoinroom(svr_conn);
	}
}

void CRoomEvent::client_rejoinroom(clienthandler_ptr svr_conn)
{
	std::set<unsigned int> client_conn_set;
	svr_conn->getallclientid(client_conn_set);

	if (!client_conn_set.empty())
	{
		LOG_PRINT(log_info, "[client rejoinroom]client-connect size:%u.server:%s:%u.", client_conn_set.size(), get_remote_ip(svr_conn), get_remote_port(svr_conn));

		std::set<unsigned int>::iterator iter = client_conn_set.begin();
		for (; iter != client_conn_set.end(); ++iter)
		{
			unsigned int client_connid = *iter;
			clienthandler_ptr pClientConn = CGateBase::m_ClientMgr.find_client_map(client_connid);
			if(!pClientConn)
			{
				LOG_PRINT(log_error, "client fd is closed.client connid:%d.server:%s:%u.", client_connid, get_remote_ip(svr_conn), get_remote_port(svr_conn));
			}
			else if(pClientConn->isconnected())  
			{
				CClientLogic logic(pClientConn);
				logic.rejoinroom(pClientConn->room_id, pClientConn->user_id);
			}
		}
	}
}

void CRoomEvent::notify_svr_exitroom(unsigned int roomid, unsigned int userid, ClientGateMask_t * pGate, clienthandler_ptr svr_conn)
{
	if (!roomid || !userid || !pGate || !svr_conn)
	{
		return;
	}

	CMDUserExitRoomInfo msginfo;
	msginfo.set_vcbid(roomid);
	msginfo.set_userid(userid);

	unsigned int reqLen = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + msginfo.ByteSize();
	SL_ByteBuffer buff(reqLen);
	buff.data_end(reqLen);

	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)buff.buffer();
	pHead->version = MDM_Version_Value;
	pHead->checkcode = CHECKCODE;
	pHead->maincmd = MDM_Vchat_Room;
	pHead->subcmd = Sub_Vchat_RoomUserExitReq;
	pHead->length = reqLen;

	ClientGateMask_t * pClientGate = (ClientGateMask_t *)(pHead->content);
	memcpy(pClientGate, pGate, SIZE_IVM_CLIENTGATE);
	char * pContent = (char *)(pHead->content + SIZE_IVM_CLIENTGATE);
	msginfo.SerializeToArray(pContent, msginfo.ByteSize());

	svr_conn->write_message(buff);
}


