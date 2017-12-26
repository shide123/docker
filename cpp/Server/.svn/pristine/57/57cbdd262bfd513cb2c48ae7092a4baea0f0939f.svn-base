#include "SL_Headers.h"
#include "DBTools.h"
#include "errcode.h"
#include "message_vchat.h"
#include "ConsumeSvr.pb.h"

Dbconnection * CDBTools::m_db = NULL;
Dbconnection * CDBTools::m_syncdb = NULL;

CDBTools::CDBTools()
{
}

CDBTools::~CDBTools()
{
}

bool CDBTools::modRoomManager(unsigned int vcbid, unsigned int userid, int mgrtype, bool bAdd /*= true*/)
{
	int row;
	if (bAdd)
	{
		if (!m_db->build_sql_run(NULL, &row, Dbconnection::SQL_s_exists_vcbmanager, vcbid, userid, mgrtype)
			|| (0 == row && !m_db->build_sql_run(NULL, NULL, Dbconnection::SQL_i_addvcbmanager, vcbid, userid, mgrtype)))
		{
			return false;
		}
	}
	else
	{
		if (!m_db->build_sql_run(NULL, NULL, Dbconnection::SQL_d_delvcbmanager, vcbid, userid, mgrtype))
		{
			return false;
		}
	}

	return true;
}

int CDBTools::getZhuanboInfoByMainRoom(int* list, unsigned int roomid, unsigned int userid)
{
	if (NULL == list || 0 == roomid || 0 == userid)
	{
		return 0;
	}
	Result res;
	int row = 0;
	char query[512];
	sprintf(query, "select nrobotvcbid from dks_vcbzhuanboconfig where nuserid=%d and nvcbid=%d ",userid,roomid);
	if (!m_db->run_sql(query, &res, &row))
	{
		m_db->CleanRes(&res);
		return -1;
	}

	for(int i = 0,j = 0; i<row; i++)
	{
		int temproomid=0;
		m_db->GetFromRes(temproomid, &res, i, 0);

		if(temproomid != 0 && temproomid != roomid)
		{
			list[j++]=temproomid;
		}
	}
	m_db->CleanRes(&res);

	return row;
}

int CDBTools::getMainroomByZhuanboInfo(unsigned int roomid, unsigned int userid)
{
	Result res;
	int row = 0;
	char query[256];
	sprintf(query, "select nvcbid,nuserid from dks_vcbzhuanboconfig where nrobotid=%d and nrobotvcbid=%d ",userid,roomid);
	if (!m_db->run_sql(query, &res, &row))
	{
		m_db->CleanRes(&res);
		return -1;
	}

	int temproomid=0;
	int tempuserid=0;
	for(int i = 0; i < row; i++)
	{
		m_db->GetFromRes(temproomid, &res, i, 0);
		if(temproomid!=0 && temproomid!=roomid)
		{
			m_db->GetFromRes(tempuserid, &res, i, 1);
			break;
		}
	}
	m_db->CleanRes(&res);

	return tempuserid;
}

int CDBTools::checkTeacherScore(unsigned int teacherid, unsigned int userid, char* logtime)
{
	char dtime[40];
	time_t t = toNumericTimestamp(logtime);
	t -= 1*60*60;
	toStringTimestamp(t,dtime);

	int row = 0;
	char query[512];
	sprintf(query, "select 1 from tbteacherscorelog where nteacherid=%d and nuserid=%d and dtime>='%s' ",teacherid , userid, dtime);
	if (!m_db->run_sql(query, NULL, &row))
	{
		return -1;
	}

	return row;
}

bool CDBTools::addTeacherScoreLog(CMDTeacherScoreRecord_t2* teacherlog)
{
	char query[4096] = {0};
	sprintf(query, "INSERT INTO tbteacherscorelog(nteacherid, cteacheralias, nuserid, calias, nscore, nvcbid, dtime) VALUES (%d, '%s', %d, '%s', %d, %d, '%s')",
		teacherlog->teacher_userid,
		teacherlog->teacheralias,
		teacherlog->userid,
		teacherlog->alias,
		teacherlog->score,
		teacherlog->vcbid,
		teacherlog->logtime);

	return m_db->run_sql(query);
}

int CDBTools::getTeacherGiftList(std::vector<CMDTeacherThisWeekFans_t> &vOut, unsigned int roomid,unsigned int teacherid)
{
	//~{<FKc1>V\R;JGDDR;Ll~}
	time_t lt =time(NULL);
	struct tm *ptr=localtime(&lt);
	int passDay = 0;
	if(ptr->tm_wday == 0){//~{Hg9{JGV\HU>MJG~}6~{Ll#,FdS`5DHUWS<uR;~}
		passDay = 6;
	} else{
		passDay = (ptr->tm_wday-1);
	}

	//~{V1=S;X5=IOR;8vV\R;Ac5c~}
	time_t lastTime = time(NULL) - (86400*passDay) - (ptr->tm_hour*3600) - (ptr->tm_min*60) - ptr->tm_sec;//1~{Ll~}(d)=86400~{Ck~}(s),1~{J1~}(h)=3600~{Ck~}(s),1~{7V~}(min)=60~{Ck~}(s)

	char szTime[32]={0};
	toStringDate(lastTime, szTime);


	//~{O^6(~}6~{Lu#,2bJTVP7">uUbLu~}sql~{So>dR;0c:DJ1~}180ms~{WsSR~}
	char query[512];
	sprintf(query, "select nuserid, sum(nusermoney) AS totalMoney from dks_usergifttradelog where nbuddyid=%d AND dtime>'%s' group by nuserid order by sum(nusermoney) desc limit 6", teacherid, szTime);

	int row = 0;
	Result res;
	if (!m_db->run_sql(query, &res, &row) || 0 == row)
	{
		m_db->CleanRes(&res);
		return 0;
	}

	vOut.resize(row);
	for(int r=0; r<row; r++)
	{
		Result res1;
		char usr_query[512];
		int row1 = 0;
		int tmp_user_id = 0;

		CMDTeacherThisWeekFans_t record;
		memset(&record, 0, sizeof(CMDTeacherThisWeekFans_t));

		m_db->GetFromRes(tmp_user_id, &res, r, 0);
		m_db->GetFromRes(record.totalmoney, &res, r, 1);

		sprintf(usr_query, "select calias from dks_user where nuserid=%d limit 1", tmp_user_id);
		if (!m_db->run_sql(usr_query, &res1, &row1) || 0 == row1)
		{
			m_db->CleanRes(&res1);
			row = r;
			vOut.resize(row);
			break;
		}

		m_db->GetFromRes(record.alias, &res, 0, 0, 32);
		vOut[r] = record;

		m_db->CleanRes(&res1);
	}
	m_db->CleanRes(&res);

	return row;
}

int CDBTools::teacherSubscript(unsigned int userid, unsigned int teacherid, bool bSub)
{
	char query[512] = {0};
	if ( bSub )
	{
		sprintf(query, "insert into dks_teachersubscription(nteacherid,nuserid) values(%d,%d)",teacherid,userid);
	}
	else
	{
		sprintf(query, "delete from dks_teachersubscription where nteacherid=%d and nuserid=%d",teacherid,userid);
	}

	return m_db->run_sql(query);
}

int CDBTools::getSubscriptState(unsigned int userid,unsigned int teacherid)
{
	int row = 0;
	char query[512];
	sprintf(query, "select 1 from dks_teachersubscription where nuserid=%d and nteacherid=%d", userid,teacherid);

	if (!m_db->run_sql(query, NULL, &row))
	{
		return 2;
	}

	return 0 == row ? 0: 1;
}

bool CDBTools::existAdKeyword(char *pKeyword)
{
	if (pKeyword == NULL || *pKeyword == 0)
		return false;

	int row = 0;
	char query[256];
	sprintf(query, "select keyname from dks_keywords where keyname='%s'", pKeyword);
	if (!m_db->run_sql(query, NULL, &row))
	{
		return false;
	}

	return row > 0;
}

bool CDBTools::addAdKeyword(CMDAdKeywordInfo_t *pKeyword)
{
	char   query[256];
	sprintf(query, "insert into dks_keywords(adtype,runerid,ctime,keyname) values(%d,%d,'%s','%s')",
		pKeyword->ntype, pKeyword->nrunerid, pKeyword->createtime, pKeyword->keyword);

	return m_db->run_sql(query);
}

bool CDBTools::delAdKeyword(CMDAdKeywordInfo_t *pKeyword)
{
	char query[256];
	sprintf(query, "delete from dks_keywords where keyname='%s'", pKeyword->keyword);
	return m_db->run_sql(query);
}

int CDBTools::getAdKeywordList(int startline, int linenum, std::vector<CMDAdKeywordInfo_t> &vKeyWord)
{
	int row = 0;
	Result res;
	char query[256];
//	sprintf(query, "select adtype, runerid, ctime, keyname,replace from dks_keywords limit %d offset %d", linenum, startline);
	sprintf(query, "select 1 ,operatorId,createTime,findpattern,replacement from talk_badword limit %d offset %d", linenum, startline);
	if (!m_db->build_sql_run(&res, &row, query) || 0 == row)
	{
		m_db->CleanRes(&res);
		return 0;
	}

	vKeyWord.resize(row);
	for(int r = 0; r < row; r++)
	{
		CMDAdKeywordInfo_t record;
		memset(&record, 0, sizeof(CMDAdKeywordInfo_t));
		m_db->GetFromRes(record.ntype,		&res, r, 0);
		m_db->GetFromRes(record.nrunerid,	&res, r, 1);
		m_db->GetFromRes(record.createtime, &res, r, 2, sizeof(record.createtime));
		m_db->GetFromRes(record.keyword,	&res, r, 3, sizeof(record.keyword));
		m_db->GetFromRes(record.replace,	&res, r, 4, sizeof(record.replace));
		vKeyWord[r] = record;
	}

	m_db->CleanRes(&res);

	return row;
}

int CDBTools::getAdKeywordList(int startline, int linenum, std::vector<std::string> &vKeyWord)
{
	int row = 0;
	Result res;
	char query[256];

	sprintf(query, "select keyword from talk_keyword limit %d offset %d", linenum, startline);

	if (!m_db->build_sql_run(&res, &row, query) || 0 == row)
	{
		m_db->CleanRes(&res);
		return 0;
	}

	vKeyWord.resize(row);
	for(int r = 0; r < row; r++)
	{
		char keyword[64] = {0};

		m_db->GetFromRes(keyword, &res, r, 0, sizeof(keyword));
		vKeyWord[r] = keyword;
	}

	m_db->CleanRes(&res);

	return row;
}

bool CDBTools::updateRoomBaseInfo(int vcbid, const DDRoomBaseInfo_t &pInfo)
{
	char query[1024];
	sprintf(query, "update dks_vcbinformation set ncreateid=%d,nop1id=%d,nop2id=%d,nop3id=%d,nop4id=%d,nusepwd=%d,cname='%s',cpassword='%s' where nvcbid=%d",
		pInfo.creatorid, pInfo.op1id, pInfo.op2id, pInfo.op3id, pInfo.op4id, pInfo.busepwd,pInfo.cname,pInfo.cpassword,pInfo.roomid);

	if (!m_db->run_sql(query))
	{
		return false;
	}

	return true;
}

int CDBTools::updateRoomOptStat(unsigned int vcbid, int nopstat)
{
	char query[256];
	sprintf(query, "update dks_vcbstatus set optstat = %d where nvcbid = %u", nopstat, vcbid);
	if (!m_db->run_sql(query))
	{
		return false;
	}
	return true;
}

bool CDBTools::isCollectRoom(int userid, int vcbid)
{
	char sql[256];
	sprintf(sql, "select nid from dks_uservcbfavorite where nuserid=%d and nvcbid=%d",userid,vcbid);

	int row;
	if (!m_db->run_sql(sql, NULL, &row))
	{
		return false;
	}

	return row > 0;
}

bool CDBTools::collectRoom(unsigned int userid, unsigned int vcbid, int action)
{
	bool bCollected = isCollectRoom(userid, vcbid);

	char sql[256];
	char upSql[256];
	sprintf(upSql, " update dks_vcbinformation a set total_attention=(select count(nuserid) from dks_uservcbfavorite where nvcbid=a.nvcbid) where nvcbid=%u ", vcbid);

	bool bAddButCollected = (1 == action && bCollected);		//collect action but user has collected
	bool bDelButNotCollected = (1 != action && !bCollected);	//unfavorite but user has not 
	if (bAddButCollected)
	{
		LOG_PRINT(log_warning, "user %u collects room %u repeatedly..", userid, vcbid);
		return false;
	}
	if (bDelButNotCollected)
	{
		LOG_PRINT(log_warning, "user %u hasn't collected room %u..", userid, vcbid);
		return false;
	}

	if (1 == action)
	{
		char szTime[32]={0};
		toStringTimestamp(time(0), szTime);
		sprintf(sql, "insert into dks_uservcbfavorite(nuserid,nvcbid,dtime) values(%d,%d,'%s')",userid, vcbid, szTime);
	}
	else 
	{
		sprintf(sql,"delete from dks_uservcbfavorite where nuserid=%d and nvcbid=%d",userid, vcbid);
	}

	if (m_db->run_sql(sql) && m_db->run_sql(upSql))
	{
		return true;
	}

	return false;
}

int CDBTools::get_push_type_and_token(unsigned int userid, std::vector<PushInfo_t> &lstOut)
{
	Result res;
	char query[256] = {0};
	int row;
	if (userid == 0) {
		sprintf(query, "select push_type, push_token from dcl_push_token_inf");
	}
	else {
		sprintf(query, "select push_type, push_token from dcl_push_token_inf where user_id=%u", userid);
	}

	if (!m_db->run_sql(query, &res, &row)){
		return -1;
	}

	for(int r = 0; r < row; r++) {
		PushInfo_t record;
		memset(&record, 0, sizeof(PushInfo_t));
		m_db->GetFromRes(record.pushtype, &res, r, 0);
		m_db->GetFromRes(record.pushtoken, &res, r, 1, sizeof(record.pushtoken));
		lstOut.push_back(record);
	}

	return 0;
}

int CDBTools::updateRoomNotice(unsigned int vcbid, unsigned int userid, const char* alias, int noticeid, char* szText)
{
	time_t tNow = time(0);
	char query[512] = {0};
	sprintf(query, "select 1 from dks_vcbbroadinfo where nindex = %d and nvcbid = %d limit 1", noticeid, vcbid);
	if (!m_db->run_sql(query))
	{
		return false;
	}

	if (m_db->getAffectedRow() > 0)
	{
		sprintf(query, "update dks_vcbbroadinfo set nuserid=%d, cname='%s', cbroad='%s', dcreatetime = FROM_UNIXTIME(%d) where nindex = %d and nvcbid = %d", userid, alias, szText ? szText :"", tNow, noticeid, vcbid);
	}
	else
	{
		sprintf(query, "insert into dks_vcbbroadinfo (nvcbid, nuserid, cname, nindex, cbroad, dcreatetime) values (%d, %d, '%s', %d, '%s', FROM_UNIXTIME(%d) )", vcbid, userid, alias, noticeid, szText ? szText: "", tNow);
	}

	if (!m_db->run_sql(query))
	{
		return false;
	}

	return true;
}

