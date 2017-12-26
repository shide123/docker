#include "CRedPacketMgr.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/transport/THttpClient.h>
#include "CLogThread.h"
#include "macro_define.h"
#include "errcode.h"
#include "DBTools.h"
#include "msgcommapi.h"
#include "utils.h"
#include "CFieldName.h"
#include "CUserGroupinfo.h"
#include "CUserBasicInfo.h"
#include "GroupBasicInfo.h"
#include "CThriftSvrMgr.h"
#include "TCommonIf_types.h"
#include "chatsvr/TChatSvr.h"
#include "roomsvr/TRoomSvr.h"
#include "ConsumeApp.h"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

redis_map_map CRedPacketMgr::m_redpacket_hash;
redis_map_map CRedPacketMgr::m_redpacket_users;
redis_map_map CRedPacketMgr::m_redpacket_senduser;
redis_map_zset CRedPacketMgr::m_redpacket_ids;

unsigned int CRedPacketMgr::m_validtime = 24 * 3600; //1 day by default
unsigned int CRedPacketMgr::m_timer_redpacket_validtime = 10 * 60; //10 mins by default
unsigned int CRedPacketMgr::m_cacheTime = 72 * 3600; //3 days by default
unsigned int CRedPacketMgr::m_comboTime = 5;		 //5 seconds by default

unsigned int CRedPacketMgr::m_goldCandyExchange = 1;
unsigned int CRedPacketMgr::m_goldContributeExchange = 1;
unsigned int CRedPacketMgr::m_cleanCacheTime = 0;
unsigned int CRedPacketMgr::m_location_diff = 100;//地点红包误差范围
unsigned int CRedPacketMgr::m_stampStartPrice = 500;//印章起拍价
unsigned int CRedPacketMgr::m_stampIncreasePrice = 500;//印章每次加价不低于多少元

#define PI                      3.1415926
#define EARTH_RADIUS            6378137.0        //地球近似半径





CRedPacketMgr::CRedPacketMgr(Dbconnection * dbconn)
{
	m_dbconn = dbconn;
}

CRedPacketMgr::~CRedPacketMgr()
{
}

int CRedPacketMgr::init(redisMgr * pRedisMgr)
{
	if (!pRedisMgr)
	{
		LOG_PRINT(log_error, "redisMgr is null.init failed.");
		return -1;
	}

	m_redpacket_hash.init(DB_REDPACKET, KEY_REDPACKET":", pRedisMgr);
	m_redpacket_users.init(DB_REDPACKET, KEY_REDPACKET_TAKEUSERS":", pRedisMgr);
	m_redpacket_senduser.init(DB_REDPACKET, KEY_REDPACKET_SENDUSER":", pRedisMgr);
	m_redpacket_ids.init(DB_REDPACKET, KEY_REDPAKCET_IDS, pRedisMgr);

	int goldCandyExchange = CDBTools::qrySysParameterInt("GOLD_CANDY_EXCHANGE", 1);
	if (goldCandyExchange > 1)
	{
		m_goldCandyExchange = goldCandyExchange;
	}
	LOG_PRINT(log_info, "red packet get gold candy exchange from db.1 gold can get %d candy.", m_goldCandyExchange);

	int goldContributeExchange = CDBTools::qrySysParameterInt("GOLD_CONTRIBUTE_EXCHANGE", 1);
	if (goldContributeExchange > 1)
	{
		m_goldContributeExchange = goldContributeExchange;
	}
	LOG_PRINT(log_info, "red packet send 1 gold packet can exchange %u contribute value.", m_goldContributeExchange);
	m_cleanCacheTime = time(NULL);

	int location_diff = CDBTools::qrySysParameterInt("RED_PACKET_LOCATION_DIFF", 100);
	if (location_diff > 1)
	{
		m_location_diff = location_diff;
	}
	LOG_PRINT(log_info, "location red packet diff deviation:%d.", m_location_diff);

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
	return 0;
}
// 求弧度
double CRedPacketMgr::radian(double d)
{
	return d * PI / 180.0;   //角度1˚ = π / 180
}

//计算两个经纬度间距离，单位米
double CRedPacketMgr::get_distance(double lat1, double lng1, double lat2, double lng2)
{
	double radLat1 = radian(lat1);
	double radLat2 = radian(lat2);
	double a = radLat1 - radLat2;
	double b = radian(lng1) - radian(lng2);

	double dst = 2 * asin((sqrt(pow(sin(a / 2), 2) + cos(radian(lat1)) * cos(radian(lat2)) * pow(sin(b / 2), 2))));

	dst = dst * EARTH_RADIUS;
	dst = round(dst * 10000) / 10000;
	return dst;
}
int CRedPacketMgr::handle_sendRedPacketReq(task_proc_data * message)
{
	if (!message || !message->connection)
	{
		LOG_PRINT(log_error, "send red packet request error.");
		return -1;
	}

	COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *) message->pdata;
	ClientGateMask_t * pGateMask = (ClientGateMask_t *)(in_msg->content);
	if(in_msg->length <= SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE)
	{
		LOG_PRINT(log_error, "handle_sendRedPacketReq packet is error!,length=%d,required at least:%u", in_msg->length, SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE);
		message->connection->resperrinf(in_msg, pGateMask, ERR_CODE_FAILED_PACKAGEERROR);
		return -1;
	}

	char * pReqData = (char *)(in_msg->content + SIZE_IVM_CLIENTGATE);
	unsigned int reqLen = SIZE_IVM_REQUEST(in_msg);
	CMDSendRedPacketReq reqData;
	reqData.ParseFromArray(pReqData, reqLen);

	LOG_PRINT(log_debug, "[handle_sendRedPacketReq]userID:%u,groupID:%u,packetType:%d,rangeType:%u.privatetype:%d,longitude：%f,latitude:%f., invalidTime:%u,fixtime:%u", \
		reqData.userid(), reqData.groupid(), (int)reqData.packettype(), (int)reqData.rangetype(), (int)reqData.privatetype(), reqData.longitude(), reqData.latitude(), reqData.invalidtime(),reqData.fixtime());
	int ret = 0;
	unsigned int packetID = 0;
	unsigned int comboNum = 0;
	do 
	{
		if (!reqData.userid() || !reqData.groupid())
		{
			LOG_PRINT(log_error, "sendRedPacket error,input userID:%u,groupID:%u.", reqData.userid(), reqData.groupid());
			ret = ERR_CODE_REDPACKET_SEND;
			break;
		}
		//校验地点红包合法性
		if (reqData.packettype() == eLocationType && ( (reqData.longitude() > 180 && reqData.longitude() < -180) || (reqData.latitude() <-90 && reqData.latitude() > 90)))
		{
			LOG_PRINT(log_error, "sendRedPacket error,input userID:%u,groupID:%u.", reqData.userid(), reqData.groupid());
			ret = ERR_CODE_REDPACKET_ERR_LNG_LAT;
			break;
		}
		//定时红包的时间必须设定在大于当前时间
		if ((reqData.packettype() == eFixTimeType || reqData.packettype() == eCommandFixTimeType || reqData.packettype() == eCommandUniFixTimeType) && reqData.fixtime() < time(NULL))
		{
			LOG_PRINT(log_error, "sendRedPacket error,fixtime too small input userID:%u,groupID:%u,fixtime:%u,service time:%u.", reqData.userid(), reqData.groupid(), reqData.fixtime(), time(NULL));
			ret = ERR_CODE_REDPACKET_FIXTIME_TOOSMALL;
			break;
		}
		switch(reqData.packettype())
		{
		case eLuckType:
		case eFixTimeType:
		case eLocationType:
		case eCommandImmediatelyType:
		case eCommandFixTimeType:
			ret = sendRedPacketLuckType(reqData, packetID, comboNum);
			break;
		case eUniType:
		case eCommandUniImmediatelyType:
		case eCommandUniFixTimeType:
			ret = sendRedPacketUniType(reqData, packetID, comboNum);
			break;
		case eDirectType:
			ret = sendRedPacketDirectType(reqData, packetID, comboNum, reqData.privatetype());
			break;
		case eLigntType:
			ret = sendRedPacketLightType(reqData, packetID, comboNum);
			break;
		case eJoinGroupType:
			ret = sendRedPacketJoinGroup(reqData, packetID, comboNum);
			break;
		default:
			LOG_PRINT(log_error, "send red packet error.unkown packetType.userID:%u,groupID:%u,packetType:%d", reqData.userid(), reqData.groupid(), (int)reqData.packettype());
			ret = ERR_CODE_REDPACKET_SEND;
			break;
		}

	} while (0);

	/*
	if (reqData.packettype() != eJoinGroupType && 0 == ret)
	{
	}
	*/

	CMDSendRedPacketResp oRspData;	
	oRspData.mutable_errinfo()->set_errid(ret);
	oRspData.set_userid(reqData.userid());
	oRspData.set_groupid(reqData.groupid());
	oRspData.set_packetid(packetID);
	oRspData.set_combonum(comboNum);
	oRspData.set_privatetype(reqData.privatetype());
	oRspData.set_combotime(m_comboTime);
	oRspData.set_packettype(reqData.packettype());
	oRspData.set_longitude(reqData.longitude());
	oRspData.set_latitude(reqData.latitude());
	LOG_PRINT(log_info, "[handle_sendRedPacketReq]userID:%u,groupID:%u,packetID:%u,errCode:%d,comboNum:%u,privateType:%d,comboTime:%u.", \
		reqData.userid(), reqData.groupid(), packetID, ret, comboNum, (int)reqData.privatetype(), m_comboTime);

	unsigned int rspDataLen = SIZE_IVM_HEAD_TOTAL + oRspData.ByteSize();
	SL_ByteBuffer buff(rspDataLen);
	buff.data_end(rspDataLen);

	COM_MSG_HEADER * pRspHead = (COM_MSG_HEADER *)buff.buffer();
	ClientGateMask_t * pRspGate = (ClientGateMask_t *)(pRspHead->content);
	GEN_RSP_MSGINFO_SET_HEAD_GATE(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_SendRedPacketResp, in_msg->reqid, pGateMask, pRspHead, pRspGate, oRspData);
	message->connection->write_message(buff);
	//推送定时红包消息， 只推送一条
	if ((reqData.packettype() == eFixTimeType || reqData.packettype() == eLocationType) && 0 == ret && reqData.groupid())
	{
		 
		CMDQryRedPacketRsp rspfixData;
		rspfixData.set_groupid(reqData.groupid());
		CMDRedPacket * fixrsp = rspfixData.add_rsp();
		std::list<stRedPacketInfo> lstPacketInfo;
		
		unsigned int  effrow = 0;
		if (CDBTools::getFixtimeRedPacketInfoByGrouID(reqData.userid(), reqData.groupid(), 0, 1, 0, effrow, reqData.packettype(), lstPacketInfo) < 0)
		{
			LOG_PRINT(log_error, "getFixtimeRedPacketInfoByGrouID error,input userID:%u,groupID:%u.", reqData.userid(), reqData.groupid());
			return -1;
		}	
		rspfixData.set_offset(effrow);
		stRedPacketInfo  &packetInfo  = *lstPacketInfo.begin();
		fixrsp->set_userid(packetInfo.srcUserID);
		fixrsp->set_packetid(packetInfo.packetID);
		fixrsp->set_packettype((ePacketType)packetInfo.packetType);		
		fixrsp->set_fixtime(packetInfo.fixTime);
		fixrsp->set_difftime(packetInfo.diffTime);
		fixrsp->set_packetmoney(packetInfo.packetMoney);
		fixrsp->set_rangetype((eRangeType)packetInfo.rangeType);
		fixrsp->set_packetnum(packetInfo.packetNum);
		fixrsp->set_rangegender((eRangeGender)packetInfo.rangeGender);
		fixrsp->set_message(packetInfo.strMessage);
		fixrsp->set_fixtype((ePacketFixType)packetInfo.fixType);
		fixrsp->set_longitude(packetInfo.longitude);
		fixrsp->set_latitude(packetInfo.latitude);

		unsigned int notyDataLen = SIZE_IVM_HEAD_TOTAL + rspfixData.ByteSize();
		SL_ByteBuffer oNotyBuff(notyDataLen);
		oNotyBuff.data_end(notyDataLen);

		COM_MSG_HEADER * pNotyHead = (COM_MSG_HEADER *)oNotyBuff.buffer();
		pNotyHead->version = MDM_Version_Value;
		pNotyHead->checkcode = CHECKCODE;
		pNotyHead->maincmd = MDM_Vchat_Room;
		pNotyHead->subcmd = Sub_Vchat_QryRedPacketRsp;
		pNotyHead->length = notyDataLen;

		ClientGateMask_t * pNotyGate = (ClientGateMask_t *)(pNotyHead->content);
		memset(pNotyGate, 0, SIZE_IVM_CLIENTGATE);
		CMsgComm::Build_BroadCastRoomGate(pNotyGate, e_Notice_AllType, reqData.groupid());
		rspfixData.SerializeToArray(pNotyHead->content + SIZE_IVM_CLIENTGATE, rspfixData.ByteSize());
		ConsumeApp::getInstance()->m_room_mgr->castGateway(pNotyHead);

	}
	return 0;
}

