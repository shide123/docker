#include "CClientLogic.h"
#include "CLogThread.h"
#include "SL_ByteBuffer.h"
#include "message_comm.h"
#include "message_vchat.h"
#include "cmd_vchat.h"
#include "utils.h"
#include "GateBase.h"
#include "json/json.h"
#include "LogonSvr.pb.h"
#include "UserMgrSvr.pb.h"
#include "RoomSvr.pb.h"
#include "ChatSvr.pb.h"
#include "ConsumeSvr.pb.h"
#include "errcode.h"

CClientLogic::CClientLogic(clienthandler_ptr handler)
:m_pClientHandler(handler)
{
}

CClientLogic::~CClientLogic(void)
{
}

bool CClientLogic::handle_message(const char * pdata, int msglen)
{
	if (!m_pClientHandler)
	{
		LOG_PRINT(log_warning, "m_pClientHandler is null, return..")
		return false;
	}
	if (pdata == NULL || msglen < SIZE_IVM_HEADER)
	{
		LOG_PRINT(log_warning, "handle_message() err. [userid:%u,client connid:%u,%s:%d]",
			user_id(), conn_id(), get_remote_ip(), get_remote_port());
		return false;
	}

	COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *)pdata;
	if(msglen < sizeof(COM_MSG_HEADER) || in_msg->length < sizeof(COM_MSG_HEADER))
	{
		LOG_PRINT(log_warning, "client packet length check failed![userid:%u,client connid:%u,%s:%d]", 
			user_id(), conn_id(), get_remote_ip(), get_remote_port());
		return false;
	}

	if (in_msg->version != MDM_Version_Value)
	{
		LOG_PRINT(log_warning, "client packet version checks failed!input version:%d.[subcmd:%d, userid:%u,client connid:%u,%s:%d]",
			in_msg->subcmd, in_msg->version, user_id(), conn_id(), get_remote_ip(), get_remote_port());
		return true;
	}

	if (in_msg->length != msglen)
	{
		LOG_PRINT(log_warning, "client msg len is wrong!in_msg->length:%d,msglen:%d.[subcmd:%d, userid:%u,client connid:%u,%s:%d]", 
			in_msg->subcmd, in_msg->length, msglen, user_id(), conn_id(), get_remote_ip(), get_remote_port());
		return false;
	}

	//status is not connected,not need to handle data.
	if(!m_pClientHandler->isconnected())
	{
		LOG_PRINT(log_error, "client-connection m_current_status != connected, then not deal with data![subcmd:%d, userid:%u, client connid:%u,%s:%d]",
			in_msg->subcmd, user_id(), conn_id(), get_remote_ip(), get_remote_port());
		return true; 
	}

	//first handle hello packet.
	if(Sub_Vchat_ClientHello == in_msg->subcmd)
	{
		handle_hello_msg(*in_msg);
		return true;
	}

	//之后hello后才是合法的消息,其他消息不处理
	if(conn_id() == 0 || !m_pClientHandler->bSayHello)
	{
		LOG_PRINT(log_error, "client-connection has not said hello, ignore![subcmd:%d, userid:%u, client connid:%u,%s:%d]",
			in_msg->subcmd, user_id(), conn_id(), get_remote_ip(), get_remote_port());
		return true;
	}

	switch (in_msg->subcmd)
	{
	case Sub_Vchat_ClientPing:
		handle_ping_msg(*in_msg);
		return true;
	case Sub_Vchat_RegistDevReq:
		handle_regist_dev(*in_msg);
		return true;
	case Sub_Vchat_SubCmdReq:
		handle_subscribe_msg(*in_msg);
		return true;
	case Sub_Vchat_SubCmdLstReq:
		handle_subscribe_cmdlst(*in_msg);
		return true;
	case Sub_Vchat_GroupOnlookerChatReq:
		{
			CMDGroupMsgReq req;
			if (!req.ParseFromArray(pdata + SIZE_IVM_HEADER, in_msg->length - SIZE_IVM_HEADER) || room_id() != req.groupid())
			{
				LOG_PRINT(log_error, "user has not join room but send OnlookerChatReq.userid:%u,client connid:%u,marked groupid:%u.request groupid:%u.", \
					user_id(), conn_id(), room_id(), req.groupid());
				resperrinf(in_msg, ERR_CODE_USER_NOT_VISITOR);
				return true;
			}
		}
		break;
	default:
		break;
	}

	if (in_msg->subcmd != Sub_Vchat_logonReq5 && clientItem() && !clientItem()->m_blogon)
	{
		LOG_PRINT(log_error, "user has not login but send cmd request.subcmd:%u,userid:%u,client connid:%u.", in_msg->subcmd, user_id(), conn_id());
		return true;
	}

	remove_svr_connect_map((char *)in_msg);

	std::string distributed_value = get_distributed_key_msg((char *)in_msg);

	//according to subcmd,get svr_type
	SVR_TYPE_SET svr_type_set = CGateBase::m_pCmdConfigMgr->getSvrType(in_msg->subcmd);
	SVR_TYPE_SET::iterator iter_set = svr_type_set.begin();
	if (svr_type_set.empty())
	{
		LOG_PRINT(log_error, "subcmd %d can not match any server type,see your config file.",in_msg->subcmd)
	}

	for (; iter_set != svr_type_set.end(); ++iter_set)
	{
		int svr_type = *iter_set;

		unsigned int problem_svr_connID = clientItem()->find_problem_svr_conn(svr_type);

		//according user's svr_type,connect id,get server connection
		clienthandler_ptr pSvrConnPtr = CGateBase::m_SvrConnMgr.get_conn_inf(svr_type, conn_id(), distributed_value, problem_svr_connID);
		if (!pSvrConnPtr)
		{
			std::string svr_name = CGateBase::change_type_to_svrname(svr_type);

			time_t t = time(NULL);
			if(t - CGateBase::m_ClientMgr.get_last_alarmnotify_time() > CGateBase::getAlarmNotifyInterval())
			{
				//alarm notify
				char content[512] = {0};
				snprintf(content, 512, "[port:%d]get server connection error,svr_name:%s.", CGateBase::Port(), svr_name.c_str());
				CAlarmNotify::sendAlarmNoty(e_all_notitype, e_network_conn, CGateBase::ProcName(), "gate alarm", "Yunwei,Usermgr", content);
				CGateBase::m_ClientMgr.set_last_alarmnotify_time(t);
			}

		    LOG_PRINT(log_error, "server name:%s is down err [userid:%u,client connid:%u,%s:%d]", svr_name.c_str(), user_id(), conn_id(), get_remote_ip(), get_remote_port());
			
			//如果网关服务器没有与房间服务器建立好连接,则通知客户端
			char szBuf[128] = {0};
			COM_MSG_HEADER * pOutMsg = (COM_MSG_HEADER *)szBuf;
			pOutMsg->version = MDM_Version_Value;
			pOutMsg->checkcode = CHECKCODE;
			pOutMsg->maincmd = MDM_Vchat_Login;
			pOutMsg->subcmd = Sub_Vchat_DoNotReachRoomServer;
			pOutMsg->length = sizeof(COM_MSG_HEADER);
			m_pClientHandler->write_message(szBuf, pOutMsg->length);
		}
		else 
		{
			std::string svr_ip = pSvrConnPtr->getremote_ip();
			unsigned int svr_port = pSvrConnPtr->getremote_port();
			unsigned int svr_connID = pSvrConnPtr->getconnid();

			

			print_specail_cmd(in_msg->subcmd, distributed_value, svr_ip, svr_port);

			//如果网关与服务器建立好连接,则转发给服务器
			setLastActiveTime();

			if (-1 == handle_logonsvr_msg((char *)in_msg, svr_ip, svr_port))
			{
				return true;
			}

			if (-1 == handle_roomsvr_msg((char *)in_msg, svr_ip, svr_port, svr_connID, svr_type))
			{
				return true;
			}

			int nMsgLen = msglen + sizeof(ClientGateMask_t);
			SL_ByteBuffer buff(nMsgLen+1);
			COM_MSG_HEADER * pOutMsg = (COM_MSG_HEADER *)buff.buffer();
			ClientGateMask_t * pGateMask = (ClientGateMask_t *)(pOutMsg->content);
			memset(pGateMask, 0, sizeof(ClientGateMask_t));
			memcpy(pOutMsg, in_msg, sizeof(COM_MSG_HEADER));
			if(msglen > sizeof(COM_MSG_HEADER) && in_msg->length - sizeof(COM_MSG_HEADER) > 0)
				memcpy(pOutMsg->content+sizeof(ClientGateMask_t), in_msg->content, in_msg->length-sizeof(COM_MSG_HEADER));
			genClientGateMask(pGateMask);
			pOutMsg->length = in_msg->length + sizeof(ClientGateMask_t);
			buff.data_end(pOutMsg->length);

			
			LOG_PRINT(log_info, "svr_type[%d],in_msg->subcmd[%d],svr_ip[%s],svr_port[%d]",svr_type,in_msg->subcmd,svr_ip.c_str(),svr_port );

			pSvrConnPtr->write_message(buff);
		}
	}
	return true;
}

