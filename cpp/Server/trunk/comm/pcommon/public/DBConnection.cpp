#include "DBConnection.h"
#include "Application.h"
map<string,Dbconnection*> Dbconn::instance ;
int Dbconn::switchindex = 0;
Config 	Dbconn::m_config;
Dbconnection Dbconn::m_syncDB;

std::map<string, int> Dbconn::m_mapDBPlatid;
int Dbconn::m_nPlatId = 1;

using namespace std;
string vdbnames[] = {"gzdb","shdb","bjdb"};
void OnDbStatusModify(string dbname)
{
	printf("dbname:%s\n",dbname.c_str());
	//call alarm;
	CAlarmNotify::sendAlarmNoty(e_all_notitype,e_db_connect,Application::getInstance()->getProcName(),"DB Connect Error",
																	"Room,Yunwei","Disconnect DB:[%s]",vdbnames[Dbconn::switchindex].c_str());
	printf("index:%d\n",Dbconn::switchindex);
	if(dbname == vdbnames[Dbconn::switchindex])
	{
		Dbconn::switchindex++;
		if(Dbconn::switchindex > 2)
		{
			Dbconn::switchindex = 0;
		}
		Dbconn::getSyncDB();
	}

}
Dbconn::Dbconn()
{


}

Dbconn::~Dbconn()
{

}

bool Dbconn::initDBSinkPool(int &platid, int poolsize/* = 5*/, string dbname /*= "maindb"*/)
{
	if (m_mapDBPlatid.find(dbname) != m_mapDBPlatid.end())
	{
		platid = m_mapDBPlatid[dbname];
		LOG_PRINT(log_info, "dbname[%s] platid[%u] has initialized yet..", dbname.c_str(), platid);
		return true;
	}

	int line;
	if(!m_config.load(line))
	{
		LOG_PRINT(log_error, "load config file error.");
		return false;
	}

	string ip = m_config.getString(dbname.c_str(), "ip");
	int port = m_config.getInt(dbname.c_str(), "port");
	string user = m_config.getString(dbname.c_str(), "user");
	string password = m_config.getString(dbname.c_str(), "password");
	string name = m_config.getString(dbname.c_str(), "dbname");
	LOG_PRINT(log_info, "initDBSinkPool ip:%s,port:%d,user:%s,password:%s,poolsize:%d.", ip.c_str(), port, user.c_str(), password.c_str(), poolsize);

	if (sink_pool_init("postgresql", m_nPlatId, poolsize, ip.c_str(), port, "", name.c_str(), user.c_str(), password.c_str()) < 0)
	{
		LOG_PRINT(log_error, "sink_pool_init error: poolsize:%d, dbname:%s, host[%s:%d], platid:%d.", poolsize, dbname.c_str(), ip.c_str(), port, m_nPlatId);
		return false;
	}

	platid = m_nPlatId;
	m_mapDBPlatid[dbname] = m_nPlatId++;

	LOG_PRINT(log_info, "initDBSinkPool success: poolsize:%d, dbname:%s, platid:%d.", poolsize, dbname.c_str(), platid); 
	return true;
}

Dbconnection* Dbconn::getInstance(string dbname/* = "maindb"*/)
{
	map<string,Dbconnection* >::iterator it = instance.find(dbname);
	if(it == instance.end())
	{
		int line;
		if(!m_config.load(line))
		{
			printf("load failed\n");
		}

		string ip = m_config.getString(dbname.c_str(), "ip");
		int port = m_config.getInt(dbname.c_str(), "port");
		string user = m_config.getString(dbname.c_str(), "user");
		string password = m_config.getString(dbname.c_str(), "password");
		string name = m_config.getString(dbname.c_str(), "dbname");
		Dbconnection* conn = new Dbconnection(ip,port,name,user,password,dbname,OnDbStatusModify);
		printf("ip:%s,port:%d,user:%s,password:%s\n",ip.c_str(),port,user.c_str(),password.c_str());
		instance[dbname] = conn;
	}
	return instance[dbname];
}

