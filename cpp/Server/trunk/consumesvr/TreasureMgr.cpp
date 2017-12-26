#include "TreasureMgr.h"
#include "msgcommapi.h"
#include "CUserGroupinfo.h"
#include "utils.h"

std::map<std::string, int> CTreasureMgr::m_mBuyError;

void CTreasureMgr::procSponsorTreasureReq(task_proc_data &message)
{
	CMDSponsorTreasureReq req;
	if (!CMsgComm::ParseProtoMessage(message.pdata, message.datalen, req))
	{
		LOG_PRINT(log_warning, "parse [%s]  failed.", req.GetTypeName().c_str());
		return;
	}
	CMDTreasureInfo &info = *req.mutable_info();
	LOG_PRINT(log_info, "begin, user[%u] group[%u].", info.user().userid(), info.groupid());

	uint32 now = time(NULL);
	CMDSponsorTreasureResp resp;
    int errid = ERR_CODE_SUCCESS;
	do
	{
		if (!info.groupid() || !info.user().userid() 
			//开奖方式为平均分，但是获奖者是所有参与者
			|| (0 == info.winner_num() && e_LotteryType_RANDOM == info.lottery_type())
			//定额宝物，宝物金额不合法或者获奖者大于金额
			|| (e_TreasureType_Quota == info.treasure_type() && (info.treasure_amount() < 2000/*least TODO*/ || info.winner_num() > info.treasure_amount()))
			//定时宝物，开奖时间小于当前时间
			|| (e_TreasureType_Timer == info.treasure_type() && info.treasure_time() < now))
		{
			errid = ERR_CODE_INVALID_PARAMETER;
			break;
		}

		if (!CUserGroupinfo::chkUserRoleBeyond(info.user().userid(), info.groupid(), e_OwnerRole))
		{
			//只有群主才能发起夺宝
			errid = ERR_CODE_USER_HAS_NO_RIGHT;
			break;
		}

		if (!CDBTools::checkSponsorTreasure(info.groupid()))
		{
			//群宝物最大数量限制
			errid = ERR_CODE_MAX_TREASURE_NUM_LIMIT;
			break;			
		}
		
		std::string treasureTime = (e_TreasureType_Quota == info.treasure_type() ? "NULL": stringFormat("FROM_UNIXTIME(%u)", info.treasure_time()));
		std::string strSql = stringFormat(
				" insert into mc_treasure_info(group_id,user_id,join_range,treasure_type,treasure_amount,treasure_time,lottery_type,winner_num,message,status,create_time) "
				" values(%u, %u, %d, %d, %u, %s, %d, %u, '%s', %d, FROM_UNIXTIME(%u)) ",
				info.groupid(), info.user().userid(), info.join_range(), info.treasure_type(), info.treasure_amount(), treasureTime.c_str(), info.lottery_type(), info.winner_num(), info.message().c_str(), (int)e_TreasureStatus_Runing, now);

		CDBSink sink;
		if (!sink.run_sql(strSql.c_str()) || 0 == sink.getAffectedRow())
		{
			errid = ERR_CODE_FAILED_DBERROR;
			break;
		}
		info.set_treasure_id(sink.getInsertId());
		info.set_create_time(now);
	} while(0);
	resp.mutable_errinfo()->set_errid(errid);
	resp.mutable_info()->CopyFrom(info);
	LOG_PRINT(log_info, "end, result[%s], user[%u] group[%u] treasure[%u]",
			CErrMsg::strerror(errid).c_str(), info.user().userid(), info.groupid(), info.treasure_id());
	message.respProtobuf(resp, Sub_Vchat_SponsorTreasureResp);

    if (ERR_CODE_SUCCESS == errid)
    	notifyTreasureUpdate(info, e_TreasureStatus_None, e_TreasureStatus_Runing);
}

void CTreasureMgr::procBuyTreasure(task_proc_data &message)
{
	CMDBuyTreasure req;
	if (!CMsgComm::ParseProtoMessage(message.pdata, message.datalen, req))
	{
		LOG_PRINT(log_warning, "parse [%s]  failed.", req.GetTypeName().c_str());
		return;
	}
	LOG_PRINT(log_info, "begin, user[%u] group[%u] treasure[%u] buy_amount[%u]",
			req.userid(), req.groupid(), req.treasure_id(), req.buy_amount());

	CMDBuyTreasureResp resp;
	int errid = ERR_CODE_UNKNOWN_ERROR;
	do
	{
		if (!req.groupid() || !req.userid() || !req.treasure_id() || !req.buy_amount())
		{
			errid = ERR_CODE_INVALID_PARAMETER;
			break;
		}

		std::string result;
		//buy_treasure(user, group, treasure, buy_amount)
		std::string strSql = stringFormat(" select buy_treasure(%u, %u, %u, %u) ", req.userid(), req.groupid(), req.treasure_id(), req.buy_amount());
		Result res;
		int row = 0;
		CDBSink sink;
		if (!sink.run_sql(strSql.c_str(), &res, &row) || 0 == row)
		{
			LOG_PRINT(log_error, "BuyTreasure call false, DB ERROR.");
			sink.CleanRes(&res);
			errid = ERR_CODE_NOT_BUY_THIS_TREASURE;
			break;
		}
		sink.GetFromRes(result, &res, 0, 0);	
		sink.CleanRes(&res);

		//errid|begin_code|end_code|buy_time
		LOG_PRINT(log_info, "BuyTreasure result:[%s].", result.c_str());
		std::vector<std::string> vResult;
		SplitAndTrim(result.c_str(), '|', vResult);
		errid = (vResult.size() >= 1 ? getBuyError(vResult[0]): ERR_CODE_UNKNOWN_ERROR);
		if (ERR_CODE_SUCCESS != errid || vResult.size() < 4)
		{
			LOG_PRINT(log_error, "BuyTreasure Failed, result:[%s]", result.c_str());
			break;
		}

		CMDBuyCodeRange *pRange = resp.mutable_buy_code();
		pRange->set_begin(atoi(vResult[1].c_str()));
		pRange->set_end(atoi(vResult[2].c_str()));
		pRange->set_buy_time(atoi(vResult[3].c_str()));
	} while(0);
	resp.mutable_errinfo()->set_errid(errid);
	resp.mutable_buy_info()->CopyFrom(req);
	LOG_PRINT(log_info, "end, result[%s], user[%u] group[%u] treasure[%u] buy_amount[%u]",
			CErrMsg::strerror(resp.errinfo().errid()).c_str(), req.userid(), req.groupid(), req.treasure_id(), req.buy_amount());
	
	message.respProtobuf(resp, Sub_Vchat_BuyTreasureResp);
}

void CTreasureMgr::procQryMyTreasureDetail(task_proc_data &message)
{
	CMDQryMyTreasureDetail req;
	if (!CMsgComm::ParseProtoMessage(message.pdata, message.datalen, req))
	{
		LOG_PRINT(log_warning, "parse [%s]  failed.", req.GetTypeName().c_str());
		return;
	}
	LOG_PRINT(log_info, "begin, user[%u] group[%u] treasure[%u]", req.userid(), req.groupid(), req.treasure_id());
	
	CMDMyTreasureDetail resp;
	do
	{
		std::string strSql = stringFormat(" select buy_amount from mc_treasure_buy_log where treasure_id = %u and group_id = %u and user_id = %u ", 
                req.treasure_id(), req.groupid(), req.userid());
		Result res;
		int row = 0;
		CDBSink sink;
		if (!sink.run_sql(strSql.c_str(), &res, &row) || 0 == row)
		{
			sink.CleanRes(&res);
			break;
		}
		int value = 0;
		sink.GetFromRes(value, &res, 0, 0);	
		sink.CleanRes(&res);
		resp.set_buy_total(value);
		row = 0;

		strSql = stringFormat(" select begin_code, end_code, UNIX_TIMESTAMP(buy_time) from mc_treasure_buy_detail where treasure_id = %u and user_id = %u",
				req.treasure_id(), req.userid());
		if (!sink.run_sql(strSql.c_str(), &res, &row) || 0 == row)
		{
			sink.CleanRes(&res);
			break;
		}

		for (int i = 0; i < row; i++)
		{
			CMDBuyCodeRange *pRange = resp.add_buy_list();
			sink.GetFromRes(value, &res, i, 0);
			pRange->set_begin(value);
			sink.GetFromRes(value, &res, i, 1);
			pRange->set_end(value);
			sink.GetFromRes(value, &res, i, 2);
			pRange->set_buy_time(value);
		}
		sink.CleanRes(&res);
	} while(0);

	LOG_PRINT(log_info, "end, user[%u] group[%u] treasure[%u], buy_total[%u]",
			 req.userid(), req.groupid(), req.treasure_id(), resp.buy_total());
	message.respProtobuf(resp, Sub_Vchat_MyTreasureDetail);
}