int CClientLogic::handle_logonsvr_msg(char * data, const std::string & svr_ip, unsigned int svr_port)
{
	bool blogon_req = false;

	COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *)data;
	switch(in_msg->subcmd)
	{
	case Sub_Vchat_logonReq5:
		{
			unsigned int reqLen = SIZE_IVM_NOMASK_REQUEST(in_msg);
			CMDUserLogonReq reqData;
			reqData.ParseFromArray(in_msg->content, reqLen);
			LOG_PRINT(log_info, "handle_logon_ipaddr[5]:client connid:%u,%s:%u,user login request:loginid %u nversion %u nmobile %u.logonsvr:%s:%u.",\
				conn_id(), get_remote_ip(), get_remote_port(), reqData.loginid(), reqData.version(), reqData.mobile(), svr_ip.c_str(), svr_port);

			clientItem()->m_uuid = reqData.serial();
			clientItem()->m_mobile = (unsigned char)reqData.mobile();
			clientItem()->m_platformType = (unsigned char)reqData.platformtype();
			clientItem()->m_nversion = reqData.version();
			blogon_req = true;
			break;
		}
	default:
		break;
	}

	if (blogon_req && clientItem()->m_blogon)
	{
		LOG_PRINT(log_warning, "this user do not leave software and logon again!userid:%u,nmobile:%u,%s:%u.", user_id(), clientItem()->m_mobile, get_remote_ip(), get_remote_port());
		return -1;
	}

	return 0;
}

int CClientLogic::handle_roomsvr_msg(char * data, const std::string & svr_ip, unsigned int svr_port, unsigned int svr_connID, int svr_type)
{
	COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *)data;

	if(Sub_Vchat_JoinRoomReq == in_msg->subcmd)
	{
		unsigned int devtype = 0;
		char * pReq = (char *)(in_msg->content);
		unsigned int reqLen = SIZE_IVM_NOMASK_REQUEST(in_msg);
		CMDJoinRoomReq reqData;
		reqData.ParseFromArray(pReq, reqLen);
		devtype = reqData.devtype();
		LOG_PRINT(log_info, "[client connid:%u,userid:%d,groupid:%d,devtype:%u,client:%s:%d]joinroomreq,roomsvr:%s:%u.",
			conn_id(), reqData.userid(), reqData.vcbid(), devtype, get_remote_ip(), get_remote_port(), svr_ip.c_str(), svr_port);

		if (!reqData.userid() || !reqData.vcbid())
		{
			LOG_PRINT(log_error, "joinroomreq data is wrong.userid:%d,groupid:%d.client connid:%u,%s:%d.", 
				reqData.userid(), reqData.vcbid(), conn_id(), get_remote_ip(), get_remote_port());
			return -1;
		}

		if (reqData.vcbid() && 0 == room_id())
		{
			set_room_id(reqData.vcbid());
		}

		if (reqData.vcbid() && reqData.vcbid() != room_id())
		{
			//exit old room
			notify_svr_exitroom(room_id(), user_id());

			unsigned int roleType = e_VisitorRole;
			if (clientItem()->getGroupRole(reqData.vcbid(), roleType) && roleType == e_VisitorRole)
			{
				CGateBase::m_RoomClientMgr.del_user_connid(room_id(), user_id(), conn_id());
				clientItem()->removeGroupRole(room_id());
			}

			//set new room
			set_room_id(reqData.vcbid());
		}

		REQ_SVR_INFO oInfo;
		oInfo.req_time = time(NULL);
		oInfo.svr_ip = svr_ip;
		oInfo.svr_port = svr_port;
		oInfo.svr_connID = svr_connID;
		oInfo.svr_type = svr_type;

		CGateBase::m_ClientMgr.insert_req_check(conn_id(), in_msg->subcmd, oInfo);
		clientItem()->m_exitroom = false;

		LOG_PRINT(log_info, "is Sub_Vchat_JoinRoomReq ");
	}

	// user left room message
	if(Sub_Vchat_RoomUserExitReq == in_msg->subcmd)
	{
		char * pReq = (char *)(in_msg->content);
		unsigned int reqLen = SIZE_IVM_NOMASK_REQUEST(in_msg);
		CMDUserExitRoomInfo reqData;
		reqData.ParseFromArray(pReq, reqLen);
		LOG_PRINT(log_info, "[client connid:%u,userid:%u,roomid:%u,client:%s:%d]user left room request,server:%s:%u.",
			conn_id(), reqData.userid(), reqData.vcbid(), get_remote_ip(), get_remote_port(), svr_ip.c_str(), svr_port);

		unsigned int roleType = e_VisitorRole;
		if (clientItem()->getGroupRole(reqData.vcbid(), roleType) && roleType == e_VisitorRole)
		{
			CGateBase::m_RoomClientMgr.del_user_connid(reqData.vcbid(), reqData.userid(), conn_id());
			clientItem()->removeGroupRole(reqData.vcbid());
		}

		if (0 == room_id() || reqData.vcbid() == room_id())
		{
			set_room_id(0);
			clientItem()->m_exitroom = true;
		}
		else
		{
			LOG_PRINT(log_warning, "client userid:%u send exit room %u request not equal to join room %u request.so do not handle it.", reqData.userid(), reqData.vcbid(), room_id());
			return -1;
		}
	}

	// user left group message
	if (Sub_Vchat_QuitGroupReq == in_msg->subcmd)
	{
		char * pReq = (char *)(in_msg->content);
		unsigned int reqLen = SIZE_IVM_NOMASK_REQUEST(in_msg);
		CMDQuitGroupReq reqData;
		reqData.ParseFromArray(pReq, reqLen);
		LOG_PRINT(log_info, "[client connid:%u,userid:%u,groupid:%u,client:%s:%d]user left group request,server:%s:%u.",
			conn_id(), reqData.userid(), reqData.groupid(), get_remote_ip(), get_remote_port(), svr_ip.c_str(), svr_port);

		CGateBase::m_RoomClientMgr.del_user_connid(reqData.groupid(), reqData.userid(), conn_id());
		clientItem()->removeGroupRole(reqData.groupid());
	}

	if(Sub_Vchat_SetMicStateReq == in_msg->subcmd)
	{
		char * pReq = (char *)(in_msg->content);
		unsigned int reqLen = SIZE_IVM_NOMASK_REQUEST(in_msg);
		CMDUserMicState reqData;
		reqData.ParseFromArray(pReq, reqLen);
		LOG_PRINT(log_info, "[set mic state client request]client connid:%u,runnid:%u,touserid:%u,groupid:%u,mic index:%d,mic state:%d.", \
			conn_id(), reqData.runid().userid(), reqData.toid().userid(), reqData.vcbid(), reqData.micindex(), reqData.micstate());
	}

	if (Sub_Vchat_RoomKickoutUserReq == in_msg->subcmd)
	{
		char * pReq = (char *)(in_msg->content);
		unsigned int reqLen = SIZE_IVM_NOMASK_REQUEST(in_msg);
		CMDUserKickoutRoomInfo reqData;
		reqData.ParseFromArray(pReq, reqLen);
		LOG_PRINT(log_info, "client send room kick out request.roomid:%u,kickout userid:%u.[client connid:%u,run_userid:%u,server:%s:%u]", \
			reqData.vcbid(), reqData.toid(), conn_id(), user_id(), svr_ip.c_str(), svr_port);
	}

	return 0;
}