void Dbconn::close(string dbname)
{
	instance[dbname]->closeConnection();
	instance.erase(dbname);
}

void Dbconn::closeAll()
{
	map<string,Dbconnection* >::iterator it;
	for(it=instance.begin();it!=instance.end();++it)
		it->second->closeConnection();
	instance.clear();
}

Dbconnection *Dbconn::getSyncDB()
{
	string dbname = vdbnames[switchindex];
	printf("get db index:%d,dbname:%s.\n", switchindex, dbname.c_str());

	if (m_syncDB.isValid())
		return &m_syncDB;

	int line;
	if(!m_config.load(line))
	{
		printf("load failed\n");
		return NULL;
	}

	string ip = m_config.getString(dbname.c_str(), "ip");
	int port = m_config.getInt(dbname.c_str(), "port");
	string user = m_config.getString(dbname.c_str(), "user");
	string password = m_config.getString(dbname.c_str(), "password");
	string name = m_config.getString(dbname.c_str(), "dbname");
	if (!m_syncDB.resetConnection(ip,port,name,user,password,dbname,OnDbStatusModify))
		return NULL;

	return &m_syncDB;
}

int Dbconn::getPlatIdAndIncrease()
{
	return m_nPlatId++;
}

//void Dbconn::OnStatusNotify(int status)
//{
//	if(status == 0)
//	{
//		//call alarm;
//		CAlarmNotify::sendAlarmNoty(e_all_notitype,e_db_connect,Application::getInstance()->getProcName(),"DB Connect Error",
//																		"Room,Yunwei","Disconnect DB:[%s]",vdbnames[switchindex].c_str());
//		switchindex++;
//		if(switchindex > 2)
//		{
//			switchindex = 0;
//		}
//	}
//}

Dbconnection::Dbconnection(string ip,int port,string dbname,string user,string password,string confname,FunHandler notify) :m_notify(notify)
{
	m_sos = NULL;
	m_dbname = confname;
	m_platid = Dbconn::getPlatIdAndIncrease();
	sink_pool_init("postgresql", m_platid, 1, ip.c_str(), port, "", dbname.c_str(), user.c_str(), password.c_str());
	sink = sink_pool_get(m_platid);
	m_sos = sink->sos;
	timerid = Timer::getInstance()->add(TIME_INTEVAL, boost::bind(&Dbconnection::keepalive,this));
}



bool Dbconnection::resetConnection(string ip,int port,string dbname,string user,string password,string confname,FunHandler notify)
{
	m_notify = notify;
	closeConnection();
	m_dbname = confname;

	if (sink == NULL)
	{
		m_platid = Dbconn::getPlatIdAndIncrease();
		if (sink_pool_init("postgresql", m_platid, 1, ip.c_str(), port, "", dbname.c_str(), user.c_str(), password.c_str()) < 0)
			return false;
		sink = sink_pool_get(m_platid);
		m_sos = sink->sos;
	}
	else
	{
		int ret = sink_reset_config(sink, m_platid, ip.c_str(), port, dbname.c_str(), user.c_str(), password.c_str());
		if (ret < 0)
		{
			LOG_PRINT(log_error, "[resetConnection]db reset fail.db info:%s:%d,dbname:%s,user:%s,pwd:%s,confname:%s.", \
				ip.c_str(), port, dbname.c_str(), user.c_str(), password.c_str(), confname.c_str());

			printf("[resetConnection]db reset fail.db info:%s:%d,dbname:%s,user:%s,pwd:%s,confname:%s.\n", \
				ip.c_str(), port, dbname.c_str(), user.c_str(), password.c_str(), confname.c_str());

			return false;
		}
		else
		{
			LOG_PRINT(log_info, "[resetConnection]db reset success.db info:%s:%d,dbname:%s,user:%s,pwd:%s,confname:%s.", \
				ip.c_str(), port, dbname.c_str(), user.c_str(), password.c_str(), confname.c_str());

			printf("[resetConnection]db reset success.db info:%s:%d,dbname:%s,user:%s,pwd:%s,confname:%s.\n", \
				ip.c_str(), port, dbname.c_str(), user.c_str(), password.c_str(), confname.c_str());
			m_sos = sink->sos;
		}
	}

	timerid = Timer::getInstance()->add(TIME_INTEVAL, boost::bind(&Dbconnection::keepalive,this));

	return true;
}