void CTreasureMgr::procQryGroupTreasureList(task_proc_data &message)
{
	CMDQryTreasureList req;
	if (!CMsgComm::ParseProtoMessage(message.pdata, message.datalen, req))
	{
		LOG_PRINT(log_warning, "parse [%s]  failed.", req.GetTypeName().c_str());
		return;
	}
	
	LOG_PRINT(log_info, "begin, user[%u] group[%u] count[%u] last_create_time[%u] last_end_time[%u]",
			 req.userid(), req.groupid(), req.count(), req.last_create_time(), req.last_end_time());
	CMDTreasureList resp;
	qryTreasureList(req, resp, false);

	message.respProtobuf(resp, Sub_Vchat_GroupTreasureList);
	LOG_PRINT(log_info, "begin, user[%u] group[%u] count[%u] last_create_time[%u] last_end_time[%u] treasure_size[%u]",
			 req.userid(), req.groupid(), req.count(), req.last_create_time(), req.last_end_time(), resp.list_size());
}

void CTreasureMgr::procQryMyTreasureList(task_proc_data &message)
{
	CMDQryTreasureList req;
	if (!CMsgComm::ParseProtoMessage(message.pdata, message.datalen, req))
	{
		LOG_PRINT(log_warning, "parse [%s]  failed.", req.GetTypeName().c_str());
		return;
	}

	LOG_PRINT(log_info, "begin, user[%u] group[%u] count[%u] last_create_time[%u] last_end_time[%u]",
			 req.userid(), req.groupid(), req.count(), req.last_create_time(), req.last_end_time());
	CMDTreasureList resp;
	qryTreasureList(req, resp, true);

	message.respProtobuf(resp, Sub_Vchat_MyTreasureList);
	LOG_PRINT(log_info, "begin, user[%u] group[%u] count[%u] last_create_time[%u] last_end_time[%u] treasure_size[%u]",
			 req.userid(), req.groupid(), req.count(), req.last_create_time(), req.last_end_time(), resp.list_size());
}

void CTreasureMgr::procQryTreasureInfo(task_proc_data &message)
{
	CMDQryTreasureInfo req;
	if (!CMsgComm::ParseProtoMessage(message.pdata, message.datalen, req))
	{
		LOG_PRINT(log_warning, "parse [%s]  failed.", req.GetTypeName().c_str());
		return;
	}
	LOG_PRINT(log_info, "begin, user[%u] group[%u] treasure_id[%u]",
			 req.userid(), req.groupid(), req.treasure_id());

	CMDQryTreasureInfoResp resp;
	resp.mutable_req()->CopyFrom(req);
	int errid = ERR_CODE_SUCCESS;
	do
	{
		if (!req.groupid() || !req.userid() || !req.treasure_id())
		{
			errid = ERR_CODE_INVALID_PARAMETER;
			break;
		}
		std::string strSql = stringFormat(
			" select id,group_id,user_id,join_range,treasure_type,treasure_amount,UNIX_TIMESTAMP(treasure_time),buy_amount,"
			"        lottery_type,winner_num,message,status,UNIX_TIMESTAMP(create_time),UNIX_TIMESTAMP(end_time) "
			"   from mc_treasure_info ti where ti.id = %u and ti.group_id = %u", req.treasure_id(), req.groupid());
		::google::protobuf::RepeatedPtrField< ::CMDTreasureInfo > list;
		int ret = qryTreasureList(strSql, req.userid(), req.groupid(), list);
		if (ret < 0)
		{
			errid = ERR_CODE_FAILED_DBERROR;
			break;
		}
		else if (0 == list.size())
		{
			errid = ERR_CODE_TREASURE_NOT_EXISTS;
			break;
		}
		else
		{
			resp.mutable_info()->CopyFrom(list.Get(0));
		}
	} while(0);

	resp.mutable_errinfo()->set_errid(errid);
	message.respProtobuf(resp, Sub_Vchat_QryTreasureInfoResp);
	LOG_PRINT(log_info, "begin, result[%s] user[%u] group[%u] treasure_id[%u]",
			 CErrMsg::strerror(errid).c_str(), req.userid(), req.groupid(), req.treasure_id());
}

void CTreasureMgr::notifyTreasureUpdate(const CMDTreasureInfo &info, e_TreasureStatus oldStatus, e_TreasureStatus newStatus)
{
	CMDTreasureInfoNotify notify;
	notify.set_old_status(oldStatus);
	notify.set_new_status(newStatus);
	notify.mutable_info()->CopyFrom(info);

	int nProtoLen = notify.ByteSize();
	SL_ByteBuffer buff(SIZE_IVM_HEAD_TOTAL + nProtoLen);
	CMsgComm::Build_COM_MSG_HEADER(buff.buffer(), 0, Sub_Vchat_TreasureInfoNotify, SIZE_IVM_HEAD_TOTAL + nProtoLen);
	DEF_IVM_CLIENTGATE(pGateMask, buff.buffer());
	notify.SerializeToArray(buff.buffer() + SIZE_IVM_HEAD_TOTAL, nProtoLen);
	CMsgComm::Build_BroadCastRoomGate(pGateMask, e_Notice_AllType, info.groupid());
	ConsumeApp::getInstance()->m_room_mgr->castGateway((COM_MSG_HEADER *)buff.buffer());
	LOG_PRINT(log_info, "notifyTreasureUpdate: group[%u] treasure[%u] old[%d] new[%d].", info.groupid(), info.treasure_id(), (int)oldStatus, (int)newStatus);
}

int CTreasureMgr::getBuyError(const std::string &errinfo)
{
	initBuyError();
	std::map<std::string, int>::iterator iter = m_mBuyError.find(errinfo);
	if (iter != m_mBuyError.end())
	{
		return iter->second;
	}

	return ERR_CODE_UNKNOWN_ERROR;
}

void CTreasureMgr::initBuyError()
{
	if (m_mBuyError.empty())
	{
		m_mBuyError["SUCCESS"] = ERR_CODE_SUCCESS;	//成功
		m_mBuyError["NO_ENOUGH_BALANCE"] = ERR_CODE_FAILED_NOT_ENOUGH_GOLD;	//用户余额不足
		m_mBuyError["TREASURE_OPENED"] = ERR_CODE_BUY_TREASURE_OPENED;	//已开奖
		m_mBuyError["TREASURE_TIMEOUT"] = ERR_CODE_BUY_TREASURE_TIMEOUT;	//已到期
		m_mBuyError["MEMBER_ONLY"] = ERR_CODE_BUY_TREASURE_MEMBER_ONLY;	//仅成员可参与
		m_mBuyError["VISITOR_ONLY"] = ERR_CODE_BUY_TREASURE_VISITOR_ONLY;	//仅游客可参与
		m_mBuyError["DEFICIT"] = ERR_CODE_BUY_TREASURE_DEFICIT;	//宝物剩余不足
		m_mBuyError["TREASURE_NOT_EXISTS"] = ERR_CODE_TREASURE_NOT_EXISTS;	//不存在该宝物
	}
}

void CTreasureMgr::qryTreasureList(CMDQryTreasureList &req, CMDTreasureList &resp, bool qryUser/* = false*/)
{
	resp.set_groupid(req.groupid());
	resp.set_userid(req.userid());
	do
	{
		if (!req.userid() ||!req.groupid())
		{
			LOG_PRINT(log_error, "invalid parameter, user[%u] group[%u] qryUser[%d]", req.userid(), req.groupid(), (int)qryUser);
			break;
		}

		if (req.count() <= 0 || req.count() > 20)
		{
			req.set_count(20);
		}

		std::string prefix = stringFormat(
			" select id,group_id,user_id,join_range,treasure_type,treasure_amount,UNIX_TIMESTAMP(treasure_time),buy_amount,"
			"        lottery_type,winner_num,message,status,UNIX_TIMESTAMP(create_time),UNIX_TIMESTAMP(end_time) "
			"   from mc_treasure_info ti where ti.group_id = %u ", req.groupid());
		if (qryUser) 
		{
			prefix += stringFormat(" and exists(select 1 from mc_treasure_buy_log bl where ti.id = bl.treasure_id and bl.user_id = %u) ", req.userid());
		}

		std::string strSql = prefix;
		if (req.last_end_time())
		{
			strSql += stringFormat(" and ti.end_time < FROM_UNIXTIME(%u) order by ti.end_time desc limit %u", 
				req.last_end_time(), req.count());
		}
		else
		{

			if (!req.last_create_time())
			{
				strSql += stringFormat(" and ti.end_time is null order by create_time asc limit %u", req.count());
			}
			else
			{
				strSql += stringFormat(" and ti.end_time is null and ti.create_time > FROM_UNIXTIME(%u) order by create_time limit %u", 
						req.last_create_time(), req.count());
			}
		}
		int count = qryTreasureList(strSql, req.userid(), req.groupid(), *resp.mutable_list(), false);
		if (count >= 0 && count < req.count() && !req.last_end_time())
		{
			strSql = prefix + stringFormat(" and ti.end_time is not null order by ti.end_time desc limit %u", req.count() - count);
			qryTreasureList(strSql, req.userid(), req.groupid(), *resp.mutable_list(), false);
		}
	} while(0);
}

