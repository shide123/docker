/*
 * CSvrEvent.cpp
 *
 *  Created on: Aug 22, 2016
 *      Author: testtest
 */

#include "UsermgrEvent.h"
#include "redis_def.h"
#include "msgcommapi.h"
#include "SL_ByteBuffer.h"
#include "Common.pb.h"
#include "UserMgrSvr.pb.h"
#include "macro_define.h"
#include "errcode.h"
#include "CUserBasicInfo.h"
#include "CUserGroupinfo.h"
#include "GroupBasicInfo.h"

UsermgrEvent::UsermgrEvent()
{
	m_pRedisMgr = NULL;
	m_redisThreadNum = 1;
	m_cmdrange_lst.clear();
	m_cmdlist = "";
	m_workid = 0;
}

UsermgrEvent::~UsermgrEvent()
{
	DELETE_POINT(m_pRedisMgr);
}

bool UsermgrEvent::onConnect(clienthandler_ptr connection)
{
	if (connection)
	{
		LOG_PRINT(log_info, "recv new connection %s:%d connid:%u", connection->getremote_ip(), connection->getremote_port(), connection->getconnid());
	}
	return true;
}

bool UsermgrEvent::onMessage(task_proc_data * message)
{
	COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *) message->pdata;
	//LOG_PRINT(log_debug, "onMessage message msgtye:%d,len:%d,main:%d:sub:%d\n", message->msgtye, message->datalen, in_msg->maincmd, in_msg->subcmd);

	if (Sub_Vchat_ClientHello == in_msg->subcmd)
	{
		handle_hello_msg(message);
		return 0;
	}

	if (Sub_Vchat_ClientPing == in_msg->subcmd)
	{
		//ping msg from gate
		handle_ping_msg(message);
		return 0;
	}

	switch(in_msg->subcmd)
	{
	//gateway
	case Sub_Vchat_LogonNot:
		{
//			if (0 == m_workid)
			{
				proc_userlogin_notify(message);
			}
			break;
		}
	case Sub_Vchat_LogoutNot:
		{
//			if (0 == m_workid)
			{
				proc_userlogout_notify(message);
			}
			break;
		}
	case Sub_Vchat_MicStateTransNoty:
		{
			proc_micstate_transfer(message);
			break;
		}
	case Sub_Vchat_UserModInfoReq:
		{
			handle_usermodinfo_req(message);
			break;
		}
	case Sub_Vchat_OperFriendshipReq:
		{
			handle_operFriendshipReq(message);
			break;
		}
	case Sub_Vchat_QryUserAttentionList:
		{
			handle_qryUserAttentionList(message);
			break;
		}
	case Sub_Vchat_UserLocationInfo:
		{
			handele_userLocationInfo(message);
			break;
		}
	default:
        LOG_PRINT(log_warning, "have not handled such sub cmd!maincmd:%u,subcmd:%u.",in_msg->maincmd, in_msg->subcmd);
		break;
	}

	return true;
}

bool UsermgrEvent::onClose(clienthandler_ptr connection)
{
	if (connection.get())
	{
		LOG_PRINT(log_info, "connection onClose, %s:%d connid:%u.", connection->getremote_ip(), connection->getremote_port(), connection->getconnid());
		if (connection->user_data)
		{
			delete connection->user_data;
			connection->user_data = NULL;
		}
		
		if (m_clientConnMgr.checkClientExist(connection->getremote_ip(), connection->getremote_port()))
		{
			LOG_PRINT(log_warning, "Gate[%s:%u]disconnect.Clean user info in memory and redis.", connection->getremote_ip(), connection->getremote_port());
//			if (0 == m_workid)
			{
				m_usermgr.delAllUser(connection->getremote_ip(), connection->getremote_port());
			}
			m_clientConnMgr.delClientConn(connection->getremote_ip(), connection->getremote_port());
		}
	}
	return true;
}
bool UsermgrEvent::onError(clienthandler_ptr connection, int code, std::string msg)
{
	if (code == boost::asio::error::eof)
	{
		//notify_svr_clientexit();
	}
	else
	{
		//notify_svr_exceptexit();
	}

	return true;
}

bool UsermgrEvent::onTimer()
{
	return true;
}

bool UsermgrEvent::loadConfig(Config & config)
{
	//redis config
	m_strRedisHost = config.getString("redis", "ip");
	m_nRedisPort = config.getInt("redis", "port");
	m_sRedisPass = config.getString("redis", "password");
    int redisThreadNum = config.getInt("redis", "threadnum");
    if (redisThreadNum > 0)
    	m_redisThreadNum = redisThreadNum;
    LOG_PRINT(log_debug, "redis config:%s:%d %s %d", m_strRedisHost.c_str(), m_nRedisPort, m_sRedisPass.c_str(), m_redisThreadNum);

	return true;
}