void Dbconnection::keepalive()
{
	static int resetcount = 0;
	sink_pool_keeplive(m_platid, false);
	if(sink_pool_status_ok(m_platid, 0) == 0)
	{
		resetcount = 0;
	}else
	{
		printf("resetcount:%d\n",resetcount);
		resetcount++;
	}
	if(resetcount > 3)
	{
		if(m_notify)
		{
			(*m_notify)(m_dbname);
			resetcount = 0;
		}
	}
}

const char *Dbconnection::SQL_s_user_calias_DB =
	" select calias from dks_user where nuserid=%u";

const char *Dbconnection::SQL_s_user_info_DB   =	
	" select calias,ctel from dks_user where nuserid=%u";

const char *Dbconnection::SQL_u_useraccount_DB =
	" update dks_useraccount set nk=nk-%llu where nuserid=%u";

const char *Dbconnection::SQL_s_useraccount_DB =
	" select nk from dks_useraccount where nuserid=%u";

const char *Dbconnection::SQL_s_zhuanbo_robotid_DB =
	" select nrobotid from dks_vcbzhuanboconfig where nrobotvcbid=%u and nuserid!=0";

const char *Dbconnection::SQL_s_vipprice_info_DB =
	" select id,price,deadline,vipinfoname,timetype from dks_vip_price_info where viptype = %u";

const char *Dbconnection::SQL_s_privatevip_viplevel_DB =
	" select p.viplevel,v.price from dks_private_vip_info p join dks_vip_price_info v on p.vid = v.id where p.nuserid=%u and p.teacherid=%u";

const char *Dbconnection::SQL_d_privatevip_viplevel_DB =
	" delete from dks_private_vip_info where nuserid=%u and teacherid=%u";

const char *Dbconnection::SQL_i_privatevip_viplevel_DB[3] = {
	//[DAY] =
	" insert into dks_private_vip_info(vid,nuserid,buytime,expirtiontime,teacherid,viplevel)"
	" values(%d,%u,now(),DATE_ADD(now(), INTERVAL %d DAY),%u,%u)",
	//[MONTH] =
	" insert into dks_private_vip_info(vid,nuserid,buytime,expirtiontime,teacherid,viplevel)"
	" values(%d,%u,now(),DATE_ADD(now(), INTERVAL %d MONTH),%u,%u)",
	//[YEAR] =
	" insert into dks_private_vip_info(vid,nuserid,buytime,expirtiontime,teacherid,viplevel)"
	" values(%d,%u,now(),DATE_ADD(now(), INTERVAL %d YEAR),%u,%u)"
};

const char *Dbconnection::SQL_s_privatevip_info_DB = 
	" select buytime,expirtiontime from dks_private_vip_info where nuserid=%u and vid=%u and teacherid=%u ";

const char *Dbconnection::SQL_i_user_daycost_DB =
	" insert into tbuserdaycost(nuserid, ncost, ddate)"
	" VALUES(%u, %llu, curdate())";

const char *Dbconnection::SQL_u_user_daycost_DB =
	" UPDATE tbuserdaycost SET ncost=ncost+%llu WHERE nuserid=%u and ddate=curdate()";

const char *Dbconnection::SQL_u_user_info_DB =
	"update dks_user set gender=%d, alias='%s' %s where user_id=%u";

const char *Dbconnection::SQL_u_user_info_without_introduce_DB =
	"update dks_user set gender=%d, alias='%s' where user_id=%u";