void CClientLogic::setLastActiveTime()
{
	if (m_pClientHandler && m_pClientHandler->user_data)
	{
		CClientItem * item = (CClientItem *)m_pClientHandler->user_data;
		item->m_last_active_time = time(NULL);
	}
}

void CClientLogic::resperrinf(COM_MSG_HEADER * reqHead, uint16 errcode)
{
	if (reqHead == NULL)
		return ;

	char szOutBuf[128] = {0};
	COM_MSG_HEADER * pOutMsg=(COM_MSG_HEADER*)szOutBuf;
	pOutMsg->version = MDM_Version_Value;
	pOutMsg->checkcode = reqHead->checkcode;
	pOutMsg->maincmd = reqHead->maincmd;
	pOutMsg->subcmd = Sub_Vchat_Resp_ErrCode;
	pOutMsg->reqid = reqHead->reqid;

	CMDErrCodeResp_t * message = (CMDErrCodeResp_t*)(pOutMsg->content);
	memset(message, 0, sizeof(CMDErrCodeResp_t));
	message->errmaincmd = reqHead->maincmd;
	message->errsubcmd = reqHead->subcmd;
	message->errcode = errcode;

	pOutMsg->length = SIZE_IVM_HEADER + sizeof(CMDErrCodeResp_t);
	m_pClientHandler->write_message(szOutBuf, pOutMsg->length);
}

void CClientLogic::handle_ping_msg(COM_MSG_HEADER &head)
{
	if (head.length != SIZE_IVM_HEADER + sizeof(CMDClientPing_t))
	{
		return;
	}

	setLastActiveTime();

	CMDClientPing_t * pPingMsg = (CMDClientPing_t *)head.content;
	CClientItem * item = (CClientItem *)m_pClientHandler->user_data;
	unsigned int last_active_time = (NULL != item) ? item->m_last_active_time : 0; // TODO: time_t or uint ??

	//Client ping msg do not need to transfer to server.
	if (user_id() == 0)
	{
		set_user_id(pPingMsg->userid);
		if (user_id() != 0) 
		{
			LOG_PRINT(log_info, "CClientEvent::handle_ping_msg() ping userid:%d,client connid:%d", user_id(), conn_id());
		}
	}
	LOG_PRINT(log_debug, "setLastActiveTime() done: roomid = %d, userid = %d,"
		" m_last_active_time = %%u[%u], %%lu[%lu], %%llu[%llu]",
		m_pClientHandler->room_id, m_pClientHandler->user_id,
		last_active_time, last_active_time, last_active_time);

	int nMsgLen = sizeof(COM_MSG_HEADER) + sizeof(CMDClientPingResp_t);
	SL_ByteBuffer buff(nMsgLen);
	buff.data_end(nMsgLen);
	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)buff.buffer();
	pHead->version = MDM_Version_Value;
	pHead->checkcode = CHECKCODE;
	pHead->maincmd = head.maincmd;
	pHead->subcmd = Sub_Vchat_ClientPingResp;
	pHead->length = nMsgLen;
	pHead->reqid = head.reqid;

	CMDClientPingResp_t * resp = (CMDClientPingResp_t *)(pHead->content);
	resp->userid = pPingMsg->userid;
	resp->roomid = pPingMsg->roomid;

	m_pClientHandler->write_message(buff);
}


void CClientLogic::handle_hello_msg(COM_MSG_HEADER &head)
{
	if (head.length != SIZE_IVM_HEADER + sizeof(CMDClientHello_t))
	{
		return;
	}

	CMDClientHello_t * pHelloMsg = (CMDClientHello_t *)head.content;
	if(pHelloMsg->param1 == 12 &&  pHelloMsg->param2 == 8 && pHelloMsg->param3 == 7 && pHelloMsg->param4 == 1)
	{
		m_pClientHandler->bSayHello = true;
		setLastActiveTime();
		CGateBase::m_ClientMgr.update_client_map(m_pClientHandler);

		LOG_PRINT(log_info, "recv new hello-msg,new client connid:%d,%s:%d.", conn_id(), get_remote_ip(), get_remote_port());

		//print all connection information
		CGateBase::m_SvrConnMgr.print_conn_inf();
	}
}

void CClientLogic::handle_regist_dev(COM_MSG_HEADER &head)
{
	RegistReq_t *pReq = (RegistReq_t *) head.content;
	if (pReq->userid)
	{
		set_user_id(pReq->userid);
		clientItem()->m_mobile = pReq->devtype;
		clientItem()->m_last_login_time = time(NULL);
		LOG_PRINT(log_info, "client connid:%u receive regist dev request, user: %u, devtype: %u", conn_id(), pReq->userid, pReq->devtype);
		CGateBase::m_ClientMgr.setuserconn(user_id(), clientItem()->m_mobile, clientItem()->m_last_login_time, conn_id());
	}
}

void CClientLogic::handle_subscribe_msg(COM_MSG_HEADER &head)
{
	SubCmdReq_t *subscribe_cmd = (SubCmdReq_t *)head.content;
	if (subscribe_cmd && subscribe_cmd->subcmd)
	{
		clientItem()->set_pushmsg_pass(subscribe_cmd->subcmd, (bool)(subscribe_cmd->action));
		LOG_PRINT(log_info, "[client connid:%u,userid:%u,subcmd:%u,subscribe:%d]recv subscribe msg", conn_id(), user_id(), subscribe_cmd->subcmd, (int)subscribe_cmd->action);
	}
}

void CClientLogic::handle_subscribe_cmdlst(COM_MSG_HEADER &head)
{
	SubCmdLstReq_t *pReq = (SubCmdLstReq_t *)head.content;
	LOG_PRINT(log_info, "[client connid:%u,userid:%u]recv subscribe msg[user:%u, action:%u, json:%s]", conn_id(), user_id(), pReq->userid, pReq->action, pReq->json);
	if (pReq->userid != user_id())
	{
		return ;
	}

	try
	{
		if (pReq->userid && pReq->jsonlen)
		{
			Json::Reader reader(Json::Features::strictMode());
			Json::Value root;
			if (!reader.parse(pReq->json, root))
			{
				LOG_PRINT(log_error, "invalid Json format: %s", pReq->json);
				return;
			}

			if (root.isMember("cmdlist"))
			{
				Json::Value cmdlist = root["cmdlist"];
				for(unsigned int i = 0; i < cmdlist.size(); ++i) 
				{
					uint32 cmd = atoi(cmdlist[i].asString().c_str());
					clientItem()->set_pushmsg_pass(cmd, pReq->action ? true: false);
				}
			}
		}
	}
	catch (exception &e)
	{
		LOG_PRINT(log_error, "handle_subscribe_cmdlst error: %s", e.what());
		return;
	}
	catch (...)
	{
		LOG_PRINT(log_error, "handle_subscribe_cmdlst throw unknown error..");
		return;
	}
}

int CClientLogic::build_netmsg_svr(char * szBuf, int nBufLen, int mainCmdId, int subCmdId, void * pData, int pDataLen)
{
	if(szBuf == 0 || pData == 0)
	{
		LOG_PRINT(log_error, "(szBuf==0 || pData==0)");
		return -1;
	}

	int nMsgLen = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + pDataLen;
	if(nBufLen < nMsgLen)
	{
		LOG_PRINT(log_error, "(nBufLen <= nMsgLen)");
		return -1;
	}

	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)szBuf;
	pHead->version = MDM_Version_Value;
	pHead->checkcode = CHECKCODE;
	pHead->maincmd = mainCmdId;
	pHead->subcmd = subCmdId;
	ClientGateMask_t * pClientGate = (ClientGateMask_t *)(pHead->content);
	memset(pClientGate, 0, sizeof(ClientGateMask_t));
	pClientGate->param1 = (uint64)this;
	pClientGate->param2 = (uint64)conn_id();
	void * pContent = (void *)(pHead->content + SIZE_IVM_CLIENTGATE);
	memcpy(pContent, pData, pDataLen);
	pHead->length = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + pDataLen;

	return pHead->length;
}