void CRedPacketMgr::notifyWeekList(unsigned int userid, unsigned int groupid, bool bCharmList/* = false*/)
{
	LOG_PRINT(log_info, "user take red packet successfully,maybe week list changed,so need to check list.userID:%u,groupID:%u,bCharm:%d.", userid, groupid, (int)bCharmList);
	bool bNeedNotify = false;
	CMDWeekListNotify oNotyData;
	getWeekList(userid, groupid, oNotyData, bNeedNotify, bCharmList);

	if (bNeedNotify)
	{
		LOG_PRINT(log_info, "user send a big red packet successfully,week contribute order changed,so need to notify contribution value order.userID:%u,groupID:%u.", userid, groupid);
		unsigned int notyDataLen = SIZE_IVM_HEAD_TOTAL + oNotyData.ByteSize();
		SL_ByteBuffer oNotyBuff(notyDataLen);
		oNotyBuff.data_end(notyDataLen);

		COM_MSG_HEADER * pNotyHead = (COM_MSG_HEADER *)oNotyBuff.buffer();
		pNotyHead->version = MDM_Version_Value;
		pNotyHead->checkcode = CHECKCODE;
		pNotyHead->maincmd = MDM_Vchat_Room;
		pNotyHead->subcmd = (bCharmList ? Sub_Vchat_WeekCharmNotify: Sub_Vchat_WeekContributeNotify);
		pNotyHead->length = notyDataLen;

		ClientGateMask_t * pNotyGate = (ClientGateMask_t *)(pNotyHead->content);
		memset(pNotyGate, 0, SIZE_IVM_CLIENTGATE);
		CMsgComm::Build_BroadCastRoomGate(pNotyGate, e_Notice_AllType, groupid);
		oNotyData.SerializeToArray(pNotyHead->content + SIZE_IVM_CLIENTGATE, oNotyData.ByteSize());
		ConsumeApp::getInstance()->m_room_mgr->castGateway(pNotyHead);
	}
	return;
}

int CRedPacketMgr::handle_catchRedPacketReq(task_proc_data * message)
{
	if (!message || !message->connection)
	{
		LOG_PRINT(log_error, "catch red packet request error.");
		return -1;
	}
	
	COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *) message->pdata;
	ClientGateMask_t * pGateMask = (ClientGateMask_t *)(in_msg->content);
	if(in_msg->length <= SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE)
	{
		LOG_PRINT(log_error, "handle_catchRedPacketReq packet is error!,length=%d,required at least:%u", in_msg->length, SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE);
		message->connection->resperrinf(in_msg, pGateMask, ERR_CODE_FAILED_PACKAGEERROR);
		return -1;
	}

	char * pReqData = (char *)(in_msg->content + SIZE_IVM_CLIENTGATE);
	unsigned int reqLen = SIZE_IVM_REQUEST(in_msg);
	CMDCatchRedPacketReq reqData;
	reqData.ParseFromArray(pReqData, reqLen);

	CMDCatchRedPacketResp oRspData;
	LOG_PRINT(log_info, "[handle_catchRedPacketReq]userID:%u,groupID:%u,packetID:%u.longitude:%lf,latitude:%lf", reqData.userid(), reqData.groupid(), reqData.packetid(),reqData.longitude(), reqData.latitude());
	int ret = checkRedPacketCanTake(reqData.userid(), reqData.groupid(), reqData.packetid(), reqData.longitude(), reqData.latitude(), oRspData.mutable_errinfo());
	
	oRspData.mutable_errinfo()->set_errid(ret);
	oRspData.set_groupid(reqData.groupid());
	oRspData.set_packetid(reqData.packetid());
	LOG_PRINT(log_info, "[handle_catchRedPacketReq]userID:%u,groupID:%u,packetID:%u.errCode:%d.", reqData.userid(), reqData.groupid(), reqData.packetid(), ret);

	stRedPacketInfo oRedPacketInfo = {};
	if (!genPacketObjFromRedis(reqData.packetid(), oRedPacketInfo))
	{
		LOG_PRINT(log_warning, "this redpacket has been removed from redis cache.userID:%u,groupID:%u,packetID:%u.", reqData.userid(), reqData.groupid(), reqData.packetid());
		CDBTools::getRedPacketBasicInfo(reqData.packetid(), oRedPacketInfo);
	}
	oRspData.set_fixtime(oRedPacketInfo.fixTime);
	int diff = oRedPacketInfo.fixTime - time(NULL);
	unsigned int diffTime = diff;
	if (diff < 0)
	{
		diffTime = 0;
	}
	 
	oRspData.set_difftime(diffTime);
	std::string strHead = "";
	std::string strAlias = "";
	UserInfo_t * pSrcUser = oRspData.mutable_srcuser();
	pSrcUser->set_userid(oRedPacketInfo.srcUserID);
	CUserBasicInfo::getUserHeadAndAlias(oRedPacketInfo.srcUserID, strHead, strAlias);
	pSrcUser->set_head(strHead);
	pSrcUser->set_alias(strAlias);

	UserInfo_t * pDstUser = oRspData.mutable_dstuser();
	if (oRedPacketInfo.dstUserID)
	{
		pDstUser->set_userid(oRedPacketInfo.dstUserID);
		CUserBasicInfo::getUserHeadAndAlias(oRedPacketInfo.dstUserID, strHead, strAlias);
		pDstUser->set_head(strHead);
		pDstUser->set_alias(strAlias);
	}

	oRspData.set_packettype((ePacketType)oRedPacketInfo.packetType);
	oRspData.set_rangetype((eRangeType)oRedPacketInfo.rangeType);
	oRspData.set_packetnum(oRedPacketInfo.packetNum);
	oRspData.set_message(oRedPacketInfo.strMessage);
	oRspData.set_privatetype(oRedPacketInfo.isPrivate);
	LOG_PRINT(log_debug, "catch this redpacket:%u private flag:%d.", reqData.packetid(), (int)oRedPacketInfo.isPrivate);

	if (oRedPacketInfo.fixPerMoney == 0)
	{
		oRspData.set_packetmoney(oRedPacketInfo.packetMoney);
	}
	else
	{
		oRspData.set_packetmoney(oRedPacketInfo.fixPerMoney * oRedPacketInfo.packetNum);
	}
	
	unsigned int rspDataLen = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + oRspData.ByteSize();
	SL_ByteBuffer buff(rspDataLen);
	buff.data_end(rspDataLen);

	COM_MSG_HEADER * pRspHead = (COM_MSG_HEADER *)buff.buffer();
	ClientGateMask_t * pRspGate = (ClientGateMask_t *)(pRspHead->content);
	GEN_RSP_MSGINFO_SET_HEAD_GATE(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_CatchRedPacketResp, in_msg->reqid, pGateMask, pRspHead, pRspGate, oRspData);
	message->connection->write_message(buff);
	return 0;
}

int CRedPacketMgr::handle_takeRedPacketReq(task_proc_data * message)
{
	CMDTakeRedPacketReq req;
	if (!CMsgComm::ParseProtoMessage(message->pdata, message->datalen, req))
	{
		LOG_PRINT(log_error, "parse CMDTakeRedPacketReq packet error! length=%d.", message->datalen);
		message->resperrinf(ERR_CODE_FAILED_PACKAGEERROR);
		return -1;
	}

	LOG_PRINT(log_info, "[handle_takeRedPacketReq]userID:%u,groupID:%u,packetID:%u.longitude:%lf,latitude:%lf", req.userid(), req.groupid(), req.packetid(), req.longitude(), req.latitude());
	uint32 money = 0;
	stRedPacketInfo packInfo = {};
	CMDTakeRedPacketResp resp;
	int ret = takeRedPacket(req.userid(), req.groupid(), req.packetid(), money, packInfo, req.longitude(), req.latitude(), resp.mutable_errinfo(), req.message());
	if (!packInfo.packetID && !genPacketObjFromRedis(req.packetid(), packInfo))
	{
		LOG_PRINT(log_warning, "this redpacket has been removed from redis cache.userID:%u,groupID:%u,packetID:%u.", req.userid(), req.groupid(), req.packetid());
		CDBTools::getRedPacketBasicInfo(req.packetid(), packInfo);
	}
	resp.mutable_errinfo()->set_errid(ret);
	resp.set_userid(req.userid());
	resp.set_packetid(req.packetid());
	resp.set_money(money);
	resp.set_privatetype(packInfo.isPrivate);
	resp.set_packettype((ePacketType)packInfo.packetType);
	resp.set_rangetype((eRangeType)packInfo.rangeType);
	resp.set_packetnum(packInfo.packetNum);
	resp.set_packetmoney((uint32)packInfo.packetMoney);
	resp.set_message(packInfo.strMessage);
	resp.set_fixtime(packInfo.fixTime);
	resp.set_difftime(packInfo.diffTime);
	LOG_PRINT(log_info, "[handle_takeRedPacketReq]userID:%u,groupID:%u,packetID:%u.errCode:%d,takeMoney:%u,privateFlag:%d.", req.userid(), req.groupid(), req.packetid(), ret, money, packInfo.isPrivate);

	unsigned int rspDataLen = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + resp.ByteSize();
	SL_ByteBuffer buff(rspDataLen);
	buff.data_end(rspDataLen);

	COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *) message->pdata;
	ClientGateMask_t * pGateMask = (ClientGateMask_t *)(in_msg->content);
	COM_MSG_HEADER * pRspHead = (COM_MSG_HEADER *)buff.buffer();
	ClientGateMask_t * pRspGate = (ClientGateMask_t *)(pRspHead->content);
	GEN_RSP_MSGINFO_SET_HEAD_GATE(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_TakeRedPacketResp, in_msg->reqid, pGateMask, pRspHead, pRspGate, resp);
	message->connection->write_message(buff);
	return 0;
}
int CRedPacketMgr::handle_QryFixTimeRedPacketReq(task_proc_data * message)
{
	CMDQryRedPacketReq reqData;
	if (!CMsgComm::ParseProtoMessage(message->pdata, message->datalen, reqData))
	{
		LOG_PRINT(log_error, "parse %s packet error! length=%d.", reqData.GetTypeName().c_str(), message->datalen);
		message->resperrinf(ERR_CODE_FAILED_PACKAGEERROR);
		return -1;
	}

	LOG_PRINT(log_info, "receive QryFixTimeRedPacketReq: group[%u] userid[%u] count[%u] offset[%u] fixtype[%u].", reqData.groupid(), reqData.userid(), reqData.count(), reqData.offset(), reqData.fixtype());
	if (!reqData.groupid() || !reqData.count())
	{
		LOG_PRINT(log_error, "check param failed..");
		message->resperrinf(ERR_CODE_INVALID_PARAMETER);
		return -1;
	}
	//限制每次查询最大条数
	int maxrowcount = reqData.count();
	if (reqData.count() > 50)
		maxrowcount = 50;
	CMDQryRedPacketRsp rspData; 
	rspData.set_groupid(reqData.groupid());
	rspData.set_clienttimestamp(reqData.clienttimestamp());
	if (getFixRePacketList(reqData.userid(), reqData.groupid(), reqData.offset(), maxrowcount, reqData.fixtype(), reqData.packettype(), rspData) == 0)
	{
		LOG_PRINT(log_info, "getFixRePacketList.groupID:%u,fixtype:%u.", reqData.groupid(), reqData.fixtype());
		message->respProtobuf(rspData, Sub_Vchat_QryRedPacketRsp);
	} 
	
	return 0;
}

