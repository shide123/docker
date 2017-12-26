
#include <stdio.h>
#include <string.h>
#include <db/optable2.h>
#include <db/sink.h>
#include <db/table.h>
#include <db/table2.h>
#include <db/timestamp.h>
#include <db/netutil.h>
#include "mysql.h"

#define SAFE_CLEAN_RES(x, y) \
if(y!=0)\
{\
result_clean(x, y);\
free(y);\
y=0;\
}

#define min(x,y)  (x)>(y) ? (x):(y)

#ifdef WIN32
#define atoll _atoi64
#endif

typedef struct _tag_Sparser
{
	char content[10][256];
	int np;
}Sparser;

Sparser StringParser(char * s)
{
	unsigned int _Tcount1,_Tcount3;
	int isQm;				
	Sparser sp;
	char * _Ts;
	int i, len;

	if(s[strlen(s) - 1] == ')')
		s[strlen(s) - 1] = 0;
	if(s[0] == '(')
		s = s + 1;

	_Ts = s;
	sp.np = 0;
	isQm  = 0;
	_Tcount1  = 0;
	_Tcount3  = 0;
	len = strlen(s);
	for(i = 0; i < len; i++)
	{
		switch(s[i])
		{
		case ',':
			if(isQm)
				_Tcount1 ++;
			else
			{
				memcpy(sp.content[_Tcount3], _Ts, _Tcount1);
				sp.content[_Tcount3][_Tcount1] = 0;
				_Ts = _Ts + _Tcount1 + 1;
				_Tcount1 = 0;
				_Tcount3 ++;
				sp.np ++;
			}
			break;
		case '\'':
			isQm = isQm ? 0 : 1;
		default:
			_Tcount1 ++;
			break;
		}
	}

	memcpy(sp.content[_Tcount3], _Ts, _Tcount1);
	sp.content[_Tcount3][_Tcount1] = 0;
	sp.np ++;

	return sp;
}

//////////////////////////////////////////////////////////////////////////


int exec_query(Sink *sink, const char * query)
{
	Result res;
	int     state;
	res.priv = 0;
	sink_exec2( sink, query, &res);
	state = result_state( sink, &res);

	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)  &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	result_clean(sink, &res);
	return 0;
}

int get_recordset_number(Sink *sink, const char * query)
{
	Result res;
	int     state;
	int		row = -1;
	res.priv = 0;
	sink_exec2( sink, query, &res);
	state = result_state( sink, &res);

	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)  &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	row = result_rn(sink, &res);

	result_clean(sink, &res);
	return row;
}

int get_affected_rows(Sink * sink)
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

//////////////////////////////////////////////////////////////////////////

int get_userid_records(Sink *sink, unsigned int userid)
{
	char sql[256];
	sprintf(sql,"select nuserid from dks_useraccount where nuserid=%u",userid);
	return get_recordset_number(sink,sql);
}

int get_client_version(Sink *sink, char* szVersion)
{
	Result res;
	char szquery[256];
	int state;
	int row;
	const char* szVal;

	sprintf(szquery, "select name from tb_version");

	res.priv = 0;
	sink_exec2( sink, szquery, &res);
	state = result_state( sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}
	row = result_rn(sink, &res);
	if (row != 1) {
		result_clean (sink, &res);
		return -1;
	}

	szVal = result_get(sink, &res, 0, 0);
	strcpy(szVersion, szVal);

	result_clean (sink, &res);
	return 0;

}

int get_user_langid_DB(Sink* sink, DDUserLangID_t *ptul, int userid, int langid)
{
	Result res;
	char query[256];
	int state, row;
	const char* str;

	if(userid == 0 && langid == 0)
		return -1;

	if(userid != 0)
		sprintf(query, "select nid,nuserid,nlangid,davadate,dexpdate from tbuserlangid  where nuserid=%d", userid);
	else
		sprintf(query, "select nid,nuserid,nlangid,davadate,dexpdate from tbuserlangid  where nlangid=%d", langid);

	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state( sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}
	row = result_rn(sink, &res);
	if (row != 1) {
		result_clean (sink, &res);
		return -1;
	}

	str = result_get(sink, &res, 0, 0);
	ptul->nid = atoi(str);
	str = result_get(sink, &res, 0, 1);
	ptul->nuserid = atoi(str);
	str = result_get(sink, &res, 0, 2);
	ptul->nlangid = atoi(str);
	str = result_get(sink, &res, 0, 3);
	strcpy(ptul->davadate, str);
	str = result_get(sink, &res, 0, 4);
	strcpy(ptul->dexpdate, str);

	result_clean (sink, &res);
	return 0;

}

int get_RoomByVcbid_DB(Sink* sink, DDRoomBaseInfo_t* ptf, int vcbid)
{
	Result res;
	char query[1024];
	int state, row;
	const char* str;

	if(vcbid ==0) return -1;
	sprintf(query, "select a.nvcbid,a.nxingji,a.ngroupid,a.nattrid,a.nseats,a.nvisible,a.ncreateid,a.nop1id,a.nop2id,a.nop3id,a.nop4id,a.cname, \
				   a.cpassword,a.croompic from dks_vcbinformation as a where a.nvcbid=%d ", vcbid);
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}
	row = result_rn(sink, &res);
	if (row != 1) {
		result_clean (sink, &res);
		return -1;
	}

	str = result_get(sink, &res, 0, 0);
	ptf->roomid = atoi(str);
	str = result_get(sink, &res, 0, 1);
	ptf->xingjiid = atoi(str);
	str = result_get(sink, &res, 0, 2);
	ptf->groupid = atoi(str);
	str = result_get(sink, &res, 0, 3);
	ptf->attrid = atoi(str);
	str = result_get(sink, &res, 0, 4);
	ptf->seats = atoi(str);
	str = result_get(sink, &res, 0, 5);
	ptf->visible = atoi(str);
	str = result_get(sink, &res, 0, 6);
	ptf->creatorid = atoi(str);
	str = result_get(sink, &res, 0, 7);
	ptf->op1id = atoi(str);
	str = result_get(sink, &res, 0, 8);
	ptf->op2id = atoi(str);
	str = result_get(sink, &res, 0, 9);
	ptf->op3id = atoi(str);
	str = result_get(sink, &res, 0, 10);
	ptf->op4id = atoi(str); 
	str = result_get(sink, &res, 0, 11);
	strcpy(ptf->cname, str);
	str = result_get(sink, &res, 0, 12);
	strcpy(ptf->cpassword, str);
	str = result_get(sink, &res, 0, 13);
	strcpy(ptf->croompic, str);

	result_clean(sink, &res);
	return 0;
}


int get_user_platform_info_DB(Sink* sink, int userid, char* nopenid, char* cOpentoken, int platformType)
{
	Result res;
	char query[256];
	int state, row;

	if(userid  == 0) return -1;
//	sprintf(query, "select userid from dks_third_login where userid=%d AND openid='%s' AND token='%s' AND type=%d", userid, nopenid, cOpentoken, platformType);
	sprintf(query, "select userid from dks_third_login where userid=%d ", userid);
	IF_TRUE_APPEND_SQL(nopenid, query, "openid='%s'", nopenid);
	IF_TRUE_APPEND_SQL(cOpentoken, query, "token='%s'", cOpentoken);
	IF_TRUE_APPEND_SQL(-1 != platformType, query, "type=%d", platformType);

	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}
	row = result_rn(sink, &res);
	if(row != 1) {
		result_clean(sink, &res);
		return -1;
	}

	result_clean(sink, &res);
	return 0;
}

int get_user_pwdinfo_DB(Sink* sink, DDUserPwdInfo_t* ppf, int userid)
{
	Result res;
	char query[256];
	int state, row;
	const char* str;

	if(userid  == 0) return -1;
	sprintf(query, "select nuserid,cpassword,crepasswd,cbankpwd from dks_user where nuserid=%d", userid);
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}
	row = result_rn(sink, &res);
	if(row != 1) {
		result_clean(sink, &res);
		return -1;
	}

	str = result_get(sink,&res, 0,0);
	ppf->userid = atoi(str);
	str = result_get(sink,&res,0,1);
	strcpy(ppf->cpassword, str);
	str = result_get(sink,&res,0,2);
	strcpy(ppf->crepasswd,str);
	str = result_get(sink,&res,0,3);
	strcpy(ppf->cbankpwd,str);

	result_clean(sink, &res);
	return 0;
}

int update_user_money_DB(Sink *sink, int64 *ble, int64 *giftble, int userid, int64 changemoney, int type)
{
	Result res;
	char query[256];
	int state;
	int row;
	const char* str;
	query[0] = 0;

	if (type == 1)
		sprintf(query, "UPDATE dks_useraccount SET nk=nk+%lld WHERE nuserid=%d",changemoney, userid);
	else
		sprintf(query, "UPDATE dks_useraccount SET nk=nk-%lld WHERE nuserid=%d",changemoney, userid);

	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state (sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)  &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	result_clean(sink, &res);

	sprintf (query, "SELECT nk,nb FROM dks_useraccount WHERE nuserid=%d",userid);

	sink_exec2(sink, query, &res);
	state = result_state (sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)  &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	row =result_rn (sink, &res);
	if (row != 1)
	{
		result_clean (sink, &res);
		return -1;
	}
	str = result_get (sink, &res, 0, 0);
	*ble = atoll(str);
	str = result_get (sink, &res, 0, 1);
	*giftble = atoll(str);

	result_clean (sink, &res);
	return 0;
}


int update_user_bmoney_DB(Sink *sink, int64 *ble, int64 *giftble, int userid, int64 changebmoney, int type)
{
	Result res;
	char query[256];
	int state;
	int row;
	const char* str;
	query[0] = 0;

	if (type == 1)
		sprintf(query, "UPDATE dks_useraccount SET nb=nb+%lld WHERE nuserid=%d",changebmoney, userid);
	else
		sprintf(query, "UPDATE dks_useraccount SET nb=nb-%lld WHERE nuserid=%d",changebmoney, userid);

	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state (sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)  &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	result_clean(sink, &res);

	sprintf (query, "SELECT nk,nb FROM dks_useraccount WHERE nuserid=%d",userid);

	sink_exec2(sink, query, &res);
	state = result_state (sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)  &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	row =result_rn (sink, &res);
	if (row != 1)
	{
		result_clean (sink, &res);
		return -1;
	}
	str = result_get (sink, &res, 0, 0);
	*ble = atoll(str);
	str = result_get (sink, &res, 0, 1);
	*giftble = atoll(str);

	result_clean (sink, &res);
	return 0;

}

int get_user_kmoney_DB(Sink *sink, int64 *ble, int64 *giftble, int64* depositble, int userid)
{
   Result res;
   char query[256]={0};
   int state,row;
   const char* str;

   if( ble !=0) *ble =0;
   if( giftble !=0) *giftble =0;
   if( depositble !=0) *depositble =0;

   sprintf(query, "select nk,nb,nkdeposit from dks_useraccount where nuserid=%d", userid);
   res.priv = 0;
   sink_exec2(sink, query, &res);
   state = result_state(sink, &res);
   if((state != RES_COMMAND_OK) &&
	   (state != RES_TUPLES_OK)  &&
	   (state != RES_COPY_IN)    &&
	   (state != RES_COPY_OUT))
   {
	   result_clean(sink, &res);
	   return -1;
   }

   row = result_rn (sink, &res);
   if (row != 1)
   {
	   result_clean (sink, &res);
	   return -1;
   }

   str = result_get(sink, &res, 0, 0);
   if(ble !=0) (*ble) = atoll(str);
   str = result_get(sink, &res, 0, 1);
   if(giftble !=0) (*giftble) = atoll(str);
   str = result_get(sink, &res, 0, 2);
   if(depositble !=0) (*depositble) = atoll(str);

   result_clean (sink, &res);
   return 0;
}

int update_user_seal_DB(Sink* sink, int userid, int sealid, int sealtime)
{
	Result res;
	char query[256];
	char szTimeNow[32];
	int state;
	query[0] = 0;

	toStringTimestamp(time(0) + sealtime*60, szTimeNow);
	sprintf(query, "update dks_user set nsealid=%d, dsealexptime='%s' where nuserid=%d", sealid, szTimeNow,userid);
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state (sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)  &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	result_clean(sink, &res);
	return 0;
}

#if 0
int get_userfreeze_DB(Sink* sink, DDUserFreezeInfo_t* puf, int userid)
{
	Result res;
	char query[256];
	int state, row;
	const char* str;

	if(userid ==0) return -1;
	sprintf(query, "select dfreezedate, dfreezedate2 from tbfreezeuser where nuserid=%d limit 1", userid);
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}
	row = result_rn(sink, &res);
	if (row != 1) {
		result_clean (sink, &res);
		return -1;
	}

	str=result_get(sink, &res, 0, 0);
    strcpy(puf->dfreezedate, str);
	str=result_get(sink, &res, 0, 1);
	strcpy(puf->dfreezedate2, str);

	puf->nuserid = userid;

	result_clean(sink, &res);
	return 0;
}
#endif

int isin_blackIpMac_DB(Sink* sink, char*pQuery, int qlen, int userid, int vcbid, int scopetype, char* szip, char* szmac, char* szdiannao, DDViolationInfo_t* pInfo)
{
	Result res;
	char query[512];
	char filter[256]={0};
	char sztemp[128]={0};
	int state,row;
	//const char* str;
	int bflag=0;

	time_t tendtime, tnow;
	int durtype, nfreezeaccount;
	const char* czvalue;

	if(userid==0 && szip==0 && szmac==0 && szdiannao == 0)
		return -1;

    if(userid != 0)
	{
		if(bflag)
			sprintf(sztemp, " nuserid=%d ", userid);
		else
			sprintf(sztemp, " where (nuserid=%d ", userid);
		strcat(filter, sztemp);
		bflag=1;
	}

    if(szmac && *szmac != '\0' && strlen(szmac) != 0)
	{
		if(bflag)
			sprintf(sztemp, " or cmac='%s' ", szmac);
		else
			sprintf(sztemp, " where (cmac='%s' ", szmac);
		strcat(filter, sztemp);
		bflag=1;
	}
    if(szdiannao && *szdiannao != '\0' && strlen(szdiannao) != 0)
	{
		if(bflag)
			sprintf(sztemp, " or chdsn='%s' ", szdiannao);
		else
			sprintf(sztemp, " where (chdsn='%s' ", szdiannao);
		strcat(filter, sztemp);
		bflag=1;
	}

	strcat(filter, ")");

	sprintf(sztemp, " and ((nscope=1 and nvcbid=%d) or nscope!=1)", vcbid);
	strcat(filter, sztemp);
	
	sprintf(query, "select nreason,nduration,nscope,tendtime,nfreezeaccount from dks_blackuser %s limit 1", filter);
	if (pQuery != NULL){
		memset(pQuery, 0, qlen);
		strncpy(pQuery, query, min(strlen(query), qlen-1));
	}
	res.priv=0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}
	row = result_rn(sink, &res);
	if (row == 0) {
		result_clean (sink, &res);
		return -2;
	}

	tnow = time(0);
	czvalue = result_get (sink, &res, 0, 0);
	pInfo->reasontype =atoi(czvalue);
	czvalue = result_get (sink, &res, 0, 1);
	durtype = atoi(czvalue);
	czvalue = result_get (sink, &res, 0, 2);
	pInfo->scopetype = atoi(czvalue);
	czvalue = result_get (sink, &res, 0, 3);
	tendtime = toNumericTimestamp(czvalue);
	czvalue = result_get (sink, &res, 0, 4);
	nfreezeaccount = atoi(czvalue);	

	pInfo->lefttime =tendtime - tnow;
	
	result_clean(sink, &res);
	return 0;
}


int get_user_templevel(Sink *sink, int nuserid, time_t tnow)
{
	char szTime[30];
	Result res;
	int row, state, nret;
	char query[256];

	toStringDate(tnow, szTime);
	sprintf(query, "select nviplevel from tb_tempuserlevel where nuserid=%d and denddate>='%s' order by nmajorid desc limit 1", nuserid, szTime);

	row=0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}

	row = result_rn(sink, &res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	nret = atoi(result_get(sink, &res, 0, 0));

	result_clean(sink, &res);
	return nret;
}

int get_usercurcarid(Sink *sink, DDUserCarInfo_t* pusercar, int nuserid)
{
	Result res;
	int row,state;
	char query[512];
	const char* szval;

	row=0;
	//reset car id
	sprintf(query,"update ws_car_buyuserlist set cartype='3',stars='0' where userid='%d' and endtime<='now()'", nuserid);
	res.priv=0;

	sink_exec2( sink, query, &res);
	state = result_state( sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}
	result_clean (sink, &res);

	//get car id
	sprintf(query, "select a.goodsid,b.goodsname from ws_car_buyuserlist as a,ws_car_shop as b where a.goodsid=b.id and a.userid ='%d' and a.stars='1'", nuserid);
	res.priv = 0;

	sink_exec2( sink, query, &res);
	state = result_state( sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}
	row = result_rn(sink, &res);
	if(row >= 1)
	{
		pusercar->ncarid = atoi(result_get(sink, &res, 0, 0));
		szval = result_get(sink, &res, 0, 1);
		strcpy(pusercar->carname,szval);
	}

	result_clean (sink, &res);
	return 0;
}

int get_userchestnum(Sink *sink, int nuserid)
{
	Result res;
	int row,state;
	char query[512];
	const char* szval;
	int nchestnum=-1;

	row=0;
	sprintf(query, "select nchestnum from dks_useraccount where nuserid=%d", nuserid);
	res.priv=0;
	sink_exec2( sink, query, &res);
	state = result_state( sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}
	row = result_rn(sink, &res);
	if(row >=1)
	{
		szval = result_get(sink, &res, 0, 0);
		nchestnum = atoi(szval);
	}

	result_clean (sink, &res);
	return nchestnum;
}

int update_userchestnum(Sink *sink, int nuserid, int change_chestnum)
{
	char query[512];
	sprintf(query, "update dks_useraccount set nchestnum=nchestnum+%d where nuserid=%d",change_chestnum,nuserid);
	exec_query(sink, query);
	return get_userchestnum(sink, nuserid);
}

int get_lastweekstarview_DB(Sink *sink, int* stararray, int stararraysize, int nuserid)
{
	Result res;
	int row,state,r;
	char query[256];
	const char* szval;

	if(stararray ==0 || stararraysize == 0)
		return -1;

	row=0;
	sprintf(query, "select ngiftid from tb_tempstarweekinfo where nuserid=%d order by nid limit %d", nuserid, stararraysize);

	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}

	row = result_rn(sink, &res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	for(r=0;r<row;++r)
	{
		szval=result_get(sink, &res, r, 0);
		stararray[r]=atoi(szval);
	}

	result_clean(sink, &res);
	return 0;
}


int get_topweekstarview_DB(Sink *sink, int* stararray, int stararraysize, int nuserid)
{
	Result res;
	int row,state,r;
	char query[256];
	const char* szval;

	if(stararray ==0 || stararraysize == 0)
		return -1;

	row=0;

	sprintf(query, "select ngiftid from tb_tempstartop1info where nuserid=%d order by nid limit %d", nuserid, stararraysize);

	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}

	row = result_rn(sink, &res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	for(r=0;r<row;++r)
	{
		szval=result_get(sink, &res, r, 0);
		stararray[r]=atoi(szval);
	}

	result_clean(sink, &res);
	return 0;
}

int get_activitystarview_DB(Sink *sink, int* stararray, int stararraysize, int nuserid)
{
	Result res;
	int row, state, r;
	char query[256];
	const char* szval;
	char sztime[128]={0};
	time_t tnow=time(0);
	toStringDate(tnow, sztime);

	if(stararray ==0 || stararraysize == 0)
		return -1;

	row=0;
	sprintf(query, "select nactitemid from tb_tempactivitystarinfo where nuserid=%d and dexpireddate>='%s' order by nid limit %d", 
		nuserid, sztime, stararraysize);

	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}

	row = result_rn(sink, &res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	for(r=0; r<row; ++r)
	{
		szval=result_get(sink, &res, r, 0);
		stararray[r]=atoi(szval);
	}

	result_clean(sink, &res);
	return 0;

}

int get_roomgrouplist_DB(Sink *sink, SList** list, int glevel)
{
    Result res;
	SList *head =0;
	int row, state, r;
	char query[256];
	const char *czvalue, *str;

	sprintf(query, "select nid,nparentid,nshowflag,nsortid,cname,curl,nshownum,cfontcolor,nbold,ctreeimg from tbvcbgroupinfo where nglevel=%d and (ngrouptype=1 or ngrouptype=2) and nshowflag=1 order by nsortid",glevel);
    row = 0;
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}

	row = result_rn(sink, &res);
	if( row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	for(r =0; r<row; ++r)
	{
		DDRoomGroupItem_t* record = (DDRoomGroupItem_t *)malloc(sizeof(DDRoomGroupItem_t));
        memset(record, 0, sizeof(DDRoomGroupItem_t));

		czvalue = result_get(sink, &res, r, 0);
        record->groupid = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 1);
		record->parentid = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 2);
		record->bshowflag = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 3);
		record->sortid = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 4);
		strcpy(record->cname, czvalue);
		czvalue = result_get(sink, &res, r, 5);
		strcpy(record->curl, czvalue);
		czvalue = result_get(sink, &res, r, 6);
		record->bshownum = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 7);
		//color 16~{=xVF~}
		record->ncolor = (int)(strtol(czvalue,&str,16));
		czvalue = result_get(sink, &res, r, 8);
		record->bfontbold = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 9);
		strcpy(record->ciconname,czvalue);

		record->glevel = glevel;
		head = slist_append( head, record);
	}

	result_clean(sink, &res);
	*list = head;
	return row;
}

int get_vcbgroupusernumlist_DB(Sink* sink, SList** list)
{
	Result res;
	SList *head = 0;
	int row, state, r;
	char query[256];
	const char* czvalue;

	sprintf(query, "select a.ngroupid,sum(b.ncount) as ntotalcntt from tbvcbgroupmap as a,dks_vcbstatus as b where a.nvcbid=b.nvcbid group by a.ngroupid");
    row=0;
	res.priv=0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}

	row=result_rn(sink, &res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	for(r=0;r<row;++r)
	{
		DDRoomGroupUserNum_t* record = (DDRoomGroupUserNum_t *)malloc(sizeof(DDRoomGroupUserNum_t));
		czvalue = result_get(sink, &res, r, 0);
		record->grouid = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 1);
		record->usernum = atoi(czvalue);

		head = slist_append(head, record);
	}

	result_clean(sink, &res);
	*list = head;
	return row;
}

int get_vcbcommands(Sink *sink, SList** list)
{
	//~{Wn:sI>3}UbP)C|An~},~{04J1<d5DWn:s9XO5I>3}~}
	Result res;
	SList* head=0;
	int row,state,r;
	char szquery[256]={0};
	char szlasttime[32]={0};
	const char* czvalue;

	sprintf(szquery,"select nvcbid,ncmdtype,ccmdtext,dcreatetime from tb_tempvcbcommand where (ncmdtype=1 or ncmdtype=2) order by dcreatetime");
	row=0;
	res.priv=0;
	sink_exec2(sink, szquery, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}
	row=result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	for(r=0;r<row;++r)
	{
		DDvcbcommand_t* record = (DDvcbcommand_t *)malloc(sizeof(DDvcbcommand_t));
		memset(record, 0, sizeof(DDvcbcommand_t));

		czvalue = result_get(sink, &res, r, 0);
		record->nvcbid = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 1);
		record->cmdtype = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 2);
		strcpy(record->cmdtext,czvalue);
		czvalue = result_get(sink, &res, r, 3);
		strncpy(record->dtime,czvalue,19);
		record->dtime[19]='\0';
		strcpy(szlasttime,czvalue);
		head = slist_append(head, record);
	}
	result_clean(sink, &res);
	//delete old records
    sprintf(szquery,"delete from tb_tempvcbcommand where (ncmdtype=1 or ncmdtype=2) and dcreatetime<='%s'",szlasttime);
    exec_query(sink,szquery);

	*list = head;
	return row;
}

//livetype:1-视频房间;2-文字房间
int get_vcbfullinfolist_DB(Sink* sink, SList** list,int livetype)
{
	Result res;
	SList* head=0;
	int row,state,r;
	char query[1024];

	const char* czvalue;

	if(livetype == 1)
	{
		sprintf(query, "select a.nvcbid,a.nxingji,a.ngroupid,a.nattrid, \
		a.nseats,a.nvisible,a.ncreateid,a.nop1id,a.nop2id,a.nop3id,a.nop4id,a.cname,a.cpassword, \
				a.croompic,a.popularity,b.nmcuid,b.cmedia,0,IF(c.viptype is null,0,c.viptype) as viplevel,a.type,UNIX_TIMESTAMP(b.dactivetime),b.optstat \
		from dks_vcbinformation as a left join dks_vcbstatus as b ON a.nvcbid=b.nvcbid left JOIN dks_vip_price_info c on a.jurisdiction=c.id \
		where  b.nstatus=1 and a.livetype=%d", livetype);
	}
	else if(livetype == 2)
	{
		sprintf(query, "select a.nvcbid,a.nxingji,a.ngroupid,a.nattrid, \
			  a.nseats,a.nvisible,a.ncreateid,a.nop1id,a.nop2id,a.nop3id,a.nop4id,a.cname,a.cpassword, \
			  a.croompic,a.popularity,b.nmcuid,b.cmedia,c.teacherid,a.jurisdiction,a.type,UNIX_TIMESTAMP(b.dactivetime),b.optstat \
		      from dks_vcbinformation as a,dks_vcbstatus as b,dks_teacherinfo as c\
			  where a.nvcbid=b.nvcbid and a.nvcbid=c.roomid and b.nstatus=1 and c.stype!=2 and a.livetype=%d", livetype);
	}

	row = 0;
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}

	row = result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	for(r = 0; r < row; ++r)  //row
	{
		DDRoomFullInfo_t * record = (DDRoomFullInfo_t *)malloc(sizeof(DDRoomFullInfo_t));
		memset(record, 0, sizeof(DDRoomFullInfo_t));

		czvalue = result_get(sink, &res, r, 0);
		record->roomid = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 1);
		record->xingjiid = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 2);
		record->groupid = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 3);
		record->attrid = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 4);
		record->seats = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 5);
		record->visible = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 6);
		record->creatorid = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 7);
		record->op1id = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 8);
		record->op2id = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 9);
		record->op3id = atoi(czvalue);
		czvalue = result_get(sink, &res, r ,10);
		record->op4id = atoi(czvalue);
		czvalue = result_get(sink, &res, r ,11);
		strcpy(record->cname, czvalue);
		czvalue = result_get(sink, &res, r ,12);
		strcpy(record->cpassword, czvalue);
		czvalue = result_get(sink, &res, r ,13);
		strcpy(record->croompic, czvalue);
		czvalue = result_get(sink, &res, r ,14);
		record->npopularity_ = atoll(czvalue);
		czvalue = result_get(sink, &res, r ,15);
		record->mcuid = atoi(czvalue);
		czvalue = result_get(sink, &res, r ,16);
		strcpy(record->cmedia, czvalue);
		czvalue = result_get(sink, &res, r ,17);
		record->ntextteacherid_= atoi(czvalue);
		czvalue = result_get(sink, &res, r ,18);
		record->viplevel = atoi(czvalue);
		czvalue = result_get(sink, &res, r ,19);
		record->roomtype = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 20);
		record->activetime = atoll(czvalue);
		czvalue = result_get(sink, &res, r, 21);
		record->optstat = atoi(czvalue);
		
		head = slist_append(head, record);
	}

	result_clean(sink, &res);
	*list = head;
	return row;
}

int get_one_vcbfullinfo_DB(Sink* sink, SList** list, int roomid, int livetype)
{
	Result res;
	SList* head=0;
	int row,state,r;
	char query[1024];

	const char* czvalue;

	if(livetype==1)
	{
			sprintf(query, "select a.nvcbid,a.nxingji,a.ngroupid,a.nattrid, \
				  a.nseats,a.nvisible,a.ncreateid,a.nop1id,a.nop2id,a.nop3id,a.nop4id,a.cname,a.cpassword, \
				  a.croompic,a.popularity,b.nmcuid,b.cmedia,0,UNIX_TIMESTAMP(b.dactivetime) \
			      from dks_vcbinformation as a,dks_vcbstatus as b\
				  where a.nvcbid=b.nvcbid and a.nvcbid=%u and a.livetype=%d", roomid, livetype);
	}
	else if(livetype==2)
	{
			sprintf(query, "select a.nvcbid,a.nxingji,a.ngroupid,a.nattrid, \
				  a.nseats,a.nvisible,a.ncreateid,a.nop1id,a.nop2id,a.nop3id,a.nop4id,a.cname,a.cpassword, \
				  a.croompic,a.popularity,b.nmcuid,b.cmedia,c.teacherid,UNIX_TIMESTAMP(b.dactivetime) \
			      from dks_vcbinformation as a,dks_vcbstatus as b,dks_teacherinfo as c\
				  where a.nvcbid=b.nvcbid and a.nvcbid=c.roomid and c.stype!=2 and a.nvcbid=%u and a.livetype=%d",roomid, livetype);
	}

	row= 0;
	res.priv= 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}

	row=result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	r = 0;
	DDRoomFullInfo_t* record = (DDRoomFullInfo_t *)malloc(sizeof(DDRoomFullInfo_t));
	memset(record, 0, sizeof(DDRoomFullInfo_t));

	czvalue = result_get(sink, &res, r, 0);
	record->roomid = atoi(czvalue);
	czvalue = result_get(sink, &res, r, 1);
	record->xingjiid = atoi(czvalue);
	czvalue = result_get(sink, &res, r, 2);
	record->groupid = atoi(czvalue);
	czvalue = result_get(sink, &res, r, 3);
	record->attrid = atoi(czvalue);
	czvalue = result_get(sink, &res, r, 4);
	record->seats = atoi(czvalue);
	czvalue = result_get(sink, &res, r, 5);
	record->visible = atoi(czvalue);
	czvalue = result_get(sink, &res, r, 6);
	record->creatorid = atoi(czvalue);
	czvalue = result_get(sink, &res, r, 7);
	record->op1id = atoi(czvalue);
	czvalue = result_get(sink, &res, r, 8);
	record->op2id = atoi(czvalue);
	czvalue = result_get(sink, &res, r, 9);
	record->op3id = atoi(czvalue);
	czvalue = result_get(sink, &res, r ,10);
	record->op4id = atoi(czvalue);
	czvalue = result_get(sink, &res, r ,11);
	strcpy(record->cname, czvalue);
	czvalue = result_get(sink, &res, r ,12);
	strcpy(record->cpassword, czvalue);
	czvalue = result_get(sink, &res, r ,13);
	strcpy(record->croompic, czvalue);
	czvalue = result_get(sink, &res, r ,14);
	record->npopularity_ = atoll(czvalue);
	czvalue = result_get(sink, &res, r ,15);
	record->mcuid = atoi(czvalue);
	czvalue = result_get(sink, &res, r ,16);
	strcpy(record->cmedia, czvalue);
	czvalue = result_get(sink, &res, r ,17);
	record->ntextteacherid_= atoi(czvalue);
	czvalue = result_get(sink, &res, r ,18);
	record->activetime= atoll(czvalue);

	head = slist_append(head, record);

	result_clean(sink, &res);
	*list = head;
	return row;
}
int get_vcballroominfolist_DB(Sink* sink, SList** list,int livetype)
{
	Result res;
	SList* head=0;
	int row,state,r;
	char query[1024];

	const char* czvalue;

	if(livetype==1)
	{
			sprintf(query, "select a.nvcbid,a.nxingji,a.ngroupid,a.nattrid, \
				  a.nseats,a.nvisible,a.ncreateid,a.nop1id,a.nop2id,a.nop3id,a.nop4id,a.cname,a.cpassword, \
				  a.croompic,a.popularity,b.nmcuid,b.cmedia,0,UNIX_TIMESTAMP(b.dactivetime),b.optstat \
			      from dks_vcbinformation as a,dks_vcbstatus as b\
				  where a.nvcbid=b.nvcbid  and a.livetype=%d",livetype);
	}
	else if(livetype==2)
	{
			sprintf(query, "select a.nvcbid,a.nxingji,a.ngroupid,a.nattrid, \
				  a.nseats,a.nvisible,a.ncreateid,a.nop1id,a.nop2id,a.nop3id,a.nop4id,a.cname,a.cpassword, \
				  a.croompic,a.popularity,b.nmcuid,b.cmedia,c.teacherid,UNIX_TIMESTAMP(b.dactivetime),b.optstat \
			      from dks_vcbinformation as a,dks_vcbstatus as b,dks_teacherinfo as c\
				  where a.nvcbid=b.nvcbid and a.nvcbid=c.roomid  and c.stype!=2 and a.livetype=%d",livetype);
	}

	row= 0;
	res.priv= 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}

	row=result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	for(r=0;r<row;++r)  //row
	{
		DDRoomFullInfo_t* record = (DDRoomFullInfo_t *)malloc(sizeof(DDRoomFullInfo_t));
		memset(record, 0, sizeof(DDRoomFullInfo_t));

		czvalue = result_get(sink, &res, r, 0);
		record->roomid = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 1);
		record->xingjiid = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 2);
		record->groupid = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 3);
		record->attrid = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 4);
		record->seats = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 5);
		record->visible = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 6);
		record->creatorid = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 7);
		record->op1id = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 8);
		record->op2id = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 9);
		record->op3id = atoi(czvalue);
		czvalue = result_get(sink, &res, r ,10);
		record->op4id = atoi(czvalue);
		czvalue = result_get(sink, &res, r ,11);
		strcpy(record->cname, czvalue);
		czvalue = result_get(sink, &res, r ,12);
		strcpy(record->cpassword, czvalue);
		czvalue = result_get(sink, &res, r ,13);
		strcpy(record->croompic, czvalue);
		czvalue = result_get(sink, &res, r ,14);
		record->npopularity_ = atoll(czvalue);
		czvalue = result_get(sink, &res, r ,15);
		record->mcuid = atoi(czvalue);
		czvalue = result_get(sink, &res, r ,16);
		strcpy(record->cmedia, czvalue);
		czvalue = result_get(sink, &res, r ,17);
		record->ntextteacherid_= atoi(czvalue);
		czvalue = result_get(sink, &res, r ,18);
		record->activetime= atoll(czvalue);
		czvalue = result_get(sink, &res, r ,19);
		record->optstat= atoi(czvalue);
		
		head = slist_append(head, record);
	}

	result_clean(sink, &res);
	*list = head;
	return row;
}

int update_room_popularity(Sink* sink, uint32_t vcbid, uint64_t accesstimes)
{
	char query[1024];

	sprintf(query, "update dks_vcbinformation set popularity=%llu where nvcbid=%u", accesstimes, vcbid);
	return exec_query(sink, query);
}

int update_room_activetime(Sink* sink, uint32_t roomid, uint64_t tActive)
{
	char query[256];
	sprintf(query, "update dks_vcbstatus set dactivetime=from_unixtime(%llu) where nvcbid=%u", tActive, roomid);
	return exec_query(sink, query);
}

//livetype:1-视频房间;2-文字房间
int get_delvcbIdlist_DB(Sink* sink, int* vcbIds, int* arrsize,int livetype)
{
	Result res;
	int row,state,r;
	char query[256];
	const char* czvalue;
	int maxnum= (*arrsize);
	(*arrsize)= 0;


	sprintf(query,"select nvcbid from dks_vcbstatus where nstatus=-1 and nvcbid in (select nvcbid from dks_vcbinformation where livetype=%d) limit %d offset 0", livetype,maxnum);
	row=0;
	res.priv=0;
	sink_exec2(sink,query,&res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}
	row=result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}
	for(r=0; r<row; ++r)
	{
		czvalue = result_get(sink, &res, r, 0);
		vcbIds[r] = atoi(czvalue);
	}
	result_clean(sink, &res);
	(*arrsize)= row;
	//
	return row;
}

int get_vcbmanagerlist_DB(Sink* sink, int* pmanagers, int* arrsize, int vcbid)
{
	Result res;
	int row, state, r;
	char query[256];
	const char* czvalue;
	int maxnum = (*arrsize);
	*arrsize = 0;

	sprintf(query, "select nuserid from tbvcbmanagermap where nvcbid=%d", vcbid);
	row = 0;
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}

	row=result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	row = (row >maxnum) ? maxnum : row;
	for(r=0; r<row; ++r)
	{
		czvalue = result_get(sink, &res, r, 0);
		pmanagers[r] = atoi(czvalue);
	}

	result_clean(sink, &res);
	(*arrsize) = row;
	return row;
}

//livetype:1-视频房间;2-文字房间
int get_vcbbaseinfolist_DB(Sink* sink, SList** list,int livetype)
{
    Result res;
	SList* head=0;
	int row,state,r;
	char query[512];
	const char* czvalue;

	sprintf(query, "select a.nvcbid,a.nxingji,a.ngroupid,a.nattrid,\
a.nseats,a.nvisible,a.ncreateid,a.nop1id,a.nop2id,a.nop3id,a.nop4id,a.cname,a.cpassword,a.croompic,a.type from dks_vcbinformation as a,dks_vcbstatus as b where \
a.nvcbid=b.nvcbid and b.nstatus=2 and a.nmodid1=1 and a.livetype=%d",livetype);

	row=0;
	res.priv=0;
	sink_exec2(sink, query, &res);
	state = result_state(sink,&res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}
	row=result_rn(sink,&res);
	if(row==0)
	{
		result_clean(sink, &res);
		return 0;
	}

	for(r=0;r<row;++r)
	{
		DDRoomBaseInfo_t* record = (DDRoomBaseInfo_t *)malloc(sizeof(DDRoomBaseInfo_t));
		memset(record, 0, sizeof(DDRoomBaseInfo_t));
        //
		czvalue = result_get(sink, &res, r, 0);
		record->roomid = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 1);
		record->xingjiid = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 2);
		record->groupid = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 3);
		record->attrid = atoi(czvalue);
		czvalue= result_get(sink, &res, r, 4);
		record->seats = atoi(czvalue);
		czvalue= result_get(sink, &res, r, 5);
		record->visible = atoi(czvalue);
		czvalue = result_get(sink,&res, r, 6);
		record->creatorid = atoi(czvalue);
		czvalue = result_get(sink, &res,r, 7);
		record->op1id = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 8);
		record->op2id = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 9);
		record->op3id = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 10);
		record->op4id = atoi(czvalue);
		czvalue = result_get(sink, &res, r, 11);
		strcpy(record->cname,czvalue);
		czvalue = result_get(sink, &res, r, 12);
		strcpy(record->cpassword, czvalue);
		czvalue = result_get(sink, &res, r, 13);
		strcpy(record->croompic, czvalue);
		czvalue = result_get(sink, &res, r, 14);
		record->roomtype = atoi(czvalue);

		head = slist_append(head, record);
	}
    result_clean(sink, &res);

	//sprintf(query, "update dks_vcbinformation set nmodid1=0");
	sprintf(query, "update dks_vcbinformation set nmodid1=0 where livetype=%d",livetype);
	exec_query(sink, query);
	*list = head;
	return row;
}

int get_vcbmedialist_DB(Sink* sink, SList** list)
{
   Result res;
   SList* head=0;
   int row,state,r;
   char query[512];
   const char* czvalue;

   sprintf(query, "select nvcbid,cmedia from dks_vcbstatus where nstatus=2 and nmodid1=1");
   row =0;
   res.priv = 0;
   sink_exec2(sink, query, &res);
   state = result_state(sink, &res);
   if((state != RES_COMMAND_OK) &&
	   (state != RES_TUPLES_OK)  &&
	   (state != RES_COPY_IN)    &&
	   (state != RES_COPY_OUT))
   {
	   result_clean(sink, &res);
	   return 0;
   }

   row=result_rn(sink,&res);
   if(row == 0)
   {
	   result_clean(sink, &res);
	   return 0;
   }
   for(r=0; r<row; ++r)
   {
	   DDRoomMediaInfo_t* record = (DDRoomMediaInfo_t *)malloc(sizeof(DDRoomMediaInfo_t));
	   memset(record, 0, sizeof(DDRoomMediaInfo_t));
       //
	   czvalue = result_get(sink, &res,r,0);
	   record->roomid = atoi(czvalue);
	   czvalue = result_get(sink, &res,r,1);
	   strcpy(record->cmedia, czvalue);
       //
	   head = slist_append(head, record);
   }
   result_clean(sink, &res);

   sprintf(query, "update dks_vcbstatus set nmodid1=0");
   exec_query(sink, query);
   *list = head;
   return row;  
}


int get_vcbmedialist_DB_filter_areaid(Sink* sink,SList** list)
{
	Result res;
	SList* head=0;
	int row,state,r;
	char query[512];
	const char* czvalue;

	sprintf(query, "select nvcbid,cmedia,careaid from dks_vcbareastatus where nmodid1='1'");
	row =0;
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}

	row=result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}
	for(r=0; r<row; ++r)
	{
		DDRoomMediaInfo_t* record = (DDRoomMediaInfo_t *)malloc(sizeof(DDRoomMediaInfo_t));
		memset(record, 0, sizeof(DDRoomMediaInfo_t));
		//
		czvalue = result_get(sink, &res,r,0);
		record->roomid = atoi(czvalue);
		czvalue = result_get(sink, &res,r,1);
		strcpy(record->cmedia, czvalue);
		czvalue = result_get(sink, &res,r,2);
		strcpy(record->areaid, czvalue);
		//
		head = slist_append(head, record);
	}
	result_clean(sink, &res);

	sprintf(query, "update dks_vcbareastatus set nmodid1=0");
	exec_query(sink, query);
	*list = head;
	return row; 
}

int get_vcbmedia_addr_byareaid(Sink* sink, int vcbid, const char* areadid, char * mediaip)
{
	Result res;
	int row, state;
	char query[256];

	sprintf(query, "select cmedia from dks_vcbareastatus where nvcbid=%d and careaid=\'%s\'", vcbid, areadid);
	row = 0;
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}

	row = result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	strcpy(mediaip, result_get(sink, &res,0,0));	
	result_clean(sink, &res);

	return row;
}


int get_vcbareaid_from_UserInfo(Sink* sink, int userid, char* areaid)
{
	Result res;
	int row, state;
	char query[256];
	const char* szVal;

	sprintf(query, "select ccity from dks_user where nuserid=%d", userid);	
	row = 0;
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}

	row = result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	szVal = result_get(sink, &res,0,0);

	strcpy(areaid, szVal);	
	result_clean(sink, &res);

	return row;
}

int get_AdKeywordList_DB(Sink* sink, SList** list, int startline, int linenum)
{
	Result res;
	SList* head = 0;
	int row, state, r;
	char query[256];

	sprintf(query, "select adtype, runerid, ctime, keyname from dks_keywords limit %d offset %d", linenum, startline);
	row = 0;
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}

	row = result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}
	for(r=0; r<row; r++)
	{
		DDAdKeywordInfo_t* record=(DDAdKeywordInfo_t*)malloc(sizeof(DDAdKeywordInfo_t));
		memset(record, 0, sizeof(DDAdKeywordInfo_t));
		record->ntype = atoi(result_get(sink, &res, r, 0));
		record->nrunerid = atoi(result_get(sink, &res, r, 1));
		strcpy(record->createtime, result_get(sink, &res, r, 2));
		strcpy(record->keyword, result_get(sink, &res, r, 3));
		head = slist_append(head, record);
	}

	result_clean(sink, &res);
    *list = head;

	return row;
}

int exist_AdKeyword_DB(Sink* sink, char	*pKeyword)
{
	Result res;
	int row, state;
	char query[256];

	if (pKeyword == NULL || *pKeyword == 0)
		return 0;

	sprintf(query, "select keyname from dks_keywords where keyname='%s'", pKeyword);
	row = 0;
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}

	row = result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	result_clean(sink, &res);

	return row;
}

int insert_AdKeyword_DB(Sink* sink, DDAdKeywordInfo_t *pKeyword)
{
	time_t tnow=time(0);
	char szTime[32]={0};
	char   query[256];

	toStringTimestamp(tnow, szTime);
    sprintf(query, "insert into dks_keywords(adtype,runerid,ctime,keyname) values(%d,%d,'%s','%s')",
		pKeyword->ntype, pKeyword->nrunerid, pKeyword->createtime, pKeyword->keyword);
	return exec_query(sink, query);
}

int del_AdKeyword_DB(Sink* sink, DDAdKeywordInfo_t *pKeyword)
{
	char query[256];
	sprintf(query, "delete from dks_keywords where keyname='%s'", pKeyword->keyword);
	return exec_query(sink, query);
}


int get_add_zhuanbo_robots_DB(Sink* sink, SList** list)
{
	Result res;
	SList* head=0;
	int row,state,r;
	char query[512];
	sprintf(query, "select nid,nvcbid,npubmicidx,nrobotid,nrobotviplevel,nrobotvcbid,nstatus,nuserid from dks_vcbzhuanboconfig where nstatus=1");
	row =0;
	res.priv =0;
	sink_exec2(sink, query, &res);
	state =result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}
	row=result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}
	for(r=0; r<row; r++)
	{
		DDzhaunborobotinfo_t* record=(DDzhaunborobotinfo_t*)malloc(sizeof(DDzhaunborobotinfo_t));
		memset(record, 0, sizeof(DDzhaunborobotinfo_t));
		record->nid =atoi(result_get(sink, &res, r, 0));
		record->nvcbid =atoi(result_get(sink, &res, r, 1));
		record->npubmicidx =atoi(result_get(sink, &res, r, 2));
		record->nrobotid =atoi(result_get(sink, &res, r, 3));
		record->nrobotviplevel =atoi(result_get(sink, &res, r, 4));
		record->nrobotvcbid=atoi(result_get(sink, &res, r, 5));
		record->nstatus=atoi(result_get(sink, &res, r, 6));
		record->nuserid=atoi(result_get(sink, &res, r, 7));
		head = slist_append(head, record);
	}
	result_clean(sink, &res);
	*list = head;
	return row;
}

int get_del_zhuanbo_robots_DB(Sink* sink, SList** list)
{
   Result res;
   SList* head=0;
   int row,state,r;
   char query[512];
   sprintf(query, "select nid,nvcbid,npubmicidx,nrobotid,nrobotviplevel,nrobotvcbid,nstatus from dks_vcbzhuanboconfig where nstatus=-1");
   row =0;
   res.priv =0;
   sink_exec2(sink, query, &res);
   state =result_state(sink, &res);
   if((state != RES_COMMAND_OK) &&
	   (state != RES_TUPLES_OK)  &&
	   (state != RES_COPY_IN)    &&
	   (state != RES_COPY_OUT))
   {
	   result_clean(sink, &res);
	   return 0;
   }
   row=result_rn(sink,&res);
   if(row == 0)
   {
	   result_clean(sink, &res);
	   return 0;
   }
   for(r=0; r<row; r++)
   {
	   DDzhaunborobotinfo_t* record=(DDzhaunborobotinfo_t*)malloc(sizeof(DDzhaunborobotinfo_t));
	   memset(record, 0, sizeof(DDzhaunborobotinfo_t));
	   record->nid =atoi(result_get(sink, &res, r, 0));
	   record->nvcbid =atoi(result_get(sink, &res, r, 1));
	   record->npubmicidx =atoi(result_get(sink, &res, r, 2));
	   record->nrobotid =atoi(result_get(sink, &res, r, 3));
	   record->nrobotviplevel =atoi(result_get(sink, &res, r, 4));
	   record->nrobotvcbid=atoi(result_get(sink, &res, r, 5));
	   record->nstatus=atoi(result_get(sink, &res, r, 6));
	   head = slist_append(head, record);
   }
   result_clean(sink, &res);
   *list = head;
   return row;
}

int get_vcbidlist_DB(Sink* sink, int* pvcbids, int* arrsize, char* szfilter)
{
	Result res;
	int row,state,r;
	char query[256];
	const char* czvalue;
	int maxnum = (*arrsize);
	*arrsize = 0;

	sprintf(query, "select a.nvcbid from dks_vcbinformation as a,dks_vcbstatus as b where %s", szfilter);
	row = 0;
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}

	row=result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	row = (row >maxnum) ? maxnum : row;
	for(r=0; r<row; ++r)
	{
		czvalue = result_get(sink,&res,r, 0);
        pvcbids[r] = atoi(czvalue);
	}
	result_clean(sink, &res);
	(*arrsize) = row;
	return row;
}

int get_flygiftlst_DB(Sink* sink, SList** list)
{
   Result res;
   SList* head=0;
   int row,state,r;
   char query[256];
   const char* czvalue;

   sprintf(query, "select nid,nvcbid,nsrcid,nbuddyid,ngiftid,ncount,dsenttime,ctext,nanonymous,nsrcplatid,ntoplatid,csrcalias,ctoalias from tbflygifttradelog");
   row=0;
   res.priv=0;
   sink_exec2(sink,query,&res);
   state = result_state(sink,&res);
   if((state != RES_COMMAND_OK) &&
	   (state != RES_TUPLES_OK)  &&
	   (state != RES_COPY_IN)    &&
	   (state != RES_COPY_OUT))
   {
	   result_clean(sink, &res);
	   return 0;
   }

   row=result_rn(sink,&res);
   if(row == 0)
   {
	   result_clean(sink, &res);
	   return 0;
   }

   for(r=0;r<row;++r)
   {
	   DDFlyGiftInfo_t* record = (DDFlyGiftInfo_t *)malloc(sizeof(DDFlyGiftInfo_t));
	   memset(record, 0, sizeof(DDFlyGiftInfo_t));

	   czvalue = result_get(sink, &res,r,0);
	   record->nid = atoi(czvalue);
	   czvalue = result_get(sink,&res,r,1);
	   record->nvcbid=atoi(czvalue);
	   czvalue = result_get(sink,&res,r,2);
	   record->nsrcid = atoi(czvalue);
	   czvalue = result_get(sink,&res,r,3);
	   record->nbuddyid=atoi(czvalue);
	   czvalue = result_get(sink,&res,r,4);
	   record->ngiftid=atoi(czvalue);
	   czvalue = result_get(sink,&res,r,5);
	   record->ncount = atoi(czvalue);
	   czvalue = result_get(sink,&res,r,6);
	   strcpy(record->dsenttime, czvalue);
	   czvalue = result_get(sink,&res,r,7);
	   strcpy(record->ctext,czvalue);
	   czvalue = result_get(sink,&res,r,8);
	   record->nanonymous=atoi(czvalue);
	   czvalue = result_get(sink,&res,r,9);
	   record->nsrcplatid=atoi(czvalue);
	   czvalue = result_get(sink,&res,r,10);
	   record->ntoplatid=atoi(czvalue);
	   czvalue = result_get(sink,&res,r,11);
	   strcpy(record->csrcalias,czvalue);
	   czvalue = result_get(sink,&res,r,12);
	   strcpy(record->ctoalias,czvalue);

       head = slist_append(head, record);
   }
   result_clean(sink, &res);
   *list = head;
   return row;  
}

int get_privilegelist_DB(Sink* sink, SList** list)
{
	Result res;
	SList* head=0;
	int row, state, r;
	char query[256];
	const char* czvalue;

	sprintf(query, "select a.qx_id,a.qx_type,b.srclevel,b.tolevel from tbquanxianinfo as a,tbquanxianaction as b where a.qx_id=b.qx_id ");
	row=0;
	res.priv=0;
	sink_exec2(sink, query, &res);
	state = result_state(sink,&res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}

	row=result_rn(sink, &res);
	if(row == 0) {
		result_clean(sink, &res);
		return 0;
	}

	for(r=0; r<row; ++r)
	{
		DDQuanxianAction_t* record = (DDQuanxianAction_t *)malloc(sizeof(DDQuanxianAction_t));
		memset(record, 0, sizeof(DDQuanxianAction_t));

		czvalue = result_get(sink,&res,r,0);
		record->qxid = atoi(czvalue);
		czvalue = result_get(sink,&res,r,1);
		record->qxtype=atoi(czvalue);
		czvalue = result_get(sink,&res,r,2);
		record->srclevel=atoi(czvalue);
		czvalue=result_get(sink,&res,r,3);
		record->tolevel=atoi(czvalue);

		 head = slist_append(head, record);
	}
	result_clean(sink, &res);
	*list = head;
	return row;  
}

int get_privilegeupdatestatus_DB(Sink* sink)
{
	Result res;
	int row,state;
	char query[256];

	sprintf(query, "select nquanxian from tbcastsvrconf where nquanxian=1");
	row=0;
	res.priv = 0;

	sink_exec2(sink, query,&res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}
	if(row == 0)
	{
		result_clean(sink, &res);
		return -1;
	}
	result_clean(sink, &res);

	sprintf(query,"update tbcastsvrconf set nquanxian=0");
	exec_query(sink, query);

	return 0;
}

int get_giftjackpot(Sink* sink,int jackpot_type, int *jiner)
{
	Result res;
	int    row,state;
	char   query[256];
	const char *szval;

	sprintf(query, "select njackpot from tb_giftjackpot where nid=%d", jackpot_type);
	row = 0;
	res.priv = 0;

	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}
	row = result_rn(sink, &res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return -1;
	}

	szval = result_get(sink, &res, 0, 0);
	*jiner = atoi(szval);

	result_clean(sink, &res);
	return 0;
}


int update_giftjackpot(Sink* sink, int giftid, int optype, int64 money, int64 *pcount)
{
	Result res;
	int row, state;
	char query[256];
	const char* str;

	if(money <=0)
		return -1;

	if(optype == 1)
		sprintf(query, "update tb_giftlotterypro set njiner=njiner+%lld where ngiftid=%d", money, giftid);
	else
		sprintf(query, "update tb_giftlotterypro set njiner=njiner-%lld where ngiftid=%d", money, giftid);

	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state (sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)  &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}
	result_clean(sink, &res);

	sprintf (query, "SELECT njiner FROM tb_giftlotterypro WHERE ngiftid=%d",giftid);
	sink_exec2(sink, query, &res);
	state = result_state (sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)  &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	row =result_rn (sink, &res);
	if (row != 1)
	{
		result_clean (sink, &res);
		return -1;
	}
	str = result_get (sink, &res, 0, 0);
	*pcount = atoll(str);

	result_clean (sink, &res);
	return 0;
}

int get_giftlotterypro(Sink* sink, DDGiftLotteryPro_t* tpro, int giftid)
{
	Result res;
	int    row,state;
	char   query[256];
	const char *szval;

	sprintf(query, "select nid,ngiftid,njiner,nwin1,nwin2,nwin3,nwin4,nwin5,nwin6,nwin7,nwin8,nwin9,nwin10 from tb_giftlotterypro where ngiftid=%d", giftid);
	row = 0;
	res.priv = 0;

	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}
	row = result_rn(sink, &res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return -1;
	}

	szval = result_get(sink, &res, 0, 0);
	tpro->nid = atoi(szval);
	szval = result_get(sink, &res, 0, 1);
	tpro->ngiftid = atoi(szval);
	szval = result_get(sink, &res, 0, 2);
	tpro->njiner = atoi(szval);
	szval = result_get(sink, &res, 0, 3);
	tpro->winpro[0] = atoi(szval);
	szval = result_get(sink, &res, 0, 4);
	tpro->winpro[1] = atoi(szval);
	szval = result_get(sink, &res, 0, 5);
	tpro->winpro[2] = atoi(szval);
	szval = result_get(sink, &res, 0, 6);
	tpro->winpro[3] = atoi(szval);
	szval = result_get(sink, &res, 0, 7);
	tpro->winpro[4] = atoi(szval);
	szval = result_get(sink, &res, 0, 8);
	tpro->winpro[5] = atoi(szval);
	szval = result_get(sink, &res, 0, 9);
	tpro->winpro[6] = atoi(szval);
	szval = result_get(sink, &res, 0, 10);
	tpro->winpro[7] = atoi(szval);
	szval = result_get(sink, &res, 0, 11);
	tpro->winpro[8] = atoi(szval);
	szval = result_get(sink, &res, 0, 12);
	tpro->winpro[9] = atoi(szval);

	result_clean(sink, &res);
	return 0;

}

int add_giftlotterywinlog(Sink* sink,int userid, int giftid, int64 giftprice, int wincount)
{
	time_t tnow=time(0);
	char szTime[32]={0};
	char   query[256];

	toStringTimestamp(tnow, szTime);
	sprintf(query, "insert into tb_giftlotterywinlog(nuserid,ngiftid,ngiftprice,nwincount,dtime) values(%d,%d,%lld,%d,'%s')",
		userid, giftid, giftprice, wincount, szTime);
	return exec_query(sink, query);
}

int update_lotterypool(Sink *sink, int64 in_money, int64* out_money)
{
	Result res;
	int row,state;
	char query[256];
	const char *szval;

	(*out_money)=0;
	sprintf(query,"update tbcastsvrconf set nlotterypool=nlotterypool+%lld", in_money);
	exec_query(sink, query);
	sprintf(query,"select nlotterypool from tbcastsvrconf");
	row=0;
	res.priv=0;
	sink_exec2(sink,query,&res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}
	row=result_rn(sink, &res);
	if(row ==0) {
		result_clean(sink,&res);
		return -1;
	}
	szval = result_get(sink, &res, 0, 0);
	(*out_money)=atoll(szval);

	result_clean(sink, &res);
	return 0;
}

/*
int get_xingyunrenpro(Sink* sink, DDXingyunrenPro_t* xpro, int userid, int giftid)
{
	time_t tnow=time(0);
	char szTime[32]={0};
	Result res;
	int row,state,nrecid,nchi,nshu;
	char query[256];
	const char *szval;

	xpro->nuserid = 0;
	xpro->nshu = 0;
	toStringTimestamp(tnow, szTime);
    sprintf(query, "select nid,nchi,nshu from tb_xingyunrenpro where nuserid=%d and njctype=%d and nchi>0 and dtime<='%s' order by nid limit 1",
		userid, jackpot_type, szTime);
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}
	row = result_rn(sink, &res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return -1;
	}
	
	szval = result_get(sink, &res, 0, 0);
	nrecid = atoi(szval);
	szval = result_get(sink, &res, 0, 1);
	nchi = atoi(szval);
	szval = result_get(sink, &res, 0, 2);
	nshu = atoi(szval);

	result_clean(sink, &res);

	sprintf(query, "update tb_xingyunrenpro set nchi=nchi-1 where nid=%d", nrecid);
	exec_query(sink, query);

	if(nchi == 1) {
		xpro->nuserid = userid;
		xpro->nshu = nshu;
	}

	return 0;
}
*/

int write_flygiftlog_DB(Sink* sink, DDFlyGiftInfo_t* tvf)
{
   Result res;
   int state;
   char query[512];
   
   sprintf(query, "delete from tbflygifttradelog where nid=%d", tvf->nid);
   res.priv = 0;
   exec_query(sink, query);

   sprintf(query,"insert into tbflygifttradelog(nid,nvcbid,nsrcid,nbuddyid,ngiftid,ncount,dsenttime,ctext,nanonymous,nsrcplatid,ntoplatid,csrcalias,ctoalias) values(%d,%d,%d,%d,%d,%d,'%s','%s',%d,%d,%d,'%s','%s')",
	   tvf->nid,
	   tvf->nvcbid,
	   tvf->nsrcid,
	   tvf->nbuddyid,
	   tvf->ngiftid,
	   tvf->ncount,
	   tvf->dsenttime,
	   tvf->ctext,
	   tvf->nanonymous,
	   tvf->nsrcplatid,
	   tvf->ntoplatid,
	   tvf->csrcalias,
	   tvf->ctoalias);

   sink_exec2(sink, query, &res);
   state = result_state(sink, &res);
   if((state != RES_COMMAND_OK) &&
	   (state != RES_TUPLES_OK)  &&
	   (state != RES_COPY_IN)    &&
	   (state != RES_COPY_OUT))
   {
	   result_clean(sink, &res);
	   return -1;
   }

   result_clean(sink, &res);
   return 0;
}

int get_isroomquzhang_DB(Sink* sink, int userid, int vcbid)
{
	Result res;
	int row, state;
	char query[256];
	sprintf(query, "select a.nid from tbvcbsupmanagermap as a,dks_vcbinformation as b where a.ngroupid=b.ngroupid and b.nvcbid=%d and a.nuserid=%d",vcbid,userid);
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	row = result_rn(sink, &res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return -1;
	}

	return 0;
}

int get_isroommanager_DB(Sink* sink, int userid, int vcbid)
{
	Result res;
	int row,state;
	char query[256];

	sprintf(query, "select nid from tbvcbmanagermap where nvcbid=%d and nuserid=%d",vcbid,userid);
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	row = result_rn(sink, &res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return -1;
	}
	return 0;
}

int get_isroomagent_DB(Sink* sink, int userid, int vcbid)
{
	Result res;
	int row,state;
	char query[256];
	sprintf(query,"select nid from ntb_agents where nuserid=%d and nvcbid=%d", userid, vcbid);
	res.priv=0;
	sink_exec2(sink,query,&res);
	state =result_state(sink,&res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}
	row= result_rn(sink, &res);
	if(row==0) {
		result_clean(sink, &res);
		return -1;
	}
	result_clean(sink, &res);
	return 0;
}

int add_roommanager_DB(Sink* sink, int userid, int vcbid,int mgrtype)
{
	Result res;
	int row, state;
	char query[256];
	sprintf(query, "select nid from tbvcbmanagermap where nvcbid=%d and nuserid=%d and nmanagertype=%d",vcbid,userid,mgrtype);
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	row = result_rn(sink, &res);
	if(row == 0)
	{
		result_clean(sink, &res);

		sprintf(query, "insert into tbvcbmanagermap(nvcbid,nuserid,nmanagertype) values(%d,%d,%d)",vcbid,userid,mgrtype);
		return exec_query(sink, query);
	}
	return 0;
}

int del_roommanager_DB(Sink* sink, int userid, int vcbid,int mgrtype)
{
	char query[256];
	sprintf(query, "delete from tbvcbmanagermap where nvcbid=%d and nuserid=%d and nmanagertype=%d",vcbid,userid,mgrtype);
	return exec_query(sink, query);
}

int write_userloginlog_DB(Sink* sink, DDUserLoginLog_t* loginlog)
{
	char szTime[32]={0};
	char query[256];
	time_t tnow=time(0);

	toStringTimestamp(tnow, szTime);
	sprintf(query, "insert into tbuserlogonlog(nuserid,ntype,clogonip,cmacaddr,cserialaddr,dtime,devicemodel,deviceos) values(%d,%d,'%s','%s','%s','%s','%s','%s')",
		loginlog->userid,
		loginlog->devtype,
		loginlog->szip,
		loginlog->szmac,
		loginlog->szserial,
		szTime,
		loginlog->devicemodel,
		loginlog->deviceos
		);
	exec_query(sink, query);

	sprintf(query, "update dks_user set dlast_login_time='%s',clast_login_ip='%s',clast_login_mac='%s' where nuserid=%d",
		szTime, 
		loginlog->szip, 
		loginlog->szmac, 
		loginlog->userid);
	exec_query(sink, query);

	return 0;
}

int write_roombaseinfo_DB(Sink* sink, int vcbid, DDRoomBaseInfo_t* pInfo)
{
	char query[1024];
	sprintf(query, "update dks_vcbinformation set ncreateid=%d,nop1id=%d,nop2id=%d,nop3id=%d,nop4id=%d,nusepwd=%d,cname='%s',cpassword='%s' where nvcbid=%d",
		pInfo->creatorid, pInfo->op1id, pInfo->op2id, pInfo->op3id, pInfo->op4id, pInfo->busepwd,pInfo->cname,pInfo->cpassword,pInfo->roomid);
	exec_query(sink, query);
	return 0;
}

//写入房间管理状态信息
int write_roomoptstat_DB(Sink* sink, int vcbid, int nopstat)
{
	char query[1024];
	sprintf(query, "update dks_vcbstatus set optstat = %d where nvcbid = %d", nopstat, vcbid);
	exec_query(sink, query);
	return 0;
}

int is_collectroom_DB(Sink* sink, int userid, int vcbid)
{
	Result res;
	int row, state;
    char query[256];
	sprintf(query, "select nid from dks_uservcbfavorite where nuserid=%d and nvcbid=%d",userid,vcbid);
	res.priv=0;
	sink_exec2(sink,query,&res);
	state =result_state(sink,&res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}
	row= result_rn(sink, &res);
	if(row==0) {
		result_clean(sink, &res);
		return -1;
	}
	result_clean(sink, &res);
	return 0;
}

int write_collectroom_DB(Sink* sink, int userid, int vcbid, int action)
{
	int nRet = -1;
	int nIsCollected = 0;
	if (0 == is_collectroom_DB(sink, userid, vcbid))
	{
		nIsCollected = 1;
	}

	if (1 == action)
	{
		if (0 == nIsCollected) 
		{
			char szTime[32]={0};
			toStringTimestamp(time(0), szTime);
			char query[256];
			sprintf(query, "insert into dks_uservcbfavorite(nuserid,nvcbid,dtime) values(%d,%d,'%s')",
				userid, vcbid, szTime);
			if (0 == exec_query(sink, query))
			{
				sprintf(query, " update dks_vcbinformation a set total_attention=(select count(nuserid) from dks_uservcbfavorite where nvcbid=a.nvcbid) where nvcbid=%u ", vcbid);
				nRet = exec_query(sink, query);
			}
		}
	}
	else 
	{
		if (1 == nIsCollected)
		{
			char query[256];
			sprintf(query,"delete from dks_uservcbfavorite where nuserid=%d and nvcbid=%d",userid, vcbid);
			if (0 == exec_query(sink, query))
			{
				sprintf(query, " update dks_vcbinformation a set total_attention=(select count(nuserid) from dks_uservcbfavorite where nvcbid=a.nvcbid) where nvcbid=%u ", vcbid);
				nRet = exec_query(sink, query);
			}
		}
	}

	return nRet;
}

int get_room_collect_num(Sink* sink, uint32_t vcbid, uint32_t *pNum)
{
	Result res;
	int row, state;
	char query[256];

	sprintf(query,"select total_attention from dks_vcbinformation where nvcbid=%u ", vcbid);

	row = 0;
	res.priv=0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	row = result_rn(sink, &res);

	if (pNum)
		*pNum = atoi(result_get(sink, &res, 0, 0));

	result_clean(sink, &res);

	return row;
}

int write_addlastgoroom_DB(Sink* sink, int userid, int vcbid, int type)
{
	char szTime[32]={0};
	char query[256];
	time_t tnow=time(0);

	toStringTimestamp(tnow, szTime);
	sprintf(query, "delete from dks_uservcbgolog where nuserid=%d and nvcbid=%d", userid, vcbid);
	exec_query(sink, query);

	sprintf(query, "insert into dks_uservcbgolog(nuserid,nvcbid,ntype,dtime1) values(%d,%d,%d,'%s')",
		userid, vcbid, type, szTime);
	exec_query(sink, query);
	return 0;
}

int get_syscasts_DB(Sink* sink, SList** list,int casttype, int recid)
{
	Result res;
	SList *head=0;
	int row, state, r;
	char szTime[32]={0};
	char query[256]={0};
    time_t tnow=time(0);
	const char *czvalue;

	toStringTimestamp(tnow, szTime);
	if(casttype == 0) 
	{
		sprintf(query, "select nid,ntype,nisoneoff,cnewstext from dks_syscastnews where nid>%d and nisoneoff=0 order by nid asc limit 1", recid);
		res.priv = 0;
		sink_exec2(sink, query, &res);
		state = result_state (sink, &res);
		if((state != RES_COMMAND_OK) &&
			(state != RES_TUPLES_OK)  &&
			(state != RES_COPY_IN)  &&
			(state != RES_COPY_OUT))
		{
			result_clean(sink, &res);
			return -1;
		}
		row = result_rn(sink, &res);
		if(row == 0)
		{
			result_clean(sink, &res);
			recid = 0;

			sprintf(query, "select nid,ntype,nisoneoff,cnewstext from dks_syscastnews where nid>%d and nisoneoff=0 order by nid asc limit 1", recid);
			res.priv = 0;
			sink_exec2(sink, query, &res);
			state = result_state (sink, &res);
			if((state != RES_COMMAND_OK) &&
				(state != RES_TUPLES_OK)  &&
				(state != RES_COPY_IN)  &&
				(state != RES_COPY_OUT))
			{
				result_clean(sink, &res);
				return -1;
			}
			row = result_rn(sink, &res);
		}

		for(r=0; r<row; r++)
		{
			DDSysCastNews_t* record=(DDSysCastNews_t*)malloc(sizeof(DDSysCastNews_t));
			memset(record, 0, sizeof(DDSysCastNews_t));

			czvalue = result_get(sink, &res, r, 0);
			record->nid = atoi(czvalue);
			czvalue = result_get(sink, &res,r, 1);
			record->ntype = atoi(czvalue);
			czvalue = result_get(sink, &res,r,2);
			record->nisoneoff = atoi(czvalue);
			czvalue = result_get(sink, &res, r, 3);
			strncpy(record->cnewstext, czvalue, sizeof(record->cnewstext));

			head = slist_append(head, record);
		}

		result_clean(sink, &res);
		*list = head;
		return row;
	}
	else
	{
		sprintf(query, "select nid,ntype,nisoneoff,cnewstext,dusetime from dks_syscastnews where nisoneoff=1 and dusetime<'%s'", szTime);
		res.priv = 0;
		sink_exec2(sink, query, &res);
		state = result_state(sink,&res);
		if((state != RES_COMMAND_OK) &&
			(state != RES_TUPLES_OK)  &&
			(state != RES_COPY_IN)  &&
			(state != RES_COPY_OUT))
		{
			result_clean(sink, &res);
			return -1;
		}
		row = result_rn(sink, &res);
		for(r=0; r<row; r++)
		{
			DDSysCastNews_t* record=(DDSysCastNews_t*)malloc(sizeof(DDSysCastNews_t));
			memset(record, 0, sizeof(DDSysCastNews_t));

			czvalue = result_get(sink, &res, r, 0);
			record->nid = atoi(czvalue);
			czvalue = result_get(sink, &res, r, 1);
			record->ntype = atoi(czvalue);
			czvalue = result_get(sink, &res,r,2);
			record->nisoneoff = atoi(czvalue);
			czvalue = result_get(sink, &res, r, 3);
			strncpy(record->cnewstext, czvalue, sizeof(record->cnewstext));

			head = slist_append(head, record);
		}

		result_clean(sink, &res);
		sprintf(query, "delete from dks_syscastnews where nisoneoff=1 and dusetime<'%s'", szTime);
		exec_query(sink, query);

		*list = head;
		return row;
	}
}

int get_single_siegelog_DB(Sink* sink,  struct tbviewsiegelog* tvs)
{
	Result res;
	int row, state;
	char query[512];
	const char *szval;

	res.priv = 0;
	//where and  dsenttime>t_tnow-interval'168 hours'
	sprintf(query, "select nvcbid,nsrcid,nbuddyid,ngiftid,ncount,dsenttime from tbsiegelog order by dsenttime desc limit 1");
	//sprintf(query, "select nvcbid,nsrcid,nbuddyid,ngiftid,ncount,dsenttime from tbsiegelog order by ncount desc limit 1");

	row = 0;
	res.priv=0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	row = result_rn(sink, &res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return -1;
	}

	szval = result_get(sink, &res, 0, 0);
	tvs->nvcbid = atoi(szval);
	szval = result_get(sink, &res, 0, 1);
	tvs->nsrcid = atoi(szval);
	szval = result_get(sink, &res, 0, 2);
	tvs->nbuddyid = atoi(szval);
	szval = result_get(sink, &res, 0, 3);
	tvs->ngiftid = atoi(szval);
	szval = result_get(sink, &res, 0, 4);
	tvs->ncount = atoi(szval);
	szval = result_get(sink, &res, 0, 5);
	tvs->nsenttime = toNumericTimestamp(szval);

	//5~{LlSPP'FZ~}
	if(time(0) - tvs->nsenttime >= 5 *24 * 3600)
	{
		result_clean(sink, &res);
		return -1;
	}

	//~{;qH!SC;'DX3F~}
	{
		Result res2;
		res2.priv = 0;
		//src alias
		sprintf(query, "select calias from dks_user where nuserid=%d", tvs->nsrcid);
		row = 0;
		sink_exec2(sink, query, &res2);
		state = result_state(sink, &res2);
		if((state != RES_COMMAND_OK) &&
			(state != RES_TUPLES_OK)  &&
			(state != RES_COPY_IN)    &&
			(state != RES_COPY_OUT))
		{
			result_clean(sink, &res2);
			result_clean(sink, &res);
			return -1;
		}

		row =result_rn (sink, &res2);
		if (row != 1)
		{
			result_clean (sink, &res2);
			result_clean(sink, &res);
			return -1;
		}

		szval = result_get(sink, &res2, 0, 0);
		strncpy(tvs->srcalias, szval, 32);
		tvs->srcalias[32 -1]='\0';

		result_clean (sink, &res2);

		//to alias
		sprintf(query, "select calias from dks_user where nuserid=%d", tvs->nbuddyid);
		row = 0;
		sink_exec2(sink, query, &res2);
		state = result_state(sink, &res2);
		if((state != RES_COMMAND_OK) &&
			(state != RES_TUPLES_OK)  &&
			(state != RES_COPY_IN)    &&
			(state != RES_COPY_OUT))
		{
			result_clean(sink, &res2);
			result_clean(sink, &res);
			return -1;
		}

		row =result_rn (sink, &res2);
		if (row != 1)
		{
			result_clean (sink, &res2);
			result_clean(sink, &res);
			return -1;
		}

		szval = result_get(sink, &res2, 0, 0);
		strncpy(tvs->toalias, szval, 32);
		tvs->toalias[32 -1]='\0';

		result_clean (sink, &res2);
	}

	result_clean(sink, &res);
	return 0;
}

int insert_useropenchestlog_DB(Sink* sink, int userid, int invcbid, int ncost, int winid, int winval, char* sztime)
{
	char szquery[256];
	sprintf(szquery, "insert into tbuseropenchestlog(nuserid,nvcbid,ncost,nwinid,nwinval,dtime) values(%d,%d,%d,%d,%d,'%s')",
		userid, invcbid, ncost, winid, winval, sztime);

	return exec_query(sink, szquery);
}

int get_useropenchest_stainfo_DB(Sink* sink, int userid, DDUserOpenChestStatInfo_t* pDDinfo, char * sztime)
{
	Result res;
	char szquery[256];
	int state;
	int row;
	const char* szVal;

	pDDinfo->userid = userid;
	sprintf(szquery, "select count(*) from tbuseropenchestlog where nuserid=%d and dtime>='%s'", userid, sztime);
	res.priv = 0;
	sink_exec2( sink, szquery, &res);
	state = result_state( sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}
	row = result_rn(sink, &res);
	if (row != 1) {
		result_clean (sink, &res);
		return -1;
	}

	szVal = result_get(sink, &res, 0, 0);
	pDDinfo->todaynum = 5-atoi(szVal);
	if(pDDinfo->todaynum <0) pDDinfo->todaynum = 0;

	result_clean (sink, &res);

	sprintf(szquery,"select nuserid, count(nid) as v1, sum(ncost) as v2,sum(nwinval) as v3 \
from tbuseropenchestlog where nuserid=%d group by nuserid",
		userid);
	res.priv = 0;
	sink_exec2( sink, szquery, &res);
	state = result_state( sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}
	row = result_rn(sink, &res);
	if (row == 1) {
		//szVal = result_get(sink, &res, 0, 0);  //~{2;PhR*Ub8vWV6N~}
		szVal = result_get(sink, &res, 0, 1);
		pDDinfo->totalnum = atoi(szVal);
		szVal = result_get(sink, &res, 0, 2);
		pDDinfo->totalcost = atoi(szVal);
		szVal = result_get(sink, &res, 0, 3);
		pDDinfo->totalwinval = atoi(szVal);
	}

	result_clean (sink, &res);
	return 0;

}

//////////////////////////////////////////////////////////////////////////

int func_addmonthgift_DB(Sink* sink, int userid, int giftid, int giftcount)
{
	Result res;
	char query[128]={0};
	int state;
	int ret_sucess = -1;
	const char* str;

	sprintf(query,"select updateusermonthgift(%d,%d,%d)", userid, giftid, giftcount);
	res.priv=0;
	sink_exec2(sink, query, &res);
	state = result_state (sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)  &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	str = result_get (sink, &res, 0, 0);
	ret_sucess = atoi(str);
	result_clean (sink, &res);
	return ret_sucess;
}

int func_getmonthgift_DB(Sink* sink, int userid, int giftid)
{
	Result res;
	char query[128]={0};
	int state;
	int ret_sucess = -1;
	const char* str;

	sprintf(query,"select getusermonthgift(%d,%d)", userid, giftid);
	res.priv=0;
	sink_exec2(sink, query, &res);
	state = result_state (sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)  &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	str = result_get (sink, &res, 0, 0);
	ret_sucess = atoi(str);
	result_clean (sink, &res);
	return ret_sucess;
}

int func_tradegift_DB(Sink* sink, int src_id, int dst_id, 
					  int gift_id, int gift_count, int gift_type, 
					  int64 total_out, int64 total_in, int trade_type, int room_id, 
					  int64* src_now_nk, int64* src_now_nb, 
					  int64* dst_now_nk, int64* dst_now_nb,
					  int* src_add_nchestnum,
					  int* src_now_nchestnum)
{
	Result res;
	char query[1024];
	int state;
	int row;
	int ret_sucess;
	int ret_return = 0;
	const char* str;
	Sparser spar;
	query[0] = 0;

	sprintf(query, "select tradegift3(%d,%d,%d,%d,%d,%lld,%lld,%d,%d)", src_id,dst_id,gift_id,gift_count,gift_type,total_out,total_in,trade_type,room_id);
	res.priv=0;
	sink_exec2(sink, query, &res);
	state = result_state (sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)  &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}
	row =result_rn(sink, &res);
	if (row != 1)
	{
		result_clean(sink, &res);
		return -1;
	}

	str = result_get(sink, &res, 0, 0);
	//printf("%s\n", str);
	spar = StringParser(str);
	//printf("%s\n", spar.content[0]);
	//printf("%s\n", spar.content[1]);
	//printf("%s\n", spar.content[2]);
	//printf("%s\n", spar.content[3]);
	//printf("%s\n", spar.content[4]);

	ret_sucess = atoi(spar.content[0]);
	if(ret_sucess == 0)
	{
		*src_now_nk =atoll(spar.content[1]);
		*src_now_nb =atoll(spar.content[2]);
		*dst_now_nk =atoll(spar.content[3]);
		*dst_now_nb =atoll(spar.content[4]);
		*src_add_nchestnum = 0;
		*src_now_nchestnum = 0;
		ret_return = 0;
	}
	else if(ret_sucess == 1)
	{
		*src_now_nk =atoll(spar.content[1]);
		*src_now_nb =atoll(spar.content[2]);
		*dst_now_nk =atoll(spar.content[3]);
		*dst_now_nb =atoll(spar.content[4]);
        *src_add_nchestnum =atoi(spar.content[5]);
		*src_now_nchestnum =atoi(spar.content[6]);
		ret_return = 1;
	}
	else{
		ret_return = -2;
	}

	result_clean(sink, &res);
	return ret_return;
}

int function_tradegift_yanhua_DB(Sink* sink, int src_id, int dst_id, 
								 int gift_id, int gift_count, int gift_type, 
								 int to_type, int to_vcbid,
								 int64 total_out, int64 total_in, int room_id, 
								 int64* src_now_nk, int64* src_now_nb,
								 int64* dst_now_nk, int64* dst_now_nb)
{
	Result res;
	char query[1024];
	int state;
	int row;
	int ret_sucess;
	int ret_return=0;
	const char* str;
	Sparser spar;
	query[0]=0;

	(*src_now_nk)=0;
	(*src_now_nb)=0;
	(*dst_now_nk)=0;
	(*dst_now_nb)=0;
	sprintf(query, "select tradegift3_yanhua_nk(%d,%d,%d,%d,%d,%d,%d,%lld,%lld,%d)",
		src_id,dst_id,
		gift_id,gift_count,gift_type,
		to_type, to_vcbid,
		total_out,total_in,room_id);
	res.priv=0;
	sink_exec2(sink, query, &res);
	state = result_state (sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)  &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}
	row =result_rn(sink, &res);
	if (row != 1)
	{
		result_clean(sink, &res);
		return -1;
	}

	str = result_get(sink, &res, 0, 0);
	//printf("%s\n", str);
	spar = StringParser(str);
	//printf("%s\n", spar.content[0]);
	//printf("%s\n", spar.content[1]);
	//printf("%s\n", spar.content[2]);
	//printf("%s\n", spar.content[3]);
	//printf("%s\n", spar.content[4]);

	ret_sucess = atoi(spar.content[0]);
	if(ret_sucess == 0)
	{
		*src_now_nk =atoll(spar.content[1]);
		*src_now_nb =atoll(spar.content[2]);
		*dst_now_nk =atoll(spar.content[3]);
		*dst_now_nb =atoll(spar.content[4]);
		ret_return = 0;
	}
	else if(ret_sucess == 1)
	{
		*src_now_nk =atoll(spar.content[1]);
		*src_now_nb =atoll(spar.content[2]);
		*dst_now_nk =atoll(spar.content[3]);
		*dst_now_nb =atoll(spar.content[4]);
		ret_return = 1;
	}
	else{
		ret_return = -2;
	}
	result_clean(sink, &res);
	return ret_return;
}

int func_tradesiege_DB(Sink* sink, int vcbid, int srcid, int toid, int giftid, int giftcount)
{
	Result res;
	char query[128]={0};
	int state;
	int ret_sucess = -1;
	const char* str;

	sprintf(query,"select updatenewsiegefunc(%d,%d,%d,%d,%d)", vcbid, srcid, toid, giftid, giftcount);
	res.priv=0;
	sink_exec2(sink, query, &res);
	state = result_state (sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)  &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	str = result_get (sink, &res, 0, 0);
	ret_sucess = atoi(str);
	result_clean (sink, &res);
	return ret_sucess;
}


//gch++
int update_vcbmedia_DB(Sink* sink,int vcbid,char* szmediasvr)
{
	char query[256];
	sprintf(query, "update dks_vcbstatus set cmedia='%s' where nvcbid=%d",szmediasvr,vcbid);
	return exec_query(sink, query);
}

int update_userpwd_DB(Sink* sink, int userid, char* pPwd,int pwdtype,char **strQuery)
{
	char query[256] = {0};
	if(pwdtype == 1) //~{SC;'5GB<C\Bk~}
	{
		sprintf(query, "update dks_user set cpassword='%s' where nuserid=%d", pPwd, userid);

		if(strQuery != NULL && *strQuery != NULL){
			sprintf(*strQuery,"%s",query);
		}

		return exec_query(sink, query);

	}
	else if(pwdtype == 2) //~{RxPPC\Bk~}
	{
		sprintf(query, "update dks_user set cbankpwd='%s' where nuserid=%d", pPwd, userid);
		return exec_query(sink, query);
	}
	return -1;
}

int update_user_baseinfo_DB(Sink* sink,DDUserBaseInfo_t* ppf, int userid)
{
	char szquery[256]={0};
	sprintf(szquery, "update dks_user set nheadid=%d,ngender=%d,calias='%s' where nuserid=%d",
		ppf->headid,ppf->gender,ppf->calias,userid);
	return exec_query(sink, szquery);
}

int insert_tbfireworkslog_DB(Sink* sink,DDFireworksLogTab_t* pTab)
{
	//~{J}>]?b1m~}
	//CREATE TABLE tb_tradefireworkslog
	//	(
	//	sid serial NOT NULL,
	//	nuserid integer NOT NULL,  //~{4KLu<GB<KySPU_~}
	//	nsrcuserid integer NOT NULL,
	//	ndestuserid integer NOT NULL,
	//	nvcbid integer NOT NULL,
	//	ngiftid integer NOT NULL,
	//	ngiftnum integer NOT NULL,
	//	nk_out integer NOT NULL DEFAULT 0,
	//	nb_out integer NOT NULL DEFAULT 0,
	//	nk_in integer NOT NULL DEFAULT 0,
	//	nb_in integer NOT NULL DEFAULT 0,
	//	nk bigint NOT NULL DEFAULT 0,
	//	nb bigint NOT NULL DEFAULT 0,
	//	tdate timestamp with time zone NOT NULL DEFAULT now()
    //  )
    char query[256] = {0};
	sprintf(query, "INSERT INTO tb_tradefireworkslog(nuserid, nsrcuserid, ndestuserid, nvcbid, ngiftid, ngiftnum, \
		            nk_out, nb_out, nk_in, nb_in, nk, nb) VALUES (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%lld,%lld)",pTab->userid,pTab->srcuserid,pTab->dstuserid,
					pTab->vcbid,pTab->giftid,pTab->giftnum,pTab->nk_out,pTab->nb_out,pTab->nk_in,pTab->nb_in,pTab->nk,pTab->nb);

	return exec_query(sink, query);
}

int insert_sendseallog_DB(Sink* sink,DDSendSealLogTab_t* pTab)
{
	//CREATE TABLE tb_sendseallog
	//	(
	//	sid serial NOT NULL,
	//	nuserid integer NOT NULL,
	//	ntouserid integer NOT NULL,
	//	nvcbid integer NOT NULL,
	//	nsealid integer NOT NULL,
	//	nk_out integer NOT NULL,
	//	nk bigint NOT NULL,
	//	nb bigint NOT NULL,
	//	CONSTRAINT pk_tb_sendseallog PRIMARY KEY (sid )
	//	)
	char query[256] = {0};
	sprintf(query, "INSERT INTO tb_sendseallog(nuserid, ntouserid, nvcbid, nsealid, nk_out,nk, nb) VALUES(%d,%d,%d,%d,%d,%lld,%lld)",
		    pTab->userid,pTab->touserid,pTab->vcbid,pTab->sealid,pTab->nk_out,pTab->nk,pTab->nb);

	return exec_query(sink, query);

}

int insert_userloginroomlog_DB(Sink* sink,DDUserLoginRoomLogTab_t* pTab)
{
	char query[256] = {0};
	sprintf(query, "INSERT INTO tbuserloginroomlog(nuserid, nvcbid, cloginip, cloginmac, tlogintime, tlogouttime) VALUES(%d,%d,'%s','%s','%s','%s')",
		pTab->userid,pTab->vcbid,pTab->ip,pTab->mac,pTab->logintime,pTab->logoutime);

	return exec_query(sink, query);
}

int insert_useronmictimelog_DB(Sink* sink,DDUserOnMicLogTab_t* pTab)
{
	char query[256] = {0};
	sprintf(query, "INSERT INTO tbuseronmictimelog(nuserid, nvcbid, nmictype, tupmictime, tdownmictime,npay) VALUES(%d,%d,%d,'%s','%s',%d)",
		pTab->userid,pTab->vcbid,pTab->mictype,pTab->upmictime,pTab->downmictime,pTab->npay);

	return exec_query(sink, query);
}

int get_vcbgateaddr_DB(Sink* sink,int vcbid,char* pAddr)
{
	Result res;
	char query[256];
	int state, row;
	const char* str;

	sprintf(query, "SELECT cgateaddr FROM dks_vcbstatus WHERE nvcbid=%d",vcbid);

	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state( sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}
	row = result_rn(sink, &res);
	if (row != 1) {
		result_clean (sink, &res);
		return -1;
	}

	str = result_get(sink, &res, 0, 0);
	strcpy(pAddr, str);

	result_clean (sink, &res);
	return 0;

}

int write_useropnknblog_DB(Sink* sink, DDUsernknbOplog_t* pkboplog)
{
	char query[512] = {0};
	sprintf(query, "insert into tbusernknboplog(nvcbid,nsrcid,ntoid,noptype,nmoney,nkbef,nbbef,nkdepositbef,nkaft,nbaft,nkdepositaft,nkbef2,nbbef2,nkaft2,nbaft2,dtime) \
values(%d,%d,%d,%d,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,'%s')",
pkboplog->nvcbid,pkboplog->nsrcid,pkboplog->ntoid,pkboplog->noptype,pkboplog->nmoney,
pkboplog->nkbefore,pkboplog->nbbefore,pkboplog->nkdepositbefore,
pkboplog->nkafter,pkboplog->nbafter,pkboplog->nkdepositafter,
pkboplog->nkbef2,pkboplog->nkbef2,
pkboplog->nkaft2,pkboplog->nbaft2,
pkboplog->dtime);
	return exec_query(sink, query);

}

int func_getusercostlevel_DB(Sink* sink, int userid)
{
	Result res;
	char query[128]={0};
	int state;
	int ret_sucess = -1;
	const char* str;

	sprintf(query,"select getusercostlevel(%d)", userid);
	res.priv=0;
	sink_exec2(sink, query, &res);
	state = result_state (sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)  &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	str = result_get (sink, &res, 0, 0);
	ret_sucess = atoi(str);
	result_clean (sink, &res);
	return ret_sucess;
}

int func_getuserlastmonthcostlevel_DB(Sink* sink, int userid)
{
	/*
	Result res;
	char query[128]={0};
	int state;
	int row;
	int ret_sucess = -1;
	const char* str;

	sprintf(query,"select getuserlastmonthcostlevel(%d)", userid);
	res.priv=0;
	sink_exec2(sink, query, &res);
	state = result_state (sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)  &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	str = result_get (sink, &res, 0, 0);
	ret_sucess = atoi(str);
	result_clean (sink, &res);
	return ret_sucess;
	*/

	return 0;
}

int func_getuserthismonthcostlevel_DB(Sink* sink, int userid)
{
	/*
	Result res;
	char query[128]={0};
	int state;
	int row;
	int ret_sucess = -1;
	const char* str;

	sprintf(query,"select getuserthismonthcostlevel(%d)", userid);
	res.priv=0;
	sink_exec2(sink, query, &res);
	state = result_state (sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)  &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	str = result_get (sink, &res, 0, 0);
	ret_sucess = atoi(str);
	result_clean (sink, &res);
	return ret_sucess;
	*/

	return 0;
}

int func_getuserthismonthcostgrade_DB(Sink* sink, int userid)
{
	/*
	Result res;
	char query[128]={0};
	int state;
	int row;
	int ret_sucess = -1;
	const char* str;

	sprintf(query,"select getuserthismonthcostgrade(%d)", userid);
	res.priv=0;
	sink_exec2(sink, query, &res);
	state = result_state (sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)  &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	str = result_get (sink, &res, 0, 0);
	ret_sucess = atoi(str);
	result_clean (sink, &res);
	return ret_sucess;
	*/
	return 0;
}

int func_getusercaifugrade_DB(Sink* sink, int userid)
{
	Result res;
	char query[128]={0};
	int state;
	int ret_sucess = -1;
	const char* str;

	sprintf(query,"select getusercaifugrade(%d)", userid);
	res.priv=0;
	sink_exec2(sink, query, &res);
	state = result_state (sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)  &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	str = result_get (sink, &res, 0, 0);
	ret_sucess = atoi(str);
	result_clean (sink, &res);
	return ret_sucess;

}

int func_isquzhang_DB(Sink* sink,int vcbid,int userid)
{
	Result res;
	char query[128]={0};
	int state;
	int ret_sucess = -1;
	const char* str;

	sprintf(query,"select isquzhang2(%d,%d)",vcbid,userid);
	res.priv=0;
	sink_exec2(sink, query, &res);
	state = result_state (sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)  &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	str = result_get (sink, &res, 0, 0);
	ret_sucess = atoi(str);
	result_clean (sink, &res);
	return ret_sucess;
}


int func_updateusermonthcost_DB(Sink* sink, int userid, int64 cost_nk)
{
	Result res;
	char query[128]={0};
	int state;

	sprintf(query,"select updateusermonthcost(%d,%lld)", userid, cost_nk);
	res.priv=0;
	sink_exec2(sink, query, &res);
	state = result_state (sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)  &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	result_clean (sink, &res);
	return 0;
}

int func_moneyandpointop(Sink* sink,int64* src_nk,int64* src_nb,int64* src_nkdeposit,
						 int64* dst_nk, int64* dst_nb, int64* dst_nkdeposit,
						 int src_userid, int dst_userid, int op_type, int64 op_money1,int64 op_money2)
{
	Result res;
	char query[128]={0};
	int row,state,ret_sucess;
	const char* str;
	Sparser spar;

	sprintf(query, "select fun_moneyandpointop(%d,%d,%d,%lld,%lld)", src_userid,dst_userid,op_type,op_money1,op_money2);
	res.priv=0;
	sink_exec2(sink, query, &res);
	state = result_state (sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)  &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	row =result_rn(sink, &res);
	if (row != 1)
	{
		result_clean(sink, &res);
		return -1;
	}
	str = result_get(sink, &res, 0, 0);
	spar = StringParser(str);
	ret_sucess = atoi(spar.content[0]);

	if(src_nk !=0) (*src_nk) = atoll(spar.content[1]);
	if(src_nb !=0) (*src_nb) = atoll(spar.content[2]);
	if(src_nkdeposit !=0) (*src_nkdeposit) = atoll(spar.content[3]);
	if(dst_nk !=0) (*dst_nk) = atoll(spar.content[4]);
	if(dst_nb !=0) (*dst_nb) = atoll(spar.content[5]);
	if(dst_nkdeposit !=0) (*dst_nkdeposit) = atoll(spar.content[6]);

	result_clean(sink, &res);
	return ret_sucess;
}

int update_roommgronlinetime(Sink* sink, DDUpdateMgrOnlineTime_t* pdata)
{
	Result res;
	int row, state;
	int itodayttime = 0;
	char query[256] = {0};
	const char* str;

	sprintf(query, "select todayttime from tbnuserid_time where nuserid=%d and nvcbid=%d",pdata->nuserid,pdata->nvcbid);
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	row = result_rn(sink, &res);
	
	if(row == 0)
	{
		result_clean(sink, &res);
		sprintf(query, "INSERT INTO tbnuserid_time(nuserid, ttime, todayttime, nvcbid) VALUES (%d, 0, 0, %d);",pdata->nuserid,pdata->nvcbid);
		return exec_query(sink, query);
	}
	else
	{
		str = result_get (sink, &res, 0, 0);
		itodayttime = atoi(str);
		itodayttime += pdata->intervaltime;

		result_clean(sink, &res);
		sprintf(query, "UPDATE tbnuserid_time SET todayttime=%d WHERE nuserid=%d and nvcbid=%d;",itodayttime,pdata->nuserid,pdata->nvcbid);
		return exec_query(sink, query);
	}

	result_clean(sink, &res);
	return 0;
}



int get_zhuanbo_mainroom_DB(Sink* sink, int* list, int roomid,int userid)
{
    int temproomid=0;
    Result res;
    int row,state,r,j=0;
    char query[512];
    sprintf(query, "select nrobotvcbid from dks_vcbzhuanboconfig where nuserid=%d and nvcbid=%d ",userid,roomid);
    row =0;
    res.priv =0;
    sink_exec2(sink, query, &res);
    state =result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK)  &&
        (state != RES_COPY_IN)    &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return -1;
    }
    row=result_rn(sink,&res);
    if(row == 0)
    {
        result_clean(sink, &res);
        return 0;
    }

    for(r=0; r<row; r++)
    {
        temproomid = atoi(result_get(sink, &res, r, 0));
        if(temproomid!=0 && temproomid!=roomid)
        {
            list[j++]=temproomid;
        }
    }
    result_clean(sink, &res);
    return row;
}

int get_mainroom_zhuanbo_DB(Sink* sink, SList** list, int roomid,int userid)
{
    int temproomid=0;
    int tempuserid=0;
    Result res;
    SList* head=0;
    int row,state,r;
    char query[512];
    sprintf(query, "select nvcbid,nuserid from dks_vcbzhuanboconfig where nrobotid=%d and nrobotvcbid=%d ",userid,roomid);
    row =0;
    res.priv =0;
    sink_exec2(sink, query, &res);
    state =result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK)  &&
        (state != RES_COPY_IN)    &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return -1;
    }
    row=result_rn(sink,&res);
    if(row == 0)
    {
        result_clean(sink, &res);
        return -1;
    }

    for(r=0; r<row; r++)
    {
        temproomid = atoi(result_get(sink, &res, r, 0));
        if(temproomid!=0 && temproomid!=roomid)
        {
            tempuserid = atoi(result_get(sink, &res, r, 1));
            break;
        }
    }
    result_clean(sink, &res);
    *list = head;
    return tempuserid;
}

int check_teacher_score_DB(Sink* sink, int teacherid,int userid,char* logtime)
{
    Result res;
    int row,state;
    char query[512];
    char dtime[40];
    time_t t1 = toNumericTimestamp(logtime);
    time_t t2 = t1-1*60*60;
    toStringTimestamp(t2,dtime);

    sprintf(query, "select 1 from tbteacherscorelog where nteacherid=%d and nuserid=%d and dtime>='%s' ",teacherid,userid,dtime);
    row =0;
    res.priv =0;
    sink_exec2(sink, query, &res);
    state =result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK)  &&
        (state != RES_COPY_IN)    &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return -1;
    }
    row=result_rn(sink,&res);
    if(row == 0)
    {
        result_clean(sink, &res);
        return 0;
    }
    else
    {
        result_clean(sink, &res);
        return 1;
    }
}
int select_user_score_DB(Sink* sink, int teacherid,int userid,int *score)
{
    Result res;
    int row,state;
    char query[512];
    char dtime[40];
	const char* szVal;
	time_t t1 = time(NULL);
	time_t t2 = t1 - 1*60*60;
    toStringTimestamp(t2,dtime);
	
    sprintf(query, "select nscore from tbteacherscorelog where nteacherid=%d and nuserid=%d and dtime>='%s'  limit 1",teacherid,userid,dtime);
    row =0;
    res.priv =0;
    sink_exec2(sink, query, &res);
    state =result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK)  &&
        (state != RES_COPY_IN)    &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return 0;
    }
    row=result_rn(sink,&res);
    if(row == 0)
    {
        result_clean(sink, &res);
        return 0;
    }
	szVal = result_get(sink, &res, 0, 0);
	*score = atoi(szVal);
	result_clean(sink, &res);
	return row;
}
int select_user_scorelist_DB(Sink* sink,int roomid,int teacherid, SList** list)
{
	Result res;
	SList* head=0;
	int row,state,r;
	char query[512];
	char dtime[40];
	time_t t1 = time(NULL);
	time_t t2 = t1 - 1*60*60;
    toStringTimestamp(t2,dtime);

	sprintf(query, "select nuserid, nscore from tbteacherscorelog where nteacherid=%d  and dtime>='%s' ",teacherid,dtime);
	row =0;
	res.priv =0;
	sink_exec2(sink, query, &res);
	state =result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}
	row=result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}
	for(r=0; r<row; r++)
	{
		UserScoreList_t* record=(UserScoreList_t*)malloc(sizeof(UserScoreList_t));
		memset(record, 0, sizeof(UserScoreList_t));
		record->vcbid = roomid;
		record->teacherid = teacherid;
		record->userid = atoi(result_get(sink, &res, r, 0));
		record->score = atoi(result_get(sink, &res, r, 1));
		head = slist_append(head, record);
	}
	result_clean(sink, &res);
	*list = head;
	return row;
}
int write_teacherscorelog_DB(Sink* sink, CMDTeacherScoreRecord_t2* teacherlog)
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
    exec_query(sink, query);

    return 0;
}

int get_teacher_all_score_DB(Sink* sink, SList** list, int teatcher_userid)
{
	Result res;
	SList* head=0;
	int row,state,r;
	char query[512];
	char szTime[32]={0};
	struct tm *ptr;
	time_t lt;
	time_t lastTime;

	lt =time(NULL);
	ptr=localtime(&lt);

	lastTime = time(NULL) - (ptr->tm_hour*3600) - (ptr->tm_min*60) - ptr->tm_sec;

	toStringDate(lastTime, szTime);

	sprintf(query, "select nscore from tbteacherscorelog where nteacherid=%d AND dtime<'%s'", teatcher_userid, szTime);
	row =0;
	res.priv =0;
	sink_exec2(sink, query, &res);

	state =result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}
	row=result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	for(r=0; r<row; r++)
	{
		ScoreValue_t* record=(ScoreValue_t*)malloc(sizeof(ScoreValue_t));
		memset(record, 0, sizeof(ScoreValue_t));

		record->score_value = atoi(result_get(sink, &res, r, 0));
		
		head = slist_append(head, record);
	}
	result_clean(sink, &res);

	*list = head;
	return row;
}

int get_teacher_gift_list_DB(Sink* sink, SList** list, int roomid,int teacherid)
{
	Result res;
	SList* head=0;
	int row,state,r;
	char query[512];
	char szTime[32]={0};
	struct tm *ptr;
	time_t lt;
	int passDay = 0;
	time_t lastTime;

	//~{<FKc1>V\R;JGDDR;Ll~}
	lt =time(NULL);
	ptr=localtime(&lt);
	
	if(ptr->tm_wday == 0){//~{Hg9{JGV\HU>MJG~}6~{Ll#,FdS`5DHUWS<uR;~}
		passDay = 6;
	} else{
		passDay = (ptr->tm_wday-1);
	}

	//~{V1=S;X5=IOR;8vV\R;Ac5c~}
	lastTime = time(NULL) - (86400*passDay) - (ptr->tm_hour*3600) - (ptr->tm_min*60) - ptr->tm_sec;//1~{Ll~}(d)=86400~{Ck~}(s),1~{J1~}(h)=3600~{Ck~}(s),1~{7V~}(min)=60~{Ck~}(s)

	toStringDate(lastTime, szTime);


	//~{O^6(~}6~{Lu#,2bJTVP7">uUbLu~}sql~{So>dR;0c:DJ1~}180ms~{WsSR~}
    sprintf(query, "select nuserid, sum(nusermoney) AS totalMoney from dks_usergifttradelog where nbuddyid=%d AND dtime>'%s' group by nuserid order by sum(nusermoney) desc limit 6", teacherid, szTime);
	row =0;
	res.priv =0;
	sink_exec2(sink, query, &res);

	state =result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}
	row=result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	for(r=0; r<row; r++)
	{
		Result res1;
		char usr_query[512];
		int row1 = 0;
		int tmp_user_id = 0;
		
		CMDTeacherThisWeekFans_t* record=(CMDTeacherThisWeekFans_t*)malloc(sizeof(CMDTeacherThisWeekFans_t));
		memset(record, 0, sizeof(CMDTeacherThisWeekFans_t));

		res1.priv = 0;
		record->totalmoney = atoi(result_get(sink, &res, r, 1));
		tmp_user_id = atoi(result_get(sink, &res, r, 0));

		sprintf(usr_query, "select calias from dks_user where nuserid=%d limit 1", tmp_user_id);
		sink_exec2(sink, usr_query, &res1);
		state = result_state(sink, &res1);
		if((state != RES_COMMAND_OK) &&
			(state != RES_TUPLES_OK)  &&
			(state != RES_COPY_IN)    &&
			(state != RES_COPY_OUT))
		{
			result_clean(sink, &res1);
			return 0;
		}

		row1 = result_rn(sink,&res1);
		if(row1 == 0)
		{
			result_clean(sink, &res1);
			return 0;
		}

		strcpy(record->alias, result_get(sink, &res1,0,0));
		result_clean(sink, &res1);
		head = slist_append(head, record);
	}
	result_clean(sink, &res);

	*list = head;
	return row;
}

int get_team_top_money(Sink* sink, Result *res, SList** list)
{
	SList* head=0;
	int row,state;
	char query[512];
	char szTime[32]={0};
	int passDay = 0;


	time_t lt = time(0);
	struct tm *ptr = localtime(&lt);

	if(ptr->tm_wday == 0){
		passDay = 6;
	} else{
		passDay = (ptr->tm_wday-1);
	}

	time_t lastTime = time(NULL) - (86400*passDay) - (ptr->tm_hour*3600) - (ptr->tm_min*60) - ptr->tm_sec;

	toStringDate(lastTime, szTime);

	sprintf(query, "select nvcbid, sum(nusermoney) as money from dks_usergifttradelog where dtime > '%s' and nvcbid in (66805,66808,66810,66812,66813,66815,66816) \
			 GROUP BY nvcbid ORDER BY money DESC limit 5", szTime);
	row =0;
	res->priv =0;
	sink_exec2(sink, query, res);

	state =result_state(sink, res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, res);
		return 0;
	}
	row=result_rn(sink,res);
	if(row == 0)
	{
		result_clean(sink, res);
		return 0;
	}

	*list = head;
	return row;
}

int get_user_token_DB(Sink* sink, int userid, char* token, char* codetime)
{
	Result res;
	int row, state;
	char query[256];
	const char* szVal;

	sprintf(query, "select code,codetime from dks_user WHERE nuserid=%d", userid);
	row = 0;
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -2;
	}

	row = result_rn(sink,&res);
	if(row != 1)
	{
		result_clean(sink, &res);
		return -1;
	}

	szVal = result_get(sink,&res, 0,0);
	strcpy(token, szVal);
	szVal = result_get(sink,&res, 0,1);
	strcpy(codetime, szVal);
	result_clean(sink, &res);

	return row;
}

int update_user_token_DB(Sink* sink, int userid, const char* token, const char* validtime)
{
	char query[256];
	if(NULL == token){
		return -1;
	}
	sprintf(query, "UPDATE dks_user SET code='%s', codetime='%s' WHERE nuserid=%d;", token, validtime, userid);
	return exec_query(sink, query);
}

int get_userid_from_robotid(Sink* sink, int subroomid, int robotid, int *userid, int *mroomid)
{
	Result res;
	int row, state;
	char query[256];
	const char* szVal;

	sprintf(query, "select nuserid, nvcbid from dks_vcbzhuanboconfig where nrobotvcbid=%d and nrobotid=%d and nuserid !=0", subroomid, robotid);
	row = 0;
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}

	row = result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	szVal = result_get(sink, &res, 0, 0);
	*userid = atoi(szVal);
	szVal = result_get(sink, &res, 0, 1);
	*mroomid = atoi(szVal);
	result_clean(sink, &res);

	return row;
}

int get_userid_from_room(Sink* sink, int roomid, int *userid)
{
    Result res;
    int row, state;
    char query[256];
    const char* szVal;

    sprintf(query, "select nuserid from dks_vcbzhuanboconfig where nvcbid=%d limit 1", roomid);
    row = 0;
    res.priv = 0;
    sink_exec2(sink, query, &res);
    state = result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK)  &&
        (state != RES_COPY_IN)    &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return 0;
    }

    row = result_rn(sink,&res);
    if(row == 0)
    {
        result_clean(sink, &res);
        return 0;
    }

    szVal = result_get(sink, &res, 0, 0);
    *userid = atoi(szVal);
    result_clean(sink, &res);

    return row;
}
int get_user_secure_info_DB(Sink* sink, CMDSecureInfo_t* pTab, int userid)
{
	Result res;
	int row,state;
	char query[512];
	char szquery[256]={0};
	const char* czvalue;

	sprintf(query, "select cemail,cqq,ctel,nreminded_time from dks_user where nuserid=%d", userid);

	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}
	row = result_rn(sink, &res);
	if(row != 1) {
		result_clean(sink, &res);
		return -1;
	}

	czvalue = result_get(sink,&res, 0,0);
	strcpy(pTab->email, czvalue);
	czvalue = result_get(sink,&res,0,1);
	strcpy(pTab->qq, czvalue);
	czvalue = result_get(sink,&res,0,2);
	strcpy(pTab->phonenumber,czvalue);
	pTab->remindtimes = atoi(result_get(sink,&res,0,3));

	//
	sprintf(szquery, "update dks_user set nreminded_time=%d where nuserid=%d", pTab->remindtimes+1, userid);

	result_clean(sink, &res);
	if(pTab->remindtimes < 3 && (strlen(pTab->email) == 0 || strlen(pTab->qq) == 0 || strlen(pTab->phonenumber) == 0))
	{
		exec_query(sink, szquery);
	}
	return row;
}

int insert_gift_ticket(Sink* sink, int userid, int analystid, const char* analystname, int votecount)
{
	time_t tnow=time(0);
	char szTime[32]={0};
	char   query[256];

	toStringTimestamp(tnow, szTime);
    sprintf(query, "insert into tb_voteanalystlog(userid, analystid, analystname, votecount, isrepay, createtime) values(%d, %d, '%s', %d, 'true', '%s')",
    		userid, analystid, analystname, votecount, szTime);
	return exec_query(sink, query);
}

int get_user_exit_message(Sink* sink, CMDSecureInfo_t* pTab, int* hit_times, int userid)
{
	Result res;
	int row,state;
	char query[512];
	char szquery[256]={0};
	char szTime[32]={0};
	const char* czvalue;
	struct tm *ptr;
	time_t lt;
	time_t lastTime;
	
	lt =time(NULL);
	ptr=localtime(&lt);

	lastTime = time(NULL) - (ptr->tm_hour*3600) - (ptr->tm_min*60) - ptr->tm_sec;

	toStringDate(lastTime, szTime);

	sprintf(query, "select cemail,cqq,ctel from dks_user where nuserid=%d", userid);

	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}
	row = result_rn(sink, &res);
	if(row != 1) {
		result_clean(sink, &res);
		return -1;
	}

	czvalue = result_get(sink,&res, 0,0);
	strcpy(pTab->email, czvalue);
	czvalue = result_get(sink,&res,0,1);
	strcpy(pTab->qq, czvalue);
	czvalue = result_get(sink,&res,0,2);
	strcpy(pTab->phonenumber,czvalue);

	result_clean(sink, &res);
	sprintf(szquery, "select nuserid from tbuserrandomgoldegg where nuserid=%d AND createtime>='%s'", userid, szTime);

	row =0;
	res.priv =0;
	sink_exec2(sink, szquery, &res);
	state =result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}
	row=result_rn(sink,&res);

	result_clean(sink, &res);
	(*hit_times) = row;
	return 0;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//登入直播间记录
int insert_textlivelog_DB(Sink* sink, CMDTextRoomUserInfo_t *userinfo)
{
	time_t tnow=time(0);
	char szTime[32]={0};
	Result res;
	int row, state;
	char query[256];
	const char* szVal;
	int64 logid=0;

	int64 nowtime=0;
	char sztime[128]={0};
	toStringTimestamp3(tnow, sztime);
	nowtime=atoll(sztime);

	sprintf(query, " select nextval('tb_textliveroomuserlog_logid_seq') ");

	row = 0;
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}

	row = result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	szVal = result_get(sink, &res, 0, 0);
	logid = atoll(szVal);
	result_clean(sink, &res);

	toStringTimestamp(tnow, szTime);
	sprintf(query, "insert into tb_textliveroomuserlog(logid,useid,roomid,dtime,logtype) values(%lld,%d,%d,%lld,%d)",
			logid, userinfo->userid, userinfo->vcbid, nowtime, userinfo->logtype);
	return exec_query(sink, query);
}

