
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <ctype.h>
#include <sstream>
#include <iostream>
#include <string>

#include "SL_Socket_CommonAPI.h"
#include "GlobalSetting.h"
#include "cmd_vchat.h"

#include "UserManager.h"
#include "CryptoHandler.h"

#include "utils.h"
#include "DBConnection.h"
#include "errcode.h"
#include "msgcommapi.h"
#include "DBTools.h"
#include "LogonSvr.pb.h"

CUserManager::CUserManager()
{
}

CUserManager::~CUserManager()
{

}

int CUserManager::GetVisitorId(char *_uuid)
{
	std::string uuid_key = KEY_VISTOR_INFO;
	uuid_key += "uuid:";
	uuid_key.append(_uuid);
	std::string val;
	uint32 uid = 0;
	string _userval = "";
	LOG_PRINT(log_debug, " now begin to find userid by uuid");
	if(!CGlobalSetting::m_app->m_redis_vistor.find(uuid_key, "userid", val, false)) //redis查不到
	{
		//find db
		SList* list = NULL;
		std::vector<VistorLogon_t> vVistor;
		int ret = CDBTools::get_vistorlogon_DB(_uuid, vVistor); //查询数据库
		if (ret < 0)
		{
			LOG_PRINT(log_error, "DB operation failed.uuid:%s.", _uuid);
			return ret;
		}

		LOG_PRINT(log_debug, "query,db ret = %d, size = %d", ret, vVistor.size());
		if (vVistor.size() > 0)
		{
			bool bUpdate = false;
			for (int i = 0; i < vVistor.size(); i++)
			{
				VistorLogon_t &vsl = vVistor[i];
				uint32 user_id = vsl.userid;
				if (vsl.state == 0 && !bUpdate)
				{
					LOG_PRINT(log_debug, "found available userid from db :%u, state = %d", vsl.userid, vsl.state);
					uid = user_id;
					vsl.state = 1;
					vsl.logontime = time(NULL);
					CDBTools::update_vistorlogon_DB(vsl);//更新db
					bUpdate = true;
				}

				char item_val[128] = {0};
				sprintf(item_val, "%u:%d:%llu", user_id, vsl.state, time(NULL));
				_userval += item_val;
				_userval += ";";
			}
			_userval.erase(_userval.end() - 1);//删除最后一个分号;
			LOG_PRINT(log_debug, "begin to insert userid into redis");
			CGlobalSetting::m_app->m_redis_vistor.insert(uuid_key, "userid", _userval, false); //刷新到redis
		}
		else//数据库也没有
		{
			VistorLogon_t _vsl;
			int retDB = CDBTools::func_get_visitor_unique_id(CGlobalSetting::m_app->m_area);
			if (retDB < 0)
			{
				LOG_PRINT(log_error, "DB operation failed.uuid:%s.", _uuid);
				return retDB;
			}

			_vsl.userid = retDB;
			strncpy(_vsl.cuuid, _uuid, sizeof(_vsl.cuuid));
			_vsl.logontime = time(NULL);
			_vsl.state = 1;
			char _val[128] = {0};
			sprintf(_val, "%u:%u:%llu", _vsl.userid, 1, _vsl.logontime);
			std::string strval = _val;
			LOG_PRINT(log_info, "not found any available userid from db, create a new one %u ", _vsl.userid);
			CGlobalSetting::m_app->m_redis_vistor.insert(uuid_key, "userid", strval, false);
			bool ret = CDBTools::insert_vistorlogon_DB(_vsl);
			LOG_PRINT(log_debug, "insert_vistorlogon_DB %s.", ret ? "successfully": "failed");
			uid = _vsl.userid;
		}	
	}
	else
	{
		bool bfind = false;
		std::list<std::string> vistor_itemlist;
		splitStrToLst(val, ';', vistor_itemlist);//userid:state:logontime;userid:state:logontime
		LOG_PRINT(log_debug, "vistor_itemlist.size()%d", vistor_itemlist.size());
		for (list<std::string>::iterator iter1 = vistor_itemlist.begin(); iter1 != vistor_itemlist.end(); ++iter1)
		{
			int state = 0;
			uint64 _logontime= 0;
			uint32 userid = 0;
			sscanf((*iter1).c_str(), "%u:%d:%llu", &userid, &state, &_logontime);
			char item_val[128] = {0};
			if (state == 0 && !bfind) //没登陆
			{
				LOG_PRINT(log_debug, "found available userid from redis=%u", userid);
				state = 1;
				sprintf(item_val, "%u:%d:%llu", userid, state, time(NULL)); //更新状态
				_userval += item_val;
				bfind = true;
				VistorLogon_t vsl;
				vsl.userid = userid;
				vsl.state = 1;
				vsl.logontime = time(NULL);
				CDBTools::update_vistorlogon_DB(vsl);//更新db
				uid = userid;
			}
			else
			{
				_userval += *iter1;
			}
			_userval += ";";
		}
		LOG_PRINT(log_debug, "get vistor _userval = %s", _userval.c_str());
		if (!bfind) //没发现可用的
		{
			VistorLogon_t _vsl;
			int retDB = CDBTools::func_get_visitor_unique_id(CGlobalSetting::m_app->m_area);
			if (retDB < 0)
			{
				LOG_PRINT(log_error, "DB operation failed.uuid:%s.", _uuid);
				return retDB;
			}

			_vsl.userid = retDB;
			strncpy(_vsl.cuuid, _uuid, sizeof(_vsl.cuuid));
			_vsl.logontime = time(NULL);
			_vsl.state = 1;
			char _val[128] = {0};
			sprintf(_val, "%u:%u:%llu", _vsl.userid, 1, _vsl.logontime);
			_userval += _val;
			LOG_PRINT(log_info, "not found any available userid, create a new one %u ", _vsl.userid);
			CGlobalSetting::m_app->m_redis_vistor.insert(uuid_key, "userid", _userval, false);
			CDBTools::insert_vistorlogon_DB(_vsl);
			uid = _vsl.userid;
		}
		else
		{
			CGlobalSetting::m_app->m_redis_vistor.insert(uuid_key, "userid", _userval, false);
		}
	}

	LOG_PRINT(log_debug, "end....");
	return uid;
}