int CRedPacketMgr::getFixRePacketList(unsigned int userid, unsigned int groupID, unsigned int currow, unsigned int rows, unsigned int fixtype, unsigned int packtype, CMDQryRedPacketRsp&rspData)
{
	std::list<stRedPacketInfo>  lstPacketInfo;	
	unsigned int effrow = 0;
	if (CDBTools::getFixtimeRedPacketInfoByGrouID(userid, groupID, currow, rows, fixtype, effrow, packtype, lstPacketInfo) < 0)
	{
		LOG_PRINT(log_error, "getFixtimeRedPacketInfoByGrouID error,input groupid:%u,fixtype:%u. currow[%u]  fixtype[%u]", groupID, fixtype, currow, rows);
		return -1;
	}
	std::list<stRedPacketInfo>::iterator iter_lst = lstPacketInfo.begin();
	rspData.set_offset(effrow);
	for (; iter_lst != lstPacketInfo.end(); ++iter_lst)
	{
		stRedPacketInfo &packetInfo = *iter_lst;
		CMDRedPacket * fixrsp = rspData.add_rsp();	
		fixrsp->set_userid(packetInfo.srcUserID);
		fixrsp->set_packetid(packetInfo.packetID);
		fixrsp->set_packettype((ePacketType)packetInfo.packetType);		
		fixrsp->set_fixtime(packetInfo.fixTime);
		fixrsp->set_difftime(packetInfo.diffTime);
		fixrsp->set_packetmoney(packetInfo.packetMoney);
		fixrsp->set_rangetype((eRangeType)packetInfo.rangeType);
		fixrsp->set_packetnum(packetInfo.packetNum);
		fixrsp->set_rangegender((eRangeGender)packetInfo.rangeGender);
		fixrsp->set_message(packetInfo.strMessage);
		fixrsp->set_fixtype((ePacketFixType)packetInfo.fixType);
		fixrsp->set_longitude(packetInfo.longitude);
		fixrsp->set_latitude(packetInfo.latitude);
	}
	return 0;
}
int CRedPacketMgr::handle_qryRedPacketInfoReq(task_proc_data * message)
{
	CMDQryRedPacketInfoReq req;
	if (!CMsgComm::ParseProtoMessage(message->pdata, message->datalen, req))
	{
		LOG_PRINT(log_error, "parse %s packet error! length=%d.", req.GetTypeName().c_str(), message->datalen);
		message->resperrinf(ERR_CODE_FAILED_PACKAGEERROR);
		return -1;
	}

	LOG_PRINT(log_info, "receive RedPacketInfoReq: group[%u] user[%u] packetid[%u] lastIndex[%u] count[%u].", req.groupid(), req.userid(), req.packetid(), req.offset(), req.count());
	if (!req.userid() || !req.packetid() || !req.groupid())
	{
		LOG_PRINT(log_error, "check param failed..");
		message->resperrinf(ERR_CODE_INVALID_PARAMETER);
		return -1;
	}
	
	if (!m_redpacket_hash.exist(req.packetid()))
	{
		reloadPacket2Redis(req.packetid());
	}

	sendRedPacketInfo(*message, req.userid(), req.packetid(), req.groupid(), req.count(), req.offset());
	return 0;
}

int CRedPacketMgr::handle_qryWeekContributeReq(task_proc_data * message)
{
	if (!message || !message->connection)
	{
		LOG_PRINT(log_error, "qryWeekContributeReq error.");
		return -1;
	}

	CMDQryWeekListReq req;
	if (!CMsgComm::ParseProtoMessage(message->pdata, message->datalen, req))
	{
		LOG_PRINT(log_error, "parse %s packet error! length=%d.", req.GetTypeName().c_str(), message->datalen);
		message->resperrinf(ERR_CODE_FAILED_PACKAGEERROR);
		return -1;
	}

	bool bNeedNotify = false;
	CMDWeekListNotify oNotyData;
	getWeekList(0, req.groupid(), oNotyData, bNeedNotify);
	message->respProtobuf(oNotyData, Sub_Vchat_WeekContributeNotify);
	oNotyData.Clear();
	getWeekList(0, req.groupid(), oNotyData, bNeedNotify, true);
	message->respProtobuf(oNotyData, Sub_Vchat_WeekCharmNotify);
	return 0;
}

int CRedPacketMgr::checkRedPacketValid(unsigned int userID, unsigned int roleType, unsigned int packetID, std::map<std::string, std::string > & field_value_map, double	lg, double	la, CMDErrCode *pErr/* = NULL*/, const string&strmsg /*= ""*/)
{
	if (!userID || !packetID || field_value_map.empty())
	{
		LOG_PRINT(log_error, "[input error]userID:%u packetID:%u field_value_map empty:%d.", userID, packetID, (int)field_value_map.empty());
		return ERR_CODE_REDPACKET_NOT_FOUND;
	}

	if (field_value_map.end() == field_value_map.find(CFieldName::REDPACKET_TYPE) || field_value_map.end() == field_value_map.find(CFieldName::REDPACKET_RANGETYPE) \
		|| field_value_map.end() == field_value_map.find(CFieldName::REDPACKET_DSTUSER) || field_value_map.end() == field_value_map.find(CFieldName::REDPACKET_CREATETIME) \
		|| field_value_map.end() == field_value_map.find(CFieldName::REDPACKET_RANGEGENDER))
	{
		LOG_PRINT(log_error, "field_value_map has no basic field.packetID:%u,userID:%u.", packetID, userID);
		return ERR_CODE_REDPACKET_NOT_FOUND;
	}

	LOG_PRINT(log_debug, "[checkRedPacketValid]input:packetID:%u,userID:%u,roleType:%u.", packetID, userID, roleType);

	if (m_redpacket_users.find(packetID, userID))
	{
		LOG_PRINT(log_error, "user[%u] has taken the red packet[%u].", userID, packetID);
		return ERR_CODE_REDPACKET_HAS_TAKEN;
	}

	unsigned int groupID = atoi(field_value_map[CFieldName::REDPACKET_GROUPID].c_str());
	ePacketType packetType = (ePacketType)atoi(field_value_map[CFieldName::REDPACKET_TYPE].c_str());
	eRangeType rangeType = (eRangeType)atoi(field_value_map[CFieldName::REDPACKET_RANGETYPE].c_str());
	eRangeGender rangeGender = (eRangeGender)atoi(field_value_map[CFieldName::REDPACKET_RANGEGENDER].c_str());
	unsigned int dstUser = atoi(field_value_map[CFieldName::REDPACKET_DSTUSER].c_str());
	unsigned int createTime = atoi(field_value_map[CFieldName::REDPACKET_CREATETIME].c_str());
	bool noMoney = (eNoLeft == atoi(field_value_map[CFieldName::REDPACKET_MONEYSTATE].c_str()));
	unsigned int gender = CUserBasicInfo::getUserGender(userID);
	unsigned int fixTime = atoi(field_value_map[CFieldName::REDPACKET_FIX_TIME].c_str()); 
	unsigned int invalidTime = atoi(field_value_map[CFieldName::REDPACKET_INVALID_TIME].c_str());
	double	longitude = atof(field_value_map[CFieldName::REDPACKET_LONGITUDE].c_str());
	double	latitude = atof(field_value_map[CFieldName::REDPACKET_LATITUDE].c_str());
	string msg = field_value_map[CFieldName::REDPACKET_PACKETMSG];
	if (noMoney)
	{
		LOG_PRINT(log_error, "[group:%u] redpacket:%u has no left. userid:%u,roleType:%u.", groupID, packetID, userID, roleType);
		return ERR_CODE_REDPACKET_HAS_NOLEFT;
	}
	unsigned int now_time = time(NULL);
	if ((packetType < eFixTimeType && now_time >= createTime + m_validtime) || ((packetType == eFixTimeType || packetType == eLocationType ) && now_time >= fixTime + m_validtime))
	{
		LOG_PRINT(log_error, "this redpacket:%u is expired.fixTime:%u, create_time:%u,validtime:%u,now_time:%u.", packetID, fixTime, createTime, m_validtime, now_time);
		return ERR_CODE_REDPACKET_EXPIRE;
	}
	if ((packetType == eCommandFixTimeType ||  packetType == eCommandUniFixTimeType) && now_time >= invalidTime)
	{
		LOG_PRINT(log_error, "this redpacket:%u is expired.fixTime:%u, create_time:%u,validtime:%u,now_time:%u.", packetID, fixTime, createTime, invalidTime, now_time);
		return ERR_CODE_REDPACKET_EXPIRE;
	}
	
	if ((packetType == eFixTimeType || packetType == eLocationType || packetType == eCommandFixTimeType || packetType == eCommandUniFixTimeType) && fixTime > time(NULL))
	{
		LOG_PRINT(log_error, "this redpacket has not start :%u,%u", fixTime, time(NULL));
		return ERR_CODE_REDPACKET_FIXTIME_NOT_SATRT;
	}
	if ((packetType == eCommandFixTimeType || packetType == eCommandUniFixTimeType || packetType == eCommandImmediatelyType || packetType == eCommandUniImmediatelyType) && strmsg != msg)
	{
		LOG_PRINT(log_error, "command redpacket:%s,%s", strmsg.c_str(), msg.c_str());
		return ERR_CODE_REDPACKET_ERR_COMMAND;
	}
	if (packetType == eLocationType )
	{
		double dis = get_distance(latitude, longitude, la, lg);
		LOG_PRINT(log_debug0, "get_distance,latitude:%lf,longitude:%lf,la:%lf,lg:%lf,dis:%lf", latitude, longitude, la, lg, dis);
		if (dis > m_location_diff)
			return ERR_CODE_REDPACKET_OUTOFF_LOCATION_RANGE;
	}
#define SET_ERROR(pErr, errid, parameter) \
	if (pErr) \
	{ \
		pErr->set_errid(errid); \
		pErr->add_paramvalue(parameter); \
	}
	std::string strLimit;
	switch (rangeGender)
	{
	case eGenderMale:
		strLimit = (eAllType == rangeType ? "男士": "男");
		break;
	case eGenderFemale:
		strLimit = (eAllType == rangeType ? "女士": "女");
		break;
	default:
		break;
	}
	switch (rangeType)
	{
	case eVisitorType:
		strLimit += "游客";
		break;
	case eMemberType:
		strLimit += "会员";
		break;
	default:
		break;
	}

	switch(packetType)
	{
	case eLuckType:
	case eUniType:
	case eJoinGroupType:
	case eFixTimeType:
		{
			if ((eVisitorType == rangeType && roleType != e_VisitorRole)
				|| (eMemberType == rangeType && roleType == e_VisitorRole))
			{
				LOG_PRINT(log_error, "[group:%u] redpacket:%u check rangeType.rangeType:%d,but userid:%u,roleType:%u.", groupID, packetID, (int)rangeType, userID, roleType);
				SET_ERROR(pErr, ERR_CODE_REDPACKET_NOT_YOURS, strLimit);
				return ERR_CODE_REDPACKET_NOT_YOURS;	
			}
			else if (eGenderAll != rangeGender && gender != rangeGender)
			{
				LOG_PRINT(log_error, "[group:%u] redpacket:%u check rangeGender. rangeGender:%d, but userid:%u, gender:%u.", groupID, packetID, (int)rangeGender, userID, gender);
				SET_ERROR(pErr, ERR_CODE_REDPACKET_NOT_YOURS, strLimit);
				return ERR_CODE_REDPACKET_NOT_YOURS;	
			}
		}
		break;
	case eDirectType:
		{
			if (dstUser != userID)
			{
				LOG_PRINT(log_error, "[group:%u] redpacket:%u check dstUser.but userid:%u is not equal dst_user:%u",groupID,  packetID, userID, dstUser);
				SET_ERROR(pErr, ERR_CODE_REDPACKET_NOT_YOURS, CUserBasicInfo::getUserAlias(dstUser));
				return ERR_CODE_REDPACKET_NOT_YOURS;
			}			
		}
		break;
	case eLigntType:
		{
			if (roleType == e_VisitorRole)
			{
				LOG_PRINT(log_error, "[group:%u] redpacket:%u check member.but userid:%u, roleType:%u.", groupID, packetID, userID, roleType);
				SET_ERROR(pErr, ERR_CODE_REDPACKET_NOT_YOURS, strLimit);
				return ERR_CODE_REDPACKET_NOT_YOURS;	
			}
			else if (eGenderAll != rangeGender && gender != rangeGender)
			{
				LOG_PRINT(log_error, "[group:%u] redpacket:%u check rangeGender. rangeGender:%d, but userid:%u, gender:%u.", groupID, packetID, (int)rangeGender, userID, gender);
				SET_ERROR(pErr, ERR_CODE_REDPACKET_NOT_YOURS, strLimit);
				return ERR_CODE_REDPACKET_NOT_YOURS;	
			}
			else if (!CDBTools::check_user_has_takephoto(userID, packetID))
			{
				LOG_PRINT(log_error, "[group:%u] redpacket need user take photo,but user has not.packet:%u,userid:%u,packetType:%u.", groupID, packetID, userID, packetType);
				return ERR_CODE_REDPACKET_NEED_PHOTO;
			}
		}
		break;
	default:
		break;
	}

	return ERR_CODE_SUCCESS;
}