//获得讲师信息
int get_teacher_info_DB(Sink* sink, CMDTextRoomTeacherNoty_t2* ptf, int roomid)
{
    Result res;
	char query[1024];
	int state, row;
	const char* str;

	if(roomid ==0) return -1;
	sprintf(query, " select t.roomid,t.teacherid,u.calias,t.headid,t.clevel,t.clabel,t.goodat,t.introduce \
            from dks_teacherinfo as t,dks_user as u \
            where t.roomid=%d and t.teacherid=u.nuserid ", roomid);
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}
	row = result_rn(sink, &res);
	if (row != 1) {
		result_clean (sink, &res);
		return -1;
	}

    str = result_get(sink, &res, 0, 0);
	ptf->vcbid = atoi(str);

	str = result_get(sink, &res, 0, 1);
	ptf->teacherid = atoi(str);
	str = result_get(sink, &res, 0, 2);
	strcpy(ptf->teacheralias, str);
	str = result_get(sink, &res, 0, 3);
	ptf->headid = atoi(str);
	str = result_get(sink, &res, 0, 4);
	strncpy(ptf->clevel, str,sizeof(ptf->clevel));
	str = result_get(sink, &res, 0, 5);
	strncpy(ptf->clabel, str,sizeof(ptf->clabel));
	str = result_get(sink, &res, 0, 6);
	strncpy(ptf->cgoodat, str,sizeof(ptf->cgoodat));
	str = result_get(sink, &res, 0, 7);
	strncpy(ptf->cintroduce, str,sizeof(ptf->cintroduce));

	result_clean(sink, &res);
	return 0;
}

//今日人气(type=1)/直播人气(type=2)
int64 get_textlivetoday_DB(Sink* sink, int roomid ,int type)
{
    Result res;
    int row, state;
    char query[256];
    const char* szVal;
    int64 count=0;

    if(type==1)
    {
    	int64 begintime=0,endtime=0;
    	char sztime[128]={0};
    	time_t tnow=time(0);
    	toStringTimestamp3(tnow, sztime);
    	endtime=atoll(sztime);
    	begintime=(endtime/1000000)*1000000;


		sprintf(query, "select count(1) from tb_textliveroomuserlog where roomid=%d and dtime>=%lld and dtime<=%lld ",
				roomid,begintime,endtime);
    }
    else if(type==2)
    {
		sprintf(query, "select count(1) from tb_textliveroomuserlog where roomid=%d ", roomid);
    }
    row = 0;
    res.priv = 0;
    sink_exec2(sink, query, &res);
    state = result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK)  &&
        (state != RES_COPY_IN)    &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return 0;
    }

    row = result_rn(sink,&res);
    if(row == 0)
    {
        result_clean(sink, &res);
        return 0;
    }

    szVal = result_get(sink, &res, 0, 0);
    count = atoll(szVal);
    result_clean(sink, &res);

    return count;
}

//粉丝数量
int64 get_textlivefans_DB(Sink* sink, int teacherid)
{
    Result res;
    int row, state;
    char query[256];
    const char* szVal;
    int64 count=0;

	sprintf(query, "select count(1) from tb_teacherfans where teacherid=%d ", teacherid);

    row = 0;
    res.priv = 0;
    sink_exec2(sink, query, &res);
    state = result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK)  &&
        (state != RES_COPY_IN)    &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return 0;
    }

    row = result_rn(sink,&res);
    if(row == 0)
    {
        result_clean(sink, &res);
        return 0;
    }

    szVal = result_get(sink, &res, 0, 0);
    count = atoll(szVal);
    result_clean(sink, &res);

    return count;
}

//直播点赞数量
int64 get_textlivemessagezans_DB(Sink* sink, int teacherid)
{
    Result res;
    int row, state;
    char query[256];
    const char* szVal;
    int64 count=0;

	sprintf(query, "select sum(czans) from tb_textlivemessagehistory where teacherid=%d ", teacherid);

    row = 0;
    res.priv = 0;
    sink_exec2(sink, query, &res);
    state = result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK)  &&
        (state != RES_COPY_IN)    &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return 0;
    }

    row = result_rn(sink,&res);
    if(row == 0)
    {
        result_clean(sink, &res);
        return 0;
    }

    szVal = result_get(sink, &res, 0, 0);
    count = atoll(szVal);
    result_clean(sink, &res);

    return count;
}

//观点点赞数量
int64 get_textliveviewzans_DB(Sink* sink, int teacherid)
{
    Result res;
    int row, state;
    char query[256];
    const char* szVal;
    int64 count=0;

	sprintf(query, "select sum(czans) from tb_viewhistory where teacherid=%d ", teacherid);

    row = 0;
    res.priv = 0;
    sink_exec2(sink, query, &res);
    state = result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK)  &&
        (state != RES_COPY_IN)    &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return 0;
    }

    row = result_rn(sink,&res);
    if(row == 0)
    {
        result_clean(sink, &res);
        return 0;
    }

    szVal = result_get(sink, &res, 0, 0);
    count = atoll(szVal);
    result_clean(sink, &res);

    return count;
}

//直播历史数量
int64 get_textlivehistorys_DB(Sink* sink, int teacherid)
{
    Result res;
    int row, state;
    char query[256];
    const char* szVal;
    int64 count=0;

	char sztime[128]={0};
	time_t tnow=time(0);
	toStringTimestamp3(tnow, sztime);
	int64 todaytime=atoll(sztime);
	todaytime=(todaytime/1000000)*1000000;

	sprintf(query, "select count(1) from tb_textlivemessagehistory where dtime<=%lld and teacherid=%d ", todaytime, teacherid);

    row = 0;
    res.priv = 0;
    sink_exec2(sink, query, &res);
    state = result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK)  &&
        (state != RES_COPY_IN)    &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return 0;
    }

    row = result_rn(sink,&res);
    if(row == 0)
    {
        result_clean(sink, &res);
        return 0;
    }

    szVal = result_get(sink, &res, 0, 0);
    count = atoll(szVal);
    result_clean(sink, &res);

    return count;
}

//直播记录
int get_TextRoomLive_list_DB(Sink* sink, long fromDate,int offset,unsigned char bHistory/*获取当天历史直播记录入口填1*/,SList** list, CMDTextRoomLiveListReq_t2 *listreq)
{
	Result res;
	SList* head=0;
	int row,state,r;
	char query[2048];

	char sztime[128]={0};
	time_t tnow=time(0);
	toStringTimestamp4(tnow, sztime);
	long int todaytime=atol(sztime);

    if ( 0 != fromDate )
    {
        todaytime = fromDate;
    }

	if(listreq->count>0)
	{
		if(listreq->messageid>0)
		{
			sprintf(query, "select teacherid,userid,textliveid,keypoint,forcast,textlivetype,interactid,source,ccontent,dtime,czans,commentstype from  \
					(( select teacherid,0 as userid,textliveid,keypoint,forcast,textlivetype,interactid,ccontent as source,'' as ccontent,dtime,czans,commentstype \
					from tb_textlivemessagehistory \
					where ddate=%ld and textliveid%s%lld and textlivetype!=4 and teacherid=%d and forcast=0 order by textliveid DESC LIMIT %d ) \
					UNION \
					( select h.teacherid,h.userid,t.textliveid,t.keypoint,t.forcast,t.textlivetype,t.interactid,h.source,h.ccontent,t.dtime,t.czans,h.commentstype \
					from tb_textlivemessagehistory as t,tb_interacthistory as h \
					where t.ddate=%ld and t.textliveid%s%lld and t.textlivetype=4 and h.liveflag=1 and t.interactid=h.interactid \
					and t.teacherid=%d and t.forcast=0  order by textliveid DESC LIMIT %d )) \
					as textlivemessage \
					order by textliveid DESC LIMIT %d offset %d", todaytime,bHistory?"<=":"<",listreq->messageid,listreq->teacherid, listreq->count,
					todaytime,bHistory?"<=":"<",listreq->messageid,listreq->teacherid, listreq->count, listreq->count,offset);

		}
		else
		{
			sprintf(query, "select teacherid,userid,textliveid,keypoint,forcast,textlivetype,interactid,source,ccontent,dtime,czans,commentstype from  \
					(( select teacherid,0 as userid,textliveid,keypoint,forcast,textlivetype,interactid,ccontent as source,'' as ccontent,dtime,czans,commentstype \
					from tb_textlivemessagehistory \
					where ddate=%ld and textlivetype!=4 and teacherid=%d and forcast=0 order by textliveid DESC LIMIT %d ) \
					UNION \
					( select h.teacherid,h.userid,t.textliveid,t.keypoint,t.forcast,t.textlivetype,t.interactid,h.source,h.ccontent,t.dtime,t.czans,h.commentstype \
					from tb_textlivemessagehistory as t,tb_interacthistory as h \
					where t.ddate=%ld and t.textlivetype=4 and h.liveflag=1 and t.interactid=h.interactid \
					and t.teacherid=%d and t.forcast=0 order by textliveid DESC LIMIT %d )) \
					as textlivemessage \
					order by textliveid DESC LIMIT %d offset %d", todaytime,listreq->teacherid, listreq->count,
					todaytime,listreq->teacherid, listreq->count, listreq->count,offset);

		}
	}

	row =0;
	res.priv =0;
	sink_exec2(sink, query, &res);

	state =result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}
	row=result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	for(r=0; r<row; r++)
	{
		Result res1;
		char usr_query[512];
		int row1 = 0;
		int tmp_user_id = 0;

		CMDTextRoomLiveListNoty_t2* record=(CMDTextRoomLiveListNoty_t2*)malloc(sizeof(CMDTextRoomLiveListNoty_t2));
		memset(record, 0, sizeof(CMDTextRoomLiveListNoty_t2));

		res1.priv = 0;
		record->vcbid = listreq->vcbid;
		record->userid = listreq->userid;
		record->teacherid = atoi(result_get(sink, &res, r, 0));
		record->srcuserid = atoi(result_get(sink, &res, r, 1));
		record->messageid = atoll(result_get(sink, &res, r, 2));
		record->pointflag = atoi(result_get(sink, &res, r, 3));
		record->forecastflag = atoi(result_get(sink, &res, r, 4));
		record->livetype = atoi(result_get(sink, &res, r, 5));
		record->viewid = atoll(result_get(sink, &res, r, 6));
		strncpy(record->srctext, result_get(sink, &res,r,7),sizeof(record->srctext));
		strncpy(record->destext, result_get(sink, &res,r,8),sizeof(record->destext));
		record->messagetime = atoll(result_get(sink, &res, r, 9));
		record->zans = atoll(result_get(sink, &res, r, 10));
		record->commentstype = atoi(result_get(sink, &res, r, 11));

		tmp_user_id = atoi(result_get(sink, &res, r, 1));
		if(tmp_user_id>0)
		{
			sprintf(usr_query, "select calias from dks_user where nuserid=%d limit 1", tmp_user_id);
			sink_exec2(sink, usr_query, &res1);
			state = result_state(sink, &res1);
			if((state != RES_COMMAND_OK) &&
				(state != RES_TUPLES_OK)  &&
				(state != RES_COPY_IN)    &&
				(state != RES_COPY_OUT))
			{
				result_clean(sink, &res1);
				strcpy(record->srcuseralias, "");
			}
			else
			{
				row1 = result_rn(sink,&res1);
				if(row1 == 0)
				{
					result_clean(sink, &res1);
					strcpy(record->srcuseralias, "");
				}
				else
				{
					strcpy(record->srcuseralias, result_get(sink, &res1,0,0));
					result_clean(sink, &res1);
				}
			}
		}
		else
		{
			strcpy(record->srcuseralias, "");
		}
		head = slist_append(head, record);
	}
	result_clean(sink, &res);

	*list = head;
	return row;
}

//直播重点/明日预测(类型：1-文字直播；2-直播重点；3-明日预测（已关注的用户可查看）；4-观点；)
int get_TextRoomPoint_list_DB(Sink* sink, SList** list, CMDTextRoomLiveListReq_t2 *listreq)
{
	Result res;
	SList* head=0;
	int row,state,r;
	char query[512];

	char sztime[128]={0};
	time_t tnow=time(0);
	toStringTimestamp4(tnow, sztime);
	long int todaytime=atol(sztime);

	if(listreq->teacherid != listreq->userid)
	{
		sprintf(query, "select count(1) from tb_teacherfans where teacherid=%d and userid=%d ",
				listreq->teacherid, listreq->userid);
		row =0;
		res.priv =0;
		sink_exec2(sink, query, &res);
		const char* szVal;

		state =result_state(sink, &res);
		if((state != RES_COMMAND_OK) &&
			(state != RES_TUPLES_OK)  &&
			(state != RES_COPY_IN)    &&
			(state != RES_COPY_OUT))
		{
			result_clean(sink, &res);
			return 0;
		}
		row=result_rn(sink,&res);
		if(row == 0)
		{
			result_clean(sink, &res);
			return 0;
		}
	    szVal = result_get(sink, &res, 0, 0);
	    int count = atoi(szVal);
	    result_clean(sink, &res);

	    if(count<=0)
	    {
	    	return 0;
	    }
	}

	if(listreq->type==2)
	{
		if(listreq->messageid>0)
		{
			sprintf(query, "select teacherid,textliveid,textlivetype,ccontent,dtime,czans,commentstype  \
					from tb_textlivemessagehistory \
					where ddate=%ld and textliveid<%lld and keypoint=1 and teacherid=%d \
					order by textliveid DESC LIMIT %d ", todaytime, listreq->messageid, listreq->teacherid, listreq->count);
		}
		else
		{
			sprintf(query, "select teacherid,textliveid,textlivetype,ccontent,dtime,czans,commentstype  \
					from tb_textlivemessagehistory \
					where ddate=%ld and keypoint=1 and teacherid=%d \
					order by textliveid DESC LIMIT %d ", todaytime, listreq->teacherid, listreq->count);
		}
	}
	else if(listreq->type==3)
	{
		if(listreq->messageid>0)
		{
			sprintf(query, "select teacherid,textliveid,textlivetype,ccontent,dtime,czans,commentstype  \
					from tb_textlivemessagehistory \
					where ddate=%ld and textliveid<%lld and forcast=1 and teacherid=%d \
					order by textliveid DESC LIMIT %d ", todaytime, listreq->messageid, listreq->teacherid, listreq->count);
		}
		else
		{
			sprintf(query, "select teacherid,textliveid,textlivetype,ccontent,dtime,czans,commentstype \
					from tb_textlivemessagehistory \
					where ddate=%ld and forcast=1 and teacherid=%d \
					order by textliveid DESC LIMIT %d ", todaytime, listreq->teacherid, listreq->count);
		}
	}

	row =0;
	res.priv =0;
	sink_exec2(sink, query, &res);

	state =result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}
	row=result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	for(r=0; r<row; r++)
	{
		CMDTextRoomLivePointNoty_t2* record=(CMDTextRoomLivePointNoty_t2*)malloc(sizeof(CMDTextRoomLivePointNoty_t2));
		memset(record, 0, sizeof(CMDTextRoomLivePointNoty_t2));

		record->vcbid = listreq->vcbid;
		record->userid = listreq->userid;
		record->type = listreq->type;
		record->teacherid = atoi(result_get(sink, &res, r, 0));
		record->messageid = atoll(result_get(sink, &res, r, 1));
		record->livetype = atoi(result_get(sink, &res, r, 2));
		strncpy(record->content, result_get(sink, &res,r,3),sizeof(record->content));
		record->messagetime = atoll(result_get(sink, &res, r, 4));
		record->zans = atoll(result_get(sink, &res, r, 5));
		record->commentstype = atoi(result_get(sink, &res, r, 6));
		head = slist_append(head, record);
	}
	result_clean(sink, &res);

	*list = head;
	return row;
}

//文字直播记录
int64 insert_textlivemessage_DB(Sink* sink, CMDTextRoomLiveMessageReq_t2 *messageinfo)
{
	Result res;
	int row, state;
	char query[3072] = {0};
	const char* szVal;
	int64 textliveid=0;

	time_t tnow=time(0);
	int64 nowtime=0;
	char sztime[128]={0};
	toStringTimestamp3(tnow, sztime);
	nowtime=atoll(sztime);
	messageinfo->messagetime=nowtime;

	sprintf(query, " select nextval('tb_textlivemessagehistory_textliveid_seq') ");

	row = 0;
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
	    (state != RES_TUPLES_OK)  &&
	    (state != RES_COPY_IN)    &&
	    (state != RES_COPY_OUT))
	{
	    result_clean(sink, &res);
	    return -1;
	}

	row = result_rn(sink,&res);
	if(row == 0)
	{
	    result_clean(sink, &res);
	    return -1;
	}

	szVal = result_get(sink, &res, 0, 0);
	textliveid = atoll(szVal);
	result_clean(sink, &res);

        memset(query, 0, 3072);
	sprintf(query, "insert into tb_textlivemessagehistory(textliveid,textlivetype,teacherid,dtime,ddate,ccontent,keypoint,forcast,commentstype) \
			values (%lld,%d,%d,%lld,%ld,'%s',%d,%d,%d)",
			textliveid, messageinfo->livetype, messageinfo->teacherid, nowtime,(nowtime)/1000000,
			messageinfo->content,messageinfo->pointflag,messageinfo->forecastflag,messageinfo->commentstype);
	int result = exec_query(sink, query);
	if(result<0)
		return -1;
	else
		return textliveid;
}

//新增讲师观点类型分类
long int insert_viewtype_DB(Sink* sink, CMDTextRoomViewTypeReq_t2 *info)
{
	Result res;
	int row, state;
	char query[256];
	const char* szVal;
	long int viewtypeid=0;

	sprintf(query, " select nextval('tb_teacherviewtype_viewtypeid_seq') ");

	row = 0;
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
	    (state != RES_TUPLES_OK)  &&
	    (state != RES_COPY_IN)    &&
	    (state != RES_COPY_OUT))
	{
	    result_clean(sink, &res);
	    return -1;
	}

	row = result_rn(sink,&res);
	if(row == 0)
	{
	    result_clean(sink, &res);
	    return -1;
	}

	szVal = result_get(sink, &res, 0, 0);
	viewtypeid = atol(szVal);
	result_clean(sink, &res);

	sprintf(query, "insert into tb_teacherviewtype(viewtypeid,teacherid,viewtypename) \
			values (%ld,%d,'%s')",
			viewtypeid, info->teacherid, info->viewtypename);
	int result = exec_query(sink, query);
	if(result<0)
		return -1;
	else
		return viewtypeid;
}

//修改讲师观点类型分类
int update_viewtype_DB(Sink* sink, CMDTextRoomViewTypeReq_t2 *info)
{
	char query[256];
	sprintf(query, " update tb_teacherviewtype set viewtypename='%s' where teacherid=%d and viewtypeid=%d ",
			info->viewtypename, info->teacherid, info->viewtypeid);
	return exec_query(sink, query);
}

//删除讲师观点类型分类
int delete_viewtype_DB(Sink* sink, CMDTextRoomViewTypeReq_t2 *info)
{
	Result res;
	int row, state;
	char query[256];

	sprintf(query, " select 1 from tb_viewhistory where viewtypeid=%lld and teacherid=%d ",
			info->viewtypeid, info->teacherid);

	row = 0;
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
	    (state != RES_TUPLES_OK)  &&
	    (state != RES_COPY_IN)    &&
	    (state != RES_COPY_OUT))
	{
	    result_clean(sink, &res);
	    return -1;
	}

	row = result_rn(sink,&res);
	if(row > 0)
	{
		result_clean(sink, &res);
		return -10137;//错误码：删除观点分类之前需要先删掉分类下所有观点
	}
	result_clean(sink, &res);

	sprintf(query, " delete from tb_teacherviewtype where teacherid=%d and viewtypeid=%lld ",
			info->teacherid,info->viewtypeid);
	return exec_query(sink, query);
}

int get_userinfo_DB(Sink* sink, CMDTextRoomUserInfo_t2 *info)
{
	Result res;
	int row, state;
	char query[256];
	const char* szVal;

	if(info->teacherfalg==0)
	{
		sprintf(query, " select calias,nheadid from dks_user where nuserid=%d limit 1 ",info->userid);
	}
	else
	{
		sprintf(query, " select u.calias,t.headid from dks_user u,dks_teacherinfo t where u.nuserid=%d and u.nuserid=t.teacherid limit 1",info->userid);
	}

	row = 0;
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
	    (state != RES_TUPLES_OK)  &&
	    (state != RES_COPY_IN)    &&
	    (state != RES_COPY_OUT))
	{
	    result_clean(sink, &res);
	    return -1;
	}

	row = result_rn(sink,&res);
	if(row > 0)
	{
		szVal = result_get(sink, &res, 0, 0);
		strcpy(info->alias,szVal);
		szVal = result_get(sink, &res, 0, 1);
		info->headid=atoi(szVal);
		result_clean(sink, &res);
	}
	result_clean(sink, &res);

	return row;
}

int view_dianzan_DB(Sink* sink,long long viewid)
{
    char query[256] = {0};
    sprintf(query, "update tb_viewhistory set czans=czans+1 where viewid=%lld",viewid);
    exec_query(sink, query);
    return 0;
}

int func_getviewdianzan_DB(Sink* sink,long long viewid)
{
    Result res;
  	int row, state;
  	char query[256]={0};
  	const char *czvalue;
      int count;
  	
  	sprintf(query, "select czans from tb_viewhistory where viewid=%lld",viewid);
  	res.priv = 0;
  	sink_exec2(sink, query, &res);
  	state = result_state (sink, &res);
  	if((state != RES_COMMAND_OK) &&
  		(state != RES_TUPLES_OK)  &&
  		(state != RES_COPY_IN)  &&
  		(state != RES_COPY_OUT))
  	{
  		result_clean(sink, &res);
  		return -1;
  	}
										
	row = result_rn(sink, &res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return -1;
	}
	else
	{
	    czvalue = result_get(sink, &res, 0, 0);
	    count = atoi(czvalue);
	    result_clean(sink, &res);
        return count;
	}
	
}

int view_sendflower_DB(Sink* sink,long long viewid,int count)
{
    char query[256] = {0};
    sprintf(query, "update tb_viewhistory set cflowers=cflowers+%d where viewid=%lld",count,viewid);
    exec_query(sink, query);
    return 0;
}

int func_getviewflower_DB(Sink* sink,long long viewid)
{
    Result res;
  	int row, state;
  	char query[256]={0};
  	const char *czvalue;
      int count;
  	
  	sprintf(query, "select cflowers from tb_viewhistory where viewid=%lld",viewid);
  	res.priv = 0;
  	sink_exec2(sink, query, &res);
  	state = result_state (sink, &res);
  	if((state != RES_COMMAND_OK) &&
  		(state != RES_TUPLES_OK)  &&
  		(state != RES_COPY_IN)  &&
  		(state != RES_COPY_OUT))
  	{
  		result_clean(sink, &res);
  		return -1;
  	}
										
	row = result_rn(sink, &res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return -1;
	}
	else
	{
	    czvalue = result_get(sink, &res, 0, 0);
	    count = atoi(czvalue);
	    result_clean(sink, &res);
        return count;
	}
	
}


int view_comment_DB(Sink* sink,long long viewid,int fromid,int toid,long long reqtime,char* ccomments,long long srcinteractid,int reqcommentstype,long long* commentid)
{
    Result res;
  	int row, state;
  	char query[2048]={0};
  	const char *czvalue;
	if ( !ccomments )
	{
        return -1;
	}
    //查出当前对应的message id
   	{
        sprintf(query, "select nextval('tb_viewcomments_commentid_seq')");
		res.priv = 0;
	    sink_exec2(sink, query, &res);
		state = result_state (sink, &res);
        if((state != RES_COMMAND_OK) &&
        	(state != RES_TUPLES_OK)  &&
        	(state != RES_COPY_IN)  &&
        	(state != RES_COPY_OUT))
        {
        	result_clean(sink, &res);
        	return -1;
        }
		row = result_rn(sink, &res);
        if(row == 0)
        {
        	result_clean(sink, &res);
        	return -1;
        }
		else
		{
        	czvalue = result_get(sink, &res,0,0);
        	*commentid = atoll(czvalue);
		}
   	}
	
    sprintf(query, "insert into tb_viewcomments(commentid,viewid,fromid,toid,reqtime,ccomments,srcinteractid,commentstype) values(%lld,%lld,%d,%d,%lld,'%s',%lld,%d)",*commentid,viewid,fromid,toid,reqtime,ccomments,srcinteractid,reqcommentstype);
    exec_query(sink, query);
    return 0;
}

int func_getlivehistorylist_DB(Sink* sink,long long datetime/*从哪个年月日开始取*/,int fromIndex,int bInc/*是否升序*/,int count/*取多少条记录*/,int teacherid,int vcbid,SList** list)
{
    Result res,res2;
	SList *head=0;
	int row, state, r,row2;
	char query[256]={0};
	const char *czvalue;

    //获取记录条数
	sprintf(query, "select distinct(ddate) from tb_textlivemessagehistory where ddate%s%lld and teacherid=%d order by ddate %s limit %d offset %d",bInc?">=":"<=",datetime,teacherid,bInc?"asc":"desc",count,fromIndex);
	res.priv = 0;
	res2.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state (sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)  &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	row = result_rn(sink, &res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}
	else
	{
        for(r=0; r<row; r++)
    	{
    	    //初始化
    	    TextLiveItem_t* record = (TextLiveItem_t*)malloc(sizeof(TextLiveItem_t));
			memset(record, 0, sizeof(TextLiveItem_t));
			
    	    czvalue = result_get(sink, &res, r, 0);
    		long date = atoi(czvalue);
			record->datetime = date;
    		
	        //对应每个日期返回直播记录总数，以及当天的最早跟最晚的记录 当天0:00~23:59
            sprintf(query,"select count(textliveid),min(dtime),max(dtime) from tb_textlivemessagehistory where ddate=%lld and teacherid=%d",datetime,teacherid);
            sink_exec2(sink, query, &res2);
            state = result_state (sink, &res2);
        	if((state != RES_COMMAND_OK) &&
        		(state != RES_TUPLES_OK)  &&
        		(state != RES_COPY_IN)  &&
        		(state != RES_COPY_OUT))
        	{
        		result_clean(sink, &res2);
        		return -1;
        	}
			row2 = result_rn(sink, &res2);
        	if(row2 == 0)
        	{
        		result_clean(sink, &res2);
        		return 0;
        	}
			else
			{
			    czvalue = result_get(sink, &res2,0,0);
        		record->totalCount = atoi(czvalue);
        		czvalue = result_get(sink, &res2,0,1);
        		record->beginTime = atoll(czvalue);
        		czvalue = result_get(sink, &res2, 0,2);
        		record->endTime = atoll(czvalue);
        		result_clean(sink, &res2);
			}
    		
            //当天总人气
        	long timeRecord2 = record->datetime;
            sprintf(query,"select count(logid) from tb_textliveroomuserlog where dtime>=%lld and dtime<=%lld and roomid=%d",timeRecord2*1000000+0,timeRecord2*1000000+235959,vcbid);
            sink_exec2(sink, query, &res2);
            state = result_state (sink, &res2);
        	if((state != RES_COMMAND_OK) &&
        		(state != RES_TUPLES_OK)  &&
        		(state != RES_COPY_IN)  &&
        		(state != RES_COPY_OUT))
        	{
        		result_clean(sink, &res2);
        		return -1;
        	}
			row2 = result_rn(sink, &res2);
        	if(row2 == 0)
        	{
        		result_clean(sink, &res2);
        		return 0;
        	}
			else
			{
        		czvalue = result_get(sink, &res2,0,0);
        		record->renQi = atoi(czvalue);
        		result_clean(sink, &res2);
			}

			//当天问答总数
            sprintf(query,"select count(answerid) from tb_answerReq where ndatetime>=%lld and ndatetime<=%lld and userid=%d",timeRecord2*1000000+0,timeRecord2*1000000+235959,teacherid);
            sink_exec2(sink, query, &res2);
            state = result_state (sink, &res2);
        	if((state != RES_COMMAND_OK) &&
        		(state != RES_TUPLES_OK)  &&
        		(state != RES_COPY_IN)  &&
        		(state != RES_COPY_OUT))
        	{
        		result_clean(sink, &res2);
        		return -1;
        	}
			row2 = result_rn(sink, &res2);
        	if(row2 == 0)
        	{
        		result_clean(sink, &res2);
        		return 0;
        	}
			else
			{
        		czvalue = result_get(sink, &res2,0,0);
        		record->cAnswer = atoi(czvalue);
        		result_clean(sink, &res2);
			}

    		head = slist_append(head, record);
    	}
    
    	result_clean(sink, &res);
    	*list = head;
    	return row;
	}	
}