bool UsermgrEvent::init(Config & config)
{
	//init redis
	m_pRedisMgr = new redisMgr(m_strRedisHost.c_str(), m_nRedisPort, m_sRedisPass.c_str(), m_redisThreadNum);

	m_usermgr.init(m_pRedisMgr);
	CUserBasicInfo::init(m_pRedisMgr);
	CUserGroupinfo::init(m_pRedisMgr);
	CGroupBasicInfo::init(m_pRedisMgr);

//	if (!m_workid)
//	{
//		//clean user online info in redis
//		m_usermgr.resetRedisDB();
//	}

	//init cmdlist to register
	m_cmdlist = config.getString("cmd", "cmdlist");

	m_cmdrange_lst.clear();
	int i = 1;
	while (1)
	{
		char cmdrange[LEN128] = {0};
		sprintf(cmdrange, "cmdrange_%d", i);
		std::string strcmdrange = config.getString("cmd", cmdrange);
		if (strcmdrange.empty())
		{
			break;
		}

		m_cmdrange_lst.push_back(std::string(cmdrange) + "=" + strcmdrange);
		++i;
	}

    return true;
}

void UsermgrEvent::setWorkID(unsigned int workid)
{
	m_workid = workid;
}

void UsermgrEvent::proc_micstate_transfer(task_proc_data * message)
{
	if (!message || !message->connection)
	{
		LOG_PRINT(log_error, "message or connection is null.");
		return;
	}

	COM_MSG_HEADER * inMsg = (COM_MSG_HEADER *) message->pdata;
	unsigned int dataLen = inMsg->length - SIZE_IVM_HEADER;
	CMDTransferMicState oReq;
	oReq.ParseFromArray(inMsg->content, dataLen);
	LOG_PRINT(log_info, "mic state transfer input:userid:%u,mic runid:%d,mic toid:%d,mic groupid:%d,mic index:%d,mic state:%d,invite mic runid:%d,invite mic toid:%d,invite mic session:%d.", \
		oReq.userid(), oReq.micrunuserid(), oReq.mictouserid(), oReq.micgroupid(), oReq.micindex(), oReq.micstate(), oReq.invitemicrunid(), oReq.invitemictoid(), oReq.invitemicsessionid());

	CConnInfo connObj;
	CUserinfo userObj;
	int ret = m_usermgr.getUserOnlineInfo(oReq.userid(), connObj, userObj);
	if (ret <= 0)
	{
		LOG_PRINT(log_warning, "cannot find this userid:%u online.", oReq.userid());
		return;
	}

	LOG_PRINT(log_info, "[mic state transfer]user(%u,%d,%u),connid:%u).gateway:%s,%u.", \
		userObj.getUserid(), (int)userObj.getDevType(), (unsigned int)userObj.getLoginTime(), userObj.getConnID(), connObj.getIpaddress().c_str(), connObj.getPort());

	clienthandler_ptr notyClient;
	int iRet = m_clientConnMgr.getClientConn(connObj, notyClient);
	if (0 == iRet && notyClient)
	{
		unsigned int dataLen = SIZE_IVM_HEAD_TOTAL + oReq.ByteSize();
		SL_ByteBuffer outbuf(dataLen);
		outbuf.data_end(dataLen);

		COM_MSG_HEADER * pReq = (COM_MSG_HEADER *) outbuf.buffer();
		memcpy(pReq, inMsg, SIZE_IVM_HEADER);
		pReq->length = dataLen;

		ClientGateMask_t * pGateMask = (ClientGateMask_t *) pReq->content;
		memset(pGateMask, 0, SIZE_IVM_CLIENTGATE);
		pGateMask->param2 = userObj.getConnID();

		oReq.SerializeToArray(pReq->content + SIZE_IVM_CLIENTGATE, oReq.ByteSize());
		notyClient->write_message(outbuf, true);
	}
}

