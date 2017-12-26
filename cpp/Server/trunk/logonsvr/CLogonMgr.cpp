/*
 * CLogonMgr.cpp
 *
 *  Created on: Mar 30, 2016
 *      Author: root
 */

#include "CLogonMgr.h"
#include "CLogThread.h"
#include "db/pool.h"
#include "db/optable2.h"
#include "db/table2.h"
#include "message_comm.h"
#include "UserManager.h"
#include "GlobalSetting.h"
#include "md5.h"
#include "HttpClient.h"
#include "errcode.h"
#include "DBConnection.h"
#include "tcpserver.h"
#include "msgcommapi.h"
#include "errcode.h"
#include "DBSink.h"
#include "DBTools.h"
#include <boost/locale.hpp>
#include <boost/regex.hpp>
#include "Common.pb.h"
#include "LogonSvr.pb.h"

CLogonMgr::CLogonMgr()
{
}

CLogonMgr::~CLogonMgr()
{
}

typedef enum enum_plateformtype
{
	QQ_TYPE		= 1,
	WEIBO_TYPE	= 2,
	WECHAT_TYPE	= 3,
	SMS_TYPE	= 4,
}PLATEFORM_TYPE;

/*
 * return logon error code
 * 101: in black list
 * 102:
 * 103: no this loginID
 * 104: password error
 * 105: version too low
 * 106: login id format error
 * 107: account freezed
 * 108: token expire time
 * 109: cannot find userID using loginID and patternLock
 * */
