/*********************************************************************
 *  Copyright 2016 by 99letou.
 *  All right reserved.
 *
 *  功能：核心功能文件
 *
 *  Edit History:
 *
 *    2016/04/13 - Created by baoshengjun to print output in the new form. 
 */
#include "LogicCore.h" 
#include "DBConnection.h"
#include "crc32.h"
#include "CryptoHandler.h"
#include "ipdb99cj.h"
#include "HttpClient.h"
#include "utils.h"
#include "serverEvent.h"
#include "AppDelegate.h"
#include "DBTools.h"
#include "DBSink.h"
#include "errcode.h"
#include "msgcommapi.h"
#include "macro_define.h"
#include "TChatSvr.h"
#include "CThriftSvrMgr.h"
#include "tppushsvr/TpPushSvr.h"
#include "CUserBasicInfo.h"
#include "CUserGroupinfo.h"
#include "CUserGroupMgr.h"
#include "UserMgrSvr.pb.h"
#include "RoomSvr.pb.h"
#include "DBSink.h"
#include "json/json.h"

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/transport/THttpClient.h>
#include <thrift/async/TEvhttpClientChannel.h>
#include <event.h>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <fstream>
#include <cctype>
#include <algorithm>
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::async;
const std::string zombieFile = "zombie.txt";
CLogicCore::CLogicCore(void)
{
	m_zombies.clear();
	string filePath = Application::getInstance()->getPath(PATH_TYPE_CONFIG) + zombieFile;
    std::ifstream in(filePath.c_str());
    if(in.is_open())
    {
    	LOG_PRINT(log_info, "m_zombie:%s opened++++++++++++++++++++++++++++++++++++++++++++++",filePath.c_str());
    }else
    {
    	LOG_PRINT(log_info, "m_zombie:%s not opened++++++++++++++++++++++++++++++++++++++++++++++",filePath.c_str());
    }
    std::string zombie;
    LOG_PRINT(log_info, "m_zombie++++++++++++++++++++++++++++++++++++++++++++++");
    while(in >> zombie){
        LOG_PRINT(log_info, "m_zombie: %s", zombie.c_str());
        m_zombies.push_back(zombie);
    }
    LOG_PRINT(log_info, "m_zombie.size: %d", m_zombies.size());
}

CLogicCore::~CLogicCore(void)
{
} 

void CLogicCore::setEvent(ServerEvent* event)
{
	m_pSvrEvent = event;
}

int CLogicCore::onMessage(task_proc_data * message)
{
	if (0 == message->connection || !message->connection->isconnected()) 
	{
		LOG_PRINT(log_error, "[connid:%u,%s:%u]connection disconnected", message->connection->getconnid(), message->connection->getremote_ip(),
				message->connection->getremote_port());
		return -1;
	}

	if (message->datalen <= 0) {
		LOG_PRINT(log_error, "message length is 0");
		return -1;
	}

	COM_MSG_HEADER* in_msg = (COM_MSG_HEADER*)message->pdata;
	ClientGateMask_t* pGateMask = (ClientGateMask_t*)in_msg->content;

//	if (Sub_Vchat_ClientHello != in_msg->subcmd && Sub_Vchat_ClientHelloResp != in_msg->subcmd && pGateMask->param1 == 0)
//		return -1;

	if (Sub_Vchat_ClientHello == in_msg->subcmd ) {
		CMDGateHello_t* pHelloMsg = (CMDGateHello_t *)in_msg->content;
		if(pHelloMsg->param1 == 12 && pHelloMsg->param2 == 8 &&	pHelloMsg->param3 == 7) {
			if (e_gateway_type == pHelloMsg->param4 || 1 == pHelloMsg->param4) {
				message->connection->setgateid(pHelloMsg->gateid);
				m_pSvrEvent->m_roomMgr.m_mapConn[message->connection->getconnid()] = message->connection;
				LOG_PRINT(log_info, "[%s:%u,connid:%u,gateid:%u]recv client hello message,my m_svrid=%d", message->connection->getremote_ip(), message->connection->getremote_port(),
						message->connection->getconnid(), message->connection->getgateid(),m_svrid);
			}
			hello_response(message);
			m_mapConn[message->connection->getconnid()] = message->connection;
		}
		return 0;
	}
	if (Sub_Vchat_ClientPing == in_msg->subcmd)	{
		in_msg->subcmd = Sub_Vchat_ClientPingResp;
		message->connection->write_message(message->pdata, message->datalen);
		//LOG_PRINT(log_debug, "[connid:%u,gateid:%u,%s:%u]ping response,my m_svrid=%d", message->connection->getconnid(), message->connection->getgateid(),
			//message->connection->getremote_ip(), message->connection->getremote_port(),m_svrid)
		return 0;
	}
	if (!findconn(message->connection->getconnid())) {
		LOG_PRINT(log_error, "[subcmd:%u,connid:%u,%s:%u] still not say hello, ignore message", in_msg->subcmd, message->connection->getconnid(),
				message->connection->getremote_ip(), message->connection->getremote_port());
		return -1;
	}

	m_gateid = message->connection->getgateid();
	LOG_PRINT(log_debug, "CLogicCore::onMessage m_gateid:%u.", m_gateid);
	switch(in_msg->subcmd) {
	case Sub_Vchat_JoinRoomReq:
		{
			unsigned int ntime_begin = 0, ntime_end = 0;
            LOG_PRINT(log_info, "++++++ Sub_Vchat_JoinRoomReq.");
			ntime_begin = SL_Socket_CommonAPI::util_process_clock_ms();
			proc_joinroom_req(message);
			ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();
			LOG_PRINT(log_info, "------ Sub_Vchat_JoinRoomReq ms = %d.", ntime_end - ntime_begin);
		}
		break;
	case Sub_Vchat_JoinGroupReq:
		{
			unsigned int ntime_begin = 0, ntime_end = 0;
			LOG_PRINT(log_info, "++++++ Sub_Vchat_JoinGroupReq.");
			ntime_begin = SL_Socket_CommonAPI::util_process_clock_ms();
			proc_joingroup_req(message);
			ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();
			LOG_PRINT(log_info, "------ Sub_Vchat_JoinGroupReq ms = %d.", ntime_end - ntime_begin);
		}
		break;
	case Sub_Vchat_MicStateListReq:
		{
			unsigned int ntime_begin = 0, ntime_end = 0;
			LOG_PRINT(log_info, "room_tag++++++ Sub_Vchat_MicStateListReq.");
			ntime_begin = SL_Socket_CommonAPI::util_process_clock_ms();
			proc_micstatelist_req(message);
			ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();
			LOG_PRINT(log_info, "room_tag------ Sub_Vchat_MicStateListReq ms = %d.", ntime_end - ntime_begin);
		}
		break;
	case Sub_Vchat_SetRoomInfoReq:
		proc_setroominfo_req(message);
		break;
	case Sub_Vchat_AfterJoinRoomReq:
		proc_afterjoinroom_req(message);
		break;
	case Sub_Vchat_GateJoinRoom:
		proc_gatejoinroom_req(message);
		break;
	case Sub_Vchat_RoomUserExitReq:
		{
			unsigned int ntime_begin = 0, ntime_end = 0;
			LOG_PRINT(log_info, "room_tag++++++ Sub_Vchat_RoomUserExitReq.");
			ntime_begin = SL_Socket_CommonAPI::util_process_clock_ms();
			proc_exitroom_req(message);
			ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();
			LOG_PRINT(log_info, "room_tag ------ Sub_Vchat_RoomUserExitReq ms = %d.", ntime_end - ntime_begin);
		}
		break;
	case Sub_Vchat_ChangeWaitMicIndexReq:
		{
			unsigned int ntime_begin = 0, ntime_end = 0;
			LOG_PRINT(log_info, "room_tag++++++ Sub_Vchat_ChangeWaitMicIndexReq.");
			ntime_begin = SL_Socket_CommonAPI::util_process_clock_ms();
			proc_changewaitmicindex_req(message);
			ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();
			LOG_PRINT(log_info, "room_tag ------ Sub_Vchat_ChangeWaitMicIndexReq ms = %d.", ntime_end - ntime_begin);
		}
		break;
	case Sub_Vchat_RoomUserExceptExitReq:
		proc_except_exitroom_req(message);
		break;
	case Sub_Vchat_RoomKickoutUserReq:
		proc_kickoutuser_req(message);
		break;
	case Sub_Vchat_GatewayKickoutUserReq:
		proc_gatekickoutuser_req(message);
		break;
	case Sub_Vchat_QuitGroupReq:
		proc_quitgroup_req(message);
		break;	
	case Sub_Vchat_SetMicStateReq:
		{
			unsigned int ntime_begin = 0, ntime_end = 0;
			LOG_PRINT(log_info, "room_tag++++++ Sub_Vchat_SetMicStateReq.");
			ntime_begin = SL_Socket_CommonAPI::util_process_clock_ms();
			proc_setmicstate_req(message);
			ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();
			LOG_PRINT(log_info, "room_tag ------ Sub_Vchat_SetMicStateReq ms = %d.", ntime_end - ntime_begin);
		}
		break;
	case Sub_Vchat_UpWaitMicReq:
		proc_upwaitmic_req(message);
		break;
	case Sub_Vchat_SetDevStateReq:
		proc_setdevstate_req(message);
		break;
	case Sub_Vchat_SetRoomOPStatusReq:
		{
			unsigned int ntime_begin = 0, ntime_end = 0;
			LOG_PRINT(log_info, "room_tag++++++ Sub_Vchat_SetRoomOPStatusReq.");
			ntime_begin = SL_Socket_CommonAPI::util_process_clock_ms();
			proc_setroomopstatus_req(message);
			ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();
			LOG_PRINT(log_info, "room_tag ------ Sub_Vchat_SetRoomOPStatusReq ms = %d.", ntime_end - ntime_begin);
		}
		break;
	case Sub_Vchat_SetRoomMediaReq:
		proc_setroommedia_req(message);
		break;
	case Sub_Vchat_ReportMediaGateReq:
		proc_client_reportmediagateip_req(message);
		break;
	case Sub_Vchat_SOFTBOT_JOINROOM:
//		proc_softbot_joinroom_req(message);
		break;
	case Sub_Vchat_SOFTBOT_EXITROOM:
//		proc_softbot_exitroom_req(message);
		break;
	case Sub_Vchat_GroupMemberReq:
		proc_groupmember_req(message);
		break;
	case Sub_Vchat_QryUserGroupInfoReq:
		proc_queryUserGroupInfo_req(message);
		break;
	case Sub_Vchat_SetGroupMsgMuteReq:
		proc_setGroupMsgMute_req(message);
		break;
	case Sub_Vchat_SetUserPriorityReq:
		proc_setUserPriority_req(message);
		break;
	case Sub_Vchat_QryGroupVisitCountReq:
		proc_qryGroupVisitCount_req(message);
		break;
	case Sub_Vchat_SetMicModeReq:
		{
			unsigned int ntime_begin = 0, ntime_end = 0;
			LOG_PRINT(log_info, "room_tag++++++ Sub_Vchat_SetMicModeReq.");
			ntime_begin = SL_Socket_CommonAPI::util_process_clock_ms();
			proc_setmicmode_req(message);
			ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();
			LOG_PRINT(log_info, "room_tag ------ Sub_Vchat_SetMicModeReq ms = %d.", ntime_end - ntime_begin);
		}
		break;
	case Sub_Vchat_SetMicPosStatusReq:
		{
			unsigned int ntime_begin = 0, ntime_end = 0;
			LOG_PRINT(log_info, "room_tag++++++ Sub_Vchat_SetMicPosStatusReq.");
			ntime_begin = SL_Socket_CommonAPI::util_process_clock_ms();
			proc_setmicposStatus_req(message);
			ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();
			LOG_PRINT(log_info, "room_tag ------ Sub_Vchat_SetMicPosStatusReq ms = %d.", ntime_end - ntime_begin);
		}
		break;
	case Sub_Vchat_GetRoomInfoExtReq:
		{
			unsigned int ntime_begin = 0, ntime_end = 0;
			LOG_PRINT(log_info, "room_tag++++++ Sub_Vchat_GetRoomInfoExtReq.");
			ntime_begin = SL_Socket_CommonAPI::util_process_clock_ms();
			proc_getroominfo_ext_req(message);
			ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();
			LOG_PRINT(log_info, "room_tag ------ Sub_Vchat_GetRoomInfoExtReq ms = %d.", ntime_end - ntime_begin);
		}
		break;
	case Sub_Vchat_InviteOnMicReq:
		{
			unsigned int ntime_begin = 0, ntime_end = 0;
			LOG_PRINT(log_info, "room_tag++++++ Sub_Vchat_InviteOnMicReq.");
			ntime_begin = SL_Socket_CommonAPI::util_process_clock_ms();
			proc_inviteonmic_req(message);
			ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();
			LOG_PRINT(log_info, "room_tag ------ Sub_Vchat_InviteOnMicReq ms = %d.", ntime_end - ntime_begin);
		}
		break;
	case Sub_Vchat_ClientPing:
		break;
	case Sub_Vchat_GroupSettingStatReq:
		proc_GroupSettingStatReq(message);
		break;
	case Sub_Vchat_QryJoinGroupCondReq:
		proc_qryJoinGroupConditionReq(message);
		break;
	case Sub_Vchat_ApplyJoinGroupAuthReq:
		proc_applyJoinGroupAuthReq(message);
		break;
	case Sub_Vchat_HandleJoinGroupAuthReq:
		proc_handleJoinGroupAuthReq(message);
		break;
	case Sub_Vchat_GroupUserSettingReq:
		proc_handleGroupUserSettingReq(message);
		break;
	case Sub_Vchat_UserAuthReq:
		{
			unsigned int ntime_begin = 0, ntime_end = 0;
			LOG_PRINT(log_info, "room_tag++++++ Sub_Vchat_UserAuthReq.");
			ntime_begin = SL_Socket_CommonAPI::util_process_clock_ms();
			proc_UserAuthReq(message);
			ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();
			LOG_PRINT(log_info, "room_tag ------ Sub_Vchat_UserAuthReq ms = %d.", ntime_end - ntime_begin);
		}
		break;
	case Sub_Vchat_ForbidUserChat:
		{
			unsigned int ntime_begin = 0, ntime_end = 0;
			LOG_PRINT(log_info, "room_tag++++++ Sub_Vchat_ForbidUserChat.");
			ntime_begin = SL_Socket_CommonAPI::util_process_clock_ms();
			procForbidUserChat(message);
			ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();
			LOG_PRINT(log_info, "room_tag ------ Sub_Vchat_ForbidUserChat ms = %d.", ntime_end - ntime_begin);
		}
		break;
	case Sub_Vchat_CourseFinishReq:
		{
			unsigned int ntime_begin = 0, ntime_end = 0;
			LOG_PRINT(log_info, "room_tag++++++ Sub_Vchat_CourseFinishReq.");
			ntime_begin = SL_Socket_CommonAPI::util_process_clock_ms();
			procCourseFinish(message);
			ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();
			LOG_PRINT(log_info, "room_tag ------ Sub_Vchat_CourseFinishReq ms = %d.", ntime_end - ntime_begin);
		}
		break;
	default:
	    LOG_PRINT(log_warning, "room_tag unknown request subcmd:%d", in_msg->subcmd);
	    resperrinf(in_msg, pGateMask, ERR_CODE_FAILED_UNKNONMESSAGETYPE, message);
		break;
	}

	return 0;
}
void CLogicCore::procForbidUserChat(task_proc_data * message)
{
	CRoomManager* pRoomMgr = m_pSvrEvent->getRoomMgr();
	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)(message->pdata);
	if (pHead->length < SIZE_IVM_HEAD_TOTAL)
	{
		LOG_PRINT(log_warning, "Sub_Vchat_UserAuthReq parse  msginfo fixed length failed, need at lease %d but recv %d!", SIZE_IVM_TOTAL(CMDForbidUserChat), pHead->length);
		return ;
	}

	ClientGateMask_t * pGateMask = (ClientGateMask_t *)(pHead->content);
	CMDForbidUserChat pReq;
	pReq.ParseFromArray(pHead->content + SIZE_IVM_CLIENTGATE, SIZE_IVM_REQUEST(pHead));
	LOG_PRINT(log_info, "room_tag++++++ procForbidUserChat=====.userid[%d],toid[%d],groupid[%d],status[%d]",pReq.userid(),pReq.toid(),pReq.groupid(),pReq.status());
	RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(pReq.groupid());
	if(!pRoomObjRef.get())
	{
		LOG_PRINT(log_error, "room_tag not found groupid:%d", pReq.groupid());
		return ;
	}
	int ret = pRoomObjRef->setForbidUserChat(pReq.userid(),pReq.toid(),pReq.status());
	LOG_PRINT(log_info, "procForbidUserChat ret :%d", ret);
	ClientGateMask_t* pClientGate;
	char szBuf[512] = {0};
	CMDForbidUserChat noty;
	noty.CopyFrom(pReq);
	noty.set_status(pReq.status());
	noty.set_errid(ret);	 
	SERIALIZETOARRAY_BUF(noty,respbuf,len);
	if (ret == 0)
	{
		pRoomMgr->Build_NetMsg(szBuf, 512, MDM_Vchat_Room, Sub_Vchat_ForbidUserChatNoty, &pClientGate, respbuf, len);
		if (pReq.toid() > 0)
		{
			CMsgComm::Build_BroadCastUser_Gate(pClientGate, pReq.toid(), e_Notice_AllType, 0);
			pRoomMgr->castGateway((COM_MSG_HEADER *)szBuf);
			LOG_PRINT(log_info, "procForbidUserChatNoty some one.");
		}
		else
		{
			LOG_PRINT(log_info, "procForbidUserChatNoty all.");
			pRoomObjRef->castSendMsgXXX((COM_MSG_HEADER*)szBuf);
		}
	}
	
	message->respProtobuf(noty, Sub_Vchat_ForbidUserChatRsp);
}
void CLogicCore::procCourseFinish(task_proc_data * message)
{
	CRoomManager* pRoomMgr = m_pSvrEvent->getRoomMgr();
	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)(message->pdata);
	if (pHead->length < SIZE_IVM_HEAD_TOTAL)
	{
		LOG_PRINT(log_warning, "Sub_Vchat_UserAuthReq parse  msginfo fixed length failed, need at lease %d but recv %d!", SIZE_IVM_TOTAL(CMDCourseFinish), pHead->length);
		return ;
	}
	LOG_PRINT(log_info, "room_tag++++++ CMDCourseFinish=====.");
	ClientGateMask_t * pGateMask = (ClientGateMask_t *)(pHead->content);
	CMDCourseFinish pReq;
	pReq.ParseFromArray(pHead->content + SIZE_IVM_CLIENTGATE, SIZE_IVM_REQUEST(pHead));

	RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(pReq.groupid());
	if(!pRoomObjRef.get())
	{
		LOG_PRINT(log_error, "room_tag not found groupid:%d", pReq.groupid());
		return ;
	}
	pRoomObjRef->setCourseFinish();
	pRoomObjRef->setForbidUserChat(0,0,0);
	ClientGateMask_t* pClientGate;
	char szBuf[512] = {0};
	CMDCourseFinish noty;
	noty.CopyFrom(pReq);
	SERIALIZETOARRAY_BUF(noty,respbuf,len);
	pRoomMgr->Build_NetMsg(szBuf,512,MDM_Vchat_Room,Sub_Vchat_CourseFinishNoty,&pClientGate,respbuf,len);

	pRoomObjRef->castSendMsgXXX((COM_MSG_HEADER*)szBuf);
}

int CLogicCore::proc_UserAuthReq(task_proc_data* message)
{
	CRoomManager* pRoomMgr = m_pSvrEvent->getRoomMgr();
	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)(message->pdata);
	if (pHead->length < SIZE_IVM_HEAD_TOTAL)
	{
		LOG_PRINT(log_warning, "Sub_Vchat_UserAuthReq parse  msginfo fixed length failed, need at lease %d but recv %d!", SIZE_IVM_TOTAL(CMDAuthReq), pHead->length);
		return -1;
	}
	LOG_PRINT(log_info, "room_tag++++++ Sub_Vchat_UserAuthReq=====.");
	ClientGateMask_t * pGateMask = (ClientGateMask_t *)(pHead->content);
	CMDAuthReq pReq;
	pReq.ParseFromArray(pHead->content + SIZE_IVM_CLIENTGATE, SIZE_IVM_REQUEST(pHead));
	int nret = 0;
	string encrypText = "";
	LOG_PRINT(log_info, "room_tag++++++ Sub_Vchat_UserAuthReq=====11111.");
	string key = m_pSvrEvent->m_pApp->m_key;
	string appid = m_pSvrEvent->m_pApp->m_Appid;
	do{
//		string key = getAppIdKey(pReq.appid());
		LOG_PRINT(log_info, "room_tag key:%s .", key.c_str());
		LOG_PRINT(log_info, "room_tag++++++ Sub_Vchat_UserAuthReq=====11111key:%s .", key.c_str());
		initKV(key.c_str());
		char text[256] = {0};
		sprintf(text,"%d:%d:%s:%d:%d:%d",m_pSvrEvent->m_pApp->m_Version,appid.c_str(),m_pSvrEvent->m_pApp->m_usetoken,pReq.vcbid(),pReq.userid(),SL_Socket_CommonAPI::util_process_clock_ms());
		encrypText = encrypt(text);
		LOG_PRINT(log_info, "room_tag++++++ Sub_Vchat_UserAuthReq=====encrypText:%s .", encrypText.c_str());
		string decryptText = decrypt(encrypText);
		LOG_PRINT(log_info, "room_tag++++++ Sub_Vchat_UserAuthReq=====decryptText:%s .", decryptText.c_str());
		RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(pReq.vcbid());
		if(pRoomObjRef.get() == 0)
		{
			LOG_PRINT(log_error, "room_tag not found groupid:%d", pReq.vcbid());
			nret = ERR_CODE_FAILED_ROOMIDNOTFOUND;
			break;
		}
	}while(0);
	char szBuf[1024] = {0};
	//操作成功
	LOG_PRINT(log_info, "room_tag++++++ Sub_Vchat_UserAuthReq=====23333");
	CMDAuthResp respInfo;
	ClientGateMask_t * pClientGate;
	respInfo.set_appid(appid);
	respInfo.set_recvkey(encrypText);
	respInfo.set_sendkey(encrypText);
	respInfo.set_vcbid(pReq.vcbid());
	respInfo.set_userid(pReq.userid());
	respInfo.set_errcode(nret);
	SERIALIZETOARRAY_BUF(respInfo,respbuf,len);
	int nMsgLen=pRoomMgr->Build_NetMsg(szBuf,1024,MDM_Vchat_Room,Sub_Vchat_UserAuthResp,&pClientGate,respbuf,len);
	if(nMsgLen > 0)
	{
		pClientGate->param1=pGateMask->param1;
		pClientGate->param2=pGateMask->param2;
		LOG_PRINT(log_info, "room_tag++++++ Sub_Vchat_UserAuthReq=====write_message");
		message->connection->write_message(szBuf, nMsgLen);
	}
	return 1;
}
int CLogicCore::proc_inviteonmic_req(task_proc_data* message)
{
	CRoomManager* pRoomMgr = m_pSvrEvent->getRoomMgr();
	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)(message->pdata);
	if (pHead->length < SIZE_IVM_HEAD_TOTAL)
	{
		LOG_PRINT(log_warning, "parse msginfo fixed length failed, need at lease %d but recv %d!", SIZE_IVM_TOTAL(CMDInviteOnMic), pHead->length);
		return -1;
	}

	ClientGateMask_t * pGateMask = (ClientGateMask_t *)(pHead->content);
	CMDInviteOnMic pReq;
	pReq.ParseFromArray(pHead->content + SIZE_IVM_CLIENTGATE, SIZE_IVM_REQUEST(pHead));

	RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(pReq.vcbid());
	if(pRoomObjRef.get() == 0)
	{
		LOG_PRINT(log_error, "room_tag not found groupid:%d", pReq.vcbid());
//		message->connection->resperrinf(in_msg, pGateMask, ERR_CODE_FAILED_ROOMIDNOTFOUND);
//		return -1;
	}

	UserObject_Ref pSrcUserRef, pToUsrRef;
	char szBuf[512] = {0};
	int optype = pReq.optype();
	int nret = 0;

	pSrcUserRef = CUserObj::findUserOnline(pReq.runuserid());
	pToUsrRef = CUserObj::findUserOnline(pReq.touserid());
	bool bself = false;
	if(pSrcUserRef.get() != 0)
	{
		LOG_PRINT(log_info,"room_tag %d, touserid:%d.", optype, pReq.touserid());
		if(pToUsrRef.get() != 0)
		{
			switch(optype)// 1 邀请 2同意 3拒绝 4超时拒绝 5连接成功 6挂断 7取消邀请 8被邀请 9强制挂断
			{
			case INVT_MIC_INVITE://邀请
				{
					if(pSrcUserRef->nprivate_time - time(0) > 30)
					{
						pSrcUserRef->noptype = 0;
					}
					else if(pSrcUserRef->noptype == 1)
					{
						nret = ERR_USER_IS_INVITING_MIC; //对方邀请中
						bself = true;
						break;
					}
					if(pToUsrRef->nprivate_time - time(0) > 30)
					{
						pToUsrRef->noptype = 0;
					}
					else if(pToUsrRef->noptype == 8)
					{
						nret = ERR_USER_IS_INVITED_MIC; //对方被邀请中
						bself = true;
						break;
					}

					uint32 intimacy = 0,threshold = 0;
					if(CCommonLogic::chkGroupPrivateChatQualitication(*m_pSvrEvent->m_pRedisMgr, pReq.vcbid(), pReq.runuserid(), pReq.touserid()
							,&threshold,&intimacy))
					{
						pReq.set_threshold(threshold);
						pReq.set_intimacy(intimacy);
						bself = true;
						nret = ERR_CODE_GROUP_PRIVATE_CHAT_LIMIT;
						break;
					}
					if(pSrcUserRef->noptype == 0)
					{
						pSrcUserRef->noptype = 1;
						pToUsrRef->noptype = 8;
						pToUsrRef->nprivate_time = time(0);
						pSrcUserRef->nprivate_time = time(0);
					}
					LOG_PRINT(log_error,"room_tag %d pToUsrRef->noptype:%d invalid.",optype,pToUsrRef->noptype);
				}
				break;
			case INVT_MIC_AGREE://同意
				{
					if(pToUsrRef->noptype == 1 || pToUsrRef->noptype == 2 || pToUsrRef->noptype == 0)
					{
						CMDInviteOnMic pMic;
						pMic.CopyFrom(pReq);
						if(pSrcUserRef->nprivate_userid > 0)
						{
							//挂断自己的连接
							pMic.set_runuserid(pReq.runuserid());
							pMic.set_touserid(pSrcUserRef->nprivate_userid);
							pMic.set_optype(6);
							pMic.mutable_errinfo()->set_errid(0);
							SERIALIZETOARRAY_BUF(pMic,respbuf,len);
							pRoomMgr->Build_NetMsg(szBuf,512,MDM_Vchat_Room,Sub_Vchat_InviteOnMicNoty,&pGateMask,respbuf,len);
							{
								CMsgComm::Build_BroadCastUser_Gate(pGateMask, pSrcUserRef->nprivate_userid, e_Notice_AllType, 0);
								pRoomMgr->castGateway((COM_MSG_HEADER*)szBuf);//to toid

								CMsgComm::Build_BroadCastUser_Gate(pGateMask, pReq.runuserid(), e_Notice_AllType, 0);
								pRoomMgr->castGateway((COM_MSG_HEADER*)szBuf);//to runuserid
							}
						}
						if(pToUsrRef->nprivate_userid > 0)
						{
							//挂断对方的连接
							pMic.set_runuserid(pReq.touserid());
							pMic.set_touserid(pToUsrRef->nprivate_userid);
							pMic.set_optype(6);
							pMic.mutable_errinfo()->set_errid(0);
							SERIALIZETOARRAY_BUF(pMic,respbuf,len);
							pRoomMgr->Build_NetMsg(szBuf,512,MDM_Vchat_Room,Sub_Vchat_InviteOnMicNoty,&pGateMask,respbuf,len);
							{
								CMsgComm::Build_BroadCastUser_Gate(pGateMask, pToUsrRef->nprivate_userid, e_Notice_AllType, 0);
								pRoomMgr->castGateway((COM_MSG_HEADER*)szBuf);//to toid

								CMsgComm::Build_BroadCastUser_Gate(pGateMask, pReq.touserid(), e_Notice_AllType, 0);
								pRoomMgr->castGateway((COM_MSG_HEADER*)szBuf);//to runuserid
							}
						}
						if(pRoomObjRef.get() != 0)
						{
							pRoomObjRef->downPublicMic(pToUsrRef);
							pRoomObjRef->delWaitMicUser(pToUsrRef->nuserid_, -1);
							pRoomObjRef->downPublicMic(pSrcUserRef);
							pRoomObjRef->delWaitMicUser(pSrcUserRef->nuserid_, -1);
						}
						pSrcUserRef->nprivate_userid = pToUsrRef->nuserid_;
						pToUsrRef->nprivate_userid = pSrcUserRef->nuserid_;
						pToUsrRef->noptype = 2;
						pSrcUserRef->noptype = 2;
					}
					else
					{
						nret = ERR_CODE_INVALID_PARAMETER;
						LOG_PRINT(log_error,"room_tag %d pToUsrRef->noptype:%d invalid.",optype,pToUsrRef->noptype);
					}
				}
				break;
			case INVT_MIC_REJECT://拒绝
				{
					if(pToUsrRef->noptype == 1 || pToUsrRef->noptype == 0)
					{
						pToUsrRef->noptype = 0;
						pSrcUserRef->noptype = 0;
						pSrcUserRef->nprivate_userid = 0;
						pToUsrRef->nprivate_userid = 0;
					}
					else
					{
						nret = ERR_CODE_INVALID_PARAMETER;
						LOG_PRINT(log_error,"room_tag %d pToUsrRef->noptype:%d invalid.",optype,pToUsrRef->noptype);
					}
				}
				break;
			case INVT_MIC_TIMEOUT://超时拒绝
				{
					if(pToUsrRef->noptype == 1 || pToUsrRef->noptype == 0)
					{
						pSrcUserRef->nprivate_userid = 0;
						pToUsrRef->nprivate_userid = 0;
						pToUsrRef->noptype = 0;
						pSrcUserRef->noptype = 0;
					}
					else
					{
						nret = ERR_CODE_INVALID_PARAMETER;
						LOG_PRINT(log_error,"room_tag %d pToUsrRef->noptype:%d invalid.",optype,pToUsrRef->noptype);
					}
				}
				break;
			case INVT_MIC_CONNECTED://连接成功
				break;
			case INVT_MIC_HANGUP://挂断
				{
					if(pToUsrRef->noptype == 2 || pSrcUserRef->noptype == 2 || pToUsrRef->noptype == 0 || pSrcUserRef->noptype == 0)
					{
						pSrcUserRef->nprivate_userid = 0;
						pToUsrRef->nprivate_userid = 0;
						pToUsrRef->noptype = 0;
						pSrcUserRef->noptype = 0;
					}
					else
					{
						nret = ERR_CODE_INVALID_PARAMETER;
					}
				}
				break;
			case INVT_MIC_CANCEL://取消邀请
				{
					if(pSrcUserRef->noptype == 1 || pSrcUserRef->noptype == 0)
					{
						pToUsrRef->noptype = 0;
						pSrcUserRef->noptype = 0;
					}
					else
					{
						nret = ERR_CODE_INVALID_PARAMETER;
					}
				}
				break;
			case INVT_MIC_FORCE_HANGUP:
				{
						pToUsrRef->noptype = 0;
						pSrcUserRef->noptype = 0;
						pSrcUserRef->nprivate_userid = 0;
						pToUsrRef->nprivate_userid = 0;
				}
				break;
			default:
				nret = ERR_CODE_INVALID_PARAMETER;
				LOG_PRINT(log_error,"room_tag %d optype invalid.",pReq.vcbid());
			}

	        pToUsrRef->updateOnlineInfo();
		}
		else
		{
			if(optype == INVT_MIC_FORCE_HANGUP)
			{
				pSrcUserRef->noptype = 0;
				pSrcUserRef->nprivate_userid = 0;
			}else
			{
				nret = ERR_CODE_FAILED_USERNOTFOUND;
			}
			LOG_PRINT(log_error,"room_tag %d touserid:%d invalid.",pReq.touserid());
		}
	    pSrcUserRef->updateOnlineInfo();
	}
	else if(pToUsrRef.get() != 0)
	{
		pToUsrRef->noptype = 0;
		pToUsrRef->nprivate_userid = 0;
		pToUsrRef->updateOnlineInfo();
	}
	pReq.mutable_errinfo()->set_errid(nret);
	SERIALIZETOARRAY_BUF(pReq, respbuf, len);
	pRoomMgr->Build_NetMsg(szBuf, 512, MDM_Vchat_Room, Sub_Vchat_InviteOnMicNoty, &pGateMask, respbuf, len);
	if(bself)
	{
		CMsgComm::Build_BroadCastUser_Gate(pGateMask, pReq.runuserid(), e_Notice_AllType, 0);
		pRoomMgr->castGateway((COM_MSG_HEADER *)szBuf);//to toid
	}else
	{
		CMsgComm::Build_BroadCastUser_Gate(pGateMask, pReq.runuserid(), e_Notice_AllType, 0);
		pRoomMgr->castGateway((COM_MSG_HEADER *)szBuf);//to toid

		CMsgComm::Build_BroadCastUser_Gate(pGateMask, pReq.touserid(), e_Notice_AllType, 0);
		pRoomMgr->castGateway((COM_MSG_HEADER *)szBuf);//to toid
	}
	LOG_PRINT(log_info, "room_tag %d invite %d on room:%d mic.", pReq.runuserid(), pReq.touserid(), pReq.vcbid());
	if(pRoomObjRef.get() != 0)
		pRoomObjRef->redisSetRoomInfo();
	LOG_PRINT(log_error, "room_tag invite end.");
	return 0;
}

int CLogicCore::proc_setmicposStatus_req(task_proc_data* message)
{
	CRoomManager* pRoomMgr = m_pSvrEvent->getRoomMgr();
	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)(message->pdata);
	if (pHead->length < SIZE_IVM_HEAD_TOTAL)
	{
		LOG_PRINT(log_warning, "parse msginfo fixed length failed, need at lease %d but recv %d!", SIZE_IVM_TOTAL(CMDSetMicPosStatusReq), pHead->length);
		return -1;
	}

	ClientGateMask_t * pGateMask = (ClientGateMask_t *)(pHead->content);
	CMDSetMicPosStatusReq pReq;
	pReq.ParseFromArray(pHead->content + SIZE_IVM_CLIENTGATE, SIZE_IVM_REQUEST(pHead));

	int vcbid = pReq.vcbid();
	int userid = pReq.userid();
	int index = pReq.index();
	int status = pReq.status();

	RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(vcbid);
	if(pRoomObjRef.get() == 0)
	{
		LOG_PRINT(log_error, "not found groupid:%d, userid:%d.", vcbid, userid);
//		message->connection->resperrinf(in_msg, pGateMask, ERR_CODE_FAILED_ROOMIDNOTFOUND);
		return -1;
	}
	ClientGateMask_t* pClientGate;
	char szBuf[512] = {0};
	int ret = pRoomObjRef->setMicposStatus(index,status);

	if(ret) 
	{
		//操作成功
		CMDSetMicPosStatusResp respInfo;
		respInfo.set_vcbid(vcbid);
		respInfo.mutable_errinfo()->set_errid(0);
		SERIALIZETOARRAY_BUF(respInfo,respbuf,len);
		int nMsgLen=pRoomMgr->Build_NetMsg(szBuf,512,MDM_Vchat_Room,Sub_Vchat_SetMicPosStatusResp,&pClientGate,respbuf,len);
		if(nMsgLen > 0)
		{
			pClientGate->param1=pGateMask->param1;
			pClientGate->param2=pGateMask->param2;
			message->connection->write_message(szBuf, nMsgLen);

			//操作通知
			CMDSetMicPosStatusNoty noty;
			noty.set_vcbid(vcbid);
			noty.set_userid(userid);
			noty.set_index(index);
			noty.set_status(status);
			SERIALIZETOARRAY_BUF(noty,respbuf,len);
			nMsgLen=pRoomMgr->Build_NetMsg(szBuf,512,MDM_Vchat_Room,Sub_Vchat_SetMicPosStatusNoty,&pClientGate,respbuf,len);
			pRoomObjRef->castSendMsgXXX((COM_MSG_HEADER*)szBuf);
		}
		pRoomObjRef->castSendMicList();
	}
	else 
	{
		//操作失败
		CMDSetMicPosStatusResp respInfo;
		respInfo.set_vcbid(vcbid);
		respInfo.mutable_errinfo()->set_errid(ERR_SET_MIC_POS);
		SERIALIZETOARRAY_BUF(respInfo, respbuf, len);
		int nMsgLen = pRoomMgr->Build_NetMsg(szBuf, 512, MDM_Vchat_Room, Sub_Vchat_SetMicPosStatusResp, &pClientGate, respbuf, len);
		if(nMsgLen > 0)
		{
			pClientGate->param1 = pGateMask->param1;
			pClientGate->param2 = pGateMask->param2;
			message->connection->write_message(szBuf, nMsgLen);
		}
	}
	return 1;
}

int CLogicCore::proc_setmicmode_req(task_proc_data* message)
{
	CRoomManager* pRoomMgr = m_pSvrEvent->getRoomMgr();
	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)(message->pdata);
	if (pHead->length < SIZE_IVM_HEAD_TOTAL)
	{
		LOG_PRINT(log_warning, "parse msginfo fixed length failed, need at lease %d but recv %d!", SIZE_IVM_TOTAL(CMDSetMicModeReq), pHead->length);
		return -1;
	}

	ClientGateMask_t * pGateMask = (ClientGateMask_t *)(pHead->content);
	CMDSetMicModeReq pReq;
	pReq.ParseFromArray(pHead->content + SIZE_IVM_CLIENTGATE, SIZE_IVM_REQUEST(pHead));

	int vcbid = pReq.vcbid();
	int userid = pReq.userid();
	e_MicMode micmode = pReq.micmode();

	RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(vcbid);
	if(pRoomObjRef.get() == 0)
	{
		LOG_PRINT(log_error, "not found groupid:%d, userid:%d.",vcbid, userid);
//		message->connection->resperrinf(in_msg, pGateMask, ERR_CODE_FAILED_ROOMIDNOTFOUND);
		return -1;
	}

	ClientGateMask_t* pClientGate;
	char szBuf[512] = {0};
	int ret = pRoomObjRef->setMicMode(micmode);

	if(ret) {
		//1表示操作成功
		CMDSetMicModeResp respInfo;
		respInfo.set_vcbid(vcbid);
		respInfo.set_userid(userid);
		respInfo.mutable_errinfo()->set_errid(0);
		SERIALIZETOARRAY_BUF(respInfo, respbuf, len);
		int nMsgLen = pRoomMgr->Build_NetMsg(szBuf, 512, MDM_Vchat_Room, Sub_Vchat_SetMicModeResp, &pClientGate, respbuf, len);
		if(nMsgLen > 0)
		{
			pClientGate->param1 = pGateMask->param1;
			pClientGate->param2 = pGateMask->param2;
			message->connection->write_message(szBuf, nMsgLen);

			//操作通知
			CMDSetMicModeNoty noty;
			noty.set_vcbid(vcbid);
			noty.set_userid(userid);
			noty.set_micmode(micmode);
			SERIALIZETOARRAY_BUF(noty,respbuf,len);
			nMsgLen=pRoomMgr->Build_NetMsg(szBuf,512,MDM_Vchat_Room,Sub_Vchat_SetMicModeNoty,&pClientGate,respbuf,len);
			pRoomObjRef->castSendMsgXXX((COM_MSG_HEADER*)szBuf);
		}
	}
	else {
		//0表示操作失败
		CMDSetMicModeResp respInfo;
		respInfo.set_vcbid(vcbid);
		respInfo.set_userid(userid);
		respInfo.mutable_errinfo()->set_errid(ERR_ROOM_SET_MIC_MODE);
		SERIALIZETOARRAY_BUF(respInfo, respbuf, len);
		int nMsgLen = pRoomMgr->Build_NetMsg(szBuf, 512, MDM_Vchat_Room, Sub_Vchat_SetMicModeResp, &pClientGate, respbuf, len);
		if(nMsgLen > 0)
		{
			pClientGate->param1 = pGateMask->param1;
			pClientGate->param2 = pGateMask->param2;
			message->connection->write_message(szBuf, nMsgLen);
		}
	}

	return 0;
}

int CLogicCore::proc_getroominfo_ext_req(task_proc_data* message)
{
	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)(message->pdata);
	if (pHead->length < SIZE_IVM_HEAD_TOTAL)
	{
		LOG_PRINT(log_warning, "parse msginfo fixed length failed, need at lease %d but recv %d!", SIZE_IVM_TOTAL(CMDMicStateListReq), pHead->length);
		return -1;
	}

	ClientGateMask_t * pGateMask = (ClientGateMask_t *)(pHead->content);
	CMDRoomInfoExtReq pReq;
	pReq.ParseFromArray(pHead->content + SIZE_IVM_CLIENTGATE, SIZE_IVM_REQUEST(pHead));

	RoomObject_Ref pRoomObjRef= m_pSvrEvent->getRoomMgr()->FindRoom(pReq.vcbid());
	if(pRoomObjRef.get()== 0)
	{
		LOG_PRINT(log_error, "<<<<<< proc_getroominfo_ext_req() end 1.");
		return 0;
	}

	CMDRoomInfoExtResp resp;
	ClientGateMask_t* pClientGate;
	char szBuf[512] = {0};
	resp.set_vcbid(pReq.vcbid());
	if(pRoomObjRef->m_micMode <0 || pRoomObjRef->m_micMode >2)
	{
		pRoomObjRef->m_micMode = e_MicMode(0);
		pRoomObjRef->updateMicMode();
	}
	resp.set_micmode(pRoomObjRef->m_micMode);
	resp.set_name(pRoomObjRef->name);
	resp.set_icon(pRoomObjRef->icon);
	int micSec = pRoomObjRef->getInterval();
	LOG_PRINT(log_info, "room_tag <<<<<< proc_getroominfo_ext_req() micsec:%d.",micSec);
	resp.set_micsec(micSec);
	SERIALIZETOARRAY_BUF(resp,respbuf,len);
	int nMsgLen= m_pSvrEvent->getRoomMgr()->Build_NetMsg(szBuf,512,MDM_Vchat_Room,Sub_Vchat_getRoomInfoExtResp,&pClientGate,respbuf,len);
	if(nMsgLen>0)
	{
		pClientGate->param1=pGateMask->param1;
		pClientGate->param2=pGateMask->param2;
		message->connection->write_message(szBuf,nMsgLen);
	}
	return 1;
}

int CLogicCore::proc_micstatelist_req(task_proc_data *message)
{
	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)(message->pdata);
	if (pHead->length < SIZE_IVM_HEAD_TOTAL)
	{
		LOG_PRINT(log_warning, "parse msginfo fixed length failed, need at lease %d but recv %d!", SIZE_IVM_HEAD_TOTAL, pHead->length);
		return -1;
	}

	ClientGateMask_t * pGateMask = (ClientGateMask_t *)(pHead->content);
	CMDMicStateListReq pReq;
	pReq.ParseFromArray(pHead->content + SIZE_IVM_CLIENTGATE, SIZE_IVM_REQUEST(pHead));

	RoomObject_Ref pRoomObjRef = m_pSvrEvent->getRoomMgr()->FindRoom(pReq.vcbid());
	if(pRoomObjRef.get() == 0)
	{
        LOG_PRINT(log_error, "room_tag<<<<< no found room end 1.");
		return 0;
	}
	std::vector<PubMicState_t> list;
	pRoomObjRef->getMicList(list);

	CMDMicStateListResp resp;
	ClientGateMask_t * pClientGate;
	char szBuf[512] = {0};
	resp.set_vcbid(pReq.vcbid());
	for(int i = 0 ; i < list.size() ; i++)
	{
		CMDMicStateListResp_PubMic * pm = resp.add_pubmic();
		std::string strHead;
		std::string strAlias;
		CUserBasicInfo::getUserHeadAndAlias(list[i].userid, strHead, strAlias);
		pm->mutable_userinfo()->set_userid(list[i].userid);
		pm->mutable_userinfo()->set_head(strHead);
		pm->mutable_userinfo()->set_alias(strAlias);
		pm->set_status(list[i].status);
	}
	LOG_PRINT(log_info, "room_tag<<<<<< proc_micstatelist_req() end size:%d,,%d.", list.size(),pReq.vcbid());
	SERIALIZETOARRAY_BUF(resp, respbuf, len);
	int nMsgLen = m_pSvrEvent->getRoomMgr()->Build_NetMsg(szBuf, 512, MDM_Vchat_Room, Sub_Vchat_MicStateListResp, &pClientGate, respbuf, len);
	if(nMsgLen > 0)
	{
		pClientGate->param1 = pGateMask->param1;
		pClientGate->param2 = pGateMask->param2;
		message->connection->write_message(szBuf,nMsgLen);
	}
	LOG_PRINT(log_info, "<<<<<< proc_micstatelist_req() end 2.");
	return 1;
}

int CLogicCore::proc_groupmember_req(task_proc_data * message)
{
	if (!message || !message->connection)
	{
		LOG_PRINT(log_error, "ERROR:message or connection is null.");
		return 0;
	}

	COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *) message->pdata;
	ClientGateMask_t * pGateMask = (ClientGateMask_t *)(in_msg->content);
	if(in_msg->length <= SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE)
	{
		LOG_PRINT(log_error, "proc_groupmember_req packet is error!,length=%d,required at least:%u", in_msg->length, SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE);
		message->connection->resperrinf(in_msg, pGateMask, ERR_CODE_FAILED_PACKAGEERROR);
		return -1;
	}

	char * pReqData = (char *)(in_msg->content + SIZE_IVM_CLIENTGATE);
	unsigned int reqLen = SIZE_IVM_REQUEST(in_msg);
	CMDGroupMemberReq reqData;
	reqData.ParseFromArray(pReqData, reqLen);
	LOG_PRINT(log_info, "userid:%u request groupid:%u member info list.", reqData.userid(), reqData.groupid());

	if (0 == reqData.groupid()) 
	{
		LOG_PRINT(log_error, "proc_groupmember_req input error.groupid must not be 0");
		message->connection->resperrinf(in_msg, pGateMask, ERR_CODE_FAILED_ROOMIDNOTFOUND);
		return -1;
	}

	RoomObject_Ref pRoomObjRef = m_pSvrEvent->getRoomMgr()->FindRoom(reqData.groupid());
	if(pRoomObjRef.get() == 0) 
	{
		LOG_PRINT(log_error, "not found groupid:%d, userid:%d.", reqData.groupid(), reqData.userid());
		message->connection->resperrinf(in_msg, pGateMask, ERR_CODE_FAILED_ROOMIDNOTFOUND);
		return -1;
	}

	pRoomObjRef->sendMemberList(message, reqData.userid(), pGateMask->param1, pGateMask->param2);
	return 0;
}

int CLogicCore::proc_queryUserGroupInfo_req(task_proc_data * message)
{
	if (!message || !message->connection)
	{
		LOG_PRINT(log_error, "ERROR:message or connection is null.");
		return 0;
	}

	COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *) message->pdata;
	ClientGateMask_t * pGateMask = (ClientGateMask_t *)(in_msg->content);
	if(in_msg->length <= SIZE_IVM_HEAD_TOTAL)
	{
		LOG_PRINT(log_error, "handle_queryUserGroupInfo_req packet is error!,length=%d,required at least:%u", in_msg->length, SIZE_IVM_HEAD_TOTAL);
		message->connection->resperrinf(in_msg, pGateMask, ERR_CODE_FAILED_PACKAGEERROR);
		return -1;
	}

	char * pReqData = (char *)(in_msg->content + SIZE_IVM_CLIENTGATE);
	unsigned int reqLen = SIZE_IVM_REQUEST(in_msg);
	CMDUserGroupInfoReq reqData;
	reqData.ParseFromArray(pReqData, reqLen);
	LOG_PRINT(log_info, "client request userid:%u info in groupid:%u.", reqData.userid(), reqData.groupid());

	unsigned int ret = 0;
	CMDRoomUserInfo oGroupInfo;
	do 
	{
		if (0 == reqData.groupid()) 
		{
			LOG_PRINT(log_error, "proc_groupmember_req input error.groupid must not be 0");
			ret = ERR_CODE_FAILED_ROOMIDNOTFOUND;
			break;
		}

		RoomObject_Ref pRoomObjRef = m_pSvrEvent->getRoomMgr()->FindRoom(reqData.groupid());
		if(pRoomObjRef.get() == 0) 
		{
			LOG_PRINT(log_error, "not found groupid:%d,userid:%d.", reqData.groupid(), reqData.userid());
			ret = ERR_CODE_FAILED_ROOMIDNOTFOUND;
			break;
		}

		ret = pRoomObjRef->queryUserGroupInfo(reqData.userid(), oGroupInfo);
	} while (0);

	CMDUserGroupInfoResp oRspData;
	oRspData.mutable_errinfo()->set_errid(ret);
	CMDRoomUserInfo * pData = oRspData.mutable_userinfo();
	pData->CopyFrom(oGroupInfo);

	unsigned int rspDataLen = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + oRspData.ByteSize();
	SL_ByteBuffer buff(rspDataLen);
	buff.data_end(rspDataLen);

	COM_MSG_HEADER * pRspHead = (COM_MSG_HEADER *)buff.buffer();
	ClientGateMask_t * pRspGate = (ClientGateMask_t *)(pRspHead->content);
	GEN_RSP_MSGINFO_SET_HEAD_GATE(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_QryUserGroupInfoResp, in_msg->reqid, pGateMask, pRspHead, pRspGate, oRspData);

	message->connection->write_message(buff);
	return 0;
}

int CLogicCore::proc_setGroupMsgMute_req(task_proc_data * message)
{
	if (!message || !message->connection)
	{
		LOG_PRINT(log_error, "ERROR:message or connection is null.");
		return -1;
	}

	COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *) message->pdata;
	ClientGateMask_t * pGateMask = (ClientGateMask_t *)(in_msg->content);
	if(in_msg->length <= SIZE_IVM_HEAD_TOTAL)
	{
		LOG_PRINT(log_error, "proc_setGroupMsgMute_req packet is error!,length=%d,required at least:%u", in_msg->length, SIZE_IVM_HEAD_TOTAL);
		message->connection->resperrinf(in_msg, pGateMask, ERR_CODE_FAILED_PACKAGEERROR);
		return -1;
	}

	char * pReqData = (char *)(in_msg->content + SIZE_IVM_CLIENTGATE);
	unsigned int reqLen = SIZE_IVM_REQUEST(in_msg);
	CMDSetGroupMsgMuteReq reqData;
	reqData.ParseFromArray(pReqData, reqLen);
	LOG_PRINT(log_info, "client request group msg mute setting.userid:%u,groupid:%u,muteset:%u.", reqData.userid(), reqData.groupid(), reqData.muteset());

	unsigned int errID = 0;
	if (!CDBSink().updateGroupMsgMute(reqData.userid(), reqData.groupid(), reqData.muteset()))
	{
		errID = ERR_CODE_SET_GROUP_MSG_MUTE;
	}

	CMDSetGroupMsgMuteResp oRspData;
	oRspData.mutable_errinfo()->set_errid(errID);
	oRspData.set_userid(reqData.userid());
	oRspData.set_groupid(reqData.groupid());

	unsigned int rspDataLen = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + oRspData.ByteSize();
	SL_ByteBuffer buff(rspDataLen);
	buff.data_end(rspDataLen);

	COM_MSG_HEADER * pRspHead = (COM_MSG_HEADER *)buff.buffer();
	ClientGateMask_t * pRspGate = (ClientGateMask_t *)(pRspHead->content);
	GEN_RSP_MSGINFO_SET_HEAD_GATE(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_SetGroupMsgMuteResp, in_msg->reqid, pGateMask, pRspHead, pRspGate, oRspData);
	message->connection->write_message(buff);
	return 0;
}

int CLogicCore::proc_changewaitmicindex_req(task_proc_data *task_node)
{
	CRoomManager* pRoomMgr = m_pSvrEvent->getRoomMgr();
	COM_MSG_HEADER* in_msg = (COM_MSG_HEADER*)task_node->pdata;
	ClientGateMask_t* pGateMask=(ClientGateMask_t*)(in_msg->content);
	CMDOperateWaitMic pReq;
	pReq.ParseFromArray(in_msg->content + SIZE_IVM_CLIENTGATE, SIZE_IVM_REQUEST(in_msg));
	LOG_PRINT(log_debug, "room_tag>>>>>> proc_changewaitmicindex_req().");

	int vcbid = pReq.vcbid();
	int ruunerid = pReq.ruunerid();
	int userid = pReq.userid();
	int micid = pReq.micid();
	int optype = pReq.optype();
	RoomObject_Ref pRoomObjRef=pRoomMgr->FindRoom(vcbid);
	if(pRoomObjRef.get()== 0) 
	{
        LOG_PRINT(log_error, "room_tag<<<<<< proc_changewaitmicindex_req() end 1.");
		return 0;
	}
	LOG_PRINT(log_info, "room_tag<<<<<< proc_changewaitmicindex_req() end 01.");
	UserObject_Ref pSrcUserRef, pToUsrRef;
	int nRet = -1;
	bool bRet = false;
	LOG_PRINT(log_info, "room_tag<<<<<< pSrcUserRef()  01:ret =%d",nRet);
	if(!pRoomObjRef->findMemberUser(pSrcUserRef, ruunerid))	//操作用户必须存在
	{
		LOG_PRINT(log_info, "room_tag<<<<<< proc_changewaitmicindex_req() 01:xxxxxxx =%d",pRoomObjRef->allow_visitor_on_mic);
		if(pRoomObjRef->allow_visitor_on_mic)
		{
			LOG_PRINT(log_info, "room_tag<<<<<< proc_changewaitmicindex_req() 01:allow_visitor_on_mic =%d",pRoomObjRef->allow_visitor_on_mic);
			if(!pRoomObjRef->findVisitUser(pSrcUserRef, ruunerid))
			{
				LOG_PRINT(log_warning, "room_tag[room:%u,runner:%u,touser:%u]not found runuser", pReq.vcbid(), pReq.ruunerid(), pReq.userid());
				nRet = ERR_CODE_FAILED_USERNOTFOUND;
			}else
			{
				LOG_PRINT(log_info, "room_tag<<<<<< proc_changewaitmicindex_req() findvisitor 01:allow_visitor_on_mic =%d",pRoomObjRef->allow_visitor_on_mic);
			}
		}
		else
		{
			LOG_PRINT(log_warning, "room_tag[room:%u,runner:%u,touser:%u]not allow_visitor_on_mic", pReq.vcbid(), pReq.ruunerid(), pReq.userid());
			nRet = ERR_NOT_ALLOW_VISIT_ON_MIC;
		}
	}else
	{
		LOG_PRINT(log_info, "room_tag<<<<<< findUserOnline() 01:xxxxxxx =%d",pRoomObjRef->allow_visitor_on_mic);
		UserObject_Ref pTemp;
		pTemp = CUserObj::findUserOnline(pReq.userid());
		if(pTemp.get() != 0)
		{
			if(pTemp->nprivate_userid > 0)
			{
				bRet = false;
				nRet = ERR_USER_ON_PRIVATE_LINK;
			}
		}
	}
	LOG_PRINT(log_info, "room_tag<<<<<< proc_changewaitmicindex_req() delWaitMicUser 01optype:%d,,,:ret =%d",optype,nRet);
	if(nRet < 0)
	{
		if(pRoomObjRef->findUser(pToUsrRef, userid))//被操作用户必须存在
		{
			//TODO: 判断无对象权限:是否可以操作麦序
			if(optype == -2)
			{
				bRet = pRoomObjRef->delWaitMicUser(userid,-1);
				LOG_PRINT(log_info, "room_tag<<<<<< proc_changewaitmicindex_req() delWaitMicUser 01.");
			}else if(optype == 0)
			{
				LOG_PRINT(log_info, "room_tag<<<<<< proc_changewaitmicindex_req() add 01.");
				bRet =  pRoomObjRef->addWaitMicUser(userid,micid,true);
			}
			else if(optype == -1)
			{
				bRet = pRoomObjRef->delWaitMicUser(userid,-1);
//				bRet = pRoomObjRef->delWaitMicUser_new(userid, micid);
			}
			else if(optype == -3)
			{
				pRoomObjRef->delAllWaitMicUser();
				bRet = true;
			}
			else
			{
				bRet= pRoomObjRef->changeWaitMicIndex_new(userid,micid, pReq.optype());
			}
		}
	}

	char szBuf[512] = {0};
	ClientGateMask_t* pClientGate;
	//返回结果
	if(bRet) 
	{
		//操作成功
		CMDChangeWaitMicIndexResp respInfo;
		respInfo.set_vcbid(vcbid);
		respInfo.mutable_errinfo()->set_errid(0);
		SERIALIZETOARRAY_BUF(respInfo,respbuf,len);
		int nMsgLen=pRoomMgr->Build_NetMsg(szBuf,512,MDM_Vchat_Room,Sub_Vchat_ChangeWaitMicIndexResp,&pClientGate,respbuf,len);
		if(nMsgLen > 0)
		{
			pClientGate->param1 = pGateMask->param1;
			pClientGate->param2 = pGateMask->param2;
			task_node->connection->write_message(szBuf, nMsgLen);

			//操作通知
			CMDChangeWaitMicIndexNoty noty;
			noty.set_vcbid(vcbid);
			noty.set_ruunerid(ruunerid);
			noty.set_userid(userid);
			noty.set_micid(micid);
			noty.set_optype(pReq.optype());
			SERIALIZETOARRAY_BUF(noty, respbuf, len);
			nMsgLen = pRoomMgr->Build_NetMsg(szBuf,512,MDM_Vchat_Room,Sub_Vchat_ChangeWaitMicIndexNotify,&pClientGate,respbuf,len);
			pRoomObjRef->castSendMsgXXX((COM_MSG_HEADER *)szBuf);

			//重新发排麦列表
			pRoomObjRef->castSendMicList();
			LOG_PRINT(log_info, "room<<<<<< proc_changewaitmicindex_req() end 1.");
		}
	}
	else 
	{
		//操作失败
		CMDChangeWaitMicIndexResp respInfo;
		respInfo.set_vcbid(vcbid);
		respInfo.mutable_errinfo()->set_errid(nRet);
		SERIALIZETOARRAY_BUF(respInfo, respbuf, len);
		int nMsgLen = pRoomMgr->Build_NetMsg(szBuf, 512, MDM_Vchat_Room, Sub_Vchat_ChangeWaitMicIndexResp, &pClientGate, respbuf, len);
		if(nMsgLen > 0)
		{
			pClientGate->param1 = pGateMask->param1;
			pClientGate->param2 = pGateMask->param2;
			task_node->connection->write_message(szBuf, nMsgLen);
		}
	}
	LOG_PRINT(log_debug, "<<<<<< proc_changewaitmicindex_req() end 2.");
	return 0;
}

//返回错误应答
void CLogicCore::resperrinf(COM_MSG_HEADER* reqHead, ClientGateMask_t* pMask, uint16 errcode, task_proc_data *pTaskNode)
{
    if (reqHead == NULL || pMask == NULL)
		 return ;

	SL_ByteBuffer szOutBuf(512);
    COM_MSG_HEADER * pOutMsg = (COM_MSG_HEADER *)szOutBuf.buffer();
    pOutMsg->version = MDM_Version_Value;
    pOutMsg->checkcode = reqHead->checkcode;
    pOutMsg->maincmd = reqHead->maincmd;
    pOutMsg->subcmd = Sub_Vchat_Resp_ErrCode;
	pOutMsg->reqid = reqHead->reqid;

	memcpy(pOutMsg->content, pMask, SIZE_IVM_CLIENTGATE);

    CMDErrCodeResp_t * message = (CMDErrCodeResp_t*)(pOutMsg->content + SIZE_IVM_CLIENTGATE);
    memset(message, 0, sizeof(CMDErrCodeResp_t));
	message->errmaincmd = reqHead->maincmd;
    message->errsubcmd = reqHead->subcmd;
	message->errcode = errcode;

	pOutMsg->length = sizeof(COM_MSG_HEADER) + SIZE_IVM_CLIENTGATE + sizeof(CMDErrCodeResp_t);
    szOutBuf.data_end(pOutMsg->length);
    pTaskNode->connection->write_message(szOutBuf.buffer(),pOutMsg->length);
	return;
}

int CLogicCore::hello_response(task_proc_data *task_node)
{
	register_cmd_msg(task_node->connection);
	return 0;
}

void CLogicCore::register_cmd_msg(clienthandler_ptr client)
{
	std::string cmdlist = m_pSvrEvent->getApp()->m_config.getString("cmd", "cmdlist");

	std::list<std::string> cmdrange_lst;
	cmdrange_lst.clear();

	int i = 1;
	while (1)
	{
		char cmdrange[LEN128] = {0};
		sprintf(cmdrange, "cmdrange_%d", i);
		std::string strcmdrange = m_pSvrEvent->getApp()->m_config.getString("cmd", cmdrange);
		if (strcmdrange.empty())
		{
			break;
		}

		cmdrange_lst.push_back(std::string(cmdrange) + "=" + strcmdrange);
		++i;
	}

	if (cmdrange_lst.empty() && cmdlist.empty())
	{
		LOG_PRINT(log_info, "[Register cmd]this server has no cmd to register.");
		return;
	}

	char szBuf[512] = {0};
	int msglen = CMsgComm::Build_RegisterCmd_Msg(szBuf, sizeof(szBuf), e_roomsvr_type, cmdlist, cmdrange_lst);
	if (msglen > 0)
	{
		client->write_message(szBuf, msglen);
	}
}