int UsermgrEvent::proc_userlogin_notify(task_proc_data * message)
{
	if (!message || !message->connection)
	{
		LOG_PRINT(log_error, "message or connection is null.");
		return 0;
	}

	COM_MSG_HEADER * inMsg = (COM_MSG_HEADER *) message->pdata;
	unsigned int dataLen = inMsg->length - SIZE_IVM_HEADER;
	CMDLogonClientInfLst infoLst;
	infoLst.ParseFromArray(inMsg->content, dataLen);
	unsigned int userNum = infoLst.userinflst_size();

	std::string strIP = message->connection->getremote_ip();
	unsigned int iPort = message->connection->getremote_port();
	int gateid = message->connection->getgateid();
	LOG_PRINT(log_info, "There are %u users login.(%s,%u)", userNum, strIP.c_str(), iPort);

	for (int i = 0; i < userNum; ++i)
	{
		//user online info
		CMDLogonClientInf * pData = infoLst.mutable_userinflst(i);
		uint32 userid = pData->userid();
		time_t logintime = (time_t) (pData->logontime());
		DEVTYPE termtype = (DEVTYPE) (pData->mobile());
		uint32 connID = pData->connid();
		std::string uuid = pData->uuid();
		CUserinfo userObj(userid, termtype, logintime, connID);

		//user connection info
		CConnInfo connObj(strIP, iPort, gateid);

		char sztime[128] = { 0 };
		toStringTimestamp3(logintime, sztime);
		
		CConnInfo kickOutConn;
		CUserinfo kickOutUser;
		LOGIN_RESULT loginRet = m_usermgr.userLogin(connObj, userObj, kickOutConn, kickOutUser);
		if (eDefault == loginRet || eFail == loginRet)
		{
			//Do nothing.
		}
		else if (eFirstLogin == loginRet)
		{
			LOG_PRINT(log_info, "user(%u,%d,%s(%u),connid:%u,uuid:%s,gateid:%d) first login.(gateway:%s,%u)", \
				userid, (int)termtype, sztime, (unsigned int)logintime, connID, uuid.c_str(), gateid, strIP.c_str(), iPort);
		}
		else
		{
			LOG_PRINT(log_info, "user(%u,%d,%s(%u),connid:%u,uuid:%s) login again.(gateway:%s,%u)", \
				userid, (int)termtype, sztime, (unsigned int)logintime, connID, uuid.c_str(), strIP.c_str(), iPort);

			unsigned int kickuserid = kickOutUser.getUserid();
			DEVTYPE kicktype = kickOutUser.getDevType();
			time_t kicklogintime = kickOutUser.getLoginTime();
			unsigned int kickConnID = kickOutUser.getConnID();

			char kicktime[128] = { 0 };
			toStringTimestamp3(kicklogintime, kicktime);

			std::string kickIP = kickOutConn.getIpaddress();
			unsigned int kickPort = kickOutConn.getPort();

			LOG_PRINT(log_warning, "kicked out old user(%u,%d,%s(%u),connid:%u).gateway:%s,%u.", \
				kickuserid, (int)kicktype, kicktime, (unsigned int)kicklogintime, kickConnID, kickIP.c_str(), kickPort);

			clienthandler_ptr kickClient;
			//int iRet = m_clientConnMgr.getClientConn(kickOutConn, kickClient);
			int iRet = m_clientConnMgr.getClientConnByGateid(kickOutConn, kickClient);	//get by gateid
			if (0 == iRet && kickClient)
			{
				//kick user
				CMDUserKickOffLine rspData;
				rspData.set_reasonid(ERR_CODE_SAMEUSER_LOGIN_OTHER_PLACE);
				rspData.set_newsessionuuid(uuid);
				unsigned notyLen = rspData.ByteSize();

				SL_ByteBuffer outbuf(SIZE_IVM_HEAD_TOTAL + notyLen);
				COM_MSG_HEADER * KickReq = (COM_MSG_HEADER *) outbuf.buffer();
				KickReq->version = MDM_Version_Value;
				KickReq->checkcode = CHECKCODE;
				KickReq->maincmd = MDM_Vchat_Usermgr;
				KickReq->subcmd = Sub_Vchat_ClientExistNot;

				ClientGateMask_t * pGateMask = (ClientGateMask_t *) KickReq->content;
				memset(pGateMask, 0, SIZE_IVM_CLIENTGATE);
				pGateMask->param2 = kickConnID;

				char * pRsp = (char *)(KickReq->content + SIZE_IVM_CLIENTGATE);
				rspData.SerializeToArray(pRsp, notyLen);

				KickReq->length = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + notyLen;
				outbuf.data_end(KickReq->length);

				kickClient->write_message(outbuf, true);
			}
			else
			{
				LOG_PRINT(log_error, "Do not find client to kick out old user(%u,%d,%s(%u),connid:%u.gateway:%s,%u.", \
					kickuserid, (int)kicktype, kicktime, (unsigned int)kicklogintime, kickConnID, kickIP.c_str(), kickPort);
			}
		}

		//post user group id list
		proc_user_relategrouplst(userid, connID, connObj);

		//post user sysparam config
		proc_user_sysparam_config(userid, termtype, connID, connObj);
	}

	return 0;
}