//query the userid who bought this private vip
const char *Dbconnection::SQL_s_privatevip_userid_DB =
	" select nuserid from dks_private_vip_info where teacherid=%u and viplevel>=%u ";

const char *Dbconnection::SQL_s_ptcourseinfo_DB =
	" select roomid,teamid,topic,begintime,endtime,teacheralias,studentlimit,studentnum,courseprice,introduction,preentertime,studentdefault "
	" from dks_platinumcourse where courseid=%u";

const char *Dbconnection::SQL_s_team_wechat_qrcode =
	" select qrcodeUrl,weChat from dks_teacherinfo where teacherid=%u";

const char *Dbconnection::SQL_u_vcb_incr_attention =
	" update dks_vcbinformation set total_attention=total_attention+1 where nvcbid=%u";

const char *Dbconnection::SQL_u_vcb_decr_attention =
	" update dks_vcbinformation set total_attention=total_attention-1 where nvcbid=%u and total_attention > 0";

const char *Dbconnection::SQL_s_roomid_teacherid_DB =
	" select roomid from dks_teacherinfo where teacherid=%u";

const char *Dbconnection::SQL_s_useralias_DB =
	" select calias, nviplevel from dks_user where nuserid=%u";

const char *Dbconnection::SQL_s_viptotal_DB =
	" SELECT v.total_attention from dks_teacherinfo t LEFT JOIN dks_vcbinformation v on t.roomid=v.nvcbid "
	" LEFT JOIN dks_vcbstatus a on t.roomid=a.nvcbid "
	" where v.ncheckweb=1 and a.nstatus=2 order by v.total_attention desc limit 7 ";

const char *Dbconnection::SQL_s_roominfo_DB =
	" select ui.calias,vi.cname,vi.croompic,vs.ncount"
	" from dks_vcbinformation vi"
	" join dks_vcbstatus vs"
	" on vs.nvcbid=vi.nvcbid,"
	" dks_user ui"
	" where vi.nvcbid=%u and ui.nuserid=%u";

const char *Dbconnection::SQL_s_fav_user_token_DB =
	" select ti.pushtype,ti.pushtoken"
	" from dks_uservcbfavorite uf"
	" join tb_pushtoken_inf ti"
	" on uf.nuserid = ti.nuserid"
	" where uf.nvcbid=%u";

const char *Dbconnection::SQL_s_log_ip_DB =
	" select id from as_log_ip"
	" where DAY=%s and room_id=%u and ip='%s'";

const char *Dbconnection::SQL_i_log_ip_DB =
	" insert INTO as_log_ip(DAY,room_id,ip)"
	" values(%s,%u,'%s')";

const char *Dbconnection::SQL_s_user_room_assistant =
	" select nassid,ismobile"
	" from tb_user_room_assistant"
	" where nroomid=%u and nuserid=%u";

const char *Dbconnection::SQL_u_user_room_assistant[2] = {
	//[NASSID] =
	" update tb_user_room_assistant"
	" set nassid=%u"
	" where nroomid=%u and nuserid=%u",
	//[ISMOBILE]  =
	" update tb_user_room_assistant set ismobile=%d where nroomid=%u and nuserid=%u"
};

const char *Dbconnection::SQL_i_user_room_assistant =
	" insert into tb_user_room_assistant(nroomid,nuserid,nassid,ismobile)"
	" values(%u,%u,%u,%d)";

const char *Dbconnection::SQL_s_vcbinformation_etr[2] = {
	//[0] =
	" select wechatid,qrcode,noassmsg from tbvcbinformation_external where nroomid=%u",
	//[1] =
	" select wechatid,qrcode,assmsg from tbvcbinformation_external where nroomid=%u"
};

const char *Dbconnection::SQL_u_question_surplus_DB =
	" update tb_question_profile set surplus=surplus+1 where userid=%u ";
const char *Dbconnection::SQL_i_question_surplus_DB =
	" insert into tb_question_profile (userid, surplus, count) values (%u, 1, 0)";