int CLogicCore::proc_upwaitmic_req(task_proc_data *task_node)
{
	CRoomManager * pRoomMgr = m_pSvrEvent->getRoomMgr();
	COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *)task_node->pdata;
	CMDUpWaitMic pReq;
	pReq.ParseFromArray(in_msg->content + SIZE_IVM_CLIENTGATE, SIZE_IVM_REQUEST(in_msg));

	LOG_PRINT(log_debug, ">>>>>> proc_upwaitmic_req().");

	RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(pReq.vcbid());
	if(pRoomObjRef.get() == 0) 
	{
		LOG_PRINT(log_error, "<<<<<< proc_upwaitmic_req() end 1.");
		return 0;
	}

	UserObject_Ref pSrcUserRef,pToUserRef;
	int nInsertMicIndex = pReq.nmicindex();
	if(!pRoomObjRef->findMemberUser(pSrcUserRef,pReq.ruunerid()))
	{
		LOG_PRINT(log_error, "<<<<<< proc_upwaitmic_req() end 2.");
		return 0;
	}

	if(pRoomObjRef->findMemberUser(pToUserRef,pReq.touser()))
	{
		if(pRoomObjRef->getUserWaitMicNum(pReq.touser()) >= m_pSvrEvent->m_maxuserwaitmic
			|| pRoomObjRef->getWaitMicUserCount() >= m_pSvrEvent->m_maxwaitmicuser)
		{
			//排麦错误
			in_msg->subcmd = Sub_Vchat_UpWaitMicResp;
			pReq.mutable_errinfo()->set_errid(ERR_WAIT_MIC_IS_FULL);
			SERIALIZETOARRAY_GATE(pReq, in_msg);
			task_node->connection->write_message((char *)in_msg, in_msg->length);
			LOG_PRINT(log_error, "<<<<<< proc_upwaitmic_req() end 3.");
			return 0;
		}

		bool ret = pRoomObjRef->addWaitMicUser(pReq.touser(),nInsertMicIndex);
		if(ret)
		{
			//排麦成功
			in_msg->subcmd = Sub_Vchat_UpWaitMicResp;
			pReq.mutable_errinfo()->set_errid(0);
			SERIALIZETOARRAY_GATE(pReq, in_msg);
			task_node->connection->write_message((char *)in_msg, in_msg->length);

//			if(pRoomObjRef->nopstate_ & FT_ROOMOPSTATUS_OPEN_AUTOPUBMIC)
			if(pRoomObjRef->m_micMode == e_FreeMode)
			{
				pRoomObjRef->checkWaitMicToPubMic(true);
			}
			//广播排麦用户列表
//			pRoomObjRef->castSendRoomWaitMicList();
		}
		else
		{
			in_msg->subcmd = Sub_Vchat_UpWaitMicResp;
			pReq.mutable_errinfo()->set_errid(ERR_WAIT_MIC_FAILED);
			SERIALIZETOARRAY_GATE(pReq, in_msg);
			task_node->connection->write_message((char *)in_msg, in_msg->length);
			LOG_PRINT(log_error, "<<<<<< proc_upwaitmic_req() end 3.");
			return 0;
		}
	}

	LOG_PRINT(log_debug, "<<<<<< proc_upwaitmic_req() end 4.");
	return 0;
}

int CLogicCore::proc_joinroom_req(task_proc_data * task_node)
{
	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)(task_node->pdata);
	if (pHead->length < SIZE_IVM_HEAD_TOTAL)
	{
		LOG_PRINT(log_warning, "parse msginfo fixed length failed, need at lease %d but recv %d!", SIZE_IVM_TOTAL(CMDJoinRoomReq), pHead->length);
		return -1;
	}

	ClientGateMask_t * pGateMask = (ClientGateMask_t *)(pHead->content);
	CMDJoinRoomReq pReq;
	pReq.ParseFromArray(pHead->content + SIZE_IVM_CLIENTGATE, SIZE_IVM_REQUEST(pHead));

	LOG_PRINT(log_info, "[connid:%llu,gateid:%u,userid:%u,roomid:%u,devtype:%d]start to join room...",
			pGateMask->param2, m_gateid, pReq.userid(), pReq.vcbid(), pReq.devtype());

	if (0 == pReq.userid())
	{
		LOG_PRINT(log_error, "[connid:%llu,gateid:%u,userid:%u,roomid:%u,devtype:%d]join room request input error.userid must not be 0",
			pGateMask->param2, m_gateid, pReq.userid(), pReq.vcbid(), pReq.devtype());
		jr_send_resp_err(task_node, ERR_CODE_INVALID_PARAMETER);
		return -1;
	}
	
	RoomObject_Ref pRoomObjRef;
	CRoomManager * pRoomMgr = m_pSvrEvent->getRoomMgr();
	int userid = pReq.userid();
	int vcbid = pReq.vcbid();

	pRoomObjRef = pRoomMgr->FindRoom(vcbid);
	if (pRoomObjRef.get() == 0){
		LOG_PRINT(log_error, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]not found room, error:404", pGateMask->param2, m_gateid, pReq.userid(), pReq.vcbid());
		jr_send_resp_err(task_node, ERR_CODE_FAILED_ROOMIDNOTFOUND);
		return true;
	}
	uint32 live_status = 1;
	CDBSink().get_LiveStatusByID(live_status, pReq.vcbid());
	/*if (live_status == 2)
	{
		LOG_PRINT(log_error, "[connid:%llu,gateid:%u,userid:%u,roomid:%u,devtype:%d]join room request input error.live_status:%u",
			pGateMask->param2, m_gateid, pReq.userid(), pReq.vcbid(), pReq.devtype(), live_status);
		jr_send_resp_err(task_node, ERR_ROOM_IS_CLOSED);
		return true;
	}*/
	//创建用户
	LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]create user info.", pGateMask->param2, m_gateid, userid, vcbid);
	CUserObj * pUserObj_new = pRoomMgr->CreateUserFromDB(vcbid, userid);
	LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]create user info.end:%d", pGateMask->param2, m_gateid, userid, vcbid,pUserObj_new->inroomstate_);
	UserObject_Ref NewUserObjRef(pUserObj_new);
	if(0 == NewUserObjRef)
	{
		LOG_PRINT(log_error, "[connid=%llu,gateid=%u,userid=%u,roomid=%u,password:%s]Failed to join room, create user fail!",
				pGateMask->param2, m_gateid, userid, vcbid, pReq.cuserpwd().c_str());
		jr_send_resp_err(task_node, ERR_FAIL_CREATE_USER);
		return -1;
	}
	LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]create user info.end NewUserObjRef:%d", pGateMask->param2, m_gateid, userid, vcbid,NewUserObjRef->inroomstate_);

	if (jr_check_room_passwd(task_node, pRoomObjRef, NewUserObjRef, false) < 0)
		return -1;

	//用户是否重复登陆
	UserObject_Ref pUserObjRef;
	 
	bool bsame_user_and_room = false;
	int ret = jr_check_user_exist(task_node, pRoomObjRef, pUserObjRef, NewUserObjRef, bsame_user_and_room);
	if (ret < 0)
		return -1;
	/*else if (ret > 0)
	{
		sendInfoAfterJoinRoom(task_node, pRoomObjRef, userid, isMember);
		return -1;
	}*/

	string areaid = getarea(pReq.cipaddr().c_str());
	if (areaid.empty())
	{
		LOG_PRINT(log_error, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]cannot get area id!", pGateMask->param2, m_gateid, NewUserObjRef->nuserid_, vcbid);
	}

	pUserObjRef = NewUserObjRef;
	jr_reset_user_info(task_node, pRoomObjRef, pUserObjRef, areaid);

	LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]add user and write redis message.", pGateMask->param2, m_gateid, pUserObjRef->nuserid_, vcbid);
	pUserObjRef->nuserlevel_ = e_VisitorRole;
	if(!CRoomObj::findOnlineUser(pRoomMgr, pRoomMgr->getDataRedis(), userid, pUserObjRef))
	{
		if (pRoomObjRef->InsertVisitUser(pUserObjRef) < 0)
		{
			jr_send_resp_err(task_node, ERR_CODE_FAILED_REDIS);
			return 0;
		}
	}
	else
	{
		pUserObjRef->setUserInfo2Redis(pRoomMgr->getDataRedis());
	}
	LOG_PRINT(log_error, "room_tag ==================cometime[%d]",pUserObjRef->cometime_);
	LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]send all data to client.%d", pGateMask->param2, m_gateid, pUserObjRef->nuserid_, vcbid,pUserObjRef->inroomstate_);
	pRoomObjRef->sendJoinRoomResp(task_node, pUserObjRef, pGateMask->param1, pGateMask->param2);//complete
	pRoomObjRef->castSendRoomVistorCount();

	int forbit = pRoomObjRef->getForbidUserChat(userid);
	LOG_PRINT(log_info, " joinroom  procForbidUserChat  %d :%d", forbit, pRoomObjRef->talk_status);
	if (forbit)
	{
		ClientGateMask_t* pClientGate;
		char szBuf[512] = { 0 };
		CMDForbidUserChat noty;		 
		noty.set_errid(0);
		noty.set_groupid(vcbid);
		noty.set_status(forbit);
		SERIALIZETOARRAY_BUF(noty, respbuf, len);
		int nMsgLen = pRoomMgr->Build_NetMsg(szBuf, 512, MDM_Vchat_Room, Sub_Vchat_ForbidUserChatNoty, &pClientGate, respbuf, len);

		CMsgComm::Build_BroadCastUser_Gate(pClientGate, userid, e_Notice_AllType, 0);
		pRoomMgr->castGateway((COM_MSG_HEADER *)szBuf);
	}
	 
 
	 

//	}

	//下发当前在群成员列表
//	sendInfoAfterJoinRoom(task_node, pRoomObjRef, userid, isMember);
	//写入进房间记录
	LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]write the record of join room.", pGateMask->param2, m_gateid, userid, vcbid);
//	CDBSink().write_addlastgoroom_DB(pReq.userid(), pReq.vcbid(), 0);

	LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]join room complete!", pGateMask->param2, m_gateid, userid, vcbid);
	return 0;
}
void CLogicCore::genAllToken(task_proc_data * task_node)
{
	CRoomManager * pRoomMgr = m_pSvrEvent->getRoomMgr();
	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)(task_node->pdata);
	if (pHead->length < SIZE_IVM_HEAD_TOTAL)
	{
		LOG_PRINT(log_warning, "parse msginfo fixed length failed, need at lease %d but recv %d!", SIZE_IVM_TOTAL(CMDJoinRoomReq), pHead->length);
		return ;
	}

	ClientGateMask_t * pGateMask = (ClientGateMask_t *)(pHead->content);
	CMDJoinRoomReq pReq;
	pReq.ParseFromArray(pHead->content + SIZE_IVM_CLIENTGATE, SIZE_IVM_REQUEST(pHead));

	int nret = 0;
	string encrypText = "";
	LOG_PRINT(log_info, "room_tag++++++ Sub_Vchat_UserAuthReq=====11111.");
	string key = m_pSvrEvent->m_pApp->m_key;
	string appid = m_pSvrEvent->m_pApp->m_Appid;
	do{
//		string key = getAppIdKey(pReq.appid());
		LOG_PRINT(log_info, "room_tag key:%s .", key.c_str());
		LOG_PRINT(log_info, "room_tag++++++ Sub_Vchat_UserAuthReq=====11111key:%s .", key.c_str());
		initKV(key.c_str());
		LOG_PRINT(log_info, "room_tag++++++ =====Sub_Vchat_UserAuthReq===== .");
		char text[256] = {0};
		sprintf(text,"%d:%s:%d:%d:%d",m_pSvrEvent->m_pApp->m_Version,appid.c_str(),pReq.vcbid(),pReq.userid(),SL_Socket_CommonAPI::util_process_clock_ms());
		LOG_PRINT(log_info, "room_tag++++++ Sub_Vchat_UserAuthReq=====text:%s .", text);
		encrypText = encrypt(text);
		LOG_PRINT(log_info, "room_tag++++++ Sub_Vchat_UserAuthReq=====encrypText:%s .", encrypText.c_str());
		string decryptText = decrypt(encrypText);
		LOG_PRINT(log_info, "room_tag++++++ Sub_Vchat_UserAuthReq=====decryptText:%s .", decryptText.c_str());
		RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(pReq.vcbid());
		if(pRoomObjRef.get() == 0)
		{
			LOG_PRINT(log_error, "room_tag not found groupid:%d", pReq.vcbid());
			nret = ERR_CODE_FAILED_ROOMIDNOTFOUND;
			break;
		}
	}while(0);
	char szBuf[1024] = {0};
	//操作成功
	LOG_PRINT(log_info, "room_tag++++++ Sub_Vchat_UserAuthReq=====23333");
	CMDAuthResp respInfo;
	ClientGateMask_t * pClientGate;
	respInfo.set_appid(appid);
	respInfo.set_recvkey(encrypText);
	respInfo.set_sendkey(encrypText);
	respInfo.set_vcbid(pReq.vcbid());
	respInfo.set_userid(pReq.userid());
	respInfo.set_errcode(nret);
	SERIALIZETOARRAY_BUF(respInfo,respbuf,len);
	int nMsgLen=pRoomMgr->Build_NetMsg(szBuf,1024,MDM_Vchat_Room,Sub_Vchat_UserAuthResp,&pClientGate,respbuf,len);
	if(nMsgLen > 0)
	{
		pClientGate->param1=pGateMask->param1;
		pClientGate->param2=pGateMask->param2;
		LOG_PRINT(log_info, "room_tag++++++ Sub_Vchat_UserAuthReq=====write_message");
		task_node->connection->write_message(szBuf, nMsgLen);
	}
}

void CLogicCore::queryAndUpdateLiveFlowStatus()
{
	// Query request example:
	// http://livect.chinanetcenter.com/api/streamStatusRealTime.jsp?domain=xxx&n=xxx&r=xxx&k=xxx&channel=xxx&type=xxx
	// NOTE: k = md5(r+key)

	//std::string domain("push.dks.dacelue.com.cn"); // test domain
	std::string domain("push.dks.99cj.com.cn"); // formal domain
	std::string n("99caijing");
	char r[32] = { 0 };
	std::string k, key("FA3UZITV82HIB");
	//std::string channel, type("0"); // Not needed so far. Query all.
	std::string url("http://livect.chinanetcenter.com/api/streamStatusRealTime.jsp?");
	std::string resp;
	CHttpClient httpClient;

	snprintf(r, sizeof(r), "%d", time(0));
	cryptohandler::md5hash(r + key, k);
	std::transform(k.begin(), k.end(), k.begin(), ::tolower);

	url.append("domain=").append(domain)
		.append("&n=").append(n)
		.append("&r=").append(r)
		.append("&k=").append(k)
		/*.append("&channel=").append(channel)
		.append("&type=").append(type)*/;
	LOG_PRINT(log_debug, "url: %s", url.c_str());

	CURLcode retcode = (CURLcode)httpClient.Get(url, resp);

	if (CURLE_OK != retcode)
	{
		LOG_PRINT(log_error, "httpClient.Get() failed: %s", curl_easy_strerror(retcode));
		return;
	}
	LOG_PRINT(log_info, "http response: %s", resp.c_str());

	Json::Reader jreader;
	Json::Value jvalue;

	if (!jreader.parse(resp, jvalue))
	{
		LOG_PRINT(log_error, "jreader.parse() failed");
		return;
	}

	const Json::Value dataArray = jvalue["dataValue"];
	unsigned int dataCount = dataArray.size();

	if (0 == dataCount) {
		LOG_PRINT(log_warning, "no dataValue items in json string, abort");
		return;
	}

	std::string id;
	//std::vector<int> jsonGroupIds(dataCount);
	std::set<int> jsonGroupIds;

	for (int i = 0; i < dataCount; ++i)
	{
		id = dataArray[i]["id"].asString();
		std::string::size_type delimPos = id.find("_");
		if (std::string::npos == delimPos)
		{
			LOG_PRINT(log_warning, "invalid format: i = %d, id string = %s, ignore it", i, id.c_str());
			continue;
		}

		std::string groupidStr(id.substr(delimPos + 1));
		int groupid = strtol(groupidStr.c_str(), NULL, 10);

		jsonGroupIds.insert(groupid); // jsonGroupIds[i] = groupid;
		LOG_PRINT(log_debug, "whole id: %s, groupid in string format: %s,"
			" groupid in integer format: %d", id.c_str(), groupidStr.c_str(), groupid);
	}

	CDBSink dbSink;

	std::vector<int> groupIds;
	std::set<int>::iterator sitEnd = jsonGroupIds.end();
	const int ID_COUNT_PER_ROUND = 1000;
	int startRow = 0;
	int retGetGroupIds = dbSink.getLiveFlowGroupIds(startRow, ID_COUNT_PER_ROUND, groupIds);
	time_t now = time(0);
	const int STATE_START = 1/*, STATE_STOP = 2*/;

	while (retGetGroupIds > 0)
	{
		int count_start = 0, count_stop = 0;

		for (int i = 0; i < retGetGroupIds; ++i)
		{
			if (sitEnd != jsonGroupIds.find(groupIds[i]))
			{
				dbSink.updateLiveFlowState(groupIds[i], STATE_START, now);
				++count_start;
			}
			else
			{
				//dbSink.updateLiveFlowState(groupIds[i], STATE_STOP, now);
				//++count_stop;
				LOG_PRINT(log_info, "groupid[%d] is not in live start list, ignore it", groupIds[i]);
			}
		}
		LOG_PRINT(log_info, "%d flow state records was updated, %d start, %d stop",
			retGetGroupIds, count_start, count_stop);

		startRow += retGetGroupIds;
		retGetGroupIds = dbSink.getLiveFlowGroupIds(startRow, ID_COUNT_PER_ROUND, groupIds);
	}
}

void CLogicCore::pushCourseStartNotice(void)
{
	CThriftHttpClient<TpPushSvrConcurrentClient> client(e_tppushsvr_type);
	CMDCourseStartNoty appNotice;
	std::vector<uint32> vgids;
	std::vector<uint32> uids;
	std::vector<std::string> classNames;
	std::vector<std::string> teacherNames;
	std::vector<std::string> beginTimestamps;
	CDBSink dbSink;
	string urlFormatStr(dbSink.qrySysParameterString("START_COURSE"));
	string noticeTitle(dbSink.qrySysParameterString("START_COURSE_NOTICE_TITLE"));
	string noticeContentFormatStr(dbSink.qrySysParameterString("START_COURSE_NOTICE_CONTENT"));
	CHttpClient httpClient;
	string httpResp;

	dbSink.getAllBeginCourseBefore(15,18,vgids,uids,classNames,teacherNames,beginTimestamps);
	LOG_PRINT(log_debug, "room_tag getAllBeginCourseBefore:%d,time:%s",vgids.size(),getTime_H("%Y-%m-%d %H:%M:%S").c_str());

	for(int i = 0 ; i < vgids.size() ; ++i)
	{
		string url = stringFormat(urlFormatStr.c_str(), vgids[i]);

		httpClient.Post(url,"", httpResp);
		LOG_PRINT(log_debug, "Notice push to web: %s, resp: %s", url.c_str(), httpResp.c_str());

		CRoomManager* pRoomMgr = m_pSvrEvent->getRoomMgr();
		RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(vgids[i]);
		if(!pRoomObjRef.get())
		{
			LOG_PRINT(log_error, "room_tag not found groupid:%d", vgids[i]);
			continue ;
		}

		std::set<uint32> studentUserIds;

		if (dbSink.queryCourseStudents(vgids[i], studentUserIds) <= 0 || 0 == studentUserIds.size())
		{
			LOG_PRINT(log_warning, "failed to query students info, or no students found, course id = %u", vgids[i]);
			continue;
		}
		LOG_PRINT(log_debug, "course id: %u, student count: %u", vgids[i], studentUserIds.size());

		string noticeContent = stringFormat(noticeContentFormatStr.c_str(),
			classNames[i].c_str(), beginTimestamps[i].c_str(), teacherNames[i].c_str());

		appNotice.set_groupid(vgids[i]);
		appNotice.set_userid(uids[i]);
		appNotice.set_title(noticeTitle);
		appNotice.set_content(noticeContent);
		SERIALIZETOARRAY_BUF(appNotice,respbuf,len);

		char msgBuf[8192] = {0};
		ClientGateMask_t* pClientGate;

		pRoomMgr->Build_NetMsg(msgBuf,sizeof(msgBuf),MDM_Vchat_Room,Sub_Vchat_CourseStartNoty,&pClientGate,respbuf,len);

		LOG_PRINT(log_debug, "Online notice to APP: groupid[%u], userid[%u], title[%s], content[%s]",
			appNotice.groupid(), appNotice.userid(), appNotice.title().c_str(), appNotice.content().c_str());

		for (std::set<uint32>::iterator it = studentUserIds.begin(); it != studentUserIds.end(); ++it)
		{
			LOG_PRINT(log_debug, "Sending to uid[%u] ...", *it);
			pRoomObjRef->castSendMsgXXX((COM_MSG_HEADER*)msgBuf, *it); // TODO: This does not seem to work.
			CMsgComm::Build_BroadCastUser_Gate(pClientGate, *it);
			pRoomMgr->castGateway((COM_MSG_HEADER *)msgBuf);
		}

		if (client) // TODO: This operation depends on TPPush service, which is not available now.
		{
			Json::Value root;
			Json::FastWriter writer;
			TChatMsgPush push;

			root["groupid"] = vgids[i];
			root["content"] = noticeContent;

			push.msg_type = e_PushMsgType::GroupChat;
			push.srcuid = uids[i];
			push.title = noticeTitle;
			push.description = writer.write(root);
			push.create_time = time(NULL);
			// TODO: fill up @list ...

			client->proc_notifyChatMsg(push, vgids[i], true);
			LOG_PRINT(log_debug, "Offline notice to APP: srcuid[%u], title[%s], description[%s]",
				push.srcuid, push.title.c_str(), push.description.c_str());
		}
	}
}

void CLogicCore::pushNewCourseNoticeInBatch(void)
{
	LOG_PRINT(log_info, "Not needed in new requirements");
}

void CLogicCore::notifyPhpToPushMessages(void)
{
	vector<uint32> msgIds;
	CDBSink dbSink;

	if (dbSink.getAllUnhandledPushMsg(0, 3, msgIds) < 0)
	{
		LOG_PRINT(log_warning, "failed to get unhandled push messages");
		return;
	}

	if (0 == msgIds.size())
	{
		LOG_PRINT(log_info, "no unhandled push messages found");
		return;
	}

	string phpApiDomain;

	if (dbSink.getPreloadedSysParamString(PRELOADED_SYS_PARAM_PHP_API_DOMAIN, phpApiDomain) < 0)
	{
		LOG_PRINT(log_error, "failed to get preloaded sys param");
		return;
	}

	CHttpClient httpClient;
	string httpResp;

	for (unsigned int i = 0; i < msgIds.size(); ++i)
	{
		string url = stringFormat((phpApiDomain + "/SendChatMessage/pushMsg?id=%u").c_str(), msgIds[i]);

		httpClient.Post(url, "", httpResp);
		LOG_PRINT(log_debug, "Notice to PHP: %s, resp: %s", url.c_str(), httpResp.c_str());
	}
}

int CLogicCore::proc_joingroup_req(task_proc_data * message)
{
	if (!message || !message->connection)
	{
		LOG_PRINT(log_error, "proc_joingroup_req ERROR:message or connection is null.");
		return -1;
	}

	COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *) message->pdata;
	ClientGateMask_t * pGateMask = (ClientGateMask_t *)(in_msg->content);
	if(in_msg->length <= SIZE_IVM_HEAD_TOTAL)
	{
		LOG_PRINT(log_error, "proc_setGroupMsgMute_req packet is error!,length=%d,required at least:%u", in_msg->length, SIZE_IVM_HEAD_TOTAL);
		message->connection->resperrinf(in_msg, pGateMask, ERR_CODE_FAILED_PACKAGEERROR);
		return -1;
	}

	char * pReqData = (char *)(in_msg->content + SIZE_IVM_CLIENTGATE);
	unsigned int reqLen = SIZE_IVM_REQUEST(in_msg);
	CMDJoinGroupReq reqData;
	reqData.ParseFromArray(pReqData, reqLen);
	LOG_PRINT(log_info, "client request join group.userid:%u,groupid:%u.", reqData.userid(), reqData.groupid());
	unsigned int ret = 0;

	do 
	{
		if (!reqData.userid() || !reqData.groupid())
		{
			LOG_PRINT(log_error, "client request join group input error.userid:%u,groupid:%u.", reqData.userid(), reqData.groupid());
			ret = ERR_CODE_INVALID_PARAMETER;
			break;
		}

		ret = proc_joingroup_req(reqData.userid(), reqData.groupid(), e_MemberRole, true);

	} while (0);

	CMDJoinGroupResp oRspData;
	oRspData.mutable_errinfo()->set_errid(ret);
	oRspData.set_userid(reqData.userid());
	oRspData.set_groupid(reqData.groupid());

	unsigned int rspDataLen = SIZE_IVM_HEAD_TOTAL + oRspData.ByteSize();
	SL_ByteBuffer buff(rspDataLen);
	buff.data_end(rspDataLen);

	COM_MSG_HEADER * pRspHead = (COM_MSG_HEADER *)buff.buffer();
	ClientGateMask_t * pRspGate = (ClientGateMask_t *)(pRspHead->content);
	GEN_RSP_MSGINFO_SET_HEAD_GATE(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_JoinGroupResp, in_msg->reqid, pGateMask, pRspHead, pRspGate, oRspData);
	message->connection->write_message(buff);
	return 0;
}

bool CLogicCore::userAllowJoinGroup(unsigned int userid, RoomObject_Ref & roomObjRef, unsigned int & errCode)
{
	errCode = 0;
	if (roomObjRef.get() == 0 || !userid)
	{
		LOG_PRINT(log_error, "user allow join group input error.room obj is null or userid is 0.userid:%u.", userid);
		errCode = ERR_CODE_INVALID_PARAMETER;
		return false;
	}

	unsigned int authType = 0;
	unsigned int redpacketNum = 0;
	unsigned int redpacketTotal = 0;
	int gender = CUserBasicInfo::getUserGender(userid);
	switch(gender)
	{
	case e_MaleGender:
		authType = roomObjRef->male_join_group_auth;
		redpacketNum = roomObjRef->male_red_packet_num;
		redpacketTotal = roomObjRef->male_red_packet_total;
		break;
	case e_FemaleGender:
		authType = roomObjRef->female_join_group_auth;
		redpacketNum = roomObjRef->female_red_packet_num;
		redpacketTotal = roomObjRef->female_red_packet_total;
		break;
	default:
		LOG_PRINT(log_error, "cannot get user gender.userid:%u.", userid);
		errCode = ERR_CODE_FAILED_USERNOTFOUND;
		return false;
	}

	switch(authType)
	{
	case e_JoinGroupAuthType::NoNeedAuth:
		LOG_PRINT(log_info, "not need to auth to join group.userid:%u,gender:%d.", userid, gender);
		return true;
	case e_JoinGroupAuthType::RedPacket:
		if (redpacketNum || redpacketTotal)
		{
			LOG_PRINT(log_info, "redpacket auth to join group and redpacket setting is not empty.red packet num:%u,red packet total:%u.userid:%u,gender:%d.", \
				redpacketNum, redpacketTotal, userid, gender);
			errCode = ERR_CODE_JOINGROUP_NEED_PACKET;
			return false;
		}
		else
		{
			LOG_PRINT(log_info, "redpacket auth to join group but redpacket setting is empty.userid:%u,gender:%d.", userid, gender);
			return true;
		}
		break;
	case e_JoinGroupAuthType::VideoAuth:
		{
			bool bRet = CDBSink().checkUserHasAuth(userid, roomObjRef->nvcbid_, 0);
			if (!bRet)
			{
				errCode = ERR_CODE_JOINGROUP_NEED_VIDEOAUTH;
			}
			return bRet;
		}
		break;
	case e_JoinGroupAuthType::MsgAuth:
		{
			bool bRet = CDBSink().checkUserHasAuth(userid, roomObjRef->nvcbid_, 3);
			if (!bRet)
			{
				errCode = ERR_CODE_JOINGROUP_NEED_MSGAUTH;
			}
			return bRet;
		}
		break;
	case e_JoinGroupAuthType::Forbidden:
		{
			LOG_PRINT(log_warning, "forbidden to join group.userid:%u,gender:%d.", userid, gender);
			errCode = ERR_CODE_NOT_ALLOW_JOINGROUP;
			return false;
		}
		break;
	default:
		return true;
	}	
}

int CLogicCore::proc_joingroup_req(unsigned int userid, unsigned int groupid, unsigned int iRoleType, bool checkInGroup, int inGroupWay /* = 0*/)
{
	if (!userid || !groupid || !m_pSvrEvent)
	{
		LOG_PRINT(log_error, "proc_joingroup_req input error.userid:%u,groupid:%u.", userid, groupid);
		return ERR_CODE_INVALID_PARAMETER;
	}

	RoomObject_Ref pRoomObjRef;
	pRoomObjRef = m_pSvrEvent->m_roomMgr.FindRoom(groupid);
	if(pRoomObjRef.get() == 0) 
	{
		LOG_PRINT(log_error, "[proc_joingroup_req error]not found room.user[%u] group[%u].", userid, groupid);
		return ERR_CODE_FAILED_ROOMIDNOTFOUND;
	}

	if (pRoomObjRef->findMemberUser(userid))
	{
		if (inGroupWay == e_JoinGroupAuthType::VideoAuth)
		{
			proc_handleAuthState(userid, groupid, 1);
		}
		LOG_PRINT(log_warning, "[proc_joingroup_req]user is group member, so not need to handle it.user[%u] group[%u].", userid, groupid);
		return 0;
	}

	unsigned int errCode = ERR_CODE_SUCCESS;
	if (checkInGroup && !userAllowJoinGroup(userid, pRoomObjRef, errCode))
	{
		LOG_PRINT(log_error, "[proc_joingroup_req error]user allow join group error:%u.userid:%u,groupid:%u.", errCode, userid, groupid);
		return errCode;
	}

	LOG_PRINT(log_info, "[proc_joingroup_req]userid:%u,groupid:%u,roleType:%u,need to check join group packet:%d.", userid, groupid, iRoleType, (int)checkInGroup);
	stJoinGroupInfo oJoinObj = {0};
	oJoinObj.userID = userid;
	oJoinObj.groupID = groupid;
	oJoinObj.role_type = iRoleType;
	oJoinObj.in_way = inGroupWay;
	int ret = CUserGroupMgr::procUserJoinGroup(oJoinObj);
	if (ret < 0)
	{
		LOG_PRINT(log_error, "[proc_joingroup_req error]join group error.userid:%u,groupid:%u.", userid, groupid);
		return ERR_CODE_USER_JOIN_GROUP;
	}

	if (ret == 1 && groupid)
	{
		UserObject_Ref pUserObjRef;
		pRoomObjRef->findMemberUser(pUserObjRef, userid);
		if (pUserObjRef.get())
		{
			//broadcast room user info
			LOG_PRINT(log_debug, "[proc_joingroup_req]user has added to group in redis,need to notify in group.user[%u] group[%u].", userid, groupid);
			pRoomObjRef->castSendRoomUserComeInfo(pUserObjRef);

			proc_clean_visitor_user(userid, groupid);
			pRoomObjRef->castSendUserPriorityChange(userid, iRoleType);
			castJoinGoupMsg_rpc(groupid, userid);
			
			proc_joingroup_rpc(userid, groupid, iRoleType);
		}
	}

	if (inGroupWay == e_JoinGroupAuthType::VideoAuth)
	{
		proc_handleAuthState(userid, groupid, 1);
	}
	return 0;
}