void UsermgrEvent::proc_user_sysparam_config(unsigned int userid, DEVTYPE termtype, unsigned int conn_id, const CConnInfo & gateObj)
{
	std::list<SysConfig_t > sysconfigLst;
	if (!CDBSink().getNotifySysConfig_DB(userid, (int)termtype, sysconfigLst))
	{
		return;
	}

	Json::Value jsonValue;
	std::list<SysConfig_t >::iterator iter_lst = sysconfigLst.begin();
	for (; iter_lst != sysconfigLst.end(); ++iter_lst)
	{
		jsonValue[iter_lst->paramName] = Json::Value(iter_lst->paramValue);
	}

	Json::FastWriter writer;
	std::string jsonoutput = writer.write(jsonValue);
	LOG_PRINT(log_debug, "sysconfig list:%s.", jsonoutput.c_str());

	CMDUserSysConfigNotify sysConfigNoty;
	sysConfigNoty.set_sysconfig(jsonoutput);

	unsigned rspLen = sysConfigNoty.ByteSize();
	SL_ByteBuffer outbuf(SIZE_IVM_HEAD_TOTAL + rspLen);
	COM_MSG_HEADER * pRsp = (COM_MSG_HEADER *) outbuf.buffer();
	pRsp->version = MDM_Version_Value;
	pRsp->checkcode = CHECKCODE;
	pRsp->maincmd = MDM_Vchat_Usermgr;
	pRsp->subcmd = Sub_Vchat_UserSysConfigNotify;
	pRsp->length = SIZE_IVM_HEAD_TOTAL + rspLen;

	ClientGateMask_t * pGateMask = (ClientGateMask_t *) pRsp->content;
	memset(pGateMask, 0, SIZE_IVM_CLIENTGATE);
	pGateMask->param2 = conn_id;

	char * pRspData = (char *)(pRsp->content + SIZE_IVM_CLIENTGATE);
	sysConfigNoty.SerializeToArray(pRspData, rspLen);

	outbuf.data_end(pRsp->length);

	clienthandler_ptr gateClient;
	int iRet = m_clientConnMgr.getClientConn(gateObj, gateClient);
	if (0 == iRet && gateClient)
	{
		gateClient->write_message(outbuf, true);
	}
}

void UsermgrEvent::proc_user_relategrouplst(unsigned int userid, unsigned int conn_id, const CConnInfo & gateObj)
{
	if (!userid)
	{
		LOG_PRINT(log_error, "userid is wrong.");
		return;
	}

	std::list<stGroupInfo> grouplst;
	CDBSink().getUserRelateGroupLst_DB(userid, grouplst);
	if (grouplst.empty())
	{
//		LOG_PRINT(log_warning, "userid:%u has no group list.", userid);
		return;
	}

	CMDUserGroupInfo rspGroupLst;
	std::list<stGroupInfo>::iterator iter = grouplst.begin();
	for (; iter != grouplst.end(); ++iter)
	{
		CMDGroupInfo * pGroupInfo = rspGroupLst.add_grouplst();
		pGroupInfo->set_groupid(iter->groupid);
		pGroupInfo->set_grouphead(iter->headaddr);
		pGroupInfo->set_groupname(iter->name);
		pGroupInfo->set_showid(iter->showid);
		pGroupInfo->set_muteset(iter->muteSet);
		pGroupInfo->set_roletype(iter->roleType);
		pGroupInfo->set_membercount(iter->memberCount);
		pGroupInfo->set_visitcount(iter->visitCount);
	}

	unsigned rspLen = rspGroupLst.ByteSize();
	SL_ByteBuffer outbuf(SIZE_IVM_HEAD_TOTAL + rspLen);
	COM_MSG_HEADER * pRsp = (COM_MSG_HEADER *) outbuf.buffer();
	pRsp->version = MDM_Version_Value;
	pRsp->checkcode = CHECKCODE;
	pRsp->maincmd = MDM_Vchat_Usermgr;
	pRsp->subcmd = Sub_Vchat_UserGroupLstResp;
	pRsp->length = SIZE_IVM_HEAD_TOTAL + rspLen;

	ClientGateMask_t * pGateMask = (ClientGateMask_t *) pRsp->content;
	memset(pGateMask, 0, SIZE_IVM_CLIENTGATE);
	pGateMask->param2 = conn_id;

	char * pRspData = (char *)(pRsp->content + SIZE_IVM_CLIENTGATE);
	rspGroupLst.SerializeToArray(pRspData, rspLen);

	outbuf.data_end(pRsp->length);

	clienthandler_ptr gateClient;
	int iRet = m_clientConnMgr.getClientConn(gateObj, gateClient);
	if (0 == iRet && gateClient)
	{
		gateClient->write_message(outbuf, true);
		LOG_PRINT(log_info, "user[%u] group size[%u]", userid, rspGroupLst.grouplst_size());
	}

	//group setting
	iter = grouplst.begin();
	for (; iter != grouplst.end(); ++iter)
	{
		notifyRelateGroupSetting(iter->groupid, conn_id, gateObj);
	}
}

