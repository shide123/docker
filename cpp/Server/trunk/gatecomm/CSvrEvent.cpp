/*
 * CSvrEvent.cpp
 *
 *  Created on: Aug 22, 2016
 *      Author: testtest
 */

#include "CSvrEvent.h"
#include "GateBase.h"
#include "CClientItem.h"
#include "CClientLogic.h"
#include "utils.h"
#include "json/json.h"
#include "LogonSvr.pb.h"
#include "RoomSvr.pb.h"
#include "UserMgrSvr.pb.h"
#include "msgcommapi.h"

CSvrEvent::CSvrEvent()
{
}

CSvrEvent::~CSvrEvent()
{
}


bool CSvrEvent::onMessage(task_proc_data * message)
{
	if (NULL == message || !message->connection)
	{
		return false;
	}

	bool bRet = (0 == handle_message(message->pdata, message->datalen, message->connection));

	return bRet;
}

bool CSvrEvent::onConnect(clienthandler_ptr svr_conn)
{
	if (svr_conn)
	{
		if (svr_conn->user_data)
		{
			CSvrItem * pItem = (CSvrItem *)svr_conn->user_data;
			Timer::getInstance()->del(pItem->m_timeid);

			delete svr_conn->user_data;
			svr_conn->user_data = NULL;
		}

		svr_conn->user_data = new CSvrItem();

		CSvrItem * item = (CSvrItem *)svr_conn->user_data;
		item->m_procname = CGateBase::change_type_to_svrname(svr_conn->getsvrtype());

		send_hello_msg(svr_conn);

		item->m_timeid = Timer::getInstance()->add(5, boost::bind(&CSvrEvent::onTimer, this, svr_conn), TIMES_UNLIMIT);

		CGateBase::m_SvrConnMgr.add_svr_node(svr_conn->getsvrtype(), conn_id(svr_conn), get_remote_ip(svr_conn), get_remote_port(svr_conn));

		afterConnect(svr_conn);
	}
	return true;
}

bool CSvrEvent::onClose(clienthandler_ptr svr_conn)
{
	if (svr_conn && svr_conn->user_data)
	{
		LOG_PRINT(log_info, "server onClose[%s] connid:%u,%s:%u.", get_svr_name(svr_conn).c_str(), conn_id(svr_conn), get_remote_ip(svr_conn), get_remote_port(svr_conn));
		CGateBase::m_SvrConnMgr.del_svr_node(svr_conn->getsvrtype(), conn_id(svr_conn), get_remote_ip(svr_conn), get_remote_port(svr_conn));
		del_req_check_by_connid(svr_conn);
		post_close_process(svr_conn);

		CSvrItem * pItem = (CSvrItem *)svr_conn->user_data;
		Timer::getInstance()->del(pItem->m_timeid);

		delete svr_conn->user_data;
		svr_conn->user_data = NULL;
	}

	return true;
}

void CSvrEvent::send_hello_msg(clienthandler_ptr svr_conn)
{
	if (!svr_conn)
	{
		LOG_PRINT(log_error, "send hello msg fail.svr_conn is null.");
		return;
	}

	SL_ByteBuffer outbuf(sizeof(COM_MSG_HEADER) + sizeof(CMDGateHello_t));
	COM_MSG_HEADER * pmsgheader = (COM_MSG_HEADER *)outbuf.buffer();
	pmsgheader->version = MDM_Version_Value;
	pmsgheader->checkcode = CHECKCODE;
	pmsgheader->maincmd = MDM_Vchat_Room;
	pmsgheader->subcmd = Sub_Vchat_ClientHello;

	CMDGateHello_t * preq = (CMDGateHello_t *)(pmsgheader->content);
	preq->param1 = 12;
	preq->param2 = 8;
	preq->param3 = 7;
	preq->param4 = e_gateway_type;
	preq->gateid = svr_conn->getgateid();
	pmsgheader->length = sizeof(COM_MSG_HEADER) + sizeof(CMDGateHello_t);
	outbuf.data_end(pmsgheader->length);
	svr_conn->write_message(outbuf, true);

	LOG_PRINT(log_info, "send hello msg to server,gateid:%u,server[%s]:%s:%u.", svr_conn->getgateid(), get_svr_name(svr_conn).c_str(), get_remote_ip(svr_conn), get_remote_port(svr_conn));
}

bool CSvrEvent::onTimer(clienthandler_ptr svr_conn)
{
	if (svr_conn && svr_conn.get()->isconnected())
	{
		send_ping_msg(svr_conn);

		time_t now_time = time(NULL);
		if (now_time - get_pingtime(svr_conn) > CGateBase::m_uServerTimeout)
		{
			std::string ipaddr = get_remote_ip(svr_conn);
			unsigned int port = get_remote_port(svr_conn);
			LOG_PRINT(log_error, "Network between gate and %s:%s:%u has problem.server timeout:%u.need to reset the connection.", get_svr_name(svr_conn).c_str(), ipaddr.c_str(), port, CGateBase::m_uServerTimeout);
			svr_conn->release();
		}
	}

	return true;
}

void CSvrEvent::send_ping_msg(clienthandler_ptr svr_conn)
{
	if (!svr_conn)
	{
		LOG_PRINT(log_error, "CSvrEvent send ping msg fail.svr_conn is null.");
		return;
	}

	SL_ByteBuffer outbuf(sizeof(COM_MSG_HEADER) + sizeof(CMDClientPing_t));
	COM_MSG_HEADER * pingReq = (COM_MSG_HEADER *)outbuf.buffer();
	pingReq->version = MDM_Version_Value;
	pingReq->checkcode = CHECKCODE;
	pingReq->maincmd = MDM_Vchat_Room;
	pingReq->subcmd = Sub_Vchat_ClientPing;

	CMDClientPing_t * pReq = (CMDClientPing_t *)pingReq->content;
	memset(pReq, 0, sizeof(CMDClientPing_t));
	pingReq->length = sizeof(COM_MSG_HEADER) + sizeof(CMDClientPing_t);

	outbuf.data_end(pingReq->length);
	svr_conn->write_message(outbuf, true);
    LOG_PRINT(log_debug, "[send ping msg]server[%s]:%s:%u.", get_svr_name(svr_conn).c_str(), get_remote_ip(svr_conn), get_remote_port(svr_conn));
}