void CLogicCore::proc_handleAuthState(uint32 userid, uint32 groupid, uint32 authState)
{
	CRoomManager * pRoomMgr = m_pSvrEvent->getRoomMgr();
	if (!userid || !groupid || !pRoomMgr)
	{
		return;
	}

	std::string strSql;
	strSql = stringFormat("update mc_group_member set is_auth=%u where user_id=%u and group_id=%u", authState, userid, groupid);

	CDBSink sink;
	if (!sink.run_sql(strSql.c_str()) || sink.getAffectedRow() == 0)
	{
		LOG_PRINT(log_error, "update mc_group_member fail.is_auth:%u user_id:%u group_id:%u.", authState, userid, groupid);
		return;
	}
	else
	{
		LOG_PRINT(log_info, "update mc_group_member success.is_auth:%u user_id:%u group_id:%u.", authState, userid, groupid);
	}

	CMDAuthStateNoty notify;
	notify.set_userid(userid);
	notify.set_groupid(groupid);
	notify.set_authstate(authState);

	int nLength = notify.ByteSize() + SIZE_IVM_HEAD_TOTAL;

	SL_ByteBuffer buff(nLength);
	buff.data_end(nLength);
	ClientGateMask_t * pGateMask = (ClientGateMask_t *)CMsgComm::Build_COM_MSG_HEADER(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_UserAuthStateNoty, nLength);
	CMsgComm::Build_BroadCastRoomGate(pGateMask, e_Notice_AllType, groupid);
	notify.SerializeToArray(buff.buffer() + SIZE_IVM_HEAD_TOTAL, notify.ByteSize());	
	pRoomMgr->castGateway((COM_MSG_HEADER *)buff.buffer());
}

int CLogicCore::proc_afterjoinroom_req(task_proc_data * task_node)
{
	GEN_MSGINFO(task_node->pdata, in_msg, pGateMask, pReq, CMDAfterJoinRoomReq_t);

	CRoomManager * pRoomMgr = m_pSvrEvent->getRoomMgr();

	unsigned int roomid = pReq->vcbid;
	unsigned int userid = pReq->userid;

	LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]after join room request", pGateMask->param2, m_gateid, userid, roomid);
	RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(roomid);
	if(pRoomObjRef.get() == 0)
	{
		resperrinf(in_msg, pGateMask, ERR_CODE_FAILED_ROOMIDNOTFOUND, task_node);
		LOG_PRINT(log_warning, "not found roomid: %d.", pReq->vcbid);
		return 0;
	}

	pRoomObjRef->sendRoomNotice(task_node, pGateMask->param1, pGateMask->param2);
	pRoomObjRef->sendRoomNoticeInfoNotify(task_node, pGateMask->param1, pGateMask->param2);
	pRoomObjRef->sendMemberList(task_node, userid, pGateMask->param1, pGateMask->param2);
	pRoomObjRef->sendTeacherUserid(task_node, pGateMask->param1, pGateMask->param2);
	pRoomObjRef->sendAdKeywordsList(task_node, pGateMask->param1, pGateMask->param2);
	pRoomObjRef->sendRoomAndSubRoomId(task_node, pGateMask->param1, pGateMask->param2);
	pRoomObjRef->castSendRoomMaxclientnum();

	return 0;
}

int CLogicCore::proc_gatejoinroom_req(task_proc_data *task_node)
{
	GEN_MSGINFO_AND_CHECK_LENGTH(task_node->pdata, in_msg, pGateMask, pReq, CMDGateJoinRoomReq_t);

	CRoomManager * pRoomMgr = m_pSvrEvent->getRoomMgr();
	if (!pRoomMgr)
	{
		LOG_PRINT(log_error, "pRoomMgr is null.");
		return 0;
	}

	RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(pReq->vcbid);
	if(pRoomObjRef.get() == 0)
	{
		LOG_PRINT(log_error, "not found roomid: %d.", pReq->vcbid);
		return 0;
	}

	UserObject_Ref userobjRef_old;
	if (!pRoomObjRef->findVisitUser(userobjRef_old, pReq->userid))
	{
		LOG_PRINT(log_error, "[vcbid:%u,userid:%u,micstate:%d]Not found user, cannot rejoin room", pReq->vcbid, pReq->userid, pReq->micstate);
		CUserObj * p = pRoomMgr->CreateUserFromDB(pReq->vcbid, pReq->userid);
		if (!p)
		{
			LOG_PRINT(log_error, "[vcbid:%u,userid:%u,micstate:%d]Failed to create user object.", pReq->vcbid, pReq->userid, pReq->micstate);
			return -1;
		}

		LOG_PRINT(log_info, "not found user:%u, created new user object in room:%u.", pReq->userid, pReq->vcbid);
		userobjRef_old.reset(p);
		userobjRef_old->nsvrid = m_svrid;
		userobjRef_old->ngateid = m_gateid;
		userobjRef_old->pConnId_ = task_node->connection->getconnid();
		userobjRef_old->pGateObj_ = pGateMask->param1;
		userobjRef_old->pGateObjId_ = pGateMask->param2;
		userobjRef_old->ngender_  = p->ngender_;
		userobjRef_old->nage_     = p->nage_;
		userobjRef_old->ncuronmictype_     = 0;
		userobjRef_old->ncurpublicmicindex_ = pReq->micindex;
		userobjRef_old->nstarttime_pubmic_ = 0;
		userobjRef_old->nwillendtime_pubmic_ = 0;

		userobjRef_old->inroomstate_ = 0;
		if (0 == pReq->micindex)
			userobjRef_old->inroomstate_ = FT_ROOMUSER_STATUS_PUBLIC_MIC;
		userobjRef_old->ninroomlevel_ = p->ninroomlevel_;
		userobjRef_old->cometime_ = time(0);
		userobjRef_old->nstarflag_ = p->nstarflag_;
		userobjRef_old->nactivityflag_ = p->nactivityflag_;
		userobjRef_old->type_ = USERTYPE_NORMALUSER;
		userobjRef_old->calias_ = p->calias_;
		userobjRef_old->headpic_ = p->headpic_;
		userobjRef_old->macserial_ = p->macserial_;
		userobjRef_old->bForbidInviteUpMic_ = 0;
		userobjRef_old->bForbidChat_ = pRoomObjRef->IsForbidChatUser(p->nuserid_); //被禁言用户
		userobjRef_old->ndevtype = pReq->devtype;

		userobjRef_old->setUserInfo2Redis(m_pSvrEvent->m_pRedisMgr->getOne(DB_ROOM));

		if (0 == pReq->micindex)
		{
			CMDUserMicState ReqData;
			ReqData.set_micindex(-1);
			ReqData.set_micstate(FT_ROOMUSER_STATUS_PUBLIC_MIC);

			std::string strHead;
			std::string strAlias;
			CUserBasicInfo::getUserHeadAndAlias(pReq->userid, strHead, strAlias);

			ReqData.mutable_runid()->set_userid(pReq->userid);
			ReqData.mutable_runid()->set_head(strHead);
			ReqData.mutable_runid()->set_alias(strAlias);

			ReqData.mutable_toid()->set_userid(pReq->userid);
			ReqData.mutable_toid()->set_head(strHead);
			ReqData.mutable_toid()->set_alias(strAlias);

			ReqData.set_vcbid(pReq->vcbid);
			ReqData.set_giftid(0);
			ReqData.set_giftnum(0);

			unsigned int reqLen = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + ReqData.ByteSize();
			SL_ByteBuffer buff(reqLen);
			buff.data_end(reqLen);

			COM_MSG_HEADER * pOutMsg = (COM_MSG_HEADER *)buff.buffer();
			pOutMsg->version = MDM_Version_Value;
			pOutMsg->checkcode = CHECKCODE;
			pOutMsg->maincmd = MDM_Vchat_Room;
			pOutMsg->subcmd = Sub_Vchat_SetMicStateReq;
			pOutMsg->length = reqLen;

			ClientGateMask_t * pGateMask1 = (ClientGateMask_t *)(pOutMsg->content);
			pGateMask1->param1 = pGateMask->param1;
			pGateMask1->param2 = pGateMask->param2;
			char * pOutCmd = (char *)(pOutMsg->content + SIZE_IVM_CLIENTGATE);
			ReqData.SerializeToArray(pOutCmd, ReqData.ByteSize());

			//消息使用task模拟处理
			if (pRoomMgr->m_clientconn.get_connecthandle()->isconnected())
				pRoomMgr->m_clientconn.get_connecthandle()->write_message(buff);
		}
	}
	else
	{
		userobjRef_old->nsvrid = m_svrid;
		userobjRef_old->ngateid = m_gateid;
		userobjRef_old->pConnId_ = task_node->connection->getconnid();
		userobjRef_old->pGateObj_ = pGateMask->param1;
		userobjRef_old->pGateObjId_ = pGateMask->param2;

		userobjRef_old->setUserInfo2Redis(m_pSvrEvent->m_pRedisMgr->getOne(DB_ROOM), "svrid gateid gateobj gateobjid gateconnid");
	}

	char buf[128] = {0};
	COM_MSG_HEADER *out_msg = (COM_MSG_HEADER*)buf;
	out_msg->version = MDM_Version_Value;
	out_msg->checkcode = CHECKCODE;
	out_msg->maincmd =MDM_Vchat_Room;
	out_msg->subcmd =Sub_Vchat_GateJoinRoomResp;
	memcpy(out_msg->content, in_msg->content, sizeof(ClientGateMask_t));
	CMDGateJoinRoomResp_t * pdata = (CMDGateJoinRoomResp_t *)(out_msg->content + SIZE_IVM_CLIENTGATE);
	pdata->vcbid =  pReq->vcbid;
	pdata->userid = userobjRef_old->nuserid_;
	pdata->errorid = 0;
	out_msg->length = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + sizeof(CMDGateJoinRoomResp_t);
	task_node->connection->write_message(buf, out_msg->length);

	return 0;
}

int CLogicCore::proc_exitroom_req(task_proc_data * task_node)
{
	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)(task_node->pdata);
	if (pHead->length < SIZE_IVM_HEAD_TOTAL)
	{
		LOG_PRINT(log_warning, "parse msginfo fixed length failed, need at lease %d but recv %d!", SIZE_IVM_HEAD_TOTAL, pHead->length);
		return -1;
	}

	ClientGateMask_t * pGateMask = (ClientGateMask_t *)(pHead->content);
	CMDUserExitRoomInfo pReq;
	pReq.ParseFromArray(pHead->content + SIZE_IVM_CLIENTGATE, SIZE_IVM_REQUEST(pHead));

	LOG_PRINT(log_info, "room_tag [connid=%llu,gateid=%u,userid=%u,roomid=%u]exit-room request!", pGateMask->param2, m_gateid, pReq.userid(), pReq.vcbid());

	CRoomManager * pRoomMgr = m_pSvrEvent->getRoomMgr();
	RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(pReq.vcbid());
	LOG_PRINT(log_info, "room_tag [connid=%llu,gateid=%u,userid=%u,roomid=%u]exit-room FindRoom!", pGateMask->param2, m_gateid, pReq.userid(), pReq.vcbid());
	if(pRoomObjRef.get() == 0)
	{
		LOG_PRINT(log_warning, "room_tag [connid=%llu,gateid=%u,userid=%u,roomid=%u]not found roomid", pGateMask->param2, m_gateid, pReq.userid(), pReq.vcbid());
		return 0;
	}

//	if (pRoomObjRef->delWaitMicUser(pReq.userid(),-1))
//		pRoomObjRef->castSendMicList();

	//group member
//	if (pRoomObjRef->outGroup(pReq.userid()))
//	{
//		return 0;
//	}

	UserObject_Ref pUserRef;
	if (!pRoomObjRef->findVisitUser(pUserRef, pReq.userid()))
	{
		LOG_PRINT(log_error, "room_tag [connid=%llu,gateid=%u,userid=%u,roomid=%u]not found user in room", pGateMask->param2, m_gateid, pReq.userid(), pReq.vcbid());
		return -1;
	}
	LOG_PRINT(log_info, "room_tag [connid=%llu,gateid=%u,userid=%u,roomid=%u]exit-room findVisitUser!", pGateMask->param2, m_gateid, pUserRef->nuserid_, pReq.vcbid());
//	if(pUserRef->ngateid != m_gateid || pUserRef->pGateObjId_ != pGateMask->param2){
//		LOG_PRINT(log_error, "room_tag [room_tag connid=%llu,gateid=%u,userid=%u,roomid=%u]user session does not match.[user: %u, %llu] <===> [request: %u, %llu]",
//				pGateMask->param2, m_gateid, pReq.userid(), pReq.vcbid(), pUserRef->ngateid, pUserRef->pGateObjId_, m_gateid, pGateMask->param2);
//		return -1;
//	}

	if(pReq.vcbid() > BROADCAST_ROOM_BEGIN_NUM)//直播间停留时长
	{
		int minutes = (time(0) - pUserRef->cometime_)/60 + 1 ;
		int date = atoi(stamp_to_standard(pUserRef->cometime_,"%Y%m%d").c_str());
		DBmongoConnection::updateUserOnlineTime(pReq.userid(),pReq.vcbid(),date,minutes);
		CDBSink().updateStatistics(pReq.userid(), pReq.vcbid(), 0, date, minutes);
		LOG_PRINT(log_debug, "live vcbid[%d],minutes[%d],date[%d],userid[%d],cometime[%d]", pReq.vcbid(),minutes,date,pReq.userid(),pUserRef->cometime_);
	}
	else//课程停留时长
	{
		int minutes = (time(0) - pUserRef->cometime_) / 60 + 1;
		int date = atoi(stamp_to_standard(pUserRef->cometime_, "%Y%m%d").c_str());
		//DBmongoConnection::updateUserOnlineTime(pReq.userid(), pReq.vcbid(), date, minutes);
		CDBSink().updateStatistics(pReq.userid(), pReq.vcbid(), 1, date, minutes);
		LOG_PRINT(log_debug, "course vcbid[%d],minutes[%d],date[%d],userid[%d],cometime[%d]", pReq.vcbid(), minutes, date, pReq.userid(), pUserRef->cometime_);
	}
	if (pRoomObjRef->eraseVisitUser(pUserRef) < 0)
		return -1;

	pUserRef->reqid = pHead->reqid;//slove reqid

	//用户离开消息
	pRoomObjRef->castSendRoomUserLeftInfo(pUserRef);

	LOG_PRINT(log_info, "room_tag [connid=%llu,gateid=%u,userid=%u,roomid=%u]exit-room request end!", pGateMask->param2, m_gateid, pReq.userid(), pReq.vcbid());
	//m_pSvrEvent->m_roomMgr.exitroom_record(pUserRef);
	return 0;
}

int CLogicCore::proc_except_exitroom_req(task_proc_data * task_node)
{
	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)(task_node->pdata);
	if (pHead->length < SIZE_IVM_HEAD_TOTAL)
	{
		LOG_PRINT(log_warning, "parse msginfo fixed length failed, need at lease %d but recv %d!", SIZE_IVM_HEAD_TOTAL, pHead->length);
		return -1;
	}

	ClientGateMask_t * pGateMask = (ClientGateMask_t *)(pHead->content);
	CMDUserExceptExitRoomInfo pReq;
	pReq.ParseFromArray(pHead->content + SIZE_IVM_CLIENTGATE, SIZE_IVM_REQUEST(pHead));
	unsigned int userid = pReq.userid();
	unsigned int vcbid = pReq.vcbid();

	LOG_PRINT(log_warning, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]recv except exit request", pGateMask->param2, m_gateid, userid, vcbid);

	CRoomManager * pRoomMgr = m_pSvrEvent->getRoomMgr();
	RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(vcbid);
	if(pRoomObjRef.get() == 0)
	{
		LOG_PRINT(log_warning, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]not found roomid", pGateMask->param2, m_gateid, userid, vcbid);
		return 0;
	}

	//group member
// 	if (pRoomObjRef->outGroup(pReq.userid()))
// 	{
// 		return 0;
// 	}

	UserObject_Ref pUserRef;
	if (!pRoomObjRef->findVisitUser(pUserRef, userid)){
		LOG_PRINT(log_error, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]not found user", pGateMask->param2, m_gateid, userid, vcbid);
		return -1;
	}

	//if (!pUserRef->bExceptExit) {
	//	LOG_PRINT(log_error, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]add to except exit queue", pGateMask->param2, m_gateid, userid, vcbid);
	//	ExceptExitRecord_t data;
	//	data.vcbid = vcbid;
	//	data.userid = userid;
	//	data.ngateid = m_gateid;
	//	data.pdata = (char*)malloc(task_node->datalen);
	//	data.len = task_node->datalen;
	//	memcpy(data.pdata, task_node->pdata, task_node->datalen);
	//	char key[64];
	//	sprintf(key, "%u:%u", vcbid, userid);
	//	m_pSvrEvent->m_mapExceptExit.push(key, data);
	//	pUserRef->bExceptExit = true;
	//	pUserRef->setUserInfo2Redis(m_pSvrEvent->m_pRedisMgr->getOne(DB_ROOM), "exceptexit");
	//	return 0;
	//}

	if(pUserRef->ngateid != m_gateid || pUserRef->pGateObjId_ != pGateMask->param2){
		LOG_PRINT(log_error, "[connid=%llu,gateid=%u,userid=%u,roomid=%u][user connid=%llu,gateid=%u]user session does not match",
				pGateMask->param2, m_gateid, userid, vcbid, pUserRef->pGateObjId_, pUserRef->ngateid);
		return -1;
	}

	if (pRoomObjRef->eraseVisitUser(pUserRef) < 0)
		return -1;

	//用户离开消息
	pRoomObjRef->castSendRoomUserExceptLeftInfo(pUserRef);
	//m_pSvrEvent->m_roomMgr.exitroom_record(pUserRef);

	return 0;
}

int CLogicCore::handle_except_exitroom_delay(ExceptExitRecord_t *pexitRecord)
{
	char *pdata = pexitRecord->pdata;

	GEN_MSGINFO_AND_CHECK_LENGTH(pdata, in_msg, pGateMask, pReq, CMDUserExceptExitRoomInfo_t)
	CRoomManager *pRoomMgr = m_pSvrEvent->getRoomMgr();

	RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(pReq->vcbid);
	if(pRoomObjRef.get() == 0)
	{
		LOG_PRINT(log_warning, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]not found roomid", pGateMask->param2, m_gateid, pReq->userid, pReq->vcbid);
		return 0;
	}

	//group member
	if (pRoomObjRef->outGroup(pReq->userid))
	{
		return 0;
	}

	UserObject_Ref pUserRef;
	if (!pRoomObjRef->findVisitUser(pUserRef, pReq->userid)){
		LOG_PRINT(log_error, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]not found user", pGateMask->param2, m_gateid, pReq->userid, pReq->vcbid);
		return 0;
	}

	if (!pUserRef->bExceptExit) {
		LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u][gateid:%u,connid:%llu]user info bExceptExit is false",
				pGateMask->param2, m_gateid, pReq->userid, pReq->vcbid, pUserRef->ngateid, pUserRef->pGateObjId_);
		return 0;
	}

	LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u][user connid:%u,gateid:%u,userid:%u,roomid:%u]handle delay except exit queue item",
			pGateMask->param2, pexitRecord->ngateid, pexitRecord->userid, pexitRecord->vcbid, pUserRef->pGateObjId_,
			pUserRef->ngateid, pUserRef->nuserid_,  pUserRef->ngroupid);

	if (0 == pRoomObjRef->eraseVisitUser(pUserRef)) {
		pRoomObjRef->castSendRoomUserExceptLeftInfo(pUserRef);
		//m_pSvrEvent->m_roomMgr.exitroom_record(pUserRef);
	}
	return 0;
}

int CLogicCore::proc_kickoutuser_req(task_proc_data * task_node)
{
	COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *)task_node->pdata;
	ClientGateMask_t * pGateMask = (ClientGateMask_t *)in_msg->content;
	CMDUserKickoutRoomInfo pReq;
	pReq.ParseFromArray(in_msg->content + SIZE_IVM_CLIENTGATE, SIZE_IVM_REQUEST(in_msg));
	unsigned int ret = proc_kickoutuser_req(pReq.srcid(), pReq.toid(), pReq.vcbid(), pReq.resonid(), pReq.mins());
	LOG_PRINT(log_info, "[connid=%llu,gateid=%u,src_userid=%u,to_userid=%u,roomid=%u,reason:%d,ret:%u]user was kicked out by client.need to response back.", \
		pGateMask->param2, m_gateid, pReq.srcid(), pReq.toid(), pReq.vcbid(), pReq.resonid(), ret);

	CMDUserKickoutRoomResp oRspData;
	oRspData.mutable_errinfo()->set_errid(ret);
	oRspData.set_srcid(pReq.srcid());
	oRspData.set_toid(pReq.toid());
	oRspData.set_vcbid(pReq.vcbid());

	unsigned int rspDataLen = SIZE_IVM_HEAD_TOTAL + oRspData.ByteSize();
	SL_ByteBuffer buff(rspDataLen);
	buff.data_end(rspDataLen);

	COM_MSG_HEADER * pRspHead = (COM_MSG_HEADER *)buff.buffer();
	ClientGateMask_t * pRspGate = (ClientGateMask_t *)(pRspHead->content);
	GEN_RSP_MSGINFO_SET_HEAD_GATE(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_RoomKickoutUserResp, in_msg->reqid, pGateMask, pRspHead, pRspGate, oRspData);
	task_node->connection->write_message(buff);

	return 0;
}

int CLogicCore::proc_kickoutuser_req(unsigned int src_userid, unsigned int to_userid, unsigned int groupid, unsigned int reasonid, unsigned int kickout_min)
{
	UserObject_Ref pToUserRef;
	RoomObject_Ref pRoomObjRef;
	bool bMember = false;

	if (!groupid || !m_pSvrEvent || !to_userid || !src_userid) 
	{
		LOG_PRINT(log_error, "[kickout user]input parameter must not be 0.src_userid:%u,to_userid:%u,groupid:%u.", src_userid, to_userid, groupid);
		return ERR_CODE_FAILED_ROOMIDNOTFOUND;
	}

	pRoomObjRef = m_pSvrEvent->getRoomMgr()->FindRoom(groupid);
	if(pRoomObjRef.get() == 0) 
	{
		LOG_PRINT(log_error, "[kickout user]not found groupid:%u,src_userid:%u,to_userid:%u.", groupid, src_userid, to_userid);
		return ERR_CODE_FAILED_ROOMIDNOTFOUND;
	}

	if (!CUserGroupinfo::chkUserRoleBeyond(src_userid, groupid, e_AdminRole))
	{
		LOG_PRINT(log_error, "[kickout user]group[%u],src_userid[%u] has no right to kick out user.", groupid, src_userid);
		return ERR_CODE_USER_HAS_NO_RIGHT;
	}

	if (pRoomObjRef->findMemberUser(pToUserRef, to_userid))
	{
		bMember = true;
		LOG_PRINT(log_info, "[kickout user]toid:%u is groupid:%d member.", to_userid, groupid);
		if (pRoomObjRef->eraseMemberUser(pToUserRef) < 0)
		{
			LOG_PRINT(log_error, "[kickout user]kick out member failed.groupid:%u,src_userid:%u,to_userid:%u.", groupid, src_userid, to_userid);
			return ERR_CODE_KICK_USER_OUT;
		}
	}
	else
	{
		LOG_PRINT(log_info, "[kickout user]to_userid:%u is groupid:%d visitor.reasonid:%u.", to_userid, groupid, reasonid);
		if (!pRoomObjRef->findVisitUser(pToUserRef, to_userid))
		{
			LOG_PRINT(log_error, "[kickout user]not found visitor to_userid:%u in groupid:%u to kick out.", to_userid, groupid);
			return ERR_CODE_FAILED_USERNOTFOUND;
		}

		if (pRoomObjRef->eraseVisitUser(pToUserRef) < 0)
		{
			return ERR_CODE_KICK_USER_OUT;
		}
	}

	LOG_PRINT(log_info, "[kickout user][src_userid:%u,to_userid:%u,groupid:%u,reasonid:%u]user was kicked out successful.", src_userid, to_userid, groupid, reasonid);

	if (pRoomObjRef.get() && pToUserRef.get())
	{
		LOG_PRINT(log_info, "[kickout user][src_userid:%u,to_userid:%u,groupid:%u]user was kicked out successful and notify in group.", src_userid, to_userid, groupid);
		if (bMember)
		{
			LOG_PRINT(log_info, "[kickout user][src_userid:%u,to_userid:%u,groupid:%u]user change roletype to visitor and notify in group.", src_userid, to_userid, groupid);
			//need to notify member role notify.
			pRoomObjRef->castSendUserPriorityChange(to_userid, e_VisitorRole);
		}

		pRoomObjRef->castSendRoomKickoutUserInfo(src_userid, pToUserRef, reasonid, kickout_min);
		//pRoomMgr->exitroom_record(pToUserRef);
	}
	return 0;
}

int CLogicCore::proc_gatekickoutuser_req(task_proc_data * task_node)
{
	COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *)task_node->pdata;
	CMDUserKickoutRoomInfo pReq;
	pReq.ParseFromArray(in_msg->content + SIZE_IVM_CLIENTGATE, SIZE_IVM_REQUEST(in_msg));
	return proc_gatekickoutuser_req(pReq.toid(), pReq.vcbid(), pReq.resonid());
}

int CLogicCore::proc_gatekickoutuser_req(unsigned int to_userid, unsigned int groupid, unsigned int reasonid)
{
	UserObject_Ref pToUserRef;
	RoomObject_Ref pRoomObjRef;

	if (!groupid || !m_pSvrEvent || !to_userid) 
	{
		LOG_PRINT(log_error, "[gate kickout user]group must not be 0.to_userid:%u,groupid:%u.", to_userid, groupid);
		return ERR_CODE_FAILED_ROOMIDNOTFOUND;
	}

	pRoomObjRef = m_pSvrEvent->getRoomMgr()->FindRoom(groupid);
	if(pRoomObjRef.get() == 0) 
	{
		LOG_PRINT(log_error, "[gate kickout user]not found groupid:%u,to_userid:%u.", groupid, to_userid);
		return ERR_CODE_FAILED_ROOMIDNOTFOUND;
	}

	//group member
	if (pRoomObjRef->outGroup(to_userid))
	{
		return true;
	}

	LOG_PRINT(log_info, "[gate kickout user]to_userid:%u is groupid:%d visitor.reasonid:%u.", to_userid, groupid, reasonid);
	if (!pRoomObjRef->findVisitUser(pToUserRef, to_userid))
	{
		LOG_PRINT(log_error, "[gate kickout user]not found visitor to_userid:%u in groupid:%u to kick out.", to_userid, groupid);
		return ERR_CODE_FAILED_USERNOTFOUND;
	}

	if (pRoomObjRef->eraseVisitUser(pToUserRef) < 0)
	{
		return ERR_CODE_KICK_USER_OUT;
	}

	LOG_PRINT(log_info, "[gate kickout user][to_userid:%u,groupid:%u,reasonid:%u]visitor user was kicked out successful.", to_userid, groupid, reasonid);

	if (pRoomObjRef.get() && pToUserRef.get())
	{
		LOG_PRINT(log_info, "[gate kickout user][to_userid:%u,groupid:%u]user was kicked out successful and notify in group.", to_userid, groupid);
		pRoomObjRef->castSendRoomKickoutUserInfo(0, pToUserRef, reasonid, 0);
	}
	return 0;
}

int CLogicCore::proc_quitgroup_req(task_proc_data * message)
{
	if (!message || !message->connection)
	{
		LOG_PRINT(log_error, "proc_quitgroup_req ERROR:message or connection is null.");
		return -1;
	}

	CMDQuitGroupReq req;
	if (!CMsgComm::ParseProtoMessage(message->pdata, message->datalen, req))
	{
		LOG_PRINT(log_error, "parse %s packet error! length=%d.", req.GetTypeName().c_str(), message->datalen);
		message->resperrinf(ERR_CODE_FAILED_PACKAGEERROR);
		return -1;
	}

	LOG_PRINT(log_info, "receive proc_quitgroup_req:user[%u] group[%u].", req.userid(), req.groupid());
	unsigned int ret = 0;
	RoomObject_Ref pRoomObjRef;
	UserObject_Ref pUserRef;
	bool needNotify = false;

	do 
	{
		CRoomManager * pRoomMgr = m_pSvrEvent->getRoomMgr();
		if (!req.userid() || !req.groupid() || !pRoomMgr)
		{
			LOG_PRINT(log_error, "[proc_quitgroup_req]check param failed.user[%u] group[%u].", req.userid(), req.groupid());
			ret = ERR_CODE_INVALID_PARAMETER;
			break;
		}

		pRoomObjRef = pRoomMgr->FindRoom(req.groupid());
		if(pRoomObjRef.get() == 0) 
		{
			LOG_PRINT(log_error, "[proc_quitgroup_req]not found room.user[%u] group[%u].", req.userid(), req.groupid());
			ret = ERR_CODE_FAILED_ROOMIDNOTFOUND;
			break;
		}

		if (pRoomObjRef->group_master == req.userid())
		{
			LOG_PRINT(log_error, "[proc_quitgroup_req]group owner cannot quit group.user[%u] group[%u].", req.userid(), req.groupid());
			ret = ERR_CODE_GROUP_OWNER_CANNOT_QUIT;
			break;
		}

		if (pRoomObjRef->findMemberUser(pUserRef, req.userid()))
		{
			LOG_PRINT(log_info, "user is group member.user[%u] group[%u].", req.userid(), req.groupid());
			if (pRoomObjRef->eraseMemberUser(pUserRef) < 0)
			{
				LOG_PRINT(log_error, "user quit group failed.user[%u] group[%u].", req.userid(), req.groupid());
				ret = ERR_CODE_USER_QUIT_GROUP;
				break;
			}
			needNotify = true;
		}
		else
		{
			LOG_PRINT(log_info, "user is group visitor.user[%u] group[%u].not need to handle it.", req.userid(), req.groupid());
		}

	} while (0);
	
	LOG_PRINT(log_info, "user quit group ret:%u, need to notify user left in group.user[%u] group[%u].", ret, req.userid(), req.groupid());
	CMDQuitGroupResp resp;
	resp.set_groupid(req.groupid());
	resp.set_userid(req.userid());
	resp.mutable_errinfo()->set_errid(ret);

	unsigned int rspDataLen = SIZE_IVM_HEAD_TOTAL + resp.ByteSize();
	SL_ByteBuffer buff(rspDataLen);
	buff.data_end(rspDataLen);

	COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *) message->pdata;
	ClientGateMask_t * pGateMask = (ClientGateMask_t *)(in_msg->content);
	COM_MSG_HEADER * pRspHead = (COM_MSG_HEADER *)buff.buffer();
	ClientGateMask_t * pRspGate = (ClientGateMask_t *)(pRspHead->content);
	GEN_RSP_MSGINFO_SET_HEAD_GATE(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_QuitGroupResp, in_msg->reqid, pGateMask, pRspHead, pRspGate, resp);
	message->connection->write_message(buff);

//	if (ret == 0 && needNotify && pUserRef.get())
//	{
//		//notify user left
//		pRoomObjRef->castSendRoomUserLeftInfo(pUserRef);
//		LOG_PRINT(log_info, "user quit group success, need to notify in group.user[%u] group[%u].", req.userid(), req.groupid());
//		add_group_assistmsg_rpc(req.userid(), req.groupid(), e_VisitorRole, /*levelUp*/false, /*NotyAdmin*/true, /*NotyUser*/false, /*QuitGroup*/true);
//	}
	return 0;
}

bool CLogicCore::proc_joingroup_rpc(unsigned int userid, unsigned int groupid, unsigned int iRoleType /* = e_MemberRole */)
{
	bool ret = false;
	CThriftHttpClient<TChatSvrConcurrentClient> rpcClient(e_roomadapter_type);
	if (rpcClient)
	{
		ret = rpcClient->procUserJoinGroupNotify(userid, groupid, iRoleType);
	}
	else
	{
		LOG_PRINT(log_error, "proc_joingroup_rpc error, cannot find ChatSvr.userid:%u,groupid:%u.", userid, groupid);
		return ret;
	}

	if (ret)
	{
		LOG_PRINT(log_info, "[member first join group rpc call return success]userid:%u,groupid:%u.", userid, groupid);
	}
	else
	{
		LOG_PRINT(log_error, "[member first join group rpc call return fail]userid:%u,groupid:%u.", userid, groupid);
	}
	return ret;
}

int CLogicCore::add_group_assistmsg_rpc(unsigned int userid, unsigned int groupid, unsigned int iRoleType, bool bLevelUp, bool bNotyAdmin/*= true*/, bool bNotyUser/*= true*/, bool bQuitGroup/*= false*/)
{
	if (!userid || !groupid)
	{
		return 0;
	}

	TGroupAssistMsg assistMsg;
	assistMsg.userID = userid;
	assistMsg.groupID = groupid;
	assistMsg.svrLevel = iRoleType;
	assistMsg.svrLevelUp = bLevelUp;
	if (iRoleType == e_VisitorRole && bQuitGroup == false)
	{
		assistMsg.svrType = MSGTYPE_KICKGROUP;
	}
	else
	{
		assistMsg.svrType = MSGTYPE_LEVEL_CHG;
	}
	
	int ret = add_group_assistmsg_rpc(assistMsg, bNotyAdmin, bNotyUser);
	if (ret)
	{
		LOG_PRINT(log_info, "[add group assist msg rpc call return success]userid:%u,groupid:%u.", userid, groupid);
	}
	else
	{
		LOG_PRINT(log_error, "[add group assist msg rpc call return fail]userid:%u,groupid:%u.", userid, groupid);
	}
	return ret;
}

int CLogicCore::add_group_assistmsg_rpc(const TGroupAssistMsg & assistMsg, bool bNotyAdmin, bool bNotyUser)
{
	CThriftHttpClient<TChatSvrConcurrentClient> rpcClient(e_roomadapter_type);
	if (rpcClient)
	{
		return rpcClient->proc_addGroupAssistMsg(assistMsg, bNotyAdmin, bNotyUser);
	}
	else
	{
		LOG_PRINT(log_error, "add group assistmsg rpc error, cannot find ChatSvr.");
		return 0;
	}
}

void CLogicCore::proc_clean_visitor_user(unsigned int userid, unsigned int groupid)
{
	if (!userid || !groupid || !m_pSvrEvent)
	{
		LOG_PRINT(log_warning, "[clean visitor user]userid:%u,groupid:%u not need to clean visitor user.", userid, groupid);
		return;
	}

	unsigned int now_groupid = 0;
	LOG_PRINT(log_info, "[clean visitor user]input userid:%u,groupid:%u.", userid, groupid);
	UserObject_Ref userobjRef;
	CRoomObj::findVisitUser(&m_pSvrEvent->m_roomMgr, m_pSvrEvent->m_roomMgr.getDataRedis(), userobjRef, userid);
	if (userobjRef)
	{
		now_groupid = userobjRef->ngroupid;
	}

	if (now_groupid != groupid)
	{
		LOG_PRINT(log_warning, "[clean visitor user]user is visiting another groupid:%u now,not need to clean visitor data.userid:%u,groupid:%u.", \
			now_groupid, userid, groupid);
		return;
	}
	else
	{
		LOG_PRINT(log_warning, "[clean visitor user]user is visiting group and now change to member,need to clean visitor data.userid:%u,groupid:%u.", userid, groupid);
		RoomObject_Ref pRoomObjRef = m_pSvrEvent->m_roomMgr.FindRoom(groupid);
		if (pRoomObjRef.get() == 0)
		{
			LOG_PRINT(log_warning, "[clean visitor user]not found group:%u,not need to handle clean visitor data.", groupid);
			return;
		}

		UserObject_Ref pUserObjRef;
		if (!pRoomObjRef->findVisitUser(pUserObjRef, userid))
		{
			LOG_PRINT(log_warning, "[clean visitor user]group:%u not found visitor userid:%u.not need to clean visitor data.", groupid, userid);
			return;
		}

		pRoomObjRef->inGroup(userid);

		//不处理down mic的逻辑
		if (pRoomObjRef->eraseVisitUser(pUserObjRef, false) < 0)
		{
			LOG_PRINT(log_error, "[clean visitor user]clean visitor data error.group:%u,userid:%u.", groupid, userid);
			return;
		}
	}
}

int CLogicCore::proc_client_reportmediagateip_req(task_proc_data *task_node)
{
	GEN_MSGINFO_AND_CHECK_LENGTH(task_node->pdata, in_msg, pGateMask, pReq, CMDReportMediaGateReq_t);
	CHECK_VARIABLE_LENGTH(in_msg, CMDReportMediaGateReq_t, pReq->textlen);

	UserObject_Ref pUserObjRef;
	CRoomManager* pRoomMgr = m_pSvrEvent->getRoomMgr();
	RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(pReq->vcbid);
	if (pRoomObjRef.get() == 0)
	{
		LOG_PRINT(log_error, "not found room:%u", pReq->vcbid);
	    resperrinf(in_msg, pGateMask, ERR_CODE_FAILED_ROOMIDNOTFOUND, task_node);
		return 0;
	}
	if (!pRoomObjRef->findVisitUser(pUserObjRef, pReq->userid))
	{
		LOG_PRINT(log_error, "not found user:%u in room", pReq->userid, pReq->vcbid);
	    resperrinf(in_msg, pGateMask, ERR_CODE_FAILED_USERNOTFOUND, task_node);
		return 0;
    }

	if (pReq->textlen >= 128)
	{
		LOG_PRINT(log_error, "CMDReportMediaGateReq_t textlen is large than 128, return!");
		resperrinf(in_msg, pGateMask, ERR_CODE_FAILED_PACKAGEERROR, task_node);
		return 0;
	}
	char ip[128] = {0};
	memcpy(ip, pReq->content, pReq->textlen);
	ip[pReq->textlen] = 0;
	LOG_PRINT(log_info, "recv report media and gate ip request: %s, user:%u", ip, pReq->userid);

	char *p = strchr(ip, '|');
	if (p){
		*p = 0;
		pUserObjRef->gateip = ip;
		pUserObjRef->mediaip = (p+1);
		pUserObjRef->setUserInfo2Redis(m_pSvrEvent->m_pRedisMgr->getOne(DB_ROOM), "gateip mediaip");
	}


	//发送请求回应
	char szOutBuf[512]={0};
	COM_MSG_HEADER* pOutMsg=(COM_MSG_HEADER*)szOutBuf;
	pOutMsg->version =10;
	pOutMsg->checkcode =0;
	pOutMsg->maincmd =MDM_Vchat_Room;
	pOutMsg->subcmd =Sub_Vchat_ReportMediaGateResp;
	ClientGateMask_t *pClientGate = (ClientGateMask_t*)pOutMsg->content;
	pClientGate->param1 = pGateMask->param1;
	pClientGate->param2 = pGateMask->param2;
	CMDReportMediaGateResp_t* pOutCmd=(CMDReportMediaGateResp_t*)(pOutMsg->content + SIZE_IVM_CLIENTGATE);
	pOutCmd->vcbid = pReq->vcbid;
	pOutCmd->userid = pReq->userid;
	pOutCmd->errid = 0;
	pOutMsg->length =sizeof(COM_MSG_HEADER) + SIZE_IVM_CLIENTGATE + sizeof(CMDReportMediaGateResp_t);

	task_node->connection->write_message(szOutBuf, pOutMsg->length);
	return 0;
}

int CLogicCore::proc_setmicstate_req(task_proc_data *task_node)
{
	LOG_PRINT(log_info, "proc_setmicstate_req begin");
	COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *)(task_node->pdata);
	if (in_msg->length < SIZE_IVM_HEAD_TOTAL)
	{
		LOG_PRINT(log_warning, "parse msginfo fixed length failed, need at lease %d but recv %d!", SIZE_IVM_HEAD_TOTAL, in_msg->length);
		return -1;
	}

	CMDUserMicState pReq;
	pReq.ParseFromArray(in_msg->content + SIZE_IVM_CLIENTGATE, SIZE_IVM_REQUEST(in_msg));
	CRoomManager * pRoomMgr = m_pSvrEvent->getRoomMgr();

	unsigned int touserid = pReq.toid().userid();
	unsigned int runuserid = pReq.runid().userid();
	LOG_PRINT(log_info, "room_tag[micstate:%d,vcbid:%u,user:%u,runner:%u] setmicstate start...", pReq.micstate(), pReq.vcbid(), touserid, runuserid);

	RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(pReq.vcbid());
	if(pRoomObjRef.get() == 0) 
	{
		LOG_PRINT(log_warning, "[room:%u,runner:%u,touser:%u]not found room", pReq.vcbid(), runuserid, touserid);
		return 0;
	}

	LOG_PRINT(log_info, "room_tag[micstate:%d,vcbid:%u,user:%u,runner:%u] setmicstate 111...", pReq.micstate(), pReq.vcbid(), touserid, runuserid);
	int nRet = -1;
	bool bWaitMicListChanged= false;
	UserObject_Ref pToUserRef , pSrcUserRef;
	if(!pRoomObjRef->findUser(pToUserRef, touserid))  //用户一定要存在
	{
		LOG_PRINT(log_warning, "room_tag[room:%u,runner:%u,touser:%u]not found srcuser", pReq.vcbid(), runuserid, touserid);
		nRet = ERR_CODE_FAILED_USERNOTFOUND;
	}

	if(!pRoomObjRef->isNowInGroup(touserid))  //用户一定要存在
	{
		LOG_PRINT(log_warning, "room_tag[room:%u,runner:%u,touser:%u]not found srcuser", pReq.vcbid(), runuserid, touserid);
		nRet = ERR_CODE_FAILED_USERNOTFOUND;
	}

	LOG_PRINT(log_info, "room_tag[micstate:%d,vcbid:%u,user:%u,runner:%u] setmicstate 111...nRet:%d", pReq.micstate(), pReq.vcbid(), touserid, runuserid, nRet);
	if(nRet < 0)
	{
		if(pReq.micstate() == 0)
		{
			nRet= pRoomObjRef->setUserMicState_0(pToUserRef, pReq.micstate(), pReq.micindex(), pReq.giftid(), pReq.giftnum(), bWaitMicListChanged);
		}else
		{
			LOG_PRINT(log_info, "room_tag[micstate:%d,vcbid:%u,user:%u,runner:%u] setmicstate xzz...nRet:%d", pReq.micstate(), pReq.vcbid(), touserid, runuserid,nRet);
			UserObject_Ref pTemp;
			pTemp = CUserObj::findUserOnline(touserid);
			pRoomObjRef->findMemberUser(pSrcUserRef, runuserid);
			if(pSrcUserRef.get() == 0)
			{
				if(pRoomObjRef->allow_visitor_on_mic)
				{
					LOG_PRINT(log_warning, "room_tag[room:%u,runner:%u,touser:%u] allow_visitor_on_mic", pReq.vcbid(), runuserid, touserid);
					if(!pRoomObjRef->findVisitUser(pSrcUserRef, runuserid))
					{
						LOG_PRINT(log_warning, "room_tag[room:%u,runner:%u,touser:%u]not found runid", pReq.vcbid(), runuserid, touserid);
						nRet = ERR_CODE_FAILED_USERNOTFOUND;
					}
				}else
				{
					LOG_PRINT(log_warning, "room_tag[room:%u,runner:%u,touser:%u]not allow_visitor_on_mic", pReq.vcbid(), runuserid, touserid);
					nRet = ERR_NOT_ALLOW_VISIT_ON_MIC;
				}
			}
			if(nRet < 0)
			{
				if(pRoomObjRef->m_micMode == e_ChairmanMode &&  pSrcUserRef->nuserlevel_ < e_AdminRole && pRoomObjRef->group_master != runuserid)
				{
					nRet = ERR_CODE_USER_HAS_NO_RIGHT;
				}
				else if(pTemp.get() == 0)
				{
					nRet = ERR_USER_NOT_ONLINE;
				}
				else if(pTemp->nprivate_userid > 0)
				{
					nRet = ERR_USER_ON_PRIVATE_LINK;
				}
				else
				{
					LOG_PRINT(log_info, "room_tag[micstate:%d,vcbid:%u,user:%u,runner:%u,micindex:%d] setmicstate 222...",
							pReq.micstate(), pReq.vcbid(), touserid, runuserid, pReq.micindex());
					nRet = pRoomObjRef->setUserMicState_0(pToUserRef, pReq.micstate(), pReq.micindex(), pReq.giftid(), pReq.giftnum(), bWaitMicListChanged);
				}
			}

		}
	}

	LOG_PRINT(log_info, "[room_tagm:%d,vcbid:%u,user:%u,runner:%u] setmicstate 333,nRet:%d...", pReq.micstate(), pReq.vcbid(), touserid, runuserid, nRet);

	CMDUserMicState oNoty;
	oNoty.CopyFrom(pReq);
	std::string strHead;
	std::string strAlias;
	CUserBasicInfo::getUserHeadAndAlias(runuserid, strHead, strAlias);
	oNoty.mutable_runid()->set_head(strHead);
	oNoty.mutable_runid()->set_alias(strAlias);
	CUserBasicInfo::getUserHeadAndAlias(touserid, strHead, strAlias);
	oNoty.mutable_toid()->set_head(strHead);
	oNoty.mutable_toid()->set_alias(strAlias);

	if(0 == nRet)
	{
		//处理成功. 因为使用相同的数据体,修改in_msg 值,直接广播noty,对于自己上麦不需要操作返回resp
		oNoty.set_micindex(pToUserRef->ncurpublicmicindex_);
		oNoty.set_micsecond(-1);
		LOG_PRINT(log_info, "room_tag[micstate:%d,vcbid:%u,user:%u,runner:%u,,index:%d] setmicstate 444...",
				oNoty.micstate(), oNoty.vcbid(), touserid, runuserid, pToUserRef->ncurpublicmicindex_);
		oNoty.mutable_errinfo()->set_errid(0);

		int nProtoLen = oNoty.ByteSize();
		SL_ByteBuffer buff(SIZE_IVM_HEAD_TOTAL + nProtoLen);
		DEF_IVM_HEADER(pHead, buff.buffer());
		CMsgComm::Build_COM_MSG_HEADER(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_SetMicStateNotify, buff.buffer_size());
		DEF_IVM_CLIENTGATE(pGateMask, buff.buffer());
		CMsgComm::Build_BroadCastRoomGate(pGateMask, e_Notice_AllType, pReq.vcbid());
		oNoty.SerializeToArray(buff.buffer() + SIZE_IVM_HEAD_TOTAL, nProtoLen);
		m_pSvrEvent->getRoomMgr()->castGateway(pHead);

		pRoomObjRef->redisSetRoomInfo();
		pRoomObjRef->checkNextMic();
		pRoomObjRef->castSendMicList();
		LOG_PRINT(log_info, "room_tag[micstate:%d,vcbid:%u,user:%u,runner:%u] setmicstate end, successfully:%d",
				oNoty.micstate(), oNoty.vcbid(), touserid, runuserid, oNoty.micindex());
		int count = pRoomObjRef->getOnMicCount();
		std::map<std::string, std::string> mValues;
		mValues[CFieldName::ROOM_MICCOUNT] = bitTostring(count);
		CDBSink().updateGroupBasicInfo_DB(pReq.vcbid(), mValues);
	}
	else
	{
		if(nRet < 0)
		{
			nRet = pReq.micstate() == 0 ? ERR_DOWN_PUBLIC_MIC_FAILED : ERR_ON_PUBLIC_MIC_FAILED;
		}

		//上麦操作失败
		oNoty.mutable_errinfo()->set_errid(nRet);
		task_node->respProtobuf(oNoty, Sub_Vchat_SetMicStateNotify);
		LOG_PRINT(log_info, "room_tag[micstate:%d,vcbid:%u,user:%u,runner:%u] setmicstate end, failure", oNoty.micstate(), oNoty.vcbid(), touserid, runuserid);
//		CAlarmNotify::sendAlarmNoty(e_all_notitype, e_logic, Application::getInstance()->getProcName(), "roomsvr set mic failed", "Yunwei,Room",
//			"micstate:%d,vcbid:%u,user:%u,runner:%u, user:%u on mic", oNoty.micstate(), oNoty.vcbid(), touserid, runuserid, pRoomObjRef->getUserIsOnMic());
		return 0;
	}

	return 0;
}

int CLogicCore::jr_check_crc32(task_proc_data* task_node)
{
	GEN_MSGINFO_PF(task_node->pdata, in_msg, pGateMask, pReq, CMDJoinRoomReq)

	LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]check crc32.", 1, m_gateid, pReq.userid(), pReq.vcbid());

	// for web
	if (e_WEB_devtype == pReq.devtype())
		return 0;

	CMDJoinRoomReq temreq = pReq;
	uint32 crcval = temreq.crc32();
	temreq.set_crc32(15);
	temreq.set_cipaddr(0);
	int len = pReq.ByteSize();
	char buf[512];
	pReq.SerializeToArray(buf,len);
	uint32 ucrc32 = crc32((void*)buf, pReq.ByteSize(), CRC_MAGIC);
	if(crcval != ucrc32) {
		LOG_PRINT(log_error, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]Failed to verify crc32.", 1, m_gateid, pReq.userid(), pReq.vcbid());
		return -1;
	}

	return 0;
}

bool CLogicCore::jr_room_isallow(task_proc_data *task_node, RoomObject_Ref &roomObjRef)
{
	if (!roomObjRef->uAllowVisitGroup)
	{
		//判断房间是否允许围观
		LOG_PRINT(log_info, "group %u is not allowed to visit, error:405", roomObjRef->nvcbid_);
		jr_send_resp_err(task_node, ERR_CODE_GROUP_NOTALLOW_VISIT);
		return true;
	}

	return false;
}

bool CLogicCore::jr_is_roomowner(task_proc_data *task_node, RoomObject_Ref &roomObjRef)
{
	GEN_MSGINFO_PF(task_node->pdata, in_msg, pGateMask, pReq, CMDJoinRoomReq)

	LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]check whether user is the owner of room.",
			pGateMask->param2, m_gateid, pReq.userid(), pReq.vcbid());
	if(pReq.userid() != roomObjRef->ncreatorid_ && (roomObjRef->nopstate_ & FT_ROOMOPSTATUS_CLOSE_ROOM))
	{
		LOG_PRINT(log_error, "[connid=%llu,gateid=%u,userid=%u,roomid=%u,ncreatorid:%u,nopstate:%u]room is closed, error: 405",
				pGateMask->param2, m_gateid, pReq.userid(), pReq.vcbid(), roomObjRef->ncreatorid_, roomObjRef->nopstate_);
		jr_send_resp_err(task_node, ERR_ROOM_IS_CLOSED);
		return false;
	}
	return true;
}

bool CLogicCore::jr_user_in_blacklist(task_proc_data * task_node)
{
	GEN_MSGINFO_PF(task_node->pdata, in_msg, pGateMask, pReq, CMDJoinRoomReq)

	if (CDBSink().checkBlockUserLst_DB(pReq.userid(), pReq.vcbid())) 
	{
		LOG_PRINT(log_error, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]user is in black list, error: 101",
				pGateMask->param2, m_gateid, pReq.userid(), pReq.vcbid());
		jr_send_resp_err(task_node, ERR_USER_IN_ROOM_BLACK_LIST);
		return true;
	}
	return false;
}

bool CLogicCore::jr_user_in_kickoutlist(task_proc_data *task_node)
{
	GEN_MSGINFO_PF(task_node->pdata, in_msg, pGateMask, pReq, CMDJoinRoomReq)
    
    LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]check whether user is in kick out list.",
    		pGateMask->param2, m_gateid, pReq.userid(), pReq.vcbid());
    
	CDBSink db;
    int total_row = 0;
    Result res;	
    bool ret = false;
    int now = time(0);
    do {
        
        if (!db.build_sql_run(&res, &total_row, db.SQL_s_kickoutuser_DB, pReq.userid(), pReq.vcbid()) )
        {
            ret = false;
            break;
        }
                
        if (total_row == 0)
        {
            ret = false;
        	break;
        }
        
        for (int row; row < total_row; ++row) 
        {
            int expiredTime;
            db.GetFromRes(expiredTime, &res, row, 0);
            if (expiredTime > now) 
            {
                ret = true;
                break;
            }
        }
    } while(0);
    
    db.CleanRes(&res);
    
    if (!ret)
    {    
        LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]user is not in kick out list.", pGateMask->param2, m_gateid, pReq.userid(), pReq.vcbid());
    }
    else
    {    
        LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]user is in kick out list.", pGateMask->param2, m_gateid, pReq.userid(), pReq.vcbid());
        jr_send_resp_err(task_node, ERR_ROOM_KICKOUT_AD);
    }
           
    if ( (!ret&&total_row > 0) || (ret&&total_row > 1) )
    	db.build_sql_run(&res, &total_row, db.SQL_d_kickoutuser_DB, now);
	
	return ret;
} 
   
bool CLogicCore::jr_room_isfull(task_proc_data * task_node, RoomObject_Ref & roomObjRef, UserObject_Ref & NewUserObjRef)
{
	GEN_MSGINFO_PF(task_node->pdata, in_msg, pGateMask, pReq, CMDJoinRoomReq)

	LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]check whether room is full.", pGateMask->param2, m_gateid, pReq.userid(), pReq.vcbid());
	unsigned int visitorCount = roomObjRef->getVisitorNum();
	if(roomObjRef->user_num + visitorCount >= roomObjRef->capacity)
	{
		LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]user_num:%u,visitor_num:%u,capacity:%u.", \
			pGateMask->param2, m_gateid, pReq.userid(), pReq.vcbid(), roomObjRef->user_num, visitorCount, roomObjRef->capacity);

		if(NewUserObjRef->nuserlevel_ < e_AdminRole)
		{	
			//小于等于注册用户
			LOG_PRINT(log_warning, "[normal user join room]Failed to join room:%u,which is full！", pReq.vcbid());
			jr_send_resp_err(task_node, ERR_ROOM_USER_IS_FULL);
			return true;
		}
	}
	return false;
}

int CLogicCore::jr_check_room_passwd(task_proc_data * task_node, RoomObject_Ref & roomObjRef, UserObject_Ref & NewUserObjRef, bool checkroompwd)
{
	GEN_MSGINFO_PF(task_node->pdata, in_msg, pGateMask, pReq, CMDJoinRoomReq)

	uint32 vcbid = pReq.vcbid();
	uint32 userid = pReq.userid();

	if (!checkroompwd)
	{
		LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]not need to verify room password before join room.",
				pGateMask->param2, m_gateid, userid, vcbid);
		return 0;
	}
	else
	{
		LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]verify room password before join room.",
				pGateMask->param2, m_gateid, userid, vcbid);
	}

	if(roomObjRef->strpwd_.length() > 1 && roomObjRef->busepwd_)
	{
		if(roomObjRef->IsNoneedkey(userid, NewUserObjRef->nuserlevel_, NewUserObjRef->ninroomlevel_))
			return 0;
		else if(roomObjRef->isRoomFangzhu(NewUserObjRef->nuserid_) || roomObjRef->isRoomFuFangzhu(NewUserObjRef->nuserid_))
			return 0;
		else{
			if (strcmp(roomObjRef->strpwd_.c_str(), pReq.croompwd().c_str()) == 0)
				return 0;
			else{
				jr_send_resp_err(task_node, ERR_JOINROOM_PWD_WRONG);
				LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]room pass: %s, request pass: %s",
						pGateMask->param2, m_gateid, userid, vcbid, roomObjRef->strpwd_.c_str(), (char*)pReq.croompwd().c_str());
				return -1;
			}
		}
	}
	return 0;
}
int CLogicCore::jr_check_user_exist(task_proc_data * task_node, RoomObject_Ref & pRoomObjRef, UserObject_Ref & pUserObjRef, UserObject_Ref & NewUserObjRef, bool & bsame_user_and_room)
{
	GEN_MSGINFO_PF(task_node->pdata, in_msg, pGateMask, pReq, CMDJoinRoomReq)

	uint32 vcbid = pReq.vcbid();
	uint32 userid = pReq.userid();

	LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]check whether user is existed.", pGateMask->param2, m_gateid, userid, vcbid);
	uint32 noldvcbid = 0;
	if (CRoomObj::findOnlineUser(m_pSvrEvent->getRoomMgr(), m_pSvrEvent->getRoomMgr()->getDataRedis(), userid, pUserObjRef) && pUserObjRef) {
		noldvcbid = pUserObjRef->ngroupid;
		RoomObject_Ref poldRoomObjRef = m_pSvrEvent->m_roomMgr.FindRoom(noldvcbid);
		LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]existed user info, room:%u, gateid:%u, connid:%llu, oldroomid:%u",
			pGateMask->param2, m_gateid, userid, vcbid, vcbid, pUserObjRef->ngateid, pUserObjRef->pGateObjId_, noldvcbid);

		/*相同session的多次join room*/
		if (pUserObjRef->ngroupid == vcbid && pUserObjRef->ngateid == m_gateid && pUserObjRef->pGateObjId_ == pGateMask->param2){
			pRoomObjRef->sendJoinRoomResp(task_node, pUserObjRef, pGateMask->param1, pGateMask->param2, 0);
			LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]user has joined room yet, ignore.", pGateMask->param2, m_gateid, userid, vcbid);
			return 1;
		}
		else
		{
			/*不同session的join room处理*/
			LOG_PRINT(log_warning, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]kick out old user(connid=%llu,roomid=%d,IP=%s)",
				pGateMask->param2, m_gateid, userid, vcbid, pUserObjRef->pGateObjId_, vcbid, pUserObjRef->ipaddr_.c_str());

			//同号同房的处理
			if (noldvcbid == vcbid){
				char szBuf[512] = { 0 };
				ClientGateMask_t* pClientGate;
				int nlen = poldRoomObjRef->Build_NetMsg_RoomKickoutUserPack(szBuf, 512, 0, pUserObjRef, ERR_KICKOUT_SAMEACCOUNT, 0, &pClientGate);
				pClientGate->param1 = pUserObjRef->pGateObj_;
				pClientGate->param2 = pUserObjRef->pGateObjId_;

				LOG_PRINT(log_info, "[connid=%u,gateid=%u,userid=%u,roomid=%u]pUserObjRef->pGateObj_: %llu pUserObjRef->pGateObjId_: %llu nlen: %d",
					pGateMask->param2, task_node->connection->getgateid(), userid, vcbid, pUserObjRef->pGateObj_, pUserObjRef->pGateObjId_, nlen);
				if (pUserObjRef->pConn_ && pUserObjRef->pConn_->isconnected() && nlen > 0)
				{
					pUserObjRef->pConn_->write_message(szBuf, nlen);
					LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]send kick out same user same room message, old room:%u connid:%llu gate:%u",
						pGateMask->param2, task_node->connection->getgateid(), userid, vcbid, noldvcbid, pUserObjRef->pGateObjId_, pUserObjRef->ngateid);
				}
				bsame_user_and_room = true;
			}
			//			else{ /*同号不同房的处理*/
			//				if (poldRoomObjRef)
			//				{
			//					poldRoomObjRef->castSendRoomKickoutUserInfo(0, pUserObjRef, ERR_KICKOUT_SAMEACCOUNT, 0);
			//				}
			//			}

			// TODO: pUserObjRef->ngroupid = vcbid; ??
			if (poldRoomObjRef->eraseVisitUser(pUserObjRef) < 0)
				return -1;
		}
	}

	return 0;
}
int CLogicCore::jr_check_user_visit_exist(task_proc_data * task_node, RoomObject_Ref & pRoomObjRef, UserObject_Ref & pUserObjRef, UserObject_Ref & NewUserObjRef, bool & bsame_user_and_room)
{
	GEN_MSGINFO_PF(task_node->pdata, in_msg, pGateMask, pReq, CMDJoinRoomReq)

	CRoomManager * pRoomMgr = m_pSvrEvent->getRoomMgr();
	uint32 vcbid = pReq.vcbid();
	uint32 userid = pReq.userid();

	LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]check whether user is existed.", pGateMask->param2, m_gateid, userid, vcbid);
	RoomObject_Ref poldRoomObjRef;
	redisOpt * pRedis = m_pSvrEvent->m_pRedisMgr->getOne();
	uint32 noldvcbid = 0;
	UserObject_Ref tmpuserRef;
	if (CRoomObj::findVisitUser(&m_pSvrEvent->m_roomMgr, pRedis, tmpuserRef, userid)) {
		noldvcbid = tmpuserRef->ngroupid;
	}

	LOG_PRINT(log_warning, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]matched old room:%u, now join room: %u",
			pGateMask->param2, m_gateid, userid, vcbid, noldvcbid, vcbid);

	if (noldvcbid != 0){
		poldRoomObjRef = pRoomMgr->FindRoom(noldvcbid);
		if (0 == poldRoomObjRef.get() || poldRoomObjRef->isClosed()){
			jr_send_resp_err(task_node, ERR_ROOM_IS_CLOSED);
			if (0 == poldRoomObjRef.get()){
				LOG_PRINT(log_error, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]not found old room:%u",
						pGateMask->param2, m_gateid, userid, vcbid, noldvcbid);
			}
			else if (poldRoomObjRef->isClosed()){
				LOG_PRINT(log_error, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]old room:%u is closed",
						pGateMask->param2, m_gateid, userid, vcbid, noldvcbid);
			}
			return -1;
		}
		poldRoomObjRef->findVisitUser(pUserObjRef, userid);
	}

	// found existed user
	if(pUserObjRef.get() != 0) {
		LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]existed user info, room:%u, gateid:%u, connid:%llu",
				pGateMask->param2, m_gateid, userid, vcbid, vcbid, pUserObjRef->ngateid, pUserObjRef->pGateObjId_);
		LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]new user info, room:%u, gateid:%u, connid:%llu",
				pGateMask->param2, m_gateid, userid, vcbid, vcbid, m_gateid, pGateMask->param2);

		/*相同session的多次join room*/
		if (pUserObjRef->ngroupid == vcbid && pUserObjRef->ngateid == m_gateid && pUserObjRef->pGateObjId_ == pGateMask->param2){
			LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]same user, same room, same gate, same connid",
					pGateMask->param2, m_gateid, userid, vcbid);
			LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]check whether room is favorited.", pGateMask->param2, m_gateid, userid, vcbid);
			int biscollectroom = 0;