void CClientLogic::rejoinroom(unsigned int roomid, unsigned int userid)
{
	if (!roomid || !userid)
	{
		return;
	}

	CMDGateJoinRoomReq msgInfo;
	msgInfo.set_userid(userid);
	msgInfo.set_cipaddr(get_remote_ip());
	msgInfo.set_uuid(clientItem()->m_uuid);
	msgInfo.set_devtype(clientItem()->m_mobile);
	msgInfo.set_micuserid(clientItem()->m_micToUserid);
	msgInfo.set_vcbid(clientItem()->m_micGroupID);
	msgInfo.set_micstate(clientItem()->m_micstate);
	msgInfo.set_micindex(clientItem()->m_micindex);

	unsigned int reqDataLen = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + msgInfo.ByteSize();
	SL_ByteBuffer reqData(reqDataLen);
	reqData.data_end(reqDataLen);

	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)reqData.buffer();
	pHead->version = MDM_Version_Value;
	pHead->checkcode = CHECKCODE;
	pHead->maincmd = MDM_Vchat_Room;
	pHead->subcmd = Sub_Vchat_GateJoinRoom;
	pHead->length = reqDataLen;

	ClientGateMask_t * pClientGate = (ClientGateMask_t *)(pHead->content);
	memset(pClientGate, 0, SIZE_IVM_CLIENTGATE);
	pClientGate->param1 = (uint64)this;
	pClientGate->param2 = (uint64)conn_id();

	char * pReq = (char *)(pHead->content + SIZE_IVM_CLIENTGATE);
	msgInfo.SerializeToArray(pReq, msgInfo.ByteSize());

	char distributed_value[32] = {0};
	sprintf(distributed_value, "%u", userid);

	unsigned int problem_svr_connID = clientItem()->find_problem_svr_conn(e_roomsvr_type);

	clienthandler_ptr pSvrConnPtr = CGateBase::m_SvrConnMgr.get_conn_inf(e_roomsvr_type, conn_id(), distributed_value, problem_svr_connID);
	if (!pSvrConnPtr)
	{
		LOG_PRINT(log_error, "cannot get roomsvr connection to rejoinroom.userid:%u,client connid:%u", userid, conn_id());

		//如果网关服务器没有与房间服务器建立好连接,则通知客户端
		char szBuf[128] = {0};
		COM_MSG_HEADER * pOutMsg = (COM_MSG_HEADER *)szBuf;
		pOutMsg->version = MDM_Version_Value;
		pOutMsg->checkcode = CHECKCODE;
		pOutMsg->maincmd = MDM_Vchat_Login;
		pOutMsg->subcmd = Sub_Vchat_DoNotReachRoomServer;
		pOutMsg->length = sizeof(COM_MSG_HEADER);
		m_pClientHandler->write_message(szBuf, pOutMsg->length);
		return;
	}

	if (pSvrConnPtr->isconnected())
	{
		LOG_PRINT(log_info, "[rejoin room]userid:%u,user ip:%s,nmobile:%d,mic roomid:%u,client connid:%u.micuserid:%u.micindex:%d.", \
			userid, get_remote_ip(), clientItem()->m_mobile, clientItem()->m_micGroupID, conn_id(), clientItem()->m_micToUserid, (int)clientItem()->m_micindex);
		pSvrConnPtr->write_message(reqData);
	}
	else
	{
		LOG_PRINT(log_warning, "[rejoin room]userid:%u,user ip:%s,nmobile:%d,mic roomid:%u,client connid:%u.roomsvr connid:%u is disconnected.", \
			userid, get_remote_ip(), clientItem()->m_mobile, clientItem()->m_micGroupID, conn_id(), pSvrConnPtr->getconnid());
	}
}

void CClientLogic::post_user_login(char * pRspData, unsigned int rspLen)
{
	if (!pRspData || !rspLen)
	{
		LOG_PRINT(log_error, "post user login fail,input is null.rspLen:%u.", rspLen);
		return;
	}

	CMDUserLogonSuccessResp oRspData;
	oRspData.ParseFromArray(pRspData, rspLen);
	unsigned int userid = oRspData.userid();

	if (userid && m_pClientHandler && m_pClientHandler->user_data)
	{
		m_pClientHandler->user_id = userid;
		CClientItem * item = (CClientItem *)m_pClientHandler->user_data;
		item->m_last_login_time = time(NULL);
		item->m_blogon = true;
		item->m_gender = oRspData.gender();

		byte nmobile = item->m_mobile;
		unsigned int nlogintime = item->m_last_login_time;
		unsigned int client_connid = m_pClientHandler->getconnid();
		//查询未读的群助手消息		handle_assist_unreadmsg();		//查询未读的通知消息
//		requestUnreadNoticeMsg();		//查询用户关注列表
//		qryUserAttentionList();
		CGateBase::m_ClientMgr.setuserconn(userid, nmobile, nlogintime, client_connid);
		//CGateBase::m_SvrConnMgr.post_user_login(userid, nmobile, nlogintime, client_connid, item->m_uuid);
/*		clienthandler_ptr pSvrConnPtr = CGateBase::m_SvrConnMgr.get_conn_inf(e_usermgrsvr_type, conn_id());
		if (pSvrConnPtr)
		{
			CMDLogonClientInfLst userInfo;
			CMDLogonClientInf * logoninf = userInfo.add_userinflst();
			logoninf->set_userid(userid);
			logoninf->set_mobile((unsigned int)nmobile);
			logoninf->set_logontime(nlogintime);
			logoninf->set_connid(client_connid);
			logoninf->set_uuid(item->m_uuid);
			sendRequestToServer(pSvrConnPtr, Sub_Vchat_LogonNot, userInfo, MDM_Vchat_Usermgr, false);
			LOG_PRINT(log_info, "post_user_login:client connid:%u,userid %u,nmobile:%d,logintime:%u,uuid:%s,usermgr[%s:%d] size:%u.", client_connid, userid, (int)nmobile, nlogintime, item->m_uuid.c_str(), pSvrConnPtr->getremote_ip(), (int)pSvrConnPtr->getremote_port(), userInfo.userinflst_size());
			return;
		}
*/
		LOG_PRINT(log_info, "[logon success]client connid:%u,%s:%u,userid %u,nmobile:%d,logintime:%u,uuid:%s,gender:%u.", \
			client_connid, get_remote_ip(), get_remote_port(), userid, (int)nmobile, nlogintime, item->m_uuid.c_str(), item->m_gender);
	}
}

void CClientLogic::requestUnreadNoticeMsg()
{
	CGateBase::m_SvrConnMgr.del_conn_inf_by_type(conn_id(), e_pushmsg_type);
	unsigned int problem_svr_connID = clientItem()->find_problem_svr_conn(e_pushmsg_type);
	char distributed_value[64] = {0};
	sprintf(distributed_value, "%u", user_id());

	//according user's svr_type,connect id,get server connection
	clienthandler_ptr pSvrConnPtr = CGateBase::m_SvrConnMgr.get_conn_inf(e_pushmsg_type, conn_id(), distributed_value, problem_svr_connID);
	if (!pSvrConnPtr)
	{
		LOG_PRINT(log_error, "cannot find server to request unread notice msg.conn_id:%u,userid:%u.", conn_id(), user_id());
		return;
	}
	else
	{
		CMDUnreadNoticeMsgReq req;
		req.set_userid(user_id());
		req.set_appversion(clientItem()->sAppVersion);
		req.set_osplatform(clientItem()->m_platformType);
		req.set_osversion(""/*TODO*/);

		sendRequestToServer(pSvrConnPtr, Sub_Vchat_UnreadNoticeMsgReq, req);
	}
}

void CClientLogic::qryUserAttentionList()
{
	clienthandler_ptr pSvrConnPtr = CGateBase::m_SvrConnMgr.get_conn_inf(e_usermgrsvr_type, conn_id());
	if (!pSvrConnPtr)
	{
		LOG_PRINT(log_error, "cannot find server to request unread notice msg.conn_id:%u,userid:%u.", conn_id(), user_id());
		return;
	}
	else
	{
		CMDQryUserAttentionList req;
		req.set_userid(user_id());
		sendRequestToServer(pSvrConnPtr, Sub_Vchat_QryUserAttentionList, req);
	}
}