void CSvrEvent::handle_pingrsp_msg(const char * msg, int len, clienthandler_ptr svr_conn)
{
	DEF_IVM_HEADER(in_msg, msg);
	//LOG_PRINT(log_debug, "[handle_pingrsp_msg]server[%s]:%s:%u.", get_svr_name(svr_conn).c_str(), get_remote_ip(svr_conn), get_remote_port(svr_conn));
	if (in_msg->length == SIZE_IVM_HEADER + sizeof(CMDClientPingResp_t))
	{
		set_pingtime(svr_conn);
	}
}

int CSvrEvent::handle_message(const char * msg, int len, clienthandler_ptr svr_conn)
{
	if (msg == NULL || len <  SIZE_IVM_HEADER)
	{
		LOG_PRINT(log_error, "CSvrEvent handle_message input len from server err!server[%s]:%s:%u.", get_svr_name(svr_conn).c_str(), get_remote_ip(svr_conn), get_remote_port(svr_conn));
		return 0;
	}

	DEF_IVM_HEADER(in_msg, msg);
	if(in_msg->length != len) 
	{
		LOG_PRINT(log_warning,  "CSvrEvent handle_message from server in_msg length error!server[%s]:%s:%u.", get_svr_name(svr_conn).c_str(), get_remote_ip(svr_conn), get_remote_port(svr_conn));
		return 0;
	}

	if (Sub_Vchat_ClientHelloResp == in_msg->subcmd)
	{
		handle_register_msg(msg, len, svr_conn);
		return 0;
	}

	if (Sub_Vchat_ClientPingResp == in_msg->subcmd)
	{
		handle_pingrsp_msg(msg, len, svr_conn);
		return 0;
	}

	if (len <  SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE)
	{
		LOG_PRINT(log_error, "CSvrEvent handle_message input len from server err!server[%s]:%s:%u.", get_svr_name(svr_conn).c_str(), get_remote_ip(svr_conn), get_remote_port(svr_conn));
		return 0;
	}

	set_pingtime(svr_conn);

	print_specail_cmd(msg, len, svr_conn);

	DEF_IVM_CLIENTGATE(pGateMask, msg);
	if (pGateMask->param3 == pGateMask->param4)
	{
		switch (pGateMask->param3)
		{
		case BROADCAST_TYPE:
			{
				handle_broadcast_user_msg(msg, len, svr_conn);
				return 0;
			}
		case MULTICAST_TYPE:
			{
				handle_multicast_user_msg(msg, len, svr_conn);
				return 0;
			}
		case CAST_CLIENTS_SUBSCRIBE:
			{
				handle_broadcast_subscribe_msg(msg, len, svr_conn);
				return 0;
			}
		case BROADCAST_ROOM_TYPE:
			{
				handle_broadcast_room_msg(msg, len, svr_conn);
				return 0;
			}
		case BROADCAST_ROOM_ROLETYPE:
			{
				handle_broadcast_room_roletype(msg, len, svr_conn);
				return 0;
			}
		case CAST_CLIENTS_ON_ONE_SVR:
			{
				handle_cast_clients_on_one_svr(msg, len, svr_conn);
				return 0;
			}
		case CAST_APP_VERSION:
			{
				handle_broadcast_app_version(msg, len, svr_conn);
				return 0;
			}
		case BROADCAST_ROOM_GENDER:
			{
				handle_broadcast_room_gender_msg(msg, len, svr_conn);
				return 0;
			}
		}
	}

	return handle_logic_msg(msg, len, svr_conn);
}

void CSvrEvent::handle_register_msg(const char * msg, int len, clienthandler_ptr svr_conn)
{
	if (!msg || !len || !svr_conn)
	{
		return;
	}

	DEF_IVM_HEADER(in_msg, msg);
	std::string register_data(in_msg->content, len - SIZE_IVM_HEADER);
	if (register_data.empty())
	{
		LOG_PRINT(log_warning, "server[%s]:%s:%u register data is empty.", get_svr_name(svr_conn).c_str(), get_remote_ip(svr_conn), get_remote_port(svr_conn));
		return;
	}

	std::string strCmdlst;
	std::list<std::string> cmdRangeLst;

	std::list<std::string> cmdconfigLst;
	splitStrToLst(register_data, '&', cmdconfigLst);

	std::list<std::string>::iterator iter_cmd = cmdconfigLst.begin();
	for (; iter_cmd != cmdconfigLst.end(); ++iter_cmd)
	{
		std::string & cmdconfig = *iter_cmd;
		if (std::string::npos != cmdconfig.find("cmdlist="))
		{
			std::size_t index = strlen("cmdlist=");
			strCmdlst = cmdconfig.substr(index);
			LOG_PRINT(log_info, "server[%s]:%s:%u register cmdlist:%s.", get_svr_name(svr_conn).c_str(), get_remote_ip(svr_conn), get_remote_port(svr_conn), strCmdlst.c_str());
		}

		if (std::string::npos != cmdconfig.find("cmdrange_"))
		{
			std::size_t index = cmdconfig.find("=");
			if (std::string::npos != index)
			{
				std::string strCmdRange = cmdconfig.substr(index + 1);
				cmdRangeLst.push_back(strCmdRange);
				LOG_PRINT(log_info, "server[%s]:%s:%u register cmdrange:%s.", get_svr_name(svr_conn).c_str(), get_remote_ip(svr_conn), get_remote_port(svr_conn), strCmdRange.c_str());
			}
		}
	}
	
	if (strCmdlst.empty() && cmdRangeLst.empty())
	{
		LOG_PRINT(log_warning, "server[%s]:%s:%u register data is empty.", get_svr_name(svr_conn).c_str(), get_remote_ip(svr_conn), get_remote_port(svr_conn));
		return;
	}

	CGateBase::m_pCmdConfigMgr->addCmdConfig(get_svr_name(svr_conn), svr_conn->getsvrtype(), strCmdlst, cmdRangeLst);

	return;
}