int CDBTools::func_tradegift_DB(unsigned int src_id, unsigned int dst_id, unsigned int gift_id, unsigned int gift_count, int gift_type, int64 total_out, int64 total_in, int trade_type, int room_id,\
								int64 * src_now_nk, int64 * src_now_nb, int64 * dst_now_nk, int64 * dst_now_nb, int * src_add_nchestnum, int * src_now_nchestnum)
{
	Result res;
	char query[1024] = {0};
	int row;
	int ret_sucess = 0;
	int ret_return = 0;

	query[0] = 0;

	sprintf(query, "select tradegift3(%d,%d,%d,%d,%d,%lld,%lld,%d,%d)", src_id, dst_id, gift_id, gift_count, gift_type, total_out, total_in, trade_type, room_id);

	do 
	{
		if (!m_db->run_sql(query, &res, &row))
		{
			ret_return = -1;
			break;
		}

		size_t result_len = m_db->GetResLen(&res, 0, 0);
		char * cResult = new char[result_len + 1];
		if (!cResult)
		{
			ret_return = -1;
			break;
		}
		memset(cResult, 0, result_len + 1);
		m_db->GetFromRes(cResult, &res, 0, 0, result_len);
		std::string strResult = cResult;
		if (cResult)
		{
			delete[] cResult;
			cResult = NULL;
		}

		std::vector<std::string> result_v;		
		LOG_PRINT(log_normal, "trade gift result:%s.", strResult.c_str());
		std::string::size_type begin_pos = strResult.find('(');
		if (std::string::npos != begin_pos)
		{
			std::string::size_type end_pos = strResult.find(')');
			if (std::string::npos != end_pos && end_pos > begin_pos + 1)
			{
				std::string::size_type len = end_pos - begin_pos - 1;
				std::string substr = strResult.substr(begin_pos + 1, len);
				result_v = split(substr, ",");
			}
		}

		if (result_v.empty())
		{
			LOG_PRINT(log_error, "gift result format is wrong.ret:%s.", strResult.c_str());
			ret_return = -1;
			break;
		}

		ret_sucess = atoi(result_v[0].c_str());
		if(ret_sucess == 0)
		{
			*src_now_nk = atoll(result_v[1].c_str());
			*src_now_nb = atoll(result_v[2].c_str());
			*dst_now_nk = atoll(result_v[3].c_str());
			*dst_now_nb = atoll(result_v[4].c_str());
			*src_add_nchestnum = 0;
			*src_now_nchestnum = 0;
			ret_return = 0;
		}
		else if(ret_sucess == 1)
		{
			*src_now_nk = atoll(result_v[1].c_str());
			*src_now_nb = atoll(result_v[2].c_str());
			*dst_now_nk = atoll(result_v[3].c_str());
			*dst_now_nb = atoll(result_v[4].c_str());
			*src_add_nchestnum = atoi(result_v[5].c_str());
			*src_now_nchestnum = atoi(result_v[6].c_str());
			ret_return = 1;
		}
		else
		{
			//here this user has no account or not enough money.
			ret_return = -2;
		}

	} while (0);

	m_db->CleanRes(&res);
	return ret_return;
}

//gifttype=2; tradetype:0-金币类礼物,1-普通礼物,2-幸运礼物(这里定义都是收积分)
int CDBTools::func_trade_normal_gift_DB(unsigned int src_id, unsigned int dst_id, unsigned int gift_id, unsigned int gift_count, int gift_type, int64 total_out, int64 total_in, int trade_type, int room_id,\
								 int64 * src_now_nk, int64 * src_now_nb, int64 * dst_now_nk, int64 * dst_now_nb, int * src_add_nchestnum, int * src_now_nchestnum)
{
	if (trade_type != 1)
	{
		LOG_PRINT(log_error, "trade_type is not equal 1, trade_type:%d.", trade_type);
		return -1;
	}

	Result res;
	int total_row = 0;
	int ret_return = 0;
	bool needRollBack = true;

	do 
	{
		m_db->transBegin();

		unsigned int count = 0;
		if (!m_db->build_sql_run(&res, &total_row, "select count(1) from dks_useraccount where nuserid=%u and nk>=%lld", src_id, total_out))
		{
			ret_return = -1;
			break;
		}

		m_db->GetFromRes(count, &res, 0, 0);
		if (count == 0)
		{
			LOG_PRINT(log_warning, "userid:%u has not enough money.", src_id);
			ret_return = -2;
			break;
		}
		m_db->CleanRes(&res);

		if (!m_db->build_sql_run(0, 0, "update dks_useraccount set nk=nk-%lld where nuserid=%u and nk>=%lld", total_out, src_id, total_out))
		{
			ret_return = -1;
			break;
		}

		if (!m_db->build_sql_run(0, 0, "update dks_useraccount set nb=nb+%lld where nuserid=%u ", total_in, dst_id))
		{
			ret_return = -1;
			break;
		}

		if (!m_db->build_sql_run(0, 0, "insert into dks_usergifttradelog(nuserid,nbuddyid,ngiftid,ncount,dtime,ngifttype,nvcbid,nusermoney,nbuddymoney) VALUES(%u,%u,%u,%u,FROM_UNIXTIME(%u),%d,%d,%lld,%lld)", \
								src_id, dst_id, gift_id, gift_count, (int)time(NULL), gift_type, room_id, total_out, total_in))
		{
			ret_return = -1;
			break;
		}

		time_t now_time = time(NULL);
		char cNowDay[LEN32] = {0};
		toStringDate(now_time, cNowDay);
		if (!m_db->build_sql_run(0, 0, "update tbuserdaycost set ncost=ncost+%lld where nuserid=%u and ddate='%s' ", total_out, src_id, cNowDay))
		{
			ret_return = -1;
			break;
		}

		if (0 == m_db->getAffectedRow())
		{
			if (!m_db->build_sql_run(0, 0, "insert into tbuserdaycost(nuserid,ncost,ddate) values(%u, %lld, '%s') ", src_id, total_out, cNowDay))
			{
				ret_return = -1;
				break;
			}
		}

		if (room_id > 0)
		{
			if (!m_db->build_sql_run(0, 0, "update tbvcbdaycost set ncost=ncost+%lld where nvcbid=%u and ddate='%s' ", total_out, room_id, cNowDay))
			{
				ret_return = -1;
				break;
			}

			if (0 == m_db->getAffectedRow())
			{
				m_db->build_sql_run(0, 0, "insert into tbvcbdaycost(nvcbid,ncost,ddate) values(%u,%lld,'%s')", room_id, total_out, cNowDay);
			}
		}

		if (dst_id > 0)
		{
			if (!m_db->build_sql_run(0, 0, "update tbuserdayincome set nincome=nincome+%lld where nuserid=%u and ddate='%s' ", total_in, dst_id, cNowDay))
			{
				ret_return = -1;
				break;
			}

			if (0 == m_db->getAffectedRow())
			{
				m_db->build_sql_run(0, 0, "insert into tbuserdayincome(nuserid,nincome,ddate) values(%u,%lld,'%s')", dst_id, total_in, cNowDay);
			}

			if (!m_db->build_sql_run(0, 0, "update tbuserdaygiftlog set nrecvcount=nrecvcount+%u where nuserid=%u and ddate='%s' and ngiftid=%u", gift_count, dst_id, cNowDay, gift_id))
			{
				ret_return = -1;
				break;
			}

			if (0 == m_db->getAffectedRow())
			{
				m_db->build_sql_run(0, 0, "insert into tbuserdaygiftlog(nuserid,ngiftid,nrecvcount,ddate) values(%u,%u,%u,'%s')", dst_id, gift_id, gift_count, cNowDay);
			}
		}

		if (!m_db->build_sql_run(0, 0, "update tbuserdaygiftlog set nsendcount=nsendcount+%u where nuserid=%u and ddate='%s' and ngiftid=%u ", gift_count, src_id, cNowDay, gift_id))
		{
			ret_return = -1;
			break;
		}
		
		if (0 == m_db->getAffectedRow())
		{
			if (!m_db->build_sql_run(0, 0, "insert into tbuserdaygiftlog(nuserid,ngiftid,nsendcount,ddate) values(%u,%u,%u,'%s')", src_id, gift_id, gift_count, cNowDay))
			{
				ret_return = -1;
				break;
			}
		}

		if (!m_db->build_sql_run(&res, &total_row, "select nk,nb from dks_useraccount where nuserid=%u", src_id))
		{
			ret_return = -1;
			break;
		}

		unsigned long long src_nk = 0;
		unsigned long long src_nb = 0;
		m_db->GetFromRes(src_nk, &res, 0, 0);
		m_db->GetFromRes(src_nb, &res, 0, 1);
		m_db->CleanRes(&res);

		if (!m_db->build_sql_run(&res, &total_row, "select nk,nb from dks_useraccount where nuserid=%u", dst_id))
		{
			ret_return = -1;
			break;
		}

		unsigned long long dst_nk = 0;
		unsigned long long dst_nb = 0;
		m_db->GetFromRes(dst_nk, &res, 0, 0);
		m_db->GetFromRes(dst_nb, &res, 0, 1);
		m_db->CleanRes(&res);

		needRollBack = false;
		ret_return = 0;

		*src_now_nk = src_nk;
		*src_now_nb = src_nb;
		*dst_now_nk = dst_nk;
		*dst_now_nb = dst_nb;
		*src_add_nchestnum = 0;
		*src_now_nchestnum = 0;

	} while (0);

	if (needRollBack)
	{
		LOG_PRINT(log_info, "func_trade_normal_gift_DB rollback.");
		m_db->transRollBack();
	}
	else
	{
		LOG_PRINT(log_info, "func_trade_normal_gift_DB commit.");
		m_db->transCommit();
	}

	m_db->CleanRes(&res);
	return ret_return;
}