void CRedPacketMgr::genPacketObj2FieldValue(const stRedPacketInfo & oRedPacketInfo, std::map<std::string, std::string> & field_value_map)
{
	field_value_map.insert(std::make_pair(CFieldName::REDPACKET_CREATETIME,	bitTostring(oRedPacketInfo.createTime)));
	field_value_map.insert(std::make_pair(CFieldName::REDPACKET_GROUPID,	bitTostring(oRedPacketInfo.groupID)));
	field_value_map.insert(std::make_pair(CFieldName::REDPACKET_TYPE,		bitTostring(oRedPacketInfo.packetType)));
	field_value_map.insert(std::make_pair(CFieldName::REDPACKET_RANGETYPE,	bitTostring(oRedPacketInfo.rangeType)));
	field_value_map.insert(std::make_pair(CFieldName::REDPACKET_RANGEGENDER,	bitTostring(oRedPacketInfo.rangeGender)));
	field_value_map.insert(std::make_pair(CFieldName::REDPACKET_DSTUSER,	bitTostring(oRedPacketInfo.dstUserID)));
	field_value_map.insert(std::make_pair(CFieldName::REDPACKET_SRCUSER,	bitTostring(oRedPacketInfo.srcUserID)));
	field_value_map.insert(std::make_pair(CFieldName::REDPACKET_MONEYSTATE,	bitTostring(oRedPacketInfo.moneyState)));
	field_value_map.insert(std::make_pair(CFieldName::REDPACKET_PER_FIX_MONEY, bitTostring(oRedPacketInfo.fixPerMoney)));
	field_value_map.insert(std::make_pair(CFieldName::REDPACKET_PACKEMONEY, bitTostring(oRedPacketInfo.packetMoney)));
	field_value_map.insert(std::make_pair(CFieldName::REDPACKET_PACKETMSG, bitTostring(oRedPacketInfo.strMessage)));
	field_value_map.insert(std::make_pair(CFieldName::REDPACKET_PACKETNUM, bitTostring(oRedPacketInfo.packetNum)));
	field_value_map.insert(std::make_pair(CFieldName::REDPACKET_ISPRIVATE, bitTostring((int)oRedPacketInfo.isPrivate)));
	field_value_map.insert(std::make_pair(CFieldName::REDPACKET_FIX_TIME, bitTostring((int)oRedPacketInfo.fixTime)));
	field_value_map.insert(std::make_pair(CFieldName::REDPACKET_INVALID_TIME, bitTostring((int)oRedPacketInfo.invalidTime)));
	field_value_map.insert(std::make_pair(CFieldName::REDPACKET_DIFF_TIME, bitTostring((int)oRedPacketInfo.diffTime)));
	field_value_map.insert(std::make_pair(CFieldName::REDPACKET_LONGITUDE, bitTostring((double)oRedPacketInfo.longitude)));
	field_value_map.insert(std::make_pair(CFieldName::REDPACKET_LATITUDE, bitTostring((double)oRedPacketInfo.latitude)));
	field_value_map.insert(std::make_pair(CFieldName::REDPACKET_LUCKYNUM, bitTostring((double)oRedPacketInfo.luckyNum)));
}

bool CRedPacketMgr::genPacketObjFromRedis(unsigned int packetID, stRedPacketInfo & oRedPacketInfo)
{
	std::map<std::string, std::string > oFieldValueRedisMap;
	IF_METHOD_FALSE_RETURN_EX(m_redpacket_hash.getall(packetID, oFieldValueRedisMap), false, "get redpacket:%u info from redis fail.", packetID);
    if (oFieldValueRedisMap.empty())
	{
		LOG_PRINT(log_error, "redpacket:%u info from redis is empty.", packetID);
		return false;
	}

	if (oFieldValueRedisMap.find(CFieldName::REDPACKET_CREATETIME) != oFieldValueRedisMap.end())
		oRedPacketInfo.createTime = atoi(oFieldValueRedisMap[CFieldName::REDPACKET_CREATETIME].c_str());

	if (oFieldValueRedisMap.find(CFieldName::REDPACKET_GROUPID) != oFieldValueRedisMap.end())
		oRedPacketInfo.groupID = atoi(oFieldValueRedisMap[CFieldName::REDPACKET_GROUPID].c_str());

	if (oFieldValueRedisMap.find(CFieldName::REDPACKET_TYPE) != oFieldValueRedisMap.end())
		oRedPacketInfo.packetType = atoi(oFieldValueRedisMap[CFieldName::REDPACKET_TYPE].c_str());

	if (oFieldValueRedisMap.find(CFieldName::REDPACKET_RANGETYPE) != oFieldValueRedisMap.end())
		oRedPacketInfo.rangeType = atoi(oFieldValueRedisMap[CFieldName::REDPACKET_RANGETYPE].c_str());

	if (oFieldValueRedisMap.find(CFieldName::REDPACKET_RANGEGENDER) != oFieldValueRedisMap.end())
		oRedPacketInfo.rangeGender = atoi(oFieldValueRedisMap[CFieldName::REDPACKET_RANGEGENDER].c_str());

	if (oFieldValueRedisMap.find(CFieldName::REDPACKET_DSTUSER) != oFieldValueRedisMap.end())
		oRedPacketInfo.dstUserID = atoi(oFieldValueRedisMap[CFieldName::REDPACKET_DSTUSER].c_str());

	if (oFieldValueRedisMap.find(CFieldName::REDPACKET_SRCUSER) != oFieldValueRedisMap.end())
		oRedPacketInfo.srcUserID = atoi(oFieldValueRedisMap[CFieldName::REDPACKET_SRCUSER].c_str());

	if (oFieldValueRedisMap.find(CFieldName::REDPACKET_MONEYSTATE) != oFieldValueRedisMap.end())
		oRedPacketInfo.moneyState = atoi(oFieldValueRedisMap[CFieldName::REDPACKET_MONEYSTATE].c_str());

	if (oFieldValueRedisMap.find(CFieldName::REDPACKET_PER_FIX_MONEY) != oFieldValueRedisMap.end())
		oRedPacketInfo.fixPerMoney = atoll(oFieldValueRedisMap[CFieldName::REDPACKET_PER_FIX_MONEY].c_str());

	if (oFieldValueRedisMap.find(CFieldName::REDPACKET_PACKEMONEY) != oFieldValueRedisMap.end())
		oRedPacketInfo.packetMoney = atoll(oFieldValueRedisMap[CFieldName::REDPACKET_PACKEMONEY].c_str());

	if (oFieldValueRedisMap.find(CFieldName::REDPACKET_PACKETMSG) != oFieldValueRedisMap.end())
		oRedPacketInfo.strMessage = oFieldValueRedisMap[CFieldName::REDPACKET_PACKETMSG];

	if (oFieldValueRedisMap.find(CFieldName::REDPACKET_PACKETNUM) != oFieldValueRedisMap.end())
		oRedPacketInfo.packetNum = atoi(oFieldValueRedisMap[CFieldName::REDPACKET_PACKETNUM].c_str());

	if (oFieldValueRedisMap.find(CFieldName::REDPACKET_ISPRIVATE) != oFieldValueRedisMap.end())
		oRedPacketInfo.isPrivate = (bool)atoi(oFieldValueRedisMap[CFieldName::REDPACKET_ISPRIVATE].c_str());

	if (oFieldValueRedisMap.find(CFieldName::REDPACKET_FIX_TIME) != oFieldValueRedisMap.end())
		oRedPacketInfo.fixTime = atoi(oFieldValueRedisMap[CFieldName::REDPACKET_FIX_TIME].c_str());
	if (oFieldValueRedisMap.find(CFieldName::REDPACKET_DIFF_TIME) != oFieldValueRedisMap.end())
		oRedPacketInfo.diffTime = atoi(oFieldValueRedisMap[CFieldName::REDPACKET_DIFF_TIME].c_str());
	return true;
}

void CRedPacketMgr::genPacketObj2MsgJson(const CMDSendRedPacketReq & oRedPacketReq, Json::Value & oPacketJson)
{
	oPacketJson["userID"]			= Json::Value(oRedPacketReq.userid());
	oPacketJson["groupID"]			= Json::Value(oRedPacketReq.groupid());
	oPacketJson["packetType"]		= Json::Value((int)oRedPacketReq.packettype());
	oPacketJson["rangeType"]		= Json::Value((int)oRedPacketReq.rangetype());
	oPacketJson["dstUserID"]		= Json::Value(oRedPacketReq.dstuserid());
	oPacketJson["packetNum"]		= Json::Value(oRedPacketReq.packetnum());
	oPacketJson["packetMoney"]		= Json::Value(oRedPacketReq.packetmoney());
	oPacketJson["perPacketMoney"]	= Json::Value(oRedPacketReq.perpacketmoney());
	oPacketJson["message"]			= Json::Value(oRedPacketReq.message());
	oPacketJson["rangeGender"]		= Json::Value((int)oRedPacketReq.rangegender());
	oPacketJson["longitude"]		= Json::Value(oRedPacketReq.longitude());
	oPacketJson["latitude"]			= Json::Value(oRedPacketReq.latitude());
	oPacketJson["location"] = Json::Value(oRedPacketReq.location());
}

int CRedPacketMgr::sendRedPacketLuckType(CMDSendRedPacketReq & reqData, unsigned int & packetID, unsigned int & comboNum)
{
	if (!reqData.packetnum() || !reqData.packetmoney())
	{
		LOG_PRINT(log_error, "send luck type red packet input error.packet num:%u,packet money:%u.send userid:%u,groupid:%u.", reqData.packetnum(), reqData.packetmoney(), reqData.userid(), reqData.groupid());
		return ERR_CODE_REDPACKET_SEND;
	}

	stRedPacketInfo oRedPacketInfo = {0};
	oRedPacketInfo.createTime = time(NULL);
	oRedPacketInfo.srcUserID = reqData.userid();
	oRedPacketInfo.groupID = reqData.groupid();
	oRedPacketInfo.packetType = reqData.packettype();
	
	if (reqData.message().empty())
	{
		std::string redpacketMsg = CDBSink().qrySysParameterString((eFixTimeType == reqData.packettype() ? "RED_PACKET_MSG_FIXTIME": "RED_PACKET_MSG_LUCK"), "");
		oRedPacketInfo.strMessage = redpacketMsg;
		reqData.set_message(redpacketMsg);
	}
	else
	{
		oRedPacketInfo.strMessage = reqData.message();
	}

	oRedPacketInfo.rangeType = reqData.rangetype();
	oRedPacketInfo.packetNum = reqData.packetnum();
	oRedPacketInfo.packetMoney = reqData.packetmoney();
	oRedPacketInfo.rangeGender = reqData.rangegender();
	oRedPacketInfo.fixTime = reqData.fixtime();
	oRedPacketInfo.longitude = reqData.longitude();
	oRedPacketInfo.latitude = reqData.latitude();
	oRedPacketInfo.invalidTime = reqData.invalidtime();
	oRedPacketInfo.fixTime = reqData.fixtime();
	Json::Value oPacketJson;
	genPacketObj2MsgJson(reqData, oPacketJson);
	return sendRedPacketCommHandle(oRedPacketInfo, oPacketJson, packetID, comboNum, false);
}

int CRedPacketMgr::sendRedPacketUniType(CMDSendRedPacketReq & reqData, unsigned int & packetID, unsigned int & comboNum)
{
	if (!reqData.packetnum() || !reqData.perpacketmoney())
	{
		LOG_PRINT(log_error, "send uni type red packet input error.packet num:%u,per packet money:%u.send userid:%u,groupid:%u.", reqData.packetnum(), reqData.perpacketmoney(), reqData.userid(), reqData.groupid());
		return ERR_CODE_REDPACKET_SEND;
	}

	stRedPacketInfo oRedPacketInfo = {0};
	oRedPacketInfo.createTime = time(NULL);
	oRedPacketInfo.srcUserID = reqData.userid();
	oRedPacketInfo.groupID = reqData.groupid();
	oRedPacketInfo.packetType = reqData.packettype();

	if (reqData.message().empty())
	{
		std::string redpacketMsg = CDBSink().qrySysParameterString("RED_PACKET_MESSAGE", "");
		oRedPacketInfo.strMessage = redpacketMsg;
		reqData.set_message(redpacketMsg);
	}
	else
	{
		oRedPacketInfo.strMessage = reqData.message();
	}

	oRedPacketInfo.rangeType = reqData.rangetype();
	oRedPacketInfo.packetNum = reqData.packetnum();
	oRedPacketInfo.fixPerMoney = reqData.perpacketmoney();
	oRedPacketInfo.packetMoney = reqData.packetnum() * reqData.perpacketmoney();
	oRedPacketInfo.rangeGender = reqData.rangegender();
	oRedPacketInfo.invalidTime = reqData.invalidtime();
	oRedPacketInfo.fixTime = reqData.fixtime();
	Json::Value oPacketJson;
	genPacketObj2MsgJson(reqData, oPacketJson);
	return sendRedPacketCommHandle(oRedPacketInfo, oPacketJson, packetID, comboNum, false);
}

int CRedPacketMgr::sendRedPacketDirectType(CMDSendRedPacketReq & reqData, unsigned int & packetID, unsigned int & comboNum, bool isPrivate)
{
	if (!reqData.packetmoney() || !reqData.dstuserid())
	{
		LOG_PRINT(log_error, "send direct type red packet input error.packet money:%u,dst_userid:%u.send userid:%u,groupid:%u.", reqData.packetmoney(), reqData.dstuserid(), reqData.userid(), reqData.groupid());
		return ERR_CODE_REDPACKET_SEND;
	}

	if (reqData.dstuserid() == reqData.userid())
	{
		LOG_PRINT(log_error, "direct red packet can not send to self, user[%u]", reqData.userid());
		return ERR_CODE_REDPACKET_TO_SELF;
	}

	stRedPacketInfo oRedPacketInfo = {0};
	oRedPacketInfo.createTime = time(NULL);
	oRedPacketInfo.srcUserID = reqData.userid();
	oRedPacketInfo.groupID = reqData.groupid();
	oRedPacketInfo.packetType = reqData.packettype();

	if (reqData.message().empty())
	{
		std::string redpacketMsg = CDBSink().qrySysParameterString("RED_PACKET_MSG_DIRECT", "");
		oRedPacketInfo.strMessage = redpacketMsg;
		reqData.set_message(redpacketMsg);
	}
	else
	{
		oRedPacketInfo.strMessage = reqData.message();
	}

	oRedPacketInfo.dstUserID = reqData.dstuserid();
	oRedPacketInfo.isPrivate = isPrivate;
	oRedPacketInfo.packetMoney = reqData.packetmoney();
	oRedPacketInfo.packetNum = 1;
	reqData.set_packetnum(1);
	
	Json::Value oPacketJson;
	genPacketObj2MsgJson(reqData, oPacketJson);
	return sendRedPacketCommHandle(oRedPacketInfo, oPacketJson, packetID, comboNum, isPrivate);
}

int CRedPacketMgr::sendRedPacketLightType(CMDSendRedPacketReq & reqData, unsigned int & packetID, unsigned int & comboNum)
{
	if (!reqData.packetnum() || !reqData.packetmoney())
	{
		LOG_PRINT(log_error, "send light type red packet input error.packet num:%u,packet money:%u.send userid:%u,groupid:%u.", reqData.packetnum(), reqData.packetmoney(), reqData.userid(), reqData.groupid());
		return ERR_CODE_REDPACKET_SEND;
	}

	stRedPacketInfo oRedPacketInfo = {0};
	oRedPacketInfo.createTime = time(NULL);
	oRedPacketInfo.srcUserID = reqData.userid();
	oRedPacketInfo.groupID = reqData.groupid();
	oRedPacketInfo.packetType = reqData.packettype();

	if (reqData.message().empty())
	{
		std::string redpacketMsg = CDBSink().qrySysParameterString("无图无真相~", "");
		oRedPacketInfo.strMessage = redpacketMsg;
		reqData.set_message(redpacketMsg);
	}
	else
	{
		oRedPacketInfo.strMessage = reqData.message();
	}

	oRedPacketInfo.packetNum = reqData.packetnum();
	oRedPacketInfo.packetMoney = reqData.packetmoney();
	oRedPacketInfo.rangeType = eMemberType;	//只有群成员才能抢
	oRedPacketInfo.rangeGender = reqData.rangegender();

	Json::Value oPacketJson;
	genPacketObj2MsgJson(reqData, oPacketJson);
	return sendRedPacketCommHandle(oRedPacketInfo, oPacketJson, packetID, comboNum, false);
}

int CRedPacketMgr::checkJoinGroupRedPacketNeed(unsigned int userID, RoomObject_Ref & roomObjRef, unsigned int & packetMoney, unsigned int & packetNum)
{
	packetMoney = 0;
	packetNum = 0;
	if (!userID || roomObjRef.get() == 0)
	{
		LOG_PRINT(log_error, "check join group redpacket need input error.");
		return ERR_CODE_FAILED_INVALIDCHAR;
	}

	unsigned int authType = 0;
	unsigned int redpacketNum = 0;
	unsigned int redpacketTotal = 0;
	int gender = CUserBasicInfo::getUserGender(userID);
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
		LOG_PRINT(log_error, "[check join group redpacket]cannot get user gender.userid:%u.", userID);
		return ERR_CODE_FAILED_USERNOTFOUND;
	}

	switch(authType)
	{
	case e_JoinGroupAuthType::NoNeedAuth:
		LOG_PRINT(log_info, "not need to auth to join group.userid:%u,gender:%d.", userID, gender);
		return ERR_CODE_SUCCESS;
	case e_JoinGroupAuthType::RedPacket:
		if (redpacketNum && redpacketTotal)
		{
			LOG_PRINT(log_info, "redpacket auth to join group and redpacket setting is not empty.red packet num:%u,red packet total:%u.userid:%u,gender:%d.", \
				redpacketNum, redpacketTotal, userID, gender);
			packetMoney = redpacketTotal;
			packetNum = redpacketNum;
		}
		else
		{
			LOG_PRINT(log_info, "redpacket auth to join group but redpacket setting is empty.userid:%u,gender:%d.", userID, gender);
		}
		return ERR_CODE_SUCCESS;
	default:
		LOG_PRINT(log_error, "join group auth is not redpacket so not need to send join group redpacket.auth type:%u,userid:%u,gender:%d.", authType, userID, gender);
		return ERR_CODE_REDPACKET_NOT_NEED;
	}
}

int CRedPacketMgr::sendRedPacketJoinGroup(CMDSendRedPacketReq & reqData, unsigned int & packetID, unsigned int & comboNum)
{
	if (!reqData.userid() || !reqData.groupid())
	{
		LOG_PRINT(log_error, "send join group red packet input error.userid:%u,groupid:%u.", reqData.userid(), reqData.groupid());
		return ERR_CODE_REDPACKET_SEND;
	}

	RoomObject_Ref oGroupObj = ConsumeApp::getInstance()->m_room_mgr->FindRoom(reqData.groupid());
	if(oGroupObj.get() == 0) 
	{
		LOG_PRINT(log_error, "send join group red packet input error.not found group.userid:%u,groupid:%u.", reqData.userid(), reqData.groupid());
		return ERR_CODE_FAILED_ROOMIDNOTFOUND;
	}

	if (CUserGroupinfo::isMember(reqData.userid(), reqData.groupid()))
	{
		LOG_PRINT(log_warning, "Not need to send join group red packet.userid:%u is groupid:%u member.", reqData.userid(), reqData.groupid());
		return ERR_CODE_REDPACKET_NOT_NEED;
	}

	unsigned int packetMoney = 0;
	unsigned int packetNum = 0;
	int ret = checkJoinGroupRedPacketNeed(reqData.userid(), oGroupObj, packetMoney, packetNum);
	if (ret != ERR_CODE_SUCCESS)
	{
		LOG_PRINT(log_error, "check if send join group redpacket need return error.userid:%u,groupid:%u,erroCode:%d.", reqData.userid(), reqData.groupid(), ret);
		return ret;
	}

	if (packetMoney && packetNum)
	{
		stRedPacketInfo oRedPacketInfo = {0};
		oRedPacketInfo.createTime = time(NULL);
		oRedPacketInfo.srcUserID = reqData.userid();
		oRedPacketInfo.groupID = reqData.groupid();
		oRedPacketInfo.packetType = reqData.packettype();
		
		oRedPacketInfo.packetMoney = packetMoney;
		oRedPacketInfo.packetNum = packetNum;
		oRedPacketInfo.rangeType = eMemberType;	//只有群成员才能抢

		if (reqData.message().empty())
		{
			std::string redpacketMsg = CDBSink().qrySysParameterString("RED_PACKET_MESSAGE", "");
			oRedPacketInfo.strMessage = redpacketMsg;
			reqData.set_message(redpacketMsg);
		}
		else
		{
			oRedPacketInfo.strMessage = reqData.message();
		}

		reqData.set_packetmoney(packetMoney);
		reqData.set_packetnum(packetNum);
		reqData.set_rangetype(eMemberType);	//只有群成员才能抢
		oRedPacketInfo.rangeGender = eGenderAll; //男女均可以领取
		reqData.set_dstuserid(0);

		Json::Value oPacketJson;
		genPacketObj2MsgJson(reqData, oPacketJson);
		return sendRedPacketCommHandle(oRedPacketInfo, oPacketJson, packetID, comboNum, false);
	}
	else
	{
		LOG_PRINT(log_info, "Not need to send join group red packet.group has no red packet limit.userid:%u,groupid:%u.", reqData.userid(), reqData.groupid());
		IF_METHOD_NOTZERO_RETURN(callJoinGroupRPC(reqData.userid(), reqData.groupid()), ERR_CODE_REDPACKET_SEND);
		return 0;
	}
}