void UsermgrEvent::notifyRelateGroupSetting(unsigned int groupID, unsigned int conn_id, const CConnInfo & gateObj)
{
	if (!groupID)
	{
		return;
	}

	CMDGroupSettingStatNotify oSetNotify;
	oSetNotify.set_groupid(groupID);

	std::set<CGroupBasicInfo::e_FieldType > fieldSet;
	fieldSet.insert(CGroupBasicInfo::e_Field_Alow_Visit_Group);
	fieldSet.insert(CGroupBasicInfo::e_Field_Contribution_Switch);
	fieldSet.insert(CGroupBasicInfo::e_Field_Voice_Chat_Switch);
	fieldSet.insert(CGroupBasicInfo::e_Field_Allow_Visitor_On_Mic);

	std::map<CGroupBasicInfo::e_FieldType, int > valueMap;
	if (!CGroupBasicInfo::getValue(groupID, fieldSet, valueMap) || valueMap.empty())
	{
		LOG_PRINT(log_warning, "groupid:%u has no setting to notify.", groupID);
		return;
	}

	std::map<CGroupBasicInfo::e_FieldType, int >::iterator iter_map = valueMap.begin();
	for (; iter_map != valueMap.end(); ++iter_map)
	{
		int allow_state = 0;
		switch(iter_map->first)
		{
		case CGroupBasicInfo::e_Field_Alow_Visit_Group:
			{
				allow_state = iter_map->second;
				CMDSettingStat * pSetState = oSetNotify.add_list();
				pSetState->set_type(e_Setting_JoinGroup);
				pSetState->set_stat(allow_state);
				LOG_PRINT(log_debug, "group:%u allow visit group setting state:%d.", groupID, allow_state);
			}
			break;
		case CGroupBasicInfo::e_Field_Contribution_Switch:
			{
				allow_state = iter_map->second;
				CMDSettingStat * pSetState = oSetNotify.add_list();
				pSetState->set_type(e_Setting_ContributionList);
				pSetState->set_stat(allow_state);
				LOG_PRINT(log_debug, "group:%u Contribution setting state:%d.", groupID, allow_state);
			}
			break;
		case CGroupBasicInfo::e_Field_Allow_Visitor_On_Mic:
			{
				allow_state = iter_map->second;
				CMDSettingStat * pSetState = oSetNotify.add_list();
				pSetState->set_type(e_Setting_AllowVisitorOnMic);
				pSetState->set_stat(allow_state);
				LOG_PRINT(log_debug, "group:%u AllowVisitorOnMic setting state:%d.", groupID, allow_state);
			}
			break;
		default:
			break;
		}
	}

	unsigned rspLen = SIZE_IVM_HEAD_TOTAL + oSetNotify.ByteSize();
	SL_ByteBuffer outbuf(rspLen);
	outbuf.data_end(rspLen);

	CMsgComm::Build_COM_MSG_HEADER(outbuf.buffer(), MDM_Vchat_Room, Sub_Vchat_GroupSettingStatNotify, rspLen);
	DEF_IVM_CLIENTGATE(pGateMask, outbuf.buffer());
	memset(pGateMask, 0, SIZE_IVM_CLIENTGATE);
	pGateMask->param2 = conn_id;
	oSetNotify.SerializeToArray(outbuf.buffer() + SIZE_IVM_HEAD_TOTAL, oSetNotify.ByteSize());

	clienthandler_ptr gateClient;
	int iRet = m_clientConnMgr.getClientConn(gateObj, gateClient);
	if (0 == iRet && gateClient)
	{
		gateClient->write_message(outbuf, true);
	}
}

int UsermgrEvent::proc_userlogout_notify(task_proc_data * message)
{
	if (!message || !message->connection)
	{
		LOG_PRINT(log_error, "ERROR:message or connection is null.");
		return 0;
	}

	COM_MSG_HEADER * inMsg = (COM_MSG_HEADER *) message->pdata;
	unsigned int dataLen = inMsg->length - SIZE_IVM_HEADER;
	CMDLogonClientInfLst infLst;
	infLst.ParseFromArray(inMsg->content, dataLen);
	unsigned int userNum = infLst.userinflst_size();

	std::string strIP = message->connection->getremote_ip();
	unsigned int iPort = message->connection->getremote_port();
	int gateid = message->connection->getgateid();

	for (int i = 0; i < userNum; ++i)
	{
		CMDLogonClientInf * logoutUser = infLst.mutable_userinflst(i);
		unsigned int userid = logoutUser->userid();
		DEVTYPE type = (DEVTYPE)(logoutUser->mobile());
		time_t logintime = (time_t)(logoutUser->logontime());
		uint32 connID = logoutUser->connid();

		char sztime[128] = { 0 };
		toStringTimestamp3(logintime, sztime);

		LOG_PRINT(log_info, "There are user(%u,%d,%s(%u),connid:%u.gateway:%s,%u,%d) logout.", \
			userid, (int)type, sztime, (unsigned int)logintime, connID, strIP.c_str(), iPort, gateid);

		CConnInfo connInfo(strIP, iPort, gateid);
		CUserinfo userObj(userid, type, logintime, connID);
		m_usermgr.delUser(connInfo, userObj);
	}
    return 0;
}

/*
3月21日-4月20日 白羊座
4月21日-5月20日 金牛座
5月21日-6月21日 双子座
6月22日-7月22日 巨蟹座
7月23日-8月22日 狮子座
8月23日-9月22日 处女座
9月23日-10月22日 天秤座
10月23日-11月21日 天蝎座
11月22日-12月21日 射手座
12月22日-1月19日 摩羯座
1月20日-2月18日 水瓶座
2月19日-3月20日 双鱼座
*/
string m_StarSignArray[12][2] = {  
	{"摩羯座", "水瓶座"}, 
	{"水瓶座", "双鱼座"}, 
	{"双鱼座", "白羊座"},  
	{"白羊座", "金牛座"}, 
	{"金牛座", "双子座"}, 
	{"双子座", "巨蟹座"}, 
	{"巨蟹座", "狮子座"}, 
	{"狮子座", "处女座"}, 
	{"处女座", "天秤座"}, 
	{"天秤座", "天蝎座"}, 
	{"天蝎座", "射手座"}, 
	{"射手座", "摩羯座"} 
}; 

