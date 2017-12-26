#ifndef __DBCONNECTION_H__
#define __DBCONNECTION_H__

#include <list>
#include <vector>
#include <map>
#include <db/optable2.h>
#include <stdarg.h>
#include "CLogThread.h"
#include "message_comm.h"
#include "message_vchat.h"
#include "db/sink.h"
#include "db/pool.h"
#include <string>
#include "Config.h"
#include "timer.h"
#include "RuntimeClock.h"
using namespace std;

#define DAY 0
#define MONTH 1
#define YEAR 2
#define TIME_INTEVAL	5
#define MAX_SQL_SIZE 4096
#define TMP_SIZE 256
//////////////////////////////////////////////////////////////////////////
#define NASSID 0
#define ISMOBILE 1
typedef void(*FunHandler)(string);
class Dbconnection {
public:
	Dbconnection()
	{
		timerid = -1;
		sink = NULL; 
	}

	Dbconnection(string ip,int port,string dbname,string user,string password,string confname,FunHandler notify);

	~Dbconnection() {
		closeConnection();
	}

	bool resetConnection(string ip,int port,string dbname,string user,string password,string confname,FunHandler notify);

	void keepalive();

	void closeConnection()
	{
		if (timerid != -1)
		{
			Timer::getInstance()->del(timerid);
			sink_disconnect(sink);
			sink_cleanup(sink);
		}
	}

	int transBegin()
	{
		return trans_begin(sink);
	}

	void transCommit()
	{
		trans_commit(sink);
	}

	void transRollBack()
	{
		trans_rollback(sink);
	}

	/*
	* @description : 并运行SQL
	* @input param : *sink          -- 数据库链接，保证在外部可以调用事务锁住同一条链接
	* @output param : *res          -- 结果 不用可填0
	* @output param : *row          -- 行数 不用可填0
	* @input param : *query         -- SQL
	* @return: true               -- success
			   false              -- false
	*/
	inline bool get_DB_default(Result *res, const char *query, int *row)
	{
		CRuntimeClock clock;
		int state = get_DB(sink, res, query, row);
		if((state != RES_COMMAND_OK) &&
			 (state != RES_TUPLES_OK)  &&
			 (state != RES_COPY_IN)    &&
			 (state != RES_COPY_OUT)) {
			LOG_PRINT(log_warning, "SQL:%s Failed, state:%d, error:%s!", query, state, sink_get_error(sink));
			return false;
		}
		clock.end();
		if (row)
		{
			LOG_PRINT(log_debug, "SQL:%s success! records %d, affected %d, cost %sms.", query, *row, getAffectedRow(), clock.runtime_ms_str());
		}
		else
		{
			LOG_PRINT(log_debug, "SQL:%s success! affected %d, cost %sms.", query, getAffectedRow(), clock.runtime_ms_str());
		}
		return true;
	}
	
	/*
	* @description : 合成一个SQL字符串 并运行
	* @input param : *sink          -- 数据库链接，保证在外部可以调用事务锁住同一条链接
	* @output param : *res          -- 结果 不用可填0
	* @output param : *row          -- 行数 不用可填0
	* @input param : *query         -- 存放SQL的空间 不用可填0
	* @input param : querylen       -- 存放SQL的空间长度 不用可填0
	* @input param : SQL_format     -- 格式串
	* @input param : ...            -- 格式串参数
	* @return: true               -- success
	           false              -- false
	*/
	inline bool build_sql_run_nolog(Result *res, int *row, char *query, int querylen, const char *SQL_format, ...)
	{
		if (sink == NULL) return false;
		Result tmp_Res;	
		char *pQuery;
		char tmp_query[MAX_SQL_SIZE];
		int QueryLen;
		if (query) 
		{
			pQuery = query;	
			QueryLen = querylen;
		} 
		else 
		{
			pQuery = tmp_query;
			QueryLen = MAX_SQL_SIZE;
		}
		
		va_list args;
		va_start(args, SQL_format);
		int n = vsnprintf(pQuery, QueryLen, SQL_format, args);
		va_end(args);
		if (n > QueryLen) return false;
		bool ret = get_DB_default(&tmp_Res, pQuery, row);
		
		if (res) 
			*res = tmp_Res;
		else 	
			result_clean(sink, &tmp_Res);
		
		return ret;
	
	}
	