unsigned int CRedPacketMgr::getComboNumber(unsigned int send_user_id, const stRedPacketInfo & oNewRedPacket)
{
	if (!send_user_id)
	{
		LOG_PRINT(log_error, "getComboNumber input error, send userid is 0.");
		return 0;
	}

	unsigned int ret = 1;
	std::map<std::string, std::string> field_value_map;
	m_redpacket_senduser.getall(send_user_id, field_value_map);
	
	do 
	{
		if (field_value_map.empty() || field_value_map.find(CFieldName::REDPACKET_RECENT_ID) == field_value_map.end())
		{
			break;
		}
		else
		{
			unsigned int redpacketID = atoi(field_value_map[CFieldName::REDPACKET_RECENT_ID].c_str());
			stRedPacketInfo oRecentRedPacket;
			if (!genPacketObjFromRedis(redpacketID, oRecentRedPacket))
			{
				LOG_PRINT(log_warning, "this redpacket has been removed from redis cache.send userID:%u,packetID:%u.", send_user_id, redpacketID);
				break;
			}

			std::stringstream oldPacketCond;
			oldPacketCond << oRecentRedPacket.groupID << " " << oRecentRedPacket.srcUserID << " " << oRecentRedPacket.dstUserID << " ";
			oldPacketCond << oRecentRedPacket.packetType << " " << oRecentRedPacket.rangeType << " " << oRecentRedPacket.packetNum << " ";
			oldPacketCond << oRecentRedPacket.fixPerMoney << " " << oRecentRedPacket.packetMoney;

			std::stringstream newPacketCond;
			newPacketCond << oNewRedPacket.groupID << " " << oNewRedPacket.srcUserID << " " << oNewRedPacket.dstUserID << " ";
			newPacketCond << oNewRedPacket.packetType << " " << oNewRedPacket.rangeType << " " << oNewRedPacket.packetNum << " ";
			newPacketCond << oNewRedPacket.fixPerMoney << " " << oNewRedPacket.packetMoney;

			LOG_PRINT(log_debug, "send userid:%u,old packet condition:%s,new packet condition:%s.", send_user_id, oldPacketCond.str().c_str(), newPacketCond.str().c_str());

			if (strcmp(oldPacketCond.str().c_str(), newPacketCond.str().c_str()) != 0 || oNewRedPacket.createTime - oRecentRedPacket.createTime > CRedPacketMgr::m_comboTime)
			{
				LOG_PRINT(log_info, "[RedPacket Combo]condition is not equal or is not in combo time.old packet time:%u,new packet time:%u,comboCheckTime in config:%u.", \
					oRecentRedPacket.createTime, oNewRedPacket.createTime, CRedPacketMgr::m_comboTime);
				break;
			}
			
			if (field_value_map.find(CFieldName::REDPACKET_COMBONUM) == field_value_map.end())
			{
				LOG_PRINT(log_warning, "cannot find combo number in redis,send userid:%u,packetID:%u.", send_user_id, redpacketID);
				break;
			}

			ret = atoi(field_value_map[CFieldName::REDPACKET_COMBONUM].c_str()) + 1;
		}

	} while (0);

	return ret;
}

int CRedPacketMgr::sendRedPacketCommHandle(stRedPacketInfo & oRedPacketInfo, Json::Value & oPacketJson, unsigned int & packetID, unsigned int & comboNum, bool isPrivate)
{
	if (oRedPacketInfo.packetMoney < oRedPacketInfo.packetNum)
	{
		LOG_PRINT(log_error, "packetMoney(%u) < packetNum(%u)", oRedPacketInfo.packetMoney, oRedPacketInfo.packetNum);
		return ERR_CODE_REDPACKET_MIN_AMOUNT_LIMIT;
	}

	if (eDirectType != oRedPacketInfo.packetType && eVisitorType == oRedPacketInfo.rangeType)
	{
		int swtch = 0;
		if (CGroupBasicInfo::getValue(oRedPacketInfo.groupID, CGroupBasicInfo::e_Field_Alow_Visit_Group, swtch) && 0 == swtch)
		{
			LOG_PRINT(log_error, "range type is eVisitorType when allow_visit_group switch is off, group:%u", oRedPacketInfo.groupID);
			return ERRR_CODE_REDPACKET_VISITOR_RANGE_LIMIT;
		}
	}

	if (0 == oRedPacketInfo.fixTime)
	{
		oRedPacketInfo.fixTime = oRedPacketInfo.createTime;
	}

	if (!m_dbconn)
	{
		LOG_PRINT(log_error, "send red packet input error.m_dbconn is null,should be initial first.");
		return ERR_CODE_REDPACKET_SEND;
	}
	//设置红包失效时间
	setInvalidTime(oRedPacketInfo);
	unsigned int ntime_begin = 0;
	unsigned int ntime_end = 0;
	ntime_begin = SL_Socket_CommonAPI::util_process_clock_ms();
	int errorcode = 0;
	bool needRollBack = false;
	int total_row = 0;
	Result res;
	unsigned int redPacketID = 0;
	unsigned int total_contribute = 0;
	unsigned int week_contribute = 0;
	unsigned int combo_number = 0;
	do 
	{
		m_dbconn->transBegin();
		needRollBack = true;
		std::string strFunSQL = CDBTools::buildSendRedPacketSQL(oRedPacketInfo, m_goldContributeExchange);

		//校验用户账号金币,成功后返回红包ID
		if (!m_dbconn->build_sql_run(&res, &total_row, strFunSQL.c_str()))
		{
			errorcode = ERR_CODE_REDPACKET_SEND;
			break;
		}

		int ret = 0;
		m_dbconn->GetFromRes(ret, &res, 0, 0);
		if (ret < 0)
		{
			LOG_PRINT(log_error, "this user has not enough coin to send this red packet.userid:%u.", oRedPacketInfo.srcUserID);
			errorcode = ERR_CODE_FAILED_NOT_ENOUGH_GOLD;
			break;
		}
		m_dbconn->CleanRes(&res);
		if (!ret)
		{
			LOG_PRINT(log_error, "Error red packetID is 0.", ret);
			errorcode = ERR_CODE_REDPACKET_SEND;
			break;
		}
		redPacketID = ret;

		//如果入群成功但是红包发送失败,只能通过踢人出群了
		//如果是入群红包先要必须入群成功, 不然红包发送要失败
		if (eJoinGroupType == oRedPacketInfo.packetType)
		{
			combo_number = 1;
			if (0 != callJoinGroupRPC(oRedPacketInfo.srcUserID, oRedPacketInfo.groupID))
			{
				LOG_PRINT(log_error, "Error send red packet ID:%u.because rpc call RoomSvr fail.", redPacketID);
				errorcode = ERR_CODE_REDPACKET_SEND;
				break;
			}
		}
		else
		{
			combo_number = getComboNumber(oRedPacketInfo.srcUserID, oRedPacketInfo);
		}

		//获取红包标识
		unsigned int luckynum=CDBTools::select_luckynum_by_packetid(redPacketID);
		LOG_PRINT(log_info, "luckynum:[%d],redPacketID:[%d]", luckynum,redPacketID);

		oPacketJson["packetID"]	= Json::Value(redPacketID);
		oPacketJson["comboNum"]	= Json::Value(combo_number);
		oPacketJson["luckyNum"]	= Json::Value(luckynum);
		Json::FastWriter fast_writer;
		std::string strJson = fast_writer.write(oPacketJson);
		LOG_PRINT(log_info, "send to chat server,strJson:[%s]", strJson.c_str());
		if (strJson.empty())
		{
			LOG_PRINT(log_error, "Error red packetID:%u,json:%s.", redPacketID, strJson.c_str());
			errorcode = ERR_CODE_REDPACKET_SEND;
			break;
		}

		if (0 != callSendChatRPC(oRedPacketInfo.srcUserID, oRedPacketInfo.dstUserID, oRedPacketInfo.groupID, strJson, isPrivate))
		{
			LOG_PRINT(log_error, "Error send red packet ID:%u.because rpc call ChatSvr fail,json:%s.", redPacketID, strJson.c_str());
			errorcode = ERR_CODE_REDPACKET_SEND;
			break;
		}

		needRollBack = false;
		LOG_PRINT(log_info, "send red packet successful,userid:%u,packetID:%u,packetType:%u,groupID:%u,total contribute value:%u,week contribute value:%u.", \
			oRedPacketInfo.srcUserID, redPacketID, oRedPacketInfo.packetType, oRedPacketInfo.groupID, total_contribute, week_contribute);

	} while (0);

	m_dbconn->CleanRes(&res);
	if (needRollBack) 
	{
		LOG_PRINT(log_info, "send red packet failed rollback,errcode:%d", errorcode);
		m_dbconn->transRollBack();
	}
	else
	{
		LOG_PRINT(log_info, "send red packet success commit.packetID:%u.", redPacketID);
		m_dbconn->transCommit();
		packetID = redPacketID;
		comboNum = combo_number;
	}

	if (errorcode == 0 && packetID)
	{
		LOG_PRINT(log_info, "load red packet info to redis.packetID:%u.", redPacketID);
		reloadPacket2Redis(redPacketID);

		std::map<string, string> fields_vals;
		fields_vals[CFieldName::REDPACKET_COMBONUM] = bitTostring(combo_number);
		fields_vals[CFieldName::REDPACKET_RECENT_ID] = bitTostring(redPacketID);
		m_redpacket_senduser.insert(oRedPacketInfo.srcUserID, fields_vals);
		m_redpacket_senduser.expireat(oRedPacketInfo.srcUserID, oRedPacketInfo.createTime + CRedPacketMgr::m_comboTime * 2);
	}
	ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();
	LOG_PRINT(log_info, "[send red packet]send userID:%u,groupID:%u,packetType:%u takes time:%u(ms)", oRedPacketInfo.srcUserID, oRedPacketInfo.groupID, oRedPacketInfo.packetType, ntime_end - ntime_begin);
	return errorcode;
}

int CRedPacketMgr::checkUserValid(unsigned int userID, unsigned int groupID, unsigned int & roleType)
{
	RoomObject_Ref oGroupObj = ConsumeApp::getInstance()->m_room_mgr->FindRoom(groupID);
	if(oGroupObj.get() == 0) 
	{
		LOG_PRINT(log_error, "checkUserValid input error.not found group.userid:%u,groupid:%u.", userID, groupID);
		return ERR_CODE_FAILED_ROOMIDNOTFOUND;
	}

	unsigned int iRoleType = e_VisitorRole;
	CUserGroupinfo::getUserRoleType(userID, groupID, iRoleType);

	if (iRoleType == e_VisitorRole && !oGroupObj->findVisitUser(userID))
	{
		LOG_PRINT(log_error, "checkUserValid error.user is not group member and still not exist in visitor list.userid:%u,groupid:%u.", userID, groupID);
		return ERR_CODE_FAILED_USERNOTFOUND;
	}

	roleType = iRoleType;
	return ERR_CODE_SUCCESS;
}

int CRedPacketMgr::checkRedPacketCanTake(unsigned int userID, unsigned int groupID, unsigned int packetID, double	lg, double	la, CMDErrCode *pErr/* = NULL*/)
{
	if (!packetID || !userID || !groupID)
	{
		LOG_PRINT(log_error, "check redpacket error,input userID:%u,groupID:%u,packetID:%u.", userID, groupID, packetID);
		return ERR_CODE_REDPACKET_CATCH;
	}

	LOG_PRINT(log_debug, "check if this redpacket can take.userID:%u,groupID:%u,packetID:%u.", userID, groupID, packetID);
	unsigned int iRoleType = e_VisitorRole;
	int ret = checkUserValid(userID, groupID, iRoleType);
	if (ret != ERR_CODE_SUCCESS)
	{
		return ret;
	}

	do 
	{
		if (m_redpacket_users.find(packetID, userID))
		{
			LOG_PRINT(log_warning, "this redpacket has been taken by this user.userID:%u,groupID:%u,packetID:%u.", userID, groupID, packetID);
			return ERR_CODE_REDPACKET_HAS_TAKEN;
		}

		std::map<std::string, std::string > oFieldValueRedisMap;
		IF_METHOD_FALSE_BREAK_EX(m_redpacket_hash.getall(packetID, oFieldValueRedisMap), "get redpacket:%u info from redis fail.", packetID);
		if (oFieldValueRedisMap.empty())
		{
			break;
		}
		else
		{
			return checkRedPacketValid(userID, iRoleType, packetID, oFieldValueRedisMap, lg, la, pErr);
		}

	} while (0);

	std::map<std::string, std::string > oFieldValueDBMap;
	reloadPacket2Redis(packetID, oFieldValueDBMap);
	if (oFieldValueDBMap.empty())
	{
		//DB failed
		LOG_PRINT(log_error, "reload redpacket info from db fail.userID:%u,groupID:%u,packetID:%u.", userID, groupID, packetID);
		return ERR_CODE_REDPACKET_CATCH;
	}
	else
	{
		return checkRedPacketValid(userID, iRoleType, packetID, oFieldValueDBMap, lg, la, pErr);
	}
}