std::string UsermgrEvent::changeBirth2StarSign(const std::string & strBirth)
{
	std::string ret = "";
	if (strBirth.empty())
	{
		LOG_PRINT(log_error, "changeBirth2StarSign input error.");
		return ret;
	}

	unsigned int year = 0;
	unsigned int month = 0;
	unsigned int day = 0;
	if (-1 == sscanf(strBirth.c_str(), "%u-%u-%u", &year, &month, &day))
	{
		LOG_PRINT(log_error, "birthday format input error.%s.", strBirth.c_str());
		return ret;
	}

	if (month <= 12 && day <= 31)
	{
		ret = m_StarSignArray[month - 1][day / 21];
	}
	return ret;
}

int UsermgrEvent::handle_usermodinfo_req(task_proc_data * message)
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
		LOG_PRINT(log_error, "handle_usermodinfo_req packet is error!,length=%d,required at least:%u", in_msg->length, SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE);
		message->connection->resperrinf(in_msg, pGateMask, ERR_CODE_FAILED_PACKAGEERROR);
		return -1;
	}

	char * pReqData = (char *)(in_msg->content + SIZE_IVM_CLIENTGATE);
	unsigned int reqLen = SIZE_IVM_REQUEST(in_msg);
	CMDUserModInfoReq reqData;
	reqData.ParseFromArray(pReqData, reqLen);
	LOG_PRINT(log_info, "[user modify request]userid:%u.", reqData.userid());

	int ret = 0;
	std::set<unsigned int> groupIDSet;
	do 
	{
		std::map<std::string, std::string> field_value_map;
		if (reqData.has_useralias())
		{
			std::string strAlias = reqData.useralias().data();
			strAlias = trim(strAlias);
			if (strAlias.empty())
			{
				LOG_PRINT(log_error, "[user modify fail]userid:%u.request alias:%s,after trim:%s.", reqData.userid(), reqData.useralias().data().c_str(), strAlias.c_str());
				ret = -1;
				break;
			}

			reqData.mutable_useralias()->set_data(strAlias);
			field_value_map[CFieldName::USER_ALIAS] = strAlias;
		}

		if (reqData.has_headadd())
		{
			field_value_map[CFieldName::USER_HEAD] = reqData.headadd().data();
		}

		if (reqData.has_birthday())
		{
			std::string data = reqData.birthday().data();		
			field_value_map["birthday"] = data;
			std::string strStarSign = changeBirth2StarSign(data);
			if (!strStarSign.empty())
			{
				field_value_map["constellation"] = strStarSign;
			}
		}

		if (reqData.has_sign())
		{
			field_value_map["sign"] = reqData.sign().data();
		}

		if (reqData.has_city())
		{
			field_value_map["city"] = reqData.city().data();
		}

		ret = m_usermgr.modUserInfo(reqData.userid(), field_value_map, groupIDSet);
	} while (0);

	CMDUserModInfoResp rspData;
	rspData.set_userid(reqData.userid());
	unsigned errid = (ret < 0? ERR_CODE_USER_MODINFO_ERR : 0);
	LOG_PRINT(log_info, "user modify info,userid:%u,errid:%u.", reqData.userid(), errid);
	rspData.mutable_errinfo()->set_errid(errid);

	unsigned int rspLen = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + rspData.ByteSize();
	SL_ByteBuffer buff(rspLen);
	buff.data_end(rspLen);

	COM_MSG_HEADER * pRspHead = (COM_MSG_HEADER *)buff.buffer();
	pRspHead->version = MDM_Version_Value;
	pRspHead->checkcode = CHECKCODE;
	pRspHead->maincmd = MDM_Vchat_Usermgr;
	pRspHead->subcmd = Sub_Vchat_UserModInfoResp;
	pRspHead->reqid = in_msg->reqid;
	pRspHead->length = rspLen;

	ClientGateMask_t * pRspGate = (ClientGateMask_t *)(pRspHead->content);
	memcpy(pRspGate, pGateMask, SIZE_IVM_CLIENTGATE);

	char * pRsp = (char *)(pRspHead->content + SIZE_IVM_CLIENTGATE);
	rspData.SerializeToArray(pRsp, rspData.ByteSize());
	message->connection->write_message(buff);

	//Broadcast in groups
	if (ret == 0)
	{
		bool noty = false;
		CMDUserInfoModNotify oNoty;
		oNoty.set_userid(reqData.userid());
		if (reqData.has_useralias())
		{
			CMDString * userAlias = oNoty.mutable_useralias();
			userAlias->set_data(reqData.useralias().data());
			noty = true;
		}
		if (reqData.has_headadd())
		{
			CMDString * headAddr = oNoty.mutable_headadd();
			headAddr->set_data(reqData.headadd().data());
			noty = true;
		}

		if (noty && !groupIDSet.empty())
		{
			unsigned int notyLen = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + oNoty.ByteSize();
			SL_ByteBuffer oNotybuff(notyLen);
			oNotybuff.data_end(notyLen);

			COM_MSG_HEADER * pNotyHead = (COM_MSG_HEADER *)oNotybuff.buffer();
			pNotyHead->version = MDM_Version_Value;
			pNotyHead->checkcode = CHECKCODE;
			pNotyHead->maincmd = MDM_Vchat_Usermgr;
			pNotyHead->subcmd = Sub_Vchat_UserModInfoNotify;
			pNotyHead->length = notyLen;

			char * pNoty = (char *)(pNotyHead->content + SIZE_IVM_CLIENTGATE);
			oNoty.SerializeToArray(pNoty, oNoty.ByteSize());

			std::set<unsigned int>::iterator iter_set = groupIDSet.begin();
			for (; iter_set != groupIDSet.end(); ++iter_set)
			{
				ClientGateMask_t * pNotyGate = (ClientGateMask_t *)(pNotyHead->content);
				CMsgComm::Build_BroadCastRoomGate(pNotyGate, e_Notice_AllType, *iter_set);
				m_clientConnMgr.castAllClientConn(oNotybuff.buffer(), oNotybuff.data_size());
			}
		}
	}

	return 0;
}