int func_userquestion_DB(Sink* sink,int userid,int teacherid,long long reqtime,char* question,char* stokeid,unsigned char questiontype,long long* questionid)
{
    Result res;
  	int row, state;
	const char *czvalue;
    char query[512] = {0};
	if ( !stokeid || !question )
	{
        return -1;
	}

    //查出当前对应的message id
   	{
        sprintf(query, "select nextval('tb_questionReq_questionid_seq')");
		res.priv = 0;
	    sink_exec2(sink, query, &res);
		state = result_state (sink, &res);
        if((state != RES_COMMAND_OK) &&
        	(state != RES_TUPLES_OK)  &&
        	(state != RES_COPY_IN)  &&
        	(state != RES_COPY_OUT))
        {
        	result_clean(sink, &res);
        	return -1;
        }
		row = result_rn(sink, &res);
        if(row == 0)
        {
        	result_clean(sink, &res);
        	return -1;
        }
		else
		{
        	czvalue = result_get(sink, &res,0,0);
        	*questionid = atoll(czvalue);
		}
   	}
	
    sprintf(query, "insert into tb_questionReq(questionid,questionstype,userid,teacherid,reqtime,question,stokeid) values(%lld,%d,%d,%d,%lld,'%s','%s')",*questionid,questiontype,userid,teacherid,reqtime,question,stokeid);
    exec_query(sink, query);
    return 0;
}

int view_modify_view_DB(Sink* sink,long long* viewidptr/*传0代表是新增的观点*/,int viewtypeid,int teacherid,long long datetime,char* title,char* content)
{
    
	Result res;
  	int row, state;
  	char query[256]={0};
	long long newviewid = 0;
  	const char *czvalue;
	unsigned char bNew = 0;
	if ( !title || !content )
	{
        return -1;
	}

	//如果传的是0
	if ( 0 == *viewidptr )
   	{
        sprintf(query, "select nextval('tb_viewhistory_viewid_seq')");
		res.priv = 0;
	    sink_exec2(sink, query, &res);
		state = result_state (sink, &res);
        if((state != RES_COMMAND_OK) &&
        	(state != RES_TUPLES_OK)  &&
        	(state != RES_COPY_IN)  &&
        	(state != RES_COPY_OUT))
        {
        	result_clean(sink, &res);
        	return -1;
        }
		row = result_rn(sink, &res);
        if(row == 0)
        {
        	result_clean(sink, &res);
        	return 0;
        }
		else
		{
        	czvalue = result_get(sink, &res,0,0);
        	newviewid = atoll(czvalue);
			*viewidptr = newviewid;
			bNew = 1;
		}
   	}

	//插入观点表
	//新增
    if ( bNew )
    {
        sprintf(query, "insert into tb_viewhistory	(viewid,viewtypeid,teacherid,dtime,title,ccontent) values(%lld,%d,%d,%lld,'%s','%s')",newviewid,viewtypeid,teacherid,datetime,title,content);
    }//修改
	else
	{
        sprintf(query, "update tb_viewhistory set dtime=%lld,ccontent='%s',title='%s' where viewid=%lld",datetime,content,title,*viewidptr);
	}
    exec_query(sink, query);
    
	//给直播表新增一条记录
	sprintf(query, "insert into  tb_textlivemessagehistory	(textlivetype,interactid,teacherid,dtime,ddate,ccontent) values(%d,%lld,%d,%lld,%lld,'%s')",5,newviewid,teacherid,datetime,datetime/1000000,title);
	exec_query(sink, query);

	//给观点类型表总数加1
	sprintf(query, "update  tb_teacherviewtype set count=count+1 where viewtypeid=%d",viewtypeid);
	exec_query(sink, query);
    return 0;
}

/*
 * Comments:Get text room live viewpoint type info from db
 * Param IN:sink
 * Param IN:teacherID
 * Param IN/OUT:viewpoint type info
 * Return:int, 0 means success, others mean failed.
 */
int get_textroomlivegroup_DB(Sink* sink, SList** list, unsigned int teacherID)
{
    Result res;
    SList * head = NULL;
    int row, state, r;
    char query[256] = {0};
    const char* czvalue;

    sprintf(query, "select a.roomid,c.viewtypeid,c.viewtypename,c.count from dks_teacherinfo a, tb_teacherviewtype c where a.teacherid=c.teacherid and a.teacherid=%u", teacherID);
    row = 0;
    res.priv = 0;
    sink_exec2(sink, query, &res);
    state = result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK) &&
        (state != RES_COPY_IN) &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return -1;
    }

    row = result_rn(sink, &res);
    if(row == 0)
    {
        result_clean(sink, &res);
        return 0;
    }

    for(r = 0; r < row; ++r)
    {
        DDLTeacherViewType_t * record = (DDLTeacherViewType_t *)malloc(sizeof(DDLTeacherViewType_t));
        record->teacherid = teacherID;
        czvalue = result_get(sink, &res, r, 0);
        if (NULL != czvalue)
        {
            record->roomid = atoi(czvalue);
        }
        
        czvalue = result_get(sink, &res, r, 1);
        if (NULL != czvalue)
        {
            record->viewpointType = atoi(czvalue);
        }
        
        czvalue = result_get(sink, &res, r, 2);
        if (NULL != czvalue)
        {
        	int len = strlen(czvalue);
            strncpy(record->viewTypeName, czvalue, len);
            record->viewTypeName[len] = '\0';
        }

        czvalue = result_get(sink, &res, r, 3);
        if (NULL != czvalue)
        {
            record->totalCount = atoi(czvalue);
        }

        head = slist_append(head, record);
    }

    result_clean(sink, &res);
    *list = head;
    return 0;
}

/*
 * Comments:Get text room live viewpoint type info from db(partial show content)该查询即支持跳页，也支持滚动查询
 * Param IN:sink, db connection
 * Param IN:int, teacherID
 * Param IN:int, viewType
 * Param IN:int, startPos,查询结果里的起始编号（表示从第startPos个开始返回，从0开始）
 * Param IN:int, messageid,viewid
 * Param IN:int, lstCount,一次查询的个数
 * Param IN/OUT:list, viewpoint list info.
 * Return:int, -1 means failed, others mean row count.
 */
int get_textroomViewPointLst_DB(Sink* sink, SList** list, unsigned int teacherID, int viewType, long long startPos, long long messageid, int lstCount)
{
    Result res;
    SList * head = NULL;
    int row, state, r;
    char query[512] = {0};
    const char* czvalue;

    //int partialLen = 50;

    char viewTypeCon[50] = {0};
    if (0 != viewType)
    {
        sprintf(viewTypeCon, " and viewtypeid=%d ", viewType);
    } 
    
    if (messageid != 0)
    {
        if (startPos == 0)
        {
            //说明messageid是上次返回包里的最小messageid.
            sprintf(query, "select viewid,title,ccontent,dtime,czans,ccomments,cflowers,reads,viewtypeid,commentstype from tb_viewhistory where teacherid=%u and viewid<%lld %s order by viewid desc limit %d offset %lld",\
                teacherID, messageid, viewTypeCon, lstCount, startPos);
        }
        else
        {
            //说明messageid是第一次返回包里的最大messageid.
            sprintf(query, "select viewid,title,ccontent,dtime,czans,ccomments,cflowers,reads,viewtypeid,commentstype from tb_viewhistory where teacherid=%u and viewid<=%lld %s order by viewid desc limit %d offset %lld",\
                teacherID, messageid, viewTypeCon, lstCount, startPos);
        }
    }
    else
    {
        //说明这是第一次查询，所以messageid和startPos都是0.
        sprintf(query, "select viewid,title,ccontent,dtime,czans,ccomments,cflowers,reads,viewtypeid,commentstype from tb_viewhistory where teacherid=%u %s order by viewid desc limit %d offset %lld",\
            teacherID, viewTypeCon, lstCount, startPos);
    }
    
    row = 0;
    res.priv = 0;
    sink_exec2(sink, query, &res);
    state = result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK) &&
        (state != RES_COPY_IN) &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return -1;
    }

    row = result_rn(sink, &res);
    if(row == 0)
    {
        result_clean(sink, &res);
        return 0;
    }

    for(r = 0; r < row; ++r)
    {
        DDLViewPointHisItem_t * record = (DDLViewPointHisItem_t *)malloc(sizeof(DDLViewPointHisItem_t));
        record->teacherID = teacherID;
        record->viewPointType = viewType;

        czvalue = result_get(sink, &res, r, 0);
        if (NULL != czvalue)
        {
            record->viewID = atoll(czvalue);
        }

        czvalue = result_get(sink, &res, r, 1);
        if (NULL != czvalue)
        {
        	int len = strlen(czvalue);
            strncpy(record->title, czvalue, len);
            record->title[len] = '\0';
        }

        czvalue = result_get(sink, &res, r, 2);
        if (NULL != czvalue)
        {
            int partialLen = strlen(czvalue);
            strncpy(record->content, czvalue, partialLen);
            record->content[partialLen] = '\0';
        }

        czvalue = result_get(sink, &res, r, 3);
        if (NULL != czvalue)
        {
            record->timeStamp = atol(czvalue);
        }

        czvalue = result_get(sink, &res, r, 4);
        if (NULL != czvalue)
        {
            record->cLikes = atoi(czvalue);
        }
        
        czvalue = result_get(sink, &res, r, 5);
        if (NULL != czvalue)
        {
            record->cComments = atoi(czvalue);
        }

        czvalue = result_get(sink, &res, r, 6);
        if (NULL != czvalue)
        {
            record->cFlowers = atoi(czvalue);
        }

        czvalue = result_get(sink, &res, r, 7);
        if (NULL != czvalue)
        {
            record->cReads = atoi(czvalue);
        }

        czvalue = result_get(sink, &res, r, 8);
        if (NULL != czvalue)
        {
            record->viewPointType = atoi(czvalue);
        }

        czvalue = result_get(sink, &res, r, 9);
        if (NULL != czvalue)
        {
            record->commentstype = atoi(czvalue);
        }
        head = slist_append(head, record);
    }

    result_clean(sink, &res);
    *list = head;
    return row;
}

/*
 * Comments:like on text live
 * Param IN:sink, db connection
 * Param IN:int, messageID, text live ID
 * Return:int, -1 means failed, 0 means success.
 */
int update_likesOnTextLive_DB(Sink* sink, long long messageID)
{
    int ret = 0;
    Result res;
    char query[256] = {0};
    sprintf(query, "update tb_textlivemessagehistory set czans=czans+1 where textliveid=%lld", messageID);
    res.priv = 0;
    sink_exec2(sink, query, &res);
    int state = result_state (sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK)  &&
        (state != RES_COPY_IN)  &&
        (state != RES_COPY_OUT))
    {
        ret = -1;
    }

    result_clean(sink, &res);
    return ret;
}

/*
 * Comments:add one fans on teacher
 * Param IN:sink, db connection
 * Param IN:teacherID,int
 * Param IN:fansID,int
 * Return:int, -1 means failed, 0 means success.
 */
int insert_teacherfans_DB(Sink* sink, unsigned int teacherID, unsigned int fansID)
{
    char cSQL[256] = {0};
    sprintf(cSQL, "insert into tb_teacherfans(teacherid, userid) values(%u,%u)", teacherID, fansID);
    return exec_query(sink, cSQL);
}

/*
 * Comments:delete the fan of teacher
 * Param IN:sink, db connection
 * Param IN:teacherID,int
 * Param IN:fansID,int
 * Return:int, -1 means failed, 0 means success.
 */
int delete_teacherfans_DB(Sink* sink, unsigned int teacherID, unsigned int fansID)
{
    char cSQL[256] = {0};
    sprintf(cSQL, "delete from tb_teacherfans where teacherid=%u and userid=%u", teacherID, fansID);
    return exec_query(sink, cSQL);
}

/*
 * Comments:check if the user is the fans of teacher
 * Param IN:sink, db connection
 * Param IN:teacherID,int
 * Param IN:fansID,int
 * Return:int, -1 means failed, 0 means success.
 */
int check_teacherfans_exist_DB(Sink* sink, unsigned int teacherID, unsigned int fansID)
{
    char cSQL[256] = {0};
    sprintf(cSQL, "select 1 from tb_teacherfans where teacherid=%u and userid=%u", teacherID, fansID);
    Result res;
    res.priv = 0;
    sink_exec2(sink, cSQL, &res);
    int state = result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK) &&
        (state != RES_COPY_IN) &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return -1;
    }

    int row = result_rn(sink, &res);
    result_clean(sink, &res);
    if(0 == row)
    {
        return 0;
    }else
    {
        return 1;   
    }
}

/*
 * Comments:Get fans count of teacherID from db
 * Param IN:sink
 * Param IN:teacherID
 * Return:int, 0 means result, -1 mean failed.
 */
long get_teacherfansCount_DB(Sink* sink, unsigned int teacherID)
{
    Result res;
    int row, state;
    char query[256] = {0};
    const char* czvalue;

    sprintf(query, "select count(userid) from tb_teacherfans where teacherid=%u", teacherID);
    row = 0;
    res.priv = 0;
    sink_exec2(sink, query, &res);
    state = result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK) &&
        (state != RES_COPY_IN) &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return -1;
    }

    row = result_rn(sink, &res);
    if(row == 0)
    {
        result_clean(sink, &res);
        return 0;
    }

    czvalue = result_get(sink, &res, 0, 0);
    if (NULL != czvalue)
    {
        return atol(czvalue);
    }

    return 0;
}

/*
 * Comments:query fans info using taecherid,该查询即支持跳页，也支持滚动查询
 * Param IN:sink
 * Param IN:int, startPos,查询结果里的起始编号（表示从第startPos个开始返回，从0开始）
 * Param IN:int, messageid,userid
 * Param IN:int, lstCount,一次查询的个数
 * Param IN/OUT:records, db query information
 * Return:int, 0 means result, -1 mean failed, others mean row number
 */
int get_fansInfoByTeacherID_DB(Sink* sink, SList** list, unsigned int teacherid, unsigned int startPos, long long messageid, unsigned int count)
{
    Result res;
    int row, state, r;
    char cond[256] = {0};
    char querySQL[512] = {0};
    SList * head = NULL;

    if (messageid != 0)
    {
        if (startPos == 0)
        {
            //说明messageid是上次返回包里的最小messageid.
            sprintf(cond, "select userid from tb_teacherfans where teacherid=%u and userid<%lld order by userid desc limit %u offset %u", teacherid, messageid, count, startPos);
        }
        else
        {
            //说明messageid是第一次返回包里的最大messageid.
            sprintf(cond, "select userid from tb_teacherfans where teacherid=%u and userid<=%lld order by userid desc limit %u offset %u", teacherid, messageid, count, startPos);
        }
    }
    else
    {
        //说明这是第一次查询，所以messageid和startPos都是0.
        sprintf(cond, "select userid from tb_teacherfans where teacherid=%u order by userid desc limit %u offset %u", teacherid, count, startPos);
    }

    sprintf(querySQL, "select nuserid,calias,nheadid from dks_user where nuserid in (%s)", cond);

    const char* czvalue = NULL;
    row = 0;
    res.priv = 0;
    sink_exec2(sink, querySQL, &res);
    state = result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK) &&
        (state != RES_COPY_IN) &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return -1;
    }

    row = result_rn(sink, &res);
    if(row == 0)
    {
        result_clean(sink, &res);
        return 0;
    }

    for(r = 0; r < row; ++r)
    {
        CMDTextRoomUserInfo_t2 * pUser = (CMDTextRoomUserInfo_t2 *)malloc(sizeof(CMDTextRoomUserInfo_t2));
        memset(pUser, 0, sizeof(CMDTextRoomUserInfo_t2));
        czvalue = result_get(sink, &res, r, 0);
        if (NULL != czvalue)
        {
            pUser->userid = atol(czvalue);
        }
        
        czvalue = result_get(sink, &res, r, 1);
        if (NULL != czvalue)
        {
            strncpy(pUser->alias, czvalue, strlen(czvalue));
        }

        czvalue = result_get(sink, &res, r, 2);
        if (NULL != czvalue)
        {
            pUser->headid = atoi(czvalue);
        }

        head = slist_append(head, pUser);
    }

    result_clean(sink, &res);
    *list = head;

    return row;
}

/*
 * Comments:query teachers info using fansID,该查询即支持跳页，也支持滚动查询
 * Param IN:sink
 * Param IN:int, startPos,查询结果里的起始编号（表示从第startPos个开始返回，从0开始）
 * Param IN:int, messageid,teacherid
 * Param IN:int, lstCount,一次查询的个数
 * Param IN/OUT:records, db query information
 * Return:int, 0 means result, -1 mean failed, others mean row number
 */
int get_teacherInfoByfansID_DB(Sink* sink, SList** records, unsigned int userid, unsigned int startPos, long long messageid, unsigned int count)
{
    Result res;
    int row, state, r;
    char selectTabs[] = "select a.teacherid,c.calias,b.headid,b.clabel,b.introduce,b.clevel from tb_teacherfans a,dks_teacherinfo b,dks_user c";
    char querySQL[512] = {0};

    if (messageid != 0)
    {
        if (startPos == 0)
        {
            //说明messageid是上次返回包里的最小messageid.
            sprintf(querySQL, "%s where a.userid=%u and a.teacherid=b.teacherid and b.teacherid=c.nuserid and b.teacherid<%lld order by a.teacherid desc limit %u offset %u", \
                selectTabs, userid, messageid, count, startPos);
        }
        else
        {
            //说明messageid是第一次返回包里的最大messageid.
            sprintf(querySQL, "%s where a.userid=%u and a.teacherid=b.teacherid and b.teacherid=c.nuserid and b.teacherid<=%lld order by a.teacherid desc limit %u offset %u", \
                selectTabs, userid, messageid, count, startPos);
        }
    }
    else
    {
        //说明这是第一次查询，所以messageid和startPos都是0.
        sprintf(querySQL, "%s where a.userid=%u and a.teacherid=b.teacherid and b.teacherid=c.nuserid order by a.teacherid desc limit %u offset %u", selectTabs, userid, count, startPos);
    }

    const char* czvalue = NULL;
    row = 0;
    res.priv = 0;
    sink_exec2(sink, querySQL, &res);
    state = result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK) &&
        (state != RES_COPY_IN) &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return -1;
    }

    row = result_rn(sink, &res);
    if(row == 0)
    {
        result_clean(sink, &res);
        return 0;
    }

    SList * head = NULL;
    for(r = 0; r < row; ++r)
    {
        //a.teacherid,c.calias,b.headid,b.clabel,b.introduce,b.clevel
        CMDTextRoomTeacherNoty_t2 * pItem = (CMDTextRoomTeacherNoty_t2 *)malloc(sizeof(CMDTextRoomTeacherNoty_t2));
        memset(pItem, 0, sizeof(CMDTextRoomTeacherNoty_t2));
        czvalue = result_get(sink, &res, r, 0);
        if (NULL != czvalue)
        {
            pItem->teacherid = atol(czvalue);
        }

        czvalue = result_get(sink, &res, r, 1);
        if (NULL != czvalue)
        {
            strncpy(pItem->teacheralias, czvalue, strlen(czvalue));
        }

        czvalue = result_get(sink, &res, r, 2);
        if (NULL != czvalue)
        {
            pItem->headid = atoi(czvalue);
        }

        czvalue = result_get(sink, &res, r, 3);
        if (NULL != czvalue)
        {
            strncpy(pItem->clabel, czvalue, strlen(czvalue));
        }

        czvalue = result_get(sink, &res, r, 4);
        if (NULL != czvalue)
        {
            strncpy(pItem->cintroduce, czvalue, strlen(czvalue));
        }

        czvalue = result_get(sink, &res, r, 5);
        if (NULL != czvalue)
        {
            strncpy(pItem->clevel, czvalue, strlen(czvalue));
        }
        
        head = slist_append(head, pItem);
    }

    result_clean(sink, &res);
    *records = head;

    return row;
}

/*
 * Comments:query teachers info,该查询即支持跳页，也支持滚动查询
 * Param IN:sink
 * Param IN:int, startPos,查询结果里的起始编号（表示从第startPos个开始返回，从0开始）
 * Param IN:int, messageid,teacherid
 * Param IN:int, lstCount,一次查询的个数
 * Param IN/OUT:records, db query information
 * Return:int, 0 means result, -1 mean failed, others mean row number
 */
int get_teacher_info_list_DB(Sink* sink, SList** records, unsigned int startPos, long long messageid, unsigned int count)
{
    Result res;
    int row, state, r;
    char selectColumn[] = "select info.teacherid,usrtab.calias,info.headid,info.clabel,info.goodat,tid.count,info.clevel";
    char condition[256] = {0};
    char querySQL[2048] = {0};

    if (messageid != 0)
    {
        if (startPos == 0)
        {
            //说明messageid是上次返回包里的最小messageid.
            sprintf(condition, "select teacherid from dks_teacherinfo where teacherid<%lld and stype!=2 order by teacherid desc limit %u offset %u", messageid, count, startPos);
        }
        else
        {
            //说明messageid是第一次返回包里的最大messageid.
            sprintf(condition, "select teacherid from dks_teacherinfo where teacherid<=%lld and stype!=2 order by teacherid desc limit %u offset %u", messageid, count, startPos);
        }
    }
    else
    {
        //说明这是第一次查询，所以messageid和startPos都是0.
        sprintf(condition, "select teacherid from dks_teacherinfo where stype!=2 order by teacherid desc limit %u offset %u", count, startPos);
    }

    sprintf(querySQL, "%s from (select a.teacherid,count(b.answerid) from (%s) as a left join tb_answerReq as b on a.teacherid=b.userid group by a.teacherid) as tid,\
                      dks_teacherinfo as info, dks_user as usrtab where tid.teacherid=info.teacherid and tid.teacherid=usrtab.nuserid and info.stype!=2 ", selectColumn, condition); 

    const char* czvalue = NULL;
    row = 0;
    res.priv = 0;
    sink_exec2(sink, querySQL, &res);
    state = result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK) &&
        (state != RES_COPY_IN) &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return -1;
    }

    row = result_rn(sink, &res);
    if(row == 0)
    {
        result_clean(sink, &res);
        return 0;
    }

    SList * head = NULL;
    for(r = 0; r < row; ++r)
    {
        //teacherid,calias,headid,clabel,goodat,count,clevel
        CMDTextRoomTeacherNoty_t2 * pItem = (CMDTextRoomTeacherNoty_t2 *)malloc(sizeof(CMDTextRoomTeacherNoty_t2));
        memset(pItem, 0, sizeof(CMDTextRoomTeacherNoty_t2));
        czvalue = result_get(sink, &res, r, 0);
        if (NULL != czvalue)
        {
            pItem->teacherid = atol(czvalue);
        }

        czvalue = result_get(sink, &res, r, 1);
        if (NULL != czvalue)
        {
            strncpy(pItem->teacheralias, czvalue, strlen(czvalue));
        }

        czvalue = result_get(sink, &res, r, 2);
        if (NULL != czvalue)
        {
            pItem->headid = atoi(czvalue);
        }

        czvalue = result_get(sink, &res, r, 3);
        if (NULL != czvalue)
        {
            strncpy(pItem->clabel, czvalue, strlen(czvalue));
        }

        czvalue = result_get(sink, &res, r, 4);
        if (NULL != czvalue)
        {
            strncpy(pItem->cgoodat, czvalue, strlen(czvalue));
        }

        czvalue = result_get(sink, &res, r, 5);
        if (NULL != czvalue)
        {
            pItem->anserCount = atol(czvalue);
        }

        czvalue = result_get(sink, &res, r, 6);
        if (NULL != czvalue)
        {
            strncpy(pItem->clevel, czvalue, strlen(czvalue));
        }

        head = slist_append(head, pItem);
    }

    result_clean(sink, &res);
    *records = head;

    return row;
}

/*
 * Comments:query forecast info list by fansID,该查询即支持跳页，也支持滚动查询
 * Param IN:sink
 * Param IN:int, startPos,查询结果里的起始编号（表示从第startPos个开始返回，从0开始）
 * Param IN:int, messageid,textliveid
 * Param IN:int, lstCount,一次查询的个数
 * Param IN/OUT:records, db query information
 * Return:int, 0 means result, -1 mean failed, others mean row number
 */
int get_forecast_info_list_DB(Sink* sink, SList** records, unsigned int userid, unsigned int startPos, long long messageid, unsigned int count)
{
    char condition[256] = {0};
    char querySQL[1024] = {0};
    
    if (messageid != 0)
    {
        if (startPos == 0)
        {
            //说明messageid是上次返回包里的最小messageid.
            sprintf(condition, "where b.forcast=1 and a.userid=%u and b.textliveid<%lld and a.teacherid=b.teacherid and a.teacherid=c.nuserid and a.teacherid=d.teacherid and d.stype!=2 order by textliveid desc limit %u offset %u", \
                userid, messageid, count, startPos);
        }
        else
        {
            //说明messageid是第一次返回包里的最大messageid.
            sprintf(condition, "where b.forcast=1 and a.userid=%u and b.textliveid<=%lld and a.teacherid=b.teacherid and a.teacherid=c.nuserid and a.teacherid=d.teacherid and d.stype!=2 order by textliveid desc limit %u offset %u", \
                userid, messageid, count, startPos);
        }
    }
    else
    {
        //说明这是第一次查询，所以messageid和startPos都是0.
        sprintf(condition, "where b.forcast=1 and a.userid=%u and a.teacherid=b.teacherid and a.teacherid=c.nuserid and a.teacherid=d.teacherid and d.stype!=2 order by textliveid desc limit %u offset %u", userid, count, startPos);
    }

    sprintf(querySQL, "select a.userid,a.teacherid,c.calias,d.headid,b.textliveid,b.textlivetype,b.ccontent,b.commentstype,b.dtime from tb_teacherfans a,tb_textlivemessagehistory b,dks_user c,dks_teacherinfo d %s", condition);
    
    Result res;
    int row, state, r;
    const char* czvalue = NULL;
    row = 0;
    res.priv = 0;
    sink_exec2(sink, querySQL, &res);
    state = result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK) &&
        (state != RES_COPY_IN) &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return -1;
    }

    row = result_rn(sink, &res);
    if(0 == row)
    {
        result_clean(sink, &res);
        return 0;
    }

    SList * head = NULL;
    for(r = 0; r < row; ++r)
    {
        //select a.userid,a.teacherid,c.calias,d.headid,b.textliveid,b.textlivetype,b.ccontent,b.commentstype,b.dtime 
        DDLTextLiveForecastItem_t * pItem = (DDLTextLiveForecastItem_t *)malloc(sizeof(DDLTextLiveForecastItem_t));
        memset(pItem, 0, sizeof(DDLTextLiveForecastItem_t));
        czvalue = result_get(sink, &res, r, 0);
        if (NULL != czvalue)
        {
            pItem->userid = atol(czvalue);
        }

        czvalue = result_get(sink, &res, r, 1);
        if (NULL != czvalue)
        {
            pItem->teacherid = atol(czvalue);
        }

        czvalue = result_get(sink, &res, r, 2);
        if (NULL != czvalue)
        {
            strncpy(pItem->teacheralias, czvalue, strlen(czvalue));
        }

        czvalue = result_get(sink, &res, r, 3);
        if (NULL != czvalue)
        {
            pItem->teacherheadid = atoi(czvalue);
        }

        czvalue = result_get(sink, &res, r, 4);
        if (NULL != czvalue)
        {
            pItem->messageid = atoll(czvalue);
        }

        czvalue = result_get(sink, &res, r, 5);
        if (NULL != czvalue)
        {
            pItem->livetype = atoi(czvalue);
        }

        czvalue = result_get(sink, &res, r, 6);
        if (NULL != czvalue)
        {
            strncpy(pItem->content, czvalue, strlen(czvalue));
        }

        czvalue = result_get(sink, &res, r, 7);
        if (NULL != czvalue)
        {
            pItem->commentstype = atoi(czvalue);
        }

        czvalue = result_get(sink, &res, r, 8);
        if (NULL != czvalue)
        {
            pItem->messagetime = atoll(czvalue);
        }

        head = slist_append(head, pItem);
    }
    
    result_clean(sink, &res);
    *records = head;

    return row;
}