void CRedPacketMgr::reloadPacket2Redis(unsigned int packetID)
{
	std::map<std::string, std::string > oFieldValueDBMap;
	reloadPacket2Redis(packetID, oFieldValueDBMap);
}

void CRedPacketMgr::reloadPacket2Redis(unsigned int packetID, std::map<std::string, std::string > & oFieldValueDBMap)
{
	oFieldValueDBMap.clear();
	stRedPacketInfo oRedPacketInfo;
	int ret = CDBTools::getRedPacketBasicInfo(packetID, oRedPacketInfo);
	if (ret < 0)
	{
		//DB failed or has no basic info in DB
		LOG_PRINT(log_error, "DB query fail or DB has not this packet info.packetID:%u.", packetID);
		return;
	}

	genPacketObj2FieldValue(oRedPacketInfo, oFieldValueDBMap);

	if (time(NULL) > oRedPacketInfo.createTime + m_cacheTime)
	{
		LOG_PRINT(log_warning, "this packet do not need to load to cache.packetID:%u,packet createtime:%u.", oRedPacketInfo.packetID, oRedPacketInfo.createTime);
		return;
	}
	
	unsigned int expirationTime = oRedPacketInfo.createTime + m_cacheTime;	//redis过期时间
	m_redpacket_hash.insert(packetID, oFieldValueDBMap);
	m_redpacket_hash.expireat(packetID, expirationTime);

	if (!oRedPacketInfo.takeNum)
		return;

	std::vector<stRedPacketLog> vLog;
	ret = CDBTools::getRedPacketLog(packetID, vLog);
	if (vLog.size())
	{
		for (int i = 0; i < vLog.size(); i++)
		{
			m_redpacket_users.insert(packetID, vLog[i].userId, vLog[i].money);
		}
		m_redpacket_users.expireat(packetID, expirationTime);
	}
}

int CRedPacketMgr::takeRedPacket(unsigned int userID, unsigned int groupID, unsigned int packetID, unsigned int & takeMoney, stRedPacketInfo &packInfo, double lg, double la, CMDErrCode *pErr/* = NULL*/, const string&strmsg/* = ""*/)
{
	takeMoney = 0;
	if (!packetID || !userID || !groupID)
	{
		LOG_PRINT(log_error, "check redpacket error,input userID:%u,groupID:%u,packetID:%u.", userID, groupID, packetID);
		return ERR_CODE_REDPACKET_HAS_NOLEFT;
	}

	LOG_PRINT(log_debug, "[takeRedPacket]userID:%u,groupID:%u,packetID:%u.", userID, groupID, packetID);
	unsigned int iRoleType = e_VisitorRole;
	int ret = checkUserValid(userID, groupID, iRoleType);
	if (ret != ERR_CODE_SUCCESS)
	{
		return ret;
	}

	bool reLoad = true;
	std::map<std::string, std::string > oFieldValueRedisMap;
	do 
	{
		IF_METHOD_FALSE_BREAK_EX(m_redpacket_hash.getall(packetID, oFieldValueRedisMap), "get redpacket info from redis fail.userID:%u,groupID:%u,packetID:%u.", userID, groupID, packetID);
		if (!oFieldValueRedisMap.empty())
		{
			LOG_PRINT(log_debug, "redpacket info is in redis.userID:%u,groupID:%u,packetID:%u.", userID, groupID, packetID);
			reLoad = false;
		}

	} while (0);

	if (reLoad)
	{
		reloadPacket2Redis(packetID, oFieldValueRedisMap);
	}

	if (oFieldValueRedisMap.empty())
	{
		//DB failed or DB has not this packet info
		return ERR_CODE_REDPACKET_NOT_FOUND;
	}

	ret = checkRedPacketValid(userID, iRoleType, packetID, oFieldValueRedisMap, lg, la, pErr, strmsg);
	if (ret != ERR_CODE_SUCCESS)
	{
		return ret;
	}


	//获取用户收入分成比例
	uint32 gainPercent = 0;
	//CDBTools::getGroupGainPercent(groupID, userID, gainPercent);

	ret = CDBTools::getRedPacketBasicInfo(packetID, packInfo);
	unsigned int packetType = packInfo.packetType;

	CDBTools::transBegin();
	if (0 == ret)
	{
		switch(packetType)
		{
		case eLuckType:
		case eLigntType:
		case eJoinGroupType:
		case eFixTimeType:
		case eLocationType:
		case eCommandImmediatelyType:
		case eCommandFixTimeType:
			{
				ret = getRandomPacket(packInfo);
				if (ret > 0)
				{
					ret = CDBTools::takeRedPacket(packetID, packInfo.srcUserID, packetType, groupID, userID, ret, gainPercent, m_goldCandyExchange);
					//ret = CDBTools::takeRandomRedPacket(packetID, packetType, groupID, userID, ret, gainPercent, m_goldCandyExchange);
				}
			}
			break;
		case eUniType:
		case eCommandUniImmediatelyType:
		case eCommandUniFixTimeType:
			{
				ret = CDBTools::takeRedPacket(packetID, packInfo.srcUserID, packetType, groupID, userID, packInfo.fixPerMoney, gainPercent, m_goldCandyExchange);
				//ret = CDBTools::takeUniRedPacket(packetID, packetType, groupID, userID, gainPercent, m_goldCandyExchange);
			}
			break;
		case eDirectType:
			{
				ret = CDBTools::takeRedPacket(packetID, packInfo.srcUserID, packetType, groupID, userID, packInfo.packetMoney, gainPercent, m_goldCandyExchange);
				//ret = CDBTools::takeDirectRedPacket(packetID, packetType, groupID, userID, gainPercent, m_goldCandyExchange);
			}
			break;
		default:
			break;
		}
	}

	switch (ret)
	{
	case -1:
		LOG_PRINT(log_error, "DB take red packet error.userID:%u,groupID:%u,packetID:%u,packetType:%u.", userID, groupID, packetID, packetType);
		ret = ERR_CODE_FAILED_DBERROR;
		break;
	case -2:
		LOG_PRINT(log_warning, "user has taken this red packet before.userID:%u,groupID:%u,packetID:%u,packetType:%u.", userID, groupID, packetID, packetType);
		ret = ERR_CODE_REDPACKET_HAS_TAKEN;
		break;
	case -3:
		LOG_PRINT(log_error, "red packet has no left money to take.userID:%u,groupID:%u,packetID:%u,packetType:%u.", userID, groupID, packetID, packetType);
		m_redpacket_hash.insert(packetID, CFieldName::REDPACKET_MONEYSTATE, bitTostring((int)eNoLeft));
		ret = ERR_CODE_REDPACKET_HAS_NOLEFT;
		break;
	default:
		takeMoney = ret;
		ret = 0;
	}

	if (0 == ret)
	{
		/*
		红包领取提示 REMIND_TAKE_REDPACKET：
		{"remind_type": 0, "user_id": 123456, "alias": 昵称, "redpacket_id": 12}
		*/
		Json::Value value;
		value["redpacket_id"] = packetID;
		value["remind_type"] = REMIND_TAKE_REDPACKET;
		value["user_id"] = packInfo.srcUserID;
		value["alias"] = CUserBasicInfo::getUserAlias(packInfo.srcUserID);
		value["takeuser_id"] = userID;
		value["takeuser_alias"] = CUserBasicInfo::getUserAlias(userID);
		value["packet_type"] = packetType;
		//(packInfo.srcUserID != userID ? pDackInfo.srcUserID: 0)  userID 领取者
		//if (userID!=packInfo.srcUserID)
		//{
			callSendChatRPC(packInfo.srcUserID , userID, groupID, Json::FastWriter().write(value), packInfo.isPrivate, MSGTYPE_REMIND); //提示红包领取者
		//}
		
		
		LOG_PRINT(log_info, "userID:%d,packInfo.srcUserID:%d,groupID[%d]", userID, packInfo.srcUserID,groupID);
		/*if (userID != packInfo.srcUserID)
		{
			/ *
			红包被领取提示 REMIND_BE_TAKEN_REDPACKET：
			{"remind_type": 1, "user_id": 123456, "alias": 昵称, "redpacket_id": 12}
			* /
			value["remind_type"] = REMIND_BE_TAKEN_REDPACKET;
			value["user_id"] = userID;
			value["alias"] = CUserBasicInfo::getUserAlias(userID);
			callSendChatRPC( userID, packInfo.srcUserID, groupID, Json::FastWriter().write(value), packInfo.isPrivate, MSGTYPE_REMIND);	//别人领取红包,红包发起者被提示
		}*/
	}

	if (0 == ret)
	{
		CDBTools::transCommit();
		LOG_PRINT(log_info, "red packet has been taken successfully.userID:%u,groupID:%u,packetID:%u,packetType:%u,takeMoney:%d.", userID, groupID, packetID, packetType, ret);
		m_redpacket_users.insert(packetID, userID, takeMoney);
		m_redpacket_users.expireat(packetID, packInfo.createTime + m_cacheTime);
		if (packInfo.takeNum + 1 >= packInfo.packetNum || packInfo.takeMoney + takeMoney/m_goldCandyExchange >= packInfo.packetMoney)
			m_redpacket_hash.insert(packetID, CFieldName::REDPACKET_MONEYSTATE, bitTostring((int)eNoLeft));

		/*if (packInfo.srcUserID != userID)
		{
			if (packInfo.packetType != eJoinGroupType)
			{
				notifyWeekList(packInfo.srcUserID, groupID);
			}

			if (packInfo.packetType == eDirectType)
			{
				notifyWeekList(userID, groupID, true);
			}
		}*/
	}
	else
	{
		CDBTools::transRollBack();
	}

	return ret;
}

bool CRedPacketMgr::sendRedPacketInfo(task_proc_data &message, uint32 userID, uint32 packetID, uint32 groupID, uint32 count /*= 20*/, uint32 offset /*= 0*/)
{
	if (count > 20) count = 20;

	CMDRedPacketInfo info;
	if (!getRedPacketInfo(info, packetID, groupID, count, offset))
	{
		LOG_PRINT(log_error, "getRedPacketInfo error: packet[%u] group[%u].", packetID, groupID);
		return false;
	}
	
	//本人金额
	uint32 money = 0;
	if (m_redpacket_users.find(packetID, userID, money))
	{
		info.set_mytakemoney(money);
	}
	else
	{
		info.set_mytakemoney((unsigned int)CDBTools::getUserTakeMoney(userID, packetID));
	}

	unsigned int rspDataLen = info.ByteSize();
	SL_ByteBuffer buff(SIZE_IVM_HEAD_TOTAL + rspDataLen);
	buff.data_end(buff.buffer_size());
	memcpy(buff.buffer(), message.pdata, SIZE_IVM_HEAD_TOTAL);
	COM_MSG_HEADER * pRspHead = (COM_MSG_HEADER *)buff.buffer();
	pRspHead->length = buff.buffer_size();
	pRspHead->subcmd = Sub_Vchat_RedPacketInfo;
	info.SerializeToArray(buff.buffer() + SIZE_IVM_HEAD_TOTAL, rspDataLen);
	message.sendMessage(buff.buffer(), buff.buffer_size());
	
	LOG_PRINT(log_info, "sendRedPacketInfo: user[%u] packet[%u] group[%u] size[%u].", userID, packetID, groupID, info.res_size());
	return true;
}