//			if (CDBSink().isCollectRoom(userid, vcbid))
//				biscollectroom = 1;
			pRoomObjRef->sendJoinRoomResp(task_node, pUserObjRef, pGateMask->param1, pGateMask->param2, biscollectroom);
			LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]user joined room.", pGateMask->param2, m_gateid, userid, vcbid);
			return 1;
		}
		else
		{
			/*之前有拦截except exit的消息*/
			if (pUserObjRef->bExceptExit) {
				char key[64] = {0};
				sprintf(key, "%u:%u", noldvcbid, userid);
				if (noldvcbid == vcbid){
					if (m_pSvrEvent->m_mapExceptExit.remove(key)) {
						LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]same room, remove except exit item from except exit queue",
								pGateMask->param2, m_gateid, userid, vcbid);
						bsame_user_and_room = true;
					}
				}
				else {
					boost::mutex::scoped_lock lock(m_pSvrEvent->m_mapExceptExit.getmutex());
					tsmap<string, ExceptExitRecord_t>::iterator it = m_pSvrEvent->m_mapExceptExit.getcon().find(key);
					if (it != m_pSvrEvent->m_mapExceptExit.getcon().end()){
						LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]diff room, remove except exit item from except exit queue",
								pGateMask->param2, m_gateid, userid, vcbid);
						ExceptExitRecord_t *pexitRecord = (ExceptExitRecord_t *)&(it->second);
						handle_except_exitroom_delay(pexitRecord);
						m_pSvrEvent->m_mapExceptExit.getcon().erase(it);
						return 0;
					}
				}
			}
			else
			{
				/*不同session的join room处理*/
				LOG_PRINT(log_warning, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]kick out old user(connid=%llu,roomid=%d,IP=%s)",
						pGateMask->param2, m_gateid, userid, vcbid, pUserObjRef->pGateObjId_, vcbid, pUserObjRef->ipaddr_.c_str());

				//同号同房的处理
				if (noldvcbid == vcbid && pUserObjRef->ndevtype == pReq.devtype()){
					/* 如果是同号同房并且登陆设备类型一样则不广播消息，只发给旧的登陆用户退出房间 */
					char szBuf[512] = {0};
					ClientGateMask_t* pClientGate;
					int nlen = poldRoomObjRef->Build_NetMsg_RoomKickoutUserPack(szBuf, 512, 0, pUserObjRef, ERR_KICKOUT_SAMEACCOUNT, 0, &pClientGate);
					pClientGate->param1 = pUserObjRef->pGateObj_;
					pClientGate->param2 = pUserObjRef->pGateObjId_;

					LOG_PRINT(log_info, "[connid=%u,gateid=%u,userid=%u,roomid=%u]pUserObjRef->pGateObj_: %llu pUserObjRef->pGateObjId_: %llu nlen: %d",
							pGateMask->param2, task_node->connection->getgateid(), userid, vcbid, pUserObjRef->pGateObj_, pUserObjRef->pGateObjId_, nlen);
					if(pUserObjRef->pConn_ && pUserObjRef->pConn_->isconnected() && nlen > 0 )
					{
						pUserObjRef->pConn_->write_message(szBuf, nlen);
						LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]send kick out same user same room message, old room:%u connid:%llu gate:%u",
								pGateMask->param2, task_node->connection->getgateid(),userid, vcbid, noldvcbid, pUserObjRef->pGateObjId_, pUserObjRef->ngateid);
					}
					bsame_user_and_room = true;
				}
				else{ /*同号不同房的处理*/
					poldRoomObjRef->castSendRoomKickoutUserInfo(0, pUserObjRef, ERR_KICKOUT_SAMEACCOUNT, 0);
				}
			}

			if (poldRoomObjRef->eraseVisitUser(pUserObjRef) < 0)
				return -1;
		}
	}

	return 0;
}

void CLogicCore::jr_send_resp_err(task_proc_data *task_node, int errid)
{
	GEN_MSGINFO_PF(task_node->pdata, in_msg, pGateMask, pReq, CMDJoinRoomReq);
	CRoomManager * pRoomMgr = m_pSvrEvent->getRoomMgr();
	char szBuf[512] = {0};
	ClientGateMask_t * pClientGate;
	CMDJoinRoomErr Error;
	Error.set_vcbid(pReq.vcbid());
	Error.set_userid(pReq.userid());
	Error.mutable_errinfo()->set_errid(errid);
	SERIALIZETOARRAY_BUF(Error, respbuf, resplen);
	int nMsgLen = pRoomMgr->Build_NetMsg(szBuf, 512, MDM_Vchat_Room, Sub_Vchat_JoinRoomErr, &pClientGate, respbuf, resplen, in_msg->reqid);
	pClientGate->param1 = pGateMask->param1;
	pClientGate->param2 = pGateMask->param2;
	COM_MSG_HEADER * pErrHead = (COM_MSG_HEADER *)(szBuf);
	pErrHead->reqid = in_msg->reqid;
	task_node->connection->write_message(szBuf, nMsgLen);
}

void CLogicCore::jr_reset_user_info(task_proc_data * task_node, RoomObject_Ref & pRoomObjRef, UserObject_Ref & pUserObjRef, string & areaid)
{
	GEN_MSGINFO_PF(task_node->pdata, in_msg, pGateMask, pReq, CMDJoinRoomReq)
	pUserObjRef->ngroupid = pReq.vcbid();
	pUserObjRef->nsvrid = m_svrid;
	pUserObjRef->ngateid = m_gateid;
	pUserObjRef->ncuronmictype_     = 0;
	pUserObjRef->ncurpublicmicindex_ = -1;
	pUserObjRef->nstarttime_pubmic_ = 0;
	pUserObjRef->nwillendtime_pubmic_ = 0;
//	pUserObjRef->inroomstate_ = 0;
	pUserObjRef->cometime_ = time(0);
	pUserObjRef->type_ = USERTYPE_NORMALUSER;
	pUserObjRef->pConnId_ = task_node->connection->getconnid();
	pUserObjRef->pGateObj_ = pGateMask->param1;
	pUserObjRef->pGateObjId_ = pGateMask->param2;
	pUserObjRef->ipaddr_= pReq.cipaddr().c_str();
	pUserObjRef->uuid_ = pReq.cserial().c_str();
	pUserObjRef->areaid = areaid; //记录区域id
	pUserObjRef->bForbidInviteUpMic_ = 0;
	pUserObjRef->bForbidChat_ = pRoomObjRef->IsForbidChatUser(pUserObjRef->nuserid_); //被禁言用户
	pUserObjRef->ndevtype = pReq.devtype();
	LOG_PRINT(log_info, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]reset user info, pGateObj:%llu, pGateObjId:%llu",
			pGateMask->param2, m_gateid, pReq.userid(), pReq.vcbid(), pUserObjRef->pGateObj_,pUserObjRef->pGateObjId_);
}

int CLogicCore::proc_setdevstate_req(task_proc_data *task_node)
{
	//每次都是收到的最终音视频状态
	GEN_MSGINFO_AND_CHECK_LENGTH(task_node->pdata, in_msg, pGateMask, pReq, CMDUserDevState_t);
	CRoomManager *pRoomMgr = m_pSvrEvent->getRoomMgr();

	RoomObject_Ref pRoomObjRef=pRoomMgr->FindRoom(pReq->vcbid);
	if(pRoomObjRef.get()==0) {
	    resperrinf(in_msg, pGateMask, ERR_CODE_FAILED_ROOMIDNOTFOUND, task_node);
		LOG_PRINT(log_error, "not found room: %d", pReq->vcbid);
		return 0;
	}
	if(pReq->videostate ==0 && pReq->audiostate ==0)
	{
		//没有任何设备变化，不做任何处理
		resperrinf(in_msg, pGateMask, ERR_CODE_FAILED_INVALIDCHAR, task_node);
		LOG_PRINT(log_warning, "not changed.");
		return 0;
	}

	UserObject_Ref pUserRef;
	if(pRoomObjRef->findVisitUser(pUserRef, pReq->userid))
	{
		//先重置所以的旧状态,再设置新状态
		if(pReq->videostate !=0)
		{
			pUserRef->inroomstate_ &= ~FT_ROOMUSER_STATUS_VIDEOON;
			pUserRef->inroomstate_ &= ~FT_ROOMUSER_STATUS_VIDEOOFF;
			if(pReq->videostate == 1)
				pUserRef->inroomstate_ |= FT_ROOMUSER_STATUS_VIDEOOFF;
			else if(pReq->videostate == 2)
				pUserRef->inroomstate_ |= FT_ROOMUSER_STATUS_VIDEOON;
		}
		if(pReq->audiostate !=0)
		{
			pUserRef->inroomstate_ &= ~FT_ROOMUSER_STATUS_MICOFF;
			if(pReq->audiostate == 1)
				pUserRef->inroomstate_ |= FT_ROOMUSER_STATUS_MICOFF;
		}

		//原消息修改 发送resp
		pReq->userinroomstate = pUserRef->inroomstate_;
		in_msg->subcmd = Sub_Vchat_SetDevStateResp;
		task_node->connection->write_message((char*)in_msg, in_msg->length);

		//原消息修改 广播新状态
		in_msg->subcmd = Sub_Vchat_SetDevStateNotify;
		LOG_PRINT(log_info, "Cast update state");
		pRoomObjRef->castSendMsgXXX(in_msg);
	}

	return 0;
}

int CLogicCore::proc_setroomopstatus_req(task_proc_data *task_node )
{
	GEN_MSGINFO_AND_CHECK_LENGTH(task_node->pdata, in_msg, pGateMask, pReq, CMDSetRoomOPStatusReq_t);
	CRoomManager* pRoomMgr = m_pSvrEvent->getRoomMgr();

	RoomObject_Ref pRoomObjRef=pRoomMgr->FindRoom(pReq->vcbid);
	if(pRoomObjRef.get()==0) {
		LOG_PRINT(log_warning, "not found room: %d", pReq->vcbid);
		return 0;
	}

	int nErrorId=0;
	UserObject_Ref pUserRef;
	if(false == pRoomObjRef->findMemberUser(pUserRef, pReq->runnerid)){
		nErrorId = 503;
		LOG_PRINT(log_error, "not found user:%u in room:%u", pReq->runnerid, pReq->vcbid);
		return -1;
	}

	pRoomObjRef->nopstate_ = pReq->opstatus;

//	bool bWaitMicListChanged = false;
//	int nOldWaitMicCount = pRoomObjRef->getWaitMicUserCount();
//	pRoomObjRef->checkWaitMicToPubMic(true);
//	int nNewWaitMicCount = pRoomObjRef->getWaitMicUserCount();
//	if(nOldWaitMicCount != nNewWaitMicCount)
//		bWaitMicListChanged = true;
//	if(bWaitMicListChanged)
//		pRoomObjRef->castSendRoomWaitMicList();

	char szBuf[512];
	ClientGateMask_t* pClientMask;
	CMDSetRoomOPStatusResp_t respInfo;
	respInfo.vcbid = pReq->vcbid;
	respInfo.errorid = nErrorId;
	int nMsgLen = pRoomMgr->Build_NetMsg(szBuf, 512, MDM_Vchat_Room, Sub_Vchat_SetRoomOPStatusResp, &pClientMask, &respInfo, sizeof(CMDSetRoomOPStatusResp_t));
	if(nMsgLen > 0)
	{
		pClientMask->param1 =pGateMask->param1;
		pClientMask->param2 =pGateMask->param2;
		task_node->connection->write_message(szBuf, nMsgLen);
	}
	if (0 == nErrorId)
		pRoomObjRef->castSendRoomOPStatus();

	return 0;
}

int CLogicCore::proc_setroommedia_req(task_proc_data *task_node )
{
	GEN_MSGINFO_AND_CHECK_LENGTH(task_node->pdata, in_msg, pGateMask, pReq, CMDRoomMediaInfo_t);
	CRoomManager* pRoomMgr = m_pSvrEvent->getRoomMgr();

	RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(pReq->vcbid);
	if(pRoomObjRef.get()==0) {
		LOG_PRINT(log_error, "not found room: %d", pReq->vcbid);
		return 0;
	}

	pRoomObjRef->strmedia_=pReq->caddr;
	CDBSink().update_vcbmedia_DB(pReq->vcbid, pReq->caddr);

	pRoomObjRef->castSendRoomMediaInfo();
	return 0;
}

//int CLogicCore::proc_softbot_joinroom_req(task_proc_data *task_node)
//{
//	GEN_MSGINFO_AND_CHECK_LENGTH(task_node->pdata, in_msg, pGateMask, pReq, CMDSoftbotJoinRoomEXT_t);
//
//	map<uint8, uint8> mapVipPercent;
//	CMDVipLevelRate_t *pItem = (CMDVipLevelRate_t*)pReq->content;
//	for (uint8 i=0; i<pReq->viplevelnum; i++) {
//		mapVipPercent.insert(std::make_pair(pItem->viplevel, pItem->vipratio));
//		++pItem;
//	}
//
//	CRoomManager* pRoomMgr = m_pSvrEvent->getRoomMgr();
//	RoomObject_Ref roomObjRef = pRoomMgr->FindRoom(pReq->roomid);
//	if (!roomObjRef.get()){
//		LOG_PRINT(log_error, "not found room:%u", pReq->roomid);
//		return -1;
//	}
//
//	LOG_PRINT(log_info, "[room:%u,num:%d]softbot join room", pReq->roomid, pReq->num);
//
//	for (size_t i=0; i<pReq->num; i++) {
//		uint32 softbot_id = pRoomMgr->getsoftbotid();
//		if (0 == softbot_id) {
//			LOG_PRINT(log_info, "invalid userid: %u", softbot_id);
//			return -1;
//		}
//		redisOpt *pRedis = m_pSvrEvent->m_pRedisMgr->getOne();
//		UserObject_Ref olduserRef;
//		uint32 noldvcbid = 0;
//		if (CRoomObj::findVisitUser(&m_pSvrEvent->m_roomMgr, pRedis, olduserRef, softbot_id)) {
//			noldvcbid = olduserRef->nvcbid_;
//		}
//
//		if (0 != noldvcbid) {
//			pRoomMgr->getnextsoftbotid();
//			continue;
//		}
//
//		byte nuserviplevel = 0;
//		map<uint8, uint8>::const_iterator it = mapVipPercent.begin();
//		for (; it!=mapVipPercent.end(); ++it) {
//			uint8 viplevel = it->first;
//			//if (roomObjRef->nviplevel_ <= viplevel) {
//				uint8 percent = it->second;
//				// if no reach the vip percent, add vip softbot
//				if ((roomObjRef->getSoftbotCountPerViplevel(viplevel) + roomObjRef->getUserCountPerViplevel(viplevel))
//						< ((double)roomObjRef->ntotalcount_ * (double)percent / 100.0)) {
//					nuserviplevel = viplevel;
//					break;
//				}
//			//}
//		}
////		if (nuserviplevel < roomObjRef->nviplevel_)
////			nuserviplevel = roomObjRef->nviplevel_;
//
//		UserObject_Ref pUserObjRef(new CUserObj());
//		pUserObjRef->nsvrid = m_svrid;
//		pUserObjRef->ngateid = m_gateid;
//		pUserObjRef->nuserid_ = softbot_id;
//		pUserObjRef->nvcbid_ = pReq->roomid;
//		//pUserObjRef->nheadid_  = 0;
//		pUserObjRef->nisadregister_ = 0;
//		pUserObjRef->ngender_  = softbot_id % 2;
//		pUserObjRef->nage_     = softbot_id % 10 + 30;
//		pUserObjRef->nuserlevel_      = 2;	//注册用户
//		pUserObjRef->nuserviplevel_ = nuserviplevel;
////		pUserObjRef->nyiyuanlevel_   = 0;
////		pUserObjRef->nshoufulevel_   = 0;
////		pUserObjRef->nzhongshenlevel_= 0;
////		pUserObjRef->ncaifulevel_    = 0;
////		pUserObjRef->lastmonthcostlevel = 0;
////		pUserObjRef->thismonthcostlevel = 0;
////		pUserObjRef->thismonthcostgrade = 0;
//		pUserObjRef->ncuronmictype_     = 0;
//		pUserObjRef->ncurpublicmicindex_ = -1;
//		pUserObjRef->nstarttime_pubmic_ = 0;
//		pUserObjRef->nwillendtime_pubmic_ = 0;
////		pUserObjRef->nflowernum_ = 0;
////		pUserObjRef->nyuanpiaonum_ = 0;
////		pUserObjRef->nchestnum_ = 0;
////		pUserObjRef->nsealexptime_ = 0;
//		pUserObjRef->inroomstate_ = 0;    //
//		pUserObjRef->ninroomlevel_ = 0;
//		pUserObjRef->cometime_ = time(0); //
//		pUserObjRef->nstarflag_ = 0;
//		pUserObjRef->nactivityflag_ = 0;
//		pUserObjRef->type_ = USERTYPE_SOFTBOT;
//		//pUserObjRef->pConn_ = task_node->connection;  //
//		pUserObjRef->pConnId_ = task_node->connection->getconnid();
//		pUserObjRef->pGateObj_ = 0;   //
//		pUserObjRef->pGateObjId_ = 0; //
//		pUserObjRef->calias_ = "";
//		{
//			//boost::mutex::scoped_lock lock(pRoomMgr->m_softbot_alias_mutex);
//			if (pRoomMgr->m_vecSoftbotAlias.size() > 0){
//				pUserObjRef->calias_ = pRoomMgr->m_vecSoftbotAlias[0];
//				pRoomMgr->m_vecSoftbotAlias.erase(pRoomMgr->m_vecSoftbotAlias.begin());
//				pRoomMgr->m_vecSoftbotAlias_used.push_back(pUserObjRef->calias_);
//			}
//		}
//		pUserObjRef->headpic_ = "0.png";
//		//pUserObjRef->nisxiaoshou_ = 0;
//		pUserObjRef->macserial_ = "";
//		pUserObjRef->ipaddr_= "";
//		pUserObjRef->uuid_ = "";
//		pUserObjRef->areaid = ""; //记录区域id
//		pUserObjRef->bForbidInviteUpMic_ = 0;  //
//		pUserObjRef->bForbidChat_ = 0; //被禁言用户
//		pUserObjRef->ndevtype = 0;
//
//		LOG_PRINT(log_info, "[user:%u,room:%u,nuserviplevel:%d] add softbot user", pUserObjRef->nuserid_, pUserObjRef->nvcbid_, pUserObjRef->nuserviplevel_);
//
//		if (roomObjRef->InsertUser(pUserObjRef) < 0)
//			return -1;
//
//		roomObjRef->castSendRoomUserComeInfo(pUserObjRef);
//		pRoomMgr->getnextsoftbotid();
//	}
//
//	return 0;
//}
//
//int CLogicCore::proc_softbot_exitroom_req(task_proc_data *task_node)
//{
//	GEN_MSGINFO(task_node->pdata, in_msg, pGateMask, pReq, CMDSoftbotJoinRoomEXT_t);
//	CRoomManager* pRoomMgr = m_pSvrEvent->getRoomMgr();
//
//	RoomObject_Ref roomObjRef = pRoomMgr->FindRoom(pReq->roomid);
//	if (!roomObjRef.get()){
//		LOG_PRINT(log_error, "not found room:%u", pReq->roomid);
//		return -1;
//	}
//
//	map<uint8, uint8> mapVipPercent;
//	CMDVipLevelRate_t *pItem = (CMDVipLevelRate_t*)pReq->content;
//	for (uint8 i=0; i<pReq->viplevelnum; i++) {
//		mapVipPercent.insert(std::make_pair(pItem->viplevel, pItem->vipratio));
//		LOG_PRINT(log_debug, "[viplevel:%u,vipratio:%u]", pItem->viplevel, pItem->vipratio);
//		++pItem;
//	}
//
//	LOG_PRINT(log_info, "[room:%u,num:%d]softbot exit room", pReq->roomid, pReq->num);
//	for (size_t i=0; i<pReq->num; i++) {
//		UserObject_Ref userObjRef;
//		uint8 viplevel, percent;
//		map<uint8, uint8>::reverse_iterator it = mapVipPercent.rbegin();
//		for (; it != mapVipPercent.rend(); it++) {
//			viplevel = it->first;
//			percent = it->second;
//			LOG_PRINT(log_debug, "[viplevel:%u,percent:%d] start to find softbot user", viplevel, percent);
//			// 从最高等级开始退出机器人
//			if ((roomObjRef->getSoftbotCountPerViplevel(viplevel) + roomObjRef->getUserCountPerViplevel(viplevel))
//					>= ((double)roomObjRef->ntotalcount_ * (double)percent / 100.0)) {
//				UserObject_Ref tmpuserobjRef;
//				if (false == roomObjRef->getSoftbot(tmpuserobjRef, viplevel))
//					continue;
//
//				userObjRef = tmpuserobjRef;
//				break;
//			}
//		}
//
//		if (!userObjRef) {
//			viplevel = 0;
//			roomObjRef->getSoftbot(userObjRef, viplevel);
//		}
//		if (!userObjRef) {
//			LOG_PRINT(log_warning, "not found the softbot user to be exited");
//			return -1;
//		}
//
//		LOG_PRINT(log_debug, "[viplevel:%u,percent:%u]the softuser:%u userviplevel:%u will exit", viplevel, percent,
//				userObjRef->nuserid_, userObjRef->nuserviplevel_);
//
//		if (roomObjRef->eraseVisitUser(userObjRef) < 0)
//			return -1;
//		roomObjRef->castSendRoomUserLeftInfo(userObjRef);
//		{
//			//boost::mutex::scoped_lock lock(pRoomMgr->m_softbot_alias_mutex);
//			vector<string>::iterator it = find(pRoomMgr->m_vecSoftbotAlias_used.begin(), pRoomMgr->m_vecSoftbotAlias_used.end(), userObjRef->calias_);
//			if (it != pRoomMgr->m_vecSoftbotAlias_used.end()) {
//				pRoomMgr->m_vecSoftbotAlias.push_back(*it);
//				pRoomMgr->m_vecSoftbotAlias_used.erase(it);
//			}
//		}
//	}
//
//	return 0;
//}

void CLogicCore::onTimer()
{
	time_t now = time(0);

	static time_t s_timeoutGuard_179sec = now;
	if(0 == now % 179 || now - s_timeoutGuard_179sec > 179)//课程开始前提醒
	{
		s_timeoutGuard_179sec = now;
		pushCourseStartNotice();
	}

	static time_t s_timeoutGuard_3559sec = now;
	if(0 == now % 3559 || now - s_timeoutGuard_3559sec > 3559)//更新在线人数
	{
		s_timeoutGuard_3559sec = now;

		std::string Hour = getTime("%H");
		LOG_PRINT(log_error, "room_tag Hour:%s", Hour.c_str());

		if(Hour == "01")
		{
			//max count
			CRoomManager* pRoomMgr = m_pSvrEvent->getRoomMgr();
			vector<string> vallrooms = pRoomMgr->getAllRoomid();
			std::map<int,int> mp;
			DBmongoConnection::queryYesterdayMaxUserCount(vallrooms,mp);
			DBmongoConnection::updateRoomMaxCount(getTime("%Y%m%d"),mp);

			//average online countupdateAvgOnline
			DBmongoConnection::queryYesterdayAvgOnline(vallrooms,mp);
			DBmongoConnection::updateAvgOnline(getTime("%Y%m%d"),mp);
			//average online time
			DBmongoConnection::queryYesterdayAvgCount(vallrooms,mp);
			DBmongoConnection::updateAvgCount(getTime("%Y%m%d"),mp);
		}

		if(Hour == "03")//定时关闭课程
		{
			CRoomManager* pRoomMgr = m_pSvrEvent->getRoomMgr();
			std::vector<uint32> vgids;
			CDBSink().getAllYesterDayCourse(vgids);
			for(int i = 0 ; i < vgids.size() ; i++ )
			{
				RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(vgids[i]);
				if(!pRoomObjRef.get())
				{
					LOG_PRINT(log_error, "room_tag not found groupid:%d", vgids[i]);
					continue ;
				}
				pRoomObjRef->setCourseFinish();

				ClientGateMask_t* pClientGate;
				char szBuf[512] = {0};
				CMDCourseFinish noty;
				noty.set_groupid(vgids[i]);
				noty.set_userid(0);
				SERIALIZETOARRAY_BUF(noty,respbuf,len);
				int nMsgLen=pRoomMgr->Build_NetMsg(szBuf,512,MDM_Vchat_Room,Sub_Vchat_CourseFinishNoty,&pClientGate,respbuf,len);

				pRoomObjRef->castSendMsgXXX((COM_MSG_HEADER*)szBuf);
			}

		}

		if(Hour == "02")//造假用户
		{
			std::vector<std::string> m_zombiesleft;
			m_zombiesleft.assign(m_zombies.begin(),m_zombies.end());
			LOG_PRINT(log_error, "room_tag m_zombiesleft:%d",m_zombiesleft.size());
			std::map<uint32,uint32> maplids;
			std::map<uint32,uint32> mapalllids;
			CDBSink().getAllLiveidAndCourseCount(mapalllids);
			LOG_PRINT(log_error, "room_tag getAllLiveidAndCourseCount:%d",mapalllids.size());
			CDBSink().getAllYesterDayLiveidAndCourseCount(maplids);
			LOG_PRINT(log_error, "room_tag getAllYesterDayLiveidAndCourseCount:%d",maplids.size());
			for(std::map<uint32,uint32>::iterator it = maplids.begin() ; it != maplids.end(); it++)
			{
				uint32 allcount = mapalllids[it->first];
				uint32 lcount = it->second;
				uint32 alreadycount = allcount - lcount;
				uint32 leftcount = 10 - alreadycount ;
				LOG_PRINT(log_error, "room_tag allcount:%d",allcount);
				if(leftcount > 0)
				{
					std::vector<std::string> mapUsed;
					CDBSink().getRobotNamebyLiveid(it->first,mapUsed);
					LOG_PRINT(log_error, "room_tag getRobotNamebyLiveid:%d",mapUsed.size());
					for(int i = 0 ;i < mapUsed.size() ; i++)
					{
						for(std::vector<std::string>::iterator it1 = m_zombiesleft.begin() ; it1 != m_zombiesleft.end() ;)
						{
							if(*it1 == mapUsed[i])
							{
								m_zombiesleft.erase(it1++);
							}else
							{
								it1++;
							}
						}
					}
					LOG_PRINT(log_error, "room_tag m_zombiesleft:%d",m_zombiesleft.size());
					random_shuffle(m_zombiesleft.begin(), m_zombiesleft.end());
					uint32 sub = it->second > leftcount ? leftcount : it->second ;
					LOG_PRINT(log_error, "room_tag insert:%d",sub * 20);
					for(int i = 0 ;  i < sub * 20 ; i++)
					{
						CDBSink().insertLiveFocusRobot(it->first,m_zombiesleft[i]);
					}
					CDBSink().updateRobots(it->first,sub * 20);
				}
			}
		}

		if(Hour == "20") // 发布新“单节课”或“系列课”的推送通知
		{
			LOG_PRINT(log_info, "checking new published single courses or serial courses ...");
			pushNewCourseNoticeInBatch();
		}
	}

	if (0 == now % 30)
		LOG_PRINT(log_info, "run here");
	if (0 == now % 5) {
		unsigned int ntime_begin,ntime_end;
		ntime_begin = SL_Socket_CommonAPI::util_process_clock_ms();
		boost::mutex::scoped_lock lock(m_pSvrEvent->m_mapExceptExit.getmutex());
		map<string, ExceptExitRecord_t>::iterator it = m_pSvrEvent->m_mapExceptExit.getcon().begin();
		while (it != m_pSvrEvent->m_mapExceptExit.getcon().end()) {
			vector<string> key = strToVec(it->first, ':');
			ExceptExitRecord_t *pexitRecord = (ExceptExitRecord_t *)&(it->second);
			COM_MSG_HEADER* in_msg = (COM_MSG_HEADER*)pexitRecord->pdata;
			ClientGateMask_t *pGateMask = (ClientGateMask_t *)in_msg->content;
			LOG_PRINT(log_info, "[connid=%u,userid=%s,roomid=%s]delayed except exit queue item.",
					pGateMask->param2, key[1].c_str(), key[0].c_str());
			if (now - pexitRecord->time > m_pSvrEvent->m_nexcept_exit_interval) {
				if (0 == handle_except_exitroom_delay(pexitRecord)) {
					m_pSvrEvent->m_mapExceptExit.getcon().erase(it++);
					LOG_PRINT(log_info, "[connid=%u,userid=%s,roomid=%s]remove except exit record",
							pGateMask->param2, key[1].c_str(), key[0].c_str());
				}
			}
			else
				++it;
		}
		ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();
		if (0 == now % 30){
			size_t size = m_pSvrEvent->m_mapExceptExit.getcon().size();
			LOG_PRINT(log_info, "time elapsed ms = %d, size: %u", ntime_end-ntime_begin, size);
		}
	}

	static time_t s_timeoutGuard_60sec = now;
	if (0 == now % 60 || now - s_timeoutGuard_60sec > 60)
	{
		s_timeoutGuard_60sec = now;

		LOG_PRINT(log_info, "Executing queryAndUpdateLiveFlowStatus() ...");
		queryAndUpdateLiveFlowStatus();

		LOG_PRINT(log_info, "Notifying PHP to push messages ...");
		notifyPhpToPushMessages();
	}
}