void CClientLogic::handle_user_grouplst(char * pRspData, unsigned int rspLen)
{
	if (!pRspData || !rspLen)
	{
		LOG_PRINT(log_error, "handle_user_grouplst fail,input is null.rspLen:%u.", rspLen);
		return;
	}

	//use 64 by default,because protobuf data length will change when you fill data after this.
	int nMsgLen = SIZE_IVM_HEAD_TOTAL + LEN64;
	SL_ByteBuffer buff(nMsgLen);
	buff.data_end(nMsgLen);

	COM_MSG_HEADER * pOutMsg = (COM_MSG_HEADER *)buff.buffer();
	pOutMsg->maincmd = MDM_Vchat_Room;
	pOutMsg->subcmd = Sub_Vchat_UnreadGroupMsgReq;
	pOutMsg->checkcode = CHECKCODE;
	pOutMsg->version = MDM_Version_Value;

	ClientGateMask_t * pGateMask = (ClientGateMask_t *)(pOutMsg->content);
	memset(pGateMask, 0, SIZE_IVM_CLIENTGATE);

	genClientGateMask(pGateMask);

	CMDUserGroupInfo oRspData;
	oRspData.ParseFromArray(pRspData, rspLen);
	unsigned int group_size = oRspData.grouplst_size();
	for (int i = 0; i < group_size; ++i)
	{
		CMDGroupInfo groupInfo = oRspData.grouplst(i);
		//LOG_PRINT(log_debug, "client connid:%u,userid:%u add groupid:%u roleType:%u.", conn_id(), user_id(), groupInfo.groupid(), groupInfo.roletype());

		handle_group_unreadmsg(groupInfo.groupid(), buff, nMsgLen);

		CGateBase::m_RoomClientMgr.add_user_connid(groupInfo.groupid(), user_id(), conn_id());
		if (clientItem())
		{
			LOG_PRINT(log_debug, "[set group roletype]client connid:%u,userid:%u groupid:%u roleType:%u.", conn_id(), user_id(), groupInfo.groupid(), groupInfo.roletype());
			clientItem()->setGroupRole(groupInfo.groupid(), groupInfo.roletype());
		}
	}
}

void CClientLogic::handleUserKickOutMsg(char * pRspData, unsigned int rspLen)
{
	if (!pRspData || !rspLen)
	{
		LOG_PRINT(log_error, "handleUserKickOutMsg fail,input is null.rspLen:%u.", rspLen);
		return;
	}

	CClientItem * pItem = clientItem();
	if (!pItem)
	{
		return;
	}

	CMDUserKickOffLine rspData;
	rspData.ParseFromArray(pRspData, rspLen);
	std::string new_uuid = rspData.newsessionuuid();
	if (pItem->m_uuid != new_uuid)
	{
		LOG_PRINT(log_debug, "new session uuid:%s is not equal with this session uuid:%s.need to down mic.userid:%u,connid:%u.", \
			new_uuid.c_str(), pItem->m_uuid.c_str(), user_id(), conn_id());
		notify_svr_downmic();
	}
	else
	{
		LOG_PRINT(log_debug, "new session uuid:%s is equal with this session uuid.userid:%u,connid:%u.", new_uuid.c_str(), user_id(), conn_id());
		if ((clientItem()->m_micToUserid && clientItem()->m_micGroupID) || (clientItem()->m_invite_mic_runid && clientItem()->m_invite_mic_toid))
		{
			CMDTransferMicState oData;
			oData.set_micrunuserid(clientItem()->m_micRunUserid);
			oData.set_mictouserid(clientItem()->m_micToUserid);
			oData.set_micgroupid(clientItem()->m_micGroupID);
			oData.set_micindex(clientItem()->m_micindex);
			oData.set_micstate(clientItem()->m_micstate);
			oData.set_invitemicrunid(clientItem()->m_invite_mic_runid);
			oData.set_invitemictoid(clientItem()->m_invite_mic_toid);
			oData.set_invitemicsessionid(clientItem()->m_invite_mic_sessionid);
			oData.set_userid(user_id());

			unsigned int reqLen = SIZE_IVM_HEADER + oData.ByteSize();
			SL_ByteBuffer buff(reqLen);
			buff.data_end(reqLen);

			COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)buff.buffer();
			pHead->version = MDM_Version_Value;
			pHead->checkcode = CHECKCODE;
			pHead->maincmd = MDM_Vchat_Usermgr;
			pHead->subcmd = Sub_Vchat_MicStateTransNoty;
			pHead->length = reqLen;

			oData.SerializeToArray(pHead->content, oData.ByteSize());
			int retCount = CGateBase::m_SvrConnMgr.send_msg_to_service_type(buff, e_usermgrsvr_type);
			LOG_PRINT(log_debug, "transfer mic state to usermgr.uuid:%s,userid:%u,connid:%u.mic runid:%u,mic toid:%u,mic groupid:%u,invite mic runid:%u,invite mic toid:%u,send usermgr:%d.", \
				new_uuid.c_str(), user_id(), conn_id(), clientItem()->m_micRunUserid, clientItem()->m_micToUserid, clientItem()->m_micGroupID, clientItem()->m_invite_mic_runid, clientItem()->m_invite_mic_toid, retCount);

			pItem->reset_mic_state();
			pItem->reset_invite_mic_state();
		}

		pItem->m_valid = false;
	}
}

void CClientLogic::handleUserTransferMicState(char * pRspData, unsigned int rspLen)
{
	if (!pRspData || !rspLen)
	{
		LOG_PRINT(log_error, "handleUserTransferMicState fail,input is null.rspLen:%u.", rspLen);
		return;
	}

	CClientItem * pItem = clientItem();
	if (!pItem)
	{
		return;
	}

	CMDTransferMicState rspData;
	rspData.ParseFromArray(pRspData, rspLen);
	LOG_PRINT(log_debug, "transfer mic state to new session.uuid:%s,userid:%u,connid:%u.mic runid:%d,mic toid:%d,mic groupid:%d,mic state:%d,mic index:%d,invite mic runid:%u,invite mic toid:%u,invite mic session:%d.", \
		pItem->m_uuid.c_str(), user_id(), conn_id(), \
		rspData.micrunuserid(), rspData.mictouserid(), rspData.micgroupid(), rspData.micstate(), rspData.micindex(), \
		rspData.invitemicrunid(), rspData.invitemictoid(), rspData.invitemicsessionid());

	if (!clientItem()->m_micGroupID && rspData.micgroupid())
	{
		clientItem()->fill_mic_state(rspData.micrunuserid(), rspData.mictouserid(), rspData.micindex(), rspData.micstate(), rspData.micgroupid());
	}

	if (!clientItem()->m_invite_mic_toid && rspData.invitemictoid())
	{
		clientItem()->fill_invite_mic_state(rspData.invitemicrunid(), rspData.invitemictoid(), rspData.invitemicsessionid());
	}
}