int CDBTools::func_updateusermonthcost_DB(unsigned int userid, int64 cost_nk)
{
	char query[128] = {0};
	sprintf(query,"select updateusermonthcost(%u,%lld)", userid, cost_nk);
	if (!m_db->run_sql(query))
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int CDBTools::get_add_zhuanbo_robots_DB(std::list<DDzhaunborobotinfo_t> &lstOut)
{
	Result res;
	int row = 0;
	const char *sql = "select nid,nvcbid,npubmicidx,nrobotid,nrobotviplevel,nrobotvcbid,nstatus,nuserid from dks_vcbzhuanboconfig where nstatus=1";
	if (!m_db->build_sql_run(&res, &row, sql) || 0 == row)
	{
		m_db->CleanRes(&res);
		return 0;
	}

	for(int r = 0; r < row; r++)
	{
		DDzhaunborobotinfo_t record;
		memset(&record, 0, sizeof(DDzhaunborobotinfo_t));
		m_db->GetFromRes(record.nid, &res, r, 0);
		m_db->GetFromRes(record.nvcbid, &res, r, 1);
		m_db->GetFromRes(record.npubmicidx, &res, r, 2);
		m_db->GetFromRes(record.nrobotid, &res, r, 3);
		m_db->GetFromRes(record.nrobotviplevel, &res, r, 4);
		m_db->GetFromRes(record.nrobotvcbid, &res, r, 5);
		m_db->GetFromRes(record.nstatus, &res, r, 6);
		m_db->GetFromRes(record.nuserid, &res, r, 7);
		lstOut.push_back(record);
	}
	m_db->CleanRes(&res);

	return row;
}

int CDBTools::get_del_zhuanbo_robots_DB(std::list<DDzhaunborobotinfo_t> &lstOut)
{
	Result res;
	int row = 0;
	const char *sql = "select nid,nvcbid,npubmicidx,nrobotid,nrobotviplevel,nrobotvcbid,nstatus from dks_vcbzhuanboconfig where nstatus=-1";
	if (!m_db->build_sql_run(&res, &row, sql) || 0 == row)
	{
		m_db->CleanRes(&res);
		return 0;
	}

	for(int r = 0; r < row; r++)
	{
		DDzhaunborobotinfo_t record;
		memset(&record, 0, sizeof(DDzhaunborobotinfo_t));
		m_db->GetFromRes(record.nid, &res, r, 0);
		m_db->GetFromRes(record.nvcbid, &res, r, 1);
		m_db->GetFromRes(record.npubmicidx, &res, r, 2);
		m_db->GetFromRes(record.nrobotid, &res, r, 3);
		m_db->GetFromRes(record.nrobotviplevel, &res, r, 4);
		m_db->GetFromRes(record.nrobotvcbid, &res, r, 5);
		m_db->GetFromRes(record.nstatus, &res, r, 6);
		lstOut.push_back(record);
	}
	m_db->CleanRes(&res);
	return row;
}

int CDBTools::get_vcballroominfolist_DB(std::list<DDRoomFullInfo_t> &lstOut)
{
	Result res;
	int row;
	const char *query = "select a.nvcbid,a.nxingji,a.ngroupid,a.nattrid, \
						a.nseats,a.nvisible,a.ncreateid,a.nop1id,a.nop2id,a.nop3id,a.nop4id,a.cname,a.cpassword, \
						a.croompic,a.popularity,b.nmcuid,b.cmedia,0,UNIX_TIMESTAMP(b.dactivetime),b.optstat \
						from dks_vcbinformation as a,dks_vcbstatus as b\
						where a.nvcbid=b.nvcbid  and a.livetype = 1";

	if (!m_db->build_sql_run(&res, &row, query) || 0 == row)
	{
		m_db->CleanRes(&res);
		return 0;
	}

	for(int r = 0; r < row; ++r)  //row
	{
		DDRoomFullInfo_t record;
		memset(&record, 0, sizeof(DDRoomFullInfo_t));

		m_db->GetFromRes(record.roomid,				&res, r, 0);
		m_db->GetFromRes(record.xingjiid,			&res, r, 1);
		m_db->GetFromRes(record.groupid,			&res, r, 2);
		m_db->GetFromRes(record.attrid,				&res, r, 3);
		m_db->GetFromRes(record.seats,				&res, r, 4);
		m_db->GetFromRes(record.visible,			&res, r, 5);
		m_db->GetFromRes(record.creatorid,			&res, r, 6);
		m_db->GetFromRes(record.op1id,				&res, r, 7);
		m_db->GetFromRes(record.op2id,				&res, r, 8);
		m_db->GetFromRes(record.op3id,				&res, r, 9);
		m_db->GetFromRes(record.op4id,				&res, r, 10);
		m_db->GetFromRes(record.cname,				&res, r, 11, sizeof(record.cname));
		m_db->GetFromRes(record.cpassword,			&res, r, 12, sizeof(record.cpassword));
		m_db->GetFromRes(record.croompic,			&res, r, 13, sizeof(record.croompic));
		m_db->GetFromRes(record.npopularity_,		&res, r, 14);
		m_db->GetFromRes(record.mcuid,				&res, r, 15);
		m_db->GetFromRes(record.cmedia,				&res, r, 16, sizeof(record.cmedia));
		m_db->GetFromRes(record.ntextteacherid_,	&res, r, 17);
		m_db->GetFromRes(record.activetime,			&res, r, 18);
		m_db->GetFromRes(record.optstat,			&res, r, 19);

		lstOut.push_back(record);
	}

	m_db->CleanRes(&res);

	return row;
}

int CDBTools::get_vcbfullinfolist_DB(std::list<DDRoomFullInfo_t> &lstOut)
{
	Result res;
	int row = 0;
	const char *query = "select a.nvcbid,a.nxingji,a.ngroupid,a.nattrid,a.nseats,a.nvisible,a.ncreateid,a.nop1id,a.nop2id,a.nop3id,a.nop4id,a.cname,a.cpassword, "
		" a.croompic,a.popularity,b.nmcuid,b.cmedia,0,IF(c.viptype is null,0,c.viptype) as viplevel,a.type,UNIX_TIMESTAMP(b.dactivetime),b.optstat "
		" from dks_vcbinformation as a left join dks_vcbstatus as b ON a.nvcbid=b.nvcbid left JOIN dks_vip_price_info c on a.jurisdiction=c.id "
		" where  b.nstatus=1 and a.livetype = 1";

	if (!m_db->build_sql_run(&res, &row, query) || 0 == row)
	{
		m_db->CleanRes(&res);
		return 0;
	}

	for(int r = 0; r < row; ++r)
	{
		DDRoomFullInfo_t record;
		memset(&record, 0, sizeof(DDRoomFullInfo_t));

		m_db->GetFromRes(record.roomid,				&res, r, 0);
		m_db->GetFromRes(record.xingjiid,			&res, r, 1);
		m_db->GetFromRes(record.groupid,			&res, r, 2);
		m_db->GetFromRes(record.attrid,				&res, r, 3);
		m_db->GetFromRes(record.seats,				&res, r, 4);
		m_db->GetFromRes(record.visible,			&res, r, 5);
		m_db->GetFromRes(record.creatorid,			&res, r, 6);
		m_db->GetFromRes(record.op1id,				&res, r, 7);
		m_db->GetFromRes(record.op2id,				&res, r, 8);
		m_db->GetFromRes(record.op3id,				&res, r, 9);
		m_db->GetFromRes(record.op4id,				&res, r, 10);
		m_db->GetFromRes(record.cname,				&res, r, 11, sizeof(record.cname));
		m_db->GetFromRes(record.cpassword,			&res, r, 12, sizeof(record.cpassword));
		m_db->GetFromRes(record.croompic,			&res, r, 13, sizeof(record.croompic));
		m_db->GetFromRes(record.npopularity_,		&res, r, 14);
		m_db->GetFromRes(record.mcuid,				&res, r, 15);
		m_db->GetFromRes(record.cmedia,				&res, r, 16, sizeof(record.cmedia));
		m_db->GetFromRes(record.ntextteacherid_,	&res, r, 17);
		m_db->GetFromRes(record.viplevel,			&res, r, 18);
		m_db->GetFromRes(record.roomtype,			&res, r, 19);
		m_db->GetFromRes(record.activetime,			&res, r, 20);
		m_db->GetFromRes(record.optstat,			&res, r, 21);

		lstOut.push_back(record);
	}

	m_db->CleanRes(&res);

	return row;
}

int CDBTools::get_delvcbIdlist_DB(int* vcbIds, int maxnum)
{
	Result res;
	int row = 0;
	char query[256];
	sprintf(query,"select nvcbid from dks_vcbstatus where nstatus=-1 and nvcbid in (select nvcbid from dks_vcbinformation where livetype=1) limit %d offset 0", maxnum);

	if (!m_db->build_sql_run(&res, &row, query) || 0 == row)
	{
		m_db->CleanRes(&res);
		return 0;
	}

	for(int r = 0; r < row; ++r)
	{
		m_db->GetFromRes(vcbIds[r], &res, r, 0);
	}

	return row;
}

int CDBTools::get_vcbbaseinfolist_DB(std::list<DDRoomBaseInfo_t> &lstOut)
{
	Result res;
	int row;
	const char *query = " select a.nvcbid,a.nxingji,a.ngroupid,a.nattrid,"
		" a.nseats,a.nvisible,a.ncreateid,a.nop1id,a.nop2id,a.nop3id,a.nop4id,a.cname,a.cpassword,a.croompic,a.type from dks_vcbinformation as a,dks_vcbstatus as b where "
		" a.nvcbid=b.nvcbid and b.nstatus=2 and a.nmodid1=1 and a.livetype=1";
	if (!m_db->build_sql_run(&res, &row, query) || 0 == row)
	{
		m_db->CleanRes(&res);
		return 0;
	}

	for(int r = 0; r < row; ++r)
	{
		DDRoomBaseInfo_t record;
		memset(&record, 0, sizeof(DDRoomBaseInfo_t));
		//
		m_db->GetFromRes(record.roomid,				&res, r, 0);
		m_db->GetFromRes(record.xingjiid,			&res, r, 1);
		m_db->GetFromRes(record.groupid,			&res, r, 2);
		m_db->GetFromRes(record.attrid,				&res, r, 3);
		m_db->GetFromRes(record.seats,				&res, r, 4);
		m_db->GetFromRes(record.visible,			&res, r, 5);
		m_db->GetFromRes(record.creatorid,			&res, r, 6);
		m_db->GetFromRes(record.op1id,				&res, r, 7);
		m_db->GetFromRes(record.op2id,				&res, r, 8);
		m_db->GetFromRes(record.op3id,				&res, r, 9);
		m_db->GetFromRes(record.op4id,				&res, r, 10);
		m_db->GetFromRes(record.cname,				&res, r, 11, sizeof(record.cname));
		m_db->GetFromRes(record.cpassword,			&res, r, 12, sizeof(record.cpassword));
		m_db->GetFromRes(record.croompic,			&res, r, 13, sizeof(record.croompic));
		m_db->GetFromRes(record.roomtype,			&res, r, 14);

		lstOut.push_back(record);
	}
	m_db->CleanRes(&res);

	query = "update dks_vcbinformation set nmodid1=0 where livetype=1";
	m_db->build_sql_run(NULL, NULL, query);

	return row;
}

int CDBTools::get_visitor_records(uint32 userid)
{
	Result res;
	int row;
	char query[256];
	sprintf(query,"select nuserid from dks_useraccount where nuserid=%u",userid);
	if (!m_db->build_sql_run(&res, &row, query) || 0 == row)
	{
		m_db->CleanRes(&res);
		return 0;
	}
	m_db->CleanRes(&res);
	return row;
}

int CDBTools::check_user_token_DB(unsigned int loginid, const std::string & strToken, bool checkTime)
{
	if(!loginid || strToken.empty()) 
	{
		LOG_PRINT(log_error, "check_user_token_DB input error.");
		return -1;
	}

	Result res;
	char query[256] = {0};
	int row;

	sprintf(query, "select code_time from mc_member where id=%u and code='%s'", loginid, strToken.c_str());
	if (!m_db->build_sql_run(&res, &row, query) || 0 == row)
	{
		m_db->CleanRes(&res);
		return -1;
	}

	unsigned int code_time = 0;
	m_db->GetFromRes(code_time, &res, 0, 0);
	m_db->CleanRes(&res);

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

int CDBTools::resetTokenValidTime(unsigned int loginid, const std::string & strToken)
{
	if(!loginid || strToken.empty()) 
	{
		LOG_PRINT(log_error, "resetTokenValidTime input error.");
		return -1;
	}

	unsigned int time_now = time(NULL);
	char updateSQL[256] = {0};
	sprintf(updateSQL, "update mc_member set code_time=%u where id=%u and code='%s'", time_now, loginid, strToken.c_str());
	return m_db->run_sql(updateSQL);
}

bool CDBTools::checkUserHasAuth(unsigned int userid, unsigned int groupid, unsigned int authType)
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
	if (!m_db->build_sql_run(&res, &row, strSQL.str().c_str()) || 0 == row)
	{
		m_db->CleanRes(&res);
		return false;
	}

	unsigned int count = 0;
	m_db->GetFromRes(count, &res, 0, 0);
	m_db->CleanRes(&res);
	return count == 0 ? false : true;
}

int CDBTools::userApplyJoinGroupAuth(unsigned int userid, unsigned int groupid, unsigned int authType, const std::string & user_msg)
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
	if (!m_db->build_sql_run(&res, &row, strSQL.str().c_str()))
	{
		m_db->CleanRes(&res);
		return 0;
	}

	if (row)
	{
		m_db->GetFromRes(id, &res, 0, 0);
	}
	m_db->CleanRes(&res);
	
	int insertID = 0;
	transBegin();
	bool rollBack = true;
	do 
	{
		if (id)
		{
			std::stringstream strUpdateSQL;
			strUpdateSQL << " update mc_authentication set is_del=1 where id=" << id;
			if (!m_db->run_sql(strUpdateSQL.str().c_str()))
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
		if (!m_db->run_sql(strInsertSQL.str().c_str()))
		{
			break;
		}
		rollBack = false;
		insertID = m_db->getInsertId();
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

bool CDBTools::updateJoinGroupAuthMsgID(unsigned int authID, unsigned int mongoMsgID)
{
	if (!authID || !mongoMsgID)
	{
		return false;
	}

	std::stringstream strUpdateSQL;
	strUpdateSQL << " update mc_authentication set msg_id=" << mongoMsgID << " where id=" << authID;
	return m_db->run_sql(strUpdateSQL.str().c_str());
}

bool CDBTools::qryJoinGroupAuth(unsigned int id, unsigned int & groupAssistID, unsigned int & apply_userID, unsigned int & groupID)
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
	if (!m_db->build_sql_run(&res, &row, strSQL.str().c_str()) || row == 0)
	{
		m_db->CleanRes(&res);
		return false;
	}

	m_db->GetFromRes(groupAssistID, &res, 0, 0);
	m_db->GetFromRes(apply_userID, &res, 0, 1);
	m_db->GetFromRes(groupID, &res, 0, 2);
	m_db->CleanRes(&res);
	return true;
}

bool CDBTools::handleJoinGroupAuth(unsigned int id, bool agree)
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
	return m_db->run_sql(strUpdateSQL.str().c_str());
}

int CDBTools::qryGroupOwner(unsigned int groupid)
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
		if (!m_db->build_sql_run(&res, &row, strSQL.str().c_str()))
		{
			ret = -1;
			break;
		}

		if (row == 0)
		{
			break;
		}

		m_db->GetFromRes(ret, &res, 0, 0);
	} while (0);

	m_db->CleanRes(&res);
	return ret;
}

int CDBTools::updateGroupOwner(unsigned int groupid, unsigned int new_owner)
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
	if (!m_db->run_sql(strUpdateSQL.str().c_str()))
		return -1;

	if (m_db->getAffectedRow() > 0)
	{
		return ret;
	}
	else
	{
		return -1;
	}	
}

bool CDBTools::updateGroupMicMode(unsigned int groupid, int mode)
{
	if (!groupid || mode > 2 || mode < 0)
	{
		return false;
	}

	std::stringstream strUpdateSQL;
	strUpdateSQL << " update mc_group set mic_mode=" << mode << " where id=" << groupid;
	return m_db->run_sql(strUpdateSQL.str().c_str());
}

int CDBTools::write_addlastgoroom_DB(uint32 userid, uint32 vcbid, uint32 type)
{
	char szTime[32] = {0};
	char query[256] = {0};
	time_t tnow = time(NULL);

	toStringDate(tnow, szTime);
	sprintf(query, "delete from mc_user_group_log where user_id=%u and group_id=%u and dev_type=%u and date_time='%s'", \
		userid, vcbid, type, szTime);
	m_db->run_sql(query);

	sprintf(query, "insert into mc_user_group_log(user_id,group_id,dev_type,date_time) values(%u,%u,%u,'%s')",
		userid, vcbid, type, szTime);
	m_db->run_sql(query);
	return 0;
}

string CDBTools::build_blacklist_sql(int userid, int vcbid, int scopetype, char* szip, char* szmac, char* sn)
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

int CDBTools::isin_blackIpMac_DB(int userid, int vcbid, int scopetype, char* szip, char* szmac, char* sn, DDViolationInfo_t* pInfo/* = NULL*/)
{
	string sql = build_blacklist_sql(userid, vcbid, scopetype, szip, szmac, sn);
	if (sql.empty())
		return -1;

	Result res;
	int row = 0;
	if (!m_db->build_sql_run(&res, &row, sql.c_str())) {
		m_db->CleanRes(&res);
		return -1;
	}
	if (0 == row) {
		m_db->CleanRes(&res);
		return -2;
	}

	if (pInfo)
	{
		time_t tendtime;
		m_db->GetFromRes(pInfo->reasontype, &res, 0, 0);
		m_db->GetFromRes(pInfo->scopetype, &res, 0, 2);
		m_db->GetFromRes(tendtime, &res, 0, 3);

		pInfo->lefttime = tendtime - time(0);
	}

	m_db->CleanRes(&res);

	return 0;
}

int CDBTools::get_room_collect_num(uint32_t vcbid, uint32_t &pNum)
{
	Result res;
	int row;
	char query[256];

	sprintf(query,"select total_attention from dks_vcbinformation where nvcbid=%u ", vcbid);
	if (!m_db->build_sql_run(&res, &row, query)) {
		m_db->CleanRes(&res);
		return -1;
	}
	if (row > 0) {
		m_db->GetFromRes(pNum, &res, 0, 0);
		m_db->CleanRes(&res);
		return 0;
	}
	return -1;

}

int CDBTools::get_vcbmedia_addr_byareaid(uint32 vcbid, const char* areadid, char * mediaip, size_t len)
{
	Result res;
	int row;
	char query[256];

	sprintf(query, "select cmedia from dks_vcbareastatus where nvcbid=%u and careaid=\'%s\'", vcbid, areadid);
	if (!m_db->build_sql_run(&res, &row, query)) {
		m_db->CleanRes(&res);
		return -1;
	}

	if (row > 0) {
		m_db->GetFromRes(mediaip, &res, 0, 0, len);
		m_db->CleanRes(&res);
		return 0;
	}
	return -1;
}

int CDBTools::update_vcbmedia_DB(uint32 vcbid, const char *paddr)
{
	char query[256];
	if (!paddr || *paddr == 0)
		return -1;
	sprintf(query, "update dks_vcbstatus set cmedia='%s' where nvcbid=%u", paddr, vcbid);
	if (!m_db->build_sql_run(0, 0, query)) {
		return -1;
	}
	return 0;
}

int CDBTools::update_room_activetime(uint32_t roomid, uint64_t tActive)
{
	char query[256];
	sprintf(query, "update dks_vcbstatus set dactivetime=from_unixtime(%llu) where nvcbid=%u", tActive, roomid);
	if (!m_db->build_sql_run(0, 0, query)) {
		return -1;
	}
	return 0;
}

int CDBTools::update_room_popularity(uint32_t vcbid, uint64_t accesstimes)
{
	char query[256];

	sprintf(query, "update dks_vcbinformation set popularity=%llu where nvcbid=%u", accesstimes, vcbid);
	if (!m_db->build_sql_run(0, 0, query)) {
		return -1;
	}
	return 0;
}

int CDBTools::get_syscasts_DB(list<DDSysCastNews_t> &listout, int casttype, int recid)
{
	Result res;
	int row = 0;
	char query[256]= {0};
	char szTime[32]= {0};
	time_t tnow=time(0);
	toStringTimestamp(tnow, szTime);
	if(casttype == 0)
	{
		sprintf(query, "select nid,ntype,nisoneoff,cnewstext from dks_syscastnews where nid>%d and nisoneoff=0 order by nid asc limit 1", recid);
		if (!m_db->build_sql_run(&res, &row, query)) {
			return -1;
		}
		if (0 == row) {
			sprintf(query, "select nid,ntype,nisoneoff,cnewstext from dks_syscastnews where nid>%d and nisoneoff=0 order by nid asc limit 1", recid);
			if (!m_db->build_sql_run(&res, &row, query)) {
				return -1;
			}
		}
		for (int r=0; r<row; r++) {
			DDSysCastNews_t record = {0};
			m_db->GetFromRes(record.nid, &res, r, 0);
			m_db->GetFromRes(record.ntype, &res, r, 1);
			m_db->GetFromRes(record.nisoneoff, &res, r, 2);
			m_db->GetFromRes(record.cnewstext, &res, r, 3, sizeof(record.cnewstext));
			listout.push_back(record);
		}
		m_db->CleanRes(&res);
	}
	else
	{
		sprintf(query, "select nid,ntype,nisoneoff,cnewstext,dusetime from dks_syscastnews where nisoneoff=1 and dusetime<'%s'", szTime);
		if (!m_db->build_sql_run(&res, &row, query)) {
			return -1;
		}
		for (int r=0; r<row; r++) {
			DDSysCastNews_t record = {0};
			m_db->GetFromRes(record.nid, &res, r, 0);
			m_db->GetFromRes(record.ntype, &res, r, 1);
			m_db->GetFromRes(record.nisoneoff, &res, r, 2);
			m_db->GetFromRes(record.cnewstext, &res, r, 3, sizeof(record.cnewstext));
			listout.push_back(record);
		}
		m_db->CleanRes(&res);
		sprintf(query, "delete from dks_syscastnews where nisoneoff=1 and dusetime<'%s'", szTime);
		m_db->build_sql_run(&res, &row, query);
	}

	return 0;
}

int CDBTools::get_teacherid_from_robotid(uint32 subroomid, uint32 robotid, uint32 &teacherid, uint32 &mroomid)
{
	Result res;
	int row = 0;
	char query[256];

	sprintf(query, "select nuserid, nvcbid from dks_vcbzhuanboconfig where nrobotvcbid=%d and nrobotid=%d and nuserid !=0", subroomid, robotid);
	if (!m_db->build_sql_run(&res, &row, query) || row == 0) {
		m_db->CleanRes(&res);
		return -1;
	}
	m_db->GetFromRes(teacherid, &res, 0, 0);
	m_db->GetFromRes(mroomid, &res, 0, 1);
	m_db->CleanRes(&res);
	return 0;
}

bool CDBTools::delete_teacherfans_DB(unsigned int teacherID, unsigned int fansID)
{
	char cSQL[256] = {0};
	sprintf(cSQL, "delete from tb_teacherfans where teacherid=%u and userid=%u", teacherID, fansID);
	return m_db->run_sql(cSQL);
}

int CDBTools::check_teacherfans_exist_DB(unsigned int teacherID, unsigned int fansID)
{
	char cSQL[256] = {0};
	sprintf(cSQL, "select 1 from tb_teacherfans where teacherid=%u and userid=%u", teacherID, fansID);
	int row = 0;
	if (!m_db->run_sql(cSQL, NULL, &row))
		return -1;

	return row;
}

bool CDBTools::insert_teacherfans_DB(unsigned int teacherID, unsigned int fansID)
{
	char cSQL[256] = {0};
	sprintf(cSQL, "insert into tb_teacherfans(teacherid, userid) values(%u,%u)", teacherID, fansID);
	return m_db->run_sql(cSQL);
}

int CDBTools::get_user_fullinfoByLoginID_DB(StUserFullInfo_t & ptf, uint32 loginID, const std::string & patternlock)
{
	if(!loginID)
	{
		LOG_PRINT(log_error, "get_user_fullinfoByLoginID_DB failed,loginid is wrong.");
		return -1;
	}

	char query[1024] = {0};
	Result res;
	int row;

	sprintf(query, "select user_id,gender,alias,head_add,birthday,freeze,tel from talk_user where user_Id=%u", loginID);

	if (!m_db->build_sql_run(&res, &row, query) || 0 == row) 
	{
		m_db->CleanRes(&res);
		return -1;
	}

	m_db->GetFromRes(ptf.userid, &res, 0, 0);
	m_db->GetFromRes(ptf.gender, &res, 0, 1);
	m_db->GetFromRes(ptf.strAlias, &res, 0, 2);
	m_db->GetFromRes(ptf.strHead, &res, 0, 3);
	m_db->GetFromRes(ptf.strBirthday, &res, 0, 4);
	m_db->GetFromRes(ptf.nfreeze, &res, 0, 5);
	m_db->GetFromRes(ptf.strTel, &res, 0, 6);
	m_db->CleanRes(&res);
	
	return 0;
}

bool CDBTools::update_userinfo_DB(unsigned int userid, const std::map<std::string, std::string> & field_value_map)
{
	if(userid == 0 || field_value_map.empty())
	{
		LOG_PRINT(log_error, "update_userinfo_DB by userid error,userid is 0 or field_value_map size:%u.", field_value_map.size());
		return -1;
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
	
	return m_db->run_sql(updateSQL.str().c_str());
}

int CDBTools::get_user_fullinfo_DB(StUserFullInfo_t & ptf, uint32 userid)
{
	if(userid == 0)
	{
		LOG_PRINT(log_error, "get user full info by userid error,userid is 0.");
		return -1;
	}

	char query[1024] = {0};
	Result res;
	int row = 0;
	sprintf(query, "select id,gender,alias,headaddr,birthday,is_freeze,member_id from mc_user where id=%u", userid);
	if (!m_db->build_sql_run(&res, &row, query) || 0 == row) 
	{
		m_db->CleanRes(&res);
		return -1;
	}

	m_db->GetFromRes(ptf.userid, &res, 0, 0);
	m_db->GetFromRes(ptf.gender, &res, 0, 1);
	m_db->GetFromRes(ptf.strAlias, &res, 0, 2);
	m_db->GetFromRes(ptf.strHead, &res, 0, 3);
	m_db->GetFromRes(ptf.strBirthday, &res, 0, 4);
	m_db->GetFromRes(ptf.nfreeze, &res, 0, 5);
	unsigned int loginID = 0;
	m_db->GetFromRes(loginID, &res, 0, 6);
	m_db->CleanRes(&res);

	do 
	{
		row = 0;
		memset(query, 0, sizeof(query));
		sprintf(query, "select tel from mc_login where member_id=%u", loginID);
		if (!m_db->build_sql_run(&res, &row, query) || 0 == row) 
		{
			m_db->CleanRes(&res);
			break;
		}

		m_db->GetFromRes(ptf.strTel, &res, 0, 0);
		m_db->CleanRes(&res);
	} while (0);

	return 0;
}

int CDBTools::get_user_vip_info(unsigned int nuserid, unsigned int platform_id, CUserVip_t & userVipInfo)
{
	char query[256] = {0};
	time_t now_time = time(NULL);
	sprintf(query, "select b.group_id,a.dead_time from dks_user_group_vip a,dks_group_vip b where a.group_vip_id=b.id and a.user_id=%u and a.platform_id=%u and a.dead_time>=%u", nuserid, platform_id, now_time);

	Result res;
	int row = 0;
	if (!m_db->run_sql(query, &res, &row))
	{
		m_db->CleanRes(&res);
		return -1;
	}

	if(row == 0)
	{
		m_db->CleanRes(&res);
		return 0;
	}
	else
	{
		int group_id = 0;
		unsigned int deadline_time = 0;
		m_db->GetFromRes(group_id, &res, 0, 0);
		m_db->GetFromRes(deadline_time, &res, 0, 1);
		m_db->CleanRes(&res);

		userVipInfo.userID = nuserid;
		userVipInfo.platformID = platform_id;
		userVipInfo.deadLineTime = deadline_time;
		userVipInfo.groupID = group_id;
		return row;
	}
}

int CDBTools::get_user_exit_message(int userid, CMDSecureInfo_t & pTab, int & hit_times)
{
	time_t lt = time(NULL);
	struct tm * ptr = localtime(&lt);

	time_t lastTime = time(NULL) - (ptr->tm_hour*3600) - (ptr->tm_min*60) - ptr->tm_sec;
	char szTime[32] = {0};
	toStringDate(lastTime, szTime);
	char query[512] = {0};
	sprintf(query, "select email,qq,tel from dks_user where user_id=%d", userid);

	Result res;
	int row = 0;
	if (!m_db->run_sql(query, &res, &row) || row != 1)
	{
		m_db->CleanRes(&res);
		return -1;
	}

	m_db->GetFromRes(pTab.email, &res, 0, 0, sizeof(pTab.email));
	m_db->GetFromRes(pTab.qq, &res, 0, 1, sizeof(pTab.qq));
	m_db->GetFromRes(pTab.phonenumber, &res, 0, 2, sizeof(pTab.phonenumber));

	m_db->CleanRes(&res);

	return 0;
}

int CDBTools::get_vistorlogon_DB(char *_uuid, std::vector<VistorLogon_t> &vVistor)
{
	if (NULL == _uuid) return 0;
	char query[256];
	sprintf(query, "select userid,state,logontime from dks_visitorlogon where uuid='%s' and state=0", _uuid);

	Result res;
	int row = 0;
	if (!m_db->run_sql(query, &res, &row))
	{
		m_db->CleanRes(&res);
		return -1;
	}

	if (0 == row)
	{
		LOG_PRINT(log_warning, "uuid record is empty.uuid:%s.", _uuid);
		return 0;
	}

	vVistor.resize(row);
	for(int r = 0; r < row; ++r)
	{
		char _logontime[32] = {0};
		m_db->GetFromRes(vVistor[r].userid, &res, r, 0);
		m_db->GetFromRes(vVistor[r].state, &res, r, 1);
		m_db->GetFromRes(_logontime, &res, r, 2, sizeof(_logontime));
		vVistor[r].logontime = toNumericTimestamp(_logontime);
	}

	m_db->CleanRes(&res);

	return 0;
}

bool CDBTools::update_vistorlogon_DB(const VistorLogon_t & vsl)
{
	char _logontime[32] = {0};
	toStringTimestamp(vsl.logontime, _logontime);

	char updateSQL[512] = {0};
	sprintf(updateSQL, "update dks_visitorlogon set state=%d,logontime='%s' where userid=%u",\
		vsl.state, _logontime, vsl.userid);
	return m_db->run_sql(updateSQL);
}

int CDBTools::func_get_visitor_unique_id(const unsigned int area)
{
	char query[128] = {0};
	sprintf(query,"select nextval('visitorid', %u)", area);

	Result res;
	if (!m_db->run_sql(query, &res))
	{
		m_db->CleanRes(&res);
		return -1;
	}

	unsigned int ret_sucess = -1;
	m_db->GetFromRes(ret_sucess, &res, 0, 0);

	m_db->CleanRes(&res);
	return ret_sucess;
}

bool CDBTools::insert_vistorlogon_DB(const VistorLogon_t &vsl)
{
	char _logontime[32]={0};
	toStringTimestamp(vsl.logontime, _logontime);    

	char insertSQL[512] = {0};
	sprintf(insertSQL, "insert into dks_visitorlogon(userid,uuid,state,logontime) values(%u,'%s',%d,'%s')",\
		vsl.userid, vsl.cuuid, vsl.state, _logontime);

	return m_db->run_sql(insertSQL);
}

bool CDBTools::get_user_token_DB(int userid, char* token, char* codetime)
{
	if (!m_syncdb)
	{
		LOG_PRINT(log_error, "[get_user_token_DB]syncdb is null.need to call initial first.");
		return false;
	}

	char query[256] = {0};
	sprintf(query, "select token,UNIX_TIMESTAMP(expiry_time) from talk_third_login where user_id=%d", userid);

	Result res;
	int row = 0;
	if (!m_syncdb->run_sql(query, &res, &row) || row != 1)
	{
		m_syncdb->CleanRes(&res);
		return false;
	}

	m_syncdb->GetFromRes(token, &res, 0, 0, -1);
	m_syncdb->GetFromRes(codetime, &res, 0, 1, -1);

	m_syncdb->CleanRes(&res);

	return true;
}

bool CDBTools::update_user_token_DB(unsigned int userid, const char* token, const char* validtime)
{
	if (!m_syncdb)
	{
		LOG_PRINT(log_error, "[update_user_token_DB]syncdb is null.need to call initial first.");
		return false;
	}

	if(!token || !validtime)
	{
		return false;
	}

	char query[256] = {0};
	sprintf(query, "update mc_user set session_code='%s',session_validtime='%s' where id=%d", token, validtime, userid);
	return m_syncdb->run_sql(query);
}

bool CDBTools::get_user_pwdinfo_DB(int userid, DDUserPwdInfo_t & ppf)
{
	Result res;
	char query[256] = {0};
	int row = 0;

	if(userid  == 0) return false;
	sprintf(query, "select user_id,password from dks_user where user_id=%d", userid);

	if (!m_db->run_sql(query, &res, &row) || row != 1)
	{
		m_db->CleanRes(&res);
		return false;
	}

	m_db->GetFromRes(ppf.userid, &res, 0, 0);
	m_db->GetFromRes(ppf.cpassword, &res, 0, 1, sizeof(ppf.cpassword));

	m_db->CleanRes(&res);
	return true;
}

bool CDBTools::update_userpwd_DB(int userid, char * pPwd, int pwdtype)
{
	if (NULL == pPwd) return false;

	char query[256] = {0};
	if(pwdtype == 1) //~{SC;'5GB<C\Bk~}
	{
		sprintf(query, "update dks_user set password='%s' where user_id=%d", pPwd, userid);
	}
	else
	{
		return false;
	}

	return m_db->run_sql(query);
}

int CDBTools::get_user_secure_info_DB(int userid, CMDSecureInfo_t &info)
{
	char query[512];
	sprintf(query, "select cemail,cqq,ctel,nreminded_time from dks_user where nuserid=%d", userid);

	Result res;
	int row = 0;
	if (!m_db->run_sql(query, &res, &row) || row != 1)
	{
		m_db->CleanRes(&res);
		return -1;
	}

	m_db->GetFromRes(info.email, &res, 0, 0, sizeof(info.email));
	m_db->GetFromRes(info.qq, &res, 0, 1, sizeof(info.qq));
	m_db->GetFromRes(info.phonenumber, &res, 0, 2, sizeof(info.phonenumber));
	m_db->GetFromRes(info.remindtimes, &res, 0, 3);
	m_db->CleanRes(&res);

	if(info.remindtimes < 3 && (strlen(info.email) == 0 || strlen(info.qq) == 0 || strlen(info.phonenumber) == 0))
	{
		sprintf(query, "update dks_user set nreminded_time=%d where nuserid=%d", info.remindtimes+1, userid);
		m_db->run_sql(query);
	}

	return row;
}

int CDBTools::get_privilegelist_DB(vector<DDQuanxianAction_t> &vAction)
{
	const char *query = "select a.qx_id,a.qx_type,b.srclevel,b.tolevel from tbquanxianinfo as a,tbquanxianaction as b where a.qx_id=b.qx_id ";

	Result res;
	int row = 0;
	if (!m_db->run_sql(query, &res, &row) || 0 == row)
	{
		m_db->CleanRes(&res);
		return 0;
	}

	vAction.resize(row);
	for(int r = 0; r < row; ++r)
	{
		m_db->GetFromRes(vAction[r].qxid, &res, 0, 0);
		m_db->GetFromRes(vAction[r].qxtype, &res, 0, 1);
		m_db->GetFromRes(vAction[r].srclevel, &res, 0, 2);
		m_db->GetFromRes(vAction[r].tolevel, &res, 0, 3);

	}
	m_db->CleanRes(&res);

	return row;
}

int CDBTools::write_logonrecord(const CMDUserLoginLog_t & oLogonlog)
{
	int ret = 0;
	ulong ntime_begin = 0;
	ulong ntime_end = 0;
	ntime_begin = SL_Socket_CommonAPI::util_process_clock_ms();

	//生成登陆记录
	time_t tnow = time(NULL);
	char szTime[32] = {0};
	toStringTimestamp(tnow, szTime);

	int total_row = 0;
	Result res;
	do
	{
		if (!m_db)
		{
			LOG_PRINT(log_error, "db is null.init failed.");
			ret = -1;
			break;
		}

		if (!m_db->build_sql_run(&res, &total_row,
			"insert into talk_userlogonlog(userid,type,logonip,macaddr,serialaddr,logontime,devicemodel,deviceos) values(%d,%d,'%s','%s','%s','%s','%s','%s')",
			oLogonlog.userid, oLogonlog.devtype, oLogonlog.szip, oLogonlog.szmac, oLogonlog.szserial, szTime, oLogonlog.devicemodel,
			oLogonlog.deviceos))
		{
			ret = -1;
			break;
		}

		if (!m_db->build_sql_run(&res, &total_row, "update talk_user set last_login_time=FROM_UNIXTIME(%u),last_login_ip='%s',last_login_mac='%s' where user_id=%d", (unsigned int)tnow, \
				oLogonlog.szip, oLogonlog.szmac, oLogonlog.userid))
		{
			ret = -1;
			break;
		}
	} while (0);
	m_db->CleanRes(&res);

	ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();
	LOG_PRINT(log_info, "userID[%d],IP[%s] logonRecord takes time:%u,", oLogonlog.userid, oLogonlog.szip, ntime_end - ntime_begin);

	return ret;
}

int CDBTools::qrySysParameterInt(const char * paramname, const int defValue /*= 0*/)
{
	if (NULL == paramname || 0 == *paramname)
	{
		return defValue;
	}
	
	char szQuerySQL[256] = {0};
	sprintf(szQuerySQL, " select paramvalue from mc_sys_parameter where paramname = '%s' and status = 1 ", paramname);

	Result res;
	int row;
	if (!m_db->run_sql(szQuerySQL, &res, &row, false) || 0 == row)
	{
		m_db->CleanRes(&res);
		return defValue;
	}

	int ret;
	m_db->GetFromRes(ret, &res, 0, 0);
	m_db->CleanRes(&res);

	return ret;
}

std::string CDBTools::qrySysParameterString(const char *paramname, const char *defValue /*= ""*/)
{
	if (NULL == paramname || 0 == *paramname)
	{
		return defValue;
	}

	char szQuerySQL[256] = {0};
	sprintf(szQuerySQL, " select paramvalue from mc_sys_parameter where paramname = '%s' and status = 1 ", paramname);

	Result res;
	int row;
	if (!m_db->run_sql(szQuerySQL, &res, &row, false) || 0 == row)
	{
		m_db->CleanRes(&res);
		return defValue;
	}

	char szValue[129] = {0};
	m_db->GetFromRes(szValue, &res, 0, 0, sizeof(szValue)-1);
	m_db->CleanRes(&res);

	return szValue;
}

bool CDBTools::qryDictItemByGroupId(const char *group, vector<DictItem_t> &vOut)
{
	if (NULL == group || *group == 0)
	{
		return false;
	}
	char szQuerySQL[256] = {0};
	sprintf(szQuerySQL, "select groupid, itemid, addvalue from dcl_dict_item a where a.groupid = '%s' and a.status = 1", group);

	Result res;
	int row;
	if (!m_db->run_sql(szQuerySQL, &res, &row) || 0 == row)
	{
		m_db->CleanRes(&res);
		return false;
	}

	vOut.resize(row);
	for (int i = 0; i < row; i++)
	{
		DictItem_t item = {0};
		m_db->GetFromRes(item.group, &res, i, 0, sizeof(item.group) - 1);
		m_db->GetFromRes(item.item, &res, i, 1, sizeof(item.item) - 1);
		m_db->GetFromRes(item.addvalue, &res, i, 2, sizeof(item.addvalue) - 1);
		vOut[i] = item;
	}
	m_db->CleanRes(&res);

	return true;
}

bool CDBTools::write_room_maxclientnum(int vcbid, int maxclientnum)
{
	char query[256];
	sprintf(query, "update dks_vcbstatus set maxclientnum = %d where nvcbid = %d", maxclientnum, vcbid);

	if (!m_db->run_sql(query))
	{
		return false;
	}

	return true;
}

int CDBTools::get_isroommanager_DB(uint32 userid, uint32 vcbid)
{
	int row = 0;
	char query[256];
	sprintf(query, "select nid from dks_vcbmanagermap where nvcbid=%d and nuserid=%d",vcbid,userid);

	do
	{
		if (!m_db->run_sql(query, NULL, &row))	{
			break;
		}

		if (row){
			return 0;
		}

	} while (0);

	return -1;
}

int CDBTools::getActiveGroupfullinfo_DB(std::list<stRoomFullInfo> & oDBRecords)
{
	oDBRecords.clear();
	Result res;
	int row = 0;
	std::stringstream strSQL;
	strSQL << " select a.id,a.showid,a.icon,a.name,a.remark,a.capacity,a.status,a.group_master,a.group_type,a.unread_msgnum,a.mic_mode,";
	strSQL << " b.male_join_group_auth,b.male_red_packet_total,b.male_red_packet_num,b.female_join_group_auth,b.female_red_packet_total,b.female_red_packet_num,b.allow_visit_group,";
	strSQL << " b.contribution_switch,b.voice_chat_switch,b.allow_visitor_on_mic,";
	strSQL << " b.gain_percent,b.gain_userrange,";
	strSQL << " a.user_num,a.male_num,a.female_num,a.audience_num,";
	strSQL << " a.is_del,a.show_square";
	strSQL << " from mc_group as a left join mc_group_plugin_setting_detail as b on a.id=b.group_id";
	strSQL << " where a.status = 1 and a.is_del = 0";

	if (!m_db->build_sql_run(&res, &row, strSQL.str().c_str()))
	{
		m_db->CleanRes(&res);
		return -1;
	}

	if (0 == row)
	{
		LOG_PRINT(log_info, "cannot find active groups in DB.");
		m_db->CleanRes(&res);
		return -1;
	}

	for (int r = 0; r < row; ++r)
	{
		stRoomFullInfo record;
		int c = 0;

		//basic info
//		m_db->GetFromRes(record.groupid,			&res, r, c++);
//		m_db->GetFromRes(record.showid,				&res, r, c++);
//		m_db->GetFromRes(record.icon,				&res, r, c++);
//		m_db->GetFromRes(record.name,				&res, r, c++);
//		m_db->GetFromRes(record.remark,				&res, r, c++);
//		m_db->GetFromRes(record.capacity,			&res, r, c++);
//		m_db->GetFromRes(record.status,				&res, r, c++);
//		m_db->GetFromRes(record.groupMaster,		&res, r, c++);
//		m_db->GetFromRes(record.groupType,			&res, r, c++);
//		m_db->GetFromRes(record.unreadMsgNum,		&res, r, c++);
//		m_db->GetFromRes(record.mic_mode,			&res, r, c++);
//
//		//setting
//		m_db->GetFromRes(record.maleJoinGroupAuth,		&res, r, c++);
//		m_db->GetFromRes(record.maleRedPacketTotal,		&res, r, c++);
//		m_db->GetFromRes(record.maleRedPacketNum,		&res, r, c++);
//		m_db->GetFromRes(record.femaleJoinGroupAuth,	&res, r, c++);
//		m_db->GetFromRes(record.femaleRedPacketTotal,	&res, r, c++);
//		m_db->GetFromRes(record.femaleRedPacketNum,		&res, r, c++);
//		m_db->GetFromRes(record.allowVisitGroup,		&res, r, c++);
//		m_db->GetFromRes(record.contributionSwitch,	&res, r, c++);
//		m_db->GetFromRes(record.voiceChatSwitch,	&res, r, c++);
//		m_db->GetFromRes(record.gainPercent,		&res, r, c++);
//		m_db->GetFromRes(record.gainUserRange,		&res, r, c++);
//
//		//count number
//		m_db->GetFromRes(record.user_num,			&res, r, c++);
//		m_db->GetFromRes(record.male_num,			&res, r, c++);
//		m_db->GetFromRes(record.female_num,			&res, r, c++);
//		m_db->GetFromRes(record.audience_num,		&res, r, c++);
//		m_db->GetFromRes(record.is_del,				&res, r, c++);
//		m_db->GetFromRes(record.show_square,		&res, r, c++);

		oDBRecords.push_back(record);
	}
	m_db->CleanRes(&res);
	return row;
}

bool CDBTools::updateGroupMsgMute(unsigned int userid, unsigned int groupid, unsigned int muteAction)
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
	return m_db->run_sql(strUpdateSQL.str().c_str());
}

//return true by default.same logic as DB
bool CDBTools::getGroupMsgMute(unsigned int userid, unsigned int groupid)
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
		if (!m_db->build_sql_run(&res, &row, strSQL.str().c_str()) || row == 0)
		{
			break;
		}

		int muteSetting = 1;
		m_db->GetFromRes(muteSetting, &res, 0, 0);
		LOG_PRINT(log_info, "userid:%u in the group:%u mute setting is %d.", userid, groupid, muteSetting);
		ret = (muteSetting == 0? false : true);
	} while (0);

	m_db->CleanRes(&res);
	return ret;
}

bool CDBTools::checkBlockUserLst_DB(unsigned int userid, unsigned int groupid)
{
	Result res;
	int row = 0;
	bool bRet = false;
	std::stringstream strSQL;
	do 
	{
		strSQL << "select run_id from mc_group_blacklist where is_del=0 and user_id=" << userid << " and group_id=" << groupid;
		if (!m_db->build_sql_run(&res, &row, strSQL.str().c_str()) || row == 0)
		{
			break;
		}

		unsigned int run_id = 0;
		m_db->GetFromRes(run_id, &res, 0, 0);
		LOG_PRINT(log_warning, "userid:%u is in the group:%u block user list and runn_id is:%u", userid, groupid, run_id);
		bRet = true;
	} while (0);

	m_db->CleanRes(&res);
	return bRet;
}

bool CDBTools::initRoomUserCountDB(uint32_t vcbid, unsigned int & userNum, unsigned int & maleNum, unsigned int & femaleNum)
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
	if (!m_db->build_sql_run(&res, &row, strSQL.str().c_str()))
	{
		m_db->CleanRes(&res);
		return false;
	}

	if (0 == row)
	{
		LOG_PRINT(log_info, "DB cannot find group members in group:%u.", vcbid);
		m_db->CleanRes(&res);
		return false;
	}

	maleNum = 0;
	femaleNum = 0;
	userNum = 0;

	for (int r = 0; r < row; ++r)
	{
		unsigned int gender = 0;
		unsigned int count = 0;
		m_db->GetFromRes(gender,	&res, r, 0);
		m_db->GetFromRes(count,		&res, r, 1);

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
	m_db->CleanRes(&res);

	std::map<std::string, std::string> mValues;
	mValues["user_num"] = bitTostring(userNum);
	mValues["male_num"] = bitTostring(maleNum);
	mValues["female_num"] = bitTostring(femaleNum);
	return updGroupBasicInfo(vcbid, mValues);
}

bool CDBTools::updateRoomVisitorNum(uint32_t vcbid, uint32_t visitorNum)
{
	if (!vcbid)
	{
		LOG_PRINT(log_error, "DB updateRoomVisitorNum input error.vcbid is 0.");
		return false;
	}

	std::stringstream strUpdateSQL;
	strUpdateSQL << " update mc_group set audience_num=" << visitorNum;
	strUpdateSQL << " where id=" << vcbid;

	return m_db->run_sql(strUpdateSQL.str().c_str());
}

bool CDBTools::updateRoomUserCount(uint32_t vcbid, const std::map<std::string, std::string> & oFieldAndIncMap)
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
	return m_db->run_sql(strUpdateSQL.str().c_str());
}

int CDBTools::getOneGroupfullinfo_DB(stRoomFullInfo & record, uint32 groupID)
{
	Result res;
	int row = 0;
	std::stringstream strSQL;
	strSQL << " select a.id,a.showid,a.icon,a.name,a.remark,a.capacity,a.status,a.group_master,a.group_type,a.unread_msgnum,a.mic_mode,";
	strSQL << " b.male_join_group_auth,b.male_red_packet_total,b.male_red_packet_num,b.female_join_group_auth,b.female_red_packet_total,b.female_red_packet_num,b.allow_visit_group,";
	strSQL << " b.contribution_switch,b.voice_chat_switch,b.allow_visitor_on_mic,";
	strSQL << " b.gain_percent,b.gain_userrange,";
	strSQL << " a.user_num,a.male_num,a.female_num,a.audience_num,";
	strSQL << " a.is_del,a.show_square";
	strSQL << " from mc_group as a left join mc_group_plugin_setting_detail as b on a.id=b.group_id";
	strSQL << " where a.status = 1 and a.is_del = 0 and a.id = " << groupID;

	if (!m_db->build_sql_run(&res, &row, strSQL.str().c_str()))
	{
		m_db->CleanRes(&res);
		return -1;
	}

	if (0 == row)
	{
		LOG_PRINT(log_info, "DB cannot find this group id:%u.", groupID);
		m_db->CleanRes(&res);
		return -1;
	}

	int r = 0, c = 0;
	//basic info
//	m_db->GetFromRes(record.groupid,			&res, r, c++);
//	m_db->GetFromRes(record.showid,				&res, r, c++);
//	m_db->GetFromRes(record.icon,				&res, r, c++);
//	m_db->GetFromRes(record.name,				&res, r, c++);
//	m_db->GetFromRes(record.remark,				&res, r, c++);
//	m_db->GetFromRes(record.capacity,			&res, r, c++);
//	m_db->GetFromRes(record.status,				&res, r, c++);
//	m_db->GetFromRes(record.groupMaster,		&res, r, c++);
//	m_db->GetFromRes(record.groupType,			&res, r, c++);
//	m_db->GetFromRes(record.unreadMsgNum,		&res, r, c++);
//	m_db->GetFromRes(record.mic_mode,			&res, r, c++);
//
//	//setting
//	m_db->GetFromRes(record.maleJoinGroupAuth,	&res, r, c++);
//	m_db->GetFromRes(record.maleRedPacketTotal,	&res, r, c++);
//	m_db->GetFromRes(record.maleRedPacketNum,	&res, r, c++);
//	m_db->GetFromRes(record.femaleJoinGroupAuth,	&res, r, c++);
//	m_db->GetFromRes(record.femaleRedPacketTotal,	&res, r, c++);
//	m_db->GetFromRes(record.femaleRedPacketNum,	&res, r, c++);
//	m_db->GetFromRes(record.allowVisitGroup,	&res, r, c++);
//	m_db->GetFromRes(record.contributionSwitch,	&res, r, c++);
//	m_db->GetFromRes(record.voiceChatSwitch,	&res, r, c++);
//	m_db->GetFromRes(record.gainPercent,		&res, r, c++);
//	m_db->GetFromRes(record.gainUserRange,		&res, r, c++);
//
//	//count number
//	m_db->GetFromRes(record.user_num,			&res, r, c++);
//	m_db->GetFromRes(record.male_num,			&res, r, c++);
//	m_db->GetFromRes(record.female_num,			&res, r, c++);
//	m_db->GetFromRes(record.audience_num,		&res, r, c++);
//	m_db->GetFromRes(record.is_del,				&res, r, c++);
//	m_db->GetFromRes(record.show_square,		&res, r, c++);
	m_db->CleanRes(&res);
	return 0;
}