bool CRedPacketMgr::getRedPacketInfo(CMDRedPacketInfo & info, uint32 packetID, uint32 groupID, uint32 count /*= 20*/, uint32 offset /*= 0*/)
{
	stRedPacketInfo packetInfo;
	if (CDBTools::getRedPacketBasicInfo(packetID, packetInfo) < 0 || groupID != packetInfo.groupID)
	{
		LOG_PRINT(log_error, "can not find red packet basic info: group: %u, packetid: %u", groupID, packetID);
		return false;
	}

	info.set_packetid(packetInfo.packetID);
	info.set_packettype((ePacketType)packetInfo.packetType);
	info.set_rangetype((eRangeType)packetInfo.rangeType);
	info.set_packetnum(packetInfo.packetNum);
	info.set_takennum(packetInfo.takeNum);
	info.set_packetmoney(packetInfo.packetMoney);
	info.set_takenmoney(packetInfo.takeMoney);
	info.set_message(packetInfo.strMessage);
	info.set_createtime(packetInfo.createTime);

	if (packetInfo.takeNum >= packetInfo.packetNum || packetInfo.takeMoney >= packetInfo.packetMoney || packetInfo.isRecycle)
	{
		if (packetInfo.finishTime > packetInfo.createTime)
			info.set_finishcosttime(packetInfo.finishTime - packetInfo.createTime);
		info.set_topuserid(CDBTools::getTopRedPacketTakeUserID(packetID));
	}
	
	std::string strHead = "";
	std::string strAlias = "";
	UserInfo_t * pSrcUser = info.mutable_srcuser();
	pSrcUser->set_userid(packetInfo.srcUserID);
	CUserBasicInfo::getUserHeadAndAlias(packetInfo.srcUserID, strHead, strAlias);
	pSrcUser->set_head(strHead);
	pSrcUser->set_alias(strAlias);

	UserInfo_t * pDstUser = info.mutable_dstuser();
	pDstUser->set_userid(packetInfo.dstUserID);
	CUserBasicInfo::getUserHeadAndAlias(packetInfo.dstUserID, strHead, strAlias);
	pDstUser->set_head(strHead);
	pDstUser->set_alias(strAlias);

	vector<stRedPacketLog> vLog;
	CDBTools::getRedPacketLog(packetID, vLog, count, offset);
	info.set_offset(offset + vLog.size());
	for (unsigned int i = 0; i < vLog.size(); i++)
	{
		CMDRedPacketTakenRes *pRes = info.add_res();
		pRes->set_userid(vLog[i].userId);
		pRes->set_money(vLog[i].money);/*客户端显示的时候除了100*/
		pRes->set_taketime(vLog[i].time);
		std::string strHead, strAlias;
		if (CUserBasicInfo::getUserHeadAndAlias(pRes->userid(), strHead, strAlias))
		{
			pRes->set_head(strHead);
			pRes->set_alias(strAlias);
		}
	}

	return true;
}

void CRedPacketMgr::getWeekList(unsigned int userid, unsigned int groupid, CMDWeekListNotify & oNotyData, bool & bNeedNotify, bool bCharmList/* = false*/)
{
	oNotyData.set_groupid(groupid);
	bNeedNotify = false;
	std::list<stContributeValue> oList;
	bool ret = (bCharmList ? CDBTools::getWeekCharmUserLst(groupid, oList): 
							CDBTools::getWeekContributeUserLst(groupid, oList));
	if (!ret)
	{
		return;
	}

	std::list<stContributeValue>::iterator iter_lst = oList.begin();
	for (; iter_lst != oList.end(); ++iter_lst)
	{
		CMDWeekListRes * pUser = oNotyData.add_res();
		pUser->set_userid(iter_lst->userID);
		pUser->set_contributevalue(iter_lst->near_week_contribute);
		std::string strHead = "";
		std::string strAlias = "";
		CUserBasicInfo::getUserHeadAndAlias(iter_lst->userID, strHead, strAlias);
		pUser->set_head(strHead);
		pUser->set_alias(strAlias);
		if (iter_lst->userID == userid)
		{
			bNeedNotify = true;
		}
	}
}

long long CRedPacketMgr::getRandomPacket(const stRedPacketInfo &packInfo)
{
	if (packInfo.packetMoney <= packInfo.takeMoney || packInfo.packetNum <= packInfo.takeNum)
	{
		return -3;
	}

	long long leftMoney = packInfo.packetMoney - packInfo.takeMoney;
	unsigned int leftNum = packInfo.packetNum - packInfo.takeNum;
	
	if (leftMoney < leftNum)
		return (leftMoney ? 1: 0);

	if (1 == leftNum)
	{
		return leftMoney;
	}

	long long money = rand() % ((leftMoney / leftNum) * 2);

	if (money == 0) money = 1;
	
	return money;
}

void CRedPacketMgr::recycleRedPacket()
{
	std::vector<stRedPacketInfo> vList;
	if (CDBSink().getRedPacketToRecycle(m_validtime, vList) > 0)
	{
		LOG_PRINT(log_info, "get redpacket to recycle, size: %u", vList.size());
		for (unsigned int i = 0; i < vList.size(); i++)
		{
			int result;
			if (CDBSink().recycleOneRedPacket(vList[i].packetID, m_goldCandyExchange, result))
			{
				if (result <= 0)
				{
					const char *error = "";
					switch (result)
					{
					case -1: error = "input packet_id is invalid"; break;
					case -2: error = "can not find red packet"; break;
					case -3: error = "red packet has been recycled yet"; break;
					default: error = "unknown"; break;
					}
					LOG_PRINT(log_info, "recycle redpacket[%u], result[%s].", vList[i].packetID, error);
				}
				else
				{
					CThriftHttpClient<TChatSvrConcurrentClient> rpcClient(e_roomadapter_type);
					if (rpcClient)
					{
						TGroupAssistMsg assistMsg;
						assistMsg.userID = vList[i].srcUserID;
						assistMsg.groupID = vList[i].groupID;
						assistMsg.svrType = MSGTYPE_REDPACK_BALANCE;
						assistMsg.msgType = ASSISTMSG_SYSTEM;
						assistMsg.packetID = vList[i].packetID;
						assistMsg.balance = (uint32)result;
						if (!rpcClient->proc_addGroupAssistMsg(assistMsg, false, true))
						{
							LOG_PRINT(log_warning, "proc_addGroupAssistMsg error, redpacket[%u], balance[%d].", vList[i].packetID, result);
						}
					}
					else
					{
						LOG_PRINT(log_error, "add group assistmsg rpc error, cannot find ChatSvr. redpacket[%u].", vList[i].packetID);
					}
					//TODO group assist
					LOG_PRINT(log_info, "recycle redpacket[%u], balance[%d].", vList[i].packetID, result);
				}
			}
			else
			{
				LOG_PRINT(log_info, "recycle redpacket[%u] failed.", vList[i].packetID);
			}
		}
	}

}

void CRedPacketMgr::cleanRedPacketCache(unsigned int now_time)
{
	LOG_PRINT(log_info, "[cleanRedPacketCache]it is time to clean redpacket cache in redis.now_time:%u.", now_time);
	std::map<std::string, std::string > value_score_map;
	m_redpacket_ids.getByPosValue(0, 200, value_score_map);
	if (value_score_map.empty())
	{
		return;
	}

	int minCreateTime = 0;
	int maxCreateTime = 0;
	std::map<std::string, std::string >::iterator iter_map = value_score_map.begin();
	for (; iter_map != value_score_map.end(); ++iter_map)
	{
		int redpacketID = atoi(iter_map->first.c_str());
		int createTime = atoi(iter_map->second.c_str());
		if (now_time - createTime > m_cacheTime)
		{
			LOG_PRINT(log_debug, "[cleanRedPacketCache]need to clean old redpacket info in cache.redpacketID:%d,createtime:%d.cacheTime:%u.", redpacketID, createTime, m_cacheTime);
			IF_METHOD_FALSE_CONTINUE_EX(m_redpacket_hash.erase(redpacketID), "erase redpacket:%d in cache fail.", redpacketID);
			IF_METHOD_FALSE_CONTINUE_EX(m_redpacket_users.erase(redpacketID), "erase redpacket_user:%d in cache fail.", redpacketID);

			if (minCreateTime == 0 || minCreateTime > createTime)
			{
				minCreateTime = createTime;
			}

			if (maxCreateTime < createTime)
			{
				maxCreateTime = createTime;
			}
		}

	}

	if (minCreateTime && maxCreateTime)
	{
		if (m_redpacket_ids.erase(minCreateTime, maxCreateTime))
		{
			LOG_PRINT(log_info, "[cleanRedPacketCache]clean old redpacket info in cache success.minCreateTime:%d.maxCreateTime:%d.", minCreateTime, maxCreateTime);
		}
		else
		{
			LOG_PRINT(log_error, "[cleanRedPacketCache]clean old redpacket info in cache fail.minCreateTime:%d.maxCreateTime:%d.", minCreateTime, maxCreateTime);
		}
		
	}
}

unsigned int CRedPacketMgr::callJoinGroupRPC(unsigned int userid, unsigned int groupid)
{
	LOG_PRINT(log_info, "callJoinGroupRPC input,userid:%u,groupid:%u.", userid, groupid);
	unsigned int errorcode = 0;
	CThriftHttpClient<TRoomSvrConcurrentClient> rpcClient(e_roomsvr_type);
	if (rpcClient)
	{
		TJoinGroup joinMsg;
		joinMsg.action = 1;
		joinMsg.userid = userid;
		joinMsg.groupid = groupid;
		joinMsg.inway = e_JoinGroupAuthType::RedPacket;
		if (rpcClient->proc_joingroup(joinMsg) != 0)
		{
			LOG_PRINT(log_error, "Error callJoinGroupRPC.because RoomSvr return join group fail,userid:%u,groupid:%u.", userid, groupid);
			errorcode = ERR_CODE_FAILED_RPCCALL;
		}
	}
	else
	{
		LOG_PRINT(log_error, "Error callJoinGroupRPC.because not find RoomSvr to join group,userid:%u,groupid:%u.", userid, groupid);
		errorcode = ERR_CODE_FAILED_RPCSVR_NOTFOUND;
	}

	return errorcode;
}

unsigned int CRedPacketMgr::callSendChatRPC(unsigned int src_userid, unsigned int dst_userid, unsigned int groupid, const std::string & chatmsg, bool isPrivate, uint32 msgtype/* = MSGTYPE_REDPACK*/)
{
	unsigned int errorcode = 0;
	CThriftHttpClient<TChatSvrConcurrentClient> rpcClient(e_roomadapter_type);
	if (rpcClient)
	{
		TChatMsg chatMsg;
		chatMsg.srcUId = src_userid;
		chatMsg.dstUId = dst_userid;
		chatMsg.msgType = msgtype;
		chatMsg.content = chatmsg;
		TChatMsgExtra extra;
		extra.isPrivate = isPrivate;
		if (!rpcClient->procGroupChatReq(groupid, chatMsg, extra))
		{
			LOG_PRINT(log_error, "Error callSendChatRPC.because ChatSvr return send msg fail,src_userid:%u,dst_userid:%u,groupid:%u,chatmsg:%s,private:%d.", \
				src_userid, dst_userid, groupid, chatmsg.c_str(), (int)isPrivate);
			errorcode = ERR_CODE_FAILED_RPCCALL;
		}
	}
	else
	{
		LOG_PRINT(log_error, "Error callSendChatRPC.because not find ChatSvr to send msg,src_userid:%u,dst_userid:%u,groupid:%u,chatmsg:%s,private:%d.", \
			src_userid, dst_userid, groupid, chatmsg.c_str(), (int)isPrivate);
		errorcode = ERR_CODE_FAILED_RPCSVR_NOTFOUND;
	}

	return errorcode;
}


void CRedPacketMgr::setInvalidTime(stRedPacketInfo &oRedPacketInfo)
{
	if (eFixTimeType == oRedPacketInfo.packetType || (eLocationType == oRedPacketInfo.packetType && oRedPacketInfo.fixTime > 0) )
	{
		oRedPacketInfo.invalidTime = oRedPacketInfo.fixTime + m_timer_redpacket_validtime;
	}
	else if (eCommandFixTimeType == oRedPacketInfo.packetType || eCommandUniFixTimeType == oRedPacketInfo.packetType)
	{
		oRedPacketInfo.invalidTime = oRedPacketInfo.invalidTime;
	}
	//	else if (eLigntType == oRedPacketInfo.packetType)
	//	{
	//		oRedPacketInfo.invalidTime = oRedPacketInfo.createTime + m_timer_redpacket_validtime;
	//	}
	else
	{
		oRedPacketInfo.invalidTime = oRedPacketInfo.createTime + m_validtime;
	}
}