int CLogonMgr::proc_logonreq5(task_proc_data * task_node)
{
    COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *)task_node->pdata;
	ClientGateMask_t * pGateMask = (ClientGateMask_t *)(in_msg->content);
	if(in_msg->length <= SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE)
	{
		LOG_PRINT(log_error, "error:login-req5 packet is error!,length=%d,required at least:%u", in_msg->length, SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE);
		if (task_node->connection)
		{
			task_node->connection->resperrinf(in_msg, pGateMask, ERR_CODE_FAILED_PACKAGEERROR);
		}
		return -1;
	}

	char * pReqData = (char *)(in_msg->content + SIZE_IVM_CLIENTGATE);
	unsigned int reqLen = SIZE_IVM_REQUEST(in_msg);
	CMDUserLogonReq reqData;
	reqData.ParseFromArray(pReqData, reqLen);

	in_addr inaddr;
	inaddr.s_addr = htonl(pGateMask->param5);
	std::string client_ip = "";
	char * ipaddr = inet_ntoa(inaddr);
	if (ipaddr)
	{
		client_ip = ipaddr;
	}

    LOG_PRINT(log_info, "enter loginid[%u] token[%s] platformType[%u][ip=%s][mac=%s][serial=%s][version=%u]",
        reqData.loginid(), reqData.token().c_str(), reqData.platformtype(), client_ip.c_str(), reqData.mac().c_str(), reqData.serial().c_str(), reqData.version());

    int nRet = -1;

    //非游客处理
    int nErrId = 0;
    unsigned int nLoginID = reqData.loginid();

	//判断PC版本登录
    if(reqData.mobile() == e_PC_devtype && CGlobalSetting::m_app->m_limitversion && reqData.version() < CGlobalSetting::m_app->m_limitversion)
    {
        char szBuf[256] = {0};
        int nSendLen = BuildUserLogonErrPack(ERR_CODE_LOGIN_VERSION_2_LOW, szBuf, pGateMask, sizeof(szBuf), in_msg->reqid);
        task_node->connection->write_message(szBuf, nSendLen);
        LOG_PRINT(log_error, "loginid[%u] version[%u] rejected logon!limit_version in config:%u.", nLoginID, reqData.version(), CGlobalSetting::m_app->m_limitversion);
        return 0;
    }

	do 
	{
		CDBSink *pSink = NULL;
		if (!reqData.hostdb().empty())
		{
			pSink = CDBSink::getSinkByHost(reqData.hostdb());
			if (NULL == pSink)
			{
				LOG_PRINT(log_error, "input host[%s] can not reach the db sink.", reqData.hostdb().c_str());
			}
		}

		if (pSink)
		{
			nRet = pSink->check_user_token_DB(nLoginID, reqData.token(), (reqData.platformtype() != SMS_TYPE ? false: true));
			delete pSink;
		}

		if (nRet)
		{
			nRet = 0;// CDBTools::check_user_token_DB(nLoginID, reqData.token(), (reqData.platformtype() != SMS_TYPE ? false: true));
		}
		nRet = 0;
		if (reqData.platformtype() != SMS_TYPE)
		{
			//get info from other platform login table
			if (nRet)
			{
				nErrId = ERR_CODE_LOGIN_ID_NOT_EXSIT;
				break;
			}
		}
		else
		{
			//sms use token.
			if (-1 == nRet)
			{
				nErrId = ERR_CODE_LOGIN_ID_NOT_EXSIT;
				break;
			}
			else if (-2 == nRet)
			{
				nErrId = ERR_CODE_LOGIN_TOKEN_EXPIRE_TIME;
				break;
			}
		}

	} while (0);

    if(nRet != 0)
    {
        LOG_PRINT(log_error, "LogonErr loginID[%u] nErrId[%d]!", nLoginID, nErrId);
        char szBuf[256] = {0};
        int nSendLen = BuildUserLogonErrPack(nErrId, szBuf, pGateMask, sizeof(szBuf), in_msg->reqid);
        task_node->connection->write_message(szBuf, nSendLen);
        return 0;
    }

    //准备封杀数据
	DDViolationInfo_t ddVioInfo;
    memset(&ddVioInfo, 0, sizeof(DDViolationInfo_t));

    //检查是否在黑名单
    /*
	nRet = 0;
    LOG_PRINT(log_info, "query black-user:[user=%d],[ip=%s],[mac=%s],[Serial=%s].", nUserId, pReq->cIpAddr, pReq->cMacAddr, pReq->cSerial);
	nRet = CDBTools::isin_blackIpMac_DB(nUserId, 0, FT_SCOPE_GLOBAL, pReq->cIpAddr, pReq->cMacAddr, pReq->cSerial, &ddVioInfo);
	if(nRet == 0)
	{
		//在黑名单，返回错误
		char szBuf[256] = {0};
		int nSendLen = BuildUserLogonErrPack(nUserId, ERR_CODE_LOGIN_BLACKLIST, ddVioInfo.reasontype, ddVioInfo.lefttime/60, szBuf, pGateMask, 256, pReq->nmessageid);
		task_node->connection->write_message(szBuf, nSendLen);
		LOG_PRINT(log_error, "visitor[%d] is limited,rejected logon !", nUserId);
		return 0;
	}
	*/

    //取出用户信息
    StUserFullInfo_t userinfo;
	nRet = CDBTools::get_user_fullinfoByLoginID_DB(userinfo, nLoginID, reqData.patternlock());
    if(nRet == 0)
    {
    	LOG_PRINT(log_info, "get_user_fullinfoByLoginID_DB id[%d] alias:%s.", userinfo.userid, userinfo.strAlias.c_str());
		//do nothing.
    }
    else
    {
        nErrId = ERR_CODE_LOGIN_NOT_FOUND_USERID;
    }

    //登录信息失败
    if(nErrId != 0)
    {
        LOG_PRINT(log_error, "LogonErr loginID[%d] ErrId[%d]!", nLoginID, nErrId);
        char szBuf[256] = {0};
        int nSendLen = BuildUserLogonErrPack(nErrId, szBuf, pGateMask, sizeof(szBuf), in_msg->reqid);
        task_node->connection->write_message(szBuf, nSendLen);
        return 0;
    }

	//账号被冻结
    if(1 == userinfo.nfreeze)
	{
        LOG_PRINT(log_error, "loginID[%d] is freezed!", nLoginID);
        char szBuf[256] = {0};
        int nSendLen = BuildUserLogonErrPack(ERR_CODE_LOGIN_ID_FREEZED, szBuf, pGateMask, sizeof(szBuf), in_msg->reqid);
        task_node->connection->write_message(szBuf, nSendLen);
        return 0;
    }

	//返回登录成功
	char buf[256] = {0};
	COM_MSG_HEADER * pOutMsg = (COM_MSG_HEADER *)buf;
    pOutMsg->version = MDM_Version_Value;
    pOutMsg->checkcode = CHECKCODE;
    pOutMsg->maincmd = MDM_Vchat_Login;
    pOutMsg->subcmd = Sub_Vchat_logonSuccess2;
	pOutMsg->reqid = in_msg->reqid;

    ClientGateMask_t* pMask = (ClientGateMask_t*)(pOutMsg->content);
	memcpy(pMask, pGateMask, sizeof(ClientGateMask_t));

	CMDUserLogonSuccessResp rspData;
	rspData.set_userid(userinfo.userid);
	rspData.set_headadd(userinfo.strHead);
	rspData.set_gender(userinfo.gender);
	rspData.set_useralias(userinfo.strAlias);
	if (reqData.platformtype() != SMS_TYPE)
	{
		rspData.set_loginplateform(1);
	}
	else
	{
		rspData.set_loginplateform(0);
	}
	rspData.set_tel(userinfo.strTel);

	//获取有效token
	std::string strSessionToken = "";
	std::string strValidTime = "";
	char szTokenBuf[512] = {0};
	int nTokenSendLen = CUserManager::BuildUserTokenPack(userinfo.userid, szTokenBuf, 0, pGateMask, sizeof(szTokenBuf), strSessionToken, strValidTime);
	if(nTokenSendLen < 0)
	{
		LOG_PRINT(log_error, "BuildUserTokenPack error[%d] userid:%d.", nTokenSendLen, userinfo.userid);
	}
	rspData.set_sessiontoken(strSessionToken);
	rspData.set_validtime(strValidTime);
	rspData.set_servarea(CGlobalSetting::m_app->m_hostArea);
	
	char * pRspData = (char *)(pOutMsg->content + SIZE_IVM_CLIENTGATE);
	unsigned int rspLen = rspData.ByteSize();
	rspData.SerializeToArray(pRspData, rspLen);

    pOutMsg->length = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + rspLen;
    task_node->connection->write_message(buf, pOutMsg->length);
	LOG_PRINT(log_info, "loginid:%u user:%u has login!", nLoginID, userinfo.userid);

	if(nTokenSendLen > 0)
	{
		cast_user_sendmsg(szTokenBuf, nTokenSendLen, userinfo.userid);
	}

 	//登陆记录
    CMDUserLoginLog_t logonlog;
    memset(&logonlog, 0, sizeof(CMDUserLoginLog_t));
    logonlog.userid = userinfo.userid;
    logonlog.devtype = reqData.mobile();
    strcpy(logonlog.szip, client_ip.c_str());
    strcpy(logonlog.szmac, reqData.mac().c_str());
    strcpy(logonlog.szserial, reqData.serial().c_str());
	strcpy(logonlog.devicemodel, reqData.devicemodel().c_str());
	strcpy(logonlog.deviceos, reqData.deviceos().c_str());
	logonlog.time = time(NULL);
	logonlog.loginid = reqData.loginid();
	CDBTools::write_logonrecord(logonlog);
    return 0;
}

