#include "SL_Headers.h"
#include "DBSink.h"
#include "timestamp.h"
#include "message_vchat.h"
#include "macro_define.h"

int CDBSink::m_nPlatId = -1;
std::map<std::string , int> CDBSink::m_mHostPlatid;

static std::map<std::string, std::string> s_preloadedConfigParams;

CDBSink::CDBSink()
{
	threadMutex.lock();
	if (-1 == m_nPlatId)
	{
		int loop = 0;
		do
		{
			if (Dbconn::initDBSinkPool(m_nPlatId))
				break;

			loop++;
			usleep(10 * 1000);
		} while (loop < 3);

		if (-1 == m_nPlatId)
		{
			throw "initDBSinkPool error.";
		}
	}

	sink = sink_pool_get(m_nPlatId);
	m_sos = sink->sos;
	threadMutex.unlock();
}

CDBSink::CDBSink(Sink *s)
{
	sink = s;
	m_sos = sink->sos;
}

CDBSink::~CDBSink(void)
{
	if (sink)
	{
		sink_pool_put(sink);
	}
}

CDBSink *CDBSink::getSinkByHost(const std::string &host)
{
	CDBSink *pSink = NULL;
	if (!host.empty())
	{
		if (m_mHostPlatid.find(host) != m_mHostPlatid.end())
		{
			Sink *sink = sink_pool_get(m_mHostPlatid[host]);
			if (sink)
				pSink = new CDBSink(sink);
		}
		else
		{
			int platid = 0;
			if (Dbconn::initDBSinkPool(platid, 5, host))
			{
				m_mHostPlatid[host] = platid;
				Sink *sink = sink_pool_get(m_mHostPlatid[host]);
				if (sink)
				    pSink = new CDBSink(sink);
			}
		}
	}

	return pSink;
}

bool CDBSink::existAdKeyword(char *pKeyword)
{
	if (pKeyword == NULL || *pKeyword == 0)
		return false;

	int row = 0;
	char query[256];
	sprintf(query, "select keyname from dks_keywords where keyname='%s'", pKeyword);
	if (!run_sql(query, NULL, &row))
	{
		return false;
	}

	return row > 0;
}

bool CDBSink::addAdKeyword(CMDAdKeywordInfo_t *pKeyword)
{
	char   query[256];
	sprintf(query, "insert into dks_keywords(adtype,runerid,ctime,keyname) values(%d,%d,'%s','%s')",
		pKeyword->ntype, pKeyword->nrunerid, pKeyword->createtime, pKeyword->keyword);

	return run_sql(query);
}

bool CDBSink::delAdKeyword(CMDAdKeywordInfo_t *pKeyword)
{
	char query[256];
	sprintf(query, "delete from dks_keywords where keyname='%s'", pKeyword->keyword);
	return run_sql(query);
}

int CDBSink::getAdKeywordList(int startline, int linenum, std::vector<CMDAdKeywordInfo_t> &vKeyWord)
{
	int row = 0;
	Result res;
	char query[256];
	sprintf(query, "select adtype, runerid, ctime, keyname from dks_keywords limit %d offset %d", linenum, startline);
	if (!build_sql_run(&res, &row, query) || 0 == row)
	{
		CleanRes(&res);
		return 0;
	}

	vKeyWord.resize(row);
	for (int r = 0; r < row; r++)
	{
		CMDAdKeywordInfo_t record;
		memset(&record, 0, sizeof(CMDAdKeywordInfo_t));
		GetFromRes(record.ntype, &res, r, 0);
		GetFromRes(record.nrunerid, &res, r, 1);
		GetFromRes(record.createtime, &res, r, 2, sizeof(record.createtime));
		GetFromRes(record.keyword, &res, r, 3, sizeof(record.keyword));
		vKeyWord[r] = record;
	}

	CleanRes(&res);

	return row;
}