int CUserManager::BuildUserTokenPack(unsigned int userid, char * pszBuf, COM_MSG_HEADER * pReqHead, ClientGateMask_t * pReqMask, int nBufSize, std::string & strSessionToken, std::string & strValidTime)
{
	if (!pszBuf || !pReqMask)
	{
		LOG_PRINT(log_error, "input buff is null.userid:%u.", userid);
		return -1;
	}

	struct tm * ptr;
	time_t lt;
	time_t now;
	char nowTime[32] = {0};
	//char dbValidTime[32] = "2017-01-01 00:00:00";
	char dbCodeTime[32] = {0};
	char newValidTime[32] = {0};

	char currentToken[34] = {0};
	char newToken[34] = {0};
	std::string strNewToken = "";
	int ret = 1;
	bool needsUpdate = false;

	if(!CDBTools::get_user_token_DB(userid, currentToken, dbCodeTime))
	{
		return -1;
	}

	COM_MSG_HEADER * pOutMsg = (COM_MSG_HEADER *)(pszBuf);
	ClientGateMask_t * pClientGate = (ClientGateMask_t *)(pOutMsg->content);
	memcpy(pClientGate, pReqMask, SIZE_IVM_CLIENTGATE);

	pOutMsg->version = MDM_Version_Value;
	pOutMsg->checkcode = CHECKCODE;
	pOutMsg->maincmd = MDM_Vchat_Login;
	pOutMsg->subcmd = Sub_Vchat_logonTokenNotify;
	if (pReqHead)
	{
		pOutMsg->reqid = pReqHead->reqid;
	}
	
	/*now = time(NULL) + 3600;//current time

	toStringTimestamp5(now, nowTime);

	int cRet = strcmp(dbCodeTime, nowTime);

	needsUpdate = (cRet <= 0) ? true : false;

	if(false == needsUpdate)
	{
		strcpy(newToken, currentToken);
		strncpy(newValidTime, dbCodeTime, 19);
	}
	else
	{
		lt = time(NULL);
		ptr = localtime(&lt);
		
		time_t future = lt + CGlobalSetting::m_app->m_token_time * 60;
		struct tm * furure_ptr = localtime(&future);
		
		sprintf(newValidTime,
			"%02d-%02d-%02d %02d:%02d:%02d",
			furure_ptr->tm_year+1900,
			furure_ptr->tm_mon+1,
			furure_ptr->tm_mday,
			furure_ptr->tm_hour,
			furure_ptr->tm_min,
			furure_ptr->tm_sec
			);
		
		std::string rand_eq = "";
		cryptohandler::getRandSeq(rand_eq, 5);
		//update the token
		std::string tmpToken = "";
		char tbuf[40] = {0};
		sprintf(tbuf,
			"%d|0|%02d%02d%02d%02d|%s",
			userid,
			ptr->tm_mday,
			ptr->tm_hour,
			ptr->tm_min,
			ptr->tm_sec,
			rand_eq.c_str());

		tmpToken = std::string(tbuf);
		std::string md5_token = "";
		cryptohandler::md5hash(tmpToken, md5_token);
		LOG_PRINT(log_info, "before md5:%s,after md5:%s.", tmpToken.c_str(), md5_token.c_str());

		//CDBTools::update_user_token_DB(userid, md5_token.c_str(), newValidTime);

		strcpy(newToken, md5_token.c_str());
	}
	
	strSessionToken = newToken;
	strValidTime = newValidTime;
*/
	strSessionToken = currentToken;
	strValidTime = dbCodeTime;
	CMDSessionTokenResp respData;
	respData.set_userid(userid);
	//respData.set_sessiontoken(newToken);
	//respData.set_validtime(newValidTime);
	respData.set_sessiontoken(currentToken);
	respData.set_validtime(dbCodeTime);
	int rspDataLen = respData.ByteSize();
	if (rspDataLen + SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE > nBufSize)
	{
		LOG_PRINT(log_error, "userid:%u buf size is not enough, require len:%d,but buf size:%d.", userid, rspDataLen + SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE, nBufSize);
		return -1;
	}

	char * pResp = (char *)(pOutMsg->content + SIZE_IVM_CLIENTGATE);
	respData.SerializeToArray(pResp, rspDataLen);
	
	LOG_PRINT(log_info, "userid[%u] token is [%s] validtime is [%s].", userid, currentToken, dbCodeTime);

	pOutMsg->length = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + rspDataLen;
	return pOutMsg->length;
}