const char *Dbconnection::SQL_s_teacherinfo_DB =
	" select t.headUrl,t.roomid,t.introduce,t.clabel,t.clevel,t.stype,v.popularity,a.fans,case when f.nuserid is NULL then 0 else 1 end as isfans, case when s.nuserid is NULL then 0 else 1 end as issub"
	" from dks_teacherinfo t"
	" join dks_vcbinformation v"
	" on t.roomid = v.nvcbid"
	" left join dks_uservcbfavorite f"
	" on t.roomid=f.nvcbid and f.nuserid=%u"
	" left join dks_teachersubscription as s"
	" on t.teacherid=s.nteacherid and s.nuserid=%u,"
	" (select count(*) as fans from dks_uservcbfavorite v join dks_teacherinfo t on t.roomid = v.nvcbid where t.teacherid=%u GROUP BY v.nvcbid) as a"
	" where t.teacherid=%u";

const char *Dbconnection::SQL_s_teacherinfo_byID_DB	 =  
	" select a.roomid,b.calias as teamname from dks_teacherinfo a,dks_user b where a.teacherid=b.nuserid and a.teacherid=%u ";

const char *Dbconnection::SQL_s_userinfo_DB =
	" select nheadid,DATE_FORMAT(dbirthday,'%%Y%%m%%d'),cidiograph,cprovince,ccity from dks_user where nuserid=%u";

const char *Dbconnection::SQL_u_textlive_gift_DB =
	" update tb_textlivemessagehistory set giftcount=giftcount+%u where textliveid=%u";

const char *Dbconnection::SQL_i_gifttradelog_DB =
	" insert into dks_usergifttradelog(nuserid,nbuddyid,ngiftid,ncount,dtime,ngifttype,nvcbid,nusermoney,nbuddymoney)"
	" VALUES(%u,%u,%u,%u,now(),%d,%u,%llu,%llu)";

const char *Dbconnection::SQL_u_send_daygiftlog_DB =
	" update tbuserdaygiftlog"
	" set nsendcount=nsendcount+%u"
	" where nuserid=%u and ddate=curdate() and ngiftid=%u";

const char *Dbconnection::SQL_i_send_daygiftlog_DB =
	" insert into tbuserdaygiftlog(nuserid,ngiftid,nsendcount,ddate)"
	" values(%u,%u,%u,curdate())";

const char *Dbconnection::SQL_u_recv_daygiftlog_DB =
	" update tbuserdaygiftlog"
	" set nrecvcount=nrecvcount+%u"
	" where nuserid=%u and ddate=curdate() and ngiftid=%u";

const char *Dbconnection::SQL_i_recv_daygiftlog_DB =
	" insert into tbuserdaygiftlog(nuserid,ngiftid,nrecvcount,ddate)"
	" values(%u,%u,%u,curdate())";

const char *Dbconnection::SQL_i_vcb_daycost_teacher_DB =
	" insert into tbvcbdaycost(nvcbid, ncost, ddate)"
	" select roomid,%llu,curdate() from dks_teacherinfo WHERE teacherid=%u";

const char *Dbconnection::SQL_u_vcb_daycost_teacher_DB =
	" UPDATE tbvcbdaycost v"
	" INNER JOIN dks_teacherinfo t ON v.nvcbid=t.roomid"
	" SET v.ncost=v.ncost+%llu"
	" WHERE t.teacherid=%u and v.ddate=curdate()";

const char *Dbconnection::SQL_i_vcb_daycost_DB =
	" insert into tbvcbdaycost(nvcbid, ncost, ddate)"
	" VALUES( %u,%llu,curdate())";

const char *Dbconnection::SQL_u_vcb_daycost_DB =
	" UPDATE tbvcbdaycost"
	" SET ncost=ncost+%llu"
	" WHERE nvcbid=%u and ddate=curdate()";