int CDBTools::getGroupBroadInfo_DB(unsigned int group_id, StRoomNotice_t & oBroadInfo)
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
		if (!m_db->build_sql_run(&res, &rows, strQuerySQL.str().c_str())) 
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

		m_db->GetFromRes(oBroadInfo.m_groupID,			&res, 0, 0);
		m_db->GetFromRes(oBroadInfo.m_uBroadUserid,		&res, 0, 1);
		m_db->GetFromRes(oBroadInfo.m_strRoomNotice,	&res, 0, 2);
		m_db->GetFromRes(oBroadInfo.m_uBroadTime,		&res, 0, 3);
	
	} while (0);

	m_db->CleanRes(&res);
	return ret;
}

bool CDBTools::updateGroupBroadInfo_DB(unsigned int group_id, const StRoomNotice_t & oBroadInfo)
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

	return m_db->run_sql(strInsertSQL.str().c_str());
}

bool CDBTools::updateGroupBasicInfo_DB(unsigned int group_id, std::map<std::string, std::string> & oFieldValueMap)
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
	return m_db->run_sql(strUpdateSQL.str().c_str());
}

int CDBTools::getGroupMemebr_DB(unsigned groupid, std::list<stJoinGroupInfo> & oGroupMemLst)
{
	oGroupMemLst.clear();
	if (!groupid)
	{
		LOG_PRINT(log_error, "DB getGroupMemebr_DB input error.groupid:%u.", groupid);
		return -1;
	}

	char cQuerySQL[256] = {0};
	sprintf(cQuerySQL, "select user_id,role_type,level_id from mc_group_member where group_id=%u and is_del=0 and role_type!=0", groupid);

	Result res;
	int row = 0;

	do 
	{
		if (!m_db->build_sql_run(&res, &row, cQuerySQL)) 
		{
			row = -1;
			break;
		}

		for (int i = 0; i < row; ++i)
		{
			stJoinGroupInfo oGroupMemInfo = {0};
			m_db->GetFromRes(oGroupMemInfo.userID,		&res, i, 0);
			m_db->GetFromRes(oGroupMemInfo.role_type,	&res, i, 1);
			m_db->GetFromRes(oGroupMemInfo.levelID,		&res, i, 2);
			oGroupMemInfo.groupID = groupid;
			oGroupMemLst.push_back(oGroupMemInfo);
		}

	} while (0);
	m_db->CleanRes(&res);

	return row;
}

int CDBTools::getGroupIDLst_DB(std::list<unsigned int> & groupIDLst)
{
	groupIDLst.clear();
	char query[256] = {0};
	Result res;
	int row = 0;
	sprintf(query, "select id from mc_group where is_del=0 and status=1");

	do 
	{
		if (!m_db->build_sql_run(&res, &row, query) || 0 == row) 
		{
			row = -1;
			break;
		}

		for (int i = 0; i < row; ++i)
		{
			unsigned int groupid;
			m_db->GetFromRes(groupid,		&res, i, 0);
			groupIDLst.push_back(groupid);
		}

	} while (0);
	m_db->CleanRes(&res);

	return row;
}

bool CDBTools::getGroupMaxUnreadNum(uint32 groupid, int &num)
{
	num = 0;
	char query[256] = {};
	Result res;
	int row = 0;
	sprintf(query, " select unread_msgnum from mc_group where id = %u ", groupid);
	if (!m_db->run_sql(query, &res, &row) || 0 == row)
	{
		m_db->CleanRes(&res);
		return false;
	}

	m_db->GetFromRes(num, &res, 0, 0);
	m_db->CleanRes(&res);
	return true;
}

int CDBTools::getUserRelateGroupLst_DB(unsigned int userid, std::list<stGroupInfo> & groupLst)
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
		if (!m_db->run_sql(strQuerySQL.str().c_str(), &res, &row))
		{
			ret = -1;
			break;
		}

		for (int i = 0; i < row; ++i)
		{
			stGroupInfo oGroupInfo;
			m_db->GetFromRes(oGroupInfo.groupid,	&res, i, 0);
			m_db->GetFromRes(oGroupInfo.headaddr,	&res, i, 1);
			m_db->GetFromRes(oGroupInfo.name,		&res, i, 2);
			m_db->GetFromRes(oGroupInfo.showid,		&res, i, 3);
			m_db->GetFromRes(oGroupInfo.muteSet,	&res, i, 4);
			m_db->GetFromRes(oGroupInfo.roleType,	&res, i, 5);
			m_db->GetFromRes(oGroupInfo.memberCount,	&res, i, 6);
			m_db->GetFromRes(oGroupInfo.visitCount,		&res, i, 7);
			groupLst.push_back(oGroupInfo);
		}
		ret = row;
	} while (0);

	m_db->CleanRes(&res);
	return ret;
}

int CDBTools::get_userGroupInfo_DB(unsigned int userid, unsigned int groupid, StUserGroupInfo & oUserGroupInfo)
{
//	if (!userid || !groupid)
//	{
//		LOG_PRINT(log_error, "DB get_userGroupInfo_DB input error.userid:%u,groupid:%u.", userid, groupid);
//		return -1;
//	}
//
//	std::stringstream strQuerySQL;
//	strQuerySQL << " select role_type,mute_notifications,custom_gain,private_chat_intimacy,is_auth ";
//	strQuerySQL << " from mc_group_member where user_id=" << userid << " and group_id=" << groupid << " and is_del=0";
//
//	int ret = ERR_CODE_FAILED_DB;
//	Result res;
//	int row = 0;
//
//	do
//	{
//		if (!m_db->run_sql(strQuerySQL.str().c_str(), &res, &row))
//		{
//			LOG_PRINT(log_error, "query user group info fail in db.userid:%u,groupid:%u.", userid, groupid);
//			break;
//		}
//
//		if (row == 0)
//		{
//			LOG_PRINT(log_error, "[DB check]user is not in the mc_group_member.userid:%u,groupid:%u.", userid, groupid);
//			ret = ERR_CODE_USER_NOT_IN_GROUP;
//			break;
//		}
//
//		oUserGroupInfo.userID = userid;
//		oUserGroupInfo.groupID = groupid;
//		m_db->GetFromRes(oUserGroupInfo.roleType, 		&res, 0, 0);
//		m_db->GetFromRes(oUserGroupInfo.noDisturbing, 	&res, 0, 1);
//		m_db->GetFromRes(oUserGroupInfo.gainPercent, 	&res, 0, 2);
//		m_db->GetFromRes(oUserGroupInfo.pchatThreshold, &res, 0, 3);
//		m_db->GetFromRes(oUserGroupInfo.authState, 		&res, 0, 4);
//		ret = 0;
//	} while (0);
//
//	m_db->CleanRes(&res);
//	return ret;
}

int CDBTools::get_groupUserInfoList_DB(unsigned int groupid, std::map<uint32, StUserGroupInfo> &mGroupUserInfo)
{
	std::stringstream strQuerySQL;
	strQuerySQL << " select user_id,role_type,mute_notifications,custom_gain,private_chat_intimacy,is_auth ";
	strQuerySQL << " from mc_group_member where group_id=" << groupid << " and is_del=0";

	Result res;
	int row = 0;
	do
	{
		if (!m_db->run_sql(strQuerySQL.str().c_str(), &res, &row))
		{
			LOG_PRINT(log_error, "query group user info list fail in db.groupid:%u.", groupid);
			break;
		}

		for (int i = 0; i < row; i++)
		{
			StUserGroupInfo oInfo;
			oInfo.groupID = groupid;
			m_db->GetFromRes(oInfo.userID,			&res, i, 0);
			m_db->GetFromRes(oInfo.roleType,		&res, i, 1);
			m_db->GetFromRes(oInfo.noDisturbing,	&res, i, 2);
			m_db->GetFromRes(oInfo.gainPercent,		&res, i, 3);
			m_db->GetFromRes(oInfo.pchatThreshold,	&res, i, 4);
			m_db->GetFromRes(oInfo.authState,		&res, i, 5);

			mGroupUserInfo[oInfo.userID] = oInfo;
		}
	} while (0);

	m_db->CleanRes(&res);
	return row;
}

int CDBTools::get_userBaseAndGroupInfo_DB(unsigned int userid, unsigned int groupid, StUserBaseAndGroupInfo & oUserInfo)
{
	if (!userid || !groupid)
	{
		LOG_PRINT(log_error, "DB get_userBaseAndGroupInfo_DB input error.userid:%u,groupid:%u.", userid, groupid);
		return -1;
	}

	std::stringstream strQuerySQL;
	strQuerySQL << " select b.alias,b.gender,b.headaddr,a.role_type,a.mute_notifications,a.custom_gain,a.private_chat_intimacy,a.is_auth from mc_group_member a,mc_user b ";
	strQuerySQL << " where a.user_id = b.id and a.user_id=" << userid << " and a.group_id=" << groupid << " and a.is_del=0";

	int ret = -1;
	Result res;
	int row = 0;

	do 
	{
		if (!m_db->run_sql(strQuerySQL.str().c_str(), &res, &row))
		{
			LOG_PRINT(log_error, "query user group info fail in db.userid:%u,groupid:%u.", userid, groupid);
			break;
		}

		if (row == 0)
		{
			LOG_PRINT(log_error, "userid:%u is not member of groupid:%u.", userid, groupid);
			ret = ERR_CODE_USER_NOT_IN_GROUP;
			break;
		}

		oUserInfo.userGroupInfo.userID = userid;
		oUserInfo.userGroupInfo.groupID = groupid;
		m_db->GetFromRes(oUserInfo.userAlias,	&res, 0, 0);
		m_db->GetFromRes(oUserInfo.userGender,	&res, 0, 1);
		m_db->GetFromRes(oUserInfo.userHead,	&res, 0, 2);
		m_db->GetFromRes(oUserInfo.userGroupInfo.roleType,	&res, 0, 3);
		m_db->GetFromRes(oUserInfo.userGroupInfo.noDisturbing,	&res, 0, 4);
		m_db->GetFromRes(oUserInfo.userGroupInfo.gainPercent,		&res, 0, 5);
		m_db->GetFromRes(oUserInfo.userGroupInfo.pchatThreshold, 	&res, 0, 6);
		m_db->GetFromRes(oUserInfo.userGroupInfo.authState, 		&res, 0, 7);
		ret = 0;
	} while (0);

	m_db->CleanRes(&res);
	return ret;
}

int CDBTools::get_groupUserInfoList_EX(unsigned int groupid, std::map<uint32, StUserBaseAndGroupInfo> &mUserInfo)
{
	if (!groupid)
	{
		LOG_PRINT(log_error, "DB get_groupUserInfoList_EX input error.groupid:%u.", groupid);
		return -1;
	}

	std::stringstream strQuerySQL;
	strQuerySQL << " select b.id, b.alias,b.gender,b.headaddr,a.role_type,a.mute_notifications,a.custom_gain,a.private_chat_intimacy,a.is_auth from mc_group_member a,mc_user b ";
	strQuerySQL << " where a.user_id = b.id and a.group_id=" << groupid << " and a.is_del=0";

	Result res;
	int row = 0;

	do
	{
		if (!m_db->run_sql(strQuerySQL.str().c_str(), &res, &row) || 0 == row)
		{
			LOG_PRINT(log_error, "query group user info list fail in db.groupid:%u.", groupid);
			break;
		}

		for (int i = 0; i < row; i++)
		{
			StUserBaseAndGroupInfo oUserInfo;
			oUserInfo.userGroupInfo.groupID = groupid;
			int j = 0;
			m_db->GetFromRes(oUserInfo.userGroupInfo.userID,			&res, i, j++);
			m_db->GetFromRes(oUserInfo.userAlias,						&res, i, j++);
			m_db->GetFromRes(oUserInfo.userGender,					&res, i, j++);
			m_db->GetFromRes(oUserInfo.userHead,						&res, i, j++);
			m_db->GetFromRes(oUserInfo.userGroupInfo.roleType,		&res, i, j++);
			m_db->GetFromRes(oUserInfo.userGroupInfo.noDisturbing, 	&res, i, j++);
			m_db->GetFromRes(oUserInfo.userGroupInfo.gainPercent,		&res, i, j++);
			m_db->GetFromRes(oUserInfo.userGroupInfo.pchatThreshold, 	&res, i, j++);
			m_db->GetFromRes(oUserInfo.userGroupInfo.authState, 		&res, i, j++);

			mUserInfo[oUserInfo.userGroupInfo.userID] = oUserInfo;
		}

	} while (0);

	m_db->CleanRes(&res);
	return row;
}

bool CDBTools::addUserGroup_DB(const stJoinGroupInfo & oJoinObj)
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
		return m_db->run_sql(strUpdateSQL.str().c_str());
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
		return m_db->run_sql(strInsertSQL.str().c_str());
	}
}

bool CDBTools::delUserGroup_DB(unsigned int userid, unsigned int groupid)
{
	if (!userid || !groupid)
	{
		LOG_PRINT(log_error, "DB user quit group input error.userid:%u,groupid:%u.", userid, groupid);
		return false;
	}

	time_t now_time = time(NULL);
	//just mark this user's quit state of this group
	char cUpdateSQL[256] = {0};
	snprintf(cUpdateSQL, sizeof(cUpdateSQL), "update mc_group_member set is_del=1,out_time=%d where user_id=%u and group_id=%u", (int)now_time, userid, groupid);
	return m_db->run_sql(cUpdateSQL);
}

bool CDBTools::isUserInGroup(unsigned int userid, unsigned int groupid, bool checkDBRecord)
{
	if (!userid || !groupid)
	{
		LOG_PRINT(log_error, "DB check isUserInGroup input error.userid:%u,groupid:%u.", userid, groupid);
		return -1;
	}

	bool ret = false;
	Result res;
	int row = 0;

	char cQuerySQL[256] = {0};
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
		if (!m_db->run_sql(cQuerySQL, &res, &row))
		{
			break;
		}

		if (row)
		{
			ret = true;
		}

	} while (0);

	m_db->CleanRes(&res);
	return ret;
}