int CDBSink::func_updateusermonthcost_DB(unsigned int userid, int64 cost_nk)
{
	char query[128] = { 0 };
	sprintf(query, "select updateusermonthcost(%u,%lld)", userid, cost_nk);
	if (!run_sql(query))
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int CDBSink::check_user_token_DB(unsigned int loginid, const std::string & strToken, bool checkTime)
{
	if (!loginid || strToken.empty())
	{
		LOG_PRINT(log_error, "check_user_token_DB input error.");
		return -1;
	}

	Result res;
	char query[256] = {0};
	int row;

	sprintf(query, "select code_time from talk_user where user_id=%u and code='%s'", loginid, strToken.c_str());
	if (!build_sql_run(&res, &row, query) || 0 == row)
	{
		CleanRes(&res);
		return -1;
	}

	unsigned int code_time = 0;
	GetFromRes(code_time, &res, 0, 0);
	CleanRes(&res);

	if (code_time == 0)
	{
		LOG_PRINT(log_error, "get_token code_time is 0.loginid:%u,token:%s.", loginid, strToken.c_str());
		return -1;
	}

	if (checkTime)
	{
		time_t valid_time = code_time;
		time_t now_time = time(NULL);
		if (now_time > valid_time)
		{
			LOG_PRINT(log_error, "token expire timeout!now_time:%u,valid_time:%u,loginID:%u,token:%s.", (unsigned int)now_time, code_time, loginid, strToken.c_str());
			return -2;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}

int CDBSink::resetTokenValidTime(unsigned int loginid, const std::string & strToken)
{
	if (!loginid || strToken.empty())
	{
		LOG_PRINT(log_error, "resetTokenValidTime input error.");
		return -1;
	}

	unsigned int time_now = time(NULL);
	char updateSQL[256] = { 0 };
	sprintf(updateSQL, "update mc_member set code_time=%u where id=%u and code='%s'", time_now, loginid, strToken.c_str());
	return run_sql(updateSQL);
}

bool CDBSink::checkUserHasAuth(unsigned int userid, unsigned int groupid, unsigned int authType)
{
	if (!userid || !groupid)
	{
		return false;
	}

	std::stringstream strSQL;
	strSQL << " select count(a.id) from mc_authentication a,mc_group_member b where a.is_del=0 and a.is_pass=1 and a.type=" << authType << " and a.group_id=" << groupid << " and a.user_id=" << userid;
	strSQL << " and a.group_id=b.group_id and a.user_id=b.user_id and b.out_time is not null and a.auth_time > b.out_time";

	Result res;
	int row = 0;
	if (!build_sql_run(&res, &row, strSQL.str().c_str()) || 0 == row)
	{
		CleanRes(&res);
		return false;
	}

	unsigned int count = 0;
	GetFromRes(count, &res, 0, 0);
	CleanRes(&res);
	return count == 0 ? false : true;
}

int CDBSink::userApplyJoinGroupAuth(unsigned int userid, unsigned int groupid, unsigned int authType, const std::string & user_msg)
{
	if (!userid || !groupid)
	{
		LOG_PRINT(log_error, "user apply join group auth input error.userid:%u,groupid:%u.", userid, groupid);
		return 0;
	}

	LOG_PRINT(log_info, "user apply join group auth input:userid:%u,groupid:%u,authType:%u.", userid, groupid, authType);

	int id = 0;
	Result res;
	int row = 0;
	std::stringstream strSQL;
	strSQL << " select id from mc_authentication where type=" << authType << " and user_id=" << userid << " and group_id=" << groupid << " and is_del=0 and is_pass=2";
	if (!build_sql_run(&res, &row, strSQL.str().c_str()))
	{
		CleanRes(&res);
		return 0;
	}

	if (row)
	{
		GetFromRes(id, &res, 0, 0);
	}
	CleanRes(&res);
	
	int insertID = 0;
	transBegin();
	bool rollBack = true;
	do 
	{
		if (id)
		{
			std::stringstream strUpdateSQL;
			strUpdateSQL << " update mc_authentication set is_del=1 where id=" << id;
			if (!run_sql(strUpdateSQL.str().c_str()))
			{
				break;
			}
		}

		std::stringstream strInsertSQL;
		strInsertSQL << " insert into mc_authentication(type,is_pass,group_id,user_id,auth_time,user_msg)value(";
		strInsertSQL << authType << ",";
		strInsertSQL << 2 << ",";
		strInsertSQL << groupid << ",";
		strInsertSQL << userid << ",";
		strInsertSQL << (int)time(NULL) << ",";
		strInsertSQL << "'" << user_msg << "'";
		strInsertSQL << ")";
		if (!run_sql(strInsertSQL.str().c_str()))
		{
			break;
		}
		rollBack = false;
		insertID = getInsertId();
	} while (0);

	if (rollBack)
	{
		transRollBack();
	}
	else
	{
		transCommit();
	}
	return insertID;
}

bool CDBSink::updateJoinGroupAuthMsgID(unsigned int authID, unsigned int mongoMsgID)
{
	if (!authID || !mongoMsgID)
	{
		return false;
	}

	std::stringstream strUpdateSQL;
	strUpdateSQL << " update mc_authentication set msg_id=" << mongoMsgID << " where id=" << authID;
	return run_sql(strUpdateSQL.str().c_str());
}

bool CDBSink::qryJoinGroupAuth(unsigned int id, unsigned int & groupAssistID, unsigned int & apply_userID, unsigned int & groupID)
{
	groupAssistID = 0;
	apply_userID = 0;
	groupID = 0;
	if (!id)
	{
		return false;
	}

	LOG_PRINT(log_info, "query user apply join group auth input:id:%u.", id);
	Result res;
	int row = 0;
	std::stringstream strSQL;
	strSQL << " select msg_id,user_id,group_id from mc_authentication where id=" << id;
	if (!build_sql_run(&res, &row, strSQL.str().c_str()) || row == 0)
	{
		CleanRes(&res);
		return false;
	}

	GetFromRes(groupAssistID, &res, 0, 0);
	GetFromRes(apply_userID, &res, 0, 1);
	GetFromRes(groupID, &res, 0, 2);
	CleanRes(&res);
	return true;
}

bool CDBSink::handleJoinGroupAuth(unsigned int id, bool agree)
{
	if (!id)
	{
		return false;
	}

	LOG_PRINT(log_info, "handle user apply join group auth input:id:%u,agree:%d.", id, (int)agree);
	std::stringstream strUpdateSQL;
	strUpdateSQL << "update mc_authentication set is_pass=";
	if (agree)
	{
		strUpdateSQL << 1;
	}
	else
	{
		strUpdateSQL << 0;
	}
	strUpdateSQL << " where id=" << id;
	return run_sql(strUpdateSQL.str().c_str());
}

int CDBSink::qryGroupOwner(unsigned int groupid)
{
	if (!groupid)
	{
		LOG_PRINT(log_error, "query group owner input error.");
		return -1;
	}

	std::stringstream strSQL;
	strSQL << " select id from mc_user where id in (select group_master from mc_group where id=" << groupid << ")";
	Result res;
	int row = 0;
	int ret = 0;
	do 
	{
		if (!build_sql_run(&res, &row, strSQL.str().c_str()))
		{
			ret = -1;
			break;
		}

		if (row == 0)
		{
			break;
		}

		GetFromRes(ret, &res, 0, 0);
	} while (0);

	CleanRes(&res);
	return ret;
}

int CDBSink::updateGroupOwner(unsigned int groupid, unsigned int new_owner)
{
	if (!groupid || !new_owner)
	{
		LOG_PRINT(log_error, "set group owner input error.groupid:%u,new_owner:%u.", groupid, new_owner);
		return -1;
	}

	int ret = qryGroupOwner(groupid);
	if (ret == -1)
	{
		LOG_PRINT(log_error, "query group old owner error.groupid:%u.", groupid);
		return ret;
	}

	std::stringstream strUpdateSQL;
	strUpdateSQL << " update mc_group set group_master=" << new_owner << " where id=" << groupid << " and exists (select id from mc_user where id=" << new_owner << ")";
	if (!run_sql(strUpdateSQL.str().c_str()))
		return -1;

	if (getAffectedRow() > 0)
	{
		return ret;
	}
	else
	{
		return -1;
	}	
}

bool CDBSink::updateGroupMicMode(unsigned int groupid, int mode)
{
	if (!groupid || mode > 2 || mode < 0)
	{
		return false;
	}

	std::stringstream strUpdateSQL;
	strUpdateSQL << " update mc_group set mic_mode=" << mode << " where id=" << groupid;
	return run_sql(strUpdateSQL.str().c_str());
}

int CDBSink::write_addlastgoroom_DB(uint32 userid, uint32 vcbid, uint32 type)
{
	char szTime[32] = { 0 };
	char query[256] = { 0 };
	time_t tnow = time(NULL);

	toStringDate(tnow, szTime);
	sprintf(query, "delete from mc_user_group_log where user_id=%u and group_id=%u and dev_type=%u and date_time='%s'", \
		userid, vcbid, type, szTime);
	run_sql(query);

	sprintf(query, "insert into mc_user_group_log(user_id,group_id,dev_type,date_time) values(%u,%u,%u,'%s')",
		userid, vcbid, type, szTime);
	run_sql(query);
	return 0;
}

string CDBSink::build_blacklist_sql(int userid, int vcbid, int scopetype, char* szip, char* szmac, char* sn)
{
	string sql;
	char query[512];
	char filter[256] = { 0 };
	char sztemp[128] = { 0 };
	//const char* str;
	int bflag = 0;

	if (userid == 0 && /*szip==0 &&*/ IS_STR_EMPTY(szmac) && IS_STR_EMPTY(sn))
		return sql;

	if (userid != 0)
	{
		if (bflag)
			sprintf(sztemp, " nuserid=%d ", userid);
		else
			sprintf(sztemp, " where (nuserid=%d ", userid);
		strcat(filter, sztemp);
		bflag = 1;
	}

	if (!IS_STR_EMPTY(szmac))
	{
		if (bflag)
			sprintf(sztemp, " or cmac='%s' ", szmac);
		else
			sprintf(sztemp, " where (cmac='%s' ", szmac);
		strcat(filter, sztemp);
		bflag = 1;
	}
	if (!IS_STR_EMPTY(sn))
	{
		if (bflag)
			sprintf(sztemp, " or chdsn='%s' ", sn);
		else
			sprintf(sztemp, " where (chdsn='%s' ", sn);
		strcat(filter, sztemp);
		bflag = 1;
	}

	strcat(filter, ")");

	sprintf(sztemp, " and ((nscope=1 and nvcbid=%d) or nscope!=1)", vcbid);
	strcat(filter, sztemp);

	sprintf(query, "select nreason,nduration,nscope,tendtime,nfreezeaccount from dks_blackuser %s limit 1", filter);

	sql = query;
	return sql;
}

int CDBSink::isin_blackIpMac_DB(int userid, int vcbid, int scopetype, char* szip, char* szmac, char* sn, DDViolationInfo_t* pInfo/* = NULL*/)
{
	string sql = build_blacklist_sql(userid, vcbid, scopetype, szip, szmac, sn);
	if (sql.empty())
		return -1;

	Result res;
	int row = 0;
	if (!build_sql_run(&res, &row, sql.c_str())) {
		CleanRes(&res);
		return -1;
	}
	if (0 == row) {
		CleanRes(&res);
		return -2;
	}

	if (pInfo)
	{
		time_t tendtime;
		GetFromRes(pInfo->reasontype, &res, 0, 0);
		GetFromRes(pInfo->scopetype, &res, 0, 2);
		GetFromRes(tendtime, &res, 0, 3);

		pInfo->lefttime = tendtime - time(0);
	}

	CleanRes(&res);

	return 0;
}

int CDBSink::update_vcbmedia_DB(uint32 vcbid, const char *paddr)
{
	char query[256];
	if (!paddr || *paddr == 0)
		return -1;
	sprintf(query, "update dks_vcbstatus set cmedia='%s' where nvcbid=%u", paddr, vcbid);
	if (!build_sql_run(0, 0, query)) {
		return -1;
	}
	return 0;
}

int CDBSink::get_user_fullinfoByLoginID_DB(StUserFullInfo_t & ptf, uint32 loginID, const std::string & patternlock)
{
	if (!loginID)
	{
		LOG_PRINT(log_error, "get_user_fullinfoByLoginID_DB failed,loginid is wrong.");
		return -1;
	}

	char query[1024] = {0};
	Result res;
	int row;

	sprintf(query, "select id,gender,alias,headaddr,birthday,is_freeze from mc_user where member_id=%u and gesture_pwd='%s'", loginID, patternlock.c_str());

	if (!build_sql_run(&res, &row, query) || 0 == row)
	{
		CleanRes(&res);
		return -1;
	}

	GetFromRes(ptf.userid, &res, 0, 0);
	GetFromRes(ptf.gender, &res, 0, 1);
	GetFromRes(ptf.strAlias, &res, 0, 2);
	GetFromRes(ptf.strHead, &res, 0, 3);
	GetFromRes(ptf.strBirthday, &res, 0, 4);
	GetFromRes(ptf.nfreeze, &res, 0, 5);
	CleanRes(&res);

	do
	{
		row = 0;
		memset(query, 0, sizeof(query));
		sprintf(query, "select tel from mc_login where member_id=%u", loginID);
		if (!build_sql_run(&res, &row, query) || 0 == row)
		{
			CleanRes(&res);
			break;
		}

		GetFromRes(ptf.strTel, &res, 0, 0);
		CleanRes(&res);
	} while (0);

	return 0;
}

bool CDBSink::update_userinfo_DB(unsigned int userid, const std::map<std::string, std::string> & field_value_map)
{
	if (userid == 0 || field_value_map.empty())
	{
		LOG_PRINT(log_error, "update_userinfo_DB by userid error,userid is 0 or field_value_map size:%u.", field_value_map.size());
		return false;
	}

	std::stringstream updateSQL;
	updateSQL << "update mc_user set ";

	unsigned int pos = 0;
	unsigned int size = field_value_map.size();
	std::map<std::string, std::string>::const_iterator iter_map = field_value_map.begin();
	for (; iter_map != field_value_map.end(); ++iter_map, ++pos)
	{
		updateSQL << iter_map->first << "='" << iter_map->second << "'";
		if (pos != size - 1)
		{
			updateSQL << ",";
		}
	}

	updateSQL << " where id=" << userid;

	return run_sql(updateSQL.str().c_str());
}

int CDBSink::get_user_fullinfo_DB(StUserFullInfo_t & ptf, uint32 userid)
{
	if (userid == 0)
	{
		LOG_PRINT(log_error, "get user full info by userid error,userid is 0.");
		return -1;
	}

	char query[1024] = { 0 };
	Result res;
	int row = 0;
	sprintf(query, "select user_id,gender,alias,head_add,birthday,freeze,invite_user_id,code,code_time,tel from talk_user where user_id=%u", userid);
	if (!build_sql_run(&res, &row, query) || 0 == row)
	{
		CleanRes(&res);
		return -1;
	}

	GetFromRes(ptf.userid, &res, 0, 0);
	GetFromRes(ptf.gender, &res, 0, 1);
	GetFromRes(ptf.strAlias, &res, 0, 2);
	GetFromRes(ptf.strHead, &res, 0, 3);
	GetFromRes(ptf.strBirthday, &res, 0, 4);
	GetFromRes(ptf.nfreeze, &res, 0, 5);
	GetFromRes(ptf.invite_id, &res, 0, 6);
	GetFromRes(ptf.code, &res, 0, 7);
	GetFromRes(ptf.code_time, &res, 0, 8);
	GetFromRes(ptf.strTel, &res, 0, 9);

	CleanRes(&res);

	return 0;
}

int CDBSink::func_get_visitor_unique_id(const unsigned int area)
{
	char query[128] = { 0 };
	sprintf(query, "select nextval('visitorid', %u)", area);

	Result res;
	if (!run_sql(query, &res))
	{
		CleanRes(&res);
		return -1;
	}

	unsigned int ret_sucess = -1;
	GetFromRes(ret_sucess, &res, 0, 0);

	CleanRes(&res);
	return ret_sucess;
}

bool CDBSink::insert_vistorlogon_DB(const VistorLogon_t &vsl)
{
	char _logontime[32] = { 0 };
	toStringTimestamp(vsl.logontime, _logontime);

	char insertSQL[512] = { 0 };
	sprintf(insertSQL, "insert into dks_visitorlogon(userid,uuid,state,logontime) values(%u,'%s',%d,'%s')", \
		vsl.userid, vsl.cuuid, vsl.state, _logontime);

	return run_sql(insertSQL);
}

bool CDBSink::get_user_pwdinfo_DB(int userid, DDUserPwdInfo_t & ppf)
{
	Result res;
	char query[256] = { 0 };
	int row = 0;

	if (userid == 0) return false;
	sprintf(query, "select user_id,password from dks_user where user_id=%d", userid);

	if (!run_sql(query, &res, &row) || row != 1)
	{
		CleanRes(&res);
		return false;
	}

	GetFromRes(ppf.userid, &res, 0, 0);
	GetFromRes(ppf.cpassword, &res, 0, 1, sizeof(ppf.cpassword));

	CleanRes(&res);
	return true;
}

bool CDBSink::update_userpwd_DB(int userid, char * pPwd, int pwdtype)
{
	if (NULL == pPwd) return false;

	char query[256] = { 0 };
	if (pwdtype == 1) //~{SC;'5GB<C\Bk~}
	{
		sprintf(query, "update dks_user set password='%s' where user_id=%d", pPwd, userid);
	}
	else
	{
		return false;
	}

	return run_sql(query);
}

int CDBSink::get_user_secure_info_DB(int userid, CMDSecureInfo_t &info)
{
	char query[512];
	sprintf(query, "select cemail,cqq,ctel,nreminded_time from dks_user where nuserid=%d", userid);

	Result res;
	int row = 0;
	if (!run_sql(query, &res, &row) || row != 1)
	{
		CleanRes(&res);
		return -1;
	}

	GetFromRes(info.email, &res, 0, 0, sizeof(info.email));
	GetFromRes(info.qq, &res, 0, 1, sizeof(info.qq));
	GetFromRes(info.phonenumber, &res, 0, 2, sizeof(info.phonenumber));
	GetFromRes(info.remindtimes, &res, 0, 3);
	CleanRes(&res);

	if (info.remindtimes < 3 && (strlen(info.email) == 0 || strlen(info.qq) == 0 || strlen(info.phonenumber) == 0))
	{
		sprintf(query, "update dks_user set nreminded_time=%d where nuserid=%d", info.remindtimes + 1, userid);
		run_sql(query);
	}

	return row;
}

int CDBSink::get_privilegelist_DB(vector<DDQuanxianAction_t> &vAction)
{
	const char *query = "select a.qx_id,a.qx_type,b.srclevel,b.tolevel from tbquanxianinfo as a,tbquanxianaction as b where a.qx_id=b.qx_id ";

	Result res;
	int row = 0;
	if (!run_sql(query, &res, &row) || 0 == row)
	{
		CleanRes(&res);
		return 0;
	}

	vAction.resize(row);
	for (int r = 0; r < row; ++r)
	{
		GetFromRes(vAction[r].qxid, &res, 0, 0);
		GetFromRes(vAction[r].qxtype, &res, 0, 1);
		GetFromRes(vAction[r].srclevel, &res, 0, 2);
		GetFromRes(vAction[r].tolevel, &res, 0, 3);

	}
	CleanRes(&res);

	return row;
}

int CDBSink::write_logonrecord(const CMDUserLoginLog_t & oLogonlog)
{
	int ret = 0;
	ulong ntime_begin = 0;
	ulong ntime_end = 0;
	ntime_begin = SL_Socket_CommonAPI::util_process_clock_ms();

	//生成登陆记录
	time_t tnow = time(NULL);
	char szTime[32] = { 0 };
	toStringTimestamp(tnow, szTime);

	int total_row = 0;
	Result res;
	do
	{
		if (!build_sql_run(&res, &total_row,
			"insert into mc_userlogonlog(userid,type,logonip,macaddr,serialaddr,logontime,devicemodel,deviceos) values(%d,%d,'%s','%s','%s','%s','%s','%s')",
			oLogonlog.userid, oLogonlog.devtype, oLogonlog.szip, oLogonlog.szmac, oLogonlog.szserial, szTime, oLogonlog.devicemodel,
			oLogonlog.deviceos))
		{
			ret = -1;
			break;
		}

		if (!build_sql_run(&res, &total_row, "update mc_user set last_login_time=%u,last_login_ip='%s',last_login_mac='%s' where id=%d", (unsigned int)tnow, \
			oLogonlog.szip, oLogonlog.szmac, oLogonlog.userid))
		{
			ret = -1;
			break;
		}
	} while (0);
	CleanRes(&res);

	ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();
	LOG_PRINT(log_info, "userID[%d],IP[%s] logonRecord takes time:%u,", oLogonlog.userid, oLogonlog.szip, ntime_end - ntime_begin);

	return ret;
}


bool CDBSink::qryDictItemByGroupId(const char *group, vector<DictItem_t> &vOut)
{
	if (NULL == group || *group == 0)
	{
		return false;
	}
	char szQuerySQL[256] = { 0 };
	sprintf(szQuerySQL, "select groupid, itemid, addvalue from dcl_dict_item a where a.groupid = '%s' and a.status = 1", group);

	Result res;
	int row;
	if (!run_sql(szQuerySQL, &res, &row) || 0 == row)
	{
		CleanRes(&res);
		return false;
	}

	vOut.resize(row);
	for (int i = 0; i < row; i++)
	{
		DictItem_t item = {};
		GetFromRes(item.group, &res, i, 0, sizeof(item.group) - 1);
		GetFromRes(item.item, &res, i, 1, sizeof(item.item) - 1);
		GetFromRes(item.addvalue, &res, i, 2, sizeof(item.addvalue) - 1);
		vOut[i] = item;
	}
	CleanRes(&res);

	return true;
}

bool CDBSink::write_room_maxclientnum(int vcbid, int maxclientnum)
{
	char query[256];
	sprintf(query, "update dks_vcbstatus set maxclientnum = %d where nvcbid = %d", maxclientnum, vcbid);

	if (!run_sql(query))
	{
		return false;
	}

	return true;
}

int CDBSink::get_isroommanager_DB(uint32 userid, uint32 vcbid)
{
	int row = 0;
	char query[256];
	uint32 actualRoomId = (vcbid < 1000000000) ? vcbid : (vcbid - 1000000000);
	const char *ROOM = (vcbid < 1000000000) ? "talk_course" : "talk_live";
	const char *UID_STR = (vcbid < 1000000000) ? "uid" : "user_id";
	sprintf(query, "select %s from %s where id=%d and %s=%d", UID_STR, ROOM, actualRoomId, UID_STR, userid);

	do
	{
		if (!run_sql(query, NULL, &row))	{
			break;
		}

		if (row){
			return 0;
		}

	} while (0);

	return -1;
}


int CDBSink::check_isMaJia(uint32 userid)
{
	int row = 0;
	char query[256];
	sprintf(query, "select 1 from talk_user  where user_id=%d and user_type=2", userid);

	do
	{
		if (!run_sql(query, NULL, &row))	{
			break;
		}

		if (row){
			return 0;
		}

	} while (0);

	return -1;
}


int CDBSink::check_isLiuLiangZhu(uint32 userid)
{
	int row = 0;
	char query[256];
	sprintf(query, "select 1 from talk_user  where user_id=%d and user_level=3", userid);

	do
	{
		if (!run_sql(query, NULL, &row))	{
			break;
		}

		if (row){
			return 0;
		}

	} while (0);

	return -1;
}

int CDBSink::getActiveGroupfullinfo_DB(std::list<stRoomFullInfo> & oDBRecords)
{
	oDBRecords.clear();
	Result res;
	int row = 0;
	std::stringstream strSQL;
	strSQL << " select id,uid,live_id,class_name,teacher_name,lecturers,img,src_img,price,level,password,invite_code,content,tags ,goal,intended_user,";
		strSQL << " requirements,UNIX_TIMESTAMP(begin_time),UNIX_TIMESTAMP(end_time),status,open_status,publish_time,UNIX_TIMESTAMP(create_time),UNIX_TIMESTAMP(update_time), talk_status";
		strSQL << " from talk_course a where a.open_status = 1 ";
		strSQL << " union ";
		strSQL << " select id,uid,live_id,class_name,teacher_name,lecturers,img,src_img,price,level,password,invite_code,content,tags ,goal,intended_user, ";
		strSQL << " requirements,UNIX_TIMESTAMP(begin_time),UNIX_TIMESTAMP(end_time),status,open_status,publish_time,UNIX_TIMESTAMP(create_time),UNIX_TIMESTAMP(update_time), talk_status ";
		strSQL << " from (select  id + 1000000000  as id, user_id uid, id live_id, name class_name,'' teacher_name,'' lecturers,img,background_img src_img,0 price,";
		strSQL << " 0 level,'' password,'' invite_code,'' brief,content,'' tags ,'' goal,0 intended_user,  '' requirements,'' begin_time,'' end_time,status,open_status,";
		strSQL << " '' publish_time,create_time,create_time update_time, 0 talk_status  from talk_live ) x where x.open_status = 1 ";

	if (!build_sql_run(&res, &row, strSQL.str().c_str()))
	{
		CleanRes(&res);
		return -1;
	}

	if (0 == row)
	{
		LOG_PRINT(log_info, "cannot find active groups in DB.");
		CleanRes(&res);
		return -1;
	}

	for (int r = 0; r < row; ++r)
	{
		stRoomFullInfo record;
		int c = 0;

		//basic info
		GetFromRes(record.ncourseid, &res, r, c++);
		GetFromRes(record.nuser_id, &res, r, c++);
		GetFromRes(record.nlive_id, &res, r, c++);
		GetFromRes(record.class_name, &res, r, c++);
		GetFromRes(record.teacher_name, &res, r, c++);
		GetFromRes(record.teacher_info, &res, r, c++);
		GetFromRes(record.img, &res, r, c++);
		GetFromRes(record.src_img, &res, r, c++);
		GetFromRes(record.nprice, &res, r, c++);
		GetFromRes(record.nlevel, &res, r, c++);
		GetFromRes(record.strpwd, &res, r, c++);
		GetFromRes(record.invite_code, &res, r, c++);
		GetFromRes(record.strremark, &res, r, c++);
	//	GetFromRes(record.strremark, &res, r, c++);
		GetFromRes(record.tags, &res, r, c++);
		GetFromRes(record.goal, &res, r, c++);
		GetFromRes(record.dest_user, &res, r, c++);
		GetFromRes(record.requirements, &res, r, c++);
		GetFromRes(record.nbegin_time, &res, r, c++);
		GetFromRes(record.nend_time, &res, r, c++);
		GetFromRes(record.nstatus, &res, r, c++);
		GetFromRes(record.nopenstatus, &res, r, c++);
		GetFromRes(record.npublish_time, &res, r, c++);
		GetFromRes(record.ncreate_time, &res, r, c++);
		GetFromRes(record.nupdate_time, &res, r, c++);
		GetFromRes(record.talk_status, &res, r, c++);

		oDBRecords.push_back(record);
	}
	CleanRes(&res);
	return row;
}

bool CDBSink::updateGroupMsgMute(unsigned int userid, unsigned int groupid, unsigned int muteAction)
{
	if (!groupid || !userid)
	{
		LOG_PRINT(log_error, "DB getGroupMemebr_DB input error.userid:%u,groupid:%u.", userid, groupid);
		return false;
	}

	muteAction = muteAction ? 1 : 0;

	std::stringstream strUpdateSQL;
	strUpdateSQL << " update mc_group_member set mute_notifications=" << muteAction;
	strUpdateSQL << " where group_id=" << groupid;
	strUpdateSQL << " and user_id=" << userid;
	return run_sql(strUpdateSQL.str().c_str());
}

//return true by default.same logic as DB
bool CDBSink::getGroupMsgMute(unsigned int userid, unsigned int groupid)
{
	if (!groupid || !userid)
	{
		LOG_PRINT(log_error, "DB getGroupMsgMute input error.userid:%u,groupid:%u.", userid, groupid);
		return true;
	}

	bool ret = true;
	Result res;
	int row = 0;
	std::stringstream strSQL;
	do 
	{
		strSQL << "select mute_notifications from mc_group_member where user_id=" << userid << " and group_id=" << groupid;
		if (!build_sql_run(&res, &row, strSQL.str().c_str()) || row == 0)
		{
			break;
		}

		int muteSetting = 1;
		GetFromRes(muteSetting, &res, 0, 0);
		LOG_PRINT(log_info, "userid:%u in the group:%u mute setting is %d.", userid, groupid, muteSetting);
		ret = (muteSetting == 0? false : true);
	} while (0);

	CleanRes(&res);
	return ret;
}

bool CDBSink::checkBlockUserLst_DB(unsigned int userid, unsigned int groupid)
{
	Result res;
	int row = 0;
	bool bRet = false;
	std::stringstream strSQL;
	do
	{
		strSQL << "select run_id from mc_group_blacklist where is_del=0 and user_id=" << userid << " and group_id=" << groupid;
		if (!build_sql_run(&res, &row, strSQL.str().c_str()) || row == 0)
		{
			break;
		}

		unsigned int run_id = 0;
		GetFromRes(run_id, &res, 0, 0);
		LOG_PRINT(log_warning, "userid:%u is in the group:%u block user list and runn_id is:%u", userid, groupid, run_id);
		bRet = true;
	} while (0);

	CleanRes(&res);
	return bRet;
}

bool CDBSink::initRoomUserCountDB(uint32_t vcbid, unsigned int & userNum, unsigned int & maleNum, unsigned int & femaleNum)
{
	if (!vcbid)
	{
		LOG_PRINT(log_error, "initRoomUserCountDB input error.vcbid is 0.");
		return false;
	}

	std::stringstream strSQL;
	strSQL << " select b.gender,count(a.id) from mc_group_member a,mc_user b where a.is_del=0 and a.role_type!=0 and a.group_id=" << vcbid;
	strSQL << " and a.user_id=b.id and b.is_del=0 group by b.gender";

	Result res;
	int row = 0;
	if (!build_sql_run(&res, &row, strSQL.str().c_str()))
	{
		CleanRes(&res);
		return false;
	}

	if (0 == row)
	{
		LOG_PRINT(log_info, "DB cannot find group members in group:%u.", vcbid);
		CleanRes(&res);
		return false;
	}

	maleNum = 0;
	femaleNum = 0;
	userNum = 0;

	for (int r = 0; r < row; ++r)
	{
		unsigned int gender = 0;
		unsigned int count = 0;
		GetFromRes(gender, &res, r, 0);
		GetFromRes(count, &res, r, 1);

		if (gender == e_MaleGender)
		{
			maleNum = count;
		}
		else if (gender == e_FemaleGender)
		{
			femaleNum = count;
		}

		userNum += count;
	}
	CleanRes(&res);

	std::map<std::string, std::string> mValues;
	mValues["user_num"] = bitTostring(userNum);
	mValues["male_num"] = bitTostring(maleNum);
	mValues["female_num"] = bitTostring(femaleNum);
	return updGroupBasicInfo(vcbid, mValues);
}

bool CDBSink::updateRoomVisitorNum(uint32_t vcbid, uint32_t visitorNum)
{
	if (!vcbid)
	{
		LOG_PRINT(log_error, "DB updateRoomVisitorNum input error.vcbid is 0.");
		return false;
	}

	std::stringstream strUpdateSQL;
	strUpdateSQL << " update mc_group set audience_num=" << visitorNum;
	strUpdateSQL << " where id=" << vcbid;

	return run_sql(strUpdateSQL.str().c_str());
}

bool CDBSink::updateRoomUserCount(uint32_t vcbid, const std::map<std::string, std::string> & oFieldAndIncMap)
{
	if (!vcbid || oFieldAndIncMap.empty())
	{
		LOG_PRINT(log_error, "DB updateRoomUserCount input error.vcbid:%u.", vcbid);
		return false;
	}

	std::stringstream sqlValues;
	std::map<std::string, std::string>::const_iterator iter_map = oFieldAndIncMap.begin();
	for (; iter_map != oFieldAndIncMap.end(); ++iter_map)
	{
		if (sqlValues.str().length() > 0)
		{
			sqlValues << ",";
		}

		std::string fieldName = iter_map->first;
		std::string strAdd = "";
		if (atoi(iter_map->second.c_str()) >= 0)
		{
			strAdd = "+" + iter_map->second;
		}
		else
		{
			strAdd = iter_map->second;
		}

		sqlValues << iter_map->first << " = " << iter_map->first << strAdd;
	}

	std::stringstream strUpdateSQL;
	strUpdateSQL << " update mc_group set " << sqlValues.str() << " where id=" << vcbid;
	return run_sql(strUpdateSQL.str().c_str());
}

void CDBSink::getGroupAdminLst_DB(unsigned int groupid, std::set<unsigned int> & oGroupAdminLst)
{
	oGroupAdminLst.clear();
	if (!groupid)
	{
		return;
	}

	Result res;
	int row = 0;
	std::stringstream strSQL;
	strSQL << " select a.user_id from mc_group_member a,mc_group b where a.group_id=b.id and a.group_id=" << groupid;
	strSQL << " and a.role_type >= " << e_AdminRole;
	strSQL << " and a.is_del = 0";

	if (!build_sql_run(&res, &row, strSQL.str().c_str()))
	{
		CleanRes(&res);
		return;
	}

	for (int i = 0; i < row; ++i)
	{
		unsigned int userID = 0;
		GetFromRes(userID, &res, i, 0);
		if (userID)
		{
			oGroupAdminLst.insert(userID);
		}
	}
}


int CDBSink::getOneGroupfullinfo_DB(stRoomFullInfo & record, uint32 groupID)
{
	bool bfound = true;
	Result res;
	int row = 0;
	std::stringstream strSQL;
	strSQL << " select id,uid,live_id,class_name,teacher_name,lecturers,img,src_img,price,level,password,a.invite_code,content,tags ,goal,intended_user,";
	strSQL << " requirements,UNIX_TIMESTAMP(begin_time),UNIX_TIMESTAMP(end_time),status,open_status,publish_time,UNIX_TIMESTAMP(create_time),UNIX_TIMESTAMP(update_time), talk_status, virtual_num,form  ";
	strSQL << " from talk_course a where a.open_status = 1 and id =" << groupID;

	if (!build_sql_run(&res, &row, strSQL.str().c_str()))
	{
		CleanRes(&res);
		return -1;
	}

	if (0 == row)
	{
		bfound = false;
		LOG_PRINT(log_info, "DB cannot find this group id:%u.", groupID);
		CleanRes(&res);
	}

	if(!bfound)
	{
		strSQL.str("");
		strSQL << "select id,uid,live_id,class_name,teacher_name,lecturers,img,src_img,price,level,password,invite_code,content,tags ,goal,intended_user, ";
		strSQL << " requirements,UNIX_TIMESTAMP(begin_time),UNIX_TIMESTAMP(end_time),status,open_status,publish_time,UNIX_TIMESTAMP(create_time),UNIX_TIMESTAMP(update_time), talk_status, virtual_num ";
		strSQL << " from (select  id + 1000000000  as id, user_id uid, id live_id, name class_name,'' teacher_name,'' lecturers,img,background_img src_img,0 price,";
		strSQL << " 0 level,'' password,'' invite_code,'' brief,content,'' tags ,'' goal,0 intended_user,  '' requirements,'' begin_time,'' end_time,status,open_status,";
		strSQL << " '' publish_time,create_time,create_time update_time, 0 talk_status, virtual_num ,0  from talk_live where id = " << groupID - 1000000000 << ") x where x.open_status = 1 and x.id = " << groupID;
		if (!build_sql_run(&res, &row, strSQL.str().c_str()))
		{
			CleanRes(&res);
			return -1;
		}
		if (0 == row)
		{
			LOG_PRINT(log_info, "DB cannot find this talk_live id:%u.", groupID);
			CleanRes(&res);
			return 0;
		}
	}
	int r = 0, c = 0;
	//basic info
	GetFromRes(record.ncourseid, &res, r, c++);
	GetFromRes(record.nuser_id, &res, r, c++);
	GetFromRes(record.nlive_id, &res, r, c++);
	GetFromRes(record.class_name, &res, r, c++);
	GetFromRes(record.teacher_name, &res, r, c++);
	GetFromRes(record.teacher_info, &res, r, c++);
	GetFromRes(record.img, &res, r, c++);
	GetFromRes(record.src_img, &res, r, c++);
	GetFromRes(record.nprice, &res, r, c++);
	GetFromRes(record.nlevel, &res, r, c++);
	GetFromRes(record.strpwd, &res, r, c++);
	GetFromRes(record.invite_code, &res, r, c++);
	GetFromRes(record.strremark, &res, r, c++);
//	GetFromRes(record.strremark, &res, r, c++);
	GetFromRes(record.tags, &res, r, c++);
	GetFromRes(record.goal, &res, r, c++);
	GetFromRes(record.dest_user, &res, r, c++);
	GetFromRes(record.requirements, &res, r, c++);
	GetFromRes(record.nbegin_time, &res, r, c++);
	GetFromRes(record.nend_time, &res, r, c++);
	GetFromRes(record.nstatus, &res, r, c++);
	GetFromRes(record.nopenstatus, &res, r, c++);
	GetFromRes(record.npublish_time, &res, r, c++);
	GetFromRes(record.ncreate_time, &res, r, c++);
	GetFromRes(record.nupdate_time, &res, r, c++);
	GetFromRes(record.talk_status, &res, r, c++);
	GetFromRes(record.virtual_num, &res, r, c++);
	GetFromRes(record.form, &res, r, c++);
	//LOG_PRINT(log_info, "errvirtual_num:%u.", record.virtual_num);
	CleanRes(&res);
	return 0;
}

int CDBSink::getGroupBroadInfo_DB(unsigned int group_id, StRoomNotice_t & oBroadInfo)
{
	if (!group_id)
	{
		LOG_PRINT(log_error, "DB getGroupMemebr_DB input error.groupid:%u.", group_id);
		return -1;
	}

	std::stringstream strQuerySQL;
	strQuerySQL << " select group_id, user_id, broad_info, UNIX_TIMESTAMP(broad_time) from mc_groupbroadinfo where";
	strQuerySQL << " group_id = " << group_id;
	strQuerySQL << " order by broad_time desc limit 1";

	int ret = 0;
	Result res;
	int rows = 0;
	do
	{
		if (!build_sql_run(&res, &rows, strQuerySQL.str().c_str()))
		{
			LOG_PRINT(log_error, "failed to exec sql: %s", strQuerySQL.str().c_str());
			ret = -1;
			break;
		}

		if (rows <= 0)
		{
			LOG_PRINT(log_error, "no result exec sql: %s", strQuerySQL.str().c_str());
			ret = -1;
			break;
		}

		GetFromRes(oBroadInfo.m_groupID, &res, 0, 0);
		GetFromRes(oBroadInfo.m_uBroadUserid, &res, 0, 1);
		GetFromRes(oBroadInfo.m_strRoomNotice, &res, 0, 2);
		GetFromRes(oBroadInfo.m_uBroadTime, &res, 0, 3);

	} while (0);

	CleanRes(&res);
	return ret;
}

bool CDBSink::updateGroupBroadInfo_DB(unsigned int group_id, const StRoomNotice_t & oBroadInfo)
{
	if (!group_id)
	{
		LOG_PRINT(log_error, "DB updateGroupBroadInfo_DB input error.groupid:%u.", group_id);
		return false;
	}

	std::stringstream strInsertSQL;
	strInsertSQL << " insert into mc_groupbroadinfo(group_id,user_id,broad_info,broad_time)values(";
	strInsertSQL << oBroadInfo.m_groupID << ",";
	strInsertSQL << oBroadInfo.m_uBroadUserid << ",";
	strInsertSQL << "'" << oBroadInfo.m_strRoomNotice << "',";
	strInsertSQL << " FROM_UNIXTIME(" << oBroadInfo.m_uBroadTime << ")";
	strInsertSQL << ")";

	return run_sql(strInsertSQL.str().c_str());
}

bool CDBSink::updateGroupBasicInfo_DB(unsigned int group_id, std::map<std::string, std::string> & oFieldValueMap)
{
	if (!group_id || oFieldValueMap.empty())
	{
		LOG_PRINT(log_error, "DB updateGroupBasicInfo_DB input error.groupid:%u.fieldValue empty(%d).", group_id, (int)oFieldValueMap.empty());
		return false;
	}

	std::stringstream strUpdateSQL;
	strUpdateSQL << " update mc_group set ";

	unsigned int pos = 0;
	unsigned int size = oFieldValueMap.size();
	std::map<std::string, std::string>::iterator iter_map = oFieldValueMap.begin();
	for (; iter_map != oFieldValueMap.end(); ++iter_map, ++pos)
	{
		strUpdateSQL << iter_map->first << "='" << iter_map->second << "'";
		if (pos != size - 1)
		{
			strUpdateSQL << ",";
		}
	}

	strUpdateSQL << " where id=" << group_id;
	return run_sql(strUpdateSQL.str().c_str());
}

int CDBSink::getGroupMemebr_DB(unsigned groupid, std::list<stJoinGroupInfo> & oGroupMemLst)
{
	oGroupMemLst.clear();
	if (!groupid)
	{
		LOG_PRINT(log_error, "DB getGroupMemebr_DB input error.groupid:%u.", groupid);
		return -1;
	}

	char cQuerySQL[256] = { 0 };
	sprintf(cQuerySQL, "select user_id,role_type,level_id from mc_group_member where group_id=%u and is_del=0 and role_type!=0", groupid);

	Result res;
	int row = 0;

	do
	{
		if (!build_sql_run(&res, &row, cQuerySQL))
		{
			row = -1;
			break;
		}

		for (int i = 0; i < row; ++i)
		{
			stJoinGroupInfo oGroupMemInfo = { 0 };
			GetFromRes(oGroupMemInfo.userID, &res, i, 0);
			GetFromRes(oGroupMemInfo.role_type, &res, i, 1);
			GetFromRes(oGroupMemInfo.levelID, &res, i, 2);
			oGroupMemInfo.groupID = groupid;
			oGroupMemLst.push_back(oGroupMemInfo);
		}

	} while (0);
	CleanRes(&res);

	return row;
}

int CDBSink::getGroupIDLst_DB(std::list<unsigned int> & groupIDLst)
{
	groupIDLst.clear();
	char query[256] = { 0 };
	Result res;
	int row = 0;
	sprintf(query, "select id from mc_group where is_del=0 and status=1");

	do
	{
		if (!build_sql_run(&res, &row, query) || 0 == row)
		{
			row = -1;
			break;
		}

		for (int i = 0; i < row; ++i)
		{
			unsigned int groupid;
			GetFromRes(groupid, &res, i, 0);
			groupIDLst.push_back(groupid);
		}

	} while (0);
	CleanRes(&res);

	return row;
}

bool CDBSink::getGroupMaxUnreadNum(uint32 groupid, int &num)
{
	num = 0;
	char query[256] = {};
	Result res;
	int row = 0;
	sprintf(query, " select unread_msgnum from mc_group where id = %u ", groupid);
	if (!run_sql(query, &res, &row) || 0 == row)
	{
		CleanRes(&res);
		return false;
	}

	GetFromRes(num, &res, 0, 0);
	CleanRes(&res);
	return true;
}

int CDBSink::getUserRelateGroupLst_DB(unsigned int userid, std::list<stGroupInfo> & groupLst)
{
	if (!userid)
	{
		LOG_PRINT(log_error, "DB getUserRelateGroupLst input error.userid:%u.", userid);
		return -1;
	}

	time_t now_time = time(NULL);
	int ret = 0;
	Result res;
	int row = 0;

	std::stringstream strQuerySQL;
	strQuerySQL << " select a.group_id,b.icon,b.name,b.showid,a.mute_notifications,a.role_type,b.user_num,b.audience_num";
	strQuerySQL << " from mc_group_member a,mc_group b where a.user_id=" << userid << " and a.is_del=0 and a.group_id=b.id and a.in_time<=" << (int)now_time << " and a.out_time is null";

	do
	{
		if (!run_sql(strQuerySQL.str().c_str(), &res, &row))
		{
			ret = -1;
			break;
		}

		for (int i = 0; i < row; ++i)
		{
			stGroupInfo oGroupInfo;
			GetFromRes(oGroupInfo.groupid,		&res, i, 0);
			GetFromRes(oGroupInfo.headaddr,		&res, i, 1);
			GetFromRes(oGroupInfo.name,			&res, i, 2);
			GetFromRes(oGroupInfo.showid,		&res, i, 3);
			GetFromRes(oGroupInfo.muteSet,		&res, i, 4);
			GetFromRes(oGroupInfo.roleType,		&res, i, 5);
			GetFromRes(oGroupInfo.memberCount,	&res, i, 6);
			GetFromRes(oGroupInfo.visitCount,	&res, i, 7);
			groupLst.push_back(oGroupInfo);
		}
		ret = row;
	} while (0);

	CleanRes(&res);
	return ret;
}

int CDBSink::get_userGroupInfo_DB(unsigned int userid, unsigned int groupid, StUserGroupInfo & oUserGroupInfo)
{
	std::stringstream strQuerySQL;
	strQuerySQL << " select role_type,mute_notifications,custom_gain,private_chat_intimacy,is_auth ";
	strQuerySQL << " from mc_group_member where user_id=" << userid << " and group_id=" << groupid << " and is_del=0";

	int ret = ERR_CODE_FAILED_DB;
	Result res;
	int row = 0;

	do
	{
		if (!run_sql(strQuerySQL.str().c_str(), &res, &row))
		{
			LOG_PRINT(log_error, "query user group info fail in db.userid:%u,groupid:%u.", userid, groupid);
			break;
		}

		if (row == 0)
		{
			LOG_PRINT(log_error, "[DB check]user is not in the mc_group_member.userid:%u,groupid:%u.", userid, groupid);
			ret = ERR_CODE_USER_NOT_IN_GROUP;
			break;
		}

		oUserGroupInfo.userID = userid;
		oUserGroupInfo.groupID = groupid;
		GetFromRes(oUserGroupInfo.roleType, 		&res, 0, 0);
		GetFromRes(oUserGroupInfo.noDisturbing, 	&res, 0, 1);
		GetFromRes(oUserGroupInfo.gainPercent, 		&res, 0, 2);
		GetFromRes(oUserGroupInfo.pchatThreshold, 	&res, 0, 3);
		GetFromRes(oUserGroupInfo.authState, 		&res, 0, 4);
		ret = 0;
	} while (0);

	CleanRes(&res);
	return ret;
}

int CDBSink::get_groupUserInfoList_DB(unsigned int groupid, std::map<uint32, StUserGroupInfo> &mGroupUserInfo)
{
	std::stringstream strQuerySQL;
	strQuerySQL << " select user_id,role_type,mute_notifications,custom_gain,private_chat_intimacy,is_auth ";
	strQuerySQL << " from mc_group_member where group_id=" << groupid << " and is_del=0";

	Result res;
	int row = 0;
	do
	{
		if (!run_sql(strQuerySQL.str().c_str(), &res, &row))
		{
			LOG_PRINT(log_error, "query group user info list fail in db.groupid:%u.", groupid);
			break;
		}

		for (int i = 0; i < row; i++)
		{
			StUserGroupInfo oInfo;
			oInfo.groupID = groupid;
			GetFromRes(oInfo.userID,			&res, i, 0);
			GetFromRes(oInfo.roleType,			&res, i, 1);
			GetFromRes(oInfo.noDisturbing,		&res, i, 2);
			GetFromRes(oInfo.gainPercent,		&res, i, 3);
			GetFromRes(oInfo.pchatThreshold,	&res, i, 4);
			GetFromRes(oInfo.authState,			&res, i, 5);

			mGroupUserInfo[oInfo.userID] = oInfo;
		}
	} while (0);

	CleanRes(&res);
	return row;
}

int CDBSink::get_userBaseAndGroupInfo_DB(unsigned int userid, unsigned int groupid, StUserBaseAndGroupInfo & oUserInfo)
{
	if (!userid || !groupid)
	{
		LOG_PRINT(log_error, "DB get_userBaseAndGroupInfo_DB input error.userid:%u,groupid:%u.", userid, groupid);
		return -1;
	}

	std::stringstream strQuerySQL;
	strQuerySQL << " select a.alias,a.gender,a.head_add,a.user_level  from talk_user a where a.user_id = "<< userid <<" and a.freeze = '0'";

	int ret = -1;
	Result res;
	int row = 0;

	do
	{
		if (!run_sql(strQuerySQL.str().c_str(), &res, &row))
		{
			LOG_PRINT(log_error, "query user group info fail in db.userid:%u,groupid:%u.", userid, groupid);
			break;
		}

		if (row == 0)
		{
			LOG_PRINT(log_error, "userid:%u is not member of groupid:%u.", userid, groupid);
			break;
		}

		oUserInfo.userGroupInfo.userID = userid;
		GetFromRes(oUserInfo.userAlias, &res, 0, 0);
		GetFromRes(oUserInfo.userGender, &res, 0, 1);
		GetFromRes(oUserInfo.userHead, &res, 0, 2);
		GetFromRes(oUserInfo.userGroupInfo.roleType, &res, 0, 3);
		//GetFromRes(oUserInfo.userGroupInfo.is_daka, &res, 0, 4);
		ret = 0;
	} while (0);

	CleanRes(&res);
	return ret;
}

int CDBSink::get_groupUserInfoList_EX(unsigned int groupid, std::map<uint32, StUserBaseAndGroupInfo> &mUserInfo)
{
	if (!groupid)
	{
		LOG_PRINT(log_error, "DB get_groupUserInfoList_EX input error.groupid:%u.", groupid);
		return -1;
	}

	std::stringstream strQuerySQL;
	strQuerySQL << " select b.id, b.alias,b.gender,b.headaddr,a.role_type,a.mute_notifications,a.custom_gain,a.private_chat_intimacy,a.is_auth, ";
	strQuerySQL << " if(a.is_auth=1,(select c.video_url from mc_authentication c where c.group_id = a.group_id and a.user_id = c.user_id and c.type = 0 and c.is_del = 0 and c.is_pass = 1 limit 1),'') as video_url";
	strQuerySQL << " from mc_group_member a,mc_user b ";
	strQuerySQL << " where a.user_id = b.id and a.group_id=" << groupid << " and a.is_del=0";

	Result res;
	int row = 0;

	do
	{
		if (!run_sql(strQuerySQL.str().c_str(), &res, &row) || 0 == row)
		{
			LOG_PRINT(log_error, "query group user info list fail in db.groupid:%u.", groupid);
			break;
		}

		for (int i = 0; i < row; i++)
		{
			StUserBaseAndGroupInfo oUserInfo;
			oUserInfo.userGroupInfo.groupID = groupid;
			int j = 0;
			GetFromRes(oUserInfo.userGroupInfo.userID,			&res, i, j++);
			GetFromRes(oUserInfo.userAlias,						&res, i, j++);
			GetFromRes(oUserInfo.userGender,					&res, i, j++);
			GetFromRes(oUserInfo.userHead,						&res, i, j++);
			GetFromRes(oUserInfo.userGroupInfo.roleType,		&res, i, j++);
			GetFromRes(oUserInfo.userGroupInfo.noDisturbing, 	&res, i, j++);
			GetFromRes(oUserInfo.userGroupInfo.gainPercent,		&res, i, j++);
			GetFromRes(oUserInfo.userGroupInfo.pchatThreshold, 	&res, i, j++);
			GetFromRes(oUserInfo.userGroupInfo.authState, 		&res, i, j++);
			GetFromRes(oUserInfo.userGroupInfo.videoUrl, 		&res, i, j++);

			mUserInfo[oUserInfo.userGroupInfo.userID] = oUserInfo;
		}

	} while (0);

	CleanRes(&res);
	return row;
}

bool CDBSink::addUserGroup_DB(const stJoinGroupInfo & oJoinObj)
{
	if (!oJoinObj.userID || !oJoinObj.groupID)
	{
		LOG_PRINT(log_error, "DB addUserGroup_DB input error.userid:%u,groupid:%u.", oJoinObj.userID, oJoinObj.groupID);
		return false;
	}

	time_t t_now = time(NULL);
	if (isUserInGroup(oJoinObj.userID, oJoinObj.groupID, true))
	{
		std::stringstream strUpdateSQL;
		strUpdateSQL << " update mc_group_member set in_time=" << (int)t_now << ",";
		strUpdateSQL << " in_way=" << (int)oJoinObj.in_way << ",";
		strUpdateSQL << " out_time=null,";
		strUpdateSQL << " is_del=0,";
		strUpdateSQL << " last_speak_time=0,";
		strUpdateSQL << " mute_notifications=default,";
		strUpdateSQL << " custom_gain=-1,";
		strUpdateSQL << " contribution_today=0,";
		strUpdateSQL << " contribution_week=0,";
		strUpdateSQL << " role_type=" << (int)oJoinObj.role_type;
		strUpdateSQL << " where group_id=" << oJoinObj.groupID;
		strUpdateSQL << " and user_id=" << oJoinObj.userID;
		strUpdateSQL << " and is_del=1";
		return run_sql(strUpdateSQL.str().c_str());
	}
	else
	{
		std::stringstream strInsertSQL;
		strInsertSQL << "insert into mc_group_member(group_id,user_id,in_time,in_way,role_type,private_chat_intimacy) select ";
		strInsertSQL << oJoinObj.groupID << ",";
		strInsertSQL << oJoinObj.userID << ",";
		strInsertSQL << (int)t_now << ",";
		strInsertSQL << (int)oJoinObj.in_way << ",";
		strInsertSQL << (int)oJoinObj.role_type;
		strInsertSQL << ",case b.gender when 1 then a.male_pchat_min_threshold else a.female_pchat_min_threshold end threshold ";
		strInsertSQL << " from mc_group_plugin_setting_detail a, mc_user b where a.group_id = " << oJoinObj.groupID << " and b.id = " << oJoinObj.userID;
		return run_sql(strInsertSQL.str().c_str());
	}
}

bool CDBSink::delUserGroup_DB(unsigned int userid, unsigned int groupid)
{
	if (!userid || !groupid)
	{
		LOG_PRINT(log_error, "DB user quit group input error.userid:%u,groupid:%u.", userid, groupid);
		return false;
	}

	time_t now_time = time(NULL);
	//just mark this user's quit state of this group
	char cUpdateSQL[256] = { 0 };
	snprintf(cUpdateSQL, sizeof(cUpdateSQL), "update mc_group_member set is_del=1,out_time=%d where user_id=%u and group_id=%u", (int)now_time, userid, groupid);
	return run_sql(cUpdateSQL);
}

bool CDBSink::isUserInGroup(unsigned int userid, unsigned int groupid, bool checkDBRecord)
{
	if (!userid || !groupid)
	{
		LOG_PRINT(log_error, "DB check isUserInGroup input error.userid:%u,groupid:%u.", userid, groupid);
		return -1;
	}

	bool ret = false;
	Result res;
	int row = 0;

	char cQuerySQL[256] = { 0 };
	if (checkDBRecord)
	{
		snprintf(cQuerySQL, sizeof(cQuerySQL), "select 1 from mc_group_member where user_id=%u and group_id=%u", userid, groupid);
	}
	else
	{
		snprintf(cQuerySQL, sizeof(cQuerySQL), "select 1 from mc_group_member where user_id=%u and group_id=%u and out_time is null", userid, groupid);
	}

	do
	{
		if (!run_sql(cQuerySQL, &res, &row))
		{
			break;
		}

		if (row)
		{
			ret = true;
		}

	} while (0);

	CleanRes(&res);
	return ret;
}

bool CDBSink::updateUserRoletype_DB(unsigned int userid, unsigned int groupid, unsigned int new_roletype)
{
	if (!userid || !groupid)
	{
		LOG_PRINT(log_error, "update user roletype input error,userid:%u,groupid:%u.", userid, groupid);
		return false;
	}

	LOG_PRINT(log_info, "[updateUserRoletype_DB]input:userid:%u,groupid:%u,new_roletype:%u.", userid, groupid, new_roletype);
	unsigned int old_roletype = 0;
	Result res;

	int row = 0;
	std::stringstream strQuerySQL;
	strQuerySQL << " select role_type from mc_group_member where is_del=0 and group_id=" << groupid << " and user_id=" << userid;
	if (!build_sql_run(&res, &row, strQuerySQL.str().c_str()))
	{
		CleanRes(&res);
		return false;
	}

	if (0 == row)
	{
		//user is visitor in db
		old_roletype = e_VisitorRole;
		CleanRes(&res);
	}
	else
	{
		//user is member in db
		GetFromRes(old_roletype, &res, 0, 0);
		CleanRes(&res);
	}

	if (old_roletype == new_roletype)
	{
		LOG_PRINT(log_info, "[DB check]user is same roletype in group.not need to handle anymore.userid:%u,groupid:%u,roletype:%u", userid, groupid, new_roletype);
		return true;
	}
	else if (old_roletype == e_VisitorRole)
	{
		LOG_PRINT(log_warning, "[DB check]user upgrade from visitor to group member,need to join group.userid:%u,groupid:%u,new roletype:%u.", userid, groupid, new_roletype);
		stJoinGroupInfo oJoinObj = { 0 };
		oJoinObj.userID = userid;
		oJoinObj.groupID = groupid;
		oJoinObj.role_type = new_roletype;
		return addUserGroup_DB(oJoinObj);
	}
	else if (new_roletype == e_VisitorRole)
	{
		LOG_PRINT(log_warning, "[DB check]user degrade from member to visitor,need to quit group.userid:%u,groupid:%u,new roletype:%u,old roletype:%u.", \
			userid, groupid, new_roletype, old_roletype);
		return delUserGroup_DB(userid, groupid);
	}
	else
	{
		std::stringstream strUpdateSQL;
		strUpdateSQL << " update mc_group_member set role_type=" << new_roletype;
		strUpdateSQL << " where group_id=" << groupid << " and user_id=" << userid;
		strUpdateSQL << " and role_type!=" << new_roletype;
		return run_sql(strUpdateSQL.str().c_str());
	}
}

int CDBSink::getRedPacketBasicInfo(unsigned int packetID, stRedPacketInfo & packetInfo)
{
	if (!packetID)
	{
		LOG_PRINT(log_error, "[getRedPacketBasicInfo]packetID is 0.");
		return -1;
	}

	std::stringstream strSelectSQL;
	strSelectSQL << "select group_id,src_user,dst_user,type,range_type,packet_num,take_num,create_time,fix_per_money,packet_money,take_money,message,is_recycle,finish_time,private_flag,range_gender,invalid_time from talk_redpacket_info where id=" << packetID;

	int ret = 0;
	Result res;
	int row = 0;
	do
	{
		if (!build_sql_run(&res, &row, strSelectSQL.str().c_str()))
		{
			ret = -1;
			break;
		}

		if (0 == row)
		{
			LOG_PRINT(log_error, "DB do not have this packet basic info.packetID:%u.", packetID);
			ret = -1;
			break;
		}

		packetInfo.packetID = packetID;
		packetInfo.moneyState = 0;
		GetFromRes(packetInfo.groupID, &res, 0, 0);
		GetFromRes(packetInfo.srcUserID, &res, 0, 1);
		GetFromRes(packetInfo.dstUserID, &res, 0, 2);
		GetFromRes(packetInfo.packetType, &res, 0, 3);
		GetFromRes(packetInfo.rangeType, &res, 0, 4);
		GetFromRes(packetInfo.packetNum, &res, 0, 5);
		GetFromRes(packetInfo.takeNum, &res, 0, 6);
		GetFromRes(packetInfo.createTime, &res, 0, 7);
		GetFromRes(packetInfo.fixPerMoney, &res, 0, 8);
		GetFromRes(packetInfo.packetMoney, &res, 0, 9);
		GetFromRes(packetInfo.takeMoney, &res, 0, 10);
		GetFromRes(packetInfo.strMessage, &res, 0, 11);
		GetFromRes(packetInfo.isRecycle, &res, 0, 12);
		GetFromRes(packetInfo.finishTime, &res, 0, 13);
		GetFromRes(packetInfo.isPrivate,	&res, 0, 14);
		GetFromRes(packetInfo.rangeGender,	&res, 0, 15);
		GetFromRes(packetInfo.invalidTime, &res, 0, 16);

		if (packetInfo.packetMoney <= packetInfo.takeMoney)
		{
			LOG_PRINT(log_warning, "red packet has no left to take.packetID:%u,packet money:%llu,take money:%llu.", packetID, packetInfo.packetMoney, packetInfo.takeMoney);
			packetInfo.moneyState = 1;
		}

	} while (0);
	CleanRes(&res);
	return ret;
}

long long CDBSink::takeRandomRedPacket(unsigned int packetID, unsigned int packetType, unsigned int groupID, unsigned int take_userID, unsigned long long rand_money, unsigned int taken_percent/* = 0 */, unsigned int m_goldCandyExchange/* = 1 */)
{
	if (!packetID || !groupID || !take_userID || taken_percent > 100 || !m_goldCandyExchange)
	{
		LOG_PRINT(log_error, "[takeRandomRedPacket]input error.packetID:%u,packetType:%u,groupID:%u,take_userID:%u,rand_money:%llu,taken_percent:%u,m_goldCandyExchange:%u.", \
			packetID, packetType, groupID, take_userID, rand_money, taken_percent, m_goldCandyExchange);
		return -1;
	}

	LOG_PRINT(log_info, "[takeRandomRedPacket]input:packetID:%u,packetType:%u,groupID:%u,take_userID:%u,rand_money:%llu,taken_percent:%u,m_goldCandyExchange:%u.", \
		packetID, packetType, groupID, take_userID, rand_money, taken_percent, m_goldCandyExchange);
	Result res;
	std::stringstream strFunSQL;
	strFunSQL << "select take_randompacket(" << packetID << "," << packetType << "," << groupID << "," << take_userID << "," << rand_money << "," << taken_percent << "," << m_goldCandyExchange << ")";
	int row = 0;
	long long ret = 0;
	do
	{
		if (!run_sql(strFunSQL.str().c_str(), &res, &row))
		{
			ret = -1;
			break;
		}

		GetFromRes(ret, &res, 0, 0);

	} while (0);
	CleanRes(&res);
	return ret;
}

long long CDBSink::takeUniRedPacket(unsigned int packetID, unsigned int packetType, unsigned int groupID, unsigned int take_userID, unsigned int taken_percent/* = 0 */, unsigned int m_goldCandyExchange/* = 1 */)
{
	if (!packetID || !groupID || !take_userID || taken_percent > 100 || !m_goldCandyExchange)
	{
		LOG_PRINT(log_error, "[takeUniRedPacket]input error.packetID:%u,packetType:%u,groupID:%u,take_userID:%u,taken_percent:%u,m_goldCandyExchange:%u.", packetID, packetType, groupID, take_userID, taken_percent, m_goldCandyExchange);
		return -1;
	}

	LOG_PRINT(log_info, "[takeUniRedPacket]input:packetID:%u,packetType:%u,groupID:%u,take_userID:%u,taken_percent:%u,m_goldCandyExchange:%u.", packetID, packetType, groupID, take_userID, taken_percent, m_goldCandyExchange);
	Result res;
	std::stringstream strFunSQL;
	strFunSQL << "select take_unipacket(" << packetID << "," << packetType << "," << groupID << "," << take_userID << "," << taken_percent << "," << m_goldCandyExchange << ")";
	int row = 0;
	long long ret = 0;
	do
	{
		if (!run_sql(strFunSQL.str().c_str(), &res, &row))
		{
			ret = -1;
			break;
		}

		GetFromRes(ret, &res, 0, 0);

	} while (0);
	CleanRes(&res);
	return ret;
}

long long CDBSink::takeDirectRedPacket(unsigned int packetID, unsigned int packetType, unsigned int groupID, unsigned int take_userID, unsigned int taken_percent/* = 0 */, unsigned int m_goldCandyExchange/* = 1 */)
{
	if (!packetID || !groupID || !take_userID || taken_percent > 100 || !m_goldCandyExchange)
	{
		LOG_PRINT(log_error, "[takeDirectRedPacket]input error.packetID:%u,packetType:%u,groupID:%u,take_userID:%u,taken_percent:%u,m_goldCandyExchange:%u.", packetID, packetType, groupID, take_userID, taken_percent, m_goldCandyExchange);
		return -1;
	}

	LOG_PRINT(log_info, "[takeDirectRedPacket]input:packetID:%u,packetType:%u,groupID:%u,take_userID:%u,taken_percent:%u,m_goldCandyExchange:%u.", packetID, packetType, groupID, take_userID, taken_percent, m_goldCandyExchange);
	Result res;
	std::stringstream strFunSQL;
	strFunSQL << "select take_directpacket(" << packetID << "," << packetType << "," << groupID << "," << take_userID << "," << taken_percent << "," << m_goldCandyExchange << ")";
	int row = 0;
	long long ret = 0;
	do
	{
		if (!run_sql(strFunSQL.str().c_str(), &res, &row))
		{
			ret = -1;
			break;
		}

		GetFromRes(ret, &res, 0, 0);

	} while (0);
	CleanRes(&res);
	return ret;
}

int CDBSink::getRedPacketLog(uint32 packetID, std::vector<stRedPacketLog> &vLog, uint32 limit/* = 0*/, uint32 offset/* = 0*/)
{
	if (!packetID)
	{
		LOG_PRINT(log_error, "getRedPacketLog check input parameter failed: packetID: %u.", packetID);
		return 0;
	}

	std::stringstream strSql;
	/*给客户端的take_money要乘以100*/
	strSql << "select take_user, take_money * 100, time from mc_redpacket_log where packet_id = " << packetID << " order by time limit " << (limit ? limit : 20) << " offset " << offset;
	int row = 0;
	Result res;
	do
	{
		if (!run_sql(strSql.str().c_str(), &res, &row) || 0 == row)
		{
			break;
		}

		vLog.resize(row);
		for (int i = 0; i < row; i++)
		{
			vLog[i].packetId = packetID;
			GetFromRes(vLog[i].userId, &res, i, 0);
			GetFromRes(vLog[i].money, &res, i, 1);
			GetFromRes(vLog[i].time, &res, i, 2);
		}
	} while (0);

	CleanRes(&res);
	return row;
}

int CDBSink::getRedPacketToRecycle(unsigned int timeout, std::vector<stRedPacketInfo> &vList)
{
	if (timeout <= 0)
		return 0;

	std::stringstream sql;
	sql << "select id, group_id,src_user,dst_user from talk_redpacket_info where ((create_time <= UNIX_TIMESTAMP()  - " << timeout;
	sql << "  and type<7 ) or ( invalid_time <= UNIX_TIMESTAMP()   ";
	sql << "  and (type  =7 or type = 8 or type=9 or type=10)))and is_recycle = 0";
	int row = 0;
	Result res;
	do
	{
		if (!run_sql(sql.str().c_str(), &res, &row) || 0 == row)
			break;

		vList.resize(row);
		for (int i = 0; i < row; i++)
		{
			int j = 0;
			GetFromRes(vList[i].packetID, &res, i, j++);
			GetFromRes(vList[i].groupID, &res, i, j++);
			GetFromRes(vList[i].srcUserID, &res, i, j++);
			GetFromRes(vList[i].dstUserID, &res, i, j++);
		}
	} while (0);

	CleanRes(&res);

	return row;
}

bool CDBSink::recycleOneRedPacket(unsigned int packetID, unsigned int goldCandyExchange, int &result)
{
	if (!packetID || !goldCandyExchange)
	{
		LOG_PRINT(log_error, "recycleOneRedPacket input error.packetID:%u,goldCandyExchange:%u.", packetID, goldCandyExchange);
		return false;
	}

	std::stringstream sql;
	sql << "select recycle_redpacket(" << packetID << "," << goldCandyExchange << ")";
	int row = 0;
	Result res;
	if (!run_sql(sql.str().c_str(), &res, &row) || 0 == row)
	{
		CleanRes(&res);
		return false;
	}

	GetFromRes(result, &res, 0, 0);
	CleanRes(&res);
	return true;
}
bool CDBSink::getWeekCharmUserLst(unsigned int groupid, std::list<stContributeValue> & oContributeLst)
{
	if (!groupid)
	{
		LOG_PRINT(log_error, "getWeekCharmUserLst input error,groupid is 0");
		return false;
	}

	std::stringstream strQuerySQL;
	strQuerySQL << " select user_id,(charm_week+charm_today) from mc_group_member where group_id=" << groupid;
	strQuerySQL << " and is_del=0 and (charm_week+charm_today) != 0 order by (charm_week+charm_today) desc limit 3";
	int row = 0;
	Result res;
	if (!run_sql(strQuerySQL.str().c_str(), &res, &row) || 0 == row)
	{
		CleanRes(&res);
		return false;
	}

	for (int i = 0; i < row; ++i)
	{
		stContributeValue oUser;
		GetFromRes(oUser.userID, &res, i, 0);
		GetFromRes(oUser.near_week_contribute, &res, i, 1);
		oUser.groupID = groupid;
		oContributeLst.push_back(oUser);
	}

	CleanRes(&res);
	return true;
}

bool CDBSink::getWeekContributeUserLst(unsigned int groupid, std::list<stContributeValue> & oContributeLst)
{
	if (!groupid)
	{
		LOG_PRINT(log_error, "getWeekContributeUserLst input error,groupid is 0");
		return false;
	}

	std::stringstream strQuerySQL;
	strQuerySQL << " select user_id,(contribution_week+contribution_today) from mc_group_member where group_id=" << groupid;
	strQuerySQL << " and is_del=0 and (contribution_week+contribution_today) != 0 order by (contribution_week+contribution_today) desc limit 3";
	int row = 0;
	Result res;
	if (!run_sql(strQuerySQL.str().c_str(), &res, &row) || 0 == row)
	{
		CleanRes(&res);
		return false;
	}

	for (int i = 0; i < row; ++i)
	{
		stContributeValue oUser;
		GetFromRes(oUser.userID, &res, i, 0);
		GetFromRes(oUser.near_week_contribute, &res, i, 1);
		oUser.groupID = groupid;
		oContributeLst.push_back(oUser);
	}

	CleanRes(&res);
	return true;
}

long long CDBSink::getUserTakeMoney(unsigned int userID, unsigned int packetID)
{
	if (!userID || !packetID)
	{
		LOG_PRINT(log_error, "getUserTakeMoney input error,userID:%u,packetID:%u.", userID, packetID);
		return 0;
	}

	std::stringstream strQuerySQL;
	/*给客户端的take_money要乘以100*/
	strQuerySQL << "select take_money * 100 from mc_redpacket_log where take_user=" << userID << " and packet_id=" << packetID;
	int row = 0;
	Result res;
	long long money = 0;
	do
	{
		if (!run_sql(strQuerySQL.str().c_str(), &res, &row))
		{
			break;
		}

		if (0 == row)
		{
			LOG_PRINT(log_warning, "this user have not taken this packet,userID:%u,packetID:%u.", userID, packetID);
			break;
		}

		GetFromRes(money, &res, 0, 0);

	} while (0);
	CleanRes(&res);
	return money;
}

bool CDBSink::getGroupGainPercent(uint32 groupid, uint32 userid, uint32 &percent)
{
	std::stringstream sql;
	sql << " select if(gain_switch=0, 0, (case when a.custom_gain is null then if(b.gain_userrange=1, 0, b.gain_percent) "/*游客*/
		"	       else if(a.custom_gain=-1, if(b.gain_userrange=0, 0, b.gain_percent), a.custom_gain) "/*正式成员*/
		"	       end)) as percent "
		"		from mc_group_member a right join mc_group_plugin_setting_detail b "
		"     on a.group_id = b.group_id "
		"    and a.user_id = " << userid <<
		"    and a.is_del = 0 "
		"  where b.group_id=" << groupid;
	int row = 0;
	Result res;
	if (!run_sql(sql.str().c_str(), &res, &row) || 0 == row)
	{
		CleanRes(&res);
		return false;
	}

	GetFromRes(percent, &res, 0, 0);
	CleanRes(&res);
	return true;
}

unsigned int CDBSink::getTopRedPacketTakeUserID(unsigned int packetID)
{
	unsigned int topuserid = 0;
	std::stringstream strSQL;
	strSQL << " select take_user from mc_redpacket_log where packet_id = " << packetID << " order by take_money desc limit 1";

	int row = 0;
	Result res;
	if (!run_sql(strSQL.str().c_str(), &res, &row) || 0 == row)
	{
		CleanRes(&res);
		return topuserid;
	}

	GetFromRes(topuserid, &res, 0, 0);
	CleanRes(&res);
	return topuserid;
}

bool CDBSink::check_user_has_takephoto(unsigned int userID, unsigned int packetID)
{
	if (!userID || !packetID)
	{
		LOG_PRINT(log_error, "check if user has taken photo input error.userID:%u,packetID:%u.", userID, packetID);
		return false;
	}

	bool ret = false;
	std::stringstream strQuerySQL;
	strQuerySQL << "select id from mc_redpacket_photo where packet_id=" << packetID << " and user_id=" << userID;
	int row = 0;
	Result res;
	do 
	{
		if (!run_sql(strQuerySQL.str().c_str(), &res, &row) || 0 == row)
		{
			break;
		}
		else
		{
			ret = true;
		}

	} while (0);
	CleanRes(&res);
	return ret;
}

bool CDBSink::updGroupMemberInfo(uint32 groupid, uint32 userid, const std::map<std::string, std::string> &mValues)
{
	if (!groupid || !userid || mValues.empty())
		return false;

	std::stringstream sql, sqlValues;
	buildSetValuesList(sqlValues, mValues);
	sql << " update mc_group_member set " << sqlValues.str() <<
		" where is_del = 0 " <<
		"   and group_id = " << groupid <<
		"   and user_id = " << userid;

	if (!run_sql(sql.str().c_str()))
		return false;

	return getAffectedRow() > 0;
}

bool CDBSink::updGroupBasicInfo(uint32 groupid, std::map<std::string, std::string> &mValues)
{
	if (!groupid || mValues.empty())
		return false;

	std::stringstream sql, sqlValues;
	buildSetValuesList(sqlValues, mValues);
	sql << " update mc_group set " << sqlValues.str() <<
		" where is_del = 0 and id = " << groupid;

	if (!run_sql(sql.str().c_str()))
		return false;

	return getAffectedRow() > 0;
}

bool CDBSink::updGroupPluginSetting(uint32 groupid, std::map<std::string, std::string> &mValues)
{
	if (!groupid || mValues.empty())
		return false;

	std::stringstream sql, sqlValues;
	buildSetValuesList(sqlValues, mValues);
	sql << " update mc_group_plugin_setting_detail set " << sqlValues.str() <<
		" where group_id = " << groupid;

	if (!run_sql(sql.str().c_str()))
		return false;

	return getAffectedRow() > 0;
}

bool CDBSink::updWhenGroupPChatThresholdMod(uint32 groupid, uint32 threshold, bool isMale)
{
	uint32 gender = (isMale ? 1: 2);
	string sql = stringFormat(" update mc_group_member a, mc_user b set a.private_chat_intimacy = %u " 
 							  "  where a.user_id = b.id "
							  "    and a.group_id = %u "
							  "    and b.gender = %u "
							  "    and a.private_chat_intimacy < %u ", threshold, groupid, gender, threshold);
	return run_sql(sql.c_str());
}

int CDBSink::getUserMinGroupPChatThreshold(uint32 userid, uint32 groupid)
{
	string sql = stringFormat(" select case b.gender when 1 then a.male_pchat_min_threshold else a.female_pchat_min_threshold end "
							  "   from mc_group_plugin_setting_detail a, mc_user b "
							  "  where a.group_id = %u "
							  "    and b.id = %u ", groupid, userid);
	int ret = -1;
	Result res;
	int row = 0;
	if (!run_sql(sql.c_str(), &res, &row) || 0 == row)
	{
		CleanRes(&res);
		return ret;
	}

	GetFromRes(ret, &res, 0, 0);
	CleanRes(&res);

	return ret;
}

bool CDBSink::getUnreadSysNoticeMsgList(uint32 userid, std::string tel, int8 devtype, std::string appVersion, std::map<uint32, StNoticeMsg_t> &mMsg, std::map<uint32, std::vector<StCondition_t> > &mCond)
{
	std::string strSql;
	strSql = stringFormat(
		" SELECT a.id, a.type, a.content, a.action, UNIX_TIMESTAMP(a.create_time), UNIX_TIMESTAMP(a.end_time), b.condition_type, b.condition_oper, "
		"        CASE condition_oper "
		"            WHEN \"in\" THEN exists "
		"                   (SELECT 1 FROM mc_notice_condition_value d "
		"                    WHERE d.notice_id = id "
		"                      AND d.value_id = cast(condition_value AS SIGNED) "
		"                      AND (CASE condition_type "
		"                               WHEN 1 THEN '%u' "			//userid
		"                               WHEN 2 THEN '%s' "			//telnum
		"                               WHEN 3 THEN '%d' "			//dev type
		"                               WHEN 4 THEN '%s' "			//app version
		"                               ELSE '' END) = d.value) "
		"            ELSE condition_value "
		"        END AS condition_value "
		" FROM mc_notice_msg a, mc_notice_condition b "
		" WHERE a.id = b.notice_id "
		"   AND a.id > ifnull((SELECT sys_msgid FROM mc_user_read_msgid c WHERE c.user_id = %u), 0) "	//userid
		"   AND a.end_time > now() "
		"   AND a.status = 1 order by id desc", userid, tel.c_str(), devtype, appVersion.c_str(), userid);

	bool ret = false;
	Result res;
	int row = 0;
	do
	{
		if (!run_sql(strSql.c_str(), &res, &row))
		{
			break;
		}
		ret = true;
		for (int i = 0; i < row; i++)
		{
			int c = 0;
			StNoticeMsg_t msg;
			GetFromRes(msg.msgid, &res, i, c++);
			GetFromRes(msg.msgType, &res, i, c++);
			GetFromRes(msg.content, &res, i, c++);
			GetFromRes(msg.action, &res, i, c++);
			GetFromRes(msg.createTime, &res, i, c++);
			GetFromRes(msg.endTime, &res, i, c++);
			mMsg[msg.msgid] = msg;
			
			StCondition_t cond;
			GetFromRes(cond.type, &res, i, c++);
			GetFromRes(cond.oper, &res, i, c++);
			GetFromRes(cond.value, &res, i, c++);
		}
	} while(0);

	CleanRes(&res);
	return ret;
}

int32 CDBSink::getUnreadSysNoticeMsgList(uint32 userid, std::vector<StNoticeMsg_t> &vList)
{
	std::string strSql;
	strSql = stringFormat(
		" SELECT a.id, a.type, a.content, a.action, UNIX_TIMESTAMP(a.create_time), UNIX_TIMESTAMP(a.end_time) FROM mc_notice_msg a "
		" WHERE a.id > ifnull((SELECT sys_msgid FROM mc_user_read_msgid c WHERE c.user_id = %u), 0) "	//userid
		"   AND (a.end_time > now() OR a.end_time is null) "
		"   AND a.status = 1 order by id desc ", userid);

	Result res;
	int row = 0;
	do 
	{
		if (!run_sql(strSql.c_str(), &res, &row) || row <= 0)
		{
			break;
		}

		vList.resize(row);
		for (int i = 0; i < row; i++)
		{
			int c = 0;
			GetFromRes(vList[i].msgid, &res, i, c++);
			GetFromRes(vList[i].msgType, &res, i, c++);
			GetFromRes(vList[i].content, &res, i, c++);
			GetFromRes(vList[i].action, &res, i, c++);
			GetFromRes(vList[i].createTime, &res, i, c++);
			GetFromRes(vList[i].endTime, &res, i, c++);
		}

	} while (0);

	CleanRes(&res);
	return row;
}

int32 CDBSink::getUnreadUserNoticeMsgList(uint32 userid, std::vector<StNoticeMsg_t> &vList)
{
	std::string strSql;
	strSql = stringFormat(
		" SELECT a.id, a.type, a.content, a.action, UNIX_TIMESTAMP(a.create_time) FROM mc_user_notice_msg a "
		" WHERE a.user_id = %u and a.id > ifnull((SELECT user_msgid FROM mc_user_read_msgid c WHERE c.user_id = %u), 0) and a.status = 1", userid, userid);

	Result res;
	int row = 0;
	do 
	{
		if (!run_sql(strSql.c_str(), &res, &row) || row <= 0)
		{
			break;
		}

		vList.resize(row);
		for (int i = 0; i < row; i++)
		{
			int c = 0;
			GetFromRes(vList[i].msgid, &res, i, c++);
			GetFromRes(vList[i].msgType, &res, i, c++);
			GetFromRes(vList[i].content, &res, i, c++);
			GetFromRes(vList[i].action, &res, i, c++);
			GetFromRes(vList[i].createTime, &res, i, c++);
		}

	} while (0);

	CleanRes(&res);
	return row;
}

bool CDBSink::chkUnreadSysNoticeMsgCondition(uint32 msgid, uint32 userid, std::string tel, int8 devtype, std::string appVersion)
{
	std::string strSql;
	strSql = stringFormat(
		" SELECT ifnull(min(CASE condition_oper "
		"                WHEN 'in' THEN EXISTS "
		"                       (SELECT 1 "
		"                        FROM mc_notice_condition_value d "
		"                        WHERE d.notice_id = b.notice_id "
		"                          AND d.value_id = cast(condition_value AS SIGNED) "
		"                          AND d.value = cmp_value) "
		"                WHEN '=' THEN cmp_value = condition_value "
		"                WHEN '!=' THEN cmp_value != condition_value "
		"                ELSE 0 "
		"            END), 1) "
		" FROM "
		"   (SELECT a.notice_id, "
		"           (CASE condition_type "
		"                WHEN 1 THEN '%u' "		//userid
		"                WHEN 2 THEN '%s' "		//telnum
		"                WHEN 3 THEN '%d' "		//os platform
		"                WHEN 4 THEN '%s' "		//app version
		"                ELSE '' "
		"            END) AS cmp_value, "
		"           condition_oper, "
		"           condition_value "
		"    FROM mc_notice_condition a "
		"    WHERE a.notice_id = %u) b "			//mc_notice_msg.id
		, userid, tel.c_str(), devtype, appVersion.c_str(), msgid);

	bool ret = false;
	Result res;
	int row = 0;
	do 
	{
		if (!run_sql(strSql.c_str(), &res, &row) || row <= 0)
			break;

		int value = 0;
		GetFromRes(value, &res, 0, 0);

		if (1 == value)
		{
			ret = true;
			break;
		}
	} while (0);

	CleanRes(&res);
	return ret;
}

void CDBSink::updReadNoticeMsgIdOfUser(uint32 userid, uint32 msgid, bool isUser /*= false*/)
{
	const char *field = isUser ? "user_msgid": "sys_msgid";
	std::string strSql;
	strSql = stringFormat(" update mc_user_read_msgid set %s = if(%u>%s,%u,%s) where user_id = %u ",
		field, msgid, field, msgid, field, userid);

	if (!run_sql(strSql.c_str()) || getAffectedRow() <= 0)
	{
		strSql = stringFormat(" insert into mc_user_read_msgid(user_id, %s) values(%u, %u)", field, userid, msgid);
		run_sql(strSql.c_str());
	}
}

int CDBSink::getAllUnhandledPushMsg(const int beginMinutesSinceNow, const int endMinutesSinceNow, std::vector<uint32> &msgIds)
{
	Result res;
	int row = 0;
	std::string strbeg = getTime_H("%Y-%m-%d %H:%M:%S",beginMinutesSinceNow);
	std::string strend = getTime_H("%Y-%m-%d %H:%M:%S",endMinutesSinceNow);
	std::stringstream strSQL;

	strSQL << "SELECT id from talk_push_message where push_type = 2 and `status` = 0 and push_time BETWEEN \'"
		<< strbeg << "\' and \'"<< strend << "\'";

	msgIds.clear();

	if(!run_sql(strSQL.str().c_str(), &res, &row))
	{
		LOG_PRINT(log_error, "run_sql() failed: %s", strSQL.str().c_str());
		return -1;
	}

	for (int i = 0; i < row; i++)
	{
		uint32 id = 0;
		GetFromRes(id, &res, i, 0);
		msgIds.push_back(id);
	}

	CleanRes(&res);

	return row;
}

void CDBSink::buildSetValuesList(std::stringstream &sqlValues, const std::map<std::string, std::string> &mValues)
{
	for (std::map<std::string, std::string>::const_iterator it = mValues.begin(); it != mValues.end(); it++)
	{
		if (sqlValues.str().length() > 0)
			sqlValues << ",";
		sqlValues << it->first << " = '" << it->second << "'";
	}
}

std::string CDBSink::buildSendRedPacketSQL(const stRedPacketInfo & oPacketReq, unsigned int gold_contribute_exchange/* = 1*/)
{
	//function input:send_userid,dst_userid,packetMoney,rangeType,packetNum,fixPerMoney,packet_type,packet_groupid,packetMessage
	std::stringstream strFunSQL;
	strFunSQL << "select useraccount_sendpacket(";
	strFunSQL << oPacketReq.srcUserID << ",";
	strFunSQL << oPacketReq.dstUserID << ",";
	strFunSQL << oPacketReq.packetMoney << ",";
	strFunSQL << oPacketReq.rangeType << ",";
	strFunSQL << oPacketReq.rangeGender << ",";
	strFunSQL << oPacketReq.packetNum << ",";
	strFunSQL << oPacketReq.fixPerMoney << ",";
	strFunSQL << oPacketReq.packetType << ",";
	strFunSQL << oPacketReq.groupID << ",";
	strFunSQL << "'" << oPacketReq.strMessage << "',";
	strFunSQL << gold_contribute_exchange << ",";
	strFunSQL << oPacketReq.createTime << ",";
	strFunSQL << oPacketReq.invalidTime << ",";
	strFunSQL << (oPacketReq.isPrivate ? 1: 0);
	strFunSQL << ")";
	return strFunSQL.str();
}

bool CDBSink::modUserFriendship(uint32 userid, uint32 dstuid, int action)
{
	std::string sql;
	sql = stringFormat(" update mc_user_friendship set relation=%d where user_id = %u and friend_id = %u", action, userid, dstuid);
	if (!run_sql(sql.c_str()))
		return false;

	if (getAffectedRow() <= 0)
	{
		sql = stringFormat(" insert into mc_user_friendship(user_id, friend_id, relation) values(%u, %u, %d) ", userid, dstuid, action);
		return run_sql(sql.c_str());
	}

	return true;
}

int32 CDBSink::getUserFriendList(uint32 userid, std::vector<uint32> &list, int relation)
{
	std::string sql = stringFormat(" select friend_id from mc_user_friendship where user_id = %u and relation = %d", userid, relation);	

	Result res;
	int row = 0;
	do 
	{
		if (!run_sql(sql.c_str(), &res, &row) || 0 == row)
		{
			break;
		}

		list.resize(row);
		for (int i = 0; i < row; i++)
		{
			GetFromRes(list[i], &res, i, 0);
		}
	} while (0);

	CleanRes(&res);
	return row;
}

int CDBSink::getNotifySysConfig_DB(uint32 userid, int devtype, std::list<SysConfig_t > & sysconfigLst)
{
	int ret = 0;
	sysconfigLst.clear();
	std::stringstream strSQL;
	strSQL << " select paramname,paramvalue from mc_sys_parameter where status=1 and notifyall=1";

	Result res;
	int row = 0;
	do 
	{
		if (!run_sql(strSQL.str().c_str(), &res, &row) || row <= 0)
		{
			break;
		}

		for (int i = 0; i < row; i++)
		{
			SysConfig_t oSysConfigItem;
			int c = 0;
			GetFromRes(oSysConfigItem.paramName,	&res, i, c++);
			GetFromRes(oSysConfigItem.paramValue,	&res, i, c++);
			sysconfigLst.push_back(oSysConfigItem);
			++ret;
		}

	} while (0);

	CleanRes(&res);
	return ret;
}

bool CDBSink::getGroupUserIntimacy(uint32 userid, uint32 dstuid, uint32 groupid, uint32 &threshold, uint32 &intimacy)
{
	std::stringstream sql;
	sql << " SELECT um.private_chat_intimacy, ifnull(ur.intimacy,0) FROM mc_group_member um LEFT JOIN mc_user_relationship ur ";
	sql << "     on ur.group_id = um.group_id and ur.user_id = um.user_id and ur.relation_id = " << dstuid;
	sql << "  where um.group_id = " << groupid << " and um.user_id = " << userid;

	Result res;
	int row = 0;
	if (!run_sql(sql.str().c_str(), &res, &row))
	{
		CleanRes(&res);
		return false;
	}

	GetFromRes(threshold, &res, 0, 0);
	GetFromRes(intimacy, &res, 0, 1);

	CleanRes(&res);
	return true;
}
int CDBSink::getAppId2Key(std::map<string,string>& vak)
{
	vak.clear();
	string sql = "select appid , appkey from mc_appid2key";

	Result res;
	int row = 0;
	if (!run_sql(sql.c_str(), &res, &row))
	{
		CleanRes(&res);
		return -1;
	}
	for (int i = 0; i < row; i++)
	{
		App2Key_t ak;
		int c = 0;
		string appid ="";
		string key = "";
		GetFromRes(appid,	&res, i, c++);
		GetFromRes(key,	&res, i, c++);
		vak[appid] = key;
	}
	return vak.size();
}

int CDBSink::getUserFriendship(uint32 userid, uint32 friendid)
{
	int ret = -1;
	if (0 == userid || 0 == friendid)
		return ret;

	string sql = stringFormat("select relation from mc_user_friendship where user_id = %u and friend_id = %u", userid, friendid);
	Result res;
	int row;
	do
	{
		if (!run_sql(sql.c_str(), &res, &row))
			break;
		
		if (0 == row)
			return 0;

		GetFromRes(ret, &res, 0, 0);
	} while (0);
	CleanRes(&res);
	return ret;
}
int CDBSink::get_jiabing_DB(unsigned int userid, unsigned int groupid)
{
	if (!userid || !groupid)
	{
		LOG_PRINT(log_error, "DB get_jiabing_DB input error.userid:%u,groupid:%u.", userid, groupid);
		return -1;
	}

	unsigned int actualRoomId = (groupid < 1000000000) ? groupid : (groupid - 1000000000);
	std::stringstream strQuerySQL;
	strQuerySQL << "SELECT * FROM talk_invitationcard_user a ,talk_third_login b  WHERE a.get_user_id = b.open_id  AND a.create_card_class = "<<actualRoomId;
	strQuerySQL << " and b.user_id ="<<userid;
	strQuerySQL << " and a.type = 2";

	int ret = -1;
	Result res;
	int row = 0;

	do
	{
		if (!run_sql(strQuerySQL.str().c_str(), &res, &row))
		{
			LOG_PRINT(log_error, "query user group info fail in db.userid:%u,groupid:%u.", userid, actualRoomId);
			break;
		}

		if (row == 0)
		{
			LOG_PRINT(log_debug, "userid:%u is not jiabing of groupid:%u.", userid, actualRoomId);
			break;
		}

		ret = 0;
	} while (0);

	CleanRes(&res);
	return ret;
}
int CDBSink::get_token_DB(std::string& token,unsigned int& tokentime)
{
	std::stringstream strQuerySQL;
	strQuerySQL << "select access_token,access_expires_time from talk_access_token a ";

	int ret = -1;
	Result res;
	int row = 0;

	do
	{
		if (!run_sql(strQuerySQL.str().c_str(), &res, &row))
		{
			LOG_PRINT(log_error, "get_token_DB fail in db.");
			break;
		}

		if (row == 0)
		{
			LOG_PRINT(log_error, "userid:%u is not member of groupid:%u.");
			break;
		}
		GetFromRes(token, &res, 0, 0);
		GetFromRes(tokentime, &res, 0, 1);
		ret = 0;
	} while (0);

	CleanRes(&res);
	return ret;
}
bool CDBSink::updateForbidUserChat(unsigned int groupid, int status)
{
	if (!groupid)
	{
		return false;
	}

	std::stringstream strUpdateSQL;
	strUpdateSQL << " update talk_course set talk_status =" << status << " where id=" << groupid;
	return run_sql(strUpdateSQL.str().c_str());
}
bool CDBSink::updateCourseFinish(unsigned int groupid)
{
	if (!groupid)
	{
		return false;
	}

	std::stringstream strUpdateSQL;
	strUpdateSQL << " update talk_course set status = 4 ,end_time = \'" << getTime("%Y-%m-%d %H:%M:%S") << "\' where id=" << groupid;
	return run_sql(strUpdateSQL.str().c_str());
}
bool CDBSink::delTalkRecommendLog(unsigned int logid)
{
	if (!logid)
	{
		return false;
	}

	std::stringstream strUpdateSQL;
	strUpdateSQL << " delete  from talk_recommend_log where id =" << logid;
	return run_sql(strUpdateSQL.str().c_str());
}
bool CDBSink::getAllBeginCourseBefore(int begin,int end,std::vector<uint32>& vcids)
{
	Result res;
	int row = 0;
	std::string strbeg = getTime_H("%Y-%m-%d %H:%M:%S",begin);
	std::string strend = getTime_H("%Y-%m-%d %H:%M:%S",end);
	std::stringstream strSQL;
	strSQL << "select id from talk_course a where a.status < 4 and a.begin_time BETWEEN \'" << strbeg << "\' and \'"<<strend << "\'";
	LOG_PRINT(log_info, "room_tag getAllYesterDayCourse :%s",strSQL.str().c_str());
	do
	{
		if(!run_sql(strSQL.str().c_str(), &res, &row))
		{
			
			break;
		}
		if (row == 0)
		{
			LOG_PRINT(log_debug, "getAllYesterDayCourse = 0");
			break;
		}
		for (int i = 0; i < row; i++)
		{
			uint32 gid = 0;
			GetFromRes(gid,	&res, i, 0);
			vcids.push_back(gid);
		}
	}while(0);
	CleanRes(&res);
	return row;
}
bool CDBSink::getAllBeginCourseBefore(int begin,int end,std::vector<uint32>& vcids,std::vector<uint32>& uids,
	std::vector<std::string>& classNames, std::vector<std::string>& teacherNames, std::vector<std::string> &beginTimestamps)
{
	Result res;
	int row = 0;
	std::string strbeg = getTime_H("%Y-%m-%d %H:%M:%S",begin);
	std::string strend = getTime_H("%Y-%m-%d %H:%M:%S",end);
	std::stringstream strSQL;
	strSQL << "select a.id, a.uid, a.class_name, b.alias, a.begin_time from talk_course a, talk_user b where b.user_id = a.uid and a.status < 4 and a.begin_time BETWEEN \'"
		<< strbeg << "\' and \'"<< strend << "\'";
	LOG_PRINT(log_info, "room_tag getAllYesterDayCourse :%s",strSQL.str().c_str());
	do
	{
		if(!run_sql(strSQL.str().c_str(), &res, &row))
		{

			break;
		}
		if (row == 0)
		{
			LOG_PRINT(log_debug, "getAllYesterDayCourse = 0");
			break;
		}
		char buf[1024];
		for (int i = 0; i < row; i++)
		{
			uint32 gid = 0;
			GetFromRes(gid,	&res, i, 0);
			vcids.push_back(gid);

			uint32 uid = 0;
			GetFromRes(uid,	&res, i, 1);
			uids.push_back(uid);

			memset(buf, 0, sizeof(buf));
			GetFromRes(buf,	&res, i, 2, sizeof(buf));
			classNames.push_back(buf);

			memset(buf, 0, sizeof(buf));
			GetFromRes(buf,	&res, i, 3, sizeof(buf));
			teacherNames.push_back(buf);

			memset(buf, 0, sizeof(buf));
			GetFromRes(buf,	&res, i, 4, sizeof(buf));
			beginTimestamps.push_back(buf);
		}
	}while(0);
	CleanRes(&res);
	return row;
}
bool CDBSink::getRobotNamebyLiveid(uint32 liveid,std::vector<std::string>& vRobots)
{
	Result res;
	int row = 0;
	std::stringstream strSQL;
	strSQL << "select name from talk_live_focus a where robot = 1 and live_id = " << liveid;
	LOG_PRINT(log_info, "getRobotNamebyLiveid :%s",strSQL.str().c_str());
	do
	{
		if(!run_sql(strSQL.str().c_str(), &res, &row))
		{
			LOG_PRINT(log_error, "get talk_course fail in db.");
			break;
		}
		if (row == 0)
		{
			LOG_PRINT(log_error, "getRobotNamebyLiveid = 0");
			break;
		}
		for (int i = 0; i < row; i++)
		{
			std::string name = "";
			GetFromRes(name,&res, i, 0);
			vRobots.push_back(name);
		}
	}while(0);
	CleanRes(&res);
	return row;
}
bool CDBSink::updateRobots(uint32 liveid,uint32 count)
{
	if (!liveid)
	{
		return false;
	}

	string sql = stringFormat("update talk_live a set a.focus_num = a.focus_num + %d where id = %d",count,liveid);
	LOG_PRINT(log_info, "room_tag updateRobots :%s",sql.c_str());
	return run_sql(sql.c_str());
}
bool CDBSink::insertLiveFocusRobot(uint32 liveid,std::string name)
{
	if (!liveid)
	{
		return false;
	}

	string sql = stringFormat("insert into talk_live_focus(live_id,user_id,create_time,robot,name) values(%d,0,'%s',1,'%s')",
			liveid, getTime("%Y-%m-%d %H:%M:%S").c_str(),name.c_str());
	LOG_PRINT(log_info, "room_tag insertLiveFocusRobot :%s",sql.c_str());
	return run_sql(sql.c_str());
}
bool CDBSink::getAllLiveidAndCourseCount(std::map<uint32,uint32> & mapgids)
{
	Result res;
	int row = 0;
	std::string yesterday = getTime("%Y-%m-%d",-24);
	std::stringstream strSQL;
	strSQL << "select live_id,count(id) from talk_course a  GROUP BY live_id ";
	LOG_PRINT(log_info, "getAllYesterDayCourse :%s",strSQL.str().c_str());
	do
	{
		if(!run_sql(strSQL.str().c_str(), &res, &row))
		{
			LOG_PRINT(log_error, "get talk_course fail in db.");
			break;
		}
		if (row == 0)
		{
			LOG_PRINT(log_error, "getAllYesterDayCourse = 0");
			break;
		}
		for (int i = 0; i < row; i++)
		{
			uint32 liveid = 0;
			uint32 count = 0;
			GetFromRes(liveid,	&res, i, 0);
			GetFromRes(count,	&res, i, 1);
			mapgids[liveid] = count;
		}
	}while(0);
	CleanRes(&res);
	return row;
}
bool CDBSink::getAllYesterDayLiveidAndCourseCount(std::map<uint32,uint32> & mapgids)
{
	Result res;
	int row = 0;
	std::string yesterday = getTime("%Y-%m-%d",-24);
	std::stringstream strSQL;
	strSQL << "select live_id,count(id) from talk_course a where a.status < 5 and a.begin_time BETWEEN \'" << yesterday << " 00:00:00\' and \'"<<yesterday << " 23:59:59\' ";
	strSQL << " GROUP BY live_id ";
	LOG_PRINT(log_info, "getAllYesterDayCourse :%s",strSQL.str().c_str());
	do
	{
		if(!run_sql(strSQL.str().c_str(), &res, &row))
		{
			LOG_PRINT(log_error, "get talk_course fail in db.");
			break;
		}
		if (row == 0)
		{
			LOG_PRINT(log_error, "getAllYesterDayCourse = 0");
			break;
		}
		for (int i = 0; i < row; i++)
		{
			uint32 liveid = 0;
			uint32 count = 0;
			GetFromRes(liveid,	&res, i, 0);
			GetFromRes(count,	&res, i, 1);
			mapgids[liveid] = count;
		}
	}while(0);
	CleanRes(&res);
	return row;
}
unsigned int CDBSink::getRedPacketLogByTakeUserAndID(unsigned int packetID, unsigned int takeUser)
{
	unsigned int status = 0;
	std::stringstream strSQL;
	strSQL << " select 1 from talk_redpacket_log where packet_id = " << packetID << " and take_user=" << takeUser;

	int row = 0;
	Result res;
	if (!run_sql(strSQL.str().c_str(), &res, &row) || 0 == row)
	{
		CleanRes(&res);
		return 0;
	}

	GetFromRes(status, &res, 0, 0);
	CleanRes(&res);
	return status;
}
bool CDBSink::getAllYesterDayCourse(std::vector<uint32> & vgids)
{
	Result res;
	int row = 0;
	std::string yesterday = getTime("%Y-%m-%d",-24);
	std::stringstream strSQL;
	strSQL << "select id from talk_course a where a.status < 4 and a.begin_time BETWEEN \'" << yesterday << " 00:00:00\' and \'"<<yesterday << " 23:59:59\'";
	LOG_PRINT(log_info, "getAllYesterDayCourse :%s",strSQL.str().c_str());
	do
	{
		if(!run_sql(strSQL.str().c_str(), &res, &row))
		{
			LOG_PRINT(log_error, "get talk_course fail in db.");
			break;
		}
		if (row == 0)
		{
			LOG_PRINT(log_error, "getAllYesterDayCourse = 0");
			break;
		}
		for (int i = 0; i < row; i++)
		{
			uint32 gid = 0;
			GetFromRes(gid,	&res, i, 0);
			vgids.push_back(gid);
		}
	}while(0);
	CleanRes(&res);
	return row;
}
bool CDBSink::getTotalTipAmount(uint32 groupid,std::list<tipUserinfo>& infolst)
{
	Result res;
	int row = 0;
	std::stringstream strSQL;
	strSQL << "select a.user_id,a.total from talk_admire_rank a , talk_live b where a.master_userid = b.user_id and b.id = " << groupid ;
	LOG_PRINT(log_info, "getTotalTipAmount :%s",strSQL.str().c_str());
	do
	{
		if(!run_sql(strSQL.str().c_str(), &res, &row))
		{
			LOG_PRINT(log_error, "get talk_course fail in db.");
			break;
		}
		if (row == 0)
		{
			LOG_PRINT(log_error, "getAllYesterDayCourse = 0");
			break;
		}
		for (int i = 0; i < row; i++)
		{
			tipUserinfo info;
			GetFromRes(info.srcid,	&res, i, 0);
			GetFromRes(info.amount,	&res, i, 1);
			infolst.push_back(info);
		}
	}while(0);
	CleanRes(&res);
	return row;
}
bool CDBSink::updateOnlineRoomUserCount(uint32 liveid, uint32 count)
{
	if (!liveid)
	{
		return false;
	}

	string sql = stringFormat("update talk_live  set  online_num =  %u   where id = %d", count, liveid);
	//LOG_PRINT(log_info, "room_tag updateOnlineRoomUserCount :%s", sql.c_str());
	return run_sql(sql.c_str());
}


bool CDBSink::updateOnlineCourseUserCount(uint32 liveid, uint32 count)
{
	if (!liveid)
	{
		return false;
	}

	string sql = stringFormat("update talk_course  set  online_num =  %u   where id = %d", count, liveid);
	//LOG_PRINT(log_info, "room_tag updateOnlineRoomUserCount :%s", sql.c_str());
	return run_sql(sql.c_str());
}

int CDBSink::qrySysParameterInt(const char * paramname, const int defValue /*= 0*/)
{
	if (NULL == paramname || 0 == *paramname)
	{
		return defValue;
	}

	char szQuerySQL[256] = { 0 };
	sprintf(szQuerySQL, " select `value` from talk_config where `key` = '%s'  ", paramname);

	Result res;
	int row;
	if (!run_sql(szQuerySQL, &res, &row, false) || 0 == row)
	{
		CleanRes(&res);
		return defValue;
	}

	int ret;
	GetFromRes(ret, &res, 0, 0);
	CleanRes(&res);

	return ret;
}

std::string CDBSink::qrySysParameterString(const char *paramname, const char *defValue /*= ""*/)
{
	if (NULL == paramname || 0 == *paramname)
	{
		return defValue;
	}

	char szQuerySQL[256] = { 0 };
	sprintf(szQuerySQL, " select `value` from talk_config where `key` = '%s'  ", paramname);

	Result res;
	int row;
	if (!run_sql(szQuerySQL, &res, &row, false) || 0 == row)
	{
		CleanRes(&res);
		return defValue;
	}

	char szValue[129] = { 0 };
	GetFromRes(szValue, &res, 0, 0, sizeof(szValue) - 1);
	CleanRes(&res);

	return szValue;
}

int CDBSink::getPreloadedSysParamInt(const char *paramname, int &result)
{
	string strResult;

	if (getPreloadedSysParamString(paramname, strResult) < 0)
		return -1;

	result = atoi(strResult.c_str());

	return 0;
}

int CDBSink::getPreloadedSysParamString(const char *paramname, string &result)
{
	if (NULL == paramname)
	{
		LOG_PRINT(log_error, "null paramname");
		return -1;
	}

	string key(paramname);
	map<string, string>::iterator it = s_preloadedConfigParams.find(key);

	if (it != s_preloadedConfigParams.end())
	{
		result = it->second;
		return 0;
	}

	threadMutex.lock();

	// Checks again in case that another instance has loaded the param value into
	// the map during this instance waiting for the lock.
	if ((it = s_preloadedConfigParams.find(key)) != s_preloadedConfigParams.end())
	{
		threadMutex.unlock();
		result = it->second;
		return 0;
	}
	LOG_PRINT(log_debug, "Value of %s is not in the preloaded map, load it now", paramname);

	char sql[256] = { 0 };
	Result res;
	int row = 0;
	char value[128] = { 0 };

	snprintf(sql, sizeof(sql), "select `value` from talk_config where `key` = '%s'", paramname);

	if (!run_sql(sql, &res, &row, false) || 0 == row)
	{
		threadMutex.unlock();
		CleanRes(&res);
		LOG_PRINT(log_error, "failed to query sys param, key = %s", paramname);
		return -1;
	}

	GetFromRes(value, &res, 0, 0, sizeof(value) - 1);
	CleanRes(&res);

	s_preloadedConfigParams.insert(std::make_pair(key, value));
	result = value;
	LOG_PRINT(log_debug, "Finished loading value of %s: %s", paramname, result.c_str());

	threadMutex.unlock();

	return 0;
}

bool CDBSink::qryUserAssistantType(unsigned int userid, unsigned int liveid)
{
	 
	char szQuerySQL[256] = { 0 };
	if (liveid > BROADCAST_ROOM_BEGIN_NUM)
		sprintf(szQuerySQL, " select 1 from talk_live a inner join talk_user_assistant b on a.user_id= b.teacher_id and a.id= %u and b.user_id=%u ", liveid - BROADCAST_ROOM_BEGIN_NUM, userid);
	else
		sprintf(szQuerySQL, " select 1 from talk_course a inner join talk_user_assistant b on a.uid= b.teacher_id and a.id= %u and b.user_id=%u ", liveid, userid);


	Result res;
	int row;
	if (!run_sql(szQuerySQL, &res, &row, true) || 0 == row)
	{
		CleanRes(&res);
		return false;
	}	 
	CleanRes(&res);
	return true;
}

bool CDBSink::updateStatistics(unsigned int userid, unsigned int liveid, unsigned int type, unsigned int staticTime, unsigned int ntotal)
{
	
	string sql = stringFormat("select 1 from talk_live_statistics where user_id = %u and live_id=%u and statistics_type=%u and statistics_time=%u ", userid, liveid, type, staticTime);
	Result res;
	int row;
	if (!run_sql(sql.c_str(), &res, &row, true))
	{
		CleanRes(&res);
		return false;
	}
	 
	if (0 == row)
	{
		sql = stringFormat("insert into talk_live_statistics(user_id ,live_id,statistics_type,total,statistics_time) values(%u,%u,%u,%u,%u)",
			userid, liveid, type, ntotal, staticTime);
	}
	else
	{
		sql = stringFormat("update talk_live_statistics set total = total + %u where user_id = %u and live_id=%u and statistics_type=%u and statistics_time=%u ", ntotal, userid, liveid, type, staticTime);

	}
	//LOG_PRINT(log_info, "room_tag updateOnlineRoomUserCount :%s", sql.c_str());
	return run_sql(sql.c_str());
}

int CDBSink::get_CourseByUserid(uint32 userid, std::vector<uint32>& vList)
{
	int row = 0;
	char query[256];	 
	sprintf(query, "select id from talk_course where uid=%d ", userid);
	Result res;	
	if (!run_sql(query, &res, &row, false) || 0 == row)
	{
		CleanRes(&res);
		return -1;
	}
	for (int i = 0; i < row; i++)
	{
		uint32 id;
		GetFromRes(id, &res, i, 0);
		vList.push_back(id);		 
	}
	CleanRes(&res);
	return row;
} 
int CDBSink::get_UseridByCourseID(int32_t& userid, const int32_t liveid, const int32_t type)
{
	int row = 0;
	char query[256];
	if (type ==3)
		sprintf(query, "select uid from talk_course where  id=%d ", liveid);
	else
		sprintf(query, "select user_id from talk_live where  id=%d ", liveid);
	Result res;	
	if (!run_sql(query, &res, &row, false) || 0 == row)
	{
		CleanRes(&res);
		return -1;
	}	 
	GetFromRes(userid, &res, 0, 0);
	CleanRes(&res);
 
	return 0;
}
int CDBSink::get_LiveStatusByID(uint32 &status, uint32 liveid)
{
	int row = 0;
	char query[256];
	if (liveid > BROADCAST_ROOM_BEGIN_NUM)
		sprintf(query, "select open_status,user_id from talk_live where id=%d ", liveid - BROADCAST_ROOM_BEGIN_NUM);
	else
		sprintf(query, "select open_status,uid from talk_course where id=%d", liveid);
	Result res;
	Result res1;
	 
	if (!run_sql(query, &res, &row, false) || 0 == row)
	{
		CleanRes(&res);
		return 0;
	}

	uint32 userid;
	GetFromRes(status, &res, 0, 0);
	GetFromRes(userid, &res, 0, 1);

	CleanRes(&res);
	if (status == 1 && liveid < BROADCAST_ROOM_BEGIN_NUM)
	{

		sprintf(query, "select status from talk_user_permissions where user_id=%d and type=1", userid);
		if (!run_sql(query, &res1, &row, false) || 0 == row)
		{
			CleanRes(&res1);
			return 0;
		}
		GetFromRes(status, &res1, 0, 0);
		CleanRes(&res1);
	}
	return status;
}

int CDBSink::queryCourseStudents(uint32 courseId, std::set<uint32> &studentUserIds)
{
	studentUserIds.clear();
	if (courseId <= 0)
	{
		LOG_PRINT(log_error, "invalid courseId: %u", courseId);
		return -1;
	}

	int payRow = 0;
	Result payRes;
	string paySql = stringFormat("SELECT user_id from talk_pay_order WHERE type = 1 and class_id = %u", courseId);

	build_sql_run(&payRes, &payRow, paySql.c_str());
	for (int r = 0; r < payRow; ++r)
	{
		int payUserId = 0;

		GetFromRes(payUserId, &payRes, r, 0);
		studentUserIds.insert(payUserId);
	}

	CleanRes(&payRes);

	int joinRow = 0;
	Result joinRes;
	string joinSql = stringFormat("SELECT user_id from talk_user_join where course_id = %u", courseId);

	build_sql_run(&joinRes, &joinRow, joinSql.c_str());
	for (int r = 0; r < joinRow; ++r)
	{
		int joinUserId = 0;

		GetFromRes(joinUserId, &joinRes, r, 0);
		studentUserIds.insert(joinUserId);
	}

	CleanRes(&joinRes);

	return payRow + joinRow;
}

int CDBSink::querySerialCourseStudents(uint32 courseId, std::set<uint32> &studentUserIds)
{
	studentUserIds.clear();
	if (courseId <= 0)
	{
		LOG_PRINT(log_error, "invalid courseId: %u", courseId);
		return -1;
	}

	int payRow = 0;
	Result payRes;
	// 注1：系列课的购买是以主课为单位
	string paySql = stringFormat("SELECT user_id from talk_pay_order WHERE type = 1 and class_id = %u", courseId);

	build_sql_run(&payRes, &payRow, paySql.c_str());
	for (int r = 0; r < payRow; ++r)
	{
		int payUserId = 0;

		GetFromRes(payUserId, &payRes, r, 0);
		studentUserIds.insert(payUserId);
	}

	CleanRes(&payRes);

	int joinRow = 0;
	Result joinRes;
	// 注2：系列课的报名则是以具体子课为单位
	string joinSql = stringFormat("SELECT DISTINCT user_id from talk_user_join"
		" where course_id in (SELECT id from talk_course where pid = %u)", courseId);

	build_sql_run(&joinRes, &joinRow, joinSql.c_str());
	for (int r = 0; r < joinRow; ++r)
	{
		int joinUserId = 0;

		GetFromRes(joinUserId, &joinRes, r, 0);
		studentUserIds.insert(joinUserId);
	}

	CleanRes(&joinRes);

	return payRow + joinRow;
}

int CDBSink::queryFocusUsersByCourseId(uint32 courseId, std::set<uint32> &userIds)
{
	userIds.clear();
	if (courseId <= 0)
	{
		LOG_PRINT(log_error, "invalid courseId: %u", courseId);
		return -1;
	}

	int row = 0;
	Result res;
	string sql = stringFormat("SELECT b.user_id"
		" from talk_course a, talk_live_focus b"
		" where b.live_id = a.live_id and a.id = %u"
		" and b.user_id > 0", courseId);

	build_sql_run(&res, &row, sql.c_str());
	for (int r = 0; r < row; ++r)
	{
		int userId = 0;

		GetFromRes(userId, &res, r, 0);
		userIds.insert(userId);
	}

	CleanRes(&res);

	return row;
}

int CDBSink::queryCourseTypeAndParentId(const uint32 courseId, int &type, uint32 &parentCourseId)
{
	int row = 0;
	Result res;
	string sql = stringFormat("SELECT type, pid from talk_course where id = %d", courseId);
	char buf[4096] = {0};

	if (!build_sql_run(&res, &row, sql.c_str()) || 0 == row)
	{
		LOG_PRINT(log_error, "can not find course, id = %u", courseId);
		CleanRes(&res);
		return -1;
	}
	GetFromRes(type, &res, 0, 0);
	GetFromRes(parentCourseId, &res, 0, 1);

	CleanRes(&res);

	return 0;
}

int CDBSink::queryPptImageInfo(const char *qiniuId, int courseId, int &dbId, std::string &imgUrl)
{
	if (NULL == qiniuId)
	{
		LOG_PRINT(log_error, "null qiniuId");
		return -1;
	}

	int row = 0;
	Result res;
	string sql = stringFormat("SELECT id, imgUrl from talk_course_ppt_img where qiniuKey = '%s'", qiniuId);
	char buf[4096] = {0};

	if (!build_sql_run(&res, &row, sql.c_str()) || 0 == row)
	{
		CleanRes(&res);
		return -1;
	}
	GetFromRes(dbId, &res, 0, 0);
	GetFromRes(buf, &res, 0, 1, sizeof(buf));
	imgUrl = buf;

	CleanRes(&res);

	return 0;
}

int CDBSink::querySubCourseInfo(const uint32 subCourseId, uint32 &courseId, uint32 &userId, std::string &subCourseName, std::string &courseName, std::string &beginTime)
{
	if (subCourseId <= 0)
	{
		LOG_PRINT(log_error, "invalid subCourseId: %u", subCourseId);
		return -1;
	}

	int row = 0;
	Result res;
	string sql = stringFormat("SELECT sub.uid, parent.id, parent.class_name, sub.class_name, sub.begin_time"
		" from talk_course parent, talk_course sub"
		" where parent.id = sub.pid and sub.id = %d", subCourseId);
	char buf[4096] = {0};

	if (!build_sql_run(&res, &row, sql.c_str()) || 0 == row)
	{
		CleanRes(&res);
		return -1;
	}
	GetFromRes(userId, &res, 0, 0);
	GetFromRes(courseId, &res, 0, 1);
	GetFromRes(buf, &res, 0, 2, sizeof(buf));
	courseName = buf;
	memset(buf, 0, sizeof(buf));
	GetFromRes(buf, &res, 0, 3, sizeof(buf));
	subCourseName = buf;
	memset(buf, 0, sizeof(buf));
	GetFromRes(buf, &res, 0, 4, sizeof(buf));
	beginTime = buf;

	CleanRes(&res);

	return 0;
}

int CDBSink::queryCourseInfo(const uint32 courseId, uint32 &userId, uint32 &liveId, std::string &courseName, std::string &teacherAlias)
{
	if (courseId <= 0)
	{
		LOG_PRINT(log_error, "invalid courseId: %u", courseId);
		return -1;
	}

	int row = 0;
	Result res;
	string sql = stringFormat("SELECT a.uid, a.live_id, a.class_name, b.alias"
		" from talk_course a, talk_user b"
		" where b.user_id = a.uid and a.id = %d", courseId);
	char buf[4096] = {0};

	if (!build_sql_run(&res, &row, sql.c_str()) || 0 == row)
	{
		CleanRes(&res);
		return -1;
	}
	GetFromRes(userId, &res, 0, 0);
	GetFromRes(liveId, &res, 0, 1);
	GetFromRes(buf, &res, 0, 2, sizeof(buf));
	courseName = buf;
	memset(buf, 0, sizeof(buf));
	GetFromRes(buf, &res, 0, 3, sizeof(buf));
	teacherAlias = buf;

	CleanRes(&res);

	return 0;
}

int CDBSink::getLiveFlowCount(void)
{
	const char *sql = "select count(1) from talk_live_state";
	int ret = -1;
	Result res;

	if (!run_sql(sql, &res))
	{
		CleanRes(&res);
		return ret;
	}

	GetFromRes(ret, &res, 0, 0);
	CleanRes(&res);

	return ret;
}

int CDBSink::getLiveFlowGroupIds(int startRow, int count, std::vector<int> &groupIds)
{
	groupIds.clear();
	if (startRow < 0 || count <= 0)
		return -1;

	int row = 0;
	Result res;
	string sql = stringFormat("select groupid from talk_live_state order by id asc limit %d,%d", startRow, count);

	LOG_PRINT(log_debug, "room_tag updateLiveFlowState: %s", sql.c_str());
	if (!build_sql_run(&res, &row, sql.c_str()) || 0 == row)
	{
		CleanRes(&res);
		return 0;
	}

	int groupid;

	groupIds.resize(row);
	for (int r = 0; r < row; r++)
	{
		GetFromRes(groupid, &res, r, 0);
		groupIds[r] = groupid;
	}

	CleanRes(&res);

	return row;
}

bool CDBSink::updateLiveFlowState(int groupid, int state, time_t updatetime)
{
	if (groupid <= 0)
		return false;

	string sql = stringFormat("update talk_live_state set state = %d,"
		" updatetime = %d where groupid = %d",
		state, updatetime, groupid);

	LOG_PRINT(log_debug, "room_tag updateLiveFlowState: %s", sql.c_str());

	return run_sql(sql.c_str());
}