const char *Dbconnection::SQL_i_question_DB =
	" insert into tb_questionreq (userid,teacherid,reqtime,question,stokeid,questionstype)"
	" values (%u,%u,%llu,'%s','%s',%d)";

const char *Dbconnection::SQL_s_question_id_DB =
	" select last_insert_id()";

const char *Dbconnection::SQL_s_question_count_DB =
	" select surplus from tb_question_profile where userid=%u";

const char *Dbconnection::SQL_u_question_count_DB =
	" update tb_question_profile set count=count+1,surplus=%d where userid=%u";

const char *Dbconnection::SQL_u_question_sumcount_DB =
	" update tb_question_sumcount set count=count+%d,time=FROM_UNIXTIME(%d) where id=1";

const char *Dbconnection::SQL_i_order_main_DB[3] = {
	//[0] =
	" insert into order_main (userid, nickname, total, discount, payment, createtime, paytime, state, remarks)"
	" select %u,calias,%llu,%llu,%llu,now(),now(),1,'收费问股'"
	" from dks_user"
	" where nuserid=%u",
	//[1] =
	" insert into order_main (userid, nickname, total, discount, payment, createtime, paytime, state, remarks)"
	" select %u,'%s',%llu,%llu,%llu,now(),now(),1,'有效期1年'"
	" from dks_private_vip_info"
	" where nuserid=%u and teacherid=%u",
	//[2] =
	" insert into order_main (userid, nickname, total, discount, payment, createtime, paytime, state, remarks)"
	" values(%u,'%s',%llu,%llu,%llu,from_unixtime(%u),from_unixtime(%u),1,'%s')"
};

const char *Dbconnection::SQL_i_order_details_DB[3] = {
	//[0] =
	" insert into order_details (order_mainid, productid, productname, typeid, typename, price, total, discount, payment, validitydate, remarks,objid)"
	" select last_insert_id(),%lld,'收费问股',1,'收费问股',%llu,%llu,%llu,%llu,UNIX_TIMESTAMP(now()),'收费问股',%u",
	//[1] =
	" insert into order_details (order_mainid, productid, productname, typeid, typename, price, total, discount, payment, validitydate, remarks,objid)"
	" select last_insert_id(),%d,'%s',0,'',%llu,%llu,%llu,%llu,UNIX_TIMESTAMP(now()),'有效期1年',%u"
	" from dks_private_vip_info"
	" where nuserid=%u and teacherid=%u",
	//[2] =
	" insert into order_details (order_mainid, productid, productname, typeid, typename, price, pcount, total, discount, payment, validitydate, remarks,objid)"
	" select last_insert_id(),%u,'%s',%u,'%s',%llu,%d,%llu,%llu,%llu,%u,'%s',%u"
};

const char *Dbconnection::SQL_s_zhuanbo_config_DB =
	" select nvcbid,nuserid"
	" from dks_vcbzhuanboconfig"
	" where nrobotvcbid=%u and nuserid!=0 and nrobotid=%u";

const char *Dbconnection::SQL_s_contribution_week_DB =
	" select ui.calias, sum(gl.nusermoney), ui.nheadid AS totalMoney"
	" from dks_usergifttradelog gl"
	" left join dks_user ui"
	" on gl.nuserid = ui.nuserid"
	" where nbuddyid=%u AND dtime>subdate(FROM_UNIXTIME(%d),date_format(FROM_UNIXTIME(%d) - 1,'%w'))"
	" group by gl.nuserid"
	" order by sum(gl.nusermoney) desc"
	" limit 6";

const char *Dbconnection::SQL_s_team_teamid_DB =
	" select teacherid from dks_teacherinfo where roomid=%u";

const char *Dbconnection::SQL_s_viproom_teacherid_DB =
	" select nvcbid from dks_vcbinformation where team=%u";

const char *Dbconnection::SQL_s_courseinfo_DB =
	" select topic,teamid,begintime,endtime,startlimitbuy,courseprice,studentlimit,studentnum,roomid,preentertime,studentdefault "
	" from dks_platinumcourse where courseid=%u";