bool CDBTools::updateUserRoletype_DB(unsigned int userid, unsigned int groupid, unsigned int new_roletype)
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
	if (!m_db->build_sql_run(&res, &row, strQuerySQL.str().c_str())) 
	{
		m_db->CleanRes(&res);
		return false;
	}

	if (0 == row)
	{
		//user is visitor in db
		old_roletype = e_VisitorRole;
		m_db->CleanRes(&res);
	}
	else
	{
		//user is member in db
		m_db->GetFromRes(old_roletype, &res, 0, 0);
		m_db->CleanRes(&res);
	}

	if (old_roletype == new_roletype)
	{
		LOG_PRINT(log_info, "[DB check]user is same roletype in group.not need to handle anymore.userid:%u,groupid:%u,roletype:%u", userid, groupid, new_roletype);
		return true;
	}
	else if (old_roletype == e_VisitorRole)
	{
		LOG_PRINT(log_warning, "[DB check]user upgrade from visitor to group member,need to join group.userid:%u,groupid:%u,new roletype:%u.", userid, groupid, new_roletype);
		stJoinGroupInfo oJoinObj = {0};
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
		return m_db->run_sql(strUpdateSQL.str().c_str());
	}
}
int CDBTools::SendPressStampReq(unsigned int userid, unsigned int  dstuserid, unsigned int groupID, unsigned int money, std::string strMessage )
{
	Result res;
	std::stringstream strFunSQL;
	strFunSQL << "select press_stamp(" << userid << "," << groupID << "," << dstuserid << "," << money << "," << strMessage << ")";
	int row = 0;
	int ret = 0;
	do
	{
		if (!m_db->run_sql(strFunSQL.str().c_str(), &res, &row))
		{
			ret = -1;
			break;
		}

		m_db->GetFromRes(ret, &res, 0, 0);

	} while (0);
	m_db->CleanRes(&res); 
	return ret;
}
int CDBTools::getStampInfo(unsigned int userid, unsigned int  dstuserid, unsigned int groupID, CMDStampInfo& info)
{
	Result res;
	int row = 0;
	std::stringstream strSelectSQL;
	unsigned int tmp;
	std::string strMsg;
	 
	strSelectSQL << " select id, src_user, dst_user, press_num, total_money, message from mc_stamp_info where ";
	strSelectSQL << "group_id = " << groupID;
	strSelectSQL << " and src_user = " << userid;
	strSelectSQL << " and dst_user = " << dstuserid;
	if (!m_db->build_sql_run(&res, &row, strSelectSQL.str().c_str()))
	{
		return -1;
	}
	 
	LOG_PRINT(log_debug0, "[geStampInfo] row:%u.", row);
	for (int i = 0; i < 1; i++)
	{
		
		m_db->GetFromRes(tmp, &res, i, 0);
		info.set_stampid(tmp);
		m_db->GetFromRes(tmp, &res, i, 1);
		info.set_userid(tmp);
		m_db->GetFromRes(tmp, &res, i, 2);
		info.set_dstuserid(tmp);
		m_db->GetFromRes(tmp, &res, i, 3);
		info.set_pressnum(tmp);
		m_db->GetFromRes(tmp, &res, i, 4);
		info.set_money(tmp);
		m_db->GetFromRes(strMsg, &res, i, 5);
		info.set_message(strMsg);
	}

	m_db->CleanRes(&res);
}
int CDBTools::getFixtimeRedPacketData(const std::stringstream &strSelectSQL, unsigned int fixtype, unsigned int &effrow, std::list<stRedPacketInfo> &lstPacketInfo)
{	
	Result res;
	int row = 0;

	if (!m_db->build_sql_run(&res, &row, strSelectSQL.str().c_str()))
	{
		return -1;
	}
	effrow= row;	 
	LOG_PRINT(log_debug0, "[getFixtimeRedPacketData] row:%u.", row);
	for (int i = 0; i < row; i++)
	{
		stRedPacketInfo info;		
		info.moneyState = 0;		
		m_db->GetFromRes(info.packetID, &res, i, 0);
		m_db->GetFromRes(info.groupID, &res, i, 1);
		m_db->GetFromRes(info.srcUserID, &res, i, 2);
		m_db->GetFromRes(info.dstUserID, &res, i, 3);
		m_db->GetFromRes(info.packetType, &res, i, 4);
		m_db->GetFromRes(info.rangeType, &res, i, 5);
		m_db->GetFromRes(info.packetNum, &res, i, 6);
		m_db->GetFromRes(info.takeNum, &res, i, 7);
		m_db->GetFromRes(info.createTime, &res, i, 8);
		m_db->GetFromRes(info.fixPerMoney, &res, i, 9);
		m_db->GetFromRes(info.packetMoney, &res, i, 10);
		m_db->GetFromRes(info.takeMoney, &res, i, 11);
		m_db->GetFromRes(info.strMessage, &res, i, 12);
		m_db->GetFromRes(info.isRecycle, &res, i, 13);
		m_db->GetFromRes(info.finishTime, &res, i, 14);
		m_db->GetFromRes(info.isPrivate, &res, i, 15);
		m_db->GetFromRes(info.rangeGender, &res, i, 16);
		m_db->GetFromRes(info.fixTime, &res, i, 17);
		m_db->GetFromRes(info.fixType, &res, i, 18);
		int diff = 0;
		m_db->GetFromRes(diff, &res, i, 19);
		info.diffTime = diff;
		if (diff < 0)
		{
			info.diffTime = 0;
		}
		m_db->GetFromRes(info.longitude, &res, i, 20);
		m_db->GetFromRes(info.latitude, &res, i, 21);
		lstPacketInfo.push_back(info);
	}
	
	m_db->CleanRes(&res);
	return 0;
}
int CDBTools::getFixtimeRedPacketInfoByGrouID(unsigned int userid, unsigned int groupID, unsigned int currow, unsigned int rows, unsigned int fixtype, unsigned int & effrow, unsigned int packtype, std::list<stRedPacketInfo> &lstPacketInfo)
{
	if (!groupID)
	{
		LOG_PRINT(log_error, "[getFixtimeRedPacketInfoByGrouID]groupID is 0.");
		return -1;
	}
	
	std::stringstream strSelectSQL;
	 
	 
	//已经开抢的定时红包	
	strSelectSQL << "(select * from (select  a.id, a.group_id,a.src_user,a.dst_user,a.type,a.range_type,a.packet_num,a.take_num,a.create_time,a.fix_per_money,a.packet_money,a.take_money,a.message,a.is_recycle,a.finish_time,a.private_flag,a.range_gender,a.fix_time, 0,a.fix_time-UNIX_TIMESTAMP(),a.longitude,a.latitude from mc_redpacket_info a ";
	strSelectSQL << "where  a.id not in (select b.packet_id  from mc_redpacket_log b where a.id=b.packet_id and b.take_user = " << userid;//过滤掉自己抢过的红包	 
	strSelectSQL << " ) and  group_id  = " << groupID;
	strSelectSQL << " and is_recycle  = 0";//未抢完,未回收
	strSelectSQL << " and fix_time  <=  UNIX_TIMESTAMP() ";
	strSelectSQL << " and type = " << packtype;
	strSelectSQL << " ORDER BY fix_time  ) as a1 ) ";
	strSelectSQL << " union ";
	
	//未开抢的定时红包	
	strSelectSQL << "(select * from  (select id, group_id,src_user,dst_user,type,range_type,packet_num,take_num,create_time,fix_per_money,packet_money,take_money,message,is_recycle,finish_time,private_flag,range_gender,fix_time,1,fix_time-UNIX_TIMESTAMP(),longitude,latitude from mc_redpacket_info ";
	strSelectSQL << " where group_id  = " << groupID;
	strSelectSQL << " and fix_time  >  UNIX_TIMESTAMP()";
	strSelectSQL << " and type = " << packtype;
	strSelectSQL << " ORDER BY fix_time  , packet_money desc ,packet_num  asc) as a2)";
	strSelectSQL << " union ";
	//已抢完	
	strSelectSQL << "(select * from  (select id, group_id,src_user,dst_user,type,range_type,packet_num,take_num,create_time,fix_per_money,packet_money,take_money,message,is_recycle,finish_time,private_flag,range_gender,fix_time,2,fix_time-UNIX_TIMESTAMP(),longitude,latitude from mc_redpacket_info ";
	strSelectSQL << " where group_id  = " << groupID;
	strSelectSQL << " and is_recycle  = 1";//抢完或者已过期
	strSelectSQL << " and fix_time  <=  UNIX_TIMESTAMP() ";
	strSelectSQL << " and  fix_time >=  UNIX_TIMESTAMP() - 3600 * 24  ";//未到期
	strSelectSQL << " and type = " << packtype;
	strSelectSQL << " and packet_num  = take_num"; 
	strSelectSQL << " ORDER BY fix_time ) as a3)";
	strSelectSQL << " union ";
	//已过期	
	strSelectSQL << "(select * from  (select id, group_id,src_user,dst_user,type,range_type,packet_num,take_num,create_time,fix_per_money,packet_money,take_money,message,is_recycle,finish_time,private_flag,range_gender,fix_time,3,fix_time-UNIX_TIMESTAMP(),longitude,latitude from mc_redpacket_info ";
	strSelectSQL << " where group_id  = " << groupID;
	strSelectSQL << " and is_recycle  = 1";//抢完或者已过期
	strSelectSQL << " and  fix_time  >= UNIX_TIMESTAMP() - 3600*24 ";//已过期   UNIX_TIMESTAMP() - fix_time <= 3600 * 24 
	strSelectSQL << " and type = " << packtype;
	strSelectSQL << " and packet_num  != take_num"; 
	strSelectSQL << " ORDER BY fix_time ) as a4)";
	strSelectSQL << " limit " << currow;
	strSelectSQL << " , " << rows;
	return getFixtimeRedPacketData(strSelectSQL, 0, effrow, lstPacketInfo); 
}
 

int CDBTools::getRedPacketBasicInfo(unsigned int packetID, stRedPacketInfo & packetInfo)
{
	if (!packetID)
	{
		LOG_PRINT(log_error, "[getRedPacketBasicInfo]packetID is 0.");
		return -1;
	}

	std::stringstream strSelectSQL;
	strSelectSQL << "select group_id,src_user,dst_user,type,range_type,packet_num,take_num,create_time,fix_per_money,packet_money,take_money,message,is_recycle,finish_time,private_flag,range_gender,fix_time,fix_time-UNIX_TIMESTAMP(),longitude,latitude,invalid_time from talk_redpacket_info where id=" << packetID;

	int ret = 0;
	Result res;
	int row = 0;
	do 
	{
		if (!m_db->build_sql_run(&res, &row, strSelectSQL.str().c_str())) 
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
		m_db->GetFromRes(packetInfo.groupID, &res, 0, 0);
		m_db->GetFromRes(packetInfo.srcUserID, &res, 0, 1);
		m_db->GetFromRes(packetInfo.dstUserID, &res, 0, 2);
		m_db->GetFromRes(packetInfo.packetType, &res, 0, 3);
		m_db->GetFromRes(packetInfo.rangeType, &res, 0, 4);
		m_db->GetFromRes(packetInfo.packetNum, &res, 0, 5);
		m_db->GetFromRes(packetInfo.takeNum, &res, 0, 6);
		m_db->GetFromRes(packetInfo.createTime, &res, 0, 7);
		m_db->GetFromRes(packetInfo.fixPerMoney, &res, 0, 8);
		m_db->GetFromRes(packetInfo.packetMoney, &res, 0, 9);
		m_db->GetFromRes(packetInfo.takeMoney, &res, 0, 10);
		m_db->GetFromRes(packetInfo.strMessage, &res, 0, 11);
		m_db->GetFromRes(packetInfo.isRecycle, &res, 0, 12);
		m_db->GetFromRes(packetInfo.finishTime, &res, 0, 13);
		m_db->GetFromRes(packetInfo.isPrivate, &res, 0, 14);
		m_db->GetFromRes(packetInfo.rangeGender, &res, 0, 15);
		m_db->GetFromRes(packetInfo.fixTime, &res, 0, 16);
		
		int diff = 0;
		m_db->GetFromRes(diff, &res, 0, 17);
		packetInfo.diffTime = diff;
		m_db->GetFromRes(packetInfo.longitude, &res, 0, 18);
		m_db->GetFromRes(packetInfo.latitude, &res, 0, 19);
		
		if (diff < 0)
		{
			packetInfo.diffTime = 0;
		}
		if (packetInfo.packetMoney <= packetInfo.takeMoney)
		{
			LOG_PRINT(log_warning, "red packet has no left to take.packetID:%u,packet money:%llu,take money:%llu.", packetID, packetInfo.packetMoney, packetInfo.takeMoney);
			packetInfo.moneyState = 1;
		}
		m_db->GetFromRes(packetInfo.invalidTime, &res, 0, 20);

		m_db->GetFromRes(packetInfo.luckyNum, &res, 0, 21);
		LOG_PRINT(log_debug, "getRedPacketBasicInfo invalidTime:%u", packetInfo.invalidTime);
	} while (0);
	m_db->CleanRes(&res);
	return ret;
}

long long CDBTools::takeRandomRedPacket(unsigned int packetID, unsigned int packetType, unsigned int groupID, unsigned int take_userID, unsigned long long rand_money, unsigned int taken_percent/* = 0 */, unsigned int m_goldCandyExchange/* = 1 */)
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
		if (!m_db->run_sql(strFunSQL.str().c_str(), &res, &row))
		{
			ret = -1;
			break;
		}

		m_db->GetFromRes(ret, &res, 0, 0);

	} while (0);
	m_db->CleanRes(&res);
	return ret;
}

long long CDBTools::takeUniRedPacket(unsigned int packetID, unsigned int packetType, unsigned int groupID, unsigned int take_userID, unsigned int taken_percent/* = 0 */, unsigned int m_goldCandyExchange/* = 1 */)
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
		if (!m_db->run_sql(strFunSQL.str().c_str(), &res, &row))
		{
			ret = -1;
			break;
		}

		m_db->GetFromRes(ret, &res, 0, 0);

	} while (0);
	m_db->CleanRes(&res);
	return ret;
}

long long CDBTools::takeDirectRedPacket(unsigned int packetID, unsigned int packetType, unsigned int groupID, unsigned int take_userID, unsigned int taken_percent/* = 0 */, unsigned int m_goldCandyExchange/* = 1 */)
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
		if (!m_db->run_sql(strFunSQL.str().c_str(), &res, &row))
		{
			ret = -1;
			break;
		}

		m_db->GetFromRes(ret, &res, 0, 0);

	} while (0);
	m_db->CleanRes(&res);
	return ret;
}

long long CDBTools::takeRedPacket(unsigned int packetID, unsigned int send_user, unsigned int packetType, unsigned int groupID, unsigned int take_user, unsigned long long take_money, unsigned int taken_percent /*= 0*/, unsigned int goldCandyExchange /*= 1*/)
{
	if (!packetID || !send_user || !groupID || !take_user || taken_percent > 100 || !goldCandyExchange)
	{
		LOG_PRINT(log_error, "[takeRedPacket]input error.packetID:%u,send_user:%u,packetType:%u,groupID:%u,take_user:%u,taken_percent:%u,goldCandyExchange:%u.", 
			packetID, send_user, packetType, groupID, take_user, taken_percent, goldCandyExchange);
		return -1;
	}

	LOG_PRINT(log_info, "[takeRedPacket]input:packetID:%u,send_user:%u,packetType:%u,group:%u,take_user:%u,taken_percent:%u,goldCandyExchange:%u.", 
		packetID, send_user, packetType, groupID, take_user, taken_percent, goldCandyExchange);
	Result res;
	std::stringstream strFunSQL;
	strFunSQL << "select take_redpacket(" << packetID << "," << send_user << "," << packetType << "," << groupID << "," << take_user << "," << take_money << "," << taken_percent << "," << goldCandyExchange << ")";
	int row = 0;
	long long ret = 0;
	do 
	{
		if (!m_db->run_sql(strFunSQL.str().c_str(), &res, &row))
		{
			ret = -1;
			break;
		}

		m_db->GetFromRes(ret, &res, 0, 0);

	} while (0);
	m_db->CleanRes(&res);
	return ret;

}