void CClientLogic::handle_group_unreadmsg(unsigned int groupid, SL_ByteBuffer & buff, unsigned int msgLen)
{
	if (!groupid || !msgLen)
	{
		LOG_PRINT(log_error, "handle group unread msg input error.groupid:%u,msgLen:%u.", groupid, msgLen);
		return;
	}

	CGateBase::m_SvrConnMgr.del_conn_inf_by_type(conn_id(), e_roomadapter_type);
	char distributed_value[64] = {0};
	sprintf(distributed_value, "%u", groupid);

	//according user's svr_type,connect id,get server connection
	clienthandler_ptr pSvrConnPtr = CGateBase::m_SvrConnMgr.get_conn_inf(e_roomadapter_type, conn_id(), distributed_value);
	if (!pSvrConnPtr)
	{
		LOG_PRINT(log_error, "cannot find ChatSvr server to handle group unread msg.conn_id:%u,userid:%u,groupid:%u.", conn_id(), user_id(), groupid);
		return;
	}
	else
	{
		COM_MSG_HEADER * pOutMsg = (COM_MSG_HEADER *)buff.buffer();
		char * pReq = (char *)(pOutMsg->content + SIZE_IVM_CLIENTGATE);

		CMDUnreadGroupMsgReq reqData;
		reqData.set_userid(user_id());
		reqData.set_groupid(groupid);

		unsigned int realLen = reqData.ByteSize() + SIZE_IVM_HEAD_TOTAL;
		if (realLen > msgLen)
		{
			LOG_PRINT(log_warning, "Not need to send group unread msg request.real data length:%u,msg length:%u,conn_id:%u,userid:%u,groupid:%u.", \
				realLen, msgLen, conn_id(), user_id(), groupid);
			return;
		}

		reqData.SerializeToArray(pReq, reqData.ByteSize());
		pOutMsg->length = realLen;

		pSvrConnPtr->write_message(buff.data(), realLen);
		LOG_PRINT(log_debug, "[Request group unread msg]client connid:%u,userid:%u,distributed_value:%s,server:%s:%u.", \
			conn_id(), user_id(), distributed_value, pSvrConnPtr->getremote_ip(), pSvrConnPtr->getremote_port());
		return;
	}
}

void CClientLogic::handle_assist_unreadmsg()
{
	if (!user_id())
	{
		return;
	}

	CGateBase::m_SvrConnMgr.del_conn_inf_by_type(conn_id(), e_roomadapter_type);
	unsigned int problem_svr_connID = clientItem()->find_problem_svr_conn(e_roomadapter_type);
	char distributed_value[64] = {0};
	sprintf(distributed_value, "%u", user_id());

	//according user's svr_type,connect id,get server connection
	clienthandler_ptr pSvrConnPtr = CGateBase::m_SvrConnMgr.get_conn_inf(e_roomadapter_type, conn_id(), distributed_value, problem_svr_connID);
	if (!pSvrConnPtr)
	{
		LOG_PRINT(log_error, "cannot find server to handle assist unread request msg.conn_id:%u,userid:%u.", conn_id(), user_id());
		return;
	}
	else
	{
		CMDUnreadAssistMsgReq reqData;
		reqData.set_userid(user_id());

		sendRequestToServer(pSvrConnPtr, Sub_Vchat_UnreadGPAssistMsgReq, reqData);
		LOG_PRINT(log_debug, "[Request assist unread msg]client connid:%u,userid:%u,distributed_value:%s,server:%s:%u.", \
			conn_id(), user_id(), distributed_value, pSvrConnPtr->getremote_ip(), pSvrConnPtr->getremote_port());
		return;
	}
}

void CClientLogic::notify_svr_clientexit()
{
	notify_svr_clientclosesocket();

	notify_svr_downmic();

	notify_svr_exitroom(room_id(), user_id());
}

void CClientLogic::notify_svr_exceptexit()
{
	notify_svr_clientclosesocket();

	notify_svr_downmic();

	notify_svr_exceptexitroom(room_id(), user_id());
}

void CClientLogic::notify_svr_clienttimeout()
{
	notify_svr_clientclosesocket();

	notify_svr_downmic();

	notify_svr_kickoutroom(room_id(), user_id(), ERR_ROOM_KICKOUT_TIMEOUT);
}

void CClientLogic::notify_svr_downmic()
{
	if (user_id() == 0 || !clientItem())
	{
		return;
	}

	if (!clientItem()->m_valid)
	{
		LOG_PRINT(log_warning, "cannot notify svr down mic.this session is invalid.userid:%u,client connid:%u", user_id(), conn_id());
		return;
	}

	if (clientItem()->m_micToUserid && clientItem()->m_micGroupID && user_id() == clientItem()->m_micToUserid)
	{
		LOG_PRINT(log_warning, "client close socket and need to down mic[userid:%u,client connid:%u,micToUserID:%u,micIndex:%d,micGroupID:%u]", \
			user_id(), conn_id(), clientItem()->m_micToUserid, clientItem()->m_micindex, clientItem()->m_micGroupID);

		CGateBase::m_SvrConnMgr.del_conn_inf_by_type(conn_id(), e_roomsvr_type);
		char distributed_value[LEN32] = {0};
		sprintf(distributed_value, "%u", clientItem()->m_micGroupID);

		clienthandler_ptr pSvrConnPtr = CGateBase::m_SvrConnMgr.get_conn_inf(e_roomsvr_type, conn_id(), distributed_value);
		if (!pSvrConnPtr)
		{
			LOG_PRINT(log_error, "client close socket and need to down mic but cannot get roomsvr connection.");
			return;
		}

		CMDUserMicState msginfo;
		msginfo.set_vcbid(clientItem()->m_micGroupID);
		msginfo.mutable_runid()->set_userid(clientItem()->m_micRunUserid);
		msginfo.mutable_toid()->set_userid(clientItem()->m_micToUserid);
		msginfo.set_micindex(clientItem()->m_micindex);
		msginfo.set_micstate(0);

		unsigned int reqLen = SIZE_IVM_HEAD_TOTAL + msginfo.ByteSize();
		SL_ByteBuffer buff(reqLen);
		buff.data_end(reqLen);

		COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)buff.buffer();
		pHead->version = MDM_Version_Value;
		pHead->checkcode = CHECKCODE;
		pHead->maincmd = MDM_Vchat_Room;
		pHead->subcmd = Sub_Vchat_SetMicStateReq;
		pHead->length = reqLen;

		ClientGateMask_t * pClientGate = (ClientGateMask_t *)(pHead->content);
		memset(pClientGate, 0, SIZE_IVM_CLIENTGATE);
		pClientGate->param1 = (uint64)this;
		pClientGate->param2 = (uint64)conn_id();

		char * pContent = (char *)(pHead->content + SIZE_IVM_CLIENTGATE);
		msginfo.SerializeToArray(pContent, msginfo.ByteSize());

		pSvrConnPtr->write_message(buff);

		clientItem()->reset_mic_state();
	}

	if (clientItem()->m_invite_mic_runid && clientItem()->m_invite_mic_toid)
	{
		LOG_PRINT(log_warning, "client close socket and need to notice down invite mic[userid:%u,client connid:%u,mic invite runid:%u,mic invite toid:%u,sessionid:%d]", \
			user_id(), conn_id(), clientItem()->m_invite_mic_runid, clientItem()->m_invite_mic_toid, clientItem()->m_invite_mic_sessionid);

		CGateBase::m_SvrConnMgr.del_conn_inf_by_type(conn_id(), e_roomsvr_type);
		char distributed_value[LEN32] = {0};
		sprintf(distributed_value, "%u", clientItem()->m_invite_mic_toid);

		clienthandler_ptr pSvrConnPtr = CGateBase::m_SvrConnMgr.get_conn_inf(e_roomsvr_type, conn_id(), distributed_value);
		if (!pSvrConnPtr)
		{
			LOG_PRINT(log_error, "client close socket and need to down mic but cannot get roomsvr connection.");
			return;
		}

		CMDInviteOnMic msginfo;
		msginfo.set_runuserid(clientItem()->m_invite_mic_runid);
		msginfo.set_touserid(clientItem()->m_invite_mic_toid);
		msginfo.set_sessionid(clientItem()->m_invite_mic_sessionid);
		msginfo.set_optype(INVT_MIC_FORCE_HANGUP);

		unsigned int reqLen = SIZE_IVM_HEAD_TOTAL + msginfo.ByteSize();
		SL_ByteBuffer buff(reqLen);
		buff.data_end(reqLen);

		COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)buff.buffer();
		pHead->version = MDM_Version_Value;
		pHead->checkcode = CHECKCODE;
		pHead->maincmd = MDM_Vchat_Room;
		pHead->subcmd = Sub_Vchat_InviteOnMicReq;
		pHead->length = reqLen;

		ClientGateMask_t * pClientGate = (ClientGateMask_t *)(pHead->content);
		memset(pClientGate, 0, SIZE_IVM_CLIENTGATE);
		pClientGate->param1 = (uint64)this;
		pClientGate->param2 = (uint64)conn_id();

		char * pContent = (char *)(pHead->content + SIZE_IVM_CLIENTGATE);
		msginfo.SerializeToArray(pContent, msginfo.ByteSize());

		pSvrConnPtr->write_message(buff);

		clientItem()->reset_invite_mic_state();
	}
}