const char *Dbconnection::SQL_i_ptcourse_order =
	" insert into dks_platinumcourse_order(userid,roomid,courseid,buytime) values(%u,%u,%u,from_unixtime(%u))";

const char *Dbconnection::SQL_u_ptcourse_studentnum_DB =
	" update dks_platinumcourse set studentnum=studentnum+1 where courseid=%u";
const char *Dbconnection::SQL_s_ptcourse_studentnum_DB =
	" select studentnum from dks_platinumcourse where courseid=%u";

const char *Dbconnection::SQL_i_kickoutuser_DB =
	" insert into dks_kickoutuser(nuserid,nrunnerid,nvcbid,nreason,nduration,tlogtime)"
	" values (%u,%u,%u,%d,%d,FROM_UNIXTIME(%d))";

const char *Dbconnection::SQL_s_kickoutuser_DB =
	" select UNIX_TIMESTAMP(DATE_ADD(tlogtime,INTERVAL nduration MINUTE))"
	" from dks_kickoutuser"
	" where nuserid=%u and nvcbid=%u";

const char *Dbconnection::SQL_d_kickoutuser_DB =
	" delete from dks_kickoutuser"
	" where DATE_ADD(tlogtime,INTERVAL nduration MINUTE) < FROM_UNIXTIME(%d)";

const char *Dbconnection::SQL_s_vipuserinfo_DB =
	" select distinct a.nuserid,a.teacherid,a.viplevel from dks_private_vip_info a,dks_teacherinfo b where a.teacherid != 0 and a.teacherid=b.teacherid and b.roomid=%u"
	" union all"
	" select distinct a.nuserid,a.teacherid,a.viplevel from dks_private_vip_info a,dks_vcbinformation b where a.teacherid != 0 and a.teacherid=b.team and b.nvcbid=%u";

const char *Dbconnection::SQL_s_get_one_vipuserinfo_DB =
	"select teacherid,userviplevel from("
	" select a.teacherid,max(a.viplevel) as userviplevel from dks_private_vip_info a,dks_teacherinfo b, dks_vip_price_info c"
	" where a.teacherid != 0 and a.teacherid=b.teacherid and c.id=a.vid and a.nuserid=%u and b.roomid=%u"
	" union all"
	" select a.teacherid,max(a.viplevel) as userviplevel from dks_private_vip_info a,dks_vcbinformation b, dks_vip_price_info c"
	" where a.teacherid != 0 and a.teacherid=b.team and c.id=b.jurisdiction and a.nuserid=%u and b.nvcbid=%u"
	") result where result.userviplevel is not null";

const char *Dbconnection::SQL_s_potential_VIP =
	" select expire,level from 99cj_suffice_vip_member"
	" where roomid=%u and userid=%u";

const char *Dbconnection::SQL_u_potential_VIP =
	" update 99cj_suffice_vip_member"
	" set last_time=%d"
	" where roomid=%u and userid=%u";

const char *Dbconnection::SQL_s_ptcourse_user_DB =
	" select courseid,begintime,endtime,preentertime from dks_platinumcourse "
	" where courseid in(select courseid from dks_platinumcourse_order where userid=%u and roomid=%u) and endtime > FROM_UNIXTIME(%u) order by begintime asc limit 1";

const char *Dbconnection::SQL_s_buyptcourse_DB =
	" select courseid from dks_platinumcourse_order where userid=%u and roomid=%u limit 1";

const char *Dbconnection::SQL_s_ptcourse_user_buy_DB =
	" select count(1) from dks_platinumcourse_order where userid=%u and courseid=%u";

//Course Begin
const char *Dbconnection::SQL_s_load_course_DB =
	" select a.id,a.starthour,a.endhour,b.nvcbid,a.serviceID,a.is_vip from dks_syllabus a,dks_vcbinformation b where a.serviceID=b.platformid and a.status=1 and a.endhour > FROM_UNIXTIME(%u)";