clienthandler_ptr CLogicCore::findconn(uint32 connid)
{
	clienthandler_ptr conn_ptr;
	map<uint32, clienthandler_ptr>::iterator it = m_mapConn.find(connid);
	if (it != m_mapConn.end())
		conn_ptr = it->second;

	return conn_ptr;
}

int CLogicCore::proc_setUserPriority_req(task_proc_data * message)
{
	CMDSetUserPriorityReq req;
	if (!CMsgComm::ParseProtoMessage(message->pdata, message->datalen, req))
	{
		LOG_PRINT(log_error, "parse %s packet error! length=%d.", req.GetTypeName().c_str(), message->datalen);
		message->resperrinf(ERR_CODE_FAILED_PACKAGEERROR);
		return -1;
	}

	LOG_PRINT(log_info, "receive proc_setUserPriority_req:run_user[%u] to_user[%u] group[%u] roletype[%u].", req.run_userid(), req.to_userid(), req.groupid(), req.roletype());
	unsigned int ret = 0;
	do
	{
		if (!req.run_userid() || !req.to_userid() || !req.groupid())
		{
			LOG_PRINT(log_error, "proc_setUserPriority_req input error.");
			ret = ERR_CODE_INVALID_PARAMETER;
			break;
		}

		ret = proc_setuserpriority_req(req.run_userid(), req.to_userid(), req.groupid(), req.roletype(), true);
	} while (0);

	LOG_PRINT(log_info, "proc_setUserPriority_req:run_user[%u] to_user[%u] group[%u] roletype[%u] ret[%u].", req.run_userid(), req.to_userid(), req.groupid(), req.roletype(), ret);
	CMDSetUserPriorityResp oRspData;
	oRspData.mutable_errinfo()->set_errid(ret);
	oRspData.set_run_userid(req.run_userid());
	oRspData.set_to_userid(req.to_userid());
	oRspData.set_groupid(req.groupid());
	oRspData.set_roletype(req.roletype());

	unsigned int rspDataLen = SIZE_IVM_HEAD_TOTAL + oRspData.ByteSize();
	SL_ByteBuffer buff(rspDataLen);
	buff.data_end(rspDataLen);

	COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *) message->pdata;
	ClientGateMask_t * pGateMask = (ClientGateMask_t *)(in_msg->content);
	COM_MSG_HEADER * pRspHead = (COM_MSG_HEADER *)buff.buffer();
	ClientGateMask_t * pRspGate = (ClientGateMask_t *)(pRspHead->content);
	GEN_RSP_MSGINFO_SET_HEAD_GATE(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_SetUserPriorityResp, in_msg->reqid, pGateMask, pRspHead, pRspGate, oRspData);
	message->connection->write_message(buff);
	return 0;
}

int CLogicCore::proc_setuserpriority_req(unsigned int run_userid, unsigned int to_userid, unsigned int groupid, unsigned int roleType, bool checkRunID /*= true*/)
{
	LOG_PRINT(log_info, "[proc_setuserpriority_req]groupid:%u,runnerid:%u,touserid:%u,roleType:%u.", groupid, run_userid, to_userid, roleType);

	unsigned int nErrorId = ERR_CODE_SUCCESS;
	CRoomManager * pRoomMgr = m_pSvrEvent->getRoomMgr();
	if (!pRoomMgr)
	{
		LOG_PRINT(log_error, "can not find group error.groupid:%u,runnerid:%u,touserid:%u,roleType:%u", groupid, run_userid, to_userid, roleType);
		return ERR_CODE_FAILED_ROOMIDNOTFOUND;
	}

	RoomObject_Ref pRoomObjRef;
	UserObject_Ref pToUserRef;
	bool isMember = false;
	do 
	{
		pRoomObjRef = pRoomMgr->FindRoom(groupid);
		if(pRoomObjRef.get() == 0) 
		{
			LOG_PRINT(log_error, "[proc_setuserpriority_req fail]can not find this room.groupid:%u,runnerid:%u,touserid:%u,roleType:%u.", \
				groupid, run_userid, to_userid, roleType);
			nErrorId = ERR_CODE_FAILED_ROOMIDNOTFOUND;
			break;
		}

		if (!checkRunID)
		{
			LOG_PRINT(log_warning, "[proc_setuserpriority_req]not need to check runnid,groupid:%u.", groupid);
			break;
		}

		if(!run_userid || !to_userid || run_userid == to_userid) 
		{
			LOG_PRINT(log_error, "[proc_setuserpriority_req fail]input error.groupid:%u,runnerid:%u,touserid:%u,roleType:%u.", groupid, run_userid, to_userid, roleType);
			nErrorId = ERR_CODE_INVALID_PARAMETER;
			break;
		}

		if (roleType == e_OwnerRole)
		{
			LOG_PRINT(log_error, "[proc_setuserpriority_req fail]user cannot change group owner.groupid:%u,runnerid:%u,touserid:%u,roleType:%u.", groupid, run_userid, to_userid, roleType);
			nErrorId = ERR_CODE_GROUP_NOT_CHANGE_OWNER;
			break;
		}

		if (roleType != e_VisitorRole && roleType != e_MemberRole && roleType != e_GuestRole && roleType != e_AdminRole)
		{
			LOG_PRINT(log_error, "[proc_setuserpriority_req fail]roleType is wrong.groupid:%u,runnerid:%u,touserid:%u,roleType:%u.", groupid, run_userid, to_userid, roleType);
			nErrorId = ERR_CODE_INVALID_PARAMETER;
			break;
		}

		UserObject_Ref pRunUserRef;
		if (!pRoomObjRef->findMemberUser(pRunUserRef, run_userid))
		{
			LOG_PRINT(log_error, "[proc_setuserpriority_req fail]run userid is not group member.group:%u,runnerid:%u,touserid:%u,roleType:%u.", \
				groupid, run_userid, to_userid, roleType);
			nErrorId = ERR_CODE_USER_NOT_IN_GROUP;
			break;
		}

		isMember = pRoomObjRef->findMemberUser(pToUserRef, to_userid);
		if (pRunUserRef->nuserlevel_ < e_AdminRole 
			|| (isMember && pRunUserRef->nuserlevel_ <= pToUserRef->nuserlevel_)
			|| (roleType >= e_AdminRole && pRunUserRef->nuserlevel_ != e_OwnerRole))
		{
			LOG_PRINT(log_error, "[proc_setuserpriority_req fail]run user has no right to set user priority.group:%u,runnerid:%u,runnid roleType:%u,touserid:%u,roleType:%u.", \
				groupid, run_userid, pRunUserRef->nuserlevel_, to_userid, roleType);
			nErrorId = ERR_CODE_USER_HAS_NO_RIGHT;
			break;
		}

	} while (0);

	if(nErrorId != ERR_CODE_SUCCESS)
	{
		LOG_PRINT(log_info, "[proc_setuserpriority_req fail]groupid:%u,runnerid:%u,touserid:%u,roleType:%u,errCode:%u.", groupid, run_userid, to_userid, roleType, nErrorId);
		return nErrorId;
	}
	else
	{
		LOG_PRINT(log_info, "[proc_setuserpriority_req]groupid:%u,runnerid:%u,touserid:%u,roleType:%u,isMember:%d.", groupid, run_userid, to_userid, roleType, (int)isMember);

		unsigned int oldRoleType = e_VisitorRole;
		CUserGroupinfo::getUserRoleType(to_userid, groupid, oldRoleType);
		int ret = CUserGroupMgr::modUserRoleType(to_userid, groupid, roleType);
		if (ret < 0)
		{
			LOG_PRINT(log_info, "proc_setuserpriority_req set failed.groupid:%u,runnerid:%u,touserid:%u,roleType:%u,isMember:%d", groupid, run_userid, to_userid, roleType, (int)isMember);
			return ERR_CODE_SET_USER_ROLETYPE;
		}
		else if (ret == 1 && groupid)
		{
			bool bLevelUp = oldRoleType < roleType ? true : false;
			if (!isMember && roleType != e_VisitorRole)
			{
				proc_clean_visitor_user(to_userid, groupid);
				pRoomObjRef->castSendUserPriorityChange(to_userid, roleType);
				proc_joingroup_rpc(to_userid, groupid, roleType);
				castJoinGoupMsg_rpc(groupid, to_userid);
				LOG_PRINT(log_info, "[proc_setuserpriority_req finish]groupid:%u,runnerid:%u,touserid:%u,roleType:%u.", groupid, run_userid, to_userid, roleType);
				return 0;
			}
			else if (isMember && roleType == e_VisitorRole && pRoomObjRef.get() && pToUserRef.get())
			{
				LOG_PRINT(log_info, "user degrade from member to visitor.so need to notify kick out.groupid:%u,runnerid:%u,touserid:%u,roleType:%u,isMember:%d", \
					groupid, run_userid, to_userid, roleType, (int)isMember);
				pRoomObjRef->castSendUserPriorityChange(to_userid, roleType);
				pRoomObjRef->outGroup(to_userid);
				pRoomObjRef->castSendRoomKickoutUserInfo(run_userid, pToUserRef, 0, 0);
				add_group_assistmsg_rpc(to_userid, groupid, roleType, bLevelUp, false, true);
			}
			else
			{
				pRoomObjRef->castSendUserPriorityChange(to_userid, roleType);
				add_group_assistmsg_rpc(to_userid, groupid, roleType, bLevelUp, false, true);
				if (bLevelUp && roleType != e_MemberRole)
				{
					castRoleChange_rpc(groupid, to_userid, oldRoleType, roleType);
				}
			}
			LOG_PRINT(log_info, "[proc_setuserpriority_req finish]groupid:%u,runnerid:%u,touserid:%u,roleType:%u.", groupid, run_userid, to_userid, roleType);
		}
	}	
	return 0;
}

void CLogicCore::proc_getGroupVisitorLst(unsigned int groupid, std::vector<TUserGroupInfo> & oVisitorLst)
{
	LOG_PRINT(log_info, "[getGroupVisitorLst]groupid:%u.", groupid);
	CRoomManager * pRoomMgr = m_pSvrEvent->getRoomMgr();
	if (!pRoomMgr)
	{
		LOG_PRINT(log_error, "[getGroupVisitorLst fail]can not find group error.groupid:%u.", groupid);
		return;
	}

	RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(groupid);
	if(pRoomObjRef.get() == 0) 
	{
		LOG_PRINT(log_error, "[getGroupVisitorLst fail]can not find this room.groupid:%u.", groupid);
		return;
	}

	std::vector<UserObject_Ref> vecUserRef;
	pRoomObjRef->sendVisitorList(vecUserRef);
	LOG_PRINT(log_info, "[getGroupVisitorLst]groupid:%u.visitor list size:%u.", groupid, vecUserRef.size());

	std::vector<UserObject_Ref>::iterator iter_v = vecUserRef.begin();
	for (; iter_v != vecUserRef.end(); ++iter_v)
	{
		if ((*iter_v).get())
		{
			TUserGroupInfo oUserGroupInfo;
			oUserGroupInfo.userID = (*iter_v).get()->nuserid_;
			oUserGroupInfo.groupID = groupid;
			oUserGroupInfo.userAlias = (*iter_v).get()->calias_;
			oUserGroupInfo.userHead = (*iter_v).get()->headpic_;
			oUserGroupInfo.roleType = (*iter_v).get()->nuserlevel_;
			oVisitorLst.push_back(oUserGroupInfo);
		}
	}
	LOG_PRINT(log_info, "[getGroupVisitorLst return]groupid:%u.visitor list size:%u.", groupid, oVisitorLst.size());
}

int CLogicCore::proc_qryGroupVisitCount_req(task_proc_data * message)
{
	CRoomManager * pRoomMgr = m_pSvrEvent->getRoomMgr();
	COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *)message->pdata;
	CMDQryGroupVisitCountReq pReq;
	pReq.ParseFromArray(in_msg->content + SIZE_IVM_CLIENTGATE, SIZE_IVM_REQUEST(in_msg));
	LOG_PRINT(log_info, "[proc_qryGroupVisitCount_req]groupid:%u", pReq.groupid());
	RoomObject_Ref pRoomObjRef;
	pRoomObjRef = pRoomMgr->FindRoom(pReq.groupid());
	if(pRoomObjRef.get() == 0) 
	{
		LOG_PRINT(log_error, "proc_qryGroupVisitCount_req fail.cannot find this group.groupid:%u", pReq.groupid());
		return -1;
	}
	pRoomObjRef->sendGroupVisitCount(message);
	return 0;
}

int CLogicCore::proc_qryJoinGroupConditionReq(task_proc_data * message)
{
	COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *)message->pdata;
	CMDQryJoinGroupCondReq pReq;
	pReq.ParseFromArray(in_msg->content + SIZE_IVM_CLIENTGATE, SIZE_IVM_REQUEST(in_msg));
	RoomObject_Ref pRoomObjRef = m_pSvrEvent->getRoomMgr()->FindRoom(pReq.groupid());
	if (!pRoomObjRef)
	{
		LOG_PRINT(log_error, "proc_qryJoinGroupConditionReq failed, can not find group[%u].", pReq.groupid());
		return -1;
	}
	pRoomObjRef->sendJoinGroupCondition(message, pReq.userid());
	return 0;
}

int CLogicCore::proc_kickAllOnlookers(uint32 runid, uint32 groupid)
{
	RoomObject_Ref pRoomObjRef = m_pSvrEvent->getRoomMgr()->FindRoom(groupid);
	if (!pRoomObjRef)
	{
		LOG_PRINT(log_error, "proc_kickAllOnlookers failed, can not find group[%u].", groupid);
		return ERR_CODE_FAILED_ROOMIDNOTFOUND;
	}

	if (!pRoomObjRef->kickAllLookers())
		return ERR_CODE_FAILED;

	CMDForbidJoinGroupKillVisitor packet;
	packet.set_groupid(groupid);
	int nProtoLen = packet.ByteSize();
	SL_ByteBuffer buff(SIZE_IVM_HEAD_TOTAL + nProtoLen);
	DEF_IVM_HEADER(pHead, buff.buffer());
	CMsgComm::Build_COM_MSG_HEADER(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_ForbidJoinGroupKillVisitor, buff.buffer_size());
	DEF_IVM_CLIENTGATE(pGateMask, buff.buffer());
	CMsgComm::Build_BroadCastRoomRoleType(pGateMask, groupid, e_VisitorRole, e_MemberRole - 1);
	packet.SerializeToArray(buff.buffer() + SIZE_IVM_HEAD_TOTAL, nProtoLen);
	m_pSvrEvent->getRoomMgr()->castGateway(pHead);

	return ERR_CODE_SUCCESS;
}

void CLogicCore::proc_GroupSettingStatReq(task_proc_data * message)
{
	CMDGroupSettingStatReq req;
	if (!CMsgComm::ParseProtoMessage(message->pdata, message->datalen, req))
	{
		LOG_PRINT(log_error, "parse [%s] failed.", req.GetTypeName().c_str());
		return;
	}

	CMDGroupSettingStatNotify notify;
	notify.set_groupid(req.groupid());

	RoomObject_Ref pRoomObjRef;
	if (m_pSvrEvent->getRoomMgr())
	{
		pRoomObjRef = m_pSvrEvent->getRoomMgr()->FindRoom(req.groupid());
		if (pRoomObjRef)
		{
			switch (req.type())
			{
			case e_Setting_All:
			case e_Setting_JoinGroup:
				addSettingStat(notify, e_Setting_JoinGroup, pRoomObjRef->uAllowVisitGroup);
				if (e_Setting_All != req.type()) break;
			case e_Setting_ContributionList:
				addSettingStat(notify, e_Setting_ContributionList, pRoomObjRef->contribution_switch);
				if (e_Setting_All != req.type()) break;
			case e_Setting_VoiceChat:
				addSettingStat(notify, e_Setting_VoiceChat, pRoomObjRef->voice_chat_switch);
				if (e_Setting_All != req.type()) break;
			case e_Setting_AllowVisitorOnMic:
				addSettingStat(notify, e_Setting_AllowVisitorOnMic, pRoomObjRef->allow_visitor_on_mic);
				if (e_Setting_All != req.type()) break;
			default:
				break;
			}
		}
	}
	message->respProtobuf(notify,Sub_Vchat_GroupSettingStatNotify);
}

int CLogicCore::proc_applyJoinGroupAuthReq(task_proc_data * message)
{
	CMDApplyJoinGroupAuthReq reqData;
	if (!CMsgComm::ParseProtoMessage(message->pdata, message->datalen, reqData))
	{
		LOG_PRINT(log_error, "parse [%s] failed.", reqData.GetTypeName().c_str());
		return -1;
	}

	int ret = ERR_CODE_SUCCESS;
	int insertID = 0;
	do 
	{
		CRoomManager * pRoomMgr = m_pSvrEvent->getRoomMgr();
		unsigned int groupID = reqData.groupid();
		unsigned int userID = reqData.userid();
		if (!userID || !groupID || !pRoomMgr)
		{
			LOG_PRINT(log_error, "apply join group auth input error.userID:%u,groupID:%u.", userID, groupID);
			ret = ERR_CODE_INVALID_PARAMETER;
			break;
		}

		RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(groupID);
		if (!pRoomObjRef)
		{
			LOG_PRINT(log_error, "apply join group auth failed,can not find group[%u].", groupID);
			ret = ERR_CODE_FAILED_ROOMIDNOTFOUND;
			break;
		}

		if (pRoomObjRef->findMemberUser(userID))
		{
			LOG_PRINT(log_warning, "user is group member,not need to apply join group auth req.userID:%u,groupID:%u.", userID, groupID);
			ret = ERR_CODE_MEMBER_NOT_NEED_AUTH;
			break;
		}

		unsigned int authType = e_NoNeedAuth;
		int gender = CUserBasicInfo::getUserGender(userID);
		switch(gender)
		{
		case e_MaleGender:
			authType = pRoomObjRef->male_join_group_auth;
			break;
		case e_FemaleGender:
			authType = pRoomObjRef->female_join_group_auth;
			break;
		default:
			LOG_PRINT(log_error, "cannot find this user gender.userID:%u,groupID:%u.", userID, groupID);
			ret = ERR_CODE_FAILED_USERNOTFOUND;
			break;
		}

		if (ret != ERR_CODE_SUCCESS)
		{
			break;
		}

		if (authType == e_NoNeedAuth || authType == e_Forbidden)
		{
			LOG_PRINT(log_warning, "this group not need to apply auth,userID:%u,groupID:%u.", userID, groupID);
			ret = ERR_CODE_JOINGROUP_NOT_NEED_AUTH;
			break;
		}

		if (authType != reqData.authtype())
		{
			LOG_PRINT(log_error, "apply join group auth is not the same.userID:%u,groupID:%u,authType in setting:%u,authType in request:%d.", userID, groupID, authType, (int)reqData.authtype());
			ret = ERR_CODE_JOINGROUP_DIFF_AUTH;
			break;
		}

		TGroupAssistMsg assistMsg;
		assistMsg.groupID = groupID;
		assistMsg.userID = userID;
		switch(reqData.authtype())
		{
		case e_MsgAuth:
			{
				insertID = CDBSink().userApplyJoinGroupAuth(userID, groupID, 3, reqData.msg());
				if (!insertID)
				{
					LOG_PRINT(log_error, "user apply join group auth db fail.auth type:%d.userID:%u,groupID:%u.", (int)reqData.authtype(), userID, groupID);
					ret = ERR_CODE_FAILED_DB;
					break;
				}

				assistMsg.msgType = (int)ASSISTMSG_AGREE;
				assistMsg.svrType = (int)MSGTYPE_MSG_AUTH;
				assistMsg.state = 1;	//need administrator to handle.
				assistMsg.msg = reqData.msg();
				assistMsg.authID = insertID;
				unsigned int groupassist_msgID = add_group_assistmsg_rpc(assistMsg, true, false);
				if (!groupassist_msgID)
				{
					LOG_PRINT(log_error, "user apply join group auth mongodb fail.auth type:%d.userID:%u,groupID:%u.", (int)reqData.authtype(), userID, groupID);
					ret = ERR_CODE_JOINGROUP_APPLY_AUTH;
					break;
				}

				if (!CDBSink().updateJoinGroupAuthMsgID(insertID, groupassist_msgID))
				{
					LOG_PRINT(log_error, "user apply join group auth db fail.auth type:%d.userID:%u,groupID:%u.authID:%u,groupassistMsgID:%u.", \
						(int)reqData.authtype(), userID, groupID, insertID, groupassist_msgID);
					ret = ERR_CODE_FAILED_DB;
					break;
				}
			}
			break;
		default:
			LOG_PRINT(log_warning, "have not handed this auth type:%d.userID:%u,groupID:%u.", (int)reqData.authtype(), userID, groupID);
			break;
		}

	} while (0);

	CMDApplyJoinGroupAuthResp rspData;
	rspData.mutable_errinfo()->set_errid(ret);
	rspData.set_groupid(reqData.groupid());
	rspData.set_userid(reqData.userid());
	message->respProtobuf(rspData, Sub_Vchat_ApplyJoinGroupAuthResp);
	return 0;
}

int CLogicCore::proc_handleJoinGroupAuthReq(task_proc_data * message)
{
	CMDHandleJoinGroupAuthReq reqData;
	if (!CMsgComm::ParseProtoMessage(message->pdata, message->datalen, reqData))
	{
		LOG_PRINT(log_error, "parse [%s] failed.", reqData.GetTypeName().c_str());
		return -1;
	}

	int ret = ERR_CODE_SUCCESS;
	do 
	{
		CRoomManager * pRoomMgr = m_pSvrEvent->getRoomMgr();
		unsigned int run_userID = reqData.run_userid();
		unsigned int authID = reqData.authid();
		bool bAgree = reqData.agree();
		if (!run_userID || !pRoomMgr || !authID)
		{
			LOG_PRINT(log_error, "handle join group auth input error.run userID:%u,authID:%u.", run_userID, authID);
			ret = ERR_CODE_INVALID_PARAMETER;
			break;
		}

		LOG_PRINT(log_info, "handle join group auth input:run userID:%u,authID:%u,agree:%d.", run_userID, authID, (int)bAgree);
		unsigned int grourpAssistID = 0;
		unsigned int apply_userID = 0;
		unsigned int groupID = 0;
		if (!CDBSink().qryJoinGroupAuth(authID, grourpAssistID, apply_userID, groupID) || !grourpAssistID || !apply_userID || !groupID)
		{
			LOG_PRINT(log_error, "cannot find groupAssistMsgID in db.authID:%u,grourpAssistID:%u,apply_userID:%u,groupID:%u.", authID, grourpAssistID, apply_userID, groupID);
			ret = ERR_CODE_HANDLE_JOINGROUP_AUTH;
			break;
		}

		RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(groupID);
		if (!pRoomObjRef)
		{
			LOG_PRINT(log_error, "handle join group auth failed,can not find group[%u].", groupID);
			ret = ERR_CODE_FAILED_ROOMIDNOTFOUND;
			break;
		}

		UserObject_Ref pRunUserRef;
		if (!pRoomObjRef->findMemberUser(pRunUserRef, run_userID))
		{
			LOG_PRINT(log_error, "[handle join group auth fail]run userid is not group member.group:%u,run userID:%u.", groupID, run_userID);
			ret = ERR_CODE_USER_NOT_IN_GROUP;
			break;
		}

		if (pRunUserRef->nuserlevel_ < e_AdminRole)
		{
			LOG_PRINT(log_error, "[handle join group auth fail]run user has no right to approve.group:%u,run userID:%u,run user roleType:%u.", \
				groupID, run_userID, pRunUserRef->nuserlevel_);
			ret = ERR_CODE_USER_HAS_NO_RIGHT;
			break;
		}

		//generate group assist msg
		bool bRPCRet = false;
		CThriftHttpClient<TChatSvrConcurrentClient> client(e_roomadapter_type);
		if (client)
		{
			bRPCRet = client->handle_approveGroupAssistMsg(run_userID, groupID, grourpAssistID, bAgree, apply_userID);
			LOG_PRINT(log_info, "[handle join group auth msg]handle_userid[%d] groupid[%d] msgid[%d] agree[%d] apply userid[%d] ret[%d].", \
				run_userID, groupID, grourpAssistID, (int)bAgree, apply_userID, (int)bRPCRet);
		}
		else
		{
			LOG_PRINT(log_error, "[handle join group auth msg]cannot find chatsvr.handle_userid[%d] groupid[%d] msgid[%d] agree[%d] apply userid[%d].", \
				run_userID, groupID, grourpAssistID, (int)bAgree, apply_userID);
		}

		if (!bRPCRet)
		{
			ret = ERR_CODE_HANDLE_JOINGROUP_AUTH;
			break;
		}

		CDBSink().handleJoinGroupAuth(authID, bAgree);

		if (bAgree && !pRoomObjRef->findMemberUser(apply_userID))
		{
			ret = proc_joingroup_req(apply_userID, groupID, e_MemberRole, false, e_JoinGroupAuthType::MsgAuth);
		}
	}while(0);

	CMDHandleJoinGroupAuthResp rspData;
	rspData.mutable_errinfo()->set_errid(ret);
	rspData.set_run_userid(reqData.run_userid());
	rspData.set_apply_userid(reqData.apply_userid());
	rspData.set_groupid(reqData.groupid());
	rspData.set_authid(reqData.authid());
	rspData.set_agree(reqData.agree());
	message->respProtobuf(rspData, Sub_Vchat_HandleJoinGroupAuthResp);
	return 0;
}

bool CLogicCore::proc_setGroupOwner(unsigned int groupid, unsigned int ownerid)
{
	CRoomManager * pRoomMgr = m_pSvrEvent->getRoomMgr();
	if (!groupid || !ownerid || !pRoomMgr)
	{
		LOG_PRINT(log_error, "set group owner input error.groupid:%u,ownerid:%u.", groupid, ownerid);
		return false;
	}

	LOG_PRINT(log_info, "set group owner input:groupid:%u,owner:%u.", groupid, ownerid);
	RoomObject_Ref pRoomObjRef;
	pRoomObjRef = pRoomMgr->FindRoom(groupid);
	if(pRoomObjRef.get() == 0) 
	{
		LOG_PRINT(log_error, "set group owner fail.cannot find this group.groupid:%u,owner:%u.", groupid, ownerid);
		return false;
	}

	if (pRoomObjRef->group_master == ownerid)
	{
		LOG_PRINT(log_warning, "not need to set group owner.because now owner in redis is the same as input one.groupid:%u,owner:%u.", groupid, ownerid);
		return true;
	}

	bool rollBack = true;
	bool bRet = true;
	CDBSink oSink;
	oSink.transBegin();
	do 
	{
		int oldOwnerID = oSink.updateGroupOwner(groupid, ownerid);
		if (oldOwnerID == -1)
		{
			LOG_PRINT(log_error, "DB update group owner fail.groupid:%u,owner:%u.", groupid, ownerid);
			bRet = false;
			break;
		}

		if (oldOwnerID == ownerid)
		{
			LOG_PRINT(log_warning, "not need to set group owner.because now owner in db is the same as input one.groupid:%u,owner:%u.", groupid, ownerid);
			rollBack = false;
			bRet = true;
			break;
		}
		
		if (oldOwnerID && ERR_CODE_SUCCESS != proc_setuserpriority_req(0, oldOwnerID, groupid, e_AdminRole, false))
		{
			LOG_PRINT(log_error, "update group owner fail.set old group owner level fail.groupid:%u,new owner:%u,old owner:%u.", groupid, ownerid, oldOwnerID);
			bRet = false;
			break;
		}

		if (ERR_CODE_SUCCESS != proc_setuserpriority_req(0, ownerid, groupid, e_OwnerRole, false))
		{
			LOG_PRINT(log_error, "update group owner fail.set new group owner level fail.groupid:%u,new owner:%u.", groupid, ownerid);
			bRet = false;
			break;
		}

		pRoomObjRef->group_master = ownerid;
		if (!pRoomObjRef->redisMsg_RoomMgrInfoUpdate(CFieldName::ROOM_GROUPMASTER))
		{
			LOG_PRINT(log_error, "update group owner fail.update redis fail.groupid:%u,new owner:%u,old owner:%u.", groupid, ownerid, oldOwnerID);
			bRet = false;
			break;
		}

		rollBack = false;
	} while (0);

	if (rollBack)
	{
		oSink.transRollBack();
	}
	else
	{
		oSink.transCommit();
	}

	return bRet;
}

void CLogicCore::addSettingStat(CMDGroupSettingStatNotify &notify, e_SettingType type, int stat)
{
	CMDSettingStat *pStat = notify.add_list();
	pStat->set_type(type);
	pStat->set_stat(stat);
}