//复位游客登陆状态
void CUserManager::ResetVistorInfo(char *_uuid, uint32 userid)
{
	std::string uuid_key = KEY_VISTOR_INFO;
	uuid_key += "uuid:";
	uuid_key.append(_uuid);
	std::string val;
	uint32 uid = 0;
	LOG_PRINT(log_debug, "uiuiid = %s, userid= %u", _uuid, userid);

	val = "";
	if (CGlobalSetting::m_app->m_redis_vistor.find(uuid_key, "userid", val,false))
	{
		list<std::string> vistor_itemlist;
		splitStrToLst(val, ';', vistor_itemlist);//userid:state:logontime;userid:state:logontime
		std::string _userval;
		LOG_PRINT(log_debug, "found userid exist in redis,uuid=:%s", _uuid);
		for (list<std::string>::iterator iter1 = vistor_itemlist.begin(); iter1 != vistor_itemlist.end(); ++iter1)
		{
			int state = 0;
			uint64 _logontime= 0;
			uint32 tmpid = 0;
			sscanf((*iter1).c_str(), "%u:%d:%llu", &tmpid, &state, &_logontime);
			char item_val[128] = {0};
			if (userid == tmpid)
			{
				uid = tmpid;
				sprintf(item_val, "%u:%d:%llu", uid, 0, time(NULL));
				LOG_PRINT(log_debug, "found matched userid = %u", uid);
				_userval += item_val;
			}
			else
			{
				_userval += *iter1;
			}
			_userval += ";";
		}
		_userval.erase(_userval.end() - 1);
		if (uid != 0)
		{
			VistorLogon_t _vsl;
			_vsl.userid = uid;
			strncpy(_vsl.cuuid, _uuid, sizeof(_vsl.cuuid) - 1);
			_vsl.state = 0;
			_vsl.logontime = time(NULL);
			LOG_PRINT(log_debug, "update vistor logon state");
			LOG_PRINT(log_debug, "uid = %u, state = %d", _vsl.userid, _vsl.state);
			CGlobalSetting::m_app->m_redis_vistor.insert(uuid_key, "userid", _userval, false);
			CDBTools::update_vistorlogon_DB(_vsl);
		}
	}
}