const char *Dbconnection::SQL_s_course_basicinfo_DB =
	" select a.starthour,a.endhour,a.title,a.teachername,b.nvcbid,a.is_vip from dks_syllabus a,dks_vcbinformation b where id=%u and a.serviceID=b.platformid";

const char *Dbconnection::SQL_s_new_courseinfo_by_serverid_DB =
	" select id,starthour,endhour,is_vip,title,teachername from dks_syllabus "
	" where id=(select id from dks_syllabus where serviceID=%u and endhour > FROM_UNIXTIME(%u) and status=1 order by starthour asc limit 1)";

const char *Dbconnection::SQL_s_course_by_roomid_DB =
	" select a.id,a.starthour,a.endhour,a.title,a.teachername,a.is_vip from dks_syllabus a,dks_vcbinformation b "
	" where a.serviceID=b.platformid and a.status=1 and b.nvcbid=%u and a.starthour <= FROM_UNIXTIME(%u) and a.endhour > FROM_UNIXTIME(%u) "
	" order by a.starthour asc limit 1";

const char *Dbconnection::SQL_s_get_course_noty_room_DB =
	" select nvcbid as roomid from dks_vcbinformation where platformid=%u";
//Course End

const char *Dbconnection::SQL_s_close_ptcourse_id_DB =
	" select courseid from dks_platinumcourse where endtime <= FROM_UNIXTIME(%u) and state != 4";

const char *Dbconnection::SQL_u_totalfavor_by_courseid_DB =
	" update dks_vcbinformation vcbinfo set vcbinfo.total_attention=(select count(nuserid) from dks_uservcbfavorite where nvcbid=vcbinfo.nvcbid) where vcbinfo.nvcbid=(select roomid from dks_platinumcourse where courseid=%u)";

const char *Dbconnection::SQL_d_uservcbfavor_by_courseid_DB =
	" delete dks_uservcbfavorite from dks_uservcbfavorite inner join dks_platinumcourse_order b where dks_uservcbfavorite.nuserid=b.userid and dks_uservcbfavorite.nvcbid=b.roomid and b.courseid=%u "
	" and not exists(select 1 from dks_platinumcourse_order c inner join dks_platinumcourse d "
	" where c.userid = b.userid and c.roomid = b.roomid and c.courseid = d.courseid and d.endtime > FROM_UNIXTIME(%u))";

const char *Dbconnection::SQL_s_get_is_filter_user =
	"select 1 from dks_filterlist where value='%s' and type=1";

const char *Dbconnection::SQL_s_viplist_teacherid_level_DB =
	" select a.nuserid from dks_private_vip_info a where a.teacherid = %u and a.viplevel >= %u ";

const char * Dbconnection::SQL_s_exists_vcbmanager = 
	" select nid from dks_vcbmanagermap where nvcbid=%d and nuserid=%d and nmanagertype=%d ";

const char * Dbconnection::SQL_i_addvcbmanager = 
	" insert into dks_vcbmanagermap(nvcbid,nuserid,nmanagertype) values(%d,%d,%d) ";

const char * Dbconnection::SQL_d_delvcbmanager = 
	" delete from dks_vcbmanagermap where nvcbid=%d and nuserid=%d and nmanagertype=%d ";

const char * Dbconnection::SQL_s_gettaskcoin_info_DB = 
	" select userid,coin_num,create_time,end_time,id from 99cj_task_coin where status=0 and end_time < FROM_UNIXTIME(%u)";

const char * Dbconnection::SQL_s_getuserdaycost_DB = 
	" select sum(ncost) from tbuserdaycost where nuserid=%u and ddate >='%s' and ddate <='%s' ";

const char * Dbconnection::SQL_u_useraccount_with_condition_DB = 
	" update dks_useraccount set nk = if(nk - %llu <0,0,nk - %llu) where nuserid=%u";

const char * Dbconnection::SQL_u_user_task_coin_DB = 
	" update 99cj_task_coin set status=1 where id=%u";