void UsermgrEvent::handle_operFriendshipReq(task_proc_data * message)
{
	CMDOperFriendshipReq req;
	if (!CMsgComm::ParseProtoMessage(message->pdata, message->datalen, req))
	{
		LOG_PRINT(log_error, "parse %s error", req.GetTypeName().c_str());
		return;
	}

	const string &actionName = e_Friendship_OpType_Name(req.action());
	LOG_PRINT(log_info, "OperFriendshipReq begin, userid[%u] dstuid[%u] action[%d:%s]", req.userid(), req.dstuid(), req.action(), actionName.c_str());
	if (!req.userid() || !req.dstuid())
	{
		message->resperrinf(ERR_CODE_INVALID_PARAMETER); 
		return;
	}

	int relation = CDBSink().getUserFriendship(req.userid(), req.dstuid());
	bool update = ((req.action() == OPTYPE_FOLLOW || req.action() == OPTYPE_BLOCK) ||	//增加关系（关注/拉黑）
		((req.action() == OPTYPE_UNFOLLOW && relation == OPTYPE_FOLLOW) ||				//取消关注
		 (req.action() == OPTYPE_UNBLOCK && relation == OPTYPE_BLOCK)));				//移除黑名单
	if (update)
	{
		CDBSink().modUserFriendship(req.userid(), req.dstuid(), req.action());
	}
	else
	{
		LOG_PRINT(log_warning, "old relation[%d:%s] not match new relation[%d:%s]", relation, e_Friendship_OpType_Name((e_Friendship_OpType)relation).c_str(), req.action(), actionName.c_str());
	}
	
	CMDOperFriendshipResp resp;
	resp.mutable_errinfo()->set_errid(ERR_CODE_SUCCESS);
	resp.set_dstuid(req.dstuid());
	resp.set_action(req.action());
	message->respProtobuf(resp, Sub_Vchat_OperFriendshipResp);
	LOG_PRINT(log_info, "OperFriendshipReq success, userid[%u] dstuid[%u] action[%d:%s]", req.userid(), req.dstuid(), req.action(), actionName.c_str());
}

void UsermgrEvent::handle_qryUserAttentionList(task_proc_data * message)
{
	CMDQryUserAttentionList req;
	if (!CMsgComm::ParseProtoMessage(message->pdata, message->datalen, req))
	{
		LOG_PRINT(log_error, "parse %s error", req.GetTypeName().c_str());
		return;
	}
	
	LOG_PRINT(log_info, "QryUserAttentionList begin, userid[%u]", req.userid());
	if (!req.userid())
	{
		message->resperrinf(ERR_CODE_INVALID_PARAMETER); 
		return;
	}

	CMDUserAttentionList resp;
	resp.set_userid(req.userid());
	std::vector<uint32> vList;
	if (CDBSink().getUserFriendList(req.userid(), vList, OPTYPE_FOLLOW) > 0)
	{
		for (int i = 0; i < vList.size(); i++)
		{
			resp.add_list(vList[i]);
		}
	}

	message->respProtobuf(resp, Sub_Vchat_UserAttentionList);
	LOG_PRINT(log_info, "QryUserAttentionList end, userid[%u] following size[%u]", req.userid(), resp.list_size());
}