	/*
	* @description : 并运行SQL
	* @input param : *sink          -- 数据库链接，保证在外部可以调用事务锁住同一条链接
	* @output param : *res          -- 结果 不用可填0
	* @output param : *row          -- 行数 不用可填0
	* @input param : *query         -- SQL
	* @return: true               -- success
	           false              -- false
	*/
	inline bool get_DB_default_nolog(Result *res, const char *query, int *row)
	{
		int state = get_DB(sink, res, query, row);
		if((state != RES_COMMAND_OK) &&
			 (state != RES_TUPLES_OK)  &&
			 (state != RES_COPY_IN)    &&
			 (state != RES_COPY_OUT)) {
			return false;
		}
		return true;
	}

	/*
	* @description : 合成一个SQL字符串 并运行
	* @output param : *res          -- 结果 不用可填0
	* @output param : *row          -- 行数 不用可填0
	* @input param : *query         -- 存放SQL的空间 不用可填0
	* @input param : querylen       -- 存放SQL的空间长度 不用可填0
	* @input param : SQL_format     -- 格式串
	* @input param : ...            -- 格式串参数
	*/
	inline bool build_sql_run(Result *res, int *row, const char *SQL_format, ...) 
	{
		
		Result tmp_Res;
			
		char *pQuery;
		char tmp_query[MAX_SQL_SIZE];
		int QueryLen;
	
		pQuery = tmp_query;
		QueryLen = MAX_SQL_SIZE;
		
		va_list args;
		va_start(args, SQL_format);
		int n = vsnprintf(pQuery, QueryLen, SQL_format, args);
		va_end(args);
		if (n > QueryLen) return false;
		//LOG_PRINT(log_debug, "SQL:%s is ready!", pQuery);
		bool ret = get_DB_default(&tmp_Res, pQuery, row);
		
		if (res) 
			*res = tmp_Res;
		else 	
			CleanRes(&tmp_Res);
		
		return ret;
	
	}

	/*
	* @description  : 执行一个sql语句
	* @input param  : pszSql		-- sql语句
	* @output param : *res          -- 结果 不用可不填
	* @output param : *row          -- 行数 不用可不填
	*/
	inline bool run_sql(const char *pszSql, Result *res = NULL, int *row = NULL, bool bLog = true) 
	{
		if (NULL == pszSql || 0 == *pszSql)
		{
			return false;
		}

		Result tmp_Res;
		bool ret = (bLog ? get_DB_default(&tmp_Res, pszSql, row): 
						get_DB_default_nolog(&tmp_Res, pszSql, row));
		
		if (res) 
			*res = tmp_Res;
		else 	
			CleanRes(&tmp_Res);
		
		return ret;
	
	}
	
	/*
	* @description : 合成一个SQL字符串 并运行
	* @output param : *res          -- 结果 不用可填0
	* @output param : *row          -- 行数 不用可填0
	* @input param : *query         -- 存放SQL的空间 不用可填0
	* @input param : querylen       -- 存放SQL的空间长度 不用可填0
	* @input param : SQL_format     -- 格式串
	* @input param : ...            -- 格式串参数
	*/
	inline bool build_sql_run_nolog(Result *res, int *row, const char *SQL_format, ...) 
	{
		
		Result tmp_Res;
			
		char *pQuery;
		char tmp_query[MAX_SQL_SIZE];
		int QueryLen;
	
		pQuery = tmp_query;
		QueryLen = MAX_SQL_SIZE;
		
		va_list args;
		va_start(args, SQL_format);
		int n = vsnprintf(pQuery, QueryLen, SQL_format, args);
		va_end(args);
		if (n > QueryLen) return false;
		bool ret = get_DB_default_nolog(&tmp_Res, pQuery, row);
		
		if (res) 
			*res = tmp_Res;
		else 	
			CleanRes(&tmp_Res);
		
		return ret;
	
	}
	
	/*
	* @description : 获取结果
	* @output param : &dest         -- 结果值，只包括int等可用atoi处理的类型，其他类型均已特化
	* @input param : *result        -- 结果
	* @input param : row            -- 行
	* @input param : col            -- 列
	*/
	template<typename Type>
	inline void GetFromRes(Type &dest, Result *result, int row, int col) 
	{	
		if (!m_sos) return;
		const char *czvalue = m_sos->get(result, row, col);
		if (czvalue) dest = atoi(czvalue);	
	}