int CDBTools::getRedPacketLog(uint32 packetID, std::vector<stRedPacketLog> &vLog, uint32 limit/* = 0*/, uint32 offset/* = 0*/)
{
	if (!packetID)
	{
		LOG_PRINT(log_error, "getRedPacketLog check input parameter failed: packetID: %u.", packetID);
		return 0;
	}

	std::stringstream strSql;
	strSql << "select take_user, take_money * 100, time from mc_redpacket_log where packet_id = " << packetID << " order by time limit " << (limit ? limit: 20) << " offset " << offset;
	int row = 0;
	Result res;
	do 
	{
		if (!m_db->run_sql(strSql.str().c_str(), &res, &row) || 0 == row)
		{
			break;
		}

		vLog.resize(row);
		for (int i = 0; i < row; i++)
		{
			vLog[i].packetId = packetID;
			m_db->GetFromRes(vLog[i].userId,	&res, i, 0);
			m_db->GetFromRes(vLog[i].money,		&res, i, 1);
			m_db->GetFromRes(vLog[i].time,		&res, i, 2);
		}
	} while (0);
	
	m_db->CleanRes(&res);
	return row;
}

int CDBTools::getRedPacketToRecycle(unsigned int timeout, std::vector<unsigned int> &vList)
{
	if (timeout <= 0)
		return 0;
	
	std::stringstream sql;
	sql << "select id from talk_redpacket_info where create_time <= UNIX_TIMESTAMP() - " << timeout << " and is_recycle = 0";
	int row = 0;
	Result res;
	do
	{
		if (!m_db->run_sql(sql.str().c_str(), &res, &row) || 0 == row)
			break;

		vList.resize(row);
		for (int i = 0; i < row; i++)
		{
			m_db->GetFromRes(vList[i], &res, i, 0);
		}
	} while(0);

	m_db->CleanRes(&res);

	return row;
}

bool CDBTools::recycleOneRedPacket(unsigned int packetID, unsigned int goldCandyExchange, std::string &result)
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
	if (!m_db->run_sql(sql.str().c_str(), &res, &row) || 0 == row)
	{
		m_db->CleanRes(&res);
		return false;
	}

	m_db->GetFromRes(result, &res, 0, 0);
	m_db->CleanRes(&res);
	return true;
}
bool CDBTools::getWeekCharmUserLst(unsigned int groupid, std::list<stContributeValue> & oContributeLst)
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
	if (!m_db->run_sql(strQuerySQL.str().c_str(), &res, &row) || 0 == row)
	{
		m_db->CleanRes(&res);
		return false;
	}

	for (int i = 0; i < row; ++i)
	{
		stContributeValue oUser;
		m_db->GetFromRes(oUser.userID, &res, i, 0);
		m_db->GetFromRes(oUser.near_week_contribute, &res, i, 1);
		oUser.groupID = groupid;
		oContributeLst.push_back(oUser);
	}

	m_db->CleanRes(&res);
	return true;
}

bool CDBTools::getWeekContributeUserLst(unsigned int groupid, std::list<stContributeValue> & oContributeLst)
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
	if (!m_db->run_sql(strQuerySQL.str().c_str(), &res, &row) || 0 == row)
	{
		m_db->CleanRes(&res);
		return false;
	}

	for (int i = 0; i < row; ++i)
	{
		stContributeValue oUser;
		m_db->GetFromRes(oUser.userID,				&res, i, 0);
		m_db->GetFromRes(oUser.near_week_contribute,&res, i, 1);
		oUser.groupID = groupid;
		oContributeLst.push_back(oUser);
	}

	m_db->CleanRes(&res);
	return true;
}

long long CDBTools::getUserTakeMoney(unsigned int userID, unsigned int packetID)
{
	if (!userID || !packetID)
	{
		LOG_PRINT(log_error, "getUserTakeMoney input error,userID:%u,packetID:%u.", userID, packetID);
		return 0;
	}

	std::stringstream strQuerySQL;
	strQuerySQL << "select take_money * 100 from mc_redpacket_log where take_user=" << userID << " and packet_id=" << packetID;
	int row = 0;
	Result res;
	long long money = 0;
	do 
	{
		if (!m_db->run_sql(strQuerySQL.str().c_str(), &res, &row))
		{
			break;
		}

		if (0 == row)
		{
			LOG_PRINT(log_warning, "this user have not taken this packet,userID:%u,packetID:%u.", userID, packetID);
			break;
		}

		m_db->GetFromRes(money, &res, 0, 0);

	} while (0);
	m_db->CleanRes(&res);
	return money;
}

bool CDBTools::getGroupGainPercent(uint32 groupid, uint32 userid, uint32 &percent)
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
	if (!m_db->run_sql(sql.str().c_str(), &res, &row) || 0 == row)
	{
		m_db->CleanRes(&res);
		return false;
	}

	m_db->GetFromRes(percent, &res, 0, 0);
	m_db->CleanRes(&res);
	return true;
}

unsigned int CDBTools::getTopRedPacketTakeUserID(unsigned int packetID)
{
	unsigned int topuserid = 0;
	std::stringstream strSQL;
	strSQL << " select take_user from mc_redpacket_log where packet_id = " << packetID << " order by take_money desc limit 1";

	int row = 0;
	Result res;
	if (!m_db->run_sql(strSQL.str().c_str(), &res, &row) || 0 == row)
	{
		m_db->CleanRes(&res);
		return topuserid;
	}

	m_db->GetFromRes(topuserid, &res, 0, 0);
	m_db->CleanRes(&res);
	return topuserid;
}

bool CDBTools::check_user_has_takephoto(unsigned int userID, unsigned int packetID)
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
		if (!m_db->run_sql(strQuerySQL.str().c_str(), &res, &row) || 0 == row)
		{
			break;
		}
		else
		{
			ret = true;
		}

	} while (0);
	m_db->CleanRes(&res);
	return ret;
}


int CDBTools::select_luckynum_by_packetid(unsigned int packetID)
{
	if (!packetID)
	{
		LOG_PRINT(log_error, "select_luckynum_by_packetid input error.packetID:%u.", packetID);
		return false;
	}

	int luckynum = 0;
	std::stringstream strQuerySQL;
	strQuerySQL << "select lucky_num from talk_redpacket_info where id=" << packetID ;
	int row = 0;
	Result res;
	do 
	{
		if (!m_db->run_sql(strQuerySQL.str().c_str(), &res, &row) || 0 == row)
		{
			break;
		}
		else
		{
			
			m_db->GetFromRes(luckynum, &res, 0, 0);
			LOG_PRINT(log_debug, "==luckynum:%d.", luckynum);
			if (luckynum== 0)
			{
				LOG_PRINT(log_error, "luckynum:%d.", luckynum);
				break;
			}

		}

	} while (0);
	m_db->CleanRes(&res);
	return luckynum;
}


//查出课程直播间PPT图片最大序号的图片标识
int CDBTools::getPPTMaxPicIdByID(const int32 &groupId, string & sPicId)
{
	int row = 0;
	char query[256];

	sprintf(query, "select qiniuKey  from talk_course_ppt_img where sort= (select max(sort) sort from  talk_course_ppt_img where course_id=%d ) and course_id=%d ORDER BY create_time ", groupId,groupId);

	LOG_PRINT(log_info, "query:%s.", query);
	Result res;

	if (!m_db->run_sql(query, &res, &row) || 0 == row)
	{
		LOG_PRINT(log_error, "error or empty ,query:[%s]", query);
		m_db->CleanRes(&res);
		return 0;
	}

	m_db->GetFromRes(sPicId, &res, 0, 0);

	m_db->CleanRes(&res);

	return 0;
}

//查出课程直播间PPT图片,序号从大到小,如果排序号一样，按照时间从小到大排列
int CDBTools::getPPTPicIdsByGroupID(const int32 &groupId, std::vector<pptPicInfo> & vPicInfo	)
{
	int row = 0;
	char query[256];

	sprintf(query, "select course_id, imgUrl,sort,qiniuKey from talk_course_ppt_img where course_id=%d  and exists(select 1 from talk_course where id=%d and form=3) order by sort desc ,create_time asc", groupId,groupId);

	Result res;

	do
	{
		if (!m_db->run_sql(query, &res, &row) || 0 == row)
			break;

		vPicInfo.resize(row);
		for (int i = 0; i < row; i++)
		{
			m_db->GetFromRes(vPicInfo[i].iGroupId, &res, i, 0);
			m_db->GetFromRes(vPicInfo[i].sPicUrl, &res, i, 1);
			m_db->GetFromRes(vPicInfo[i].iRank, &res, i, 2);
			m_db->GetFromRes(vPicInfo[i].sPicId, &res, i, 3);
		}
	} while(0);

	m_db->CleanRes(&res);

	return row;

	return 0;
}

bool CDBTools::insertCommentAudit(const commentAudit &tcommentAudit	)
{
	char query[4096] = {0};
	int now=time(NULL);
	sprintf(query, "INSERT INTO talk_comment_audi(uid, groupid,content,msgid,pmsgid,msgtype,state,createtime,updatetime) VALUES (%d,  %d, '%s', %d, %d, %d,%d,%d,%d) ",
		tcommentAudit.iUid,
		tcommentAudit.iGroupid,
		tcommentAudit.sContent.c_str(),
		tcommentAudit.iMsgid,
		tcommentAudit.iPmsgid,
		tcommentAudit.iMsgtype,
		tcommentAudit.iStatus,
		now,now);

	LOG_PRINT(log_info, "query:%s.", query);
	return m_db->run_sql(query);
}

bool CDBTools::updateCommentAuditMsgId(int modifyId,int msgId,int pMsgId )
{
	if (!modifyId || !msgId || !pMsgId )
		return false;

	std::stringstream sql;
	sql << " update talk_comment_audi set msgid=" << msgId <<
		" where id =  " <<modifyId<<
		" and   pmsgid =  " <<pMsgId;
	if (!m_db->run_sql(sql.str().c_str()))
		return false;

	return m_db->getAffectedRow() > 0;
}


bool CDBTools::updGroupMemberInfo(uint32 groupid, uint32 userid, std::map<std::string, std::string> &mValues)
{
	if (!groupid || !userid || mValues.empty())
		return false;

	std::stringstream sql, sqlValues;
	buildSetValuesList(sqlValues, mValues);
	sql << " update mc_group_member set " << sqlValues.str() <<
			" where is_del = 0 " <<
			"   and group_id = " << groupid <<
			"   and user_id = " << userid;

	if (!m_db->run_sql(sql.str().c_str()))
		return false;

	return m_db->getAffectedRow() > 0;
}


bool CDBTools::updGroupBasicInfo(uint32 groupid, std::map<std::string, std::string> &mValues)
{
	if (!groupid || mValues.empty())
		return false;

	std::stringstream sql, sqlValues;
	buildSetValuesList(sqlValues, mValues);
	sql << " update mc_group set " << sqlValues.str() <<
		" where is_del = 0 and id = " << groupid;

	if (!m_db->run_sql(sql.str().c_str()))
		return false;

	return m_db->getAffectedRow() > 0;
}

bool CDBTools::updGroupPluginSetting(uint32 groupid, std::map<std::string, std::string> &mValues)
{
	if (!groupid || mValues.empty())
		return false;

	std::stringstream sql, sqlValues;
	buildSetValuesList(sqlValues, mValues);
	sql << " update mc_group_plugin_setting_detail set " << sqlValues.str() <<
		" where group_id = " << groupid;

	if (!m_db->run_sql(sql.str().c_str()))
		return false;

	return m_db->getAffectedRow() > 0;
}

void CDBTools::buildSetValuesList(std::stringstream &sqlValues, const std::map<std::string, std::string> &mValues)
{
	for (std::map<std::string, std::string>::const_iterator it = mValues.begin(); it != mValues.end(); it++)
	{
		if (sqlValues.str().length() > 0)
			sqlValues << ",";
		sqlValues << it->first << " = '" << it->second << "'";
	}
}

std::string CDBTools::buildSendRedPacketSQL(const stRedPacketInfo & oPacketReq, unsigned int gold_contribute_exchange/* = 1*/)
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
	strFunSQL << (oPacketReq.isPrivate ? 1 : 0) << ",";
	strFunSQL << oPacketReq.fixTime ;	
	strFunSQL << ")";
	return strFunSQL.str();
}

bool CDBTools::checkSponsorTreasure(uint32 groupid)
{
	std::string strSql = stringFormat(" select paramvalue > (select count(1) from mc_treasure_info ti where ti.group_id = %u and ti.end_time is null ) "
									  "   from mc_sys_parameter where paramname = 'MAX_TREASURE_NUM' ", groupid);

	bool ret = false;
	Result res;
	int row = 0;
	do
	{
		if (!m_db->run_sql(strSql.c_str(), &res, &row))
		{
			break;
		}

		if (0 == row)
		{
			ret = true;
			break;
		}

		int pass = 0;
		m_db->GetFromRes(pass, &res, 0, 0);
		ret = (1 == pass);
	} while(0);
	m_db->CleanRes(&res);

	return ret;
}


bool CDBTools::insertLiveState(uint32 iGroupId,uint32 iUid,int state,const string&sLiveUrl)
{
	char query[4096] = {0};
	int now=time(NULL);
	sprintf(query, "INSERT INTO talk_live_state(uid, groupid, liveurl, state,createtime,updatetime) VALUES (%d,  %d, '%s', %d, %d,%d) ON DUPLICATE KEY UPDATE state=%d,updatetime=%d",
		iUid,
		iGroupId,
		sLiveUrl.c_str(),
		state,
		now,now,state,now);

	return m_db->run_sql(query);
}

bool CDBTools::getLiveUrlByGroupid(uint32 iGroupId, string&sLiveUrl)
{
	char query[4096] = {0};

	sprintf(query, "select liveurl from talk_live_state where groupid=%d and state=1",iGroupId);
	int row=0;
	Result res;

	if (!m_db->run_sql(query, &res, &row) || 0 == row)
	{
		m_db->CleanRes(&res);
		return false;
	}

	m_db->GetFromRes(sLiveUrl, &res, 0, 0);

	m_db->CleanRes(&res);

	return true;
}


bool CDBTools::insertLiveFlow(uint32 iGroupId,uint32 iUid,int state,const string&sLiveUrl)
{
	char query[4096] = {0};
	int now=time(NULL);
	sprintf(query, "INSERT INTO talk_live_flow(uid, groupid, liveurl, state,createtime) VALUES (%d,  %d, '%s', %d, %d)",
		iUid,
		iGroupId,
		sLiveUrl.c_str(),
		state,
		now);

	return m_db->run_sql(query);
}