int CLogicCore::proc_setroominfo_req(task_proc_data * task_node)
{
	CRoomManager * pRoomMgr = m_pSvrEvent->getRoomMgr();
	COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *)task_node->pdata;
	ClientGateMask_t * pGateMask = (ClientGateMask_t *)(in_msg->content);
	CMDRoomInfoReq pReq;
	pReq.ParseFromArray(in_msg->content + SIZE_IVM_CLIENTGATE, SIZE_IVM_REQUEST(in_msg));

	LOG_PRINT(log_info, "[proc_setroominfo_req]run_userid:%u,groupid:%u", pReq.runnerid(), pReq.vcbid());

	int nErrorId = 0;
	RoomObject_Ref pRoomObjRef;
	do 
	{
		pRoomObjRef = pRoomMgr->FindRoom(pReq.vcbid());
		if(pRoomObjRef.get() == 0) 
		{
			LOG_PRINT(log_error, "proc_setroominfo_req fail.cannot find this group.run_userid:%u,groupid:%u", pReq.runnerid(), pReq.vcbid());
			nErrorId = ERR_CODE_FAILED_ROOMIDNOTFOUND;
			break;
		}

		unsigned int groupID = pReq.vcbid();
		
		UserObject_Ref pUserRef;
		if(!pRoomObjRef->findMemberUser(pUserRef, pReq.runnerid()))
		{
			LOG_PRINT(log_error, "proc_setroominfo_req fail.this user is not group member.run_userid:%u,groupid:%u", pReq.runnerid(), pReq.vcbid());
			nErrorId = ERR_CODE_USER_HAS_NO_RIGHT;
			break;
		}

		if (pUserRef->nuserlevel_ < e_AdminRole)
		{
			LOG_PRINT(log_error, "proc_setroominfo_req fail.this user has no right.run_userid:%u,groupid:%u", pReq.runnerid(), pReq.vcbid());
			nErrorId = ERR_CODE_USER_HAS_NO_RIGHT;
			break;
		}

		//修改房间基本信息
		nErrorId = ERR_CODE_SET_ROOMINFO;
		std::map<std::string, std::string> oFieldValueMap;
		string fields = "";
		if(pReq.has_roomname())
		{
			std::string strGroupName = pReq.mutable_roomname()->data();
			oFieldValueMap["name"] = strGroupName;
			pRoomObjRef->name = strGroupName;
			fields += CFieldName::ROOM_ROOMNAME + " ";
		}

		if(pReq.has_roomremark())
		{
			std::string strRemark = pReq.mutable_roomremark()->data();
			oFieldValueMap["remark"] = strRemark;
			pRoomObjRef->remark = strRemark;
			fields += CFieldName::ROOM_REMARK + " ";
		}

		if (!oFieldValueMap.empty())
		{
			IF_METHOD_FALSE_BREAK_EX(CDBSink().updateGroupBasicInfo_DB(groupID, oFieldValueMap), "update group basic info fail.userid:%u.groupid:%u.", pReq.runnerid(), pReq.vcbid());
		}

		if(pReq.has_roomnotice())
		{
			StRoomNotice_t oBroadInfo;
			oBroadInfo.m_groupID = pReq.vcbid();
			oBroadInfo.m_uBroadUserid = pReq.runnerid();
			oBroadInfo.m_uBroadTime = time(NULL);
			oBroadInfo.m_strRoomNotice = pReq.mutable_roomnotice()->data();
			IF_METHOD_FALSE_BREAK_EX(CDBSink().updateGroupBroadInfo_DB(groupID, oBroadInfo), "update group broad info fail.userid:%u.groupid:%u.", pReq.runnerid(), pReq.vcbid());

			pRoomObjRef->m_roomnotice.m_strRoomNotice = oBroadInfo.m_strRoomNotice;
			pRoomObjRef->m_roomnotice.m_uBroadUserid = oBroadInfo.m_uBroadUserid;
			pRoomObjRef->m_roomnotice.m_uBroadTime = oBroadInfo.m_uBroadTime;
			fields += CFieldName::ROOM_NOTICE + " ";
		}

		if (pRoomObjRef->redisMsg_RoomMgrInfoUpdate(fields))
		{
			nErrorId = 0;
		}
		else
		{
			LOG_PRINT(log_error, "proc_setroominfo_req fail.write redis error.run_userid:%u,groupid:%u", pReq.runnerid(), pReq.vcbid());
		}

	} while (0);

	LOG_PRINT(log_info, "[proc_setroominfo_req result]run_userid:%u,groupid:%u,ret:%d.", pReq.runnerid(), pReq.vcbid(), nErrorId);
	//返回错误响应消息
	CMDRoomInfoResp oRspData;
	oRspData.mutable_errinfo()->set_errid(nErrorId);
	oRspData.set_vcbid(pReq.vcbid());
	oRspData.set_runnerid(pReq.runnerid());

	unsigned int rspDataLen = SIZE_IVM_HEAD_TOTAL + oRspData.ByteSize();
	SL_ByteBuffer buff(rspDataLen);
	buff.data_end(rspDataLen);

	COM_MSG_HEADER * pRspHead = (COM_MSG_HEADER *)buff.buffer();
	ClientGateMask_t * pRspGate = (ClientGateMask_t *)(pRspHead->content);
	GEN_RSP_MSGINFO_SET_HEAD_GATE(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_SetRoomInfoResp, in_msg->reqid, pGateMask, pRspHead, pRspGate, oRspData);
	task_node->connection->write_message(buff);

	if(nErrorId == 0)
	{
		//产生广播信息，通知给所有房间用户
		LOG_PRINT(log_info, "[proc_setroominfo_req success]need to notify group member.run_userid:%u,groupid:%u.", pReq.runnerid(), pReq.vcbid());
		pRoomObjRef->castSendRoomBaseInfo();
	}
	return 0;
}

void CLogicCore::proc_handleGroupUserSettingReq(task_proc_data * message)
{
	CMDGroupUserSettingReq req;
	if (!CMsgComm::ParseProtoMessage(message->pdata, message->datalen, req))
	{
		LOG_PRINT(log_error, "parse [%s] failed.", req.GetTypeName().c_str());
		return;
	}

	uint32 groupid = req.groupid();
	uint32 userid = req.userid();
	LOG_PRINT(log_info, "GroupUserSettingReq begin, group[%u] user[%u].", groupid, userid);
	CMDGroupUserSettingResp resp;
	resp.set_groupid(groupid);
	resp.set_userid(userid);
	
	int ret = ERR_CODE_SUCCESS;
	do
	{
		if (!groupid || !userid)
		{
			ret = ERR_CODE_INVALID_PARAMETER;
			break;
		}

		std::map<std::string, std::string> mValues;
		if (req.has_pchatthreshold())
		{
			int minThreshold = CDBSink().getUserMinGroupPChatThreshold(userid, groupid);
			if (minThreshold < 0)
			{
				ret = ERR_CODE_FAILED_DB;
				break;
			}

			if (req.pchatthreshold().data() < (uint32)minThreshold)
			{
				LOG_PRINT(log_info, "chkModUserGroupPChatThreshold error.");
				ret = ERR_CODE_BELOW_MIN_GROUP_PCHAT_THRESHOLD;
				resp.mutable_errinfo()->add_paramvalue()->assign(bitTostring(minThreshold));
				break;
			}
			mValues[CFieldName::ROOM_USER_PCHAT_INTIMACY] = bitTostring(req.pchatthreshold().data());
			LOG_PRINT(log_info, "change pchat threshold, group[%u] user[%u] new[%u].", groupid, userid, req.pchatthreshold().data());
		}

		if (!mValues.empty())
		{
			if (!CDBSink().updGroupMemberInfo(groupid, userid, mValues))
				ret = ERR_CODE_FAILED_DB;
		}
	} while (0);

	resp.mutable_errinfo()->set_errid(ret);
	message->respProtobuf(resp, Sub_Vchat_GroupUserSettingResp);
	LOG_PRINT(log_info, "GroupUserSettingReq %s, group[%u] user[%u].", CErrMsg::strerror(ret).c_str(), groupid, userid);
}

void CLogicCore::initKV(const char* key)
{
	memset( m_key, 0x00, CryptoPP::AES::DEFAULT_KEYLENGTH );
	memset( m_iv, 0x00, CryptoPP::AES::BLOCKSIZE );
	memcpy(m_key,key,strlen(key));
}
string CLogicCore::encrypt(string plainText)
{
    string cipherText;
    CryptoPP::AES::Encryption aesEncryption(m_key, CryptoPP::AES::DEFAULT_KEYLENGTH);
    CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption( aesEncryption, m_iv );
    CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new CryptoPP::StringSink( cipherText ));
    stfEncryptor.Put( reinterpret_cast<const unsigned char*>( plainText.c_str() ), plainText.length() + 1 );
    stfEncryptor.MessageEnd();

    string cipherTextHex;
    for( int i = 0; i < cipherText.size(); i++ )
    {
        char ch[3] = {0};
        sprintf(ch, "%02x",  static_cast<byte>(cipherText[i]));
        cipherTextHex += ch;
    }

    return cipherTextHex;
}
string CLogicCore::decrypt(string cipherTextHex)
{
    string cipherText;
    string decryptedText;

    int i = 0;
    while(true)
    {
        char c;
        int x;
        stringstream ss;
        ss<<hex<<cipherTextHex.substr(i, 2).c_str();
        ss>>x;
        c = (char)x;
        cipherText += c;
        if(i >= cipherTextHex.length() - 2)break;
        i += 2;
    }

    CryptoPP::AES::Decryption aesDecryption(m_key, CryptoPP::AES::DEFAULT_KEYLENGTH);
    CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption( aesDecryption, m_iv );
    CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::StringSink( decryptedText ));
    stfDecryptor.Put( reinterpret_cast<const unsigned char*>( cipherText.c_str() ), cipherText.size());

    stfDecryptor.MessageEnd();

    return decryptedText;
}

string CLogicCore::getAppIdKey(string appid)
{
	std::map<string,string>::iterator it = m_AppId2Key.find(appid);
	if(it != m_AppId2Key.end())
	{
		return it->second;
	}
	CDBSink().getAppId2Key(m_AppId2Key);
	it = m_AppId2Key.find(appid);
	if(it != m_AppId2Key.end())
	{
		return it->second;
	}
	return "";
}
bool CLogicCore::cast_SendChat_rpc(uint32 groupid, const string& content, e_MsgType msgType, uint32 userid)
{
	bool ret = false;
	if (!groupid)
		return ret;
	CThriftHttpClient<TChatSvrConcurrentClient> client(e_roomadapter_type);
	if (client)
	{
		/*
		新成员入群提示 SEND_COURSE：const int32_t groupid, const std::string& title, const std::string& source, const std::string& summary, const int32_t bTip,
			const int32_t amount, const int32_t courseId
		{"title": 123456, "source": 昵称,"summary": "" ,"bTip":0,"amount":1,"NumId":1}
		*/
		TChatMsg msg;
		msg.msgType = msgType;
		msg.content = content;
		msg.srcUId = userid;
		TChatMsgExtra extra;
		extra.pushOnline = true;
		ret = client->procGroupChatReq(groupid, msg, extra);
		LOG_PRINT(log_info, "[castSendCourse_rpc]groupid[%u] content[%s] ret[%d].", groupid, content.c_str(), ret);
	}
	else
	{
		LOG_PRINT(log_error, "[castSendCourse_rpc]cannot find chatsvr.groupid[%u] content[%s].", groupid, content.c_str());
	}

	return ret;
}
bool CLogicCore::castJoinGoupMsg_rpc(uint32 groupid, uint32 userid)
{
	bool ret = false;
	std::string alias = CUserBasicInfo::getUserAlias(userid);
	if (!groupid || !userid || alias.empty())
		return false;
	CThriftHttpClient<TChatSvrConcurrentClient> client(e_roomadapter_type);
	if (client)
	{
		/*
		新成员入群提示 REMIND_JOINGROUP：
		{"remind_type": 2, "user_id": 123456, "alias": 昵称}
		*/
		TChatMsg msg;
		msg.msgType = MSGTYPE_REMIND;
		Json::Value value;
		value["user_id"] = userid;
		value["alias"] = alias;
		value["remind_type"] = REMIND_JOINGROUP;
		msg.content = Json::FastWriter().write(value);
		TChatMsgExtra extra;
		extra.pushOnline = true;
		extra.memberOnly = true;
		ret = client->procGroupChatReq(groupid, msg, extra);
		LOG_PRINT(log_info, "[castJoinGoupMsg_rpc]groupid[%u] userid[%u] ret[%d].", groupid, userid, ret);
	}
	else
	{
		LOG_PRINT(log_error, "[castJoinGoupMsg_rpc]cannot find chatsvr.groupid[%u] userid[%u].", groupid, userid);
	}

	return ret;
}

bool CLogicCore::castRoleChange_rpc(uint32 groupid, uint32 userid, uint32 oldRoleType, uint32 newRoleType)
{
	bool ret = false;
	std::string alias = CUserBasicInfo::getUserAlias(userid);
	if (!groupid || !userid || alias.empty())
		return ret;
	CThriftHttpClient<TChatSvrConcurrentClient> client(e_roomadapter_type);
	if (client)
	{
		/*
		身份变更提示 REMIND_ROLECHANGE：
		{"remind_type": 3, "user_id": 123456, "alias": 昵称, "old_roletype": 10, "new_roletype": 30}
		*/
		TChatMsg msg;
		msg.msgType = MSGTYPE_REMIND;
		Json::Value value;
		value["user_id"] = userid;
		value["alias"] = alias;
		value["remind_type"] = REMIND_ROLECHANGE;
		value["old_roletype"] = oldRoleType;
		value["new_roletype"] = newRoleType;
		msg.content = Json::FastWriter().write(value);
		TChatMsgExtra extra;
		extra.pushOnline = true;
		extra.memberOnly = true;
		ret = client->procGroupChatReq(groupid, msg, extra);
		LOG_PRINT(log_info, "[castJoinGoupMsg_rpc]groupid[%u] userid[%u] ret[%d].", groupid, userid, ret);
	}
	else
	{
		LOG_PRINT(log_error, "[castJoinGoupMsg_rpc]cannot find chatsvr.groupid[%u] userid[%u].", groupid, userid);
	}

	return ret;
}

int CLogicCore::queryInGroupList(std::map< ::e_INGROUP_TYPE::type, std::vector<int32_t> > & _return, const uint32_t userid, const uint32_t groupid)
{
	LOG_PRINT(log_debug, "queryInGroupList begin: user[%d] group[%d]", userid, groupid);
	if (!userid || !groupid)
	{
		return ERR_CODE_INVALID_PARAMETER;
	}

	RoomObject_Ref pRoomObjRef = m_pSvrEvent->getRoomMgr()->FindRoom(groupid);	
	if (!pRoomObjRef)
	{
		LOG_PRINT(log_warning, "queryInGroupList failed, group[%u] is not exists", groupid);
		return ERR_CODE_FAILED_ROOMIDNOTFOUND;
	}
	std::vector<int32_t> vecList;
	pRoomObjRef->getInGroupList(vecList, false);
	if (!vecList.empty())
	{
		_return[e_INGROUP_TYPE::Member] = vecList;
		LOG_PRINT(log_debug, "group[%u] member in group[%u]", groupid, vecList.size());
		vecList.clear();
	}
	pRoomObjRef->getInGroupList(vecList, true);
	if (!vecList.empty())
	{
		_return[e_INGROUP_TYPE::Visitor] = vecList;
		LOG_PRINT(log_debug, "group[%u] visitor in group[%u]", groupid, vecList.size());
		vecList.clear();
	}

	return ERR_CODE_SUCCESS;
}

void CLogicCore::sendInfoAfterJoinRoom(task_proc_data * message, RoomObject_Ref pRoomObjRef, uint32 userid, bool isMember)
{
	if (!message || !userid || !pRoomObjRef || !pRoomObjRef->nvcbid_)
	{
		LOG_PRINT(log_error, "invalid input info, message: %p, userid:%u, pRoomObjRef:%p", message, userid, pRoomObjRef.get())
		return ;
	}

	//在线成员列表
	pRoomObjRef->sendOnlineMemberList(message);
	pRoomObjRef->sendGroupVisitCount(message);
	if (!isMember)
	{
		pRoomObjRef->sendJoinGroupCondition(message, userid);
	}
}

int CLogicCore::procGiveTip(const int32_t srcid, const int32_t toid, const int32_t groupid, const std::string& gifname, const int32_t tiptype,
	const std::string& srcname, const std::string& toname, const std::string& gifpicture)
{
	int ret = 0;
	CRoomManager* pRoomMgr = m_pSvrEvent->getRoomMgr();
	pRoomMgr->procTip(srcid, toid, groupid, gifname, tiptype, srcname, toname, gifpicture);
	string strHeadAddr = CUserBasicInfo::getUserHeadAddr(srcid);
	//noty
	Json::Value value;
	value["srcuserid"] = srcid;
	value["toid"] = toid;
	value["groupid"] = groupid;
	value["gifname"] = gifname;
	value["tiptype"] = tiptype;
	value["srcname"] = srcname;
	value["toname"] = toname;
	value["gifpicture"] = gifpicture;	
	value["srcuserhead"] = strHeadAddr;
	
	Json::Value lst;
	cast_SendChat_rpc(groupid,Json::FastWriter().write(value),MSGTYPE_TIP);

//	char szBuf[1024] = {0};
//	ClientGateMask_t* pClientGate;
//	CMDGiveTip noty;
//	noty.set_srcid(srcid);
//	noty.set_toid(toid);
//	noty.set_groupid(groupid);
//	noty.set_amount(amount);
//	noty.set_tiptype(tiptype);
//	SERIALIZETOARRAY_BUF(noty,respbuf,len);
//	int nMsgLen= Build_NetMsg(szBuf,1024,MDM_Vchat_Room,Sub_Vchat_TipsNoty,&pClientGate,respbuf,len);
//	if(nMsgLen > 0)
//		pRoomObjRef->castSendMsgXXX((COM_MSG_HEADER*)szBuf);
}
int CLogicCore::procSendLink(const int32_t groupid, const std::string& title, const std::string& source, const std::string& summary, const int32_t bTip,
	const int32_t amount, const int32_t pointId, const std::string& head_add, const int32_t recommendid, const int32_t user_id)
{
	int ret = 0;
	CRoomManager* pRoomMgr = m_pSvrEvent->getRoomMgr();
	RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(groupid);
	if(pRoomObjRef.get())
	{
		Json::Value value;
		value["title"] = title;
		value["source"] = source;
		value["summary"] = summary;
		value["numid"] = pointId;
		value["btip"] = bTip;
		value["amount"] = amount;
		value["head_add"] = head_add;
		value["recommendid"] = recommendid;		
		cast_SendChat_rpc(groupid, Json::FastWriter().write(value), MSGTYPE_LINKS, user_id);
		ret = 1;
		LOG_PRINT(log_info, "procSendLink noty end");
	}
	return ret;
}
int CLogicCore::procNotice(const string& notice)
{
	CRoomManager* pRoomMgr = m_pSvrEvent->getRoomMgr();
	char szBuf[1024] = {0};
	ClientGateMask_t* pClientGate;
	CMDNoticeNoty noty;
	noty.set_notice(notice);
	SERIALIZETOARRAY_BUF(noty,respbuf,len);
	int nMsgLen=pRoomMgr->Build_NetMsg(szBuf,1024,MDM_Vchat_Room,Sub_Vchat_noticeNoty,&pClientGate,respbuf,len);
	CMsgComm::Build_BroadCastOnLine_Gate(pClientGate, e_Notice_AllType);
	pRoomMgr->castGateway((COM_MSG_HEADER *)szBuf);
	LOG_PRINT(log_info, "proc_send_notice noty end");
	return 1;
}
int CLogicCore::procSendCourse(const int32_t groupid, const std::string& title, const std::string& source, const std::string& summary, const int32_t bTip,
	const int32_t amount, const int32_t courseId, const std::string& head_add, const std::string& teachername, const std::string& srcimg, const int32_t recommendid, const int32_t user_id)
{
	int ret = 0;
	CRoomManager* pRoomMgr = m_pSvrEvent->getRoomMgr();
	RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(groupid);
	if(pRoomObjRef.get())
	{

		Json::Value value;
		value["title"] = title;
		value["source"] = source;
		value["summary"] = summary;
		value["numId"] = courseId;
		value["btip"] = bTip;
		value["amount"] = amount;
		value["head_add"] = head_add;
		value["teachername"] = teachername;
		value["srcimg"] = srcimg;
		value["recommendid"] = recommendid;		
		cast_SendChat_rpc(groupid, Json::FastWriter().write(value), MSGTYPE_STICKER, user_id);

		ret = 1;
		LOG_PRINT(log_info, "proc_send_course noty end");
	}
	return ret;
}

int CLogicCore::proc_reloadRoomVirtualNumInfo(const int32_t liveid, const int32_t numbers)
{
	int ret = 0;
	CRoomManager* pRoomMgr = m_pSvrEvent->getRoomMgr();
	RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(liveid,false);
	if (pRoomObjRef.get())
	{
		pRoomObjRef->virtual_num = numbers;
		pRoomObjRef->redisInitRoomInfo();
	}
}
int32_t CLogicCore::proc_CloseLiveAndCourse(const int32_t runuserid, const int32_t liveid, const int32_t status, const int32_t type)
{
	if (liveid <= 0)
		return -1;
	 
	int32_t touser_id = 0;
	CMDCloseLiveAndCourseeNoty noty;
	int32_t realliveid = liveid;
	if (type == 1 || type == 3)//直播间或者单个课程
	{
		if (type == 1)
			realliveid += BROADCAST_ROOM_BEGIN_NUM;
		CDBSink().get_UseridByCourseID(touser_id, liveid, type);
		noty.set_groupid(liveid);
		noty.set_toid(touser_id);
	}
	else//禁用所有课程
	{
		std::vector<uint32> vList;
		CDBSink().get_CourseByUserid(liveid, vList);
		if (vList.size() > 0)
		{
			noty.set_groupid(vList[0]);
			noty.set_toid(liveid);
			realliveid = vList[0];			
		}
			
	}
	CRoomManager* pRoomMgr = m_pSvrEvent->getRoomMgr();
	RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(realliveid, false);
	if (pRoomObjRef.get())
	{
		noty.set_runuserid(runuserid);
		noty.set_status(status);
		noty.set_errid(0);
		noty.set_type(type);		

		ClientGateMask_t* pClientGate;
		char szBuf[512] = { 0 };

		SERIALIZETOARRAY_BUF(noty, respbuf, len);
		int nMsgLen = pRoomMgr->Build_NetMsg(szBuf, 512, MDM_Vchat_Room, Sub_Vchat_CloseLiveAndCourseNoty, &pClientGate, respbuf, len);
		if (type == 3)
		{
			pRoomObjRef->castSendMsgXXX((COM_MSG_HEADER*)szBuf);

		}			
		else
		{
			CMsgComm::Build_BroadCastUser_Gate(pClientGate, (type == 1) ? touser_id : liveid);
			pRoomMgr->castGateway((COM_MSG_HEADER *)szBuf);
		}
			
	}
		 
	 
	return 0;
}
	

int32_t CLogicCore::proc_liveStart(const std::string& streamID)
{
	string sGroupid,sUid;
	uint32 groupId=0,uid=0;
	CMDLiveStartNoty notify;
	
	int pos=streamID.find("_");
	if (pos)
	{
		sUid=streamID.substr(0,pos);

		stringstream ss;
		ss<<sUid;
		ss>>uid;

		sGroupid=streamID.substr(pos+1);
		LOG_PRINT(log_info, "sGroupid=%s,sUid=%s",sGroupid.c_str(),sUid.c_str());
		stringstream ss2;
		//ss.str(""); todo 复用该对象需要清空,但解释字段有问题
		ss2<<sGroupid;
		ss2>>groupId;

	}
	else
	{
		LOG_PRINT(log_error, "can't find _ flag");
		return -1;
	}

	LOG_PRINT(log_info, "groupid=%d,uid=%d",groupId,uid);

	if (groupId<=0 || uid<=0)
	{
		return -1;
	}
	CRoomManager* pRoomMgr = m_pSvrEvent->getRoomMgr();
	RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(groupId, false);
	if (pRoomObjRef.get())
	{

		string liveUrl="http://pull.dks.dacelue.com.cn/live/"+streamID+"/playlist.m3u8";

		CDBTools::insertLiveState(groupId,uid,1,liveUrl);//state 1 为开播

		CDBTools::insertLiveFlow(groupId,uid,1,liveUrl);
		//todo: should set the state to  redis,so next time get from redis
		notify.set_groupid(groupId);
		notify.set_uid(uid);
		notify.set_liveurl(liveUrl);

		int nLength = notify.ByteSize() + SIZE_IVM_HEAD_TOTAL;

		SL_ByteBuffer buff(nLength);
		buff.data_end(nLength);
		ClientGateMask_t * pGateMask = (ClientGateMask_t *)CMsgComm::Build_COM_MSG_HEADER(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_LiveStart, nLength);
		CMsgComm::Build_BroadCastRoomGate(pGateMask, e_Notice_AllType, groupId);
		notify.SerializeToArray(buff.buffer() + SIZE_IVM_HEAD_TOTAL, notify.ByteSize());	
		pRoomMgr->castGateway((COM_MSG_HEADER *)buff.buffer());

	}


	return 0;
}

int32_t CLogicCore::proc_liveStop(const std::string& streamID)
{
	string sGroupid,sUid;
	uint32 groupId,uid;
	CMDLiveStartNoty notify;

	int pos=streamID.find("_");
	if (pos)
	{
		sUid=streamID.substr(0,pos);

		stringstream ss;
		ss<<sUid;
		ss>>uid;

		sGroupid=streamID.substr(pos+1);
		LOG_PRINT(log_info, "sGroupid=%s,sUid=%s",sGroupid.c_str(),sUid.c_str());
		stringstream ss2;
		//ss.str(""); 
		ss2<<sGroupid;
		ss2>>groupId;
	}
	else
	{
		LOG_PRINT(log_error, "can't find _ flag");
		return -1;
	}

	LOG_PRINT(log_info, "groupid=%d,uid=%d",groupId,uid);
	if (groupId<=0 || uid<=0)
	{
		return -1;
	}
	CRoomManager* pRoomMgr = m_pSvrEvent->getRoomMgr();
	RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(groupId, false);
	if (pRoomObjRef.get())
	{
		string liveUrl="http://pull.dks.dacelue.com.cn/live/"+streamID+"/playlist.m3u8";

		CDBTools::insertLiveState(groupId,uid,2,liveUrl);//state 2 为停播
		CDBTools::insertLiveFlow(groupId,uid,2,liveUrl);
		notify.set_groupid(groupId);
		notify.set_uid(uid);
		notify.set_liveurl(liveUrl);

		int nLength = notify.ByteSize() + SIZE_IVM_HEAD_TOTAL;

		SL_ByteBuffer buff(nLength);
		buff.data_end(nLength);
		ClientGateMask_t * pGateMask = (ClientGateMask_t *)CMsgComm::Build_COM_MSG_HEADER(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_LiveStop, nLength);
		CMsgComm::Build_BroadCastRoomGate(pGateMask, e_Notice_AllType, groupId);
		notify.SerializeToArray(buff.buffer() + SIZE_IVM_HEAD_TOTAL, notify.ByteSize());	
		pRoomMgr->castGateway((COM_MSG_HEADER *)buff.buffer());

	}

	return 0;
}

int32_t CLogicCore::proc_sendNewSubCourseNotice(const int32_t courseId, const int32_t subCourseId, const int32_t userId, const std::string& courseName, const std::string& subCourseName, const std::string& beginTime)
{
	CRoomManager* pRoomMgr = m_pSvrEvent->getRoomMgr();
	RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(subCourseId);
	if(!pRoomObjRef.get())
	{
		LOG_PRINT(log_error, "subcourse not found, subCourseId:%d", subCourseId);
		return -1;
	}

	int unusedCourseType = 0;
	uint32 parentCourseId = courseId;

	if (0 == parentCourseId
		&& CDBSink().queryCourseTypeAndParentId(subCourseId, unusedCourseType, parentCourseId) < 0)
	{
		LOG_PRINT(log_error, "failed to query course type and parent course id, courseId = %d", subCourseId);
		return -1;
	}

	std::set<uint32> studentUserIds;

	if (CDBSink().querySerialCourseStudents(parentCourseId, studentUserIds) <= 0 || 0 == studentUserIds.size())
	{
		LOG_PRINT(log_warning, "failed to query students info, or no students found,"
			" subcourse id = %u, parent course id = %u", subCourseId, parentCourseId);
		return 0;
	}
	LOG_PRINT(log_debug, "parent course id: %u, student count: %u", parentCourseId, studentUserIds.size());

	CMDNewSubCourseNoty appNotice;
	uint32 courseIdFromDb = 0;
	uint32 userIdFromDb = 0;
	std::string courseNameFromDb;
	std::string subCourseNameFromDb;
	std::string beginTimeFromDb;

	// 系列课及子课信息直接从数据库查询，PHP只确保传子课ID，其余信息不保证，但thrift接口维持原样。
	if (CDBSink().querySubCourseInfo(subCourseId, courseIdFromDb, userIdFromDb, subCourseNameFromDb, courseNameFromDb, beginTimeFromDb) < 0)
	{
		LOG_PRINT(log_error, "querySubCourseInfo() failed, subCourseId:%d", subCourseId);
		return -1;
	}

	appNotice.set_courseid(courseIdFromDb);
	appNotice.set_subcourseid(subCourseId);
	appNotice.set_userid(userIdFromDb);
	appNotice.set_coursename(courseNameFromDb);
	appNotice.set_subcoursename(subCourseNameFromDb);
	appNotice.set_begintime(beginTimeFromDb);
	SERIALIZETOARRAY_BUF(appNotice,respbuf,len);

	char msgBuf[8192] = {0};
	ClientGateMask_t* pClientGate;

	pRoomMgr->Build_NetMsg(msgBuf,sizeof(msgBuf),MDM_Vchat_Room,Sub_Vchat_NewSubCourseNoty,&pClientGate,respbuf,len);

	LOG_PRINT(log_debug, "Online notice to APP: courseId[%u], subCourseId[%u], teacher userId[%u], courseName[%s], subCourseName[%s], beginTime[%s]",
		appNotice.courseid(), appNotice.subcourseid(), appNotice.userid(), appNotice.coursename().c_str(), appNotice.subcoursename().c_str(), appNotice.begintime().c_str());

	for (std::set<uint32>::iterator it = studentUserIds.begin(); it != studentUserIds.end(); ++it)
	{
		LOG_PRINT(log_debug, "Sending to uid[%u] ...", *it);
		CMsgComm::Build_BroadCastUser_Gate(pClientGate, *it);
		pRoomMgr->castGateway((COM_MSG_HEADER *)msgBuf);
	}

	return 0;
}

int32_t CLogicCore::proc_sendNewCourseNotice(int32_t courseId)
{
	CRoomManager* pRoomMgr = m_pSvrEvent->getRoomMgr();
	RoomObject_Ref pRoomObjRef = pRoomMgr->FindRoom(courseId);
	if(!pRoomObjRef.get())
	{
		LOG_PRINT(log_error, "course not found, courseId:%d", courseId);
		return -1;
	}

	std::set<uint32> userIds;

	if (CDBSink().queryFocusUsersByCourseId(courseId, userIds) <= 0 || 0 == userIds.size())
	{
		LOG_PRINT(log_warning, "failed to query focus user info, or no focus users found, course id = %u", courseId);
		return 0;
	}
	LOG_PRINT(log_debug, "course id: %u, focus user count: %u", courseId, userIds.size());

	CMDNewCourseNoty appNotice;
	uint32 userId = 0;
	uint32 liveId = 0;
	std::string courseName;
	std::string teacherAlias;

	if (CDBSink().queryCourseInfo(courseId, userId, liveId, courseName, teacherAlias) < 0)
	{
		LOG_PRINT(log_error, "queryCourseInfo() failed, courseId:%d", courseId);
		return -1;
	}

	appNotice.set_courseid(courseId);
	appNotice.set_liveid(liveId);
	appNotice.set_userid(userId);
	appNotice.set_coursename(courseName);
	appNotice.set_useralias(teacherAlias);
	SERIALIZETOARRAY_BUF(appNotice,respbuf,len);

	char msgBuf[8192] = {0};
	ClientGateMask_t* pClientGate;

	pRoomMgr->Build_NetMsg(msgBuf,sizeof(msgBuf),MDM_Vchat_Room,Sub_Vchat_NewCourseNoty,&pClientGate,respbuf,len);

	LOG_PRINT(log_debug, "Online notice to APP: courseId[%u], teacher userId[%u], liveId[%u], courseName[%s], userAlias[%s]",
		appNotice.courseid(), appNotice.userid(), appNotice.liveid(), appNotice.coursename().c_str(), appNotice.useralias().c_str());

	for (std::set<uint32>::iterator it = userIds.begin(); it != userIds.end(); ++it)
	{
		LOG_PRINT(log_debug, "Sending to uid[%u] ...", *it);
		CMsgComm::Build_BroadCastUser_Gate(pClientGate, *it);
		pRoomMgr->castGateway((COM_MSG_HEADER *)msgBuf);
	}

	return 0;
}