int get_DB(Sink *sink, Result *res, const char *query, int *row) {

	/* check connection status */
	if(sink_state(sink) != CONN_OK) 
	{
		printf("[sink_pool_get]sink platid=%d connect bad! then reset.\n", sink->platid);
		sink_reset(sink);
	}

	int  state;
	res->priv = 0;
	sink_exec2(sink, query, res);
	state = result_state(sink, res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		return state;
	}

	if (row) *row = result_rn(sink, res);

	return state;

}

//获取用户的未回答列表
//获取用户的已回答列表
//获取讲师的未回答列表
//获取讲师的已回答列表
int func_getquestionlist_DB(Sink* sink,int userid,unsigned char bteacher, unsigned char bunread,long long messageid,int startpos,int count,SList** list)
{
    Result res,res2;
    SList * head = NULL;
    int row, row2,state, r;
    char query[512] = {0};
    const char* czvalue;
    int questionmaxlen = 512;
	int stokeidmaxlen = 512;
	int answermaxlen = 512;
	
    if ( bteacher )
    {
        if ( bunread )
        {
            sprintf(query,"select questionid,userid,reqtime,left(question,%d),left(stokeid,%d),questionstype from tb_questionreq where teacherid=%d and unread=1 and questionid%s%d order by questionid desc limit %d offset %d",questionmaxlen,stokeidmaxlen,userid,(0==messageid)?"!=":"<=",messageid,count,startpos);            
        }
		else 
		{
            sprintf(query,"select left(a.question,%d),a.userid,b.answerid,b.ndatetime,left(b.answer,%d),b.questionstype,left(a.stokeid,%d) from tb_questionreq a,tb_answerreq b where a.questionid=b.questionid and b.userid=%d and a.unread=0 and b.answerid%s%d order by b.answerid desc limit %d offset %d",questionmaxlen,answermaxlen,stokeidmaxlen,userid,(0==messageid)?"!=":"<=",messageid,count,startpos);
		}
    }
	else
	{
	    if ( bunread )
        {
            sprintf(query,"select questionid,teacherid,reqtime,left(question,%d),left(stokeid,%d),questionstype from tb_questionreq where userid=%d and unread=1 and questionid%s%d order by questionid desc limit %d offset %d",questionmaxlen,stokeidmaxlen,userid,(0==messageid)?"!=":"<=",messageid,count,startpos);               
        }
		else 
		{
            sprintf(query,"select left(a.question,%d),a.teacherid,b.answerid,b.ndatetime,left(b.answer,%d),b.questionstype,left(a.stokeid,%d) from tb_questionreq a,tb_answerreq b where a.questionid=b.questionid and a.userid=%d and a.unread=0 and b.answerid%s%d order by b.answerid desc limit %d offset %d",questionmaxlen,answermaxlen,stokeidmaxlen,userid,(0==messageid)?"!=":"<=",messageid,count,startpos);
		}
	}
    //获取记录条数
        //printf("====>sql:%s<===\n",query);
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state (sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)  &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	row = result_rn(sink, &res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}
	else
	{
        for(r=0; r<row; r++)
    	{
            DDInteractRes_t* record = (DDInteractRes_t*)malloc(sizeof(DDInteractRes_t));
			memset(record, 0, sizeof(DDInteractRes_t));
            

			if ( bunread )
			{
			    memset(record->srccontent,0,sizeof(record->srccontent)/sizeof(char));
				memset(record->dstcontent,0,sizeof(record->dstcontent)/sizeof(char));
    	        czvalue = result_get(sink, &res, r, 0);
				record->messageid = atoll(czvalue);
				czvalue = result_get(sink, &res, r, 1);
				record->userid = atoi(czvalue);
				czvalue = result_get(sink, &res, r, 2);
				record->messagetime = atoll(czvalue);
				czvalue = result_get(sink, &res, r, 3);
				strncpy(record->question, czvalue, questionmaxlen);
                record->question[questionmaxlen] = '\0';
				czvalue = result_get(sink, &res, r, 4);
				strncpy(record->stoke, czvalue,stokeidmaxlen);
				record->stoke[stokeidmaxlen] = '\0';
				czvalue = result_get(sink, &res, r, 5);
				record->commentstype = atoi(czvalue);
			}
			else
			{          
			    memset(record->srccontent,0,sizeof(record->srccontent)/sizeof(char));
				memset(record->dstcontent,0,sizeof(record->dstcontent)/sizeof(char));
				czvalue = result_get(sink, &res, r, 0);
				strncpy(record->question, czvalue, questionmaxlen);
				record->srccontent[questionmaxlen] = '\0';
				czvalue = result_get(sink, &res, r, 1);
				record->userid = atoi(czvalue); 
				czvalue = result_get(sink, &res, r, 2);
				record->messageid = atoll(czvalue); 
				czvalue = result_get(sink, &res, r, 3);
				record->messagetime = atoll(czvalue);		
				czvalue = result_get(sink, &res, r, 4);
				strncpy(record->answer, czvalue, answermaxlen);
				record->srccontent[answermaxlen] = '\0';
				czvalue = result_get(sink, &res, r, 5);
				record->commentstype = atoi(czvalue);
				czvalue = result_get(sink, &res, r, 6);
				strncpy(record->stoke, czvalue, stokeidmaxlen);
			}

			//获取用户头像与用户
			memset(query, 0, sizeof(query));
            sprintf(query,"select left(calias,50),nheadid from dks_user where nuserid=%d",record->userid);
            sink_exec2(sink, query, &res2);
            state = result_state (sink, &res2);
        	if((state != RES_COMMAND_OK) &&
        		(state != RES_TUPLES_OK)  &&
        		(state != RES_COPY_IN)  &&
        		(state != RES_COPY_OUT))
        	{
        		result_clean(sink, &res2);
        		break;
        	}
			row2 = result_rn(sink, &res2);
        	if(row2 == 0)
        	{
        		result_clean(sink, &res2);
        		break;
        	}
			else
			{
			    memset(record->useralias,0,sizeof(record->useralias)/sizeof(char));
        		czvalue = result_get(sink, &res2,0,0);
        		strncpy(record->useralias, czvalue, 50);
				record->useralias[50] = '\0';
				czvalue = result_get(sink, &res2,0,1);
				record->userheadid = atoi(czvalue);
        		result_clean(sink, &res2);
			}
			
            do{
                memset(query, 0, sizeof(query));
                sprintf(query,"select headid from dks_teacherinfo where teacherid=%d",record->userid);
                sink_exec2(sink, query, &res2);
                state = result_state (sink, &res2);
            	if((state != RES_COMMAND_OK) &&
            		(state != RES_TUPLES_OK)  &&
            		(state != RES_COPY_IN)  &&
            		(state != RES_COPY_OUT))
            	{
            		result_clean(sink, &res2);
            		break;
            	}
    			row2 = result_rn(sink, &res2);
            	if(row2 == 0)
            	{
            		result_clean(sink, &res2);
            		break;
            	}
    			else
    			{
    			    //memset(record->useralias,0,sizeof(record->useralias)/sizeof(char));
            		czvalue = result_get(sink, &res2,0,0);
					if ( atoi(czvalue) != 0 )
    				    record->userheadid = atoi(czvalue);
            		result_clean(sink, &res2);
    			}
            }while(0);
			
			//当普通用户获取未读消息时，头像ID取的用户自己的ID
			if ( !bteacher && bunread )
			{
				memset(query, 0, sizeof(query));
				sprintf(query,"select nheadid from dks_user where nuserid=%d",userid);
				sink_exec2(sink, query, &res2);
				state = result_state (sink, &res2);
				if((state != RES_COMMAND_OK) &&
					(state != RES_TUPLES_OK)  &&
					(state != RES_COPY_IN)  &&
					(state != RES_COPY_OUT))
				{
					result_clean(sink, &res2);
					break;
				}
				row2 = result_rn(sink, &res2);
				if(row2 == 0)
				{
					result_clean(sink, &res2);
					break;
				}
				else
				{
					czvalue = result_get(sink, &res2,0,0);
					record->userheadid = atoi(czvalue);
					result_clean(sink, &res2);
				}
			}
			
			head = slist_append(head, record);
        }
	}
    result_clean(sink, &res);
    *list = head;
    return 0;
}


int func_getunreaddot_DB(Sink* sink,int userid)
{
    Result res;
    int row, state;
    char query[256] = {0};

    //以下四个表，只需其中一个有记录就返回
    sprintf(query, "select * from tb_questionreq where unread=1 and teacherid=%d limit 1",userid);
    row = 0;
    res.priv = 0;
    sink_exec2(sink, query, &res);
    state = result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK) &&
        (state != RES_COPY_IN) &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return -1;
    }

    row = result_rn(sink, &res);
	result_clean(sink, &res);
    if(row != 0)
    {
        return 1;
    }

	sprintf(query,"select * from tb_questionreq a,tb_answerreq b where a.questionid=b.questionid and b.unread=1 and a.userid=%d limit 1",userid);
	row = 0;
    res.priv = 0;
    sink_exec2(sink, query, &res);
    state = result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK) &&
        (state != RES_COPY_IN) &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return -1;
    }

    row = result_rn(sink, &res);
	result_clean(sink, &res);
    if(row != 0)
    {
        return 1;
    }

    sprintf(query,"select * from tb_interacthistory where unread=1 and userid=%d limit 1",userid);
	row = 0;
    res.priv = 0;
    sink_exec2(sink, query, &res);
    state = result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK) &&
        (state != RES_COPY_IN) &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return -1;
    }

    row = result_rn(sink, &res);
	result_clean(sink, &res);
    if(row != 0)
    {
        return 1;
    }

    sprintf(query,"select * from tb_viewcomments where unread=1 and toid=%d limit 1",userid);
	row = 0;
    res.priv = 0;
    sink_exec2(sink, query, &res);
    state = result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK) &&
        (state != RES_COPY_IN) &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return -1;
    }

    row = result_rn(sink, &res);
	result_clean(sink, &res);
    if(row != 0)
    {
        return 1;
    }
	
    return 0;
}

int get_textlivechataction_DB(Sink* sink, SList** list, CMDHallMessageReq_t2 *info)
{
	Result res;
	SList* head=0;
	int row,state,r;
	char query[512];

	//if(info->teacherflag==0)
	//{
		if(info->messageid>0)
		{
			sprintf(query, "select userid,teacherid,interactid,source,ccontent,dtime,commentstype,unread  \
					from tb_interacthistory \
					where userid=%d and interactid<=%d \
					order by interactid DESC LIMIT %d offset %ld",
					info->userid,info->messageid,info->count,info->startIndex);
		}
		else
		{
			sprintf(query, "select userid,teacherid,interactid,source,ccontent,dtime,commentstype,unread  \
					from tb_interacthistory \
					where userid=%d \
					order by interactid DESC LIMIT %d ",
					info->userid,info->count);
		}
	//}
	//else if(info->teacherflag==1)
	//{
		/*
		if(info->messageid>0)
		{
			sprintf(query, "select teacherid,userid,interactid,source,ccontent,dtime,commentstype,unread  \
					from tb_interacthistory \
					where teacherid=%d and interactid<=%d \
					order by interactid DESC LIMIT %d offset %ld",
					info->userid,info->messageid,info->count,info->startIndex);
		}
		else
		{
			sprintf(query, "select teacherid,userid,interactid,source,ccontent,dtime,commentstype,unread  \
					from tb_interacthistory \
					where teacherid=%d \
					order by interactid DESC LIMIT %d ",
					info->userid,info->count);
		}
		*/
	//}
	row =0;
	res.priv =0;
	sink_exec2(sink, query, &res);

	state =result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}
	row=result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	for(r=0; r<row; r++)
	{
		CMDInteractRes_t2* record=(CMDInteractRes_t2*)malloc(sizeof(CMDInteractRes_t2));
		memset(record, 0, sizeof(CMDInteractRes_t2));

		record->userid = atoi(result_get(sink, &res, r, 0));
		record->touserid = atoi(result_get(sink, &res, r, 1));
		record->messageid = atoll(result_get(sink, &res, r, 2));
		strcpy(record->srccontent, result_get(sink, &res,r,3));
		strcpy(record->dstcontent, result_get(sink, &res,r,4));
		record->messagetime = atoll(result_get(sink, &res, r, 5));
		record->commentstype = atoi(result_get(sink, &res, r, 6));
		record->unread = atoi(result_get(sink, &res, r, 7));

		Result res1;
		char usr_query[512];
		int row1 = 0;

		res1.priv = 0;
		//if(info->teacherflag==1)
		//{
		//	sprintf(usr_query, "select calias,nheadid from dks_user where nuserid=%d limit 1", record->touserid);
		//}
		//else
		//{
			sprintf(usr_query, " select u.calias,t.headid from dks_user u,dks_teacherinfo t where u.nuserid=%d and u.nuserid=t.teacherid limit 1",record->touserid);
		//}
		sink_exec2(sink, usr_query, &res1);
		state = result_state(sink, &res1);
		if((state != RES_COMMAND_OK) &&
			(state != RES_TUPLES_OK)  &&
			(state != RES_COPY_IN)    &&
			(state != RES_COPY_OUT))
		{
			strcpy(record->touseralias, "");
			record->touserheadid=0;
		}
		else
		{
			row1 = result_rn(sink,&res1);
			if(row1 == 0)
			{
				strcpy(record->touseralias, "");
				record->touserheadid=0;
			}
			else
			{
				strcpy(record->touseralias, result_get(sink, &res1,0,0));
				record->touserheadid=atoi(result_get(sink, &res1,0,1));
			}
		}
		result_clean(sink, &res1);

		head = slist_append(head, record);
	}
	result_clean(sink, &res);

	*list = head;
	return row;
}

int update_textlivechatread_DB(Sink* sink, int64 messageid)
{
	char query[256];
	sprintf(query, "update tb_interacthistory set unread=0 where interactid=%d",messageid);
	return exec_query(sink, query);
}

int get_teacherflag_DB(Sink* sink,int userid)
{
    Result res;
    int row, state;
    char query[256];

	sprintf(query, " select 1 from dks_teacherinfo where teacherid=%d ", userid);

    row = 0;
    res.priv = 0;
    sink_exec2(sink, query, &res);
    state = result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK)  &&
        (state != RES_COPY_IN)    &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return 0;
    }

    row = result_rn(sink,&res);
    if(row == 0)
    {
        result_clean(sink, &res);
        return 0;
    }
    else
    {
    	return 1;
    }
}

int get_interactunreads_DB(Sink* sink,int userid)
{
    Result res;
    int row, state;
    char query[256];
    const char* szVal;
    int64 count=0;

	sprintf(query, "select count(1) from tb_interacthistory where userid=%d and unread=1", userid);

    row = 0;
    res.priv = 0;
    sink_exec2(sink, query, &res);
    state = result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK)  &&
        (state != RES_COPY_IN)    &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return 0;
    }

    row = result_rn(sink,&res);
    if(row == 0)
    {
        result_clean(sink, &res);
        return 0;
    }

    szVal = result_get(sink, &res, 0, 0);
    count = atoll(szVal);
    result_clean(sink, &res);

    return count;
}

int get_viewunreads_DB(Sink* sink,int userid)
{
    Result res;
    int row, state;
    char query[256];
    const char* szVal;
    int64 count=0;

    sprintf(query, "select count(1) from tb_viewcomments where toid=%d and unread=1", userid);
    row = 0;
    res.priv = 0;
    sink_exec2(sink, query, &res);
    state = result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK)  &&
        (state != RES_COPY_IN)    &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return 0;
    }

    row = result_rn(sink,&res);
    if(row == 0)
    {
        result_clean(sink, &res);
        return 0;
    }

    szVal = result_get(sink, &res, 0, 0);
    count = atoll(szVal);
    result_clean(sink, &res);

    return count;
}

int get_questionunreads_DB(Sink* sink,int teacherid)
{
    Result res;
    int row, state;
    char query[256];
    const char* szVal;
    int64 count=0;

    sprintf(query, "select count(1) from tb_questionReq where teacherid=%d and unread=1", teacherid);
    row = 0;
    res.priv = 0;
    sink_exec2(sink, query, &res);
    state = result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK)  &&
        (state != RES_COPY_IN)    &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return 0;
    }

    row = result_rn(sink,&res);
    if(row == 0)
    {
        result_clean(sink, &res);
        return 0;
    }

    szVal = result_get(sink, &res, 0, 0);
    count = atoll(szVal);
    result_clean(sink, &res);

    return count;
}

int get_answerunreads_DB(Sink* sink,int userid)
{
    Result res;
    int row, state;
    char query[256];
    const char* szVal;
    int64 count=0;

    sprintf(query, "select count(1) from tb_answerReq where userid=%d and unread=1", userid);
    row = 0;
    res.priv = 0;
    sink_exec2(sink, query, &res);
    state = result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK)  &&
        (state != RES_COPY_IN)    &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return 0;
    }

    row = result_rn(sink,&res);
    if(row == 0)
    {
        result_clean(sink, &res);
        return 0;
    }

    szVal = result_get(sink, &res, 0, 0);
    count = atoll(szVal);
    result_clean(sink, &res);

    return count;
}

//获取几月份以前的体验号
int get_supaccount_DB(Sink* sink, SList** list,char* date,int count)
{
	Result res;
	SList* head=0;
	int row,state,r;
	char query[2048];

    if ( !date )
    {
        
return -1;
    }
 
	sprintf(query,"select t.nuserid,t.contribution,t.dtime,u.calias from (select nuserid, sum(nusermoney) as \
		contribution,max(dtime) as dtime from dks_usergifttradelog where dtime<'%s' and nuserid not \
		in(1694357,1680006,19831205,1762266,1752923,1680961,1680007,1735497,1725674,1680024,1702221,1770187,\
		1683010,1703970,1732463,1757229,1680064,1707745,1704004,1776063,1682818,1747268,1680150,1722757,1682593,\
		1702212,1693851,1702208,1732942,1702163,1692964,1705536,1690287,1689474,1715121,1694045,1769428,90003,1721121,\
		1721868,1743344,90011,1680010,1772165,1772171,1693488,1694552,1680539,1767041,1756512,1774513,1710672,1754051,\
		1734004,1683711,1680502,1693179,1715125,1680137,1680040,1680012,1682458,1680028,1682026,1680660,1702307,1680485,\
		1704232,1731502,1760339,600101,1680014,1680008,1720398,1702131,1702039,1701745,1683247,1683662,1731218,1694146,\
		1694151,1725429,1715123,1702223,90002,1681453,1734760,1681465,1684274,1758071,1702240,1680952,1749312,1708450,\
		1680181,1680153,1693483,1693520,1723951,1753362,951125,90020,1682828,1758727,1721531,1740893,1680997,1691109,\
		1710884,1684886,1735147,1680009,1680592,1690269,1737985,1681975,1723188,1715930,1694066,1693485,1711107,1702287,\
		1702318,1734377,1754632,1758064,1702156,1747037,1715520,1691382,1754498,19890815,1714928,1702243,1684131,1705572,\
		1701467,1684920,90014,1707234,1683271,1680144,1734055,1691386,1691390,1680198,1693928,1693929,1762983,1714928,1741271,\
		1706716) group by nuserid order by sum(nusermoney) desc,dtime asc limit %d)t left join dks_user u on u.nuserid=t.nuserid",date,count);
	row =0;
	res.priv =0;
	sink_exec2(sink, query, &res);

	state =result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}
	row=result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	for(r=0; r<row; r++)
	{
		SupAccountS_t* record=(SupAccountS_t*)malloc(sizeof(SupAccountS_t));
		memset(record, 0, sizeof(SupAccountS_t));

        record->userid = atoi(result_get(sink, &res, r, 0));
		record->contribution = atoi(result_get(sink, &res, r, 1));
		strcpy(record->alias, result_get(sink, &res,r,3));
		
		head = slist_append(head, record);
	}
	result_clean(sink, &res);

	*list = head;
	return row;
}

int update_viplevel_DB(Sink* sink,int userid,int viplevel)
{
	char query[256];
	sprintf(query, "update dks_user set nviplevel=%d where nuserid=%d",viplevel,userid);
	return exec_query(sink, query);
}

unsigned int func_get_visitor_unique_id(Sink * sink, unsigned int area)
{
	Result res;
	char query[128]={0};
	int state;
	unsigned int ret_sucess = -1;
	const char* str;

	sprintf(query,"select nextval('visitorid', %u)", area);

	res.priv = 0;

	sink_exec2(sink, query, &res);

	state = result_state (sink, &res);

	if((state != RES_COMMAND_OK) &&
			(state != RES_TUPLES_OK)  &&
			(state != RES_COPY_IN)  &&
			(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	str = result_get (sink, &res, 0, 0);
	ret_sucess = atoi(str);
	result_clean (sink, &res);
	return ret_sucess;
}

int func_getsyscast_DB(Sink* sink,SList** list)
{
    Result res;
	char query[128]={0};
	int state;
	int row;
	const char* str;
    SList* head=0;
    
	SysBoardCast* syscast = 0;
    
	
	sprintf(query,"select nid,nisoneoff,cnewstitle,cnewstext from dks_syscastnews order by dcreatetime desc limit 1");

	res.priv=0;
    //printf("====================\n");
	sink_exec2(sink, query, &res);

	state = result_state (sink, &res);

	if((state != RES_COMMAND_OK) &&
			(state != RES_TUPLES_OK)  &&
			(state != RES_COPY_IN)  &&
			(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}
    //printf("====================\n");
    row = result_rn(sink,&res);
    if( row == 0 )
    {
        result_clean(sink, &res);
        return -1;
    }

    syscast = (SysBoardCast*)malloc(sizeof(SysBoardCast));
    memset(syscast, 0, sizeof(SysBoardCast));

    //printf("====================\n");
	str = result_get (sink, &res, 0, 0);
	syscast->nid = atoll(str);
	str = result_get (sink, &res, 0, 1);
	syscast->newType = (unsigned char)atoi(str);
	str = result_get (sink, &res, 0, 2);
	strcpy(syscast->title,str);
	str = result_get (sink, &res, 0, 3);
	strcpy(syscast->content,str);
	//printf("%d %d %s %s",syscast->nid,syscast->newType,syscast->title,syscast->content);

	head = slist_append(head, syscast);
	
	*list = head;
	//printf("====================\n");
	result_clean (sink, &res);
	return 0;
}

int64 insert_textliveanswer_DB(Sink* sink, CMDViewAnswerReq_t2 *info)
{
	Result res;
	int row, state;
	char query[2048];
	const char* szVal;
	int64 answerid=0;

	time_t tnow=time(0);
	int64 nowtime=0;
	char sztime[128]={0};
	toStringTimestamp3(tnow, sztime);
	nowtime=atoll(sztime);

	sprintf(query, " select nextval('tb_answerreq_answerid_seq') ");

	row = 0;
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
	    (state != RES_TUPLES_OK)  &&
	    (state != RES_COPY_IN)    &&
	    (state != RES_COPY_OUT))
	{
	    result_clean(sink, &res);
	    return -1;
	}

	row = result_rn(sink,&res);
	if(row == 0)
	{
	    result_clean(sink, &res);
	    return -1;
	}

	szVal = result_get(sink, &res, 0, 0);
	answerid = atoll(szVal);
	result_clean(sink, &res);

	sprintf(query, "insert into tb_answerReq(answerid,questionid,userid,ndatetime,answer,questionstype) values(%lld,%lld,%d,%lld,'%s',%d)",
			answerid, info->messageid, info->fromid, nowtime, info->content, info->commentstype);
	int result = exec_query(sink, query);
	if(result<0)
		return -1;
	else
		return answerid;
}

//query user notify config in db
int load_usernotify_config_DB(Sink * sink, SList ** list)
{
    Result res;
    int row = 0;
    int state = 0;
    char querySQL[512] = {0};
    //0:no push,1:pushed,2:responsed
    sprintf(querySQL, "select nuserid,termtype,notitype,versionflag,version,inserttime,validtime,needresp,state from tb_usernotify where state != 2");
    res.priv = 0;
    sink_exec2(sink, querySQL, &res);
    state = result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK) &&
        (state != RES_COPY_IN) &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return -1;
    }

    row = result_rn(sink, &res);
    if(0 == row)
    {
        result_clean(sink, &res);
        return 0;
    }

    const char * czvalue = NULL;
    SList * head = NULL;
	int r = 0;
    for(; r < row; ++r)
    {
        DDLUserNotify * pItem = (DDLUserNotify *)malloc(sizeof(DDLUserNotify));
        memset(pItem, 0, sizeof(DDLUserNotify));

        //nuserid,termtype,notitype,versionflag,version,inserttime,validtime,needresp,state
        czvalue = result_get(sink, &res, r, 0);
        if (NULL != czvalue)
        {
            pItem->nuserid = atoi(czvalue);
        }

        czvalue = result_get(sink, &res, r, 1);
        if (NULL != czvalue)
        {
            pItem->termtype = atoi(czvalue);
        }

        czvalue = result_get(sink, &res, r, 2);
        if (NULL != czvalue)
        {
            pItem->notitype = atoi(czvalue);
        }

        czvalue = result_get(sink, &res, r, 3);
        if (NULL != czvalue)
        {
            pItem->versionflag = atoi(czvalue);
        }

        czvalue = result_get(sink, &res, r, 4);
        if (NULL != czvalue)
        {
            pItem->version = atoi(czvalue);
        }

        czvalue = result_get(sink, &res, r, 5);
        if (NULL != czvalue)
        {
            pItem->inserttime = toNumericTimestamp(czvalue);
        }

        czvalue = result_get(sink, &res, r, 6);
        if (NULL != czvalue)
        {
            pItem->validtime = atoi(czvalue);
        }

        czvalue = result_get(sink, &res, r, 7);
        if (NULL != czvalue)
        {
            pItem->needresp = atoi(czvalue);
        }

        czvalue = result_get(sink, &res, r, 8);
        if (NULL != czvalue)
        {
            pItem->state = atoi(czvalue);
        }

        head = slist_append(head, pItem);
    }

    result_clean(sink, &res);
    *list = head;

    return row;
}