void CClientLogic::notify_svr_clientclosesocket()
{
	if (user_id() == 0)
		return;

	LOG_PRINT(log_warning, "client close socket[userid:%u,client connid:%u,%s:%d]", user_id(), conn_id(), get_remote_ip(), get_remote_port());

	clienthandler_ptr pSvrConnPtr = CGateBase::m_SvrConnMgr.get_conn_inf(e_logonsvr_type, conn_id());
	if (!pSvrConnPtr)
		return;

	CMDUserOnlineBaseInfoNoty_t exitNoty;
	exitNoty.userid = user_id();
	exitNoty.devicetype = (uint32)clientItem()->m_mobile;
	memset(exitNoty.cIpAddr, 0, IPADDRLEN);
	strcpy(exitNoty.cIpAddr, get_remote_ip());
	exitNoty.cIpAddr[IPADDRLEN - 1] = '\0';
	exitNoty.logontime = clientItem()->m_last_login_time;
	exitNoty.exittime = time(NULL);

	memset(exitNoty.uuid, 0, URLLEN);
	memcpy(exitNoty.uuid, clientItem()->m_uuid.c_str(), URLLEN);

	exitNoty.platformType = clientItem()->m_platformType;

	memset(exitNoty.cloginid, 0, LEN32);
	memcpy(exitNoty.cloginid, clientItem()->m_cloginid, LEN32);

	char szBuf[512] = {0};
	int nMsgLen = build_netmsg_svr(szBuf, sizeof(szBuf), MDM_Vchat_Login, Sub_Vchat_ClientCloseSocket_Req, &exitNoty, sizeof(CMDUserOnlineBaseInfoNoty_t));
	SL_ByteBuffer buff;
	buff.write(szBuf, nMsgLen);

	pSvrConnPtr->write_message(buff);
}

void CClientLogic::notify_svr_exceptexitroom(unsigned int roomid, unsigned int userid)
{
	if (!roomid || !userid)
	{
		return;
	}

	CMDUserExceptExitRoomInfo msginfo;
	msginfo.set_userid(userid);
	msginfo.set_vcbid(roomid);
	
	unsigned int reqLen = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + msginfo.ByteSize();
	SL_ByteBuffer buff(reqLen);
	buff.data_end(reqLen);

	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)buff.buffer();
	pHead->version = MDM_Version_Value;
	pHead->checkcode = CHECKCODE;
	pHead->maincmd = MDM_Vchat_Room;
	pHead->subcmd = Sub_Vchat_RoomUserExceptExitReq;
	pHead->length = reqLen;

	ClientGateMask_t * pClientGate = (ClientGateMask_t *)(pHead->content);
	memset(pClientGate, 0, SIZE_IVM_CLIENTGATE);
	pClientGate->param1 = (uint64)this;
	pClientGate->param2 = (uint64)conn_id();
	char * pContent = (char *)(pHead->content + SIZE_IVM_CLIENTGATE);
	msginfo.SerializeToArray(pContent, msginfo.ByteSize());

	clienthandler_ptr pSvrConnPtr = CGateBase::m_SvrConnMgr.get_conn_inf(e_roomsvr_type, conn_id());
	if (!pSvrConnPtr)
	{
		LOG_PRINT(log_error, "cannot get roomsvr connection to handle user except exit room.userid:%u,user ip:%s,nmobile:%d,roomid:%u,client connid:%u.", userid, get_remote_ip(), clientItem()->m_mobile, roomid, conn_id());
		return;
	}

	if (pSvrConnPtr->isconnected())
	{
		LOG_PRINT(log_info, "[except exit room]userid:%u,user ip:%s,nmobile:%d,roomid:%u,client connid:%u.", userid, get_remote_ip(), clientItem()->m_mobile, roomid, conn_id());
		pSvrConnPtr->write_message(buff);
	}
	else
	{
		LOG_PRINT(log_warning, "[except exit room]userid:%u,user ip:%s,nmobile:%d,roomid:%u,client connid:%u.roomsvr connid:%u is disconnected.", \
			userid, get_remote_ip(), clientItem()->m_mobile, roomid, conn_id(), pSvrConnPtr->getconnid());
	}

	clientItem()->m_exitroom = true;
}

void CClientLogic::notify_svr_exitroom(unsigned int roomid, unsigned int userid)
{
	if (!roomid || !userid)
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
	memset(pClientGate, 0, SIZE_IVM_CLIENTGATE);
	pClientGate->param1 = (uint64)this;
	pClientGate->param2 = (uint64)conn_id();
	char * pContent = (char *)(pHead->content + SIZE_IVM_CLIENTGATE);
	msginfo.SerializeToArray(pContent, msginfo.ByteSize());

	clienthandler_ptr pSvrConnPtr = CGateBase::m_SvrConnMgr.get_conn_inf(e_roomsvr_type, conn_id());
	if (!pSvrConnPtr)
	{
		LOG_PRINT(log_error, "cannot get roomsvr connection to handle user exit room.userid:%u,user ip:%s,nmobile:%d,roomid:%u,client connid:%u.", userid, get_remote_ip(), clientItem()->m_mobile, roomid, conn_id());
		return;
	}

	if (pSvrConnPtr->isconnected())
	{
		LOG_PRINT(log_info, "[exit room]userid:%u,user ip:%s,nmobile:%d,roomid:%u,client connid:%u.roomsvr:%s:%u.", \
			userid, get_remote_ip(), clientItem()->m_mobile, roomid, conn_id(), pSvrConnPtr->getremote_ip(), pSvrConnPtr->getremote_port());
		pSvrConnPtr->write_message(buff);
	}

	clientItem()->m_exitroom = true;
}

void CClientLogic::notify_svr_kickoutroom(unsigned int roomid, unsigned int userid, int reasonid)
{
	if (!roomid || !userid)
	{
		return;
	}

	CMDUserKickoutRoomInfo msgInfo;
	msgInfo.set_vcbid(roomid);
	msgInfo.set_srcid(0);
	msgInfo.set_toid(userid);
	msgInfo.set_resonid(reasonid);

	unsigned int reqLen = SIZE_IVM_HEAD_TOTAL + msgInfo.ByteSize();
	SL_ByteBuffer buff(reqLen);
	buff.data_end(reqLen);

	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)buff.buffer();
	pHead->version = MDM_Version_Value;
	pHead->checkcode = CHECKCODE;
	pHead->maincmd = MDM_Vchat_Room;
	pHead->subcmd = Sub_Vchat_GatewayKickoutUserReq;
	pHead->length = reqLen;

	ClientGateMask_t * pClientGate = (ClientGateMask_t *)(pHead->content);
	memset(pClientGate, 0, SIZE_IVM_CLIENTGATE);
	pClientGate->param1 = (uint64)this;
	pClientGate->param2 = (uint64)conn_id();
	char * pContent = (char *)(pHead->content + SIZE_IVM_CLIENTGATE);
	msgInfo.SerializeToArray(pContent, msgInfo.ByteSize());

	clienthandler_ptr pSvrConnPtr = CGateBase::m_SvrConnMgr.get_conn_inf(e_roomsvr_type, conn_id());
	if (!pSvrConnPtr)
	{
		LOG_PRINT(log_error, "cannot get roomsvr connection to kick out room.userid:%u,user ip:%s,nmobile:%d,roomid:%u,client connid:%u.", userid, get_remote_ip(), clientItem()->m_mobile, roomid, conn_id());
		return;
	}

	if (pSvrConnPtr->isconnected())
	{
		LOG_PRINT(log_info, "[kick out room]userid:%u,user ip:%s,nmobile:%d,roomid:%u,client connid:%u.roomsvr:%s:%u.", \
			userid, get_remote_ip(), clientItem()->m_mobile, roomid, conn_id(), pSvrConnPtr->getremote_ip(), pSvrConnPtr->getremote_port());
		pSvrConnPtr->write_message(buff);
	}

	clientItem()->m_exitroom = true;
}