void UsermgrEvent::handle_ping_msg(task_proc_data * message)
{
	if (message && message->connection && message->pdata)
	{
		COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *)message->pdata;

		SL_ByteBuffer outbuf(SIZE_IVM_HEADER + sizeof(CMDClientPingResp_t));

		COM_MSG_HEADER * pmsgheader = (COM_MSG_HEADER *)outbuf.buffer();
		memcpy(pmsgheader, in_msg, SIZE_IVM_HEADER);
		pmsgheader->subcmd = Sub_Vchat_ClientPingResp;

		CMDClientPingResp_t * rsp = (CMDClientPingResp_t *)(pmsgheader->content);
		CMDClientPing_t * req = (CMDClientPing_t *)in_msg->content;
		rsp->userid = req->userid;
		rsp->roomid = req->roomid;

		pmsgheader->length = SIZE_IVM_HEADER + sizeof(CMDClientPingResp_t);
		outbuf.data_end(pmsgheader->length);
		message->connection->write_message(outbuf);
	}
}

void UsermgrEvent::handle_hello_msg(task_proc_data * message)
{
	if (!message || !message->connection || !message->pdata)
	{
		LOG_PRINT(log_error, "handle hello msg input error.");
		return;
	}

	message->connection->bSayHello = true;
	COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *) message->pdata;
	if (in_msg->length == sizeof(COM_MSG_HEADER) + sizeof(CMDGateHello_t))
	{
		CMDGateHello_t * pHelloMsg = (CMDGateHello_t *) in_msg->content;
		if (pHelloMsg->param1 == 12 && pHelloMsg->param2 == 8 && pHelloMsg->param3 == 7)
		{
			switch (pHelloMsg->param4)
			{
			case e_gateway_type:
				{
					uint32 connid = message->connection->getconnid();
					//非gateway该值置零
					int gateid = pHelloMsg->gateid;
					if (gateid)
					{
						message->connection->setgateid(gateid);
						m_clientConnMgr.addClientConn(message->connection, message->connection->getremote_ip(), message->connection->getremote_port());
						LOG_PRINT(log_info, "recv gateway hello %s:%d connect,conn_id=%u, gateid=%u", message->connection->getremote_ip(), \
							message->connection->getremote_port(), connid, gateid);
					}

					if (!m_workid)
					{
						register_cmd_msg(message->connection);
					}

					break;
				}
			default:
				LOG_PRINT(log_error, "hello msg from unknown server type:%d,%s:%d connect.", (int)pHelloMsg->param4, \
					message->connection->getremote_ip(), message->connection->getremote_port());
				break;
			}
		}
	}
	else
	{
		LOG_PRINT(log_error, "handle hello msg input msg length error.");
	}
	return;
}

void UsermgrEvent::register_cmd_msg(clienthandler_ptr client)
{
	if (m_cmdrange_lst.empty() && m_cmdlist.empty())
	{
		return;
	}

	char szBuf[512] = {0};
	int msglen = CMsgComm::Build_RegisterCmd_Msg(szBuf, sizeof(szBuf), e_usermgrsvr_type, m_cmdlist, m_cmdrange_lst);
	if (msglen > 0 && client)
	{
		client->write_message(szBuf, msglen);
	}
}
void UsermgrEvent::handele_userLocationInfo(task_proc_data * message)
{
	CMDUserLocateInfo req;
	if (!CMsgComm::ParseProtoMessage(message->pdata, message->datalen, req))
	{
		LOG_PRINT(log_error, "parse %s error", req.GetTypeName().c_str());
		return;
	}

	LOG_PRINT(log_info, "QryUserAttentionList begin, userid[%u]", req.userid());
	if (!req.userid())
	{
		message->resperrinf(ERR_CODE_INVALID_PARAMETER);
		return;
	}

	string hash = m_geohash.getGeohash(req.longitude(),req.latitude());
	int row = 0;
	CDBSink().build_sql_run(0,&row,"select userid from mc_location where userid = %d",req.userid());
	LOG_PRINT(log_info, "usermgr_tag hash:%s,,row:%d,req.longitude():%lf,req.latitude():%lf",hash.c_str(),row,req.longitude(),req.latitude());
	if(row == 0)
	{
		CDBSink().build_sql_run(0,0,"insert into mc_location(userid,geohash,longitude,latitude) values(%d,'%s',%lf,%lf)"
				,req.userid(),hash.c_str(),req.longitude(),req.latitude());
	}else
	{
		CDBSink().build_sql_run(0,0,"update mc_location set geohash = '%s',longitude = %lf,latitude = %lf where userid = %d"
				,hash.c_str(),req.userid(),req.longitude(),req.latitude());
	}
	CMDUserLocationResp resp;
	resp.set_userid(req.userid());
	m_geohash.geohash_search(hash);
	CMDUserLocateInfo* locateInfo = resp.mutable_nearlist()->Add();
	locateInfo->set_userid(125365);
	locateInfo->set_longitude(36.2561452);
	locateInfo->set_latitude(102.3654854);

	message->respProtobuf(resp, Sub_Vchat_UserLocationInfoResp);

}