//insert user notify config in db
int insert_usernotify_config_DB(Sink * sink, DDLUserNotify * record)
{
    if (NULL == record)
    {
        return -1;
    }

    unsigned int userid = record->nuserid;
    unsigned int termtype = record->termtype;
    unsigned int notitype = record->notitype;
    unsigned int versionflag = record->versionflag;
    unsigned int version = record->version;

    time_t inserttime = record->inserttime;
    char szInsertTime[32]={0};
    toStringTimestamp(inserttime, szInsertTime);    

    unsigned int needresp = record->needresp;
    unsigned int validtime = record->validtime;
    unsigned int state = record->state;

    char insertSQL[512] = {0};
    sprintf(insertSQL, "insert into tb_usernotify(nuserid,termtype,notitype,versionflag,version,validtime,needresp,state,inserttime) values(%u,%u,%u,%u,%u,%u,%u,%u,'%s')",\
        userid, termtype, notitype, versionflag, version, validtime, needresp, state, szInsertTime);
    return exec_query(sink, insertSQL);
}

//update user notify config in db
void update_usernotify_config_DB(Sink * sink, DDLUserNotify * record)
{
    if (NULL == record)
    {
        return;
    }

    unsigned int userid = record->nuserid;
    unsigned int termtype = record->termtype;
    unsigned int notitype = record->notitype;

    time_t inserttime = record->inserttime;
    char szInsertTime[32]={0};
    toStringTimestamp(inserttime, szInsertTime);

    unsigned int needresp = record->needresp;
    unsigned int validtime = record->validtime;
    unsigned int versionflag = record->versionflag;
    unsigned int version = record->version;
    unsigned int state = record->state;

    char updateSQL[512] = {0};
    sprintf(updateSQL, "update tb_usernotify set versionflag=%u,version=%u,validtime=%u,needresp=%u,state=%u,inserttime='%s',pushtime=NULL where nuserid=%u and termtype=%u and notitype=%u",\
        versionflag, version, validtime, needresp, state, szInsertTime, userid, termtype, notitype);
    exec_query(sink, updateSQL);
}

void update_usernotify_state_DB(Sink * sink, DDLUserNotify * record)
{
    if (NULL == record)
    {
        return;
    }

    unsigned int userid = record->nuserid;
    unsigned int termtype = record->termtype;
    unsigned int notitype = record->notitype;
    unsigned int state = record->state;
    time_t pushtime = record->pushtime;
    char szPushTime[32]={0};
    toStringTimestamp(pushtime, szPushTime);

    char updateSQL[512] = {0};
    sprintf(updateSQL, "update tb_usernotify set state=%u,pushtime='%s' where nuserid=%u and termtype=%u and notitype=%u",\
        state, szPushTime, userid, termtype, notitype);
    exec_query(sink, updateSQL);
}

int check_userconfig_exist_DB(Sink * sink, DDLUserNotify * record)
{
    if (NULL == record)
    {
        return -1;
    }

    unsigned int userid = record->nuserid;
    unsigned int termtype = record->termtype;
    unsigned int notitype = record->notitype;

    char cSQL[256] = {0};
    sprintf(cSQL, "select 1 from tb_usernotify where nuserid=%u and termtype=%u and notitype=%u", userid, termtype, notitype);
    Result res;
    res.priv = 0;
    sink_exec2(sink, cSQL, &res);
    int state = result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK) &&
        (state != RES_COPY_IN) &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return -1;
    }

    int row = result_rn(sink, &res);
    result_clean(sink, &res);
    if(0 == row)
    {
        return 0;
    }else
    {
        return 1;   
    }
}


int func_getFreeTimes_DB(Sink* sink,int classid)
{
	Result res;
	int row, state;
	char query[2048];
	const char* szVal;
    int times = 0;

	sprintf(query, "select a.nattdata from tb_attribute_param a,tb_goodattribute b where a.nattid=b.nattid and b.ngoodclassid=%d ",classid);
    row = 0;
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
	    (state != RES_TUPLES_OK)  &&
	    (state != RES_COPY_IN)    &&
	    (state != RES_COPY_OUT))
	{
	    result_clean(sink, &res);
	    return -1;
	}

	row = result_rn(sink,&res);
	if(row == 0)
	{
	    result_clean(sink, &res);
	    return -1;
	}

	szVal = result_get(sink, &res, 0, 0);
	times = atoi(szVal);
	result_clean(sink, &res);

	return times;
}

int get_shitulist_DB(Sink* sink,unsigned char bTeacherTag,int userid,SList** list)
{	
	Result res,res2;
	SList* head=0;
	int row,state,r,row2;
	char query[2048];
	const char* czvalue = 0;
 
	sprintf(query,"select %s,to_char(mastertime,'yyyymmddhhmiss'),to_char(mastertime+(effecttime||'days')::interval,'yyyymmddhhmiss') from tb_taketeacher where %s=%d and mastertime > now() - (effecttime || 'days')::interval",bTeacherTag?"userid":"teacherid",bTeacherTag?"teacherid":"userid",userid);
	//printf("-------------------------->%s<_______________________________\n",query);
	row =0;
	res.priv =0;
	sink_exec2(sink, query, &res);

	state =result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}
	row=result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	for(r=0; r<row; r++)
	{
		ShiTuItem_t2* record=(ShiTuItem_t2*)malloc(sizeof(ShiTuItem_t2));
		memset(record, 0, sizeof(ShiTuItem_t2));

        record->userid = atoi(result_get(sink, &res, r, 0));
		record->startTime = atoll(result_get(sink, &res, r, 1));
		record->effectTime = atoll(result_get(sink, &res, r, 2));
		
		//获取用户头像与用户
		sprintf(query,"select left(calias,32) from dks_user where nuserid=%d",record->userid);
		sink_exec2(sink, query, &res2);
		state = result_state (sink, &res2);
		if((state != RES_COMMAND_OK) &&
			(state != RES_TUPLES_OK)  &&
			(state != RES_COPY_IN)	&&
			(state != RES_COPY_OUT))
		{
			result_clean(sink, &res2);
			break;
		}
		row2 = result_rn(sink, &res2);
		if(row2 == 0)
		{
			result_clean(sink, &res2);
			break;
		}
		else
		{
			memset(record->alias,0,sizeof(record->alias)/sizeof(char));
			czvalue = result_get(sink, &res2,0,0);
			strncpy(record->alias, czvalue, 32);
			record->alias[32] = '\0';
			result_clean(sink, &res2);
		}
		
		head = slist_append(head, record);
	}
	result_clean(sink, &res);

	*list = head;
	return row;
}

int insert_shitulist_DB(Sink* sink,int vcbid,int userid,int teacherid,long long startTime,int effectTime)
{
	char cSQL[1024] = {0};
    sprintf(cSQL, "insert into tb_taketeacher(userid,teacherid,roomid,effecttime,mastertime) values(%d,%d,%d,%d,now())",userid,teacherid,vcbid,effectTime);
	printf("%s\n\n",cSQL);
    return exec_query(sink, cSQL);
}

int get_IsStudent_DB(Sink* sink,int teacherid,int userid)
{
	Result res;
	int row, state;
	char query[2048];

    //是否在有效拜师时间内
	sprintf(query, "select * from tb_taketeacher where teacherid=%d and userid=%d and mastertime > now() - (effecttime || 'days')::interval",teacherid,userid);
    row = 0;
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
	    (state != RES_TUPLES_OK)  &&
	    (state != RES_COPY_IN)    &&
	    (state != RES_COPY_OUT))
	{
	    result_clean(sink, &res);
	    return 0;
	}

	row = result_rn(sink,&res);
	if(row == 0)
	{
	    result_clean(sink, &res);
	    return 0;
	}
	else
	{
		result_clean(sink, &res);
		return 1;
	}
}

int get_studentCount_DB(Sink* sink,int teacherid)
{
	Result res;
	int row, state;
	char query[2048];
	const char* szVal;
    int count = 0;

	sprintf(query, "select count(*) from tb_taketeacher where teacherid=%d and mastertime > now() - (effecttime || 'days')::interval",teacherid);
    row = 0;
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
	    (state != RES_TUPLES_OK)  &&
	    (state != RES_COPY_IN)    &&
	    (state != RES_COPY_OUT))
	{
	    result_clean(sink, &res);
	    return -1;
	}

	row = result_rn(sink,&res);
	if(row == 0)
	{
	    result_clean(sink, &res);
	    return -1;
	}

	szVal = result_get(sink, &res, 0, 0);
	count = atoi(szVal);
	result_clean(sink, &res);

	return count;
}

int get_secretsbyteacher_info_DB(Sink* sink, int teacherid)
{
    Result res;
    int row, state;
    char query[256];
    const char* szVal;
    int count=0;

    sprintf(query, "select count(1) from tb_personalsecrets where teacherid=%d and status=1", teacherid);
    row = 0;
    res.priv = 0;
    sink_exec2(sink, query, &res);
    state = result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK)  &&
        (state != RES_COPY_IN)    &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return 0;
    }

    row = result_rn(sink,&res);
    if(row == 0)
    {
        result_clean(sink, &res);
        return 0;
    }

    szVal = result_get(sink, &res, 0, 0);
    count = atoi(szVal);
    result_clean(sink, &res);

    return count;
}

int get_secretsbyuser_info_DB(Sink* sink, int teacherid, int userid)
{
    Result res;
    int row, state;
    char query[256];
    const char* szVal;
    int count=0;

    sprintf(query, "select count(1) from tb_personalsecrets as p,tb_good as g "
    		"where p.teacherid=%d and p.status=1 and p.goodsid=g.ngoodclassid and g.nuserid=%d ", teacherid,userid);
    row = 0;
    res.priv = 0;
    sink_exec2(sink, query, &res);
    state = result_state(sink, &res);
    if((state != RES_COMMAND_OK) &&
        (state != RES_TUPLES_OK)  &&
        (state != RES_COPY_IN)    &&
        (state != RES_COPY_OUT))
    {
        result_clean(sink, &res);
        return 0;
    }

    row = result_rn(sink,&res);
    if(row == 0)
    {
        result_clean(sink, &res);
        return 0;
    }

    szVal = result_get(sink, &res, 0, 0);
    count = atoi(szVal);
    result_clean(sink, &res);

    return count;
}

int get_TextRoomSecretsAll_list_DB(Sink* sink, SList** list,CMDTextRoomLiveListReq_t2 *listreq)
{
	Result res;
	SList* head=0;
	int row,state,r;
	char query[2048];

	if(listreq->messageid>0)
	{
		sprintf(query, "select psid,cover,title,description,dtime,subscribenum,price,goodsid "
				" from tb_personalsecrets where psid<%lld and teacherid=%d and status=1 "
				" order by psid DESC LIMIT %d ",
				listreq->messageid,listreq->teacherid,listreq->count);
	}
	else
	{
		sprintf(query, "select psid,cover,title,description,dtime,subscribenum,price,goodsid "
				" from tb_personalsecrets where teacherid=%d and status=1 "
				" order by psid DESC LIMIT %d ",
				listreq->teacherid,listreq->count);
	}

	row =0;
	res.priv =0;
	sink_exec2(sink, query, &res);

	state =result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}
	row=result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	for(r=0; r<row; r++)
	{
		CMDTextRoomSecretsListResp_t2* record=(CMDTextRoomSecretsListResp_t2*)malloc(sizeof(CMDTextRoomSecretsListResp_t2));
		memset(record, 0, sizeof(CMDTextRoomSecretsListResp_t2));

		record->secretsid = atoi(result_get(sink, &res, r, 0));
		strncpy(record->coverlittle, result_get(sink, &res,r,1),sizeof(record->coverlittle));
		strncpy(record->title, result_get(sink, &res,r,2),sizeof(record->title));
		strncpy(record->text, result_get(sink, &res,r,3), sizeof(record->text));
		record->messagetime = atoll(result_get(sink, &res, r, 4));
		record->buynums = atoi(result_get(sink, &res, r, 5));
		record->prices = atoi(result_get(sink, &res, r, 6));
		record->goodsid = atoi(result_get(sink, &res, r, 7));

		head = slist_append(head, record);
	}
	result_clean(sink, &res);

	*list = head;
	return row;
}

int get_TextRoomSecretsUser_list_DB(Sink* sink, SList** list,CMDTextRoomLiveListReq_t2 *listreq)
{
	Result res;
	SList* head=0;
	int row,state,r;
	char query[2048];

	if(listreq->messageid>0)
	{
		sprintf(query, "select p.psid"
				" from tb_personalsecrets as p,tb_good as g where p.psid<%lld and p.teacherid=%d "
				" and p.status=1 and p.goodsid=g.ngoodclassid and g.nuserid=%d "
				" order by p.psid DESC LIMIT %d ",
				listreq->messageid,listreq->teacherid,listreq->userid,listreq->count);
	}
	else
	{
		sprintf(query, "select p.psid"
				" from tb_personalsecrets as p,tb_good as g where p.teacherid=%d "
				" and p.status=1 and p.goodsid=g.ngoodclassid and g.nuserid=%d "
				" order by p.psid DESC LIMIT %d ",
				listreq->teacherid,listreq->userid,listreq->count);
	}

	row =0;
	res.priv =0;
	sink_exec2(sink, query, &res);

	state =result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}
	row=result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	for(r=0; r<row; r++)
	{
		CMDTextRoomSecretsListOwn_t2* record=(CMDTextRoomSecretsListOwn_t2*)malloc(sizeof(CMDTextRoomSecretsListOwn_t2));
		memset(record, 0, sizeof(CMDTextRoomSecretsListOwn_t2));

		record->secretsid = atoi(result_get(sink, &res, r, 0));

		head = slist_append(head, record);
	}
	result_clean(sink, &res);

	*list = head;
	return row;
}

int get_TextRoomSecretsOwn_list_DB(Sink* sink, SList** list,CMDTextRoomLiveListReq_t2 *listreq)
{
	Result res;
	SList* head=0;
	int row,state,r;
	char query[2048];

	if(listreq->messageid>0)
	{
		sprintf(query, "select p.psid,p.cover,p.title,p.description,p.dtime,p.subscribenum,p.price,p.goodsid "
				" from tb_personalsecrets as p,tb_good as g where p.psid<%lld and p.teacherid=%d and p.status=1"
				" and p.goodsid=g.ngoodclassid and g.nuserid=%d "
				" order by p.psid DESC LIMIT %d ",
				listreq->messageid,listreq->teacherid,listreq->userid,listreq->count);
	}
	else
	{
		sprintf(query, "select p.psid,p.cover,p.title,p.description,p.dtime,p.subscribenum,p.price,p.goodsid "
				" from tb_personalsecrets as p,tb_good as g where p.teacherid=%d and p.status=1"
				" and p.goodsid=g.ngoodclassid and g.nuserid=%d "
				" order by p.psid DESC LIMIT %d ",
				listreq->teacherid,listreq->userid,listreq->count);
	}

	row =0;
	res.priv =0;
	sink_exec2(sink, query, &res);

	state =result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}
	row=result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	for(r=0; r<row; r++)
	{
		CMDTextRoomSecretsListResp_t2* record=(CMDTextRoomSecretsListResp_t2*)malloc(sizeof(CMDTextRoomSecretsListResp_t2));
		memset(record, 0, sizeof(CMDTextRoomSecretsListResp_t2));

		record->secretsid = atoi(result_get(sink, &res, r, 0));
		strncpy(record->coverlittle, result_get(sink, &res,r,1),sizeof(record->coverlittle));
		strncpy(record->title, result_get(sink, &res,r,2),sizeof(record->title));
		strncpy(record->text, result_get(sink, &res,r,3), sizeof(record->text));
		record->messagetime = atoll(result_get(sink, &res, r, 4));
		record->buynums = atoi(result_get(sink, &res, r, 5));
		record->prices = atoi(result_get(sink, &res, r, 6));
		record->goodsid = atoi(result_get(sink, &res, r, 7));

		head = slist_append(head, record);
	}
	result_clean(sink, &res);

	*list = head;
	return row;
}

int check_TextRoomSecretsBuy_DB(Sink* sink,int messageid, int teacherid, int userid)
{
	Result res;
	int row,state;
	char query[2048];

	sprintf(query, "select p.psid "
			" from tb_personalsecrets as p,tb_good as g where p.psid=%d and p.teacherid=%d and p.status=1"
			" and p.goodsid=g.ngoodclassid and g.nuserid=%d ",
			messageid,teacherid,userid);

	row =0;
	res.priv =0;
	sink_exec2(sink, query, &res);

	state =result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}
	row=result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}
	else
	{
		result_clean(sink, &res);
		return 1;		
	}
}

int update_TextRoomSecretsBuy_DB(Sink* sink,int messageid, int teacherid)
{
	Result res;
	char query[256];
	int state;
	query[0] = 0;

	sprintf(query, "update tb_personalsecrets set subscribenum=subscribenum+1 where psid=%d and teacherid=%d and status=1 ", messageid, teacherid);
	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state (sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)  &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	result_clean(sink, &res);
	return 0;
}

int get_PackagePrivilege_DB(Sink* sink, SList** list,int packageNum)
{
	Result res;
	SList* head=0;
	int row,state,r;
	char query[2048];
 
	sprintf(query,"select left(privileges,256) from tb_packageprivilege where packageid=%d order by recordid asc",packageNum);
	row =0;
	res.priv =0;
	sink_exec2(sink, query, &res);

	state =result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}
	row=result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	for(r=0; r<row; r++)
	{
		PrivilegeItem2_t2* record=(PrivilegeItem2_t2*)malloc(sizeof(PrivilegeItem2_t2));
		memset(record, 0, sizeof(SupAccountS_t));

        record->index = r;
		strcpy(record->privilege,result_get(sink, &res, r, 0));
		
		head = slist_append(head, record);
	}
	result_clean(sink, &res);

	*list = head;
	return row;
}


int get_tb_teachersubscription_DB(Sink *sink, SList** list, int *size, int nteacherid)
{
	Result res;
	int row,state,r;
	char query[256];
	const char* szval;
    SList* head=0;

	row=0;

	sprintf(query, "select nuserid from tb_teachersubscription where nteacherid=%d", nteacherid);

	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}

	row = result_rn(sink, &res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	for(r=0;r<row;++r)
	{
		szval=result_get(sink, &res, r, 0);
        DDTeacherSubscribeItem *item = (DDTeacherSubscribeItem *)malloc(sizeof(int));
        item->userid = atoi(szval);
	    head = slist_append(head, item);
    }

    *size = row;
    *list = head;
	result_clean(sink, &res);
	return 0;
}

int func_teacherSubscript_DB(Sink *sink,int userid,int teacherid,unsigned char bSub)
{
	char query[512] = {0};
	if ( bSub )
	{
		sprintf(query, "insert into tb_teachersubscription(nteacherid,nuserid) values(%d,%d)",teacherid,userid);
	}
	else
	{
		sprintf(query, "delete from tb_teachersubscription where nteacherid=%d and nuserid=%d",teacherid,userid);
	}
	exec_query(sink, query);
    return 0;
}

int func_getsubscriptstate_DB(Sink *sink,int userid,int teacherid)
{
	Result res;
	int row,state;
	char query[512];
	sprintf(query, "select * from tb_teachersubscription where nuserid=%d and nteacherid=%d", userid,teacherid);

	res.priv = 0;
	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 2;
	}
	row = result_rn(sink, &res);
	if(row == 0) {
		result_clean(sink, &res);
		return 0;
	}
	else
	{
		result_clean(sink, &res);
		return 1;
	}
}

int func_IsTeacherOnLive_DB(Sink *sink,int teacherid)
{
	Result res;
	int row,state;
	char query[2048];
	int roomid = 0;
	
	if ( teacherid == 0 )
	{
		return 0;
	}
 
	sprintf(query,"select nvcbid from dks_vcbzhuanboconfig where nuserid=%d limit 1",teacherid);
	row =0;
	res.priv =0;
	sink_exec2(sink, query, &res);

	state =result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}
	row=result_rn(sink,&res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}
	else
	{
		roomid = atoi(result_get(sink, &res,0, 0));
		result_clean(sink, &res);
		return roomid;
	}
}

int get_zhuanborobot_DB(Sink *sink, SList** list)
{
    //int nstatus;
    SList *head =0;
    Result *res;
    int row,state,r;
    char szSql[1024];
    sprintf(szSql, "select nvcbid,npubmicidx,nrobotid,nrobotvcbid,nuserid from dks_vcbzhuanboconfig where nstatus>0 and nuserid>0");
    res =sink_exec(sink, szSql);
    state = result_state(sink, res);
    if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, res);
		return -1;
	}
	row = result_rn(sink, res);
	for ( r = 0; r < row; r++) {
		DDZhuanboRobotInfo_t* record = (DDZhuanboRobotInfo_t *)malloc(sizeof(DDZhuanboRobotInfo_t));
        memset(record, 0, sizeof(DDZhuanboRobotInfo_t));
        record->nvcbid = atoi(result_get(sink, res, r, 0));
		record->npubmicidx = atoi(result_get(sink, res, r, 1));
        record->nrobotid = atoi(result_get(sink, res, r, 2));
        record->nrobotvcbid = atoi(result_get(sink, res, r, 3));
        record->nuserid = atoi(result_get(sink, res, r, 4));
	head = slist_append( head, record);
    }
	*list = head;
    result_clean(sink, res);
	return row;
}

//查询数据库里极光推送的配置
int load_jpush_config_db(Sink * sink, SList ** list)
{
	time_t now_time = time(NULL);
	char szTime[32] = {0};
	toStringTimestamp(now_time, szTime);

	Result res;
	int row = 0;
	int state = 0;
	char querySQL[512] = {0};

	sprintf(querySQL, "select termtype,pushtype,pushtime,pushinterval,validtime,content from tb_pushconfig where validtime >= '%s'", szTime);
	res.priv = 0;
	sink_exec2(sink, querySQL, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK) &&
		(state != RES_COPY_IN) &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return -1;
	}

	row = result_rn(sink, &res);
	if(0 == row)
	{
		result_clean(sink, &res);
		return 0;
	}

	const char * czvalue = NULL;
	SList * head = NULL;
	int r = 0;
	for(; r < row; ++r)
	{
		DDLJPushConfig * pItem = (DDLJPushConfig *)malloc(sizeof(DDLJPushConfig));
		memset(pItem, 0, sizeof(DDLJPushConfig));

		//termtype,pushtype,pushtime,pushinterval,validtime,content
		//termtype
		czvalue = result_get(sink, &res, r, 0);
		if (NULL != czvalue)
		{
			pItem->termtype = atoi(czvalue);
		}

		//pushtype
		czvalue = result_get(sink, &res, r, 1);
		if (NULL != czvalue)
		{
			pItem->pushtype = atoi(czvalue);
		}

		//pushtime
		czvalue = result_get(sink, &res, r, 2);
		if (NULL != czvalue)
		{
			pItem->pushtime = toNumericTimestamp(czvalue);
		}

		//pushinterval
		czvalue = result_get(sink, &res, r, 3);
		if (NULL != czvalue)
		{
			pItem->interval = atoi(czvalue);
		}

		//validtime
		czvalue = result_get(sink, &res, r, 4);
		if (NULL != czvalue)
		{
			pItem->endtime = toNumericTimestamp(czvalue);
		}

		//content
		czvalue = result_get(sink, &res, r, 5);
		if (NULL != czvalue)
		{
			strncpy(pItem->content, czvalue, 1024);
		}

		head = slist_append(head, pItem);
	}

	result_clean(sink, &res);
	*list = head;

	return row;
}

//更新数据库里极光推送配置的validtime
int update_jpush_config_db(Sink * sink, DDLJPushConfig * record)
{
	if (NULL == record)
	{
		return -1;
	}

	unsigned int termtype = record->termtype;
	unsigned int pushtype = record->pushtype;
	unsigned int pushinterval = record->interval;
	
	time_t pushtime = record->pushtime;
	char szPushTime[32] = {0};
	toStringTimestamp(pushtime, szPushTime);

	unsigned int validtime = record->endtime;
	char szValidTime[32] = {0};
	toStringTimestamp(validtime, szValidTime);

	char updateSQL[512] = {0};
	sprintf(updateSQL, "update tb_pushconfig set validtime='%s' where termtype=%u and pushtype=%u and pushtime='%s' and pushinterval=%u ",\
		szValidTime, termtype, pushtype, szPushTime, pushinterval);
	return exec_query(sink, updateSQL);
}

int get_tb_uservcbfavorite_DB(Sink *sink, SList** list, int *size, int nroomid)
{
	Result res;
	int row,state,r;
	char query[256];
	const char* szval;
    SList* head=0;

	row=0;

	sprintf(query, "select distinct nuserid from dks_uservcbfavorite where nvcbid=%d", nroomid);

	sink_exec2(sink, query, &res);
	state = result_state(sink, &res);
	state = result_state(sink, &res);
	if((state != RES_COMMAND_OK) &&
		(state != RES_TUPLES_OK)  &&
		(state != RES_COPY_IN)    &&
		(state != RES_COPY_OUT))
	{
		result_clean(sink, &res);
		return 0;
	}

	row = result_rn(sink, &res);
	if(row == 0)
	{
		result_clean(sink, &res);
		return 0;
	}

	for(r=0;r<row;++r)
	{
		szval=result_get(sink, &res, r, 0);
        DDUservcbFavoriteItem *item = (DDUservcbFavoriteItem *)malloc(sizeof(int));
        item->userid = atoi(szval);
	    head = slist_append(head, item);
    }

    *size = row;
    *list = head;
	result_clean(sink, &res);
	return 0;
}

//插入游客登陆状态数据
int insert_vistorlogon_DB(Sink *sink, VistorLogon_t *vsl)
{
	  if (NULL == vsl)
    {
        return -1;
    }
    time_t logontime = vsl->logontime;
    char _logontime[32]={0};
    toStringTimestamp(logontime, _logontime);    

    char insertSQL[512] = {0};
    sprintf(insertSQL, "insert into tbvisitorlogon(nuserid,cuuid,nstate,dlogontime) values(%u,'%s',%d,'%s')",\
        vsl->userid, vsl->cuuid, vsl->state, _logontime);
    return exec_query(sink, insertSQL);
}

////更新游客登陆状态数据
int update_vistorlogon_DB(Sink *sink, VistorLogon_t *vsl)
{
	  time_t logontime = vsl->logontime;
	  char _logontime[32] = {0};
	  toStringTimestamp(logontime, _logontime);
	  
	  char updateSQL[512] = {0};
	  sprintf(updateSQL, "update tbvisitorlogon set nstate=%d,dlogontime='%s' where nuserid=%u",\
		    vsl->state, _logontime, vsl->userid);
	  return exec_query(sink, updateSQL);
}

//获取可用的游客id
int get_vistorlogon_DB(Sink *sink, char *_uuid, SList** list)
{
    Result res;
	  int row,state,r;
	  char query[256];
    SList* head=0;
	  row=0;

	  sprintf(query, "select nuserid, nstate, dlogontime from tbvisitorlogon where cuuid='%s' and nstate=0", _uuid);

	  sink_exec2(sink, query, &res);
	  state = result_state(sink, &res);
	  state = result_state(sink, &res);
	  if((state != RES_COMMAND_OK) &&
		    (state != RES_TUPLES_OK)  &&
		    (state != RES_COPY_IN)    &&
		    (state != RES_COPY_OUT))
	  {
		    result_clean(sink, &res);
		    return 0;
	  }
	  row = result_rn(sink, &res);
	  if(row == 0)
	  {
		    result_clean(sink, &res);
		    return 0;
	  }
	  for(r=0;r<row;++r)
	  {
	  	  char _logontime[32] = {0};
        VistorLogon_t *vsl = (VistorLogon_t*)malloc(sizeof(VistorLogon_t));
        vsl->userid = atoi(result_get(sink, &res, r, 0));
        vsl->state = atoi(result_get(sink, &res, r, 1));
        strncpy(_logontime, result_get(sink, &res, r, 2), sizeof(_logontime));
        vsl->logontime = toNumericTimestamp(_logontime);
	      head = slist_append(head, vsl);
    }
    *list = head;
	  result_clean(sink, &res);
	  return 0;
}