	/*
	* @description : 获取结果
	* @output param : *dest         -- 结果值，char *
	* @input param : *result        -- 结果
	* @input param : row            -- 行
	* @input param : col            -- 列
	* @input param : destlen        -- 目标字符串可承载的长度
	*/
	inline void GetFromRes(char *dest, Result *result, int row, int col, int destlen) 
	{
		if (destlen == 0) return;
		if (!m_sos) return;
		const char *czvalue = m_sos->get(result, row, col);
		if (destlen > 0)
			strncpy(dest, czvalue, destlen);
		else 
			memcpy(dest, czvalue, strlen(czvalue) );
	}

	/*
	* @description  : 获取结果
	* @input  param : *result        -- 结果
	* @input  param : row            -- 行
	* @input  param : col            -- 列
	* @return value : int			 -- 结果值
	*/
	inline int GetIntFromRes(Result *result, int row, int col)
	{
		if (!m_sos) return 0;
		const char *czvalue = m_sos->get(result, row, col);
		if (czvalue) return atoi(czvalue);	
		return 0;
	}
	

	/*
	* @description  : 获取结果
	* @input  param : *result        -- 结果
	* @input  param : row            -- 行
	* @input  param : col            -- 列
	* @return value : string		 -- 结果值
	*/
	inline string GetStringFromRes(Result *result, int row, int col)
	{
		if (!m_sos) return "";
		const char *czvalue = m_sos->get(result, row, col);
		if (czvalue) return czvalue;	
		return "";
	}
	
	/*
	* @description : 获取结果长度
	* @input param : *result        -- 结果
	* @input param : row            -- 行
	* @input param : col            -- 列
	* @return:  size_t             -- 长度
	*/
	inline size_t GetResLen(Result *result, int row, int col) 
	{	
		if (!m_sos) return 0;
		const char *czvalue = m_sos->get(result, row, col);
		if (czvalue) 
			return strlen(czvalue);
		else
			return 0;	
	}
	
	/*
	* @description : 清理结果
	* @input param : *result        -- 结果
	*/
	inline void CleanRes(Result *result) 
	{	
		if (!m_sos || !m_sos->clean) return;
		m_sos->clean(result);
	}

	int getAffectedRow()
	{
		if (sink)
		{
			return sink_get_affected_rows(sink);
		}
		else
		{
			return -1;
		}
	}

	unsigned long getInsertId()
	{
		if (sink)
		{
			return sink_get_insert_id(sink);
		}
		else
		{
			return 0;
		}
	}

	bool isValid()
	{
		if (sink)
			return sink_state(sink) == CONN_OK;

		return false;
	}
	
	SinkOperations *m_sos; 	
	static const char *SQL_s_zhuanbo_robotid_DB;
	static const char *SQL_s_vipprice_info_DB;
	static const char *SQL_u_useraccount_DB;
	static const char *SQL_s_useraccount_DB;
	static const char *SQL_s_privatevip_viplevel_DB;
	static const char *SQL_d_privatevip_viplevel_DB;
	static const char *SQL_i_privatevip_viplevel_DB[3];
	static const char *SQL_s_privatevip_info_DB;
	static const char *SQL_i_user_daycost_DB;
	static const char *SQL_u_user_daycost_DB;
	static const char *SQL_i_vcb_daycost_DB;
	static const char *SQL_u_vcb_daycost_DB;
	static const char *SQL_u_user_info_DB;
	static const char *SQL_u_user_info_without_introduce_DB;
	static const char *SQL_s_privatevip_userid_DB;
	static const char *SQL_s_ptcourseinfo_DB;
	static const char *SQL_s_team_wechat_qrcode;
	static const char *SQL_u_vcb_incr_attention;
	static const char *SQL_u_vcb_decr_attention;
	static const char *SQL_s_roomid_teacherid_DB;
	static const char *SQL_s_useralias_DB;
	static const char *SQL_s_viptotal_DB;
	static const char *SQL_s_roominfo_DB;
	static const char *SQL_s_fav_user_token_DB;
	static const char *SQL_s_userinfo_DB;
	static const char *SQL_s_user_calias_DB;
	static const char *SQL_s_user_info_DB;
	static const char *SQL_u_textlive_gift_DB;
	static const char *SQL_i_gifttradelog_DB;
	static const char *SQL_u_send_daygiftlog_DB;
	static const char *SQL_i_send_daygiftlog_DB;
	static const char *SQL_u_recv_daygiftlog_DB;
	static const char *SQL_i_recv_daygiftlog_DB;
	static const char *SQL_i_vcb_daycost_teacher_DB;
	static const char *SQL_u_vcb_daycost_teacher_DB;
	static const char *SQL_i_question_DB;
	static const char *SQL_s_question_id_DB;
	static const char *SQL_s_question_count_DB;
	static const char *SQL_u_question_count_DB;
	static const char *SQL_u_question_sumcount_DB;
	static const char *SQL_i_order_main_DB[3];
	static const char *SQL_i_order_details_DB[3];
	static const char *SQL_s_zhuanbo_config_DB;
	static const char *SQL_s_contribution_week_DB;
	static const char *SQL_s_team_teamid_DB;
	static const char *SQL_s_viproom_teacherid_DB;
	static const char *SQL_s_courseinfo_DB;
	static const char *SQL_s_buyptcourse_DB;
	static const char *SQL_i_ptcourse_order;
	static const char *SQL_u_ptcourse_studentnum_DB;
	static const char *SQL_s_ptcourse_studentnum_DB;
	static const char *SQL_s_log_ip_DB; 
	static const char *SQL_i_log_ip_DB;
	static const char *SQL_s_user_room_assistant; 
	static const char *SQL_u_user_room_assistant[2];
	static const char *SQL_i_user_room_assistant;
	static const char *SQL_s_vcbinformation_etr[2];
	static const char *SQL_u_question_surplus_DB;
	static const char *SQL_i_question_surplus_DB;
	static const char *SQL_s_teacherinfo_DB;
	static const char *SQL_s_teacherinfo_byID_DB;
	static const char *SQL_i_kickoutuser_DB;
	static const char *SQL_s_kickoutuser_DB;
	static const char *SQL_d_kickoutuser_DB;
	static const char *SQL_s_vipuserinfo_DB;
	static const char *SQL_s_get_one_vipuserinfo_DB;
	static const char *SQL_s_potential_VIP;
	static const char *SQL_u_potential_VIP;
	static const char *SQL_s_ptcourse_user_DB;
	static const char *SQL_s_ptcourse_user_buy_DB;