int CTreasureMgr::qryTreasureList(const std::string &strSql, uint32 userid, uint32 groupid, ::google::protobuf::RepeatedPtrField< ::CMDTreasureInfo > &list, bool qryUser/* = false*/)
{
	CDBSink sink;
	Result res;
	int row = 0;
	do
	{
		if (!sink.run_sql(strSql.c_str(), &res, &row))
		{
			LOG_PRINT(log_error, "QryGroupTreasureList error.");
			row = -1;
			break;
		}

		if (0 == row)
			break;

		std::map<uint32, CMDTreasureInfo *> mTreasure;
		for (int i = 0; i < row; i++)
		{
			CMDTreasureInfo *pInfo = list.Add();
			pInfo->set_treasure_id(sink.GetIntFromRes(&res, i, 0));
			pInfo->set_groupid(sink.GetIntFromRes(&res, i, 1));
			pInfo->mutable_user()->set_userid(sink.GetIntFromRes(&res, i, 2));
			pInfo->set_join_range((e_UserRange)sink.GetIntFromRes(&res, i, 3));
			pInfo->set_treasure_type((e_TreasureType)sink.GetIntFromRes(&res, i, 4));
			pInfo->set_treasure_amount(sink.GetIntFromRes(&res, i, 5));
			pInfo->set_treasure_time(sink.GetIntFromRes(&res, i, 6));
			pInfo->mutable_ext()->set_buy_amount(sink.GetIntFromRes(&res, i, 7));
			pInfo->set_lottery_type((e_LotteryType)sink.GetIntFromRes(&res, i, 8));
			pInfo->set_winner_num(sink.GetIntFromRes(&res, i, 9));
			pInfo->set_message(sink.GetStringFromRes(&res, i, 10));
			pInfo->mutable_ext()->set_status((e_TreasureStatus)sink.GetIntFromRes(&res, i, 11));
			pInfo->set_create_time(sink.GetIntFromRes(&res, i, 12));
			pInfo->mutable_ext()->set_finished_time(sink.GetIntFromRes(&res, i, 13));
            mTreasure[pInfo->treasure_id()] = pInfo;
		}

		qryTreasureBuyList(mTreasure);	//查询购买列表
		qryTreasureBuyList(mTreasure, userid);	//查询我购买的信息
	} while(0);

	return row;
}

bool CTreasureMgr::qryTreasureBuyList(std::map<uint32, CMDTreasureInfo *> &mTreasure, uint32 userid/* = 0*/)
{
	int num = 0;
	std::string strSql;	
	std::map<uint32, CMDTreasureInfo *>::iterator iter = mTreasure.begin();
	for (; iter != mTreasure.end(); iter++, num++)
	{
		if (strSql.length())
		{
			strSql += " UNION ALL ";
		}

		if (userid)
		{
			strSql += stringFormat(
					" SELECT * FROM "
					" (select treasure_id,user_id,buy_amount from mc_treasure_buy_log "
					"   where treasure_id = %u and user_id = %u order by buy_amount desc limit 5"
					" ) AS r%d ",
				iter->second->treasure_id(), userid, num);
		}
		else
		{
			strSql += stringFormat(
					" SELECT * FROM "
					" (select treasure_id,user_id,buy_amount from mc_treasure_buy_log "
					"   where treasure_id = %u order by buy_amount desc limit 5"
					" ) AS r%d ",
				iter->second->treasure_id(), num);
		}
	}

	CDBSink sink;
	Result res;
	int row = 0;
	if (!sink.run_sql(strSql.c_str(), &res, &row) || 0 == row)
	{
		return true;
	}

	for (int i = 0; i < row; i++)
	{
		std::map<uint32, CMDTreasureInfo *>::iterator itTreasure = mTreasure.find(sink.GetIntFromRes(&res, i, 0));
		if (itTreasure != mTreasure.end())
		{
			CMDUesrTreasureBuyInfo *pBuy = (userid ? itTreasure->second->mutable_ext()->mutable_my_buy(): itTreasure->second->mutable_ext()->add_buyers());
			pBuy->mutable_buy_user()->set_userid(sink.GetIntFromRes(&res, i, 1));
			pBuy->set_buy_num(sink.GetIntFromRes(&res, i, 2));
		}
	}
}