int CLogonMgr::proc_synusertoken_req(task_proc_data * task_node)
{
    LOG_PRINT(log_info, "enter %s!", __FUNCTION__);
    COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *)task_node->pdata;
    ClientGateMask_t * pGateMask = (ClientGateMask_t *)(in_msg->content);
    char * pReq = (char *)(in_msg->content + SIZE_IVM_CLIENTGATE);
    unsigned int reqLen = SIZE_IVM_REQUEST(in_msg);

	CMDSessionTokenReq tokenReq;
	tokenReq.ParseFromArray(pReq, reqLen);

    if(in_msg->length <= SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE)
    {
        LOG_PRINT(log_warning, "recv length error!,length:%d,required at least:%u", in_msg->length, SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE);
		if (task_node->connection)
		{
			task_node->connection->resperrinf(in_msg, pGateMask, ERR_CODE_FAILED_PACKAGEERROR);
		}
		return 0;
    }

    LOG_PRINT(log_info, "proc_synusertoken_req recv :[%d]", tokenReq.userid());

	std::string strSessionToken;
	std::string strValidTime;
    char szTokenBuf[512] = {0};
    int nSendLen = CUserManager::BuildUserTokenPack(tokenReq.userid(), szTokenBuf, in_msg, pGateMask, 512, strSessionToken, strValidTime);

    if(nSendLen > 0)
    {
        if (task_node->connection)
        {
            task_node->connection->write_message(szTokenBuf, nSendLen);
        }
		cast_user_sendmsg(szTokenBuf, nSendLen, tokenReq.userid());
    }
    else
    {
        LOG_PRINT(log_error, "BuildUserTokenPack error, SendLen: %d.", nSendLen);
	    if (task_node->connection)
	    {
			task_node->connection->resperrinf(in_msg, pGateMask, ERR_CODE_FAILED_DBERROR);
	    }
    }
    LOG_PRINT(log_info, "end.");

    return 0;
}