std::string CClientLogic::get_distributed_key_msg(char * in_msg)
{
	char distributed_key[128] = {0};

	COM_MSG_HEADER * req = (COM_MSG_HEADER *)in_msg;
	unsigned int reqLen = SIZE_IVM_NOMASK_REQUEST(req);
	char * pReqData = (char *)(req->content);

	switch(req->subcmd)
	{
	////////////////////////////////charsvr//////////////////////////////////////////////////
	case Sub_Vchat_GroupMsgReq:
		{
			CMDGroupMsgReq oReqData;
			oReqData.ParseFromArray(pReqData, reqLen);
			sprintf(distributed_key, "%u", oReqData.groupid());
		}
		break;
	case Sub_Vchat_GroupMsgNotifyRecv:
		{
			CMDGroupMsgNotifyRecv oReqData;
			oReqData.ParseFromArray(pReqData, reqLen);
			sprintf(distributed_key, "%u", oReqData.groupid());
		}
		break;
	case Sub_Vchat_GroupPrivateMsgReq:
		{
			CMDGroupPrivateMsgReq oReqData;
			oReqData.ParseFromArray(pReqData, reqLen);
			sprintf(distributed_key, "%u", oReqData.groupid());
		}
		break;
	case Sub_Vchat_GroupPrivateMsgNotifyRecv:
		{
			CMDGroupPrivateMsgNotifyRecv oReqData;
			oReqData.ParseFromArray(pReqData, reqLen);
			sprintf(distributed_key, "%u", oReqData.groupid());
		}
		break;
	case Sub_Vchat_UnreadGroupMsgReq:
		{
			CMDUnreadGroupMsgReq oReqData;
			oReqData.ParseFromArray(pReqData, reqLen);
			sprintf(distributed_key, "%u", oReqData.groupid());
		}
		break;

	//consumesvr
	case Sub_Vchat_SendRedPacketReq:
		{
			CMDSendRedPacketReq reqData;
			reqData.ParseFromArray(pReqData, reqLen);
			sprintf(distributed_key, "%u", reqData.groupid());
		}
		break;
	case Sub_Vchat_CatchRedPacketReq:
		{
			CMDCatchRedPacketReq reqData;
			reqData.ParseFromArray(pReqData, reqLen);
			sprintf(distributed_key, "%u", reqData.groupid());
		}
		break;
	case Sub_Vchat_TakeRedPacketReq:
		{
			CMDTakeRedPacketReq reqData;
			reqData.ParseFromArray(pReqData, reqLen);
			sprintf(distributed_key, "%u", reqData.groupid());
		}
		break;
	case Sub_Vchat_QryRedPacketInfoReq:
		{
			CMDQryRedPacketInfoReq reqData;
			reqData.ParseFromArray(pReqData, reqLen);
			sprintf(distributed_key, "%u", reqData.groupid());
		}
		break;

	//roomsvr
	case Sub_Vchat_JoinRoomReq:
		{
			CMDJoinRoomReq reqData;
			reqData.ParseFromArray(pReqData, reqLen);
			sprintf(distributed_key, "%u", reqData.vcbid());
		}
		break;
	case Sub_Vchat_RoomUserExitReq:
		{
			CMDUserExitRoomInfo reqData;
			reqData.ParseFromArray(pReqData, reqLen);
			sprintf(distributed_key, "%u", reqData.vcbid());
		}
		break;
	case Sub_Vchat_SetUserPriorityReq:
		{
			CMDSetUserPriorityReq reqData;
			reqData.ParseFromArray(pReqData, reqLen);
			sprintf(distributed_key, "%u", reqData.groupid());
		}
		break;
	case Sub_Vchat_QuitGroupReq:
		{
			CMDQuitGroupReq reqData;
			reqData.ParseFromArray(pReqData, reqLen);
			sprintf(distributed_key, "%u", reqData.groupid());
		}
		break;
	case Sub_Vchat_RoomKickoutUserReq:
		{
			CMDUserKickoutRoomInfo reqData;
			reqData.ParseFromArray(pReqData, reqLen);
			sprintf(distributed_key, "%u", reqData.vcbid());
		}
		break;
	default:
		break;
	}

	return std::string(distributed_key);
}

void CClientLogic::print_specail_cmd(unsigned int subcmd, const std::string & distributed_value, const std::string & svr_ip, unsigned int svr_port)
{
	//print the server inform.
	switch(subcmd)
	{
	case Sub_Vchat_UnreadGroupMsgReq:
		{
			LOG_PRINT(log_info, "[client request][Request group unread msg]client connid:%u,userid:%u,distributed_value:%s,server:%s:%u.", \
				conn_id(), user_id(), distributed_value.c_str(), svr_ip.c_str(), svr_port);
		}
		break;

	//consumesvr
	case Sub_Vchat_SendRedPacketReq:
		{
			LOG_PRINT(log_info, "[client request][Request send red packet]client connid:%u,userid:%u,distributed_value[groupid]:%s,server:%s:%u.", \
				conn_id(), user_id(), distributed_value.c_str(), svr_ip.c_str(), svr_port);
		}
		break;
	case Sub_Vchat_CatchRedPacketReq:
		{
			LOG_PRINT(log_info, "[client request][Request catch red packet]client connid:%u,userid:%u,distributed_value[groupid]:%s,server:%s:%u.", \
				conn_id(), user_id(), distributed_value.c_str(), svr_ip.c_str(), svr_port);
		}
		break;
	case Sub_Vchat_TakeRedPacketReq:
		{
			LOG_PRINT(log_info, "[client request][Request take red packet]client connid:%u,userid:%u,distributed_value[groupid]:%s,server:%s:%u.", \
				conn_id(), user_id(), distributed_value.c_str(), svr_ip.c_str(), svr_port);
		}
		break;
	case Sub_Vchat_QryRedPacketInfoReq:
		{
			LOG_PRINT(log_info, "[client request][Request query red packet info]client connid:%u,userid:%u,distributed_value[groupid]:%s,server:%s:%u.", \
				conn_id(), user_id(), distributed_value.c_str(), svr_ip.c_str(), svr_port);
		}
		break;

	//roomsvr
	case Sub_Vchat_SetUserPriorityReq:
		{
			LOG_PRINT(log_info, "[client request][Request set user priority]client connid:%u,run_userid:%u,distributed_value[groupid]:%s,server:%s:%u.", \
				conn_id(), user_id(), distributed_value.c_str(), svr_ip.c_str(), svr_port);
		}
		break;
	default:
		break;
	}
	return;
}

void CClientLogic::remove_svr_connect_map(char * in_msg)
{
	COM_MSG_HEADER * req = (COM_MSG_HEADER *)in_msg;
	unsigned int subcmd = req->subcmd;
	if (Sub_Vchat_JoinRoomReq == subcmd || Sub_Vchat_SetMicStateReq == subcmd || Sub_Vchat_RoomUserExitReq == subcmd || Sub_Vchat_SetUserPriorityReq == subcmd || Sub_Vchat_QuitGroupReq == subcmd ||
		Sub_Vchat_RoomKickoutUserReq == subcmd)
	{
		CGateBase::m_SvrConnMgr.del_conn_inf_by_type(conn_id(), e_roomsvr_type);
	}

	if (Sub_Vchat_UnreadGroupMsgReq == subcmd)
	{
		CGateBase::m_SvrConnMgr.del_conn_inf_by_type(conn_id(), e_roomadapter_type);
	}

	if (Sub_Vchat_SendRedPacketReq == subcmd || Sub_Vchat_CatchRedPacketReq == subcmd || Sub_Vchat_TakeRedPacketReq == subcmd || Sub_Vchat_QryRedPacketInfoReq == subcmd)
	{
		CGateBase::m_SvrConnMgr.del_conn_inf_by_type(conn_id(), e_consumesvr_type);
	}
}

void CClientLogic::genClientGateMask(ClientGateMask_t *pGateMask)
{
	pGateMask->param1 = (uint64)m_pClientHandler.get();
	pGateMask->param2 = (uint64)conn_id();
	pGateMask->param4 = clientItem()->m_mobile;
	pGateMask->param5 = ntohl(inet_addr(get_remote_ip()));
	pGateMask->param6 = get_remote_port();
}