int CSvrEvent::handle_logic_msg(const char * msg, int len, clienthandler_ptr svr_conn)
{
	DEF_IVM_HEADER(in_msg, msg);
	DEF_IVM_CLIENTGATE(pGateMask, msg);
	char * pData = (char *)DEF_IVM_DATA(msg);
	unsigned int nMsgLen2 = in_msg->length - SIZE_IVM_CLIENTGATE;
	unsigned int nDataLen2 = in_msg->length - SIZE_IVM_HEADER - SIZE_IVM_CLIENTGATE;

	//转发消息
	unsigned int client_connid = pGateMask->param2;
	//应该通过 再次 查找 一下该 conn 是否存在
	//直接使用变量进行 判断的原因 是防止 部分野指针 直接调用函数 引起的 奔溃
	clienthandler_ptr pClientConn = CGateBase::m_ClientMgr.find_client_map(client_connid);
	if(!pClientConn)
	{
		//当收到logonsvr的响应后，客户端已经关闭
		LOG_PRINT(log_warning, "client fd is closed,client connid:%u,maincmd:%u,subcmd:%u.server:%s:%u.", \
			client_connid, in_msg->maincmd, in_msg->subcmd, get_remote_ip(svr_conn), get_remote_port(svr_conn));
		return 0;
	}
	else if(pClientConn->isconnected())
	{
		if (Sub_Vchat_logonSuccess2 == in_msg->subcmd)
		{
			CClientLogic logic(pClientConn);
			logic.post_user_login(pData, nDataLen2);
		}

		if (Sub_Vchat_UserGroupLstResp == in_msg->subcmd)
		{
			CClientLogic logic(pClientConn);
			logic.handle_user_grouplst(pData, nDataLen2);
		}

		if (Sub_Vchat_ClientExistNot == in_msg->subcmd)
		{
			//kick out old user,print log.
			LOG_PRINT(log_warning, "user has been kicked out,userid:%u,connid:%u.", pClientConn->user_id, client_connid);
			CClientLogic logic(pClientConn);
			logic.handleUserKickOutMsg(pData, nDataLen2);
		}

		if (Sub_Vchat_MicStateTransNoty == in_msg->subcmd)
		{
			CClientLogic logic(pClientConn);
			logic.handleUserTransferMicState(pData, nDataLen2);
			return 0;
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

void CSvrEvent::handle_broadcast_user_msg(const char * msg, int len, clienthandler_ptr svr_conn)
{
	if (!msg || !len || !svr_conn)
	{
		return;
	}

	DEF_IVM_HEADER(in_msg, msg);
	DEF_IVM_CLIENTGATE(pGateMask, msg);
	char * pData = (char *)DEF_IVM_DATA(msg);
	int nMsgLen2 = in_msg->length - SIZE_IVM_CLIENTGATE;
	int nDataLen2 = in_msg->length - SIZE_IVM_HEADER - SIZE_IVM_CLIENTGATE;
	unsigned int maincmd = in_msg->maincmd;
	unsigned int subcmd = in_msg->subcmd;

	SL_ByteBuffer buff(nMsgLen2);
	COM_MSG_HEADER * pOutMsg = (COM_MSG_HEADER *)buff.buffer();
	memcpy(pOutMsg, in_msg, SIZE_IVM_HEADER);
	memcpy(pOutMsg->content, pData, nDataLen2);
	pOutMsg->length = nMsgLen2;
	buff.data_end(pOutMsg->length);

	/*format: broadcast all connects online
	client devtype == pGateMask->param1 && 
	 except_connid == pGateMask->param2 && 
	BROADCAST_TYPE == pGateMask->param3 && 
	BROADCAST_TYPE == pGateMask->param4 &&
			userid == pGateMask->param5
	*/

	e_NoticeDevType devtype = e_NoticeDevType(pGateMask->param1);
	unsigned int except_connid = pGateMask->param2;
	unsigned int userid = pGateMask->param5;

	if (0 == userid)
	{
		//means online users
		CGateBase::m_ClientMgr.broadcast_all_client((byte)devtype, buff, except_connid);
		return;
	}
	else
	{
		//focus on userid
		if (subcmd == Sub_Vchat_logonTokenNotify) 
		{
			CMDSessionTokenResp respData;
			respData.ParseFromArray(pOutMsg->content, nDataLen2);
			LOG_PRINT(log_debug, "Broadcast user token:userid:%u,subcmd:%u,token:%s,validtime:%s from server:%s:%u.", \
				userid, subcmd, respData.sessiontoken().c_str(), respData.validtime().c_str(), get_remote_ip(svr_conn), get_remote_port(svr_conn));
		}
		else
		{
			LOG_PRINT(log_debug, "Broadcast all connects of this user from server:%s:%u,userid:%u,maincmd:%u,subcmd:%u.", get_remote_ip(svr_conn), get_remote_port(svr_conn), userid, maincmd, subcmd);
		}

		CGateBase::m_ClientMgr.broadcast_user_all_connects(userid, buff, devtype, except_connid);

		if (subcmd == Sub_Vchat_InviteOnMicNoty)
		{
			CMDInviteOnMic oNoty;
			oNoty.ParseFromArray(pOutMsg->content, nDataLen2);
			LOG_PRINT(log_debug, "Broadcast invite on mic notify:userid:%u,runnid:%u,touserid:%u,optype:%d,errCode:%d,sessionid:%d from server:%s:%u.", \
				userid, oNoty.runuserid(), oNoty.touserid(), oNoty.optype(), oNoty.errinfo().errid(), oNoty.sessionid(), get_remote_ip(svr_conn), get_remote_port(svr_conn));

			if (oNoty.errinfo().errid() == 0)
			{
				handleInviteOnMicState(oNoty.runuserid(), oNoty.touserid(), oNoty.optype(), oNoty.sessionid());
			}
		}
	}	
}

void CSvrEvent::handle_multicast_user_msg(const char * msg, int len, clienthandler_ptr svr_conn)
{
	if (!msg || !len || !svr_conn)
	{
		return;
	}

	DEF_IVM_HEADER(in_msg, msg);
	DEF_IVM_CLIENTGATE(pGateMask, msg);
	char * pData = (char *)DEF_IVM_DATA(msg);
	int nMsgLen2 = in_msg->length - SIZE_IVM_CLIENTGATE;
	int nDataLen2 = in_msg->length - SIZE_IVM_HEADER - SIZE_IVM_CLIENTGATE;
	unsigned int maincmd = in_msg->maincmd;
	unsigned int subcmd = in_msg->subcmd;

	/*format: multicast all connects online
	client devtype == pGateMask->param1 &&
	 except_connid == pGateMask->param2 &&
	MULTICAST_TYPE == pGateMask->param3 &&
	MULTICAST_TYPE == pGateMask->param4 &&
			userid == pGateMask->param5
	*/

	e_NoticeDevType devtype = e_NoticeDevType(pGateMask->param1);
	unsigned int except_connid = pGateMask->param2;

	try
	{
		Json::Reader reader(Json::Features::strictMode());
		Json::Value root;
		char * pJson = pData + sizeof(MsgPushNotify_t);
		if (!reader.parse(pJson, root))
		{
			LOG_PRINT(log_error, "invalid Json format: %s", pJson);
			return;
		}

		SL_ByteBuffer buff(nMsgLen2);
		COM_MSG_HEADER * pOutMsg = (COM_MSG_HEADER *)buff.buffer();
		memcpy(pOutMsg, in_msg, SIZE_IVM_HEADER);
		memcpy(pOutMsg->content, pData, nDataLen2);
		pOutMsg->length = nMsgLen2;
		buff.data_end(pOutMsg->length);

		if (!root.isMember("user"))
		{
			CGateBase::m_ClientMgr.broadcast_all_client((byte)devtype, buff, except_connid);
			return;
		}

		Json::Value user = root["user"];

		root.removeMember("user");
		Json::FastWriter writer;
		std::string jsonoutput = writer.write(root);
		memcpy(pOutMsg->content + sizeof(MsgPushNotify_t), jsonoutput.c_str(), jsonoutput.size());
		pOutMsg->length = SIZE_IVM_HEADER + sizeof(MsgPushNotify_t) + jsonoutput.size();
		buff.data_end(pOutMsg->length);

		if (0 == user.size())
		{
			CGateBase::m_ClientMgr.broadcast_all_client((byte)devtype, buff, except_connid);
			return;
		}

		for(unsigned int i = 0; i < user.size(); ++i) 
		{
			uint32 userid = atoi(user[i].asString().c_str());
			//focus on userid
			LOG_PRINT(log_debug, "Broadcast all connects of this user from server:%s:%u,userid:%u,maincmd:%u,subcmd:%u.", get_remote_ip(svr_conn), get_remote_port(svr_conn), userid, maincmd, subcmd);
			if (!userid)
			{
				continue;
			}

			CGateBase::m_ClientMgr.broadcast_user_all_connects(userid, buff, devtype, except_connid);
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

void CSvrEvent::handle_broadcast_room_msg(const char * msg, int len, clienthandler_ptr svr_conn)
{
	if (!msg || !len || !svr_conn)
	{
		return;
	}

	DEF_IVM_HEADER(in_msg, msg);
	DEF_IVM_CLIENTGATE(pGateMask, msg);
	char * pData = (char *)DEF_IVM_DATA(msg);
	int nMsgLen2 = in_msg->length - SIZE_IVM_CLIENTGATE;
	int nDataLen2 = in_msg->length - SIZE_IVM_HEADER - SIZE_IVM_CLIENTGATE;
	unsigned int maincmd = in_msg->maincmd;
	unsigned int subcmd = in_msg->subcmd;

	SL_ByteBuffer buff(nMsgLen2);
	COM_MSG_HEADER * pOutMsg = (COM_MSG_HEADER *)buff.buffer();
	memcpy(pOutMsg, in_msg, SIZE_IVM_HEADER);
	memcpy(pOutMsg->content, pData, nDataLen2);
	pOutMsg->length = nMsgLen2;
	buff.data_end(pOutMsg->length);

	/* format:
	notice devtype == pGateMask->param1 && 
			connid == pGateMask->param2 && 
	BROADCAST_TYPE == pGateMask->param3 && 
	BROADCAST_TYPE == pGateMask->param4 &&
			roomid == pGateMask->param5
	*/

	e_NoticeDevType e_dev_type = e_NoticeDevType(pGateMask->param1);
	unsigned int connid = pGateMask->param2;
	unsigned int roomid = pGateMask->param5;
	bool castInGroupOnly = pGateMask->param6;

	if(Sub_Vchat_RoomKickoutUserNoty == in_msg->subcmd)
	{
		//kick out need to save connid to call del_user_connid but need to reset param2 to broadcast all connects in one room.
		pGateMask->param2 = 0;
	}

	if (roomid)
	{
		LOG_PRINT(log_debug, "Broadcast msg from server:%s:%u,roomid:%u,client connid:%u,maincmd:%u,subcmd:%u.", get_remote_ip(svr_conn), get_remote_port(svr_conn), roomid, pGateMask->param2, maincmd, subcmd);

		if (Sub_Vchat_SetUserPriorityNotify == in_msg->subcmd)
		{
			unsigned int rspDataLen = SIZE_IVM_REQUEST(in_msg);
			char * pRes = (char *)(in_msg->content + SIZE_IVM_CLIENTGATE);
			CMDUserInfoModNotify oNoty;
			oNoty.ParseFromArray(pRes, rspDataLen);
			if (oNoty.has_roletype() && oNoty.mutable_roletype())
			{
				CMDUInt32 * pRoleType = oNoty.mutable_roletype();
				LOG_PRINT(log_info, "user roletype in group is changed.userid:%u,groupid:%u,new roleType:%u.", oNoty.userid(), roomid, pRoleType->data());
				changeGroupRoleType(oNoty.userid(), roomid, pRoleType->data());
			}
		}

		if (Sub_Vchat_SetMicStateNotify == in_msg->subcmd)
		{
			unsigned int rspDataLen = SIZE_IVM_REQUEST(in_msg);
			char * pRes = (char *)(in_msg->content + SIZE_IVM_CLIENTGATE);
			CMDUserMicState oNoty;
			oNoty.ParseFromArray(pRes, rspDataLen);
			LOG_PRINT(log_info, "[set mic state success]errCode:%d,runnid:%u,touserid:%u,groupid:%u,mic index:%d,mic state:%d.", \
				oNoty.errinfo().errid(), oNoty.runid().userid(), oNoty.toid().userid(), oNoty.vcbid(), oNoty.micindex(), oNoty.micstate());
			handleMicState(oNoty.toid().userid(), oNoty.runid().userid(), oNoty.vcbid(), oNoty.micindex(), oNoty.micstate());
		}

		CGateBase::m_ClientMgr.broadcast_room_all_connects(in_msg->subcmd, roomid, pGateMask->param2, buff, e_dev_type, castInGroupOnly);

		if(Sub_Vchat_RoomKickoutUserNoty == in_msg->subcmd)
		{
			unsigned int rspDataLen = SIZE_IVM_REQUEST(in_msg);
			char * pRes = (char *)(in_msg->content + SIZE_IVM_CLIENTGATE);
			CMDUserKickoutRoomInfo rspData;
			rspData.ParseFromArray(pRes, rspDataLen);
			cleanKickoutUserRes(connid, rspData.vcbid(), rspData.toid());
		}
	}
	else
	{
		LOG_PRINT(log_debug, "Broadcast msg from server:%s:%u,maincmd:%u,subcmd:%u,need to broadcast all connects in all rooms.", get_remote_ip(svr_conn), get_remote_port(svr_conn), maincmd, subcmd);
		CGateBase::m_ClientMgr.broadcast_all_room_all_connects(pGateMask->param2, buff, e_dev_type);
	}
}

void CSvrEvent::handle_broadcast_room_gender_msg(const char * msg, int len, clienthandler_ptr svr_conn)
{
	if (!msg || !len || !svr_conn)
	{
		return;
	}

	DEF_IVM_HEADER(in_msg, msg);
	DEF_IVM_CLIENTGATE(pGateMask, msg);
	char * pData = (char *)DEF_IVM_DATA(msg);
	int nMsgLen2 = in_msg->length - SIZE_IVM_CLIENTGATE;
	int nDataLen2 = in_msg->length - SIZE_IVM_HEADER - SIZE_IVM_CLIENTGATE;
	unsigned int maincmd = in_msg->maincmd;
	unsigned int subcmd = in_msg->subcmd;

	SL_ByteBuffer buff(nMsgLen2);
	COM_MSG_HEADER * pOutMsg = (COM_MSG_HEADER *)buff.buffer();
	memcpy(pOutMsg, in_msg, SIZE_IVM_HEADER);
	memcpy(pOutMsg->content, pData, nDataLen2);
	pOutMsg->length = nMsgLen2;
	buff.data_end(pOutMsg->length);

	/* format:
                        0 == pGateMask->param1 && 
                   roomid == pGateMask->param2 && 
    BROADCAST_ROOM_GENDER == pGateMask->param3 && 
    BROADCAST_ROOM_GENDER == pGateMask->param4 &&
                   gender == pGateMask->param5
                        0 == pGateMask->param6
	*/

	unsigned int roomid = pGateMask->param2;
	unsigned int gender = pGateMask->param5;

	if (roomid)
	{
		LOG_PRINT(log_debug, "Broadcast msg from server:%s:%u,roomid:%u,gender:%u,maincmd:%u,subcmd:%u.", get_remote_ip(svr_conn), get_remote_port(svr_conn), roomid, gender, maincmd, subcmd);
		CGateBase::m_ClientMgr.broadcast_room_gender_connects(roomid, buff, (USER_GENDER_TYPE)gender);
	}
	else
	{
		LOG_PRINT(log_debug, "Broadcast msg from server:%s:%u,maincmd:%u,subcmd:%u,need to broadcast all gender:%u connects in all rooms.", get_remote_ip(svr_conn), get_remote_port(svr_conn), maincmd, subcmd, gender);
		CGateBase::m_ClientMgr.broadcast_all_room_gender_connects(buff, (USER_GENDER_TYPE)gender);
	}
}

void CSvrEvent::handle_broadcast_room_roletype(const char *msg, int len, clienthandler_ptr svr_conn)
{
	if (!msg || !len || !svr_conn)
	{
		return;
	}

	DEF_IVM_HEADER(in_msg, msg);
	DEF_IVM_CLIENTGATE(pGateMask, msg);

	e_NoticeDevType devType = (e_NoticeDevType)pGateMask->param1;
	unsigned int roomid = pGateMask->param2;
	unsigned long long param = pGateMask->param5;
	unsigned int maxRole = param & 0xFFFFFFFF;
	unsigned int minRole = (param >> 32) & 0xFFFFFFFF;
	unsigned int userid = pGateMask->param6;
	LOG_PRINT(log_debug, "Broadcast msg from server:%s:%u,roomid:%u,minRole:%u,maxRole:%u,devType:%u,subcmd:%u,except user:%u.", \
		get_remote_ip(svr_conn), get_remote_port(svr_conn), roomid, minRole, maxRole, devType, in_msg->subcmd, userid);

	std::set<clienthandler_ptr> sConn;
	CGateBase::m_ClientMgr.get_room_user_conn_by_roletype_filter(sConn, roomid, minRole, maxRole, devType);
	if (sConn.empty())
	{
		return;
	}

	SL_ByteBuffer buff;
	CMsgComm::convSvrMsgToClientMsg(msg, len, buff);
	for (std::set<clienthandler_ptr>::iterator it = sConn.begin(); it != sConn.end(); it++)
	{
		clienthandler_ptr conn = *it;
		CClientItem * item = (CClientItem *)conn->user_data;
		if (Sub_Vchat_ForbidJoinGroupKillVisitor == in_msg->subcmd)
		{
			if (conn->room_id == roomid && (userid == 0 || conn->user_id != userid))
			{
				conn->room_id = 0;
				CGateBase::m_RoomClientMgr.del_user_connid(roomid, conn->user_id, conn->getconnid());
				item->removeGroupRole(roomid);
				LOG_PRINT(log_debug, "[kick out all visitor]client connid:%u,userid:%u,roomid:%u.", conn->getconnid(), conn->user_id, conn->room_id);
			}
		}
		conn->write_message(buff);
	}
}


void CSvrEvent::handle_broadcast_app_version(const char * msg, int len, clienthandler_ptr svr_conn)
{
	if (!msg || !len || !svr_conn)
	{
		return;
	}

	//DEF_IVM_HEADER(in_msg, msg);
	DEF_IVM_CLIENTGATE(pGateMask, msg);
	e_NoticeDevType devType = (e_NoticeDevType)pGateMask->param1;
	uint64 uVersion = pGateMask->param5;

	SL_ByteBuffer buff;
	CMsgComm::convSvrMsgToClientMsg(msg, len, buff);

	std::vector<clienthandler_ptr> vConn;
	CGateBase::m_ClientMgr.getallconnptr(vConn);
	for (int i = 0; i < vConn.size(); i++)
	{
		if (!vConn[i] || NULL == vConn[i]->user_data) continue;

		CClientItem * item = (CClientItem *)vConn[i]->user_data;
		if (NULL == item || (e_Notice_AllType != devType && devType != item->m_mobile)) continue;
		
		if (convVersionStringToU64(item->sAppVersion) == uVersion)
			vConn[i]->write_message(buff);
	}
}

void CSvrEvent::handleMicState(unsigned int toUserID, unsigned int runUserID, unsigned int groupID, int micIndex, int micState)
{
	if (!toUserID || !groupID)
	{
		return;
	}

	CONNECT_SET connectid_set;
	CGateBase::m_RoomClientMgr.get_one_user_connids(groupID, toUserID, connectid_set);
	CONNECT_SET::iterator iter_set = connectid_set.begin();
	for (; iter_set != connectid_set.end(); ++iter_set)
	{
		clienthandler_ptr pClientConn = CGateBase::m_ClientMgr.find_client_map(*iter_set);
		if (pClientConn && pClientConn->user_data)
		{
			CClientItem * item = (CClientItem *)pClientConn->user_data;
			if (micState == 0)
			{
				item->reset_mic_state();
			}
			else
			{
				item->fill_mic_state(runUserID, toUserID, micIndex, micState, groupID);
			}
		}
	}
}

void CSvrEvent::handleInviteOnMicState(unsigned int runnerID, unsigned int touserID, int optype, int sessionid)
{
	if (!runnerID || !touserID)
	{
		return;
	}

	if (optype != INVT_MIC_AGREE && optype != INVT_MIC_HANGUP)
	{
		return;
	}

	switch(optype)
	{
	case INVT_MIC_AGREE:
		CGateBase::m_ClientMgr.save_user_invite_mic(runnerID, runnerID, touserID, sessionid);
		CGateBase::m_ClientMgr.save_user_invite_mic(touserID, runnerID, touserID, sessionid);
		break;
	case INVT_MIC_HANGUP:
		CGateBase::m_ClientMgr.reset_user_invite_mic(runnerID);
		CGateBase::m_ClientMgr.reset_user_invite_mic(touserID);
		break;
	}
}

void CSvrEvent::changeGroupRoleType(unsigned int userid, unsigned int groupid, unsigned int new_roleType)
{
	if (!userid || !groupid)
	{
		return;
	}

	CONNECT_SET connectid_set;
	CGateBase::m_ClientMgr.get_user_all_connects(userid, connectid_set);
	CONNECT_SET::iterator iter_set = connectid_set.begin();
	for (; iter_set != connectid_set.end(); ++iter_set)
	{
		clienthandler_ptr pClientConn = CGateBase::m_ClientMgr.find_client_map(*iter_set);
		if (pClientConn && pClientConn->user_data)
		{
			CClientItem * item = (CClientItem *)pClientConn->user_data;
			item->setGroupRole(groupid, new_roleType);
			CGateBase::m_RoomClientMgr.add_user_connid(groupid, userid, pClientConn->getconnid());

			LOG_PRINT(log_debug, "[changeGroupRoleType]user roletype in group is changed.client connid:%u,%s:%d,userid:%u,groupid:%u,new roleType:%u.", \
				pClientConn->getconnid(), pClientConn->getremote_ip(), pClientConn->getremote_port(), userid, groupid, new_roleType);
		}
	}
}

void CSvrEvent::cleanKickoutUserRes(unsigned int connid, unsigned int roomid, unsigned int touserid)
{
	unsigned int req_cmd = CGateBase::m_pCmdConfigMgr->getReqbyRspCmd(Sub_Vchat_RoomKickoutUserNoty);
	if (connid)
	{
		clienthandler_ptr pClientConn = CGateBase::m_ClientMgr.find_client_map(connid);
		if (pClientConn && pClientConn->room_id == roomid)
		{
			pClientConn->room_id = 0;
			LOG_PRINT(log_debug, "[kick out user]need to reset roomid in user connection.client connid:%u,userid:%u,roomid:%u.", \
				connid, pClientConn->user_id, pClientConn->room_id);
		}

		if (pClientConn && pClientConn->user_data)
		{
			CClientItem * item = (CClientItem *)pClientConn->user_data;
			item->removeGroupRole(roomid);
		}

		CGateBase::m_ClientMgr.del_req_check(connid, req_cmd);
		CGateBase::m_RoomClientMgr.del_user_connid(roomid, touserid, connid);
	}
	else
	{
		CONNECT_SET connectid_set;
		CGateBase::m_RoomClientMgr.get_one_user_connids(roomid, touserid, connectid_set);
		CONNECT_SET::iterator iter_set = connectid_set.begin();
		for (; iter_set != connectid_set.end(); ++iter_set)
		{
			clienthandler_ptr pClientConn = CGateBase::m_ClientMgr.find_client_map(*iter_set);
			if (pClientConn && pClientConn->room_id == roomid)
			{
				pClientConn->room_id = 0;
				LOG_PRINT(log_debug, "[kick out user]need to reset roomid in user connection.client connid:%u,userid:%u,roomid:%u.", \
					connid, pClientConn->user_id, pClientConn->room_id);
			}

			if (pClientConn && pClientConn->user_data)
			{
				CClientItem * item = (CClientItem *)pClientConn->user_data;
				item->removeGroupRole(roomid);
			}

			CGateBase::m_ClientMgr.del_req_check(*iter_set, req_cmd);
		}

		CGateBase::m_RoomClientMgr.del_one_user_connid(roomid, touserid);
	}
}

void CSvrEvent::handle_broadcast_subscribe_msg(const char * msg, int len, clienthandler_ptr svr_conn)
{
	if (!msg || !len || !svr_conn)
	{
		return;
	}

	DEF_IVM_HEADER(in_msg, msg);
	DEF_IVM_CLIENTGATE(pGateMask, msg);
	char * pData = (char *)DEF_IVM_DATA(msg);
	int nMsgLen2 = in_msg->length - SIZE_IVM_CLIENTGATE;
	int nDataLen2 = in_msg->length - SIZE_IVM_HEADER - SIZE_IVM_CLIENTGATE;
	unsigned int maincmd = in_msg->maincmd;
	unsigned int subcmd = in_msg->subcmd;

	/*format: multicast all connects which subscribe the subcmd.
			client devtype == pGateMask->param1 &&
						 0 == pGateMask->param2 &&
	CAST_CLIENTS_SUBSCRIBE == pGateMask->param3 &&
	CAST_CLIENTS_SUBSCRIBE == pGateMask->param4 &&
						 0 == pGateMask->param5
	*/

	try
	{
		bool broadcast_all = false;
		e_NoticeDevType devtype = e_NoticeDevType(pGateMask->param1);
		std::string strJson(pData, nDataLen2);
		Json::Reader reader(Json::Features::strictMode());
		Json::Value root;
		Json::Value userLst;
		bool bJson = false;
		if (!reader.parse(strJson.c_str(), root))
		{
			LOG_PRINT(log_warning, "Data is not Json format,so broadcast all connects which subscribe this subcmd:%u.", subcmd);
			broadcast_all = true;
		}
		else
		{
			bJson = true;
			if (!root.isMember("user"))
			{
				LOG_PRINT(log_warning, "Data is Json format,but dont contain user,so broadcast all connects which subscribe this subcmd:%u.", subcmd);
				broadcast_all = true;
			}
			else
			{
				userLst = root["user"];
				if (userLst.size() == 0)
				{
					LOG_PRINT(log_warning, "Data is Json format,but contain empty user,so broadcast all connects which subscribe this subcmd:%u.", subcmd);
					broadcast_all = true;
				}
				root.removeMember("user");
			}
		}

		SL_ByteBuffer buff(nMsgLen2);
		COM_MSG_HEADER * pOutMsg = (COM_MSG_HEADER *)buff.buffer();
		memcpy(pOutMsg, in_msg, SIZE_IVM_HEADER);
		if (!bJson)
		{
			memcpy(pOutMsg->content, pData, nDataLen2);
			pOutMsg->length = nMsgLen2;
			buff.data_end(pOutMsg->length);
		}
		else
		{
			Json::FastWriter writer;
			std::string jsonoutput = writer.write(root);
			memcpy(pOutMsg->content, jsonoutput.c_str(), jsonoutput.size());
			pOutMsg->length = SIZE_IVM_HEADER + jsonoutput.size();
			buff.data_end(pOutMsg->length);
		}

		if (broadcast_all)
		{
			CGateBase::m_ClientMgr.broadcast_all_subscribe_client(devtype, subcmd, buff);
		}
		else
		{
			for(unsigned int i = 0; i < userLst.size(); ++i) 
			{
				unsigned int userid = atoi(userLst[i].asString().c_str());
				//focus on userid
				LOG_PRINT(log_debug, "Broadcast connects which subscribe msg from server:%s:%u,userid:%u,maincmd:%u,subcmd:%u.", get_remote_ip(svr_conn), get_remote_port(svr_conn), userid, maincmd, subcmd);
				if (!userid)
				{
					continue;
				}

				CGateBase::m_ClientMgr.broadcast_user_subscribe_client(userid, devtype, subcmd, buff);
			}
		}
	}
	catch(...)
	{
		LOG_PRINT(log_error, "[handle_broadcast_subscribe_msg]Json format error.");
	}
}

//broadcast all clients which connect on one server
void CSvrEvent::handle_cast_clients_on_one_svr(const char * msg, int len, clienthandler_ptr svr_conn)
{
	if (!msg || !len)
	{
		return;
	}

	std::set<unsigned int> client_conn_set;
	svr_conn->getallclientid(client_conn_set);

	if (client_conn_set.empty())
	{
		LOG_PRINT(log_warning, "server:%s:%u has no client connection.", get_remote_ip(svr_conn), get_remote_port(svr_conn));
		return;
	}

	DEF_IVM_HEADER(in_msg, msg);
	DEF_IVM_CLIENTGATE(pGateMask, msg);
	char * pData = (char *)DEF_IVM_DATA(msg);
	int nMsgLen2 = in_msg->length - SIZE_IVM_CLIENTGATE;
	int nDataLen2 = in_msg->length - SIZE_IVM_HEADER - SIZE_IVM_CLIENTGATE;
	unsigned int maincmd = in_msg->maincmd;
	unsigned int subcmd = in_msg->subcmd;

	SL_ByteBuffer buff(nMsgLen2);
	COM_MSG_HEADER * pOutMsg = (COM_MSG_HEADER *)buff.buffer();
	memcpy(pOutMsg, in_msg, SIZE_IVM_HEADER);
	memcpy(pOutMsg->content, pData, nDataLen2);
	pOutMsg->length = nMsgLen2;
	buff.data_end(pOutMsg->length);
	LOG_PRINT(log_info, "[Broadcast all client on one server]client connects which connect on server:%s:%u,maincmd:%u,subcmd:%u.client count:%u.msg len:%u.", \
		get_remote_ip(svr_conn), get_remote_port(svr_conn), maincmd, subcmd, client_conn_set.size(), nMsgLen2);

	e_NoticeDevType e_dev_type = (e_NoticeDevType)pGateMask->param1;
	unsigned int except_connid = pGateMask->param2;
	//是否只广播给指定房间
	unsigned int roomid = pGateMask->param5;
	std::set<unsigned int> user_conn_set;
	if (0 != roomid)
	{
		CGateBase::m_RoomClientMgr.get_user_connids(roomid, user_conn_set);
	}

	int success_count = 0;
	std::set<unsigned int>::iterator iter = client_conn_set.begin();
	for (; iter != client_conn_set.end(); ++iter)
	{
		if (except_connid == *iter)
		{
			continue;
		}

		if (0 != roomid && user_conn_set.find(*iter) == user_conn_set.end())
		{
			//非目标房间
			continue;
		}

		if (0 == CGateBase::m_ClientMgr.send_msg_to_client_by_connID(*iter, buff, e_dev_type))
		{
			++success_count;
		}
	}
	LOG_PRINT(log_info, "[Broadcast all client on one server]result has sent %u client connects which connect on server:%s:%u,maincmd:%u,subcmd:%u.msg len:%u.", \
		success_count, get_remote_ip(svr_conn), get_remote_port(svr_conn), maincmd, subcmd, nMsgLen2);
}

void CSvrEvent::del_req_check_by_connid(clienthandler_ptr svr_conn)
{
	if (!svr_conn)
	{
		return;
	}

	std::set<unsigned int> client_conn_set;
	svr_conn->getallclientid(client_conn_set);

	if (client_conn_set.empty())
	{
		return;
	}

	std::set<unsigned int> req_set;
	CGateBase::m_pCmdConfigMgr->getReqbySvrName(CGateBase::change_type_to_svrname(svr_conn->getsvrtype()), req_set);
	if (req_set.empty())
	{
		return;
	}

	std::set<unsigned int>::iterator iter = req_set.begin();
	for (; iter != req_set.end(); ++iter)
	{
		unsigned int req_cmd = *iter;
		CGateBase::m_ClientMgr.del_req_check_by_conn_set(client_conn_set, req_cmd);
	}
}

void CSvrEvent::print_specail_cmd(const char * msg, int len, clienthandler_ptr svr_conn)
{
	//if (!msg || !len ||!svr_conn)
	//{
	//	return;
	//}

	//DEF_IVM_HEADER(in_msg, msg);
	//DEF_IVM_CLIENTGATE(pGateMask, msg);

	//switch (in_msg->subcmd)
	//{
	//default:
	//	break;
	//}
}

std::string CSvrEvent::get_svr_name(clienthandler_ptr svr_conn)
{
	std::string ret = "";
	if (svr_conn && svr_conn->user_data)
	{
		CSvrItem * item = (CSvrItem *)svr_conn->user_data;
		ret = item->m_procname;
	}
	return ret;
}

void CSvrEvent::set_pingtime(clienthandler_ptr svr_conn)
{
	if (svr_conn && svr_conn->user_data)
	{
		//LOG_PRINT(log_debug, "[set_pingtime]server[%s]:%s:%u.", get_svr_name(svr_conn).c_str(), get_remote_ip(svr_conn), get_remote_port(svr_conn));
		CSvrItem * item = (CSvrItem *)svr_conn->user_data;
		item->m_pingtime = time(NULL);
	}
}

time_t CSvrEvent::get_pingtime(clienthandler_ptr svr_conn)
{
	if (svr_conn && svr_conn->user_data)
	{
		CSvrItem * item = (CSvrItem *)svr_conn->user_data;
		return item->m_pingtime;
	}
	else
	{
		return 0;
	}
}