int CLogonMgr::proc_client_closeSocket_req(task_proc_data * task_node)
{
	LOG_PRINT(log_info, "[proc_client_closeSocket_req]enter!");
	COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *)task_node->pdata;
	CMDUserOnlineBaseInfoNoty_t * pReq = (CMDUserOnlineBaseInfoNoty_t *)(in_msg->content + SIZE_IVM_CLIENTGATE);
	//CGlobalSetting::m_app->m_user_mgr.ResetVistorInfo(pReq->uuid, pReq->userid);
	LOG_PRINT(log_info, "[proc_client_closeSocket_req end]userid %d", pReq->userid);
    return 0;
}

/*
 * 用户退出软件时,请求一个信息,服务器回应其提示信息
*/
int CLogonMgr::proc_client_getExitSoftMessage_req(task_proc_data * task_node)
{
    LOG_PRINT(log_info, "enter %s!", __FUNCTION__);

    COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *)task_node->pdata;
    ClientGateMask_t * pGateMask = (ClientGateMask_t *)(in_msg->content);
	char * pReq = (char *)(in_msg->content + SIZE_IVM_CLIENTGATE);
	unsigned int reqLen = SIZE_IVM_REQUEST(in_msg);
	CMDUserExistReq existReq;
	existReq.ParseFromArray(pReq, reqLen);
	if (existReq.platformtype() == SMS_TYPE)
	{
		CDBTools::resetTokenValidTime(existReq.loginid(), existReq.token());
	}

    LOG_PRINT(log_info, "<<<<<< end.");
    return 0;
}

bool CLogonMgr::check_mobilenum_valid(const std::string & mobile)
{
	//先检查手机号长度
	if (mobile.size() != 11)
	{
		return false;
	}

	char cmobile[12] = {0};
	strncpy(cmobile, mobile.c_str(), 11);

	std::string regstr = "^1[3,4,5,7,8][0-9]{9}$";
	//再检查手机号是否正常
	boost::regex expression(regstr);
	return boost::regex_match(cmobile, expression);
}

int CLogonMgr::check_loginid(char * cloginid, int nLength, int & loginType)
{
    if (NULL == cloginid || 0 == nLength)
    {
        return 106;
    }

    int nErrId = 0;

	bool bTel = check_mobilenum_valid(cloginid);
	if (bTel)
	{
		//telephone login
		loginType = 1;
	}
	else
	{
		//name login
		loginType = 2;
	}

    return nErrId;
}

void CLogonMgr::cast_user_sendmsg(char * pPkt, int Len, int userid)
{	
	if (!pPkt || !Len || !userid)
	{
		  LOG_PRINT(log_error, "castUserSendMsg input error.");
		  return;
	}

	COM_MSG_HEADER * pOutMsg = (COM_MSG_HEADER *)pPkt;
	ClientGateMask_t * pClientGate = (ClientGateMask_t *)pOutMsg->content;
	CMsgComm::Build_BroadCastUser_Gate(pClientGate, userid, e_Notice_AllType, 0);
	tcpserver::cast_msg_to_allconn(pPkt, Len);
}

int CLogonMgr::BuildUserLogonErrPack(int errid, char * pszBuf, ClientGateMask_t * pMask, int nBufSize, uint32 reqid)
{
	COM_MSG_HEADER * pOutMsg = (COM_MSG_HEADER *)(pszBuf);
	ClientGateMask_t * pClientGate = (ClientGateMask_t *)(pOutMsg->content);
	memcpy(pClientGate, pMask, SIZE_IVM_CLIENTGATE);

	pOutMsg->version = MDM_Version_Value;
	pOutMsg->checkcode = CHECKCODE;
	pOutMsg->maincmd = MDM_Vchat_Login;
	pOutMsg->subcmd = Sub_Vchat_logonErr2;
	pOutMsg->reqid = reqid;

	CMDUserLogonErrResp errRsp;
	errRsp.mutable_errinfo()->set_errid(errid);

	unsigned int rspLen = errRsp.ByteSize();
	char * pErr = (char *)(pOutMsg->content + SIZE_IVM_CLIENTGATE);
	errRsp.SerializeToArray(pErr, rspLen);

	pOutMsg->length = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + rspLen;
	return pOutMsg->length;
}
