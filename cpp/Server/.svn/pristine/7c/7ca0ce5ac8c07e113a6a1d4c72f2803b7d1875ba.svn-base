#include "PressStampMgr.h"
#include "msgcommapi.h"
#include "CUserGroupinfo.h"
#include "utils.h"
#include "DBTools.h"

unsigned int CPressStampMgr::m_stampStartPrice = 500;//印章起拍价
unsigned int CPressStampMgr::m_stampIncreasePrice = 100;//印章每次加价不低于多少元
 
CPressStampMgr::CPressStampMgr()
{ 
}

CPressStampMgr::~CPressStampMgr()
{

}
void CPressStampMgr::init(Dbconnection * dbconn)
{
	int stampStartPrice = CDBTools::qrySysParameterInt("STAMP_START_PRICE", 100);
	if (stampStartPrice > 1)
	{
		m_stampStartPrice = stampStartPrice;
	}
	LOG_PRINT(log_info, "stamp's start price:%d.", m_stampStartPrice);

	int stampIncreasePrice = CDBTools::qrySysParameterInt("STAMP_START_PRICE", 100);
	if (stampIncreasePrice > 1)
	{
		m_stampIncreasePrice = stampIncreasePrice;
	}
	LOG_PRINT(log_info, "stamp's Increase price:%d.", m_stampStartPrice);
}
int CPressStampMgr::handle_QryStampInfoReq(task_proc_data * message)
{
	CMDQryStampReq reqData;
	if (!CMsgComm::ParseProtoMessage(message->pdata, message->datalen, reqData))
	{
		LOG_PRINT(log_error, "parse %s packet error! length=%d.", reqData.GetTypeName().c_str(), message->datalen);
		message->resperrinf(ERR_CODE_FAILED_PACKAGEERROR);
		return -1;
	}

	LOG_PRINT(log_info, "receive handle_QryStampInfoReq: group[%u] userid[%u] dstUserID[%u] .", reqData.groupid(), reqData.userid(), reqData.dstuserid());
	if (!reqData.groupid() || !reqData.userid())
	{
		LOG_PRINT(log_error, "check param failed..");
		message->resperrinf(ERR_CODE_INVALID_PARAMETER);
		return -1;
	}

	CMDQryStampRsp rspData;
	rspData.set_groupid(reqData.groupid());
	rspData.set_startmoney(m_stampStartPrice);
	rspData.set_increasemoney(m_stampIncreasePrice);
	CMDStampInfo  stampInfo;
	
	if (CDBTools::getStampInfo(reqData.userid(), reqData.dstuserid(), reqData.groupid(), stampInfo) == 0)
	{
		CMDStampInfo *pInfo = rspData.add_stamp();
		*pInfo = stampInfo;
		LOG_PRINT(log_info, "getFixRePacketList.groupID:%u ", reqData.groupid() );
		message->respProtobuf(rspData, Sub_Vchat_QryStampRsp);
	}	
	return 0;
}
/* 
void CPressStampMgr::genPacketObj2MsgJson(const CMDSendPressStampReq & oRedPacketReq, Json::Value & oPacketJson)
{
	oPacketJson["userID"] = Json::Value(oRedPacketReq.userid());
	oPacketJson["groupID"] = Json::Value(oRedPacketReq.groupid());	
	oPacketJson["dstUserID"] = Json::Value(oRedPacketReq.dstuserid());	
	oPacketJson["Money"] = Json::Value(oRedPacketReq.packetmoney());	
	oPacketJson["message"] = Json::Value(oRedPacketReq.message());	
}*/

int CPressStampMgr::handle_SendPressStampReq(task_proc_data * message)
{
	
	/*CMDSendPressStampReq reqData;
	if (!CMsgComm::ParseProtoMessage(message->pdata, message->datalen, reqData))
	{
		LOG_PRINT(log_error, "parse %s packet error! length=%d.", reqData.GetTypeName().c_str(), message->datalen);
		message->resperrinf(ERR_CODE_FAILED_PACKAGEERROR);
		return -1;
	}

	LOG_PRINT(log_info, "receive handle_SendPressStampReq: group[%u] userid[%u] dstUserID[%u] money[%u] .", reqData.groupid(), reqData.userid(), reqData.dstuserid() , reqData.money());
	if (!reqData.groupid() || !reqData.userid() || !reqData.dstuserid())
	{
		LOG_PRINT(log_error, "check param failed..");
		message->resperrinf(ERR_CODE_INVALID_PARAMETER);
		return -1;
	}
	long srcUserid = 0;
	do
	{
		m_dbconn->transBegin();
		long ret = CDBTools::SendPressStampReq(reqData.userid(), reqData.dstuserid(), reqData.groupid(), reqData.money(), reqData.message());
		switch (ret)
		{
		case -1:
			LOG_PRINT(log_error, "DB SendPressStampReq error.userID:%u,groupID:%u,dstuserid:%u,money:%u.", reqData.userid(), reqData.groupid(), reqData.dstuserid(), reqData.money());
			ret = ERR_CODE_FAILED_DBERROR;
			break;
		case -2:
			LOG_PRINT(log_warning, "SendPressStampReq not enough money.userID:%u,groupID:%u,dstuserid:%u,money:%u.", reqData.userid(), reqData.groupid(), reqData.dstuserid(), reqData.money());
			ret = ERR_CODE_STAMP_NOT_ENOUGH_GOLD;
			break;
		case -3:
			LOG_PRINT(log_error, "SendPressStampReq  not under fix gold userID:%u,groupID:%u,dstuserid:%u,money:%u.", reqData.userid(), reqData.groupid(), reqData.dstuserid(), reqData.money());
			 
			ret = ERR_CODE_STAMP_NOT_UNDER_FIXGOLD;
			break;
		 
		default:
			srcUserid = ret;
			 
			印章被覆盖 REMIND_STAMP_COVER：
			{"remind_type": 0, "user_id": 123456, "alias": 昵称, "redpacket_id": 12}
			 
			Json::Value value;
			value["user_id"] = reqData.userid();			
			value["remind_type"] = REMIND_STAMP_COVER;
			value["alias"] = reqData.message();
			callSendChatRPC(reqData.userid(), srcUserid, reqData.groupid(), Json::FastWriter().write(value), true, MSGTYPE_REMIND);
			ret = 0;
		}
		Json::Value oPacketJson;
		genPacketObj2MsgJson(reqData, oPacketJson);
		Json::FastWriter fast_writer;
		std::string strJson = fast_writer.write(oPacketJson);
		if (0 != callSendChatRPC(reqData.userid(), reqData.dstuserid(), reqData.groupid(), strJson, false))
		{
			LOG_PRINT(log_error, "Error press stamp ID:%u.because rpc call ChatSvr fail,json:%s.", redPacketID, strJson.c_str());
			errorcode = ERR_CODE_REDPACKET_SEND;
			break;
		}
	} while (0);*/
	return 0;
}