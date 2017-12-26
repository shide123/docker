#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <list>

#include "RoomManager.h"

#include "clienthandler.h"
#include "CLogThread.h"
#include "Config.h"
#include "redis_def.h"
#include "time.h"
#include "DBConnection.h"
#include "redisdataHelper.h"
#include "utils.h"
#include "DBTools.h"
#include "msgcommapi.h"
#include "errcode.h"
#include "ProtocolsBase.h"
#include "CUserBasicInfo.h"
#include "CUserGroupMgr.h"


CRoomManager::CRoomManager(void)
	: m_clientconn("Frame")
	, m_softbot_index(0)
	, m_workid(0)
	, m_nsvrid(0)
	, m_QuestionAnswerNewestTime(0)
	, m_pRedisMgr(NULL)
	, m_initData(false)
	, m_softbot_min_no(0)
	, m_softbot_max_no(0)
{
}

CRoomManager::~CRoomManager(void)
{
}

int CRoomManager::init(void)
{
	int ret = -1;
	do {
		m_redis_viptotal_map.init(DB_VIPMGR, "viptotalnum", m_pRedisMgr, SYNC);

		string2set(m_WhisperRoomLevels, whisperRoomlevelset);
		string2set(m_WhisperLevels, whisperlevelset);
		string2set(m_WhisperUserIDs, whisperUserIDset);

		string2set(m_FreelabaRoomLevels, labaRoomlevelset);
		string2set(m_FreelabaLevels, labalevelset);

		string2set(m_noroomkeyRoomLevels, noroomkeyRoomlevelset);
		string2set(m_noroomkeyLevels, noroomkeylevelset);

	//	string2set(m_logonmultiroomRoomLevels, logonmultiroomRoomlevelset);
	//	string2set(m_logonmultiroomLevels, logonmultiroomlevelset);

		load_softbot_alias(m_szSoftbotaliasfile.c_str());
		init_softbotids();

		initwaitmicspeciality();
// 		if (!m_workid)
// 		{
// 			LOG_PRINT(log_info, "Because multi processors mode,only one processor in one area need to load room info to redis is ok.");
// 			if (LoadActiveRooms() < 0)
// 				break;
// 		}

//		if (LoadPermissionFile() < 0)
//			break;
//		if (LoadZhuanboRebot() < 0)
//			break;
//		if (LoadVipTotalNum() < 0)
//			break;
		ret = 0;
	} while (0);

	return ret;
}

void CRoomManager::finishInitData()
{
	m_initData = true;
}

void CRoomManager::setConfSetting(Config &config, string configPath)
{
	m_uNonLiveTimeout = config.getInt("self", "nonlivetimeout");
	if (0 == m_uNonLiveTimeout)
		m_uNonLiveTimeout = 900;

	// 机器人配置
	m_softbot_min_no = config.getInt("softbot", "min_no");
	m_softbot_max_no = config.getInt("softbot", "max_no");

	if (configPath.size()) 
	{
		m_szPermissionFilePath = configPath + "/roompermission.conf";
		 m_szSoftbotaliasfile = configPath + "/softbot_alias.txt";
	}
}

void CRoomManager::setRedisConf(redisMgr *pRedisMgr)
{
	m_pRedisMgr = pRedisMgr;
}

int CRoomManager::load_permission_config(const char *confile)
{
	if (!confile || *confile == 0)
		return -1;

	int line;
	Config config(confile);
	if(!config.load(line)){
		std::cerr << "load configure file roompermission.conf failed\n";
		return -1;
	}

	m_szPermissionFilePath = confile;
	m_szPtime = config.getString("time","ptime");
	//悄悄话
	m_WhisperRoomLevels = config.getString("Whisper","roomlevels");
	m_WhisperLevels = config.getString("Whisper","levels");
	m_WhisperUserIDs = config.getString("Whisper","userid");
	//免费喇叭
	m_bsiege_laba = config.getInt("laba","bSiege");
	m_FreelabaRoomLevels = config.getString("laba","roomlevels");
	m_FreelabaLevels = config.getString("laba","levels");
	//无须房间密码
	m_bsiege_noroomkey = config.getInt("roomkey","bSiege");
	m_noroomkeyRoomLevels = config.getString("roomkey","roomlevels");
	m_noroomkeyLevels = config.getString("roomkey","levels");
//	//登录多个房间
//	m_bsiege_logonmultiroom = config.getInt("logonmultiroom","bSiege");
//	m_logonmultiroomRoomLevels = config.getString("logonmultiroom","roomlevels");
//	m_logonmultiroomLevels = config.getString("logonmultiroom","levels");
	//md5
	m_szPFileMD5 = config.getString("PFileMD5","md5");

	return 0;
}

redisOpt* CRoomManager::getDataRedis()
{
	if (m_pRedisMgr)
		return m_pRedisMgr->getOne();

	return NULL;
}

void CRoomManager::loadKeyword()
{
	int recnum = 0;
	const int linenum = 10000;
	int offset = 0;

	WRITE_LOCK(m_mtxKeywordMap);
    m_keyword_map.clear();

    vector<CMDAdKeywordInfo_t> vecAdKeyWord;
	while(1){
		recnum = CDBSink().getAdKeywordList(offset, linenum, vecAdKeyWord);
		vector<CMDAdKeywordInfo_t>::iterator it = vecAdKeyWord.begin();
		for (; it != vecAdKeyWord.end(); it++) {
			m_keyword_map[it->keyword] = (*it);
		}
		if (recnum < linenum)
			break;
		offset += recnum;
	}
}

bool CRoomManager::isMainRoom(uint32 vcbid)
{
	bool bret = false;
	m_zhuanbo_robots_mutex.lock();
	for(size_t i=0; i<m_add_zhuanbo_robots.size(); i++)
	{
		if(m_add_zhuanbo_robots[i].vcbid==vcbid && m_add_zhuanbo_robots[i].robotvcbid == vcbid){
			bret = true;
			break;
		}
	}
	m_zhuanbo_robots_mutex.unlock();
	return bret;
}

uint32 CRoomManager::getMainRoom(uint32 nrobotid)
{
	uint32 main_roomid = 0;
	m_zhuanbo_robots_mutex.lock();
	for(size_t i=0; i<m_add_zhuanbo_robots.size(); i++)
	{
		if(m_add_zhuanbo_robots[i].robotid == nrobotid){
			main_roomid = m_add_zhuanbo_robots[i].vcbid;
			break;
		}
	}
	m_zhuanbo_robots_mutex.unlock();
	return main_roomid;
}

uint CRoomManager::makeTeacherRobotPack(CRoomObj *pRoomObj, CMDRoborTeacherIdNoty_t* pInfo)
{
	uint num = 0;
	RoomObject_Ref roomObjRef;
	UserObject_Ref userObjRef;

	if (!pRoomObj || !pInfo)
		return num;

	m_zhuanbo_robots_mutex.lock();
	for(size_t i=0; i<m_add_zhuanbo_robots.size(); i++)
	{
		uint32 teacherid = 0;
		if(m_add_zhuanbo_robots[i].vcbid != pRoomObj->nvcbid_ && m_add_zhuanbo_robots[i].robotvcbid == pRoomObj->nvcbid_){
			pInfo->roborid = m_add_zhuanbo_robots[i].robotid;
			pInfo->vcbid = pRoomObj->nvcbid_;
			roomObjRef = FindRoom(m_add_zhuanbo_robots[i].vcbid);
			if (roomObjRef.get())
				teacherid = roomObjRef->getUserIsOnMic();
			pInfo->teacherid = teacherid;

			if (0 == teacherid){
		    	if (pRoomObj->findVisitUser(userObjRef, pInfo->roborid)){
		    		strcpy(pInfo->teacheralias, userObjRef->calias_.c_str());
		    	}
			}
			else{
				if (roomObjRef->findVisitUser(userObjRef, teacherid)){
					strcpy(pInfo->teacheralias, userObjRef->calias_.c_str());
				}
			}
			LOG_PRINT(log_info, "[teacherid=%u, main roomid=%u, sub roomid:%u, roborid=%u]", teacherid, m_add_zhuanbo_robots[i].vcbid, pRoomObj->nvcbid_, pInfo->roborid);
			num++;
			pInfo++;
		}
	}
	m_zhuanbo_robots_mutex.unlock();
	return num;
}

int CRoomManager::LoadZhuanboRebot()
{
	Result res;
	int rows = 0;
	char query[512];
	sprintf(query, "select nid,nvcbid,npubmicidx,nrobotid,nrobotviplevel,nrobotvcbid,nstatus,nuserid from dks_vcbzhuanboconfig where nstatus=1");
	CDBSink db;
	if (!db.build_sql_run(&res, &rows, query) || rows <= 0) {
		LOG_PRINT(log_error, "[rows:%d] failed to exec sql: %s", rows, query);
		return -1;
	}
	m_add_zhuanbo_robots.clear();
	for (int i=0; i<rows; i++) {
		RoomZhuanboRobotConfig_t newitem;
		memset(&newitem, 0, sizeof(newitem));
		db.GetFromRes(newitem.nid, &res, i, 0);
		db.GetFromRes(newitem.vcbid, &res, i, 1);
		db.GetFromRes(newitem.pubmicindex, &res, i, 2);
		db.GetFromRes(newitem.robotid, &res, i, 3);
		db.GetFromRes(newitem.robotviplevel, &res, i, 4);
		db.GetFromRes(newitem.robotvcbid, &res, i, 5);
		db.GetFromRes(newitem.robotstatus, &res, i, 6);
		db.GetFromRes(newitem.userid, &res, i, 7);
		m_add_zhuanbo_robots.push_back(newitem);


		RoomObject_Ref pRoomObjRef = FindRoom(newitem.robotvcbid);
        if(newitem.vcbid != newitem.robotvcbid && pRoomObjRef && !pRoomObjRef->findVisitUser(newitem.robotid))  //只有新增加的机器人
		{
        	AndroidParam_t Data3 = {0};
			Data3.carid =0;
			Data3.lifetime =0;
			Data3.ngender =0;
			Data3.roomlevel =0;
			strcpy(Data3.szcarname, "");
			sprintf(Data3.szuseralias, "转播员%d", newitem.pubmicindex);
			Data3.userid =newitem.robotid;
			Data3.vcbid =newitem.robotvcbid;
			Data3.viplevel =newitem.robotviplevel;
			pRoomObjRef->newRobotUser(Data3);
		}
	}
	LOG_PRINT(log_info, "the size of m_add_zhuanbo_robots: %u", m_add_zhuanbo_robots.size());

	return 0;
}

void CRoomManager::setRoommicstate(int vcbid, int userid, int pubmicindex, const char* szalias, int action)
{
	RoomObject_Ref pRoomObjRef = FindRoom(vcbid);

	std::string strHead;
	std::string strAlias;
	CUserBasicInfo::getUserHeadAndAlias(userid, strHead, strAlias);

	if(pRoomObjRef.get() != 0) {
		//自动上线，下线
		{
			char szOutBuf[1024] = {0};
			COM_MSG_HEADER* pOutMsg=(COM_MSG_HEADER*)szOutBuf;
			pOutMsg->version = MDM_Version_Value;
			pOutMsg->checkcode = CHECKCODE;
			pOutMsg->maincmd = MDM_Vchat_Room;
			pOutMsg->subcmd = Sub_Vchat_SetMicStateReq;
			CMDUserMicState pOutCmd;
			if(action == 1)
			{
				pOutCmd.set_micindex(-1);
				pOutCmd.set_micstate(FT_ROOMUSER_STATUS_PUBLIC_MIC);
				pOutCmd.set_giftid(0);
				pOutCmd.set_giftnum(0);
			}
			else
			{
				pOutCmd.set_micindex(-1);
				pOutCmd.set_micstate(0);
			}
			pOutCmd.set_vcbid(vcbid);
			pOutCmd.mutable_runid()->set_userid(userid);
			pOutCmd.mutable_runid()->set_head(strHead);
			pOutCmd.mutable_runid()->set_alias(strAlias);

			pOutCmd.mutable_toid()->set_userid(userid);
			pOutCmd.mutable_toid()->set_head(strHead);
			pOutCmd.mutable_toid()->set_alias(strAlias);
			
			pOutMsg->length = SIZE_IVM_HEAD_TOTAL + pOutCmd.ByteSize();
			pOutCmd.SerializeToArray(pOutMsg->content + SIZE_IVM_CLIENTGATE, pOutCmd.ByteSize());
			//消息使用task模拟处理
			if (m_clientconn.get_connecthandle()->isconnected())
				m_clientconn.get_connecthandle()->write_message(szOutBuf, pOutMsg->length);
		}
	}
}
void CRoomManager::set_subroom_micstate(int vcbid, int userid, int pubmicindex, const char* szalias, int action)
{
	LOG_PRINT(log_info, "the size of m_add_zhuanbo_robots: %u", m_add_zhuanbo_robots.size());
	for(size_t i=0; i<m_add_zhuanbo_robots.size(); i++)
	{
		if(m_add_zhuanbo_robots[i].vcbid == vcbid && m_add_zhuanbo_robots[i].pubmicindex == pubmicindex)
		{
			if (m_add_zhuanbo_robots[i].robotvcbid == vcbid)
				continue;

			RoomObject_Ref pRoomObjRef = FindRoom(m_add_zhuanbo_robots[i].robotvcbid);
			if(pRoomObjRef.get()!=0) {

				//自动上线，下线
				{
					char szOutBuf[512]={0};
					COM_MSG_HEADER* pOutMsg=(COM_MSG_HEADER*)szOutBuf;
					pOutMsg->version =10;
					pOutMsg->checkcode =0;
					pOutMsg->maincmd =MDM_Vchat_Room;
					pOutMsg->subcmd =Sub_Vchat_SetMicStateReq;
					CMDUserMicState_t* pOutCmd=(CMDUserMicState_t*)(pOutMsg->content + SIZE_IVM_CLIENTGATE);
					if(action ==1) 
					{
						pOutCmd->micindex = -1;
						pOutCmd->micstate = FT_ROOMUSER_STATUS_PUBLIC_MIC;
						pOutCmd->runid = m_add_zhuanbo_robots[i].robotid;
						pOutCmd->toid = m_add_zhuanbo_robots[i].robotid;
						pOutCmd->vcbid = m_add_zhuanbo_robots[i].robotvcbid;
						pOutCmd->giftid = 0;
						pOutCmd->giftnum = 0;
					}
					else
					{
						pOutCmd->micindex = -1;
						pOutCmd->micstate = 0;
						pOutCmd->runid = m_add_zhuanbo_robots[i].robotid;
						pOutCmd->toid = m_add_zhuanbo_robots[i].robotid;
						pOutCmd->vcbid = m_add_zhuanbo_robots[i].robotvcbid; 
					}
					pOutMsg->length = sizeof(COM_MSG_HEADER) + SIZE_IVM_CLIENTGATE + sizeof(CMDUserMicState_t);
					//消息使用task模拟处理
					if (m_clientconn.get_connecthandle()->isconnected())
						m_clientconn.get_connecthandle()->write_message(szOutBuf, pOutMsg->length);
				}
			}
		}
	}

}

int CRoomManager::LoadActiveRooms(uint32 vcbid, bool create/* = false*/, const std::string host/* = std::string("")*/)
{
	std::list<stRoomFullInfo> oDBRecords;
	CDBSink *pSink = NULL;
	if (!host.empty())
	{
		pSink = CDBSink::getSinkByHost(host);
		if (NULL == pSink)
		{
			LOG_PRINT(log_error, "input host[%s] can not reach the db sink.", host.c_str());
		}
	}

	if (0 == vcbid) 
	{
		int ret = -1;
		if (pSink)
		{
			ret = pSink->getActiveGroupfullinfo_DB(oDBRecords);
			delete pSink;
		}

		if (-1 == ret && -1 == CDBSink().getActiveGroupfullinfo_DB(oDBRecords))
		{
			LOG_PRINT(log_error, "load active group failed.");
			return -1;
		}
	}
	else 
	{
		stRoomFullInfo record;
		int ret = -1;
		if (pSink)
		{
			ret = pSink->getOneGroupfullinfo_DB(record, vcbid);	
			delete pSink;
		}
		if (-1 == ret && -1 == CDBSink().getOneGroupfullinfo_DB(record, vcbid))
		{
			LOG_PRINT(log_error, "load active group failed.groupid:%u.", vcbid);
			return -1;
		}
		else
		{
			oDBRecords.push_back(record);
		}
	}

	std::list<stRoomFullInfo>::iterator iter_lst = oDBRecords.begin();
	for (; iter_lst != oDBRecords.end(); ++iter_lst) 
	{
		RoomObject_Ref roomobjRef = FindRoom_Redis(vcbid);
		if (!roomobjRef)
		{
			roomobjRef.reset(new CRoomObj(this));
		}

		roomobjRef->resetInfo(*iter_lst);
		

		roomobjRef->redisInitRoomInfo();
		CUserGroupMgr::loadMember2Redis(iter_lst->ncourseid);
		roomobjRef->initGroupUserCount();

		//broadcast user roletype change.
		if (create && roomobjRef->group_master)
		{
			roomobjRef->castSendUserPriorityChange(roomobjRef->group_master, e_OwnerRole);
		}
		if(iter_lst->ncourseid > BROADCAST_ROOM_BEGIN_NUM)
		{
			int timerid = Timer::getInstance()->add(15 * 60, boost::bind(&CRoomManager::OnSilenceTimer,this,iter_lst->ncourseid));
			TipGiven tg;
			tipUserinfo info;
			std::list<tipUserinfo> infolst;
			int groupid = iter_lst->ncourseid - BROADCAST_ROOM_BEGIN_NUM;
			CDBSink().getTotalTipAmount(groupid,tg.lstinfo);
			for(std::list<tipUserinfo>::iterator itn = tg.lstinfo.begin() ; itn != tg.lstinfo.end() ; itn ++)
			{
				itn->srcname = CUserBasicInfo::getUserAlias(itn->srcid);
				itn->srchead = CUserBasicInfo::getUserHeadAddr(itn->srcid);
				LOG_PRINT(log_info, "OnSilenceTimer init groupid[%d].itn->srcname[%s],itn->srchead[%s],srcid[%d],amonut[%d]",
						iter_lst->ncourseid,itn->srcname.c_str(),itn->srchead.c_str(),itn->srcid,itn->amount);
			}
			tg.timerid = timerid;
			m_roomid2Appre[iter_lst->ncourseid] = tg;
			LOG_PRINT(log_info, "OnSilenceTimer init groupid[%d].",iter_lst->ncourseid);
		}

	}

	return 0;
}
void CRoomManager::OnSilenceTimer(int32_t groupid)
{
	RoomObject_Ref pRoomObjRef = FindRoom(groupid);
	if(pRoomObjRef.get())
	{
		std::map<int32_t,TipGiven>::iterator it = m_roomid2Appre.find(groupid);
		char szBuf[1024] = {0};
		ClientGateMask_t* pClientGate;
		CMDSilenceNoty noty;
		noty.set_groupid(groupid);
		for(list<tipUserinfo>::iterator it1 = it->second.lstinfo.begin(); it1 != it->second.lstinfo.end() ; it1 ++)
		{
			CMDSilenceNoty_tipUser* user = noty.add_user();
			user->set_srcname(it1->srcname);
			user->set_srchead(it1->srchead);
			user->set_gifname(it1->gifname);
			user->set_gifpicture(it1->picname);
			LOG_PRINT(log_info, "OnSilenceTimer groupid[%d].srcname:[%s],srchead[%s],amount[%d].",groupid,it1->srcname.c_str(),it1->srchead.c_str(),it1->amount);
		}
		SERIALIZETOARRAY_BUF(noty,respbuf,len);
		int nMsgLen= Build_NetMsg(szBuf,1024,MDM_Vchat_Room,Sub_Vchat_SilenceNoty,&pClientGate,respbuf,len);
		if(nMsgLen > 0)
			pRoomObjRef->castSendMsgXXX((COM_MSG_HEADER*)szBuf);
		LOG_PRINT(log_info, "OnSilenceTimer groupid[%d].size:[%d]",groupid,it->second.lstinfo.size());
	}
}
int CRoomManager::procTip(const int32_t srcid, const int32_t toid, const int32_t groupid, const std::string& gifname, const int32_t tiptype,
	const std::string& srcname, const std::string& toname, const std::string& gifpicture)
{
	int ret = 0;
	RoomObject_Ref pRoomObjRef = FindRoom(groupid);
	if(pRoomObjRef.get())
	{
		TipGiven tipgiven;
		//silence
		std::map<int32_t,TipGiven>::iterator it = m_roomid2Appre.find(groupid);
		if(it != m_roomid2Appre.end())
		{
			tipgiven = it->second;
			Timer::getInstance()->del(it->second.timerid);
			m_roomid2Appre.erase(it);
		}
		int timerid = Timer::getInstance()->add(15 * 60, boost::bind(&CRoomManager::OnSilenceTimer,this,groupid));
		tipgiven.timerid = timerid;
		tipUserinfo info;
		info.srcname = srcname;
		info.gifname = gifname;
		info.picname = gifpicture;
		//info.amount += amount;
		info.srchead = CUserBasicInfo::getUserHeadAddr(srcid);
		list<tipUserinfo>::iterator it1 = find(tipgiven.lstinfo.begin(),tipgiven.lstinfo.end(),info);
		if(it1 != tipgiven.lstinfo.end())
		{
			//info.amount += it1->amount;
			tipgiven.lstinfo.erase(it1);
		}
		tipgiven.lstinfo.push_front(info);
		if(tipgiven.lstinfo.size() > 20)
		{
			tipgiven.lstinfo.pop_back();
		}
		m_roomid2Appre[groupid] = tipgiven;
		LOG_PRINT(log_info, "proc_give_tip OnSilenceTimer end,srcname[%s]",srcname.c_str());

		ret = 1;
	}
	LOG_PRINT(log_info, "proc_give_tip noty end");
	return ret;
}
int CRoomManager::LoadDelRoom(uint32 vcbid)
{
	RoomObject_Ref proomObjRef = FindRoom(vcbid);
	if (!proomObjRef) {
		LOG_PRINT(log_error, "not found room:%u", vcbid);
		return -1;
	}
	proomObjRef->close();
	return 0;
}

void CRoomManager::LoadModRoom(uint32 vcbid, const std::string host/* = std::string("")*/)
{
	RoomObject_Ref oldRoomObjRef = FindRoom_Redis(vcbid);
	RoomObject_Ref roomobjRef = FindRoom_Redis(vcbid);
	FindRoom_DB(vcbid, roomobjRef, true, host);
	if (!oldRoomObjRef || !roomobjRef)
	{
		LOG_PRINT(log_error, "load mod group failed.groupid:%u.", vcbid);
		return;
	}

	if (oldRoomObjRef)
	{
		if (oldRoomObjRef->name != roomobjRef->name 
		|| oldRoomObjRef->icon != roomobjRef->icon
		|| oldRoomObjRef->remark != roomobjRef->remark)
		{
			LOG_PRINT(log_info, "notify room base info modify, room[%u:%s] icon[%s].", vcbid, roomobjRef->name.c_str(), roomobjRef->icon.c_str());
			roomobjRef->castSendRoomBaseInfo();
		}
	}
}

bool CRoomManager::IsRoomExist(int vcbId)
{
   bool bFound=false;

   redisOpt *pRedis = m_pRedisMgr->getOne();
   if (!pRedis) {
	   LOG_PRINT(log_error, "pRedis is NULL");
	   return false;
   }
   char szKey[32];
   sprintf(szKey, "%s:%u", KEY_HASH_USER_INFO, vcbId);
   if (0 == pRedis->redis_existkey(szKey))
	   bFound = true;

   return bFound;
}

void CRoomManager::updateRoomInfo(uint32 vcbid, int action)
{
	if (m_workid)
	{
		LOG_PRINT(log_info, "Not need to handle update room info.groupid:%u,action:%d,workid:%u.", vcbid, action, m_workid);
		return;
	}

	LOG_PRINT(log_info, "handle update room info.groupid:%u,action:%d,workid:%u.", vcbid, action, m_workid);
	switch(action){
	case 1:	/*启动房间 房间状态为1*/
		LoadActiveRooms(vcbid);
		break;
	case 2: /*修改房间 房间状态为2*/
		LoadModRoom(vcbid);
		break;
	case 3: /*停止房间 房间状态为-1*/
		{
			LoadDelRoom(vcbid);
		}
		break;
	case 4: /*更新转播信息*/
		LoadZhuanboRebot();
		break;
	default:
		LOG_PRINT(log_warning, "should not be %d operation on room:%u", action, vcbid);
	}

	if (1 == action || 2 == action){
		redisSetRoomid(vcbid);
		RoomObject_Ref pRoomObjectRef = FindRoom(vcbid);
		if (pRoomObjectRef)
			pRoomObjectRef->redisSetRoomInfo();
	}
}

vector<string> CRoomManager::getAllRoomid()
{
	redisOpt *pRedis = m_pRedisMgr->getOne();
	vector<string> vecRoomid;
	string key = KEY_SET_ROOMIDS;
	if (pRedis->redis_smembers(key, vecRoomid) < 0){
		LOG_PRINT(log_info, "Failed to get roomids from redis.");
	}
	return vecRoomid;
}

RoomObject_Ref CRoomManager::FindRoom(uint32 vcbId, bool updateFromDB/* = false*/)
{
	RoomObject_Ref roomObj = FindRoom_Redis(vcbId);

	if (!roomObj || updateFromDB)
	{
		roomObj = FindRoom_DB(vcbId, roomObj);//同时会更新redis room信息
	}

	return roomObj;
}

RoomObject_Ref CRoomManager::FindRoom_Redis(uint32 vcbId)
{
	RoomObject_Ref roomObjRef;

	do {
		redisOpt *pRedis = getDataRedis();
		if (!pRedis) {
			break;
		}

		vector<string> vecfield, vecvalue;
		std::stringstream ss;
		std::string key;
		ss << KEY_HASH_ROOM_INFO << ":" << vcbId;
		key = ss.str();

		pRedis->redis_hgetall(key, vecfield, vecvalue);
		if (!vecvalue.empty()) {
			roomObjRef.reset(new CRoomObj(vecfield, vecvalue, this));
			roomObjRef->setSvrid(m_nsvrid);
		}
	}while(0);

	return roomObjRef;
}

RoomObject_Ref CRoomManager::FindRoom_DB(uint32 vcbid, RoomObject_Ref oldRoomObj/* = RoomObject_Ref()*/, bool writeRedis/* = true*/, const std::string host/* = std::string("")*/)
{
	RoomObject_Ref roomObj;
	stRoomFullInfo record = { 0 };
	CDBSink *pSink = NULL;
	if (!host.empty())
	{
		pSink = CDBSink::getSinkByHost(host);
		if (NULL == pSink)
		{
			LOG_PRINT(log_error, "input host[%s] can not reach the db sink.", host.c_str());
		}
	}
	int ret = -1;
	if (pSink)
	{
		ret = pSink->getOneGroupfullinfo_DB(record, vcbid);
		delete pSink;
	}
	if (-1 == ret && -1 == CDBSink().getOneGroupfullinfo_DB(record, vcbid))
	{
		LOG_PRINT(log_error, "FindRoom_DB failed.groupid:%u.", vcbid);
		return roomObj;
	}

	if (oldRoomObj)
		roomObj = oldRoomObj;
	else
		roomObj.reset(new CRoomObj(this));

	roomObj->resetInfo(record);
	LOG_PRINT(log_debug, "FindRoom_DB  groupid:%u. virtual_num:%u ", vcbid, record.virtual_num);
	if (writeRedis)
	{
		roomObj->redisInitRoomInfo();
		if (!oldRoomObj)
			CUserGroupMgr::loadMember2Redis(vcbid);
	}

	return roomObj;
}

void CRoomManager::initwaitmicspeciality(void)
{
	LevelWaitMicSpeciality_t  vip_micspecs[] ={
		{1,  0, 0},
		{2,  1, 0},

		{11, 1, 0},
		{12, 1, 0},
		{13, 1, 0},
		{14, 1, 0},
		{15, 1, 0},
		{16, 1, 0},
		{17, 1, 0},
		{18, 1, 0},
		{19, 1, 0},

		{21, 2, 1},
		{22, 2, 1},
		{23, 2, 1},
		{24, 2, 1},
		{25, 2, 1},
		{26, 2, 1},
		{27, 2, 1},
		{28, 2, 1},
		{29, 2, 1},

		{31, 3, 1},
		{32, 4, 1},
		{33, 5, 1},
		{34, 6, 1},
		{35, 7, 1},
		{36, 8, 1}
	};

	LevelWaitMicSpeciality_t yiyuan_micspecs[] = {
		{1, 1, 0},
		{2, 1, 0},
		{3, 1, 0},
		{4, 2, 0},
		{5, 2, 0},
		{6, 3, 0},
		{7, 3, 0},
		{8, 4, 1},
		{9, 4, 1},

		{11, 1, 0},
		{12, 1, 0},
		{13, 1, 0},
		{14, 2, 0},
		{15, 2, 0},
		{16, 3, 0},
		{17, 3, 0},
		{18, 4, 1},
		{19, 4, 1},
	};

	size_t i;
	for(i=0; i<sizeof(vip_micspecs)/sizeof(vip_micspecs[0]); i++)
	{
		m_viplevel_waitmic_speciality[vip_micspecs[i].levelid] = vip_micspecs[i];
	}
	for(i=0; i<sizeof(yiyuan_micspecs)/sizeof(yiyuan_micspecs[0]); i++)
	{
		m_yiyuanlevel_waitmic_speciality[yiyuan_micspecs[i].levelid] = yiyuan_micspecs[i];
	}
}

int CRoomManager::load_softbot_alias(const char* filepath)
{
	FILE* pfile = fopen(filepath, "r");
	if (!pfile) {
		LOG_PRINT(log_error, "failed to open file %s", filepath);
		return -1;
	}
	char buf[128] = {0};
	while (fgets(buf, sizeof(buf), pfile)) {
		if (0 == *buf || 0 == strlen(buf))
			continue;
		char *p = strrchr(buf, '\n');
		if (p)
			*p = 0;

		if (0 != strlen(buf))
			m_vecSoftbotAlias.push_back(buf);
	}

	fclose(pfile);
	return 0;
}

void CRoomManager::updateRoomPopularity()
{
	LOG_PRINT(log_debug, "it is time to updateRoomPopularity.");

	std::map<uint32, uint32 > m_room_popularity;
	m_room_popularity.clear();

	vector<string> vecRoomid = getAllRoomid();
	for (size_t i = 0; i < vecRoomid.size(); ++i) 
	{
		uint32 vcbid = atoi(vecRoomid[i].c_str());
		/*if(vcbid < BROADCAST_ROOM_BEGIN_NUM) 
			continue;*/
		LOG_PRINT(log_debug, "it is time to updateRoomPopularity.vcbid[%d]",vcbid);
		RoomObject_Ref roomobjRef = FindRoom(vcbid);
		if (roomobjRef)
		{
			m_room_popularity[vcbid] = roomobjRef->getVisitorNum();
		}
	}

	std::map<uint32, uint32 >::iterator iter = m_room_popularity.begin();
	for (; iter != m_room_popularity.end(); ++iter)
	{
		std::string date = getTime("%Y%m%d%H");
		LOG_PRINT(log_debug, "it is time to updateRoomPopularity.update vcbid[%d]",iter->first);
		DBmongoConnection::updateRoomCount(iter->first,atoi(date.c_str()),iter->second);

		//如果大于10亿则为talk_live,否则更新talk_course
		if (iter->first>BROADCAST_ROOM_BEGIN_NUM)
		{
			long roomid =  iter->first - BROADCAST_ROOM_BEGIN_NUM;

			CDBSink().updateOnlineRoomUserCount(roomid, iter->second);
		}
		else
		{
			CDBSink().updateOnlineCourseUserCount(iter->first, iter->second);
		}
	}
}

void CRoomManager::CastSendMsgXXX(COM_MSG_HEADER* pOutMsg)
{
	CastSendMsgXXX_i(pOutMsg);
}

void CRoomManager::CastSendMsgXXX_i(COM_MSG_HEADER* pOutMsg)
{
	vector<string> vecRoomid = getAllRoomid();
	for (size_t i=0; i<vecRoomid.size(); i++) {
		uint32 vcbid = atoi(vecRoomid[i].c_str());
		RoomObject_Ref roomobjRef = FindRoom(vcbid);
		if (roomobjRef)
			roomobjRef->castSendMsgXXX(pOutMsg);
	}
}

int CRoomManager::Build_NetMsg(char * szBuf, int nBufLen, int mainCmdId, int subCmdId, ClientGateMask_t ** ppGateMask, void * pData, int pDataLen, int reqid)
{
	if(szBuf == 0 || pData == 0)
	{
		LOG_PRINT(log_error, "(szBuf==0 || pData==0).");
		return -1;
	}
	int nMsgLen = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + pDataLen;
	if(nBufLen <= nMsgLen)
	{
		LOG_PRINT(log_error, "(nBufLen <= nMsgLen).mainCmd:%d,subCmd:%d.", mainCmdId, subCmdId);
		return -1;
	}

	COM_MSG_HEADER* pHead=(COM_MSG_HEADER*)szBuf;
	pHead->version=MDM_Version_Value;
	pHead->checkcode=CHECKCODE;
	pHead->maincmd=mainCmdId;
	pHead->subcmd=subCmdId;
	pHead->reqid = reqid;
	ClientGateMask_t* pClientGate=(ClientGateMask_t*)(pHead->content);
	memset(pClientGate,0,sizeof(ClientGateMask_t));
	void* pContent=(void*)(pHead->content + SIZE_IVM_CLIENTGATE);
	memcpy(pContent,pData,pDataLen);
	pHead->length= SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + pDataLen;
	(*ppGateMask)= pClientGate;
    return pHead->length;
}

int CRoomManager::LoadPermissionFile()
{
	int line;
	Config config_permission(m_szPermissionFilePath);
	if(!config_permission.load(line)){
		LOG_PRINT(log_error, "load configure file roompermission.conf failed");
		return -1;
	}
	std::string tmp_ptime = config_permission.getString("time","ptime");
	if(tmp_ptime == m_szPtime) {
		return 0; //不需要修改
	}
	else
	{
		m_szPtime = config_permission.getString("time","ptime");
		//悄悄话
		m_WhisperRoomLevels = config_permission.getString("Whisper","roomlevels");
		m_WhisperLevels = config_permission.getString("Whisper","levels");
		m_WhisperUserIDs = config_permission.getString("Whisper","userid");

		//免费喇叭
		m_bsiege_laba = config_permission.getInt("laba","bSiege");
		m_FreelabaRoomLevels = config_permission.getString("laba","roomlevels");
		m_FreelabaLevels = config_permission.getString("laba","levels");

		//无须房间密码
		m_bsiege_noroomkey = config_permission.getInt("roomkey","bSiege");
		m_noroomkeyRoomLevels = config_permission.getString("roomkey","roomlevels");
		m_noroomkeyLevels = config_permission.getString("roomkey","levels");

		//登录多个房间
//		m_bsiege_logonmultiroom = config_permission.getInt("logonmultiroom","bSiege");
//		m_logonmultiroomRoomLevels = config_permission.getString("logonmultiroom","roomlevels");
//		m_logonmultiroomLevels = config_permission.getString("logonmultiroom","levels");

		//md5
		m_szPFileMD5 = config_permission.getString("PFileMD5","md5");

		string2set(m_WhisperRoomLevels, whisperRoomlevelset);
		string2set(m_WhisperLevels, whisperlevelset);
		string2set(m_WhisperUserIDs, whisperUserIDset);

		string2set(m_FreelabaRoomLevels, labaRoomlevelset);
		string2set(m_FreelabaLevels, labalevelset);

		string2set(m_noroomkeyRoomLevels, noroomkeyRoomlevelset);
		string2set(m_noroomkeyLevels, noroomkeylevelset);

//		string2set(m_logonmultiroomRoomLevels, logonmultiroomRoomlevelset);
//		string2set(m_logonmultiroomLevels, logonmultiroomlevelset);
	}
	return 0;
}

int CRoomManager::string2set(std::string &lst, std::set<int> &st) 
{
	st.clear();
	std::string str = lst + ",";
	while(true){
		std::string::size_type idx = str.find(",");
		if(idx == std::string::npos)
			break;
		std::string s = str.substr(0, idx);
		str = str.substr(idx + 1);
		if(s.length() == 0)
			continue;
		int id = atoi(s.c_str());
		if(id > 0)
			st.insert(id);
	}
	return 0;
}


int CRoomManager::set2string(std::set<int> &st, std::string &lst)
{
	char szTmp[128];
	lst.clear();
	std::set<int>::iterator iter = st.begin();
	for(; iter != st.end(); ++iter){
		sprintf(szTmp, "%d,", (*iter));
		lst += szTmp;
	}

	return 0;
}

//CUserObj* CRoomManager::CreateUserFromDB(int vcbid, int userid, char * pszUserPwd, int loginsource, unsigned int devtype)
//{
//	int nret = 0;
//	int nstarflag = 0, nactivityflag = 0, nRoomMgrType = RoomMgrType_Null;
//
//	if((pszUserPwd == 0 || strlen(pszUserPwd) < 1) && 0 == loginsource)
//	{
//		printf("1\n");
//		//游客
//		if(!IsInVisitorId(userid))
//			nret = 1;
//		if(nret == 0){
//			//游客号码被占用
//			//nret = CDBSink().get_visitor_records(userid);
//		}
//		if(nret == 1) {
//			printf("2\n");
//			return 0;
//		}
//		CUserObj * pUserObj = new CUserObj();
//		pUserObj->nuserid_ = userid;
//		pUserObj->ngender_ = 0;
//		pUserObj->nuserlevel_ = 1;
//		sprintf(szTemp,"游客");
//		pUserObj->calias_ = szTemp;
//		printf("3\n");
//		return pUserObj;
//	}
//	else
//	{
//		if (nRoomMgrType == 0) 
//		{
//			//判断是不是正管
//			nret = CDBSink().get_isroommanager_DB(userid, vcbid);
//			if(nret == 0)
//				nRoomMgrType = RoomMgrType_Guan;
//		}
//
//		StUserFullInfo_t ufi = {0};
//		nret = CDBSink().get_user_fullinfo_DB(ufi, userid);
//		if(nret == -1) {
//			return 0;
//		}
//
//		if (0 == loginsource) {
//			if (e_WEB_devtype != devtype)
//			{
//				if (strcmp(ufi.cpassword, pszUserPwd) != 0)
//				{
//					printf("4\n");
//					LOG_PRINT(log_error, "CreateUserFromDB fail!vcbid:%d,userid:%d,loginsource:%d,devtype:%u", vcbid, userid, loginsource, devtype);
//					return 0;
//				}
//			}
//			else
//			{
//				//for web and local 99 login only
//				if (strncmp(ufi.cpassword, pszUserPwd, 12) != 0)
//				{
//					printf("5\n");
//					LOG_PRINT(log_error, "CreateUserFromDB fail!vcbid:%d,userid:%d,loginsource:%d,devtype:%u", vcbid, userid, loginsource, devtype);
//					return 0;
//				}
//			}
//		}
//		else if (1 == loginsource && e_WEB_devtype == devtype)
//		{
//			//check token of third platform if require is from web
//			char pszToken[64] = {};
//			strncpy(pszToken, pszUserPwd, 32);
//			nret = CDBSink().get_user_platform_info_DB(userid, NULL, pszToken, -1);
//			if (0 != nret)
//			{
//				LOG_PRINT(log_error, "CreateUserFromDB failed: third platform token not match! vcbid:%d,userid:%d,loginsource:%d,devtype:%u", vcbid, userid, loginsource, devtype)
//				return 0;
//			}
//		}
//		printf("5\n");
//		CUserObj * pUserObj = new CUserObj();
//		pUserObj->nuserid_ = userid;
//		pUserObj->nstarflag_ = nstarflag;
//		pUserObj->nactivityflag_ = nactivityflag;
//		pUserObj->decocolor_ = 0;
//		printf("55\n");
//        pUserObj->nuserlevel_ = ufi.nuserlevel;
//		pUserObj->nisadregister_ = 0;
//		pUserObj->ngender_ = ufi.ngender;
//		printf("56\n");
//		pUserObj->ninroomlevel_ = nRoomMgrType;
//		printf("58\n");
//		pUserObj->calias_ =  ufi.calias;
//		printf("6\n");
//		return pUserObj;
//	}
//}

CUserObj * CRoomManager::CreateUserFromDB(int vcbid, int userid)
{
	int nret = 0;
	int nRoomMgrType = RoomMgrType_Null;
	int inroomstate = 1;
	//判断是不是正管
	if(CDBSink().get_isroommanager_DB(userid, vcbid) == 0)
	{
		nRoomMgrType = RoomMgrType_Guan;
		inroomstate = 10;
	}
	if(CDBSink().get_jiabing_DB(userid,vcbid) == 0)
	{
		inroomstate = 10;
	}
	StUserBaseAndGroupInfo oUserInfo;
	nret = CDBSink().get_userBaseAndGroupInfo_DB(userid, vcbid, oUserInfo);
	if(nret == -1) 
	{
		LOG_PRINT(log_error, "get_userBaseAndGroupInfo_DB error,vcbid:%d,userid:%d.", vcbid, userid);
		return 0;
	}
	else
	{
		CUserObj * pUserObj = new CUserObj();
		pUserObj->nuserid_ = userid;
		pUserObj->ngroupid = vcbid;

		{
			pUserObj->headpic_ = oUserInfo.userHead;
			pUserObj->calias_ = oUserInfo.userAlias;
			pUserObj->nuserlevel_ = oUserInfo.userGroupInfo.roleType;
			pUserObj->ngender_ = oUserInfo.userGender;
			pUserObj->ninroomlevel_ = nRoomMgrType;
			pUserObj->is_daka = oUserInfo.userGroupInfo.is_daka;
			pUserObj->inroomstate_ = inroomstate;
			pUserObj->cometime_ = time(0);
		}
		
		return pUserObj;
	}
}

int CRoomManager::redisSetRoomid(uint32 roomid)
{
	redisOpt *pRedis = getDataRedis();
	if (!pRedis)
		return -1;

	string key = KEY_SET_ROOMIDS;
	stringstream ss;
	ss << roomid;
	string sroomid = ss.str();

	return pRedis->redis_sadd(key, sroomid);
}

int CRoomManager::redisSetRoomInfo(CRoomObj* p)
{
	char key[32];
	vector<string> vecvalues;
	sprintf(key, "%s:%u", KEY_HASH_ROOM_INFO, p->nvcbid_);
	string skey = key;

	redisOpt *pRedis = getDataRedis();
	if (!pRedis) {
		LOG_PRINT(log_error, "redisOpt is NULL");
		return -1;
	}

	if (0 == pRedis->redis_hmget(skey, "vcbid", vecvalues) && 0 != vecvalues.size())
		return -1;
	string fields, values;
	vector<string> vecfield;
	Json::Value body;
	redisMsg_ObjectUpdate(skey, fields, body, p);//根据配置的field，将成员值以key=value放入body
	JsonToVec(body, vecfield, vecvalues);//根据body过滤一次vecvalues
	redisSetRoomid(p->nvcbid_);
	return pRedis->redis_hmset(skey, vecfield, vecvalues);//设置到redis
}

clienthandler_ptr CRoomManager::findconn(uint32 connid)
{
	clienthandler_ptr conn_ptr;
	map<uint32, clienthandler_ptr>::iterator it = m_mapConn.find(connid);
	if (it != m_mapConn.end())
		conn_ptr = it->second;

	return conn_ptr;
}

//void CRoomManager::exitroom_record(UserObject_Ref &userObjRef)
//{
//	int guest = IsInVisitorId(userObjRef->nuserid_) ? 0 : 1;
//
//	CMDRoomOnlineTS_t record ;
//    record.vcbid = userObjRef->ngroupid;
//    record.userid = userObjRef->nuserid_;
//    record.devtype = userObjRef->ndevtype;
//    record.guest = guest;
//    record.joinroom_time = userObjRef->cometime_;
//    record.timespan = time(0) - userObjRef->cometime_;
//    strncpy(record.ipaddr, userObjRef->ipaddr_.c_str(), sizeof(record.ipaddr));
//}

int CRoomManager::onGatewayDisconnected(clienthandler_ptr conn)
{
	LOG_PRINT(log_info, "[connid:%u,%s:%u] not gateway connection", conn->getconnid(), conn->getremote_ip(), conn->getremote_port());

	if (0 == conn->getgateid()) {
		return -1;
	}

	vector<string> vecRoomids = getAllRoomid();
	for (size_t i=0; i<vecRoomids.size(); i++) {
		RoomObject_Ref pRoomObjRef = FindRoom(atoi(vecRoomids[i].c_str()));
		if (!pRoomObjRef) {
			LOG_PRINT(log_error, "not found room:%s", vecRoomids[i].c_str());
			continue;
		}
		if (pRoomObjRef->isClosed())
			continue;

		vector<string> vecUserids;
		pRoomObjRef->getAllVisitorid(vecUserids);
		for (size_t k=0; k<vecUserids.size(); k++) {
			UserObject_Ref userobjRef;
			if (false == pRoomObjRef->findVisitUser(userobjRef, atoi(vecUserids[k].c_str()))) {
				LOG_PRINT(log_error, "not found user:%s", vecUserids[k].c_str());
				continue;
			}
			
			if (conn->getgateid() == userobjRef->ngateid && userobjRef->nsvrid == m_nsvrid) 
			{
				//get all users on this server.
				if (0 == pRoomObjRef->eraseVisitUser(userobjRef)) 
				{
					pRoomObjRef->castSendRoomUserExceptLeftInfo(userobjRef);
					//exitroom_record(userobjRef);
				}
			}
		}
	}

	return 0;
}

void CRoomManager::init_softbotids()
{
	uint32 min_no = m_softbot_min_no;
	uint32 max_no = m_softbot_max_no;
	for(int i=0; i<(max_no - min_no); ++i) {
		randindex_arr.push_back(i);
		m_vecSoftbotids.push_back(i+min_no);
	}
	// 把序号打乱，用于得到随即的softbotid
	for(int i=(max_no - min_no)-1; i>=1; --i)
		swap(randindex_arr[i], randindex_arr[rand()%i]);

	LOG_PRINT(log_info, "init softbot id complete");
}

void CRoomManager::init_softbot_vip_percent()
{

}

uint32 CRoomManager::getsoftbotid() const
{
	boost::mutex::scoped_lock(m_softbot_id_mutex);
	if (m_softbot_index >= randindex_arr.size() || randindex_arr[m_softbot_index] >= m_vecSoftbotids.size())
		return 0;
	return m_vecSoftbotids[randindex_arr[m_softbot_index]];
}

uint32 CRoomManager::getnextsoftbotid()
{
	boost::mutex::scoped_lock(m_softbot_id_mutex);
	if (++m_softbot_index >= (m_softbot_max_no - m_softbot_min_no))
		m_softbot_index = 0;

	if (m_softbot_index >= randindex_arr.size() || randindex_arr[m_softbot_index] >= m_vecSoftbotids.size())
		return 0;
	return m_vecSoftbotids[randindex_arr[m_softbot_index]];
}

void CRoomManager::getuseralias(int userid, char * alias, unsigned int aliasSize)
{
	std::string strAlias = CUserBasicInfo::getUserAlias(userid);
	strncpy(alias, strAlias.c_str(), aliasSize);
}

int CRoomManager::LoadVipTotalNum()
{
	CDBSink db;
	int total_row = 0;
	Result res;
	//查询VIP总人数
	if (!db.build_sql_run(&res, &total_row, db.SQL_s_viptotal_DB))
	{
		LOG_PRINT(log_error, "db error ");
		return -1;
	}
	if (total_row <= 0)
	{
		LOG_PRINT(log_error, "error line:%d.", total_row);
		return -1;
	}
	int viptotal = 0;
	for (int i = 0; i < total_row; i++)
	{
		int num = 0;
		db.GetFromRes(num, &res, i, 0);
		num *= 0.3;
		viptotal += num;
	}
	db.CleanRes(&res);
	m_redis_viptotal_map.insert("viptotal", viptotal);
	return 0;
}

void CRoomManager::castSendWeekFansList(uint32 teacherid, uint32 roomid, task_proc_data *task_node)
{
    LOG_PRINT(log_info, "enter");
    if (teacherid == 0 || roomid == 0) return;
	CDBSink db;
    int total_row = 0;
    Result res;

    do
    {
        if (!db.build_sql_run(&res, &total_row, db.SQL_s_contribution_week_DB, teacherid, (int)time(0), (int)time(0)) )
            break;

        char szBuf[2048] = {0};
        char *pPkt = szBuf;
        pPkt = CMsgComm::Build_COM_MSG_HEADER(pPkt, MDM_Vchat_Room, Sub_Vchat_TeacherGiftListResp_v2);
        pPkt = pPkt + sizeof(ClientGateMask_t);
        int *size = (int*)pPkt;
        pPkt = pPkt + sizeof(int);
        for (int row = 0; row < total_row; ++row)
        {
            *size = 0;
            for (; row < total_row; ++row)
            {
                if (pPkt + sizeof(CMDTeacherGiftListResp_v2_t) > szBuf + sizeof(szBuf))
                {
                    LOG_PRINT(log_info, "num %d, over size %d", row, sizeof(szBuf));
                    break;
                }

                CMDTeacherGiftListResp_v2_t* record = (CMDTeacherGiftListResp_v2_t*)pPkt;
                record->seqid = row + 1;
                record->vcbid = roomid;
                record->teacherid = teacherid;
                db.GetFromRes(record->useralias, &res, row, 0, sizeof(record->useralias));
                db.GetFromRes(record->t_num, &res, row, 1);
                db.GetFromRes(record->usericon, &res, row, 2, sizeof(record->usericon));

                pPkt = pPkt + sizeof(CMDTeacherGiftListResp_v2_t);
                *size += 1;
            }

            ((COM_MSG_HEADER*)szBuf)->length = pPkt - szBuf;
            if (task_node) {
                memcpy(szBuf + sizeof(COM_MSG_HEADER), task_node->pdata + sizeof(COM_MSG_HEADER), sizeof(ClientGateMask_t));
                task_node->connection->write_message(szBuf, ((COM_MSG_HEADER*)szBuf)->length);
            }
            else
                castSubroom((COM_MSG_HEADER*)szBuf, roomid, teacherid);
        }
        memset(szBuf, 0, sizeof(szBuf));
        pPkt = szBuf;
        pPkt = CMsgComm::Build_COM_MSG_HEADER(pPkt, MDM_Vchat_Room, Sub_Vchat_TeacherGiftListResp);
        pPkt = pPkt + sizeof(ClientGateMask_t);
        pPkt = pPkt + sizeof(int);
        for (int row = 0; row < total_row; ++row)
        {
            *size = 0;
            for (; row < total_row; ++row)
            {
                if (pPkt + sizeof(CMDTeacherGiftListResp_t) > szBuf + sizeof(szBuf))
                {
                    LOG_PRINT(log_info, "num %d, over size %d", row, sizeof(szBuf));
                    break;
                }

                CMDTeacherGiftListResp_t* record = (CMDTeacherGiftListResp_t*)pPkt;
                record->seqid = row + 1;
                record->vcbid = roomid;
                record->teacherid = teacherid;
                db.GetFromRes(record->useralias, &res, row, 0, sizeof(record->useralias) - 1);
                db.GetFromRes(record->t_num, &res, row, 1);

                pPkt = pPkt + sizeof(CMDTeacherGiftListResp_t);
                *size += 1;
            }

            ((COM_MSG_HEADER*)szBuf)->length = pPkt - szBuf;
            if (task_node) {
                memcpy(szBuf + sizeof(COM_MSG_HEADER), task_node->pdata + sizeof(COM_MSG_HEADER), sizeof(ClientGateMask_t));
                task_node->connection->write_message(szBuf, ((COM_MSG_HEADER*)szBuf)->length);
            }
            else
                castSubroom((COM_MSG_HEADER*)szBuf, roomid, teacherid);
        }


    } while(0);

    db.CleanRes(&res);
}


void CRoomManager::castSendTopTeamList(task_proc_data *task_node)
{
    LOG_PRINT(log_info, "enter");
    vector<CMDTeamTopNResp_t> vecTeamTopMoney;
    calcTeamTopMoney(vecTeamTopMoney);
	int itemsize = vecTeamTopMoney.size();
	if (0 == itemsize){
		LOG_PRINT(log_error, "no team top money record");
		return;
	}

	int pktlen = sizeof(COM_MSG_HEADER) + sizeof(ClientGateMask_t) + sizeof(itemsize) + sizeof(CMDTeamTopNResp_t)*itemsize;
    char szBuf[2048] = {0};
    char *pPkt = szBuf;
    pPkt = CMsgComm::Build_COM_MSG_HEADER(pPkt, MDM_Vchat_Room, Sub_Vchat_TeamTopNResp, pktlen);
    pPkt = pPkt + sizeof(ClientGateMask_t);
    pPkt = CMsgComm::Add_Struct(pPkt, &itemsize);
	CMDTeamTopNResp_t* pItem = (CMDTeamTopNResp_t*)pPkt;
	for(size_t i=0; i<itemsize; i++){
		memcpy(pItem, &vecTeamTopMoney[i], sizeof(CMDTeamTopNResp_t));
		pItem++;
	}

    if (task_node) {
        memcpy(szBuf + sizeof(COM_MSG_HEADER), task_node->pdata + sizeof(COM_MSG_HEADER), sizeof(ClientGateMask_t));
        task_node->connection->write_message(szBuf, pktlen);
    }
    else
        castSubroom((COM_MSG_HEADER*)szBuf, 0, 0);

	LOG_PRINT(log_info, "complete");
	return;
}

void CRoomManager::calcTeamTopMoney(vector<CMDTeamTopNResp_t> &vecTeamTopMoney)
{
    RoomObject_Ref roomObjRef;
	CDBSink db;
    int total_row;
	Result res;
	int errorid = 0;
	do
	{
    	if (!db.build_sql_run(&res, &total_row, "select nvcbid, sum(nusermoney) as money from dks_usergifttradelog where dtime > subdate(FROM_UNIXTIME(%d),date_format(FROM_UNIXTIME(%d) - 1,'%w')) and nvcbid in (66805,66808,66809,66812,66813,66815,66816) GROUP BY nvcbid ORDER BY money DESC limit 5", (int)time(0), (int)time(0)) ) {
            LOG_PRINT(log_error, "DB error!");
            errorid = 2;
            break;
        }
        for (int row = 0; row < total_row; ++row){
    		CMDTeamTopNResp_t data = {0};
    		db.GetFromRes(data.vcbid, &res, row, 0);
    		db.GetFromRes(data.giftmoney, &res, row, 1);
    		roomObjRef = FindRoom(data.vcbid);
    		if (roomObjRef.get())
    			strncpy(data.teamname, roomObjRef->name.c_str(), sizeof(data.teamname)-1);
    		vecTeamTopMoney.push_back(data);
    	}

    } while(0);

	db.CleanRes(&res);

}

void CRoomManager::updateGroupUserCount(unsigned int groupid, unsigned int userid, bool bAdd)
{
	if (!m_initData)
	{
		LOG_PRINT(log_warning, "[updateGroupUserCount]not need to update db and redis when processor init.groupid:%u,userid:%u,bAdd:%d.", groupid, userid, (int)bAdd);
		return;
	}

	RoomObject_Ref pRoomObjRef = FindRoom(groupid);
	if (!pRoomObjRef) 
	{
		LOG_PRINT(log_error, "[updateGroupUserCount]not found group:%u.", groupid);
		return;
	}

	pRoomObjRef->UpdateGroupUserCount(userid, bAdd);
}

void CRoomManager::castRooms_onesvr(const char * pData, unsigned int datalen, unsigned int maincmd, unsigned int subcmd, const std::set<unsigned int> & roomset)
{
	if (roomset.empty())
	{
		LOG_PRINT(log_warning, "Not need to cast_room_ptcourse_state,because roomset is empty.");
		return;
	}

	std::set<unsigned int>::const_iterator iter = roomset.begin();
	for (; iter != roomset.end(); ++iter)
	{
		unsigned int roomID = *iter;
		if (!roomID)
		{
			continue;
		}

		unsigned int pPktLen = sizeof(COM_MSG_HEADER) + sizeof(ClientGateMask_t) + datalen;
		SL_ByteBuffer sendbuf(pPktLen);

		char * pwritebuf = sendbuf.buffer();
		COM_MSG_HEADER * pOutput = (COM_MSG_HEADER *)pwritebuf;
		pOutput->checkcode = CHECKCODE;
		pOutput->version = MDM_Version_Value;
		pOutput->maincmd = maincmd;
		pOutput->subcmd = subcmd;
		pOutput->length = pPktLen;

		memset(pOutput->content, 0, sizeof(ClientGateMask_t));
		memcpy(pOutput->content + sizeof(ClientGateMask_t), pData, datalen);
		sendbuf.data_end(pPktLen);

		castRoom_onesvr((COM_MSG_HEADER *)sendbuf.buffer(), roomID);
		LOG_PRINT(log_info, "[cast_room_packet]cast_room:%u.msg len:%u", roomID, pPktLen);
	}
}

void CRoomManager::castRoomSendMsg(COM_MSG_HEADER* pOutMsg, uint32 vcbid, bool IsMaskCur)
{
	ClientGateMask_t * pClientGate = (ClientGateMask_t *)pOutMsg->content;
	unsigned int except_connid = pClientGate->param2;
	if (!IsMaskCur)
	{
		except_connid = 0;
	}

	CMsgComm::Build_BroadCastRoomGate(pClientGate, e_Notice_AllType, vcbid, except_connid);

	unsigned int count = 0;
	map<uint32, clienthandler_ptr>::const_iterator it = m_mapConn.begin();
	for(; it != m_mapConn.end(); it++) {
		clienthandler_ptr pclient = it->second;
		if (pclient && pclient->isconnected()){
			pclient->write_message((char*)pOutMsg, pOutMsg->length);
			++count;
		}
	}

	LOG_PRINT(log_info, "broadcast all gate. room:%u,maincmd:%u,subcmd:%u,msglen:%u,gate count:%u.", \
		vcbid, pOutMsg->maincmd, pOutMsg->subcmd, pOutMsg->length, count);
}

void CRoomManager::castUserSendMsg(COM_MSG_HEADER* pOutMsg, uint32 userid)
{
	if (!userid)
	{
		LOG_PRINT(log_error, "userid is 0.");
		return;
	}

	ClientGateMask_t * pClientGate = (ClientGateMask_t *)pOutMsg->content;
	CMsgComm::Build_BroadCastUser_Gate(pClientGate, userid, e_Notice_AllType, 0);

	unsigned int count = 0;
	map<uint32, clienthandler_ptr>::const_iterator it = m_mapConn.begin();
	for(; it!= m_mapConn.end(); it++) {
		clienthandler_ptr pclient = it->second;
		if (pclient && pclient->isconnected()){
			pclient->write_message((char*)pOutMsg, pOutMsg->length);
			++count;
		}
	}

	LOG_PRINT(log_info, "broadcast all gate.maincmd:%u,subcmd:%u,msglen:%u,gate count:%u.", \
		pOutMsg->maincmd, pOutMsg->subcmd, pOutMsg->length, count);
}

void CRoomManager::castGateway(COM_MSG_HEADER * pOutMsg)
{
	map<uint32, clienthandler_ptr>::const_iterator it = m_mapConn.begin();
	for(; it != m_mapConn.end(); it++) 
	{
		clienthandler_ptr pclient = it->second;
		if (pclient && pclient->isconnected()){
			pclient->write_message((char*)pOutMsg, pOutMsg->length);
		}
	}
}

void CRoomManager::castSubroom(COM_MSG_HEADER * pOutMsg, uint32 roomid, uint32 teacherid)
{
	CDBSink db;
	int total_row = 0;
	Result res;	
	if (!db.build_sql_run(&res, &total_row, "select nrobotvcbid from dks_vcbzhuanboconfig where nuserid=%u", teacherid) )
	{
	}
	else
	{
		if (total_row == 0) castRoom(pOutMsg, roomid, 0);
		for (int row = 0; row < total_row; ++row) 
		{
			db.GetFromRes(roomid, &res, row, 0);
			castRoom(pOutMsg, roomid, 0);
		}       
	}
	db.CleanRes(&res);
}

void CRoomManager::castRoom(COM_MSG_HEADER * pOutMsg, uint32 roomid, bool IsMaskCur)
{
	ClientGateMask_t* pClientGate = (ClientGateMask_t*)pOutMsg->content;
	unsigned int except_connid = pClientGate->param2;
	if (!IsMaskCur) 
	{
		except_connid = 0;
	}

	CMsgComm::Build_BroadCastRoomGate(pClientGate, e_Notice_AllType, roomid, except_connid);

	castGateway(pOutMsg);
}

void CRoomManager::castRoom_onesvr(COM_MSG_HEADER * pOutMsg, uint32 roomid)
{
	ClientGateMask_t * pClientGate = (ClientGateMask_t *)pOutMsg->content;
	CMsgComm::Build_BroadCastUsersOnOneSvr_Gate(pClientGate, roomid, e_Notice_AllType, 0);

	castGateway(pOutMsg);
}
void CRoomManager::changeWaitUser(uint32 userid,uint32 vcbid,int action)
{
	RoomObject_Ref pRoomObjRef = FindRoom(vcbid);

	if(pRoomObjRef.get() != 0) {
		//自动上线，下线
		{
			char szOutBuf[1024] = {0};
			COM_MSG_HEADER* pOutMsg=(COM_MSG_HEADER*)szOutBuf;
			pOutMsg->version = MDM_Version_Value;
			pOutMsg->checkcode = CHECKCODE;
			pOutMsg->maincmd = MDM_Vchat_Room;
			pOutMsg->subcmd = Sub_Vchat_ChangeWaitMicIndexReq;
			CMDOperateWaitMic pOutCmd;
			pOutCmd.set_micid(-1);
			pOutCmd.set_optype(action);
			pOutCmd.set_ruunerid(userid);
			pOutCmd.set_userid(userid);
			pOutCmd.set_vcbid(vcbid);

			pOutMsg->length = SIZE_IVM_HEAD_TOTAL + pOutCmd.ByteSize();
			pOutCmd.SerializeToArray(pOutMsg->content + SIZE_IVM_CLIENTGATE, pOutCmd.ByteSize());
			//消息使用task模拟处理
			if (m_clientconn.get_connecthandle()->isconnected())
				m_clientconn.get_connecthandle()->write_message(szOutBuf, pOutMsg->length);
			LOG_PRINT(log_info,"room_tag changeWaitUser timer 300 end.roomid:%u.", vcbid);
		}
	}
}