	static const char *SQL_s_course_by_roomid_DB;
	static const char *SQL_s_new_courseinfo_by_serverid_DB;
	static const char *SQL_s_load_course_DB;
	static const char *SQL_s_course_basicinfo_DB;
	static const char *SQL_s_get_course_noty_room_DB;

	static const char *SQL_s_close_ptcourse_id_DB;
	static const char *SQL_u_totalfavor_by_courseid_DB;
	static const char *SQL_d_uservcbfavor_by_courseid_DB;
	static const char *SQL_s_get_is_filter_user;
	static const char *SQL_s_viplist_teacherid_level_DB;

	static const char *SQL_s_exists_vcbmanager;
	static const char *SQL_i_addvcbmanager;
	static const char *SQL_d_delvcbmanager;

	static const char *SQL_s_gettaskcoin_info_DB;
	static const char *SQL_s_getuserdaycost_DB;
	static const char *SQL_u_useraccount_with_condition_DB;
	static const char *SQL_u_user_task_coin_DB;
protected:
	Sink* sink;
	FunHandler m_notify;
	string m_dbname;
	int m_platid;
	int timerid;
};

template<>	
inline void Dbconnection::GetFromRes<std::string>(std::string &dest, Result *result, int row, int col)
{	
	const char *czvalue = result_get(sink, result, row, col);
	dest.assign(czvalue);
}

template<>	
inline void Dbconnection::GetFromRes<int64>(int64 &dest, Result *result, int row, int col)
{	
	const char *czvalue = result_get(sink, result, row, col);
	if (czvalue) dest = atoll(czvalue);	
}

template<>	
inline void Dbconnection::GetFromRes<uint64>(uint64 &dest, Result *result, int row, int col)
{	
	const char *czvalue = result_get(sink, result, row, col);
	if (czvalue) dest = atoll(czvalue);	
}
template<>
inline void Dbconnection::GetFromRes<double>(double &dest, Result *result, int row, int col)
{
	const char *czvalue = result_get(sink, result, row, col);
	if (czvalue) dest = atof(czvalue);
}
class Dbconn
{
public:
	Dbconn();
	virtual ~Dbconn();

	static bool initDBSinkPool(int &platid, int poolsize = 5, string dbname = "maindb");

	void close(string dbname);
	void closeAll();
	static Dbconnection* getInstance(string dbname = "maindb");
	static Dbconnection* getSyncDB();
	static int getPlatIdAndIncrease();
	static Config 				m_config;
	static int switchindex;
private:
	static std::map<string,Dbconnection*> instance;
	static Dbconnection m_syncDB;
	static std::map<string, int> m_mapDBPlatid;
	static int m_nPlatId;
};
#endif //__DBCONNECTION_H__


