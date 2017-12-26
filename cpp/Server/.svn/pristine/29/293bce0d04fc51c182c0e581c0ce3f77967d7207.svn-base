#include "RoomObj.h"
#include "yc_defines.h"
#include "CLogThread.h"
#include "redisdataHelper.h"
#include "DBConnection.h"
#include "DBTools.h"
#include <mysql.h>
#include "utils.h"
#include "clienthandler.h"
#include "msgcommapi.h"
#include "errcode.h"
#include "RoomManager.h"
#include "UserMgrSvr.pb.h"
#include "CUserBasicInfo.h"
#include "CUserGroupinfo.h"
#include "CUserGroupMgr.h"
#include "DBSink.h"

const int VIPLEVEL_COUNT = 2;

CRoomObj::CRoomObj(CRoomManager * pMgr):m_pRoomMgr(pMgr)
{
	reset();
	init();
}

CRoomObj::CRoomObj(const vector<string> &fields, const vector<string> &values, CRoomManager * pMgr):m_pRoomMgr(pMgr)
{
	reset();
	init();
	findvalue(nvcbid_, "courseid", fields, values);
	findvalue(nuser_id, "user_id", fields, values);						//群主
	findvalue(nlive_id, "live_id", fields, values);
	findvalue(class_name, "class_name", fields, values);
	findvalue(teacher_name, "teacher_name", fields, values);
	findvalue(teacher_name, "teacher_name", fields, values);
	findvalue(img, "img", fields, values);
	findvalue(src_img, "src_img", fields, values);
	findvalue(nprice, "price", fields, values);
	findvalue(nlevel, "level", fields, values);
	findvalue(strpwd, "strpwd", fields, values);
	findvalue(invite_code, "invite_code", fields, values);
	findvalue(invite_code, "invite_code", fields, values);
	findvalue(invite_code, "invite_code", fields, values);
	findvalue(goal, "goal", fields, values);
	findvalue(dest_user, "dest_user", fields, values);
	findvalue(requirements, "requirements", fields, values);
	findvalue(nbegin_time, "begin_time", fields, values);
	findvalue(nend_time, "end_time", fields, values);
	findvalue(nstatus, "status", fields, values);
	findvalue(nopenstatus, "openstatus", fields, values);
	findvalue(npublish_time, "publish_time", fields, values);
	findvalue(ncreate_time, "create_time", fields, values);
	findvalue(nupdate_time, "update_time", fields, values);
	findvalue(audience_num, "audience_num", fields, values);
	findvalue(virtual_num, "virtual_num", fields, values);
	 
	findvalue(talk_status, "talk_status", fields, values);
	findvalue(form, "form", fields, values);

	Json::Reader reader(Json::Features::strictMode());
	Json::Value root;
	std::string value = "";
	findvalue(value, "nopusers", fields, values);
	if (reader.parse(value, root))
	{
		if (root.isArray()) {
			for(int i = 0 ;i < root.size(); i++)
			{
				if(i < MAX_GUEST)
				{
					nopuser[i] = root[i].asInt();
				}
			}
		}
	}
}

CRoomObj::~CRoomObj(void)
{
}

void CRoomObj::init()
{
	if (m_pRoomMgr)
	{
		m_rmGroupInfo.init(DB_ROOM, KEY_HASH_ROOM_INFO":", m_pRoomMgr->m_pRedisMgr);
		m_rsMemberInGroup.init(DB_ROOM, KEY_SET_INGROUP":", m_pRoomMgr->m_pRedisMgr);
		m_rsVisitorInGroup.init(DB_ROOM, KEY_SET_ROOM_USERIDS":", m_pRoomMgr->m_pRedisMgr);
		m_rmRoomuserSetting.init(DB_ROOM, KEY_HASH_ROOM_USER_SETTING":", m_pRoomMgr->m_pRedisMgr);
		setRedis(m_pRoomMgr->getDataRedis());
	}
}

string CRoomObj::getRedisKey()
{
	char key[32];
	sprintf(key, "%s:%u", KEY_HASH_ROOM_INFO, nvcbid_);
	return string(key);
}

string CRoomObj::getRedisFields()
{
	char sztemp[64];
	string fields = CFieldName::ROOM_FIELDS;
	for (int i=1; i<=VIPLEVEL_COUNT; i++) {
		sprintf(sztemp, "uservip%d_count", i);
		fields.append(sztemp);
	}
	for (int i=1; i<=VIPLEVEL_COUNT; i++) {
		sprintf(sztemp, "softbotvip%d_count", i);
		fields.append(sztemp);
	}
	return fields;
}

void CRoomObj::getRedisValues(Json::Value &jval, const string &fields)
{
	if (fields.find("courseid") != string::npos)
		jval["courseid"] = nvcbid_;
	if (fields.find("user_id") != string::npos)
		jval["user_id"] = nuser_id;
	if (fields.find("live_id") != string::npos)
		jval["live_id"] = nlive_id;
	if (fields.find("class_name") != string::npos)
		jval["class_name"] = class_name;
	if (fields.find("teacher_name") != string::npos)
		jval["teacher_name"] = teacher_name;
	if (fields.find("teacher_info") != string::npos)
		jval["teacher_info"] = teacher_info;
	if (fields.find("img") != string::npos)
		jval["img"] = img;
	if (fields.find("src_img") != string::npos)
		jval["src_img"] = src_img;
	if (fields.find("price") != string::npos)
		jval["price"] = nprice;
	if (fields.find("level") != string::npos)
		jval["level"] = nlevel;
	if (fields.find("strpwd") != string::npos)
		jval["strpwd"] = strpwd;
	if (fields.find("invite_code") != string::npos)
		jval["invite_code"] = invite_code;
	if (fields.find("strremark") != string::npos)
		jval["strremark"] = strremark;
	if (fields.find("tags") != string::npos)
		jval["tags"] = tags;
	if (fields.find("goal") != string::npos)
		jval["goal"] = goal;
	if (fields.find("dest_user") != string::npos)
		jval["dest_user"] = dest_user;
	if (fields.find("requirements") != string::npos)
		jval["requirements"] = requirements;
	if (fields.find("begin_time") != string::npos)
		jval["begin_time"] = nbegin_time;
	if (fields.find("end_time") != string::npos)
		jval["end_time"] = nend_time;
	if (fields.find("status") != string::npos)
		jval["status"] = nstatus;
	if (fields.find("openstatus") != string::npos)
			jval["openstatus"] = nopenstatus;
	if (fields.find("publish_time") != string::npos)
		jval["publish_time"] = npublish_time;
	if (fields.find("create_time") != string::npos)
		jval["create_time"] = ncreate_time;
	if (fields.find("update_time") != string::npos)
		jval["update_time"] = nupdate_time;
	if (fields.find("class_name") != string::npos)
		jval["class_name"] = class_name;
	if (fields.find("audience_num") != string::npos)
		jval["audience_num"] = audience_num;

	if (fields.find("virtual_num") != string::npos)
		jval["virtual_num"] = virtual_num;

	if (fields.find("form") != string::npos)
		jval["form"] = form;

	if (fields.find("talk_status") != string::npos)
		jval["talk_status"] = talk_status;
	if (fields.find("opusers") != string::npos) {
		Json::Value val;
		for(int i =0 ; i < MAX_GUEST ; i++)
		{
			val.append(nopuser[i]);
		}
		jval["opusers"] = val;
	}

}

void CRoomObj::setRedis(redisOpt *pRedis)
{
	m_pRedis = pRedis;
}

void CRoomObj::setSvrid(uint16_t svrid)
{
	m_svrid = svrid;
}

void CRoomObj::reset()
{
	nvcbid_ = 0;
	nuser_id = 0;							//群主
	nlive_id = 0;
	class_name = "";
	teacher_name = "";
	teacher_info = "";
	img = "";
	src_img = "";
	nprice = 0;
	nlevel = 0;
	strpwd = "";
	invite_code = "";
	strremark = "";
	tags = "";
	goal = "";
	dest_user = "";
	requirements = "";
	nbegin_time = "";
	nend_time = "";
	nstatus = 0;
	npublish_time = "";
	ncreate_time = "";
	nupdate_time = "";
	audience_num = 0;
	virtual_num = 0;
	talk_status = 0;
	for(int i = 0 ; i < MAX_GUEST; i++)
	{
		nopuser[i] = 0;
	}

	form=0;
}

int CRoomObj::addVisitorUser(UserObject_Ref &userobjRef)
{
	if (findVisitUser(userobjRef->nuserid_)) {
		LOG_PRINT(log_error, "[addVisitorUser]user:%u exists", userobjRef->nuserid_);
		return -1;
	}

	if (!m_pRedis) {
		LOG_PRINT(log_error, "pRedis is NULL");
		return -1;
	}

	string key, fields, values;
	char key1[64], roomkey[64], userid[32];
	Json::Value body;
	redisMsg_ObjectUpdate(key, fields, body, userobjRef.get());

	sprintf(key1, "%s:%u", KEY_SET_ROOM_USERIDS, nvcbid_);
	sprintf(userid, "%u", userobjRef->nuserid_);
	sprintf(roomkey, "%s:%u", KEY_HASH_ROOM_INFO, nvcbid_);
	vector<string> vecfields, vecvalues;
	JsonToVec(body, vecfields, vecvalues);

	redisPipeline pipe(m_pRedis);
	m_pRedis->redis_sadd(key1, userid);
	m_pRedis->redis_hmset(key, vecfields, vecvalues);

	int ret = 0;
	while((ret = pipe.getreply()) != NO_MORE_REDIS_REPLY) {
		if (ret < 0) {
			LOG_PRINT(log_error, "addVisitorUser redis return error.");
		}
	}

	return ret;
}

void CRoomObj::UpdateGroupUserCount(unsigned int userid, bool bAdd)
{
	LOG_PRINT(log_debug, "[UpdateGroupUserCount]groupid:%u userid:%u add:%d.", nvcbid_, userid, (int)bAdd);
	if (!m_pRedis)
	{
		LOG_PRINT(log_error, "[UpdateGroupUserCount fail]redis is null.groupid:%u userid:%u add:%d.", nvcbid_, userid, (int)bAdd);
		return;
	}

	int increment_ = 0;
	if (bAdd)
	{
		increment_ = 1;
	}
	else
	{
		increment_ = -1;
	}

	map<string, string> map_field_value;
	int iGender = CUserBasicInfo::getUserGender(userid);
	if (iGender >= 0)
	{
		map_field_value["user_num"] = bitTostring(increment_);
		if (iGender == e_MaleGender)
		{
			//male
			map_field_value["male_num"] = bitTostring(increment_);
			LOG_PRINT(log_debug, "[fresh redis num]groupid:%u userid:%u add:%d gender:%d.", nvcbid_, userid, (int)bAdd, iGender);
		}
		else if (iGender == e_FemaleGender)
		{
			//female
			map_field_value["female_num"] = bitTostring(increment_);
			LOG_PRINT(log_debug, "[fresh redis num]groupid:%u userid:%u add:%d gender:%d.", nvcbid_, userid, (int)bAdd, iGender);
		}
	}

	if (CDBSink().updateRoomUserCount(nvcbid_, map_field_value))
	{
		char roomkey[64] = {0};
		sprintf(roomkey, "%s:%u", KEY_HASH_ROOM_INFO, nvcbid_);

		if (!m_rmGroupInfo.insert(nvcbid_, map_field_value))
		{
			LOG_PRINT(log_error, "[UpdateGroupUserCount]failed to write redis msg.groupid:%u userid:%u add:%d gender:%d.", nvcbid_, userid, (int)bAdd, iGender);
		}
	}

	return;
}

unsigned int CRoomObj::UpdateVistorCount()
{
	getVisitorNum();

	string fields = "audience_num";
	char key[32] = {0};
	sprintf(key, "%s:%u", KEY_HASH_ROOM_INFO, nvcbid_);
	string skey = key;
	updateFields(skey, fields);

	castSendRoomVistorCount();

	return audience_num;
}

bool CRoomObj::kickAllLookers()
{
	vector<string> vVisitor;
	getAllVisitorid(vVisitor);
	if (vVisitor.size() > 0)
	{
		vector<string>::iterator iter_v = vVisitor.begin();
		for (; iter_v != vVisitor.end(); ++iter_v)
		{
			eraseVisitUser(atoi(iter_v->c_str()));
		}
	}

	return true;
}

int CRoomObj::delVisitUser(UserObject_Ref &userobjRef, bool downMic)
{
	uint16 ngateid = 0;
	uint64 connid = 0;

	if (!m_pRedis) {
		LOG_PRINT(log_error, "delVisitUser pRedis is NULL");
		return -1;
	}

	string key, fields, values;
	char key1[64], roomkey[64], userkey[64], suserid[32];

	ngateid = userobjRef->ngateid;
	connid = userobjRef->pGateObjId_;

	sprintf(key1, "%s:%u", KEY_SET_ROOM_USERIDS, nvcbid_); // TODO: uint32 roomid = userobjRef->ngroupid; ??
	sprintf(suserid, "%u", userobjRef->nuserid_);
	sprintf(userkey, "%s:%u", KEY_HASH_USER_INFO, userobjRef->nuserid_);

	LOG_PRINT(log_debug, "delVisitUser roomid:%u,userid:%u", nvcbid_, userobjRef->nuserid_);

	int ret = 0;
	redisPipeline pipe(m_pRedis);
	m_pRedis->redis_srem(key1, suserid);
	m_pRedis->redis_delkey(userkey);

	while((ret = pipe.getreply()) != NO_MORE_REDIS_REPLY) {
		if (ret < 0) {
			LOG_PRINT(log_error, "delVisitUser redis return error.");
		}
	}

	LOG_PRINT(log_info, "room_tag [connid=%llu,gateid=%u,userid=%u,roomid=%u]user was deleted from room, totalcount=%u,robotcount:%u,softbotcount:%u",
			connid, ngateid, userobjRef->nuserid_, nvcbid_, ntotalcount_, nrobotcount_, nsoftbotcount_);

	return ret;
}

int CRoomObj::eraseVisitUser(uint32 userID, bool downMic)
{
	if (!userID)
	{
		LOG_PRINT(log_error, "erase visit user input error.userID cannot be 0.");
		return -1;
	}

	UserObject_Ref userObjRef;
	if (findVisitUser(userObjRef, userID) && userObjRef.get())
	{
		return eraseVisitUser(userObjRef, downMic);
	}
	else
	{
		return -1;
	}
}

int CRoomObj::eraseVisitUser(UserObject_Ref & pUserObjRef, bool downMic)
{
	if (delVisitUser(pUserObjRef, downMic) < -1/* TODO: 0 ?? */) {
		LOG_PRINT(log_error, "Failed to del user:%u", pUserObjRef->nuserid_);
		return -1;
	}

	UpdateVistorCount(); // TODO: UpdateVistorCount(pUserObjRef->ngroupid); ??

	return 0;
}

int CRoomObj::InsertVisitUser(UserObject_Ref & pUserObjRef)
{
	if (addVisitorUser(pUserObjRef) < 0) {
		LOG_PRINT(log_error, "[connid=%llu,gateid=%u,userid=%u,roomid=%u]failed to add user", pUserObjRef->pGateObjId_, pUserObjRef->ngateid,
				pUserObjRef->nuserid_, nvcbid_);
		return -1;
	}

	UpdateVistorCount(); // TODO: UpdateVistorCount(pUserObjRef->ngroupid); ??
	return 0;
}

bool CRoomObj::findUser(UserObject_Ref & userobjRef, uint32 userId)
{
	bool bfound = false;
	bfound = findMemberUser(userobjRef,userId);
	if(!bfound)
	{
		bfound = findVisitUser(userobjRef,userId);
	}
	return bfound;
}

bool CRoomObj::findVisitUser(UserObject_Ref & userobjRef, uint32 userId)
{
	bool nfound = false;
	CRoomObj::loadVisitUserData(m_pRoomMgr, m_pRedis, userobjRef, userId);
	if (userobjRef.get()) {
		nfound = true;
	}
	return nfound;
}

bool CRoomObj::findVisitUser(CRoomManager * pRoomMgr, redisOpt * pRedis, UserObject_Ref & userobjRef, uint32 userId)
{
	bool nfound = false;
	loadVisitUserData(pRoomMgr, pRedis, userobjRef, userId);
	if (userobjRef.get()) {
		nfound = true;
	}
	return nfound;
}

bool CRoomObj::findVisitUser(uint32 userId)
{
	bool bRet = false;
	vector<string> result;
	char key[32];
	sprintf(key, "%s:%u", KEY_HASH_USER_INFO, userId);

	if (!m_pRedis) {
		LOG_PRINT(log_error, "pRedis is NULL");
		return -1;
	}
	int ret = m_pRedis->redis_existkey(key);
	if (ret < 0) {
		LOG_PRINT(log_error, "Failed to run EXISTS command");
	}
	bRet = (1 == ret);

	return bRet;
}

void CRoomObj::loadVisitUserData(CRoomManager* pRoomMgr, redisOpt *pRedis, UserObject_Ref &userObjRef, uint32 userid)
{
	if (0 == userid) {
		LOG_PRINT(log_warning, "userid must not be 0");
		return;
	}
	if (!pRoomMgr || !pRedis) {
		return;
	}

	vector<string> vecfield, vecvalue;

	char key[32];
	sprintf(key, "%s:%u", KEY_HASH_USER_INFO, userid);
	{
		redisPipeline pipeline(pRedis);
		pRedis->redis_hgetall(key, vecfield, vecvalue);
		pipeline.getreply(vecfield);
		pipeline.getreply(vecvalue);
	}

	if (0 == vecfield.size() || 0 == vecvalue.size()) {
		LOG_PRINT(log_error, "no found user:%u", userid);
		return ;
	}
	CUserObj* pUser = new CUserObj(vecfield, vecvalue);
	if (pUser){
		pUser->bIsMember = false;
		userObjRef.reset(pUser);
		map<uint32, clienthandler_ptr>::iterator it = pRoomMgr->m_mapConn.find(userObjRef->pConnId_);
		if (it != pRoomMgr->m_mapConn.end())
		{
			clienthandler_ptr pconn = it->second;
			if (pconn && pconn->getgateid() == userObjRef->ngateid) {
				userObjRef->pConn_ = pconn;
			}
		}
	}
	else {
		LOG_PRINT(log_error, "[user:%u] Failed to read user info from redis", userid);
	}
}

bool CRoomObj::findMemberUser(UserObject_Ref & userobjRef, uint32 userId)
{
	bool nfound = false;
	CRoomObj::loadMemberUserData(userobjRef, userId, nvcbid_);
	if (userobjRef.get()) 
	{
		nfound = true;
	}
	return nfound;
}

bool CRoomObj::findMemberUser(uint32 userId)
{
	return CUserGroupinfo::isMember(userId, nvcbid_);
}

void CRoomObj::loadMemberUserData(UserObject_Ref & userObjRef, uint32 userid, uint32 groupid)
{
	if (!userid || !groupid) 
	{
		LOG_PRINT(log_error, "userid and groupid must not be 0,userid:%u,groupid:%u.", userid, groupid);
		return;
	}

	if (!CUserGroupinfo::isMember(userid, groupid))
	{
		LOG_PRINT(log_error, "user is not group member.userid:%u,groupid:%u.", userid, groupid);
		return;
	}

	std::map<std::string, std::string> oGroupInfoMap;
	CUserGroupinfo::getall(groupid, userid, oGroupInfoMap);

	std::map<std::string, std::string> oUserInfoMap;
	CUserBasicInfo::getUserInfo(userid, oUserInfoMap);

	oUserInfoMap.insert(oGroupInfoMap.begin(), oGroupInfoMap.end());
	if (oUserInfoMap.empty())
	{
		LOG_PRINT(log_error, "user has no any information about group.userid:%u,groupid:%u.", userid, groupid);
		return;
	}

	CUserObj * pUser = new CUserObj(oUserInfoMap);
	if (pUser)
	{
		pUser->bIsMember = true;
		userObjRef.reset(pUser);
	}
	else 
	{
		LOG_PRINT(log_error, "[user:%u] Failed to create user info from redis.userid:%u,groupid:%u.", userid, groupid);
	}
}

int CRoomObj::eraseMemberUser(UserObject_Ref & pUserRef)
{
	if (!m_pRedis) 
	{
		LOG_PRINT(log_error, "erase member pRedis is NULL");
		return -1;
	}

	if (pUserRef.get() == 0)
	{
		LOG_PRINT(log_error, "erase member user input error.");
		return -1;
	}
	outGroup(pUserRef->nuserid_);

	//再处理成员退群
	return CUserGroupMgr::procMemberQuitGroup(pUserRef->nuserid_, nvcbid_);
}

void CRoomObj::loadBatchVisitor(vector<string> &userids, vector<UserObject_Ref> &vecUserRef)
{
	vector<string> vecfield, vecvalue;

	if (!m_pRedis) {
		LOG_PRINT(log_error, "pRedis is NULL");
		return ;
	}

	redisPipeline pipeline(m_pRedis);
	for (size_t i=0; i<userids.size(); i++) {
		string key = string(KEY_HASH_USER_INFO) + string(":") + userids[i];
		m_pRedis->redis_hgetall(key, vecfield, vecvalue);
	}

	int ret = -1;
	while (1) {
		ret = pipeline.getreply(vecfield);
		if (NO_MORE_REDIS_REPLY == ret)
			break;
		ret = pipeline.getreply(vecvalue);
		if (NO_MORE_REDIS_REPLY == ret)
			break;
		if (vecfield.empty() || vecvalue.empty())
			continue;

		UserObject_Ref userObjRef(new CUserObj(vecfield, vecvalue));
		vecUserRef.push_back(userObjRef);
	}
}

//bool CRoomObj::getSoftbot(UserObject_Ref& userobjRef, uint8 nuserviplevel)
//{
//	vector<string> vecUserid = getAllVisitorid();
//	vector<UserObject_Ref> vecUser;
//	loadBatchVisitor(vecUserid, vecUser);
//
//	for (size_t i=0; i<vecUser.size(); i++) {
//		if (USERTYPE_SOFTBOT == vecUser[i]->type_ && nuserviplevel == vecUser[i]->nuserviplevel_) {
//			userobjRef = vecUser[i];
//			break;
//		}
//	}
//	return (0 != userobjRef);
//}

bool CRoomObj::isRoomFangzhu(uint32 userid)
{
   if(userid == ncreatorid_) return true;
   return false;
}

bool CRoomObj::isRoomFuFangzhu(uint32 userid)
{
	for(int i=0; i<MAX_ROOMOPUSER_COUNT; i++)
	{
		if(userid == nopuserid_[i])
			return true;
	}
	return false;
}

int CRoomObj::addPubMicUser(uint32 userId, int nSpecMicIndex, int bIsGuanliMic)
{
	//找到公麦空位置坐下,修改状态为公麦
	//TODO: 公麦结束时间处理,需要新参数
	int i, nret=-1;
	bool badd = false;
	int m = getMicMode();
	getPubMicState();
	LOG_PRINT(log_error, "room_tag setmicstate: b%u,,mode:%d,,userid:%d", nvcbid_,m,pubmicstate_[0].userid);
	if(getMicMode()  == e_QueueMode && pubmicstate_[0].userid > 0 )
		return nret;
	if(nSpecMicIndex >= nMaxPubMicCount_ || nSpecMicIndex <-1){
		LOG_PRINT(log_warning, "room_tag nSpecMicIndex: %d, >= nMaxPubMicCount_: %d", nSpecMicIndex, nMaxPubMicCount_);
		return -1;
	}

	//已经在公麦上
	for(int i = 0 ; i < MAX_PUBMIC_COUNT ; i++)
	{
		if(pubmicstate_[i].userid == userId)
		{
			return i;
		}
	}

	//管理麦,管理麦一定要有麦id(全局id)
	if(bIsGuanliMic )
	{
		//assert(nSpecMicIndex >=0);
		if(nSpecMicIndex <=0 ) return -1;
		nret = -1;
		if(pubmicstate_[nSpecMicIndex].userid == 0 && pubmicstate_[nSpecMicIndex].mictimetype == -2)
		{
			pubmicstate_[nSpecMicIndex].userid = userId;
			pubmicstate_[nSpecMicIndex].userendtime = 0;
			nret = nSpecMicIndex;
			pubmicstate_[nSpecMicIndex].onmic = 1;
			badd = true;
			nCurPubMicCount_ ++;
			LOG_PRINT(log_info, "room_tag manager-MIC nCurPubMicCount_: %d, user:%u, room:%u", nCurPubMicCount_, userId, nvcbid_);
		}
		return nret;
	}

	//不指定麦id, 找个空自由麦
	if(nSpecMicIndex == -1)
	{
		LOG_PRINT(log_info, "room_tag setmicstate not  zhiding manager-MIC nCurPubMicCount_: %d, user:%u, room:%u", nCurPubMicCount_, userId, nvcbid_);
		for(i=0; i< nMaxPubMicCount_; ++i)
		{
			if (pubmicstate_[i].userid == 0 && pubmicstate_[i].status == 1 && pubmicstate_[i].mictimetype != 0 && pubmicstate_[i].mictimetype != -2)
			{
				pubmicstate_[i].userid = userId;
				pubmicstate_[i].onmic = 1;
				pubmicstate_[i].userendtime = time(0) + pubmicstate_[i].mictimetype * 60;
				nret = i;
				badd = true;
				nCurPubMicCount_ ++;
				LOG_PRINT(log_info, "setmicstate nCurPubMicCount_: %d, index:%d user:%u, room:%u", nCurPubMicCount_,i, userId, nvcbid_);
				break;
			}
		}
	}
	else { //指定麦,包含管理麦
		LOG_PRINT(log_info, "room_tag setmicstate zhiding manager-MIC nCurPubMicCount_: %d, user:%u, room:%u  ,nSpecMicIndex:%d",
				nCurPubMicCount_, userId, nvcbid_,nSpecMicIndex);
		if(pubmicstate_[nSpecMicIndex].userid == 0 && pubmicstate_[nSpecMicIndex].status == 1 && pubmicstate_[nSpecMicIndex].mictimetype != 0)
		{
			pubmicstate_[nSpecMicIndex].userid = userId;
			pubmicstate_[nSpecMicIndex].userendtime = time(0) + pubmicstate_[nSpecMicIndex].mictimetype * 60;
			pubmicstate_[nSpecMicIndex].onmic = 1;
			nret = nSpecMicIndex;
			badd = true;
			nCurPubMicCount_ ++;
			LOG_PRINT(log_info, "room_tag setmicstate nCurPubMicCount_: %d, user:%u, room:%u", nCurPubMicCount_, userId, nvcbid_);
		}
	}
	LOG_PRINT(log_error, "room_tag setmicstate user:%u, room:%u, nret :%d!\n", userId, nvcbid_,nret);
	updateMicState();
	if(nret < 0)
	{
		LOG_PRINT(log_error, "room_tag setmicstate user:%u, room:%u, not found available public mic!\n", userId, nvcbid_);
	}

    return nret;
}

int CRoomObj::HitTestFreePubMic()
{
	for(int i=0; i< nMaxPubMicCount_; ++i)
	{
		if(pubmicstate_[i].userid == 0 && pubmicstate_[i].status == 1  && pubmicstate_[i].mictimetype != 0 && pubmicstate_[i].mictimetype != -2)
		{
			return 1;
		}
	}
	return 0;
}

int CRoomObj::getUserPubMicType(uint32 userid) //0: 管 1:计时 2:不限时
{
	for(int i=0; i<nMaxPubMicCount_; ++i) {
		if(pubmicstate_[i].userid == userid) 
		{
			if(pubmicstate_[i].mictimetype == -2)
				return 0; 
			else if(pubmicstate_[i].mictimetype > 0)
				return 1;
			else if(pubmicstate_[i].mictimetype == -1)
				return 2;
		}
	}
	return -1;
}

uint32 CRoomObj::getUserIsOnMic()
{
	if (!m_pRedis) {
		LOG_PRINT(log_error, "[vcbid:%u] redisOpt is NULL", nvcbid_);
		return 0;
	}
	char szKey[32];
	sprintf(szKey, "%s:%u", KEY_HASH_ROOM_INFO, nvcbid_);
	string val;
	if (m_pRedis->redis_hget(szKey, "userid_onmic", val) < 0) {
		LOG_PRINT(log_error, "[vcbid:%u] Failed to get the user on mic", nvcbid_);
		return 0;
	}

	return atoi(val.c_str());
}

int CRoomObj::setOnMicUser(uint32 userid)
{
	if (!m_pRedis) {
		LOG_PRINT(log_error, "[vcbid:%u] redisOpt is NULL", nvcbid_);
		return -1;
	}
	char szKey[32], szUserid[32];
	sprintf(szKey, "%s:%u", KEY_HASH_ROOM_INFO, nvcbid_);
	sprintf(szUserid, "%u", userid);
	string val = szUserid;
	if (m_pRedis->redis_hset(szKey, "userid_onmic", val) < 0) {
		LOG_PRINT(log_error, "[vcbid:%u,userid:%u] Failed to set the user on mic", nvcbid_, userid);
		return -1;
	}

	return 0;
}

void CRoomObj::downPublicMic(UserObject_Ref& userobjref)
{
    	//主房间下麦
	int ret = clearPubMicByUserid(userobjref->nuserid_);
	if(ret >= 0)
		forceUserDownMic(userobjref);
	LOG_PRINT(log_info,"clearPubMicByUserid(userobjref->nuserid_):ret :%d\n",ret);

}

void CRoomObj::forceUserDownMic(UserObject_Ref& userobjref)
{
	userobjref->ncuronmictype_ = 0;
	userobjref->ncurpublicmicindex_ = -1;
	userobjref->inroomstate_ &= ~FT_ROOMUSER_STATUS_PUBLIC_MIC;

	char szOutBuf[2048] = {0};
	COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *)szOutBuf;
	in_msg->version = MDM_Version_Value;
	in_msg->checkcode = CHECKCODE;
	in_msg->maincmd = MDM_Vchat_Room;
	in_msg->reqid = 0;
	in_msg->subcmd = Sub_Vchat_SetMicStateNotify;
	ClientGateMask_t * pGateMask = (ClientGateMask_t *)(in_msg->content);

	CMDUserMicState pOutCmd;
	pOutCmd.set_micindex(-1);
	pOutCmd.set_micstate(0);
	pOutCmd.mutable_runid()->set_userid(userobjref->nuserid_);
	pOutCmd.mutable_runid()->set_head(userobjref->headpic_);
	pOutCmd.mutable_runid()->set_alias(userobjref->calias_);
	pOutCmd.mutable_toid()->set_userid(userobjref->nuserid_);
	pOutCmd.mutable_toid()->set_head(userobjref->headpic_);
	pOutCmd.mutable_toid()->set_alias(userobjref->calias_);
	pOutCmd.set_vcbid(nvcbid_);
	pOutCmd.set_micindex(userobjref->ncurpublicmicindex_);
	SERIALIZETOARRAY_GATE(pOutCmd, in_msg);
	castSendMsgXXX(in_msg);

	redisSetRoomInfo();

	LOG_PRINT(log_info, "[micstate:%d,vcbid:%u,user:%u,runner:%u] setmicstate end, successfully", pOutCmd.micstate(), pOutCmd.vcbid(), pOutCmd.toid().userid(), pOutCmd.runid().userid());
}

void CRoomObj::delPubMicByUserId(uint32 userId)
{
	if (nuseridonmic_ == userId) {
		nteacherid_ = 0;
		nmroomid_ = 0;
		nuseridonmic_ = 0;
		update_zhuanbo_robot_DB(nvcbid_, 0, userId);
		m_pRoomMgr->set_subroom_micstate(nvcbid_, userId, 1, 0, 0);
	}
	else {
		LOG_PRINT(log_error, "[vcbid:%u,user:%u] user is not on mic", nvcbid_, userId);
	}
}

int CRoomObj::clearPubMicByUserid(uint32 userId)
{
	getPubMicState();
	bool bfound = false;
    for(int i=0; i<nMaxPubMicCount_; ++i)
	{
		if(pubmicstate_[i].userid == userId)
		{
			nteacherid_ = 0;
			nmroomid_ = 0;

			pubmicstate_[i].userid = 0;
			pubmicstate_[i].userendtime = 0;
			pubmicstate_[i].onmic = 0;
			nCurPubMicCount_ --;
			LOG_PRINT(log_info, "room_tag nCurPubMicCount_: %d, user:%u, room:%u", nCurPubMicCount_, userId, nvcbid_);
			bfound = true;
			clearTimerid();
			updateMicState();
			return i;
		}
	}
    if (!bfound)
       	LOG_PRINT(log_error, "not found user:%u record in pubmicstate_ data", userId);
    return -1;
}

void CRoomObj::sendGroupVisitCount(task_proc_data * task_node)
{
	CMDQryGroupVisitCountResp oRspData;
	oRspData.set_groupid(nvcbid_);
	oRspData.set_visitcount(getVisitorNum());
	LOG_PRINT(log_debug, "vcbid: %u, visitor num: %u", oRspData.groupid(), oRspData.visitcount());
	task_node->respProtobuf(oRspData, Sub_Vchat_QryGroupVisitCountResp);
}

void CRoomObj::sendMemberList(task_proc_data * task_node, uint32 userId, int64 gateObj, int64 gateObjId)
{
	COM_MSG_HEADER * in_msg = (COM_MSG_HEADER *)task_node->pdata;
	ClientGateMask_t * pGateMask = (ClientGateMask_t *)in_msg->content;
	LOG_PRINT(log_info, "[connid=%llu,userid=%u,roomid=%u]send user list", pGateMask->param2, userId, nvcbid_);

	std::list<CMDRoomUserInfo> userLst;
	getGroupMemLst(userLst);

	// begin send user list
	char cBuf[256] = {0};
	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)cBuf;
	pHead->version = MDM_Version_Value;
	pHead->checkcode = CHECKCODE;
	pHead->maincmd = MDM_Vchat_Room;
	pHead->subcmd = Sub_Vchat_RoomUserListBegin;
	pHead->reqid = in_msg->reqid;
	pHead->length = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE;

	ClientGateMask_t * pClientGate = (ClientGateMask_t *)(pHead->content);
	memset(pClientGate, 0, SIZE_IVM_CLIENTGATE);
	memcpy(pClientGate, pGateMask, SIZE_IVM_CLIENTGATE);
	task_node->connection->write_message(cBuf, pHead->length);

	//max 50
	int page_cnt = 0;

	CMDRoomUserInfos rspMemLst;
	std::list<CMDRoomUserInfo>::iterator iter_member = userLst.begin();
	for (; iter_member != userLst.end(); ++iter_member)
	{
		CMDRoomUserInfo * pMemInfo = rspMemLst.add_userinfo();
		pMemInfo->CopyFrom(*iter_member);
		++page_cnt;
		if (page_cnt == 50)
		{
			unsigned int rspDataLen = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + rspMemLst.ByteSize();
			SL_ByteBuffer buff(rspDataLen);
			buff.data_end(rspDataLen);

			COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)buff.buffer();
			pHead->version = MDM_Version_Value;
			pHead->checkcode = CHECKCODE;
			pHead->maincmd = MDM_Vchat_Room;
			pHead->subcmd = Sub_Vchat_RoomUserListResp;
			pHead->reqid = in_msg->reqid;
			pHead->length = rspDataLen;

			ClientGateMask_t * pGate = (ClientGateMask_t *)(pHead->content);
			memcpy(pGate, pGateMask, SIZE_IVM_CLIENTGATE);
			char * pRsp = (char *)(pHead->content + SIZE_IVM_CLIENTGATE);
			rspMemLst.SerializeToArray(pRsp, rspMemLst.ByteSize());

			task_node->connection->write_message(buff);
			LOG_PRINT(log_info, "[connid=%llu,userid=%u,roomid=%u]send user data:[%d]", pGateMask->param2, userId, nvcbid_, page_cnt);
			page_cnt = 0;
			rspMemLst.Clear();
		}
	}

	if (page_cnt)
	{
		unsigned int rspDataLen = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + rspMemLst.ByteSize();
		SL_ByteBuffer buff(rspDataLen);
		buff.data_end(rspDataLen);

		COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)buff.buffer();
		pHead->version = MDM_Version_Value;
		pHead->checkcode = CHECKCODE;
		pHead->maincmd = MDM_Vchat_Room;
		pHead->subcmd = Sub_Vchat_RoomUserListResp;
		pHead->reqid = in_msg->reqid;
		pHead->length = rspDataLen;

		ClientGateMask_t * pGate = (ClientGateMask_t *)(pHead->content);
		memcpy(pGate, pGateMask, SIZE_IVM_CLIENTGATE);
		char * pRsp = (char *)(pHead->content + SIZE_IVM_CLIENTGATE);
		rspMemLst.SerializeToArray(pRsp, rspMemLst.ByteSize());

		task_node->connection->write_message(buff);
		LOG_PRINT(log_info, "[connid=%llu,userid=%u,roomid=%u]send user data:[%d]", pGateMask->param2, userId, nvcbid_, page_cnt);
	}

   //finish
   pHead->maincmd = MDM_Vchat_Room;
   pHead->subcmd = Sub_Vchat_RoomUserListFinished;
   task_node->connection->write_message(cBuf, pHead->length);
}

void CRoomObj::sendVisitorList(vector<UserObject_Ref> & vecUserRef)
{
	vector<string> room_userids;
	getAllVisitorid(room_userids);
	LOG_PRINT(log_info, "room:%u, have %u userids in visitor list", nvcbid_, room_userids.size());

	const uint GROUPCOUNT = 100;
	uint nItemCount = 0;
	vector<string> vecUserids;

	// read all visitor info in room from redis
	for (size_t i = 0; i < room_userids.size(); i++) {
		nItemCount++;
		vecUserids.push_back(room_userids[i]);
		if (nItemCount == GROUPCOUNT || (nItemCount < GROUPCOUNT && room_userids.size() - 1 == i)) {
			loadBatchVisitor(vecUserids, vecUserRef);
			vecUserids.clear();
			nItemCount = 0;
		}
	}
}

void CRoomObj::setRoomUserInfo(CMDRoomUserInfo& info, UserObject_Ref &userobjRef)
{
	if ( !userobjRef) {
		return;
	}

	info.set_vcbid(nvcbid_);
	info.set_userid(userobjRef->nuserid_);
	info.set_devtype(userobjRef->ndevtype);	//设备类型
	info.set_chead(userobjRef->headpic_);
	info.set_userlevel(userobjRef->nuserlevel_);
	info.set_userviplevel( userobjRef->nuserviplevel_);
	info.set_roomlevel( userobjRef->ninroomlevel_);
	info.set_gender(userobjRef->ngender_);
	info.set_pubmicindex(userobjRef->ncurpublicmicindex_);
	info.set_userstate( userobjRef->inroomstate_);
	info.set_cometime( userobjRef->cometime_);
	info.set_bforbidchat( userobjRef->bForbidChat_);
	info.set_useralias(userobjRef->calias_.c_str());
	info.set_usertype(userobjRef->type_);
	if (userobjRef->inroomstate_ > 0)
	   LOG_PRINT(log_info, "user:%u, inroomstate:%d, room:%u", userobjRef->nuserid_, userobjRef->inroomstate_, nvcbid_);
}

void CRoomObj::sendPubMicState(task_proc_data *task_node,int64 gateObj,int64 gateObjId)
{
	int page_cnt = 0;
	int memsize = 256;
	char *szOutBuf = (char*)malloc(memsize);
	if (!szOutBuf){
	   LOG_PRINT(log_error, "Failed to allocate memory.");
	   return;
	}

	ClientGateMask_t* pClientGate;
	uint32 tNow = time(0);
	COM_MSG_HEADER* pHead = (COM_MSG_HEADER *)szOutBuf;
	pHead->version = MDM_Version_Value;
	pHead->checkcode = CHECKCODE;
	pHead->maincmd = MDM_Vchat_Room;
	pHead->subcmd = Sub_Vchat_RoomPubMicState;
	pClientGate=(ClientGateMask_t*)(pHead->content);
	memset(pClientGate,0,sizeof(ClientGateMask_t));
	int* pItemCount= (int*)(pHead->content + SIZE_IVM_CLIENTGATE);
	CMDRoomPubMicState_t* pItem = (CMDRoomPubMicState_t *)(pHead->content+ SIZE_IVM_CLIENTGATE +sizeof(int));

	page_cnt = nMaxPubMicCount_;
	for(int i=0; i< nMaxPubMicCount_; ++i)
	{
	    pItem->micid        = i;
		pItem->mictimetype  = pubmicstate_[i].mictimetype;
		pItem->userid       = pubmicstate_[i].userid;
		pItem->userlefttime = 0;

		if(pubmicstate_[i].userid != 0)
		{
			if(pubmicstate_[i].userendtime == 0)
			pItem->userlefttime = -1;
			else if(pubmicstate_[i].userendtime >= tNow)
				pItem->userlefttime = pubmicstate_[i].userendtime - tNow; 
		}
		
		pItem++;
	}
	(*pItemCount) = page_cnt;
	pClientGate->param1 =gateObj;
	pClientGate->param2 =gateObjId;
	pHead->length = SIZE_IVM_HEADER +SIZE_IVM_CLIENTGATE + sizeof(int) + sizeof(CMDRoomPubMicState_t) *page_cnt;
	task_node->connection->write_message(szOutBuf, pHead->length);

	if (szOutBuf)
		free(szOutBuf);
}

void CRoomObj::sendRoomNotice(task_proc_data *task_node,int64 gateObj,int64 gateObjId)
{
	int memsize = 4096;
	char *szOutBuf = (char*)malloc(memsize);
	if (!szOutBuf){
	   LOG_PRINT(log_error, "Failed to allocate memory.");
	   return;
	}

	COM_MSG_HEADER *pHead = (COM_MSG_HEADER *)szOutBuf;
	pHead->version = 10;
	pHead->checkcode = 0;
	pHead->maincmd = MDM_Vchat_Room;
	pHead->subcmd = Sub_Vchat_RoomNoticeNotify;
	ClientGateMask_t* pClientGate=(ClientGateMask_t*)(pHead->content);
	CMDRoomNotice_t* pInfo = (CMDRoomNotice_t*)(pHead->content+ SIZE_IVM_CLIENTGATE);
	for(int i=0; i<4; ++i)
	{
		if(m_RoomNoticeArray[i].m_strRoomNotice.length()>0)
		{
			pInfo->vcbid = nvcbid_;
			pInfo->index = i;
			pInfo->textlen = m_RoomNoticeArray[i].m_strRoomNotice.length() + 1;
			strcpy(pInfo->content, m_RoomNoticeArray[i].m_strRoomNotice.c_str());

			pHead->length = SIZE_IVM_HEADER +SIZE_IVM_CLIENTGATE +sizeof(CMDRoomNotice_t) + pInfo->textlen;
			pClientGate->param1=gateObj;
			pClientGate->param2=gateObjId;
			task_node->connection->write_message(szOutBuf, pHead->length);

		}
	}
	if (szOutBuf)
		free(szOutBuf);
}

//发送房间公告,带发布者信息版本
void CRoomObj::sendRoomNoticeInfoNotify(task_proc_data *task_node,int64 gateObj,int64 gateObjId)
{
	char szOutBuf[4096] = {0};
	COM_MSG_HEADER *pHead = (COM_MSG_HEADER *)szOutBuf;
	pHead->version = 10;
	pHead->checkcode = 0;
	pHead->maincmd = MDM_Vchat_Room;
	pHead->subcmd = Sub_Vchat_RoomNoticeInfoNotify;
	ClientGateMask_t* pClientGate=(ClientGateMask_t*)(pHead->content);
	tag_CMDRoomNoticeNoty* pInfo = (tag_CMDRoomNoticeNoty*)(pHead->content+ SIZE_IVM_CLIENTGATE);
	for(int i=0; i<4; ++i)
	{
		if(m_RoomNoticeArray[i].m_strRoomNotice.length()>0)
		{
			pInfo->vcbid = nvcbid_;
			pInfo->userid = m_RoomNoticeArray[i].m_uBroadUserid;
			pInfo->index = i;
			pInfo->textlen = m_RoomNoticeArray[i].m_strRoomNotice.length() +1;
			strcpy(pInfo->content, m_RoomNoticeArray[i].m_strRoomNotice.c_str());

			pHead->length = SIZE_IVM_HEADER +SIZE_IVM_CLIENTGATE +sizeof(tag_CMDRoomNoticeNoty) + pInfo->textlen;
			pClientGate->param1=gateObj;
			pClientGate->param2=gateObjId;
			task_node->connection->write_message(szOutBuf, pHead->length);
		}
	}
}

void CRoomObj::sendJoinRoomResp(task_proc_data *task_node, UserObject_Ref &userobjRef, uint64 gateObj, uint64 gateObjId, int biscollectroom /* = 0*/)
{
	int memsize = 1024;
	char * szBuf = (char *)malloc(memsize);
	if (!szBuf){
	   LOG_PRINT(log_error, "Failed to allocate memory.");
	   return;
	}
	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)(task_node->pdata);
	ClientGateMask_t * pClientGate;
	if(userobjRef)	{
		int nSendLen = Build_NetMsg_JoinRoomRespPack(szBuf, memsize, userobjRef, &pClientGate, biscollectroom);
		COM_MSG_HEADER * pHead1 = (COM_MSG_HEADER *)(szBuf);
		pHead1->reqid = pHead->reqid;
		pClientGate->param1 = gateObj;
		pClientGate->param2 = gateObjId;
		LOG_PRINT(log_info, "will send join room msg to gateway, userid=%d, roomid=%d, clientobject=%llu, connid=%llu!", userobjRef->nuserid_,
				nvcbid_, gateObj, gateObjId);
		task_node->connection->write_message(szBuf, nSendLen);
	}
	else {
		LOG_PRINT(log_error, "not found user:%u in room:%u", userobjRef->nuserid_, nvcbid_);
	}
	if (szBuf)
		free(szBuf);
}

void CRoomObj::sendAdKeywordsList(task_proc_data *task_node, int64 gateObj,int64 gateObjId)
{
	int memsize = 4096;
	char *szOutBuf = (char*)malloc(memsize);
	if (!szOutBuf){
	   LOG_PRINT(log_error, "Failed to allocate memory.");
	   return;
	}

	const int batchnum = 20;
	char *pInbuff = (char*)malloc(sizeof(CMDAdKeywordInfo_t) * batchnum);
	if (!pInbuff){
	   LOG_PRINT(log_error, "Failed to allocate memory.");
	   free(szOutBuf);
	   return;
	}
	int donenum = 0, count;
	ClientGateMask_t* pClientGate;

	{
		READ_LOCK(m_pRoomMgr->m_mtxKeywordMap);
		count = m_pRoomMgr->m_keyword_map.size();
		CRoomManager::KEYWORD_MAP::iterator iter = m_pRoomMgr->m_keyword_map.begin();
		for(; iter != m_pRoomMgr->m_keyword_map.end(); ++iter){
			count--;

			memcpy(pInbuff + sizeof(CMDAdKeywordInfo_t) * donenum++, &(iter->second), sizeof(CMDAdKeywordInfo_t));
			if (donenum == batchnum || (count == 0)){
				int nSendLen = Build_NetMsg_AdKeyworkListPack(szOutBuf, pInbuff, donenum * sizeof(CMDAdKeywordInfo_t), &pClientGate);
				pClientGate->param1 = gateObj;
				pClientGate->param2 = gateObjId;
				task_node->connection->write_message(szOutBuf, nSendLen);
				donenum = 0;
			}
		}
	}

	if (pInbuff)
		free(pInbuff);
	if (szOutBuf)
		free(szOutBuf);
}

void CRoomObj::sendRoomAndSubRoomId(task_proc_data *task_node, int64 gateObj,int64 gateObjId)
{
	ClientGateMask_t* pClientGate;
	COM_MSG_HEADER* pHead = NULL;

	int memsize = 1024;
	char *szBuf = (char*)malloc(memsize);
    if (!szBuf){
 	   LOG_PRINT(log_error, "Failed to allocate memory.");
 	   return;
    }

	pHead = (COM_MSG_HEADER *)szBuf;
	pHead->version = 10;
	pHead->checkcode = 0;
	pHead->maincmd = MDM_Vchat_Room;
	pHead->subcmd = Sub_Vchat_RoomAndSubRoomId_Noty;
	pClientGate=(ClientGateMask_t*)(pHead->content);
	memset(pClientGate,0,sizeof(ClientGateMask_t));
	CMDRoomAndSubRoomIdNoty_t* pInfo = (CMDRoomAndSubRoomIdNoty_t *)(pHead->content+ SIZE_IVM_CLIENTGATE);

	if(0 == nteacherid_)
	{
		LOG_PRINT(log_info, "teacherid is 0, return");
		goto FUNC_EXIT;
	}

	if(0 == nmroomid_)
	{
		pInfo->roomid = nvcbid_;
		pInfo->subroomid = 0;
	}
	else
	{
		pInfo->roomid = nmroomid_;
		pInfo->subroomid = nvcbid_;
	}
	LOG_PRINT(log_info, "pInfo->roomid:%u, pInfo->subroomid:%u.", pInfo->roomid, pInfo->subroomid);
	
	pClientGate->param1 =gateObj;
	pClientGate->param2 =gateObjId;
	pHead->length = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + sizeof(CMDRoomAndSubRoomIdNoty_t);
	task_node->connection->write_message(szBuf, pHead->length);

FUNC_EXIT:
	if (szBuf)
		free(szBuf);
}

void CRoomObj::sendTeacherUserid(task_proc_data *task_node, int64 gateObj,int64 gateObjId)
{
	int memsize = 4096;
    char *szBuf = (char*)malloc(memsize);
    if (!szBuf){
 	   LOG_PRINT(log_error, "Failed to allocate memory.");
 	   return;
    }
    ClientGateMask_t* pClientGate;
    int len = Build_NetMsg_TeacheridPack(szBuf, memsize, &pClientGate);
    if (len > 0)
    {
        pClientGate->param1 =gateObj;
        pClientGate->param2 =gateObjId;
        task_node->connection->write_message(szBuf, len);
    }

    if (szBuf)
    	free(szBuf);
}

void CRoomObj::castSendTeacherUserid()
{
	int memsize = 4096;
	char *szBuf = (char*)malloc(memsize);
    if (!szBuf){
 	   LOG_PRINT(log_error, "Failed to allocate memory.");
 	   return;
    }
	ClientGateMask_t* pClientGate;
	int len = Build_NetMsg_TeacheridPack(szBuf, memsize, &pClientGate);
	if (len > 0)
		castSendMsgXXX((COM_MSG_HEADER*)szBuf);

	if (szBuf)
		free(szBuf);
}

void CRoomObj::castSendMsgXXX(COM_MSG_HEADER* pOutMsg, uint32 userId/* = 0*/, bool inGroupOnly/* = false*/)
{
	castSendMsgXXX_i(pOutMsg, userId, inGroupOnly);
}

void CRoomObj::castSendMsgXXX_i(COM_MSG_HEADER* pOutMsg, uint32 userId/* = 0*/, bool inGroupOnly/* = false*/)
{
	UserObject_Ref userObjRef;
	unsigned int except_connid = 0;

	ClientGateMask_t* pClientGate = (ClientGateMask_t*)pOutMsg->content;
	memset(pClientGate, 0, sizeof(ClientGateMask_t));
	if (0 != userId && findVisitUser(userObjRef, userId)){
		except_connid = userObjRef->pGateObjId_;
	}

	CMsgComm::Build_BroadCastRoomGate(pClientGate, e_Notice_AllType, nvcbid_, except_connid, inGroupOnly);

	map<uint32, clienthandler_ptr>::const_iterator it = m_pRoomMgr->m_mapConn.begin();
	for(; it!=m_pRoomMgr->m_mapConn.end(); it++) {
		clienthandler_ptr pclient = it->second;
		if (pclient && pclient->isconnected()){
			pclient->write_message((char*)pOutMsg, pOutMsg->length);
		}
	}
}

void CRoomObj::castSendRoomMaxclientnum()
{
	char szBuf[1024]={0};
	COM_MSG_HEADER* pHead=(COM_MSG_HEADER*)szBuf;
	pHead->version=MDM_Version_Value;
	pHead->checkcode=CHECKCODE;
	pHead->maincmd=MDM_Vchat_Room;
	pHead->subcmd=Sub_Vchat_MaxClientNum;
	ClientGateMask_t* pClientGate=(ClientGateMask_t*)(pHead->content);
	memset(pClientGate,0,sizeof(ClientGateMask_t));
	int *p = (int *)(szBuf + SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE);
	*p = nMaxclientNum_;
	pHead->length = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + sizeof(int);
	if (m_pRoomMgr)
		m_pRoomMgr->castRoom_onesvr((COM_MSG_HEADER*)szBuf, nvcbid_);
	LOG_PRINT(log_debug, "[vcbid:%d] maxclientnum:%d", nvcbid_, nMaxclientNum_);
}

void CRoomObj::castSendRoomUserComeInfo(UserObject_Ref &userobjRef)
{
	if (userobjRef.get() == NULL)
	{
		LOG_PRINT(log_error, "userobjRef is null,not need to castSendRoomUserComeInfo.");
		return;
	}

	int memsize = 512;
	char *szBuf = (char*)malloc(memsize);
    if (!szBuf){
 	   LOG_PRINT(log_error, "Failed to allocate memory.");
 	   return;
    }
	ClientGateMask_t* pClientGate;
	Build_NetMsg_RoomUserComePack(szBuf, memsize, userobjRef, &pClientGate);
	castSendMsgXXX((COM_MSG_HEADER*)szBuf, userobjRef->nuserid_);

	if (szBuf)
		free(szBuf);
}

void CRoomObj::castSendRoomUserLeftInfo(UserObject_Ref &userobjRef)
{
	if (userobjRef.get() == NULL)
	{
		LOG_PRINT(log_error, "userobjRef is null,not need to castSendRoomUserLeftInfo.");
		return;
	}

	char szBuf[512] = {0};
	ClientGateMask_t* pClientGate;
	Build_NetMsg_RoomUserLeftPack(szBuf, 512, userobjRef, &pClientGate);
	castSendMsgXXX_i((COM_MSG_HEADER*)szBuf, userobjRef->nuserid_, 0);
}

void CRoomObj::castSendRoomUserExceptLeftInfo(UserObject_Ref &userobjRef)
{
	if (userobjRef.get() == NULL)
	{
		LOG_PRINT(log_error, "userobjRef is null,not need to castSendRoomUserExceptLeftInfo.");
		return;
	}
	
	char szBuf[512] = {0};
	ClientGateMask_t* pClientGate;
//	Build_NetMsg_RoomUserExceptLeftPack(szBuf, 512, userobjRef, &pClientGate);
	Build_NetMsg_RoomUserLeftPack(szBuf, 512, userobjRef, &pClientGate);
	castSendMsgXXX_i((COM_MSG_HEADER*)szBuf, userobjRef->nuserid_, 0);
}

void CRoomObj::castSendRoomKickoutUserInfo(uint32 srcId, UserObject_Ref &toUserobjRef, int reasionId, int minsId)
{
	if (toUserobjRef.get() == NULL)
	{
		LOG_PRINT(log_error, "toUserobjRef is null,not need to castSendRoomKickoutUserInfo.");
		return;
	}

	char szBuf[512] = {0};
	ClientGateMask_t* pClientGate;
	Build_NetMsg_RoomKickoutUserPack(szBuf, 512, srcId, toUserobjRef, reasionId, minsId,&pClientGate);
	castSendMsgXXX_i((COM_MSG_HEADER*)szBuf, toUserobjRef->nuserid_, 0);
}

void CRoomObj::castSendRoomBaseInfo(void)
{
	char szBuf[4096] = {0};
	ClientGateMask_t* pClientGate;
	if (Build_NetMsg_RoomBaseInfoPack(szBuf, 4096, &pClientGate))
	{
		castSendMsgXXX((COM_MSG_HEADER *)szBuf);
	}
	else
	{
		LOG_PRINT(log_error, "cast room base info fail.because buff size if not enough.");
	}
}

void CRoomObj::castSendRoomOPStatus(void)
{
	char szBuf[512] = {0};
	ClientGateMask_t* pClientGate;
	Build_NetMsg_RoomOPStatusPack(szBuf, 512,&pClientGate);
	castSendMsgXXX((COM_MSG_HEADER*)szBuf);
}

void CRoomObj::castSendRoomMediaInfo(void)
{
	char szBuf[512] = {0};
	ClientGateMask_t* pClientGate;
	Build_NetMsg_RoomMediaPack(szBuf, 512, &pClientGate);
	castSendMsgXXX((COM_MSG_HEADER*)szBuf);
}

void CRoomObj::OpenAllPubMic()
{
	for(int i=0; i<nMaxPubMicCount_; ++i)
	{
		pubmicstate_[i].mictimetype=-1;
	}
}

void CRoomObj::SetWaitMicMode(bool bVal)
{
	if(bVal)
		nopstate_ |= FT_ROOMOPSTATUS_OPEN_WAITMIC;
	else
		nopstate_ &= ~FT_ROOMOPSTATUS_OPEN_WAITMIC;
}

void CRoomObj::SetChairManMode(bool bVal)
{
	if(bVal)
		nopstate_ |= FT_ROOMOPSTATUS_CHAIR_ROOM;
	else
		nopstate_ &= ~FT_ROOMOPSTATUS_CHAIR_ROOM;
}

bool CRoomObj::changePubMicState(uint32 ruunerId, int micId, int optype, int param1)
{
	if(micId <0 || micId >= nMaxPubMicCount_) 
		return false;
	PubMicState_t * pMicState = &(pubmicstate_[micId]);
	if(optype == 1)
	{
		//延长当前用户麦时: 只有在有用户有麦时的情况下才能延长麦时
		if(pMicState->userid != 0 && pMicState->mictimetype>0)
		{
			pMicState->userendtime += (param1 * 60); //秒
			return true;
		}
	}
	else if(optype == 2)
	{
		//设置麦时属性: 改变麦时状态
		//TODO: 对于现有在公麦用户,是否收到影响？ 未实现
		int mictype = param1;
		if(mictype != pMicState->mictimetype && mictype >= -2) 
		{
			pMicState->mictimetype = mictype;
			return true;
		}
	}
	return false;
}

bool CRoomObj::lootUserMic(uint32 runnerId, uint32 userId, int micId)
{
	//说明: 夺麦,夺麦不会产生新麦上下，因为是替换关系，因此不应该触发check排麦上公麦事件！
	//0.上层已经通过权限判断,判断这两个用户都在房间。
	char szBuf[512];
	ClientGateMask_t* pClientGate;

    UserObject_Ref pSrcUserRef, pToUsrRef;
	findVisitUser(pSrcUserRef, runnerId);
	findVisitUser(pToUsrRef, userId);

	//1. 判断自己非公麦/私麦/密麦/收费麦 状态 才能夺麦
	if(pSrcUserRef->inroomstate_ & (FT_ROOMUSER_STATUS_PUBLIC_MIC | FT_ROOMUSER_STATUS_PRIVE_MIC | FT_ROOMUSER_STATUS_SECRET_MIC | FT_ROOMUSER_STATUS_CHARGE_MIC) )
		return false;

	//mic id 有效性判断
	if(micId <0 || micId >= nMaxPubMicCount_) 
		return false;

	//2. 判断当前公麦id 对应的用户是该用户 (麦上有人,并且是该用户)
	if(pubmicstate_[micId].userid != userId || !(pToUsrRef->inroomstate_ & FT_ROOMUSER_STATUS_PUBLIC_MIC))
		return false;

	//3. 让该用户下麦,广播其状态
	pToUsrRef->inroomstate_ &= ~FT_ROOMUSER_STATUS_PUBLIC_MIC;
	//assert(pToUsrRef->inroomstate_ == 0); //gch-- 20150420
	pToUsrRef->ncurpublicmicindex_ = -1;
	pToUsrRef->ncuronmictype_= 0;

	pubmicstate_[micId].userid = 0;
	pubmicstate_[micId].userendtime = 0;

	// 生成广播消息Sub_Vchat_SetMicStateNotify
	CMDUserMicState_t stateInfo;
	memset(&stateInfo,0,sizeof(CMDUserMicState_t));
	stateInfo.vcbid = nvcbid_;
	stateInfo.runid = runnerId;
	stateInfo.toid = userId;
	stateInfo.micstate = 0;  //下麦
	stateInfo.micindex = -1;
    int nMsgLen = m_pRoomMgr->Build_NetMsg(szBuf,512,MDM_Vchat_Room,Sub_Vchat_SetMicStateNotify,&pClientGate,&stateInfo,sizeof(CMDUserMicState_t));
	if(nMsgLen > 0)
	    castSendMsgXXX((COM_MSG_HEADER*)szBuf);

	//4. 让自己上麦,广播其状态
	pSrcUserRef->inroomstate_ |= FT_ROOMUSER_STATUS_PUBLIC_MIC;
	pSrcUserRef->ncurpublicmicindex_ = micId;
	pSrcUserRef->nstarttime_mic_ = time(0);
	pSrcUserRef->ncuronmictype_= 1;

	pubmicstate_[micId].userid = pSrcUserRef->nuserid_;
	pubmicstate_[micId].userendtime = 0;

	// 生成广播消息Sub_Vchat_SetMicStateNotify
	memset(&stateInfo, 0, sizeof(CMDUserMicState_t));
	stateInfo.vcbid = nvcbid_;
	stateInfo.runid = runnerId;
	stateInfo.toid = runnerId;
	stateInfo.micstate = FT_ROOMUSER_STATUS_PUBLIC_MIC;  //上公麦
	stateInfo.micindex = micId;
	//pInfo2->optype=;

	nMsgLen = m_pRoomMgr->Build_NetMsg(szBuf,512,MDM_Vchat_Room,Sub_Vchat_SetMicStateNotify,&pClientGate, &stateInfo,sizeof(CMDUserMicState_t));
	if(nMsgLen > 0)
	   castSendMsgXXX((COM_MSG_HEADER*)szBuf);
	return true;
}

int CRoomObj::setUserMicState_0(UserObject_Ref &userobjRef, int newMicState, int nspecmicindex, int giftid, int giftnum, bool& bWaitMicListChanged)
{
	//房间状态判断,是否允许上麦,关闭公麦
	if((newMicState & FT_ROOMUSER_STATUS_PUBLIC_MIC) && (nopstate_ & FT_ROOMOPSTATUS_CLOSE_PUBMIC))
		return -2;
    //关闭私麦
	if( (newMicState & (FT_ROOMUSER_STATUS_PRIVE_MIC| FT_ROOMUSER_STATUS_SECRET_MIC| FT_ROOMUSER_STATUS_CHARGE_MIC))
		&& (nopstate_ & FT_ROOMOPSTATUS_CLOSE_PRIVATEMIC))
		return -2;
	return setUserMicState(userobjRef, newMicState, nspecmicindex, giftid, giftnum, bWaitMicListChanged);
}

int CRoomObj::setUserMicState(UserObject_Ref &pUserRef, int newMicState, int nspecmicindex, int giftid, int giftnum, bool& bWaitMicListChanged)
{
	int nRet;

	int oldState = pUserRef->inroomstate_;

	LOG_PRINT(log_warning, " room_tag user:%u in room:%u is not FT_ROOMUSER_STATUS_PUBLIC_MIC state, down mic.new:%d,old:%d",
			pUserRef->nuserid_, nvcbid_,newMicState,newMicState);
//	castSendTeacherUserid();
	if(newMicState == 0)
	{
		LOG_PRINT(log_warning, " room_tag user:%u in room:%u is not FT_ROOMUSER_STATUS_PUBLIC_MIC state, down mic.1111",pUserRef->nuserid_, nvcbid_);
		//下麦
//		if(oldState & FT_ROOMUSER_STATUS_PUBLIC_MIC)
		{
//			setTeacherid(pUserRef->nuserid_, newMicState);
			updateActiveTime();
			int ret = clearPubMicByUserid(pUserRef->nuserid_);
			if(ret >= 0)
			{
				pUserRef->ncuronmictype_= 0;
				pUserRef->ncurpublicmicindex_ = ret;

			}else
			{
				return -1;
			}

			LOG_PRINT(log_warning, "room_tag user:%u in room:%u is not FT_ROOMUSER_STATUS_PUBLIC_MIC state, down mic.222",pUserRef->nuserid_, nvcbid_);
		}
//		else{
//			uint32 onmic_userid = getUserIsOnMic();
//			if (onmic_userid){
//				LOG_PRINT(log_warning, "user:%u in room:%u is not FT_ROOMUSER_STATUS_PUBLIC_MIC state, cannot down mic. user:%u is on mic",
//						pUserRef->nuserid_, nvcbid_, onmic_userid);
//			}
//			else
//				LOG_PRINT(log_warning, "user:%u in room:%u is not FT_ROOMUSER_STATUS_PUBLIC_MIC state, cannot down mic", pUserRef->nuserid_, nvcbid_);
//			if (onmic_userid != pUserRef->nuserid_)
//				return -1;
//		}
	}
	else if(newMicState == FT_ROOMUSER_STATUS_PUBLIC_MIC)
	{
		//上公麦
		//如果用户处于 私，密，收，不能上公麦,只能先下麦(不能继续操作)
//		if(oldState & FT_ROOMUSER_STATUS_SECRET_MIC)
//			return -2;
//		if(oldState & FT_ROOMUSER_STATUS_PRIVE_MIC)
//			return -2;
//		if(oldState & FT_ROOMUSER_STATUS_CHARGE_MIC)
//			return -2;
//		if(oldState & FT_ROOMUSER_STATUS_PUBLIC_MIC)
//		{
//			//用户已经在公麦上,为什么?
//			LOG_PRINT(log_error, "error:[userid=%d] already in up mic state.", pUserRef->nuserid_);
//			return -2;
//		}
//		if(nCurPubMicCount_ >= nMaxPubMicCount_){
//			LOG_PRINT(log_warning, "nCurPubMicCount_: %d, user:%u is on mic, room: %u", nCurPubMicCount_,  pUserRef->nuserid_, nvcbid_);
//			return -1;
//		}

		//删除麦序洗第一个位置(不管在不在麦序里,都删除一次)
		bWaitMicListChanged = delWaitMicUser(pUserRef->nuserid_, 1);
		nRet= addPubMicUser(pUserRef->nuserid_, nspecmicindex, 0);
		LOG_PRINT(log_info, "room_tag user[id=%d] on pubmic[ret=%d, mictype=%d, downtime=%d],nspecmicindex:%d.\n",
						pUserRef->nuserid_, nRet, pubmicstate_[nRet].mictimetype, pubmicstate_[nRet].userendtime,nspecmicindex );
		if(nRet >= 0)
		{
			pUserRef->ncurpublicmicindex_= nRet;

			if(m_micMode == e_QueueMode && pubmicstate_[nRet].onmic == 1)
			{
				addTimerid();
				pUserRef->m_OnMicSec = MIC_TIME_PER;
			}else
			{
				pUserRef->m_OnMicSec = -1;
			}
            //记录
			pUserRef->ncuronmictype_= 1;
			pUserRef->nstarttime_mic_= time(0);
			LOG_PRINT(log_info, "room_tag user[id=%d] on pubmic[micid=%d, mictype=%d, downtime=%d].\n",
				pUserRef->nuserid_, nRet, pubmicstate_[nRet].mictimetype, pubmicstate_[nRet].userendtime );
		}
		else
			return -1;
	}
	else if(newMicState == FT_ROOMUSER_STATUS_PRIVE_MIC)
	{
		//上私麦
		//如果用户有公麦或密麦或收费麦,只能先下麦
		if(oldState & FT_ROOMUSER_STATUS_PUBLIC_MIC)
			return -2;
		if(oldState & FT_ROOMUSER_STATUS_SECRET_MIC)
			return -2;
		if(oldState & FT_ROOMUSER_STATUS_CHARGE_MIC)
			return -2;

		pUserRef->nstarttime_mic_= time(0);
		pUserRef->ncuronmictype_= 2;
	}
	else if(newMicState == FT_ROOMUSER_STATUS_SECRET_MIC)
	{
		//上密麦
		//如果用户有公麦或私麦或收费麦,只能先下麦
		if(oldState & FT_ROOMUSER_STATUS_PRIVE_MIC)
			return -2;
		if(oldState & FT_ROOMUSER_STATUS_PUBLIC_MIC)
			return -2;
		if(oldState & FT_ROOMUSER_STATUS_CHARGE_MIC)
			return -2;
		if(oldState & FT_ROOMUSER_STATUS_CHAIRMAN_MIC)
			return -2;
		pUserRef->nstarttime_mic_= time(0);
		pUserRef->ncuronmictype_= 3;
	}
	else if(newMicState == FT_ROOMUSER_STATUS_CHARGE_MIC)
	{
		//上礼物麦,
//		pUserRef->nmicgitftid_=giftid;
//		pUserRef->nmicgiftnum_=giftnum;

		pUserRef->nstarttime_mic_= time(0);
		pUserRef->ncuronmictype_= 4;
	}

	//设置麦状态(清除旧状态,设置新状态) 2013.09.10 只适用于 sinshow(只允许单人单次排序)
	pUserRef->inroomstate_ &= ~FT_ROOMUSER_STATUS_PUBLIC_MIC;
	pUserRef->inroomstate_ &= ~FT_ROOMUSER_STATUS_PRIVE_MIC;
	pUserRef->inroomstate_ &= ~FT_ROOMUSER_STATUS_SECRET_MIC;
	pUserRef->inroomstate_ &= ~FT_ROOMUSER_STATUS_CHARGE_MIC;
	pUserRef->inroomstate_ |= newMicState;
	LOG_PRINT(log_info, "user:%u, inroomstate:%d, room:%u", pUserRef->nuserid_, pUserRef->inroomstate_, nvcbid_);
	return 0;
}

int CRoomObj::getWaitMicUserCount()
{
	getWaitMics();
	return vecWaitMics_.size();
}

bool CRoomObj::addWaitMicUser(uint32 userId,int nInserIndex,bool once)
{
	//当前版本只允许一个用户排一次麦
	//assert(getUserWaitMicNum(userId) <= 1);//gch--

	//允许无穷尽的进行排序.上层已经判断了(TODO:等待其他部分配合)

	for(int i =0 ; i < MAX_PUBMIC_COUNT ; i++)
	{
		if(pubmicstate_[i].userid == userId)
			return false;
	}
	bool onmic = false;
	for(int i  = 0 ; i < MAX_PUBMIC_COUNT; i++ )
	{
		if(pubmicstate_[i].userid > 0)
		{
			onmic = true;
			break;
		}
	}
	int nret = -1;
	for(int i  = 0 ; i < MAX_PUBMIC_COUNT; i++ )
	{
		if (pubmicstate_[i].userid == 0 && pubmicstate_[i].status == 1 && pubmicstate_[i].mictimetype != 0 && pubmicstate_[i].mictimetype != -2)
		{
			pubmicstate_[i].userid = userId;
			pubmicstate_[i].userendtime = time(0) + pubmicstate_[i].mictimetype * 60;
			pubmicstate_[i].onmic = onmic?0:1;
			nCurPubMicCount_ ++;
			nret = i;
			LOG_PRINT(log_info, "setmicstate nCurPubMicCount_: %d, index:%d user:%u, room:%u", nCurPubMicCount_,i, userId, nvcbid_);
			break;
		}
	}
	if(!onmic)
	{
		addTimerid();
		castSendOnMicState(userId,1,nret,getInterval());
	}
	updateMicState();
	return true;
}

int CRoomObj::getUserWaitMicNum(uint32 userId)
{
	int nCount=0;
	if(vecWaitMics_.size() == 0) return 0;
	for(int i=0; i<vecWaitMics_.size(); i++)
	{
		if(vecWaitMics_[i].userid == userId)
			nCount++;
	}
	return nCount;
}
void CRoomObj::checkPubMicToWaitMic(bool bcast,bool badd)
{
//	if(getMicMode() == e_FreeMode || getMicMode() == e_ChairmanMode)
//		return;
	LOG_PRINT(log_info,"room_tag room:%d checkPubMicToWaitMic",nvcbid_);
	PubMicState_t* p = getPubMicState();
	LOG_PRINT(log_info, "room_tag user,, room:%u MAX_PUBMIC_COUNT:%d",nvcbid_,MAX_PUBMIC_COUNT);
	for(int i = 0 ; i < MAX_PUBMIC_COUNT ; i++)
	{
		int nUserId = p[i].userid;
		LOG_PRINT(log_info, "room_tag user:%u, status:%d, room:%u", nUserId, p[i].status, nvcbid_);
		if(nUserId > 0 && p[i].status == 1)
		{
			int nret = clearPubMicByUserid(nUserId);
			LOG_PRINT(log_info, "room_tag ret:%u.",nret);
			if(bcast && nret >= 0)
			{
				std::string strHead;
				std::string strAlias;
				CUserBasicInfo::getUserHeadAndAlias(nUserId, strHead, strAlias);

				char szOutBuf[1024];
				ClientGateMask_t* pClientGate;
				CMDUserMicState micInfo;
				micInfo.set_vcbid(nvcbid_);
				micInfo.mutable_runid()->set_userid(0);
				micInfo.mutable_toid()->set_userid(nUserId);
				micInfo.mutable_toid()->set_head(strHead);
				micInfo.mutable_toid()->set_alias(strAlias);
				micInfo.set_micindex(nret);
				micInfo.set_micstate(0);

				SERIALIZETOARRAY_BUF(micInfo,respbuf,len);
				int nMsgLen=m_pRoomMgr->Build_NetMsg(szOutBuf,1024,MDM_Vchat_Room,Sub_Vchat_SetMicStateNotify,&pClientGate,respbuf,len);
				if(nMsgLen > 0)
					castSendMsgXXX((COM_MSG_HEADER*)szOutBuf);
				LOG_PRINT(log_info, "room_tag checkPubMicToWaitMic castSendMsgXXX.");
				if(badd)
				{
					addWaitMicUser(nUserId,-1,true);
					LOG_PRINT(log_info, "room_tag checkPubMicToWaitMic addWaitMicUser. bonce:true.");
				}
			}
		}
	}
}
int CRoomObj::checkWaitMicToPubMic(bool bcast ,bool once)
{
	int ret = 0;
    //判断房间是不是可以自动转麦,以下代码只适用于单排麦(排麦和麦序冲突的情况)
//	if(nopstate_ & FT_ROOMOPSTATUS_CLOSE_PUBMIC)
//		return 0;
//	if(!(nopstate_ & FT_ROOMOPSTATUS_OPEN_AUTOPUBMIC))
//		return;
//	if(!(nopstate_ & FT_ROOMOPSTATUS_OPEN_WAITMIC))
//		return;
//	if(nCurPubMicCount_ >= nMaxPubMicCount_)
//		return 0;
	LOG_PRINT(log_info, " room_tag checkWaitMicToPubMic() begin1..nCurPubMicCount_:%d,,nMaxPubMicCount_:%d\n",
			nCurPubMicCount_,nMaxPubMicCount_);
	//有空麦
	//新设计,如果该用户在公麦上,则取后面的排麦用户,直到一个不在公麦上的,如果该用户在私麦的话,自动踢下私麦,换成公麦?
	//lock

	bool bUserFound = false;
	int32 nUserId = 0;
	getWaitMics();
	while(nCurPubMicCount_ < nMaxPubMicCount_)
	{
		//1.取出第一个排麦用户,没有用户则直接break
		nUserId = 0;
		bUserFound = false;
		WaitMicState_t tState;
		if(vecWaitMics_.size() >0)
		{
			tState = vecWaitMics_.front();
			vecWaitMics_.erase(vecWaitMics_.begin());
			nUserId = tState.userid;
		}else
			break;

		//2. 判断用户状态
		if(nUserId >0)
		{
			//用户存在判断,如果到了这个用户, 用户不存在的话(如用户掉线,退出等等), 直接跳过该数据(删除)
			int nret;
			UserObject_Ref pUserObjRef;
			bUserFound = findMemberUser(pUserObjRef, nUserId);
			if( !bUserFound)
			{
				if(allow_visitor_on_mic)
				{
					bUserFound = findVisitUser(pUserObjRef, nUserId);
					if( !bUserFound)
						continue;
				}
				else
					continue;
			}
			//麦状态判断,如果这时用户在其他麦排麦实效 或不是排麦状态? 直接跳过该数据(删除)
			//gch-- 13.12.11
			//if(pUserObjRef->inroomstate_ &  (FT_ROOMUSER_STATUS_PUBLIC_MIC| FT_ROOMUSER_STATUS_PRIVE_MIC |FT_ROOMUSER_STATUS_SECRET_MIC))
			//	continue;

			//gch++ 13.12.11
			if(pUserObjRef->inroomstate_ & FT_ROOMUSER_STATUS_PUBLIC_MIC)
			{
				int nType = getUserPubMicType(nUserId);
				if(nType == 0)//管
				{
				}
				else if(nType == 1)//计
				{
					//增加单位时长
				}
				else if(nType == 2)//无限
				{
				}
				continue;
			}
			//20120226 步骤:------------------------------------------------------------------
			// 1)先检查有没有空公麦? HitTest(增加新函数，目的是先下其他麦)
			// 2)有,则用户下私麦,修改用户房间状态,广播该用户下麦消息->客户端好进行下麦处理
			// 3)用户上公麦(addPubMicUser) ,如上成功,广播该用户上公麦消息->客户端再进行上麦处理
			// 4)对多线程操作的唯一失败可能是先发现有空，下了私后，上公的失败而已，其他都很完美
			// end ---------------------------------------------------------------------------

			nret = HitTestFreePubMic();
			if(nret == 0) //无可用公麦(自由麦或限时麦)
			{
				//测试失败,保存原姿势,停止循环(后面的肯定也成功不了）
				vecWaitMics_.insert(vecWaitMics_.begin(), tState);
				break;
			}
            //有麦可上,但用户在私密上
			if(pUserObjRef->inroomstate_ & (FT_ROOMUSER_STATUS_PRIVE_MIC | FT_ROOMUSER_STATUS_SECRET_MIC))
			{
				//下私密,记录时间
//				recordUserOnMicTime(pUserObjRef);

				//修改状态并且广播
				pUserObjRef->inroomstate_ &= ~FT_ROOMUSER_STATUS_PRIVE_MIC;
				pUserObjRef->inroomstate_ &= ~FT_ROOMUSER_STATUS_SECRET_MIC;
				pUserObjRef->inroomstate_ &= ~FT_ROOMUSER_STATUS_CHARGE_MIC;
				pUserObjRef->ncuronmictype_ =0;

				//广播给房间内的所有用户,该用户下麦了( micstate变化了）
				{
					std::string strHead;
					std::string strAlias;
					CUserBasicInfo::getUserHeadAndAlias(nUserId, strHead, strAlias);

					char szOutBuf[1024];
					ClientGateMask_t* pClientGate;
					CMDUserMicState micInfo;
					micInfo.set_vcbid(nvcbid_);
					micInfo.mutable_runid()->set_userid(0);
					micInfo.mutable_toid()->set_userid(nUserId);
					micInfo.mutable_toid()->set_head(strHead);
					micInfo.mutable_toid()->set_alias(strAlias);
					micInfo.set_micindex(-1);
					micInfo.set_micstate(0);

					SERIALIZETOARRAY_BUF(micInfo,respbuf,len);
					int nMsgLen=m_pRoomMgr->Build_NetMsg(szOutBuf,1024,MDM_Vchat_Room,Sub_Vchat_SetMicStateNotify,&pClientGate,respbuf,len);
                    if(nMsgLen > 0)
						castSendMsgXXX((COM_MSG_HEADER*)szOutBuf);
				}
				LOG_PRINT(log_info, " room_tag checkWaitMicToPubMic() end.\n");
			}

            //增加到公麦
			nret = addPubMicUser(nUserId, -1, 0);
			if(nret >= 0)
			{
				//增加到公麦成功
				pUserObjRef->inroomstate_ |= FT_ROOMUSER_STATUS_PUBLIC_MIC;
				pUserObjRef->ncurpublicmicindex_ = nret;

				//计算该用户上公麦时间
				pUserObjRef->nstarttime_pubmic_=(int)time(0);
				pUserObjRef->nstarttime_mic_ = time(0);
				pUserObjRef->ncuronmictype_ =1;

				//广播给房间内的所有用户,该用户上了公麦( micstate变化了）
				if(bcast)
				{
					char szOutBuf[1024];
					ClientGateMask_t* pClientGate;
					CMDUserMicState micInfo;
					micInfo.set_vcbid(nvcbid_);

					std::string strHead;
					std::string strAlias;
					CUserBasicInfo::getUserHeadAndAlias(nUserId, strHead, strAlias);

					micInfo.mutable_runid()->set_userid(0);
					micInfo.mutable_toid()->set_userid(nUserId);
					micInfo.mutable_toid()->set_head(strHead);
					micInfo.mutable_toid()->set_alias(strAlias);
					micInfo.set_micindex(pUserObjRef->ncurpublicmicindex_);
					micInfo.set_micstate(FT_ROOMUSER_STATUS_PUBLIC_MIC);
					SERIALIZETOARRAY_BUF(micInfo,micbuf,miclen);
					int nMsgLen=m_pRoomMgr->Build_NetMsg(szOutBuf,1024,MDM_Vchat_Room,Sub_Vchat_SetMicStateNotify,&pClientGate,micbuf,miclen);
                    if(nMsgLen > 0)
					    castSendMsgXXX((COM_MSG_HEADER*)szOutBuf);
				}
				ret++;
				LOG_PRINT(log_info, " room_tag checkWaitMicToPubMic() end..%d\n",ret);
				if(once)
					break;
			}
			else
			{
				//增加到公麦失败，保存原姿势,停止循环(后面的肯定也成功不了）
				vecWaitMics_.insert(vecWaitMics_.begin(), tState);
				LOG_PRINT(log_info, "room_tag checkWaitMicToPubMic() failed..9\n");
				break;
			}
		}
		else
		{
			LOG_PRINT(log_info, " room_tag checkWaitMicToPubMic() continue..10\n");
			continue;  //继续下一个
		}
	}
	updateWaitUsers();
	return ret;
}
bool CRoomObj::delWaitMicUser(uint32 userId, int micId)
{
	LOG_PRINT(log_info, " room_tag delWaitMicUser() 10, user:%u\n", userId);
	int nret = -1;
	bool bonmic = false;
	PubMicState_t* p = getPubMicState();
	for(int i = 0 ; i < MAX_PUBMIC_COUNT ; i++ )
	{
		if(p[i].userid == userId)
		{
			if(p[i].onmic == 1)
			{
				castSendOnMicState(p[i].userid,0,i);
				bonmic = true;
				clearTimerid();
			}
			p[i].onmic = 0;
			p[i].userid = 0;
			nret = i;
			
			//update mic_count of mysql
			std::map<std::string, std::string> mValues;
			mValues[CFieldName::ROOM_MICCOUNT] = bitTostring(getOnMicCount());
			CDBSink().updateGroupBasicInfo_DB(nvcbid_, mValues);

			LOG_PRINT(log_info, "group:%u, user:%u, down mic success.", nvcbid_, userId);
			break;
		}
	}
	LOG_PRINT(log_info, " room_tag delWaitMicUser() useridindex:%d,bonmic:%d\n",nret,bonmic);
	do
	{
		if(m_micMode != e_QueueMode)
			break;
		if(nret >=0)
		{
			bool bfirst = false;
			for(int i = 0 ; i < MAX_PUBMIC_COUNT ; i++)
			{
				if(p[i].userid == 0 && p[i].status ==1)
				{
					for (int j = i ; j < MAX_PUBMIC_COUNT ; j++)
					{
						if(p[j].userid > 0 && p[j].onmic == 0 && p[j].status == 1)
						{
							p[i].userid = p[j].userid;
							p[i].onmic = p[j].onmic;
							p[i].status = p[j].status;

							p[j].userid = 0;
							p[j].onmic = 0;
							p[j].status = 1;
							if(!bfirst)
							{
								bfirst = true;
								if(bonmic)
								{
									p[i].onmic = 1;
									addTimerid();
									castSendOnMicState(p[i].userid,1,i,getInterval());
								}
							}
							break;
						}
					}
				}
			}
		}
	}while(0);
	updateMicState();
	return true;
}

void CRoomObj::setTeacherid(uint32 userid, int newMicState)
{
	if (newMicState == 0){
        LOG_PRINT(log_info, "[userid=%d, roomid=%u]down mic", userid, nvcbid_);
		nteacherid_ = 0;
		//nrobotid_ = 0;
		nmroomid_ = 0;
	}
	else if (newMicState == FT_ROOMUSER_STATUS_PUBLIC_MIC){
		// for sub room
		if (false == isNormalUser(userid)){
			getTeacheridFromRobotid(userid);
			nrobotid_ = userid;
			LOG_PRINT(log_info, "[userid=%d, teacherid=%u, roomid=%u]up mic", userid, nteacherid_, nvcbid_);
		}
		// for main room
		else{
			nteacherid_ = userid;
			nrobotid_ = 0;
			nmroomid_ = 0;
			LOG_PRINT(log_info, "[userid=%d, teacherid=%u, roomid=%u]up mic", userid, nteacherid_, nvcbid_);
		}
	}
}

void CRoomObj::getTeacheridFromRobotid(int robotid)
{
/*	uint32 teacherid = 0, mroomid = 0;
	CDBSink().get_teacherid_from_robotid(nvcbid_, robotid, teacherid, mroomid);
	if (teacherid != 0){
		nteacherid_ = teacherid;
		nmroomid_ = mroomid;
	}
	LOG_PRINT(log_info, "[robotid=%d, teacherid=%u, roomid=%u, mainroom=%u]", robotid, nteacherid_, nvcbid_, nmroomid_);
*/
}

void CRoomObj::updateActiveTime(bool bUpDb/* = true*/)
{
/*	m_tLastActive = time(0);

	if (bUpDb)
	{
		CDBSink().update_room_activetime(nvcbid_, m_tLastActive);
	}
*/
}

int CRoomObj::Build_NetMsg_JoinRoomRespPack(char * szBuf, int bufLen, UserObject_Ref & userobjRef, ClientGateMask_t ** ppGateMask, int biscollectroom)
{
	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)szBuf;
	pHead->version = MDM_Version_Value;
	pHead->checkcode = CHECKCODE;
	pHead->maincmd = MDM_Vchat_Room;
	pHead->subcmd = Sub_Vchat_JoinRoomResp;
	ClientGateMask_t * pClientGate = (ClientGateMask_t *)(pHead->content);
	memset(pClientGate, 0, sizeof(ClientGateMask_t));
	CMDJoinRoomResp pResp;
	pResp.set_userid(userobjRef->nuserid_);
	pResp.set_vcbid(nvcbid_);
	pResp.set_roomtype(nattribute_);
	pResp.set_busepwd(busepwd_);
	pResp.set_biscollectroom(biscollectroom);
	pResp.set_seats(capacity);
	pResp.set_groupid(0);
	pResp.set_runstate(talk_status);
	pResp.set_creatorid(ncreatorid_);	//房主
	pResp.set_op1id(nopuserid_[0]);		//副房主
	pResp.set_op2id(nopuserid_[1]);
	pResp.set_op3id(nopuserid_[2]);
	pResp.set_op4id(nopuserid_[3]);
	pResp.set_inroomstate(userobjRef->inroomstate_) ;
	pResp.set_nlotterypool(100);
	//pResp->nchestnum = userobjRef->nchestnum_;
	pResp.set_devtype(userobjRef->ndevtype); //设备类型
	pResp.set_cpwd(strpwd_.c_str());
	pResp.set_cname(name.c_str());
	pResp.set_npopularity_(audience_num);
	pResp.set_userroletype(userobjRef->ninroomlevel_);
	pResp.set_groupicon(icon);
	pResp.set_groupshowid(showid);
	pResp.set_muteswitch(1);

	uint32 ncollecttimes = 0;
	//if (0 == CDBSink().get_room_collect_num(nvcbid_, ncollecttimes))
	pResp.set_ncollect_times(ncollecttimes);

	char addr[64] = {0};
	//CDBSink().get_vcbmedia_addr_byareaid(nvcbid_, userobjRef->areaid.c_str(), addr, sizeof(addr));
	pResp.set_cmediaaddr(addr);
	LOG_PRINT(log_info, "media ip address: %s.", pResp.cmediaaddr().c_str());

	
	if (nvcbid_<1000000000)
	{
		pResp.set_liveurl(getRoomLiveUrl());//todo : get from redis
	}

	(*ppGateMask) = pClientGate;
	pHead->length = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + pResp.ByteSize();
	int length = pResp.ByteSize();
	char  buf[1024];
	pResp.SerializeToArray(buf, length);
	memcpy(pHead->content + SIZE_IVM_CLIENTGATE, buf, length);

	LOG_PRINT(log_info, "pResp.ByteSize(): %d.", length);
	return pHead->length;
}

int CRoomObj::Build_NetMsg_RoomUserComePack(char* szBuf, int bufLen, UserObject_Ref &userobjRef, ClientGateMask_t** ppGateMask)
{
	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)szBuf;
	pHead->version = MDM_Version_Value;
	pHead->checkcode = CHECKCODE;
	pHead->maincmd = MDM_Vchat_Room;
	pHead->subcmd = Sub_Vchat_RoomUserNoty;
    ClientGateMask_t * pClientGate = (ClientGateMask_t *)(pHead->content);
	memset(pClientGate, 0, sizeof(ClientGateMask_t));
	CMDRoomUserInfo info;
	setRoomUserInfo(info, userobjRef);
	int len = info.ByteSize();
	char buf[512] ;
	info.SerializeToArray(buf,len);
	memcpy(pHead->content + SIZE_IVM_CLIENTGATE,buf,len);

	(*ppGateMask) = pClientGate;
	pHead->length = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + len;
	return pHead->length;
}

int CRoomObj::Build_NetMsg_RoomUserLeftPack(char* szBuf, int bufLen, UserObject_Ref &userobjRef, ClientGateMask_t** ppGateMask)
{
	COM_MSG_HEADER* pHead = (COM_MSG_HEADER *)szBuf;
	pHead->version = MDM_Version_Value;
	pHead->checkcode = CHECKCODE;
	pHead->maincmd = MDM_Vchat_Room;
	pHead->subcmd = Sub_Vchat_RoomUserExitNoty;
	
	ClientGateMask_t * pClientGate = (ClientGateMask_t *)(pHead->content);
	(*ppGateMask) = pClientGate;
	memset(pClientGate, 0, sizeof(ClientGateMask_t));
	if (userobjRef.get())
	{
		pHead->reqid = userobjRef->reqid;
		CMDUserExitRoomInfo_ext Info;
		Info.set_userid(userobjRef->nuserid_);
		Info.set_vcbid(nvcbid_);
		Info.set_roletype(userobjRef->nuserlevel_);
		SERIALIZETOARRAY_GATE(Info,pHead);
	}
	else
		pHead->length = 0;

	printf("Build_NetMsg_RoomUserLeftPack,len:%d\n",pHead->length);
	return pHead->length;
}

int CRoomObj::Build_NetMsg_RoomUserExceptLeftPack(char* szBuf, int bufLen, UserObject_Ref &userobjRef, ClientGateMask_t** ppGateMask)
{
	COM_MSG_HEADER* pHead = (COM_MSG_HEADER *)szBuf;
	pHead->version = 10;
	pHead->checkcode = 0;
	pHead->maincmd = MDM_Vchat_Room;
	pHead->subcmd = Sub_Vchat_RoomUserExceptExitNoty;
	ClientGateMask_t* pClientGate=(ClientGateMask_t*)(pHead->content);
	memset(pClientGate,0,sizeof(ClientGateMask_t));
	(*ppGateMask)=pClientGate;
	if (userobjRef.get()){
		CMDUserExceptExitRoomInfo_ext_t* pInfo = (CMDUserExceptExitRoomInfo_ext_t *)(pHead->content + SIZE_IVM_CLIENTGATE);
		pInfo->userid = userobjRef->nuserid_;
		pInfo->vcbid = nvcbid_;
		char szTemp[64];
		sprintf(szTemp, "C:%s|M:%s", userobjRef->ipaddr_.c_str(), userobjRef->gateip.c_str());
		strcpy(pInfo->content, szTemp);
		pInfo->textlen = strlen(szTemp);
		pHead->length = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE+ sizeof(CMDUserExceptExitRoomInfo_ext_t) + pInfo->textlen;
	}
	else
		pHead->length = 0;

	return pHead->length;
}

int CRoomObj::Build_NetMsg_RoomKickoutUserPack(char * szBuf, int bufLen, uint32 srcId, UserObject_Ref &toUserobjRef, int reasionId, int minsId,ClientGateMask_t** ppGateMask)
{
	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)szBuf;
	pHead->version = MDM_Version_Value;
	pHead->checkcode = CHECKCODE;
	pHead->maincmd = MDM_Vchat_Room;
	pHead->subcmd = Sub_Vchat_RoomKickoutUserNoty;
	pHead->reqid = toUserobjRef->reqid;
	ClientGateMask_t * pClientGate = (ClientGateMask_t *)(pHead->content);
	memset(pClientGate, 0, sizeof(ClientGateMask_t));
	(*ppGateMask) = pClientGate;

	if (toUserobjRef.get()){
		CMDUserKickoutRoomInfo pResp;
		pResp.set_vcbid(nvcbid_);
		pResp.set_srcid(srcId);
		pResp.set_toid(toUserobjRef->nuserid_);
		pResp.set_resonid(reasionId);
		pResp.set_mins(minsId);
		SERIALIZETOARRAY_GATE(pResp,pHead);
	}
	else
		pHead->length = 0;

	return pHead->length;
}

int CRoomObj::Build_NetMsg_RoomBaseInfoPack(char * szBuf, int bufLen, ClientGateMask_t ** ppGateMask)
{
	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)szBuf;
	pHead->version = MDM_Version_Value;
	pHead->checkcode = CHECKCODE;
	pHead->maincmd = MDM_Vchat_Room;
	pHead->subcmd = Sub_Vchat_RoomInfoNotify;

	ClientGateMask_t * pClientGate = (ClientGateMask_t *)(pHead->content);
	memset(pClientGate, 0, sizeof(ClientGateMask_t));

	CMDRoomBaseInfo pInfo;
	pInfo.set_vcbid(nvcbid_);
	pInfo.set_seats(capacity);
	pInfo.set_creatorid(group_master);
	pInfo.set_opstate(nopstate_);
	pInfo.mutable_roomname()->set_data(name);
	pInfo.mutable_icon()->set_data(icon);
	pInfo.mutable_roomremark()->set_data(remark);
	pInfo.mutable_noticeinfo()->set_roomnotice(m_roomnotice.m_strRoomNotice);
	pInfo.mutable_noticeinfo()->set_noticeuserid(m_roomnotice.m_uBroadUserid);
	pInfo.mutable_noticeinfo()->set_noticetime(m_roomnotice.m_uBroadTime);

	if (SIZE_IVM_HEAD_TOTAL + pInfo.ByteSize() > bufLen)
	{
		pHead->length = 0;
		return 0;
	}
	else
	{
		SERIALIZETOARRAY_GATE(pInfo, pHead);
		(*ppGateMask) = pClientGate;
		return pHead->length;
	}
}

int CRoomObj::Build_NetMsg_RoomOPStatusPack(char* szBuf, int bufLen, ClientGateMask_t** ppGateMask)
{
	COM_MSG_HEADER* pHead = (COM_MSG_HEADER *)szBuf;
	pHead->version = MDM_Version_Value;
	pHead->checkcode = CHECKCODE;
	pHead->maincmd = MDM_Vchat_Room;
	pHead->subcmd = Sub_Vchat_RoomOPStatusNotify;
	ClientGateMask_t* pClientGate=(ClientGateMask_t*)(pHead->content);
	memset(pClientGate,0,sizeof(ClientGateMask_t));
	CMDRoomOpState_t* pInfo =(CMDRoomOpState_t *)(pHead->content+ SIZE_IVM_CLIENTGATE);
	pInfo->vcbid = nvcbid_;
	pInfo->opstate = nopstate_;

	(*ppGateMask)=pClientGate;
	pHead->length = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + sizeof(CMDRoomOpState_t);
	return pHead->length;
}

int CRoomObj::Build_NetMsg_RoomMediaPack(char* szBuf, int bufLen, ClientGateMask_t** ppGateMask)
{
   COM_MSG_HEADER* pHead = (COM_MSG_HEADER *)szBuf;
   pHead->version = MDM_Version_Value;
   pHead->checkcode = CHECKCODE;
   pHead->maincmd = MDM_Vchat_Room;
   pHead->subcmd = Sub_Vchat_RoomMediaNotify;
   ClientGateMask_t* pClientGate=(ClientGateMask_t*)(pHead->content);
   memset(pClientGate, 0, sizeof(ClientGateMask_t));
   CMDRoomMediaInfo_t* pInfo= (CMDRoomMediaInfo_t *)(pHead->content + SIZE_IVM_CLIENTGATE);
   strcpy(pInfo->caddr, strmedia_.c_str());
   pInfo->vcbid = nvcbid_;

   (*ppGateMask)=pClientGate;
   pHead->length = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + sizeof(CMDRoomMediaInfo_t);
   return pHead->length;
}

int CRoomObj::Build_NetMsg_AdKeyworkListPack(char* pOutBuf, char* pInBuf, int inLen, ClientGateMask_t** ppGateMask)
{
	int recnum = inLen/sizeof(CMDAdKeywordInfo_t);

	COM_MSG_HEADER* pHead = (COM_MSG_HEADER *)pOutBuf;
	pHead->version = MDM_Version_Value;
	pHead->checkcode = CHECKCODE;
	pHead->maincmd = MDM_Vchat_Room;
	pHead->subcmd = Sub_Vchat_AdKeyWordOperateNoty;
	ClientGateMask_t* pClientGate=(ClientGateMask_t*)(pHead->content);
	memset(pClientGate, 0, sizeof(ClientGateMask_t));

	CMDAdKeywordsNotify_t* pAdKeyNotify= (CMDAdKeywordsNotify_t *)(pHead->content + SIZE_IVM_CLIENTGATE);
	memcpy(pAdKeyNotify->keywod, pInBuf, inLen);
	pAdKeyNotify->num = recnum;

	(*ppGateMask)=pClientGate;
    pHead->length = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + sizeof(CMDAdKeywordsNotify_t) + sizeof(CMDAdKeywordInfo_t)*recnum;
	return pHead->length;
}

int CRoomObj::Build_NetMsg_TeacheridPack(char* szBuf, int bufLen, ClientGateMask_t** ppGateMask)
{
    COM_MSG_HEADER* pHead = (COM_MSG_HEADER *)szBuf;
    pHead->version = MDM_Version_Value;
    pHead->checkcode = CHECKCODE;
    pHead->maincmd = MDM_Vchat_Room;
    pHead->subcmd = Sub_Vchat_RoborTeacherIdNoty;
    ClientGateMask_t* pClientGate=(ClientGateMask_t*)(pHead->content);
    memset(pClientGate, 0, sizeof(ClientGateMask_t));
    uint *num = (uint*)(pHead->content + SIZE_IVM_CLIENTGATE);
    CMDRoborTeacherIdNoty_t* pInfo = (CMDRoborTeacherIdNoty_t *)(pHead->content + SIZE_IVM_CLIENTGATE + sizeof(uint));
    pInfo->vcbid = nvcbid_;

    if (m_pRoomMgr->isMainRoom(nvcbid_)){
    	LOG_PRINT(log_warning, "room:%u is main room, return", nvcbid_);
    	return -1;
    }
    *num = m_pRoomMgr->makeTeacherRobotPack(this, pInfo);
    if (0 == *num){
    	LOG_PRINT(log_warning, "room:%u has no main room, return", nvcbid_);
    	return -1;
    }
    (*ppGateMask)=pClientGate;
    pHead->length = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + sizeof(uint) + sizeof(CMDRoborTeacherIdNoty_t)*(*num);
    return pHead->length;
}

int CRoomObj::Build_NetMsg_RoomNoticeNotyPack(char* szBuf, int bufLen, int noticeid, int userid, string alias, ClientGateMask_t** ppGateMask)
{
	COM_MSG_HEADER* pHead = (COM_MSG_HEADER *)szBuf;
	pHead->version = MDM_Version_Value;
	pHead->checkcode = CHECKCODE;
	pHead->maincmd = MDM_Vchat_Room;
	pHead->subcmd = Sub_Vchat_RoomNoticeInfoNotify;
	ClientGateMask_t* pClientGate=(ClientGateMask_t*)(pHead->content);
	memset(pClientGate,0,sizeof(ClientGateMask_t));

	CMDRoomNoticeNoty_t* pInfo = (CMDRoomNoticeNoty_t *)(pHead->content+ SIZE_IVM_CLIENTGATE);
	memset(pInfo,0,sizeof(CMDRoomNoticeNoty_t));
	pInfo->vcbid = nvcbid_;
	pInfo->userid = userid;								//用户id
	memcpy(pInfo->useralias, alias.c_str(), NAMELEN);	//用户昵称
	pInfo->index = noticeid;
	pInfo->textlen = m_RoomNoticeArray[noticeid].m_strRoomNotice.length() + 1;
	memcpy(pInfo->content, m_RoomNoticeArray[noticeid].m_strRoomNotice.c_str(), pInfo->textlen);
	pInfo->content[pInfo->textlen -1] = '\0';

	(*ppGateMask)=pClientGate;
	pHead->length = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + sizeof(CMDRoomNoticeNoty_t) + pInfo->textlen;
	return pHead->length;
}

bool CRoomObj::IsNoneedkey(uint32 userid,byte viplevel,byte roomlevel)
{
	std::set<int>::iterator itor_level;
	std::set<int>::iterator itor_roomlevel;
	//vip等级
	itor_level = m_pRoomMgr->noroomkeylevelset.find(viplevel);
	if(itor_level != m_pRoomMgr->noroomkeylevelset.end())
		return true;
	//房间等级
	itor_roomlevel = m_pRoomMgr->noroomkeyRoomlevelset.find(roomlevel);
	if(itor_roomlevel != m_pRoomMgr->noroomkeyRoomlevelset.end())
		return true;
	return false;
}

int CRoomObj::IsForbidChatUser(int userId)
{
	int bForbidChat=0;
	std::map<uint32, UserObject_Ref >::iterator itor = forbidchatusers_.find(userId);
	if(itor != forbidchatusers_.end())
		bForbidChat=1;
	return bForbidChat;
}

int CRoomObj::updateUserForbidChat(uint32 userid)
{
	vector<string> result;
	char key[32];
	sprintf(key, "%s:%u", KEY_HASH_USER_INFO, userid);
	if (m_pRedis->redis_hmget(key, "forbidchat", result) < 0){
		LOG_PRINT(log_warning, "failed to read user:%u from redis", userid);
		return -1;
	}

	if (0 == result.size()){
		LOG_PRINT(log_warning, "not found user:%u in redis", userid);
		return -1;
	}

	UserObject_Ref userObjRef;
	if (!findVisitUser(userObjRef, userid)){
		LOG_PRINT(log_warning, "not found user:%u in room:%u", userid, nvcbid_);
		return -1;
	}

	if (userObjRef->bForbidChat_ == atoi(result[0].c_str()))
		return -1;

	userObjRef->bForbidChat_ = atoi(result[0].c_str());
	if(userObjRef->bForbidChat_ == 0) //解禁
	{
		std::map<uint32, UserObject_Ref >::iterator itor = forbidchatusers_.find(userid);
		if(itor != forbidchatusers_.end())
			forbidchatusers_.erase(itor);
	}
	else
	{
		std::map<uint32, UserObject_Ref >::iterator itor = forbidchatusers_.find(userid);
		if(itor == forbidchatusers_.end())
		{
			forbidchatusers_.insert(std::make_pair(userid, userObjRef));
		}
	}
	return 0;
}

bool CRoomObj::getAllVisitorid(vector<string> &userids)
{
	char key[32] = {0};
	sprintf(key, "%s:%u", KEY_SET_ROOM_USERIDS, nvcbid_);
	if (!m_pRedis) {
		LOG_PRINT(log_error, "[getAllVisitorid]pRedis is NULL");
		return false;
	}
	if (m_pRedis->redis_smembers(key, userids) < 0) {
		LOG_PRINT(log_error, "Failed to get user ids from room:%u", nvcbid_);
		return false;
	}
	return true;
}

unsigned int CRoomObj::getVisitorNum()
{
	char key[32] = {0};
	sprintf(key, "%s:%u", KEY_SET_ROOM_USERIDS, nvcbid_);

	if (!m_pRedis) {
		LOG_PRINT(log_error, "[getVisitorNum]pRedis is NULL");
		return 0;
	}

	int ret = m_pRedis->redis_scard(std::string(key));
	if (ret < 0) {
		LOG_PRINT(log_error, "Failed to getVisitorNum from room:%u", nvcbid_);
		return 0;
	}

	LOG_PRINT(log_debug, "key:%s,VisitorNum=%d", key,ret);
	audience_num = ret;
	return ret;
}

void CRoomObj::newRobotUser(AndroidParam_t &param)
{
	CUserObj* p= new CUserObj;
	if (!p){
		LOG_PRINT(log_info, "Failed to allocate memory");
		return;
	}
	UserObject_Ref pUserObjRef(p);

	pUserObjRef->nuserid_ = param.userid;
	pUserObjRef->ngroupid = param.vcbid;
	pUserObjRef->nsvrid = m_svrid;
	pUserObjRef->ngateid= 0;
	pUserObjRef->ngender_  =param.ngender;
	pUserObjRef->nage_     =0;
	pUserObjRef->nuserlevel_      = param.viplevel;
	pUserObjRef->ncuronmictype_     =0;
	pUserObjRef->ncurpublicmicindex_=-1;
	pUserObjRef->nstarttime_pubmic_ =0;
	pUserObjRef->nwillendtime_pubmic_=0;
	pUserObjRef->inroomstate_ =0;
	pUserObjRef->ninroomlevel_ =param.roomlevel;
	pUserObjRef->cometime_ =time(0);
	pUserObjRef->nstarflag_ = 0;
	pUserObjRef->nactivityflag_ =0;
	pUserObjRef->type_ = USERTYPE_NPC;
	pUserObjRef->pConnId_ = 0; //
	pUserObjRef->pGateObj_ = 0;   //
	pUserObjRef->pGateObjId_ = 0; //
	pUserObjRef->calias_ = param.szuseralias;
	pUserObjRef->headpic_ = "";
	pUserObjRef->macserial_ = "";
	pUserObjRef->bForbidInviteUpMic_ = 0;  //
	pUserObjRef->bForbidChat_ = 0;
	pUserObjRef->ipaddr_= "";

	char szKey[32] = {0};
	sprintf(szKey, "%s:%u", KEY_HASH_USER_INFO, pUserObjRef->nuserid_);
	if (1 == m_pRedis->redis_existkey(szKey)) {
		return;
	}

	if (addVisitorUser(pUserObjRef) < 0) {
		LOG_PRINT(log_error, "[userid=%u,roomid=%u] failed to add user", pUserObjRef->nuserid_, nvcbid_);
		return;
	}
}

int CRoomObj::loadRoomBroadInfo()
{
	//加载房间公告broad
	if (CDBSink().getGroupBroadInfo_DB(nvcbid_, m_roomnotice))
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int CRoomObj::update_zhuanbo_robot_DB(uint32 vcbid, int micstate, UserObject_Ref micuser)
{
	char query[512];

	CDBSink db;
	/* 转播员上麦 */
	if(USERTYPE_NPC == micuser->type_) {
		/*上麦*/
		if (micstate) {
			uint32 teacherid = 0;
			uint32 mainvcbid = m_pRoomMgr->getMainRoom(micuser->nuserid_);
			RoomObject_Ref mainRoomRef = m_pRoomMgr->FindRoom(mainvcbid);
			if (!mainRoomRef) {
				LOG_PRINT(log_error, "[subvcb:%u,micuser:%u]setmicstate: not found main vcb", vcbid, micuser->nuserid_);
				return -1;
			}
			teacherid = mainRoomRef->getUserIsOnMic();
			if (0 == teacherid) {
				LOG_PRINT(log_error, "[mainvcb:%u,subvcb:%u,micuser:%u]setmicstate: no teacher on mic", mainvcbid, vcbid, micuser->nuserid_);
				return -1;
			}
			sprintf(query, "update dks_vcbzhuanboconfig set nuserid=%u where nrobotid=%u", teacherid, micuser->nuserid_);
		}
		/*下麦*/
		else {
			sprintf(query, "update dks_vcbzhuanboconfig set nuserid=0 where nrobotid=%u", micuser->nuserid_);
		}
	}
	/*非转播员(正常用户)上麦*/
	else if(USERTYPE_NORMALUSER == micuser->type_) {
		if (micstate)
			sprintf(query, "update dks_vcbzhuanboconfig set nuserid=%d where nvcbid=%d and nvcbid=nrobotvcbid", micuser->nuserid_, vcbid);
		else
			sprintf(query, "update dks_vcbzhuanboconfig set nuserid=0 where nvcbid=%d and nvcbid=nrobotvcbid", vcbid);
	}

	LOG_PRINT(log_info, "[vcbid:%u,micstate:%d,touserid:%u] setmicstate sql: %s", vcbid, micstate, micuser->nuserid_, query);
	if (!db.build_sql_run(NULL, NULL, query)) {
		LOG_PRINT(log_error, "setmicstate, Failed to exec sql:%s", query);
		return -1;
	}

	return 0;
}

int CRoomObj::update_zhuanbo_robot_DB(uint32 vcbid, int micstate, uint32 micuserid)
{
	UserObject_Ref micuserRef;
	if (!findVisitUser(micuserRef, micuserid)) {
		LOG_PRINT(log_error, "not found user:%u", micuserid);
		return -1;
	}

	return update_zhuanbo_robot_DB(vcbid, micstate, micuserRef);
}

void CRoomObj::UpdateVcbStatusDB()
{
	char szQuery[512];
	sprintf(szQuery,"update dks_vcbstatus a join dks_vcbinformation b on a.nvcbid=b.nvcbid set a.nstatus=2,a.ncount=%u,a.nroborcount=%u,a.nsoftbotcount=%u,"
			"a.filtercount=%u,a.guestfiltercount=%u,a.nipcount=%u,a.nguestcount=%u,a.onmic_userid=%u \
			where a.nstatus=2 and a.nvcbid=%u and b.livetype=1",
      ntotalcount_,
	  nrobotcount_,
	  nsoftbotcount_,
	  nfilterusercount_,
	  nfilterguestcount_,
	  nipcount_,
	  nguestcount_,
	  nuseridonmic_,
	  nvcbid_);
	CDBSink db;
	db.build_sql_run(0, 0, szQuery);
}

int CRoomObj::redisSetRoomid()
{
	if (!m_pRedis)
		return -1;

	string key = KEY_SET_ROOMIDS;
	stringstream ss;
	ss << nvcbid_;
	string sroomid = ss.str();

	int ret = m_pRedis->redis_sadd(key, sroomid);
	if (ret < 0) {
		LOG_PRINT(log_error, "Failed sadd %s to %s", sroomid.c_str(), key.c_str());
	}
	return ret;
}

int CRoomObj::redisInitRoomInfo()
{
	char key[32];
	sprintf(key, "%s:%u", KEY_HASH_ROOM_INFO, nvcbid_);
	string skey = key;
	if (!m_pRedis) {
		LOG_PRINT(log_error, "pRedis is NULL");
		return -1;
	}

// 	int ret = m_pRedis->redis_existkey(key);
// 	if (ret < 0) {
// 		LOG_PRINT(log_error, "Failed to run EXISTS cmd");
// 		return -1;
// 	}
// 	if (1 == ret) {
// 		LOG_PRINT(log_error, "key:%s exists, return", key);
// 		return -1;
// 	}

	string fields, values;
	vector<string> vecfield;
	vector<string> vecvalues;
	Json::Value body;
	redisMsg_ObjectUpdate(skey, fields, body, this);//根据配置的field，将成员值以key=value放入body
	JsonToVec(body, vecfield, vecvalues);////根据body过滤一次vecvalues
	if (redisSetRoomid() < 0)
		return -1;

	return m_pRedis->redis_hmset(skey, vecfield, vecvalues);
}

int CRoomObj::redisSetRoomInfo(const char *pfields)
{
	char key[32];
	vector<string> vecvalues;
	sprintf(key, "%s:%u", KEY_HASH_ROOM_INFO, nvcbid_);
	string skey = key;

	if (!m_pRedis) {
		LOG_PRINT(log_error, "pRedis is NULL");
		return -1;
	}

	int ret = m_pRedis->redis_existkey(key);
	if (ret < 0) {
		LOG_PRINT(log_error, "Failed to run EXISTS cmd");
		return -1;
	}
	if (0 == ret) {
		LOG_PRINT(log_error, "key:%s does not exists, return");
		return -1;
	}

	string fields, values;
	if (pfields)
		fields = pfields;
	vector<string> vecfield;
	Json::Value body;
	redisMsg_ObjectUpdate(skey, fields, body, this);
	JsonToVec(body, vecfield, vecvalues);

	return m_pRedis->redis_hmset(skey, vecfield, vecvalues);
}

void CRoomObj::close()
{
	if(ncloseflag_ == 1) {
		LOG_PRINT(log_info, "room[%d] is closed!",nvcbid_);
		return;
	}
	ncloseflag_ = 1;
	LOG_PRINT(log_info, "room[%d] is closing!",nvcbid_);

	//清除数据,通知所有客户端,房间被关闭;关闭所有客户端连接connetion;删除所有用户对象
	if(ntotalcount_ > 0)
	{
		char szBuf[512];
		ClientGateMask_t* pClientGate;
		CMDCloseRoomNoty_t closeInfo;
		memset(&closeInfo,0,sizeof(CMDCloseRoomNoty_t));
		closeInfo.vcbid=this->nvcbid_;
		sprintf(closeInfo.closereason, "系统维护,房间被关闭！");
		int nMsgLen= m_pRoomMgr->Build_NetMsg(szBuf, 512, MDM_Vchat_Room, Sub_Vchat_CloseRoomNotify, &pClientGate, &closeInfo,
				sizeof(CMDCloseRoomNoty_t));
		if(nMsgLen > 0) {
		   castSendMsgXXX((COM_MSG_HEADER*)szBuf,0,0);
		   redisMsg_RoomMgrInfoUpdate("closeflag");
		}
	}
	//vecWaitMics_.clear();
	//lstA4Users_.clear();
	forbidchatusers_.clear();
}

void CRoomObj::castSendRoomAndSubRoomId()
{
	char szBuf[1024]={0};
	COM_MSG_HEADER* pHead=(COM_MSG_HEADER*)szBuf;
	pHead->version=MDM_Version_Value;
	pHead->checkcode=CHECKCODE;
	pHead->maincmd=MDM_Vchat_Room;
	pHead->subcmd=Sub_Vchat_RoomAndSubRoomId_Noty;
	ClientGateMask_t* pClientGate=(ClientGateMask_t*)(pHead->content);
	memset(pClientGate,0,sizeof(ClientGateMask_t));
	CMDRoomAndSubRoomIdNoty_t* pInfo=(CMDRoomAndSubRoomIdNoty_t*)(pHead->content + SIZE_IVM_CLIENTGATE);

	//讲师id为0则放弃
	if(0 == nteacherid_)
	{
		LOG_PRINT(log_info, "nteacherid:%u, room:%u", nteacherid_, nvcbid_);
		return;
	}

	if(0 == nmroomid_)
	{
		pInfo->roomid = nvcbid_;
		pInfo->subroomid = 0;
	}
	else
	{
		pInfo->roomid = nmroomid_;
		pInfo->subroomid = nvcbid_;
	}
	LOG_PRINT(log_info, "pInfo->roomid:%u, pInfo->subroomid:%u, nteacherid:%u", pInfo->roomid, pInfo->subroomid, nteacherid_);
	
	pHead->length = SIZE_IVM_HEADER + SIZE_IVM_CLIENTGATE + sizeof(CMDRoomAndSubRoomIdNoty_t);
	castSendMsgXXX((COM_MSG_HEADER*)szBuf);
}

bool CRoomObj::isClosed()
{
	return (ncloseflag_==1) ? true : false;
}

bool CRoomObj::isNormalUser(uint32 userid, bool block)
{
	UserObject_Ref userRef;
	findVisitUser(userRef, userid);

	return userRef.get() && userRef->isNormalUser();
}

bool CRoomObj::isZhuanboUser(uint32 userid)
{
	UserObject_Ref userRef;
	findVisitUser(userRef, userid);

	return userRef.get() && userRef->isZhuanboUser();
}

void CRoomObj::sendOnMicMediaMGR(int micstate, UserObject_Ref pToUserRef)
{
	if (USERTYPE_NPC != pToUserRef->type_)
		return ;

	char buf[512] = {0};
	int len = SIZE_IVM_HEADER + sizeof(CMDOnMicNotifyMedia_t) + sizeof(CMDSubInfo_t);
	char *p = CMsgComm::Build_COM_MSG_HEADER(buf, MDM_Vchat_MediaMGR, Sub_Vchat_NoticeRoomOnMic, len);
	CMDOnMicNotifyMedia_t *precord = (CMDOnMicNotifyMedia_t*)p;
	precord->type = micstate;
	precord->userid = pToUserRef->nuserid_;
	precord->num = 1;
	precord->roomid = nvcbid_;
	CMDSubInfo_t *psub = (CMDSubInfo_t *)precord->content;

	psub->subroomid = nvcbid_;
	psub->robotid = pToUserRef->nuserid_;
	precord->roomid = m_pRoomMgr->getMainRoom(psub->robotid);
	RoomObject_Ref mainRoomObjRef = m_pRoomMgr->FindRoom(precord->roomid);
	if (mainRoomObjRef)
		precord->userid = mainRoomObjRef->getUserIsOnMic();

	LOG_PRINT(log_info, "micstate:%u,mainroom:%u,subroom:%u,robotid:%u", micstate, precord->roomid, psub->subroomid, psub->robotid);

	//TODO;
}

uint32 CRoomObj::getSoftbotCountPerViplevel(uint8 viplevel)
{
	map<uint8, uint32>::iterator it = map_softbotvipcount_.find(viplevel);
	if (it == map_softbotvipcount_.end())
		return 0;
	else
		return it->second;
}

void CRoomObj::addoneSoftbotCountPerViplevel(uint8 viplevel)
{
	if (0 == viplevel)
		return;

	map<uint8, uint32>::iterator it = map_softbotvipcount_.find(viplevel);
	if (it == map_softbotvipcount_.end())
		map_softbotvipcount_[viplevel] = 1;
	else
		it->second++;
}
void CRoomObj::suboneSoftbotCountPerViplevel(uint8 viplevel)
{
	if (0 == viplevel)
		return;

	map<uint8, uint32>::iterator it = map_softbotvipcount_.find(viplevel);
	if (it == map_softbotvipcount_.end())
		map_softbotvipcount_[viplevel] = 0;
	else if (it->second > 0)
		it->second--;
	else
		it->second = 0;
}

uint32 CRoomObj::getUserCountPerViplevel(uint8 viplevel)
{
	map<uint8, uint32>::iterator it = map_uservipcount_.find(viplevel);
	if (it == map_uservipcount_.end())
		return 0;
	else
		return it->second;
}
void CRoomObj::addoneUserCountPerViplevel(uint8 viplevel)
{
	if (0 == viplevel)
		return;

	map<uint8, uint32>::iterator it = map_uservipcount_.find(viplevel);
	if (it == map_uservipcount_.end())
		map_uservipcount_[viplevel] = 1;
	else
		it->second++;
}
void CRoomObj::suboneUserCountPerViplevel(uint8 viplevel)
{
	if (0 == viplevel)
		return;

	map<uint8, uint32>::iterator it = map_uservipcount_.find(viplevel);
	if (it == map_uservipcount_.end())
		map_uservipcount_[viplevel] = 0;
	else if (it->second > 0)
		it->second--;
	else
		it->second = 0;
}

bool CRoomObj::redisMsg_RoomMgrInfoUpdate(const string fields/* = ""*/)
{
	std::map<std::string, std::string> mValues;
	getRedisValuesMap(mValues, fields);
	return m_rmGroupInfo.insert(nvcbid_, mValues);
}

void CRoomObj::castSendRoomWaitMicList(void)
{
	char szBuf[4096];
	ClientGateMask_t* pClientGate;
	Build_NetMsg_WaitMicListPack(szBuf, 4096,&pClientGate);
    castSendMsgXXX((COM_MSG_HEADER*)szBuf);
}

void CRoomObj::castSendRoomVistorCount(void)
{
	CMDQryGroupVisitCountResp oRspData;
	oRspData.set_groupid(nvcbid_);
	oRspData.set_visitcount(audience_num + virtual_num);
	LOG_PRINT(log_debug, "vcbid; %u, audience_num[%u] + virtual_num[%u] = %u",
		oRspData.groupid(), audience_num, virtual_num, oRspData.visitcount());	

	unsigned int rspDataLen = SIZE_IVM_HEAD_TOTAL + oRspData.ByteSize();
	SL_ByteBuffer buff(rspDataLen);
	buff.data_end(rspDataLen);

	CMsgComm::Build_COM_MSG_HEADER(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_QryGroupVisitCountResp, rspDataLen);
	oRspData.SerializeToArray(buff.buffer() + SIZE_IVM_HEAD_TOTAL, oRspData.ByteSize());
	castSendMsgXXX((COM_MSG_HEADER *)buff.buffer());
}

void CRoomObj::castSendUserPriorityChange(unsigned int userid, unsigned int roleType)
{
	//broadcast user roletype change.
	CMDUserInfoModNotify oNoty;
	oNoty.set_userid(userid);
	CMDUInt32 * pRoleType = oNoty.mutable_roletype();
	pRoleType->set_data(roleType);
	std::string strHead = "";
	std::string strAlias = "";
	CUserBasicInfo::getUserHeadAndAlias(userid, strHead, strAlias);
	oNoty.mutable_headadd()->set_data(strHead);
	oNoty.mutable_useralias()->set_data(strAlias);

	CMDGroupInfo * groupInfo = oNoty.mutable_groupinfo();
	groupInfo->set_groupid(nvcbid_);
	groupInfo->set_grouphead(icon);
	groupInfo->set_groupname(name);
	groupInfo->set_showid(showid);
	groupInfo->set_roletype(roleType);
	unsigned int user_num = CUserGroupinfo::getGroupMemberNum(nvcbid_);
	unsigned int visit_num = getVisitorNum();
	groupInfo->set_membercount(user_num);
	groupInfo->set_visitcount(visit_num);
	bool bMute = CDBSink().getGroupMsgMute(userid, nvcbid_);
	groupInfo->set_muteset((int)bMute);

	unsigned int notyLen = SIZE_IVM_HEAD_TOTAL + oNoty.ByteSize();
	SL_ByteBuffer oNotybuff(notyLen);
	oNotybuff.data_end(notyLen);

	COM_MSG_HEADER * pNotyHead = (COM_MSG_HEADER *)oNotybuff.buffer();
	pNotyHead->version = MDM_Version_Value;
	pNotyHead->checkcode = CHECKCODE;
	pNotyHead->maincmd = MDM_Vchat_Room;
	pNotyHead->subcmd = Sub_Vchat_SetUserPriorityNotify;
	pNotyHead->length = notyLen;

	char * pNoty = (char *)(pNotyHead->content + SIZE_IVM_CLIENTGATE);
	oNoty.SerializeToArray(pNoty, oNoty.ByteSize());

	ClientGateMask_t * pNotyGate = (ClientGateMask_t *)(pNotyHead->content);
	CMsgComm::Build_BroadCastRoomGate(pNotyGate, e_Notice_AllType, nvcbid_);
	if (m_pRoomMgr)
	{
		LOG_PRINT(log_debug, "[cast send user priority change]notify groupid:%u,icon:%s,groupName:%s,showid:%u,userid:%u,roleType:%u,msg mute:%d,visit_num:%u,user_num:%u.", \
			nvcbid_, icon.c_str(), name.c_str(), showid, userid, roleType, (int)bMute, visit_num, user_num);
		m_pRoomMgr->castGateway(pNotyHead);
	}
}

int CRoomObj::Build_NetMsg_WaitMicListPack(char* szBuf, int bufLen,ClientGateMask_t** ppGateMask)
{
	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)szBuf;
	pHead->version = MDM_Version_Value;
	pHead->checkcode = CHECKCODE;
	pHead->maincmd = MDM_Vchat_Room;
	pHead->subcmd = Sub_Vchat_WaitiMicListInfo;
	ClientGateMask_t * pClientGate = (ClientGateMask_t *)(pHead->content);
	memset(pClientGate, 0, sizeof(ClientGateMask_t));
	char * pData = (char *)(pHead->content + SIZE_IVM_CLIENTGATE);

	CMDUpWaitMicList Resp;
	Resp.set_vcbid(nvcbid_);
	Resp.set_size(vecWaitMics_.size());

	int nlength = vecWaitMics_.size();
	for(int i = 0; i < nlength; ++i)
	{
		unsigned int userid = vecWaitMics_[i].userid;
		std::string strHead;
		std::string strAlias;
		CUserBasicInfo::getUserHeadAndAlias(userid, strHead, strAlias);
		UserInfo_t * pUserInfo = Resp.add_userid();
		pUserInfo->set_userid(userid);
		pUserInfo->set_head(strHead);
		pUserInfo->set_alias(strAlias);
	}

	unsigned int packetLen = SIZE_IVM_HEAD_TOTAL + Resp.ByteSize();
	if (packetLen > bufLen)
	{
		pHead->length = 0;
		return 0;
	}
	else
	{
		(*ppGateMask) = pClientGate;
		pHead->length = packetLen;
		Resp.SerializeToArray(pData, Resp.ByteSize());
		return pHead->length;
	}
}

e_MicMode CRoomObj::getMicMode()
{
	if (!m_pRedis) {
		LOG_PRINT(log_error, "[vcbid:%u] redisOpt is NULL", nvcbid_);
		return m_micMode;
	}
	char szKey[32];
	sprintf(szKey, "%s:%u", KEY_HASH_ROOM_INFO, nvcbid_);
	string val;
	if (m_pRedis->redis_hget(szKey, "micmode", val) < 0) {
		LOG_PRINT(log_error, "[vcbid:%u] Failed to get the user on mic", nvcbid_);
		return m_micMode;
	}

	return e_MicMode(atoi(val.c_str()));
}
std::vector<WaitMicState_t > CRoomObj::getWaitMics()
{
	if (!m_pRedis) {
		LOG_PRINT(log_error, "[vcbid:%u] redisOpt is NULL", nvcbid_);
		return vecWaitMics_;
	}
	char szKey[32];
	sprintf(szKey, "%s:%u", KEY_HASH_ROOM_INFO, nvcbid_);
	vector<string> vals;
	if (m_pRedis->redis_hmget(szKey, "waitusers", vals) < 0) {
		LOG_PRINT(log_error, "[vcbid:%u] Failed to get the user on mic", nvcbid_);
		return vecWaitMics_;
	}
	vecWaitMics_.clear();

	for(int i  = 0 ; i < vals.size() ; i++)
	{
		string value = vals[i].c_str();
		Json::Reader reader(Json::Features::strictMode());
		Json::Value root;
		if (reader.parse(value, root)) {
			if (root.isArray())
			{
				for(int j = 0 ; j < root.size() ; j++)
				{
					WaitMicState_t w;
					w.userid = root[j].asInt();
					vecWaitMics_.push_back(w);
				}
			}
		}
	}
	return vecWaitMics_;
}

PubMicState_t* CRoomObj::getPubMicState()//micindex micuserid mictimetype
{
	if (!m_pRedis) {
		LOG_PRINT(log_error, "room_tag[vcbid:%u] redisOpt is NULL", nvcbid_);
		return pubmicstate_;
	}
	char szKey[32];
	sprintf(szKey, "%s:%u", KEY_HASH_ROOM_INFO, nvcbid_);
	vector<string> vals;
	if (m_pRedis->redis_hmget(szKey, "micindex", vals) < 0) {
		LOG_PRINT(log_error, "room_tag[vcbid:%u] Failed to get the user on mic", nvcbid_);
		return pubmicstate_;
	}
	for ( int i  = 0 ; i < vals.size(); i++)
	{
		string value = vals[i].c_str();
		Json::Reader reader(Json::Features::strictMode());
		Json::Value root;
		if (reader.parse(value, root)) {
			if (root.isArray())
			{
				for(int j = 0 ; j < root.size() ; j++)
				{
					m_vMicInfo[j].micindex = root[j].asInt();
				}
			}
		}
	}
	vals.clear();
	if (m_pRedis->redis_hmget(szKey, "micuserid", vals) < 0) {
		LOG_PRINT(log_error, "room_tag[vcbid:%u] Failed to get the user on mic", nvcbid_);
		return pubmicstate_;
	}
	for ( int i  = 0 ; i < vals.size(); i++)
	{
		string value = vals[i].c_str();
		Json::Reader reader(Json::Features::strictMode());
		Json::Value root;
		if (reader.parse(value, root)) {
			if (root.isArray())
			{
				for(int j = 0 ; j < root.size() ; j++)
				{
					m_vMicInfo[j].userid = root[j].asInt();
				}
			}
		}
	}
	vals.clear();
	if (m_pRedis->redis_hmget(szKey, "mictimetype", vals) < 0) {
		LOG_PRINT(log_error, "room_tag[vcbid:%u] Failed to get the user on mic", nvcbid_);
		return pubmicstate_;
	}
	LOG_PRINT(log_info, "proc_micstatelist_req mictimetype fileds size :%d", vals.size());
	for ( int i  = 0 ; i < vals.size(); i++)
	{
		string value = vals[i].c_str();
		Json::Reader reader(Json::Features::strictMode());
		Json::Value root;
		if (reader.parse(value, root)) {
			if (root.isArray())
			{
				for(int j = 0 ; j < root.size() ; j++)
				{
					m_vMicInfo[j].mictype = root[j].asInt();
				}
			}
		}
	}

	vals.clear();
	if (m_pRedis->redis_hmget(szKey, "micstatus", vals) < 0) {
		LOG_PRINT(log_error, "room_tag[vcbid:%u] Failed to get the user on mic", nvcbid_);
		return pubmicstate_;
	}
	for ( int i  = 0 ; i < vals.size(); i++)
	{
		string value = vals[i].c_str();
		Json::Reader reader(Json::Features::strictMode());
		Json::Value root;
		if (reader.parse(value, root)) {
			if (root.isArray())
			{
				for(int j = 0 ; j < root.size() ; j++)
				{
					m_vMicInfo[j].status = root[j].asInt();
				}
			}
		}
	}

	for(int i = 0 ; i < MAX_PUBMIC_COUNT ;i++)
	{
		pubmicstate_[m_vMicInfo[i].micindex].userid = m_vMicInfo[i].userid;
		pubmicstate_[m_vMicInfo[i].micindex].mictimetype = m_vMicInfo[i].mictype;
		pubmicstate_[m_vMicInfo[i].micindex].status = m_vMicInfo[i].status;
	}
	return pubmicstate_;
}

void CRoomObj::updateMicState()
{
	if (!updateRedisInfo(CFieldName::ROOM_MIC_LIST_FIELDS))
	{
		LOG_PRINT(log_error, "group:%u updateMicState failed.", nvcbid_)
	}
}

void CRoomObj::updateFields(string& key,string& fields)
{
	int ret = m_pRedis->redis_existkey(key.c_str());
	if (ret < 0) {
		LOG_PRINT(log_error, "Failed to run EXISTS cmd");
		return ;
	}
	if(ret == 0)
	{
		LOG_PRINT(log_error, "key not exists");
		return;
	}
	string values;
	vector<string> vecfield;
	vector<string> vecvalues;
	Json::Value body;
	redisMsg_ObjectUpdate(key, fields, body, this);
	JsonToVec(body, vecfield, vecvalues);

	m_pRedis->redis_hmset(key, vecfield, vecvalues);
}

void CRoomObj::updateWaitUsers()
{
	char key[32];
	sprintf(key, "%s:%u", KEY_HASH_ROOM_INFO, nvcbid_);
	string skey = key;
	if (!m_pRedis) {
		LOG_PRINT(log_error, "pRedis is NULL");
		return ;
	}

	int ret = m_pRedis->redis_existkey(key);
	if (ret < 0) {
		LOG_PRINT(log_error, "Failed to run EXISTS cmd");
		return ;
	}
	if(ret == 0)
	{
		LOG_PRINT(log_error, "key not exists");
		return;
	}
//	ret = m_pRedis->redis_hdel(key,"waitusers");
//	if (ret < 0) {
//		LOG_PRINT(log_error, "Failed to run EXISTS cmd");
//		return;
//	}
	string fields, values;
	fields = "waitusers";
	vector<string> vecfield;
	vector<string> vecvalues;
	Json::Value body;
	redisMsg_ObjectUpdate(skey, fields, body, this);
	JsonToVec(body, vecfield, vecvalues);

	m_pRedis->redis_hmset(skey, vecfield, vecvalues);
}

bool CRoomObj::changeWaitMicIndex_new(uint32 userId,int micId, int optype)
{
	if(vecWaitMics_.size() <=1) return false;     //麦序数组有东西
	if(userId <= 0) return false;    //用户id
	if(micId <0 || micId >= vecWaitMics_.size()) return false; //修改的麦序在麦序数组之间

	if(optype == 1)
	{
		//往上
		if((micId > 0) && (micId < vecWaitMics_.size()) ) {
			WaitMicState_t tmpVal = vecWaitMics_[micId-1];
			vecWaitMics_[micId-1] = vecWaitMics_[micId];
			vecWaitMics_[micId] = tmpVal;
		}
		else {
			return false;
		}
	}
	else if(optype == 2)
	{
		//往下
		if((micId < vecWaitMics_.size() -1)  && (micId >=0) ) {
			WaitMicState_t tmpVal = vecWaitMics_[micId+1];
			vecWaitMics_[micId+1] = vecWaitMics_[micId];
			vecWaitMics_[micId] = tmpVal;
		}
		else {
			return false;
		}
	}
	else if(optype == 3)
	{
		//置顶
		if((micId >0) && (micId < vecWaitMics_.size())) {
			WaitMicState_t tmpVal = vecWaitMics_[micId];
			vecWaitMics_.erase(vecWaitMics_.begin() + micId);
			vecWaitMics_.insert(vecWaitMics_.begin(), tmpVal);
		}
		else {
			return false;
		}

	}
	else if(optype == 4)
	{
		//置低
		if((micId)>=0 && (micId < vecWaitMics_.size() -1)) {
			WaitMicState_t tmpVal = vecWaitMics_[micId];
			vecWaitMics_.erase(vecWaitMics_.begin() + micId);
			vecWaitMics_.push_back(tmpVal);
		}
		else {
			return false;
		}
	}
	else
		return false;

	return true;
}

int CRoomObj::delAllWaitMicUser()
{
	printf("delWaitMicUser(), 删除了所有排麦用户.\n");
	int ret = 0;
	if(vecWaitMics_.size() >0)
	{
		ret = vecWaitMics_.size();
		vecWaitMics_.clear();
	}
	return ret;
}
bool CRoomObj::delWaitMicUser_new(uint32 userId,int micId)
{
	bool bDeleted =false;
	if(vecWaitMics_.size() == 0)  return false;
	printf("delWaitMicUser_new(), 删除了排麦用户,删除类型=%d,删除id=%d.\n",micId, userId);

	std::vector<WaitMicState_t>::iterator iter;
	//全部删除该用户的麦序
	if(micId == -1)
	{
		for(iter = vecWaitMics_.begin(); iter !=vecWaitMics_.end();)
		{
			if((*iter).userid == userId) {
				iter = vecWaitMics_.erase(iter);
				bDeleted = true;
			}
			else
				iter++;
		}
	}

	//删除该用户的最后一个
	else if(micId == -2)
	{
		int i, nIndex = -1;
		for(i=0; i<vecWaitMics_.size(); i++)
		{
			if(vecWaitMics_[i].userid == userId)
				nIndex = i;
		}
		if(nIndex >0) {
			vecWaitMics_.erase(vecWaitMics_.begin()+i);
			bDeleted = true;
		}
	}

	//删除指定的index
	else if(micId >= 0)
	{
		//防止数组越界
		if(micId >= vecWaitMics_.size())
			micId = vecWaitMics_.size() -1;
		vecWaitMics_.erase(vecWaitMics_.begin() + micId);
		bDeleted = true;
	}
	updateWaitUsers();
	return bDeleted;
}

void CRoomObj::getGroupMemLst(std::list<CMDRoomUserInfo> & memberLst)
{
	memberLst.clear();

	std::map<uint32, StUserBaseAndGroupInfo> user_value_map;
	CDBSink().get_groupUserInfoList_EX(nvcbid_, user_value_map);

	std::map<unsigned int, StUserBaseAndGroupInfo >::iterator iter_v = user_value_map.begin();
	for (; iter_v != user_value_map.end(); ++iter_v)
	{
		unsigned int userid = iter_v->first;

		//User Basic info
		CMDRoomUserInfo oMemInfo;
		oMemInfo.set_userid(userid);
		oMemInfo.set_vcbid(nvcbid_);
		oMemInfo.set_useralias(iter_v->second.userAlias);
		oMemInfo.set_chead(iter_v->second.userHead);
		oMemInfo.set_gender(iter_v->second.userGender);

		//User Group info,including role type
		oMemInfo.set_userlevel(iter_v->second.userGroupInfo.roleType);
		if (iter_v->second.userGroupInfo.authState && !iter_v->second.userGroupInfo.videoUrl.empty())
		{
			oMemInfo.set_authstate(iter_v->second.userGroupInfo.authState);
			oMemInfo.set_videourl(iter_v->second.userGroupInfo.videoUrl);
		}

		memberLst.push_back(oMemInfo);
	}
}

int CRoomObj::queryUserGroupInfo(unsigned int userid, CMDRoomUserInfo & oGroupInfo)
{
	if (!userid)
	{
		LOG_PRINT(log_error, "query user group info input error.userid:%u groupid:%u.", userid, nvcbid_);
		return ERR_CODE_INVALID_PARAMETER;
	}

	oGroupInfo.set_userid(userid);
	oGroupInfo.set_vcbid(nvcbid_);
	std::string strHead = "";
	std::string strAlias = "";
	CUserBasicInfo::getUserHeadAndAlias(userid, strHead, strAlias);
	oGroupInfo.set_chead(strHead);
	oGroupInfo.set_useralias(strAlias);

	unsigned int roleType = 0;
	if (CUserGroupinfo::getUserRoleType(userid, nvcbid_, roleType) == 0)
	{
		oGroupInfo.set_userlevel(roleType);
	}
	else
	{
		oGroupInfo.set_userlevel(e_VisitorRole);
	}
	
	return 0;
}

void CRoomObj::initGroupUserCount()
{
	unsigned int userNum = 0;
	unsigned int maleNum = 0;
	unsigned int femaleNum = 0;
	if (CDBSink().initRoomUserCountDB(nvcbid_, userNum, maleNum, femaleNum))
	{
		LOG_PRINT(log_info, "initRoomUserCountDB success and need to fresh to redis.groupid:%u,userNum:%u,maleNum:%u,femaleNum:%u.", nvcbid_, userNum, maleNum, femaleNum);
		if (!m_pRedis)
		{
			LOG_PRINT(log_error, "[initRoomUserCount to redis fail]redis is null.groupid:%u.", nvcbid_);
			return;
		}

		std::string fields = "user_num male_num female_num";
		user_num = userNum;
		male_num = maleNum;
		female_num = femaleNum;

		if (!updateRedisInfo(fields));
		{
			LOG_PRINT(log_error, "[initRoomUserCount]failed to write redis msg.groupid:%u,userNum:%u,maleNum:%u,femaleNum:%u.", nvcbid_, userNum, maleNum, femaleNum);
		}
	}
}

int CRoomObj::getMicList(std::vector<PubMicState_t>& list)
{
	PubMicState_t* p = getPubMicState();
	for(int i = 0 ; i < MAX_PUBMIC_COUNT ; i++)
	{
		list.push_back(p[i]);
	}

	return list.size();
}

int  CRoomObj::setMicposStatus(int index ,int status)
{
	int nret = 1;
	PubMicState_t* p = getPubMicState();

	if(index >= 0 && index < MAX_PUBMIC_COUNT)
	{
		int nUserId = p[index].userid;
		if(p[index].status != status)
		{
			if(status == 0)
			{
				if(p[index].onmic == 1)
				{
					if(nUserId > 0)
					{
						int nret = clearPubMicByUserid(nUserId);
						if(nret >= 0)
						{
							castSendOnMicState(nUserId,0,nret);
							clearTimerid();
							p[index].onmic = 0;
						}
						//下一个mic
						for(int i = index ; i < MAX_PUBMIC_COUNT ; i++)
						{
							if(p[i].status == 1 && p[i].userid > 0&&p[i].onmic == 0)
							{
								addTimerid();
								castSendOnMicState(p[i].userid,1,i,getInterval());
								p[i].onmic = 1;
								break;
							}
						}
					}
				}else if(p[index].onmic == 0)
				{
					if(nUserId > 0)
					{
						p[index].userid = 0;
						p[index].onmic = 0;
					}
				}
			}else if(status == 1)
			{
				if(m_micMode == e_QueueMode)
				{
					p[index].status = status;
					for(int i = 0 ; i < MAX_PUBMIC_COUNT ; i++)
					{
						if(p[i].userid == 0 && p[i].status ==1)
						{
							for (int j = i ; j < MAX_PUBMIC_COUNT ; j++)
							{
								if(p[j].userid > 0  && p[j].status == 1)
								{
									p[i].userid = p[j].userid;
									p[i].onmic = p[j].onmic;
									p[i].status = p[j].status;

									p[j].userid = 0;
									p[j].onmic = 0;
									p[j].status = 1;
									break;
								}
							}
						}
					}
				}
			}
		}
	}
	p[index].status = status;
	char key[32];
	sprintf(key, "%s:%u", KEY_HASH_ROOM_INFO, nvcbid_);
	string skey = key;
	string field = "micstatus";
	updateFields(skey,field);
	updateMicState();

	return nret;
}

int CRoomObj::setMicMode(e_MicMode mode)
{
	LOG_PRINT(log_info, "room_tag  <<<<<< setMicMode() end mode:2");

	LOG_PRINT(log_info, "room_tag  <<<<<< setMicMode() end mode:3");
	if(mode > 2 || mode < 0)
		mode = e_MicMode(0);
	m_micMode = mode;
	if (!CDBSink().updateGroupMicMode(nvcbid_, (int)m_micMode))
	{
		LOG_PRINT(log_error, "room_tag update group mic mode in db fail.groupid:%u,mode:%d.", nvcbid_, (int)m_micMode);
		return 0;
	}
	updateMicMode();
	for(int i = 0 ; i < MAX_PUBMIC_COUNT ;i++)
	{
		if(pubmicstate_[i].onmic == 1)
		{
			castSendOnMicState(pubmicstate_[i].userid,0,i);
			pubmicstate_[i].onmic = 0;
			clearTimerid();
		}
		pubmicstate_[i].userid = 0;
		pubmicstate_[i].status = 1;
	}
	updateMicState();
	return 1;
}

void CRoomObj::updateMicMode()
{
	char key[32];
	sprintf(key, "%s:%u", KEY_HASH_ROOM_INFO, nvcbid_);
	string skey = key;
	string field = "micmode";
	updateFields(skey,field);
}

int CRoomObj::getCurPubMicCount()
{
	if (!m_pRedis) {
		LOG_PRINT(log_error, "[vcbid:%u] redisOpt is NULL", nvcbid_);
		return nCurPubMicCount_;
	}
	char szKey[32];
	sprintf(szKey, "%s:%u", KEY_HASH_ROOM_INFO, nvcbid_);
	string val;
	if (m_pRedis->redis_hget(szKey, "curpubmiccount", val) < 0) {
		LOG_PRINT(log_error, "[vcbid:%u] Failed to get the user on mic", nvcbid_);
		return nCurPubMicCount_;
	}

	return atoi(val.c_str());
}

void CRoomObj::updateCurPubMicCount()
{
	char key[32];
	sprintf(key, "%s:%u", KEY_HASH_ROOM_INFO, nvcbid_);
	string skey = key;
	string field = "curpubmiccount";
	updateFields(skey,field);
}

int CRoomObj::getMaxPubMicCount()
{
	if (!m_pRedis) {
		LOG_PRINT(log_error, "[vcbid:%u] redisOpt is NULL", nvcbid_);
		return nMaxPubMicCount_;
	}
	char szKey[32];
	sprintf(szKey, "%s:%u", KEY_HASH_ROOM_INFO, nvcbid_);
	string val;
	if (m_pRedis->redis_hget(szKey, "maxpubmiccount", val) < 0) {
		LOG_PRINT(log_error, "[vcbid:%u] Failed to get the user on mic", nvcbid_);
		return nMaxPubMicCount_;
	}

	return atoi(val.c_str());
}

void CRoomObj::updateMaxPubMicCount()
{
	char key[32];
	sprintf(key, "%s:%u", KEY_HASH_ROOM_INFO, nvcbid_);
	string skey = key;
	string field = "maxpubmiccount";
	updateFields(skey,field);
}

void CRoomObj::OnTimer()
{
	LOG_PRINT(log_info, "room_tag exec timer 30 begin room:%u,this:%p,,timerid:%d", nvcbid_, this,timerid);
	PubMicState_t* p = getPubMicState();
	int nret = -1;
	for(int i = 0 ; i < MAX_PUBMIC_COUNT ;i++)
	{
		if(p[i].onmic == 1)
		{
			nret = i;
			break;
		}
	}
	LOG_PRINT(log_info, "room_tag exec timer 30 begin room:%u,this:%p,,ret:%d", nvcbid_, this,nret);
	if(nret >= 0)
	{
		int nUserId = pubmicstate_[nret].userid;
//		m_pRoomMgr->setRoommicstate(nvcbid_,nUserId,nret,"",0);
		m_pRoomMgr->changeWaitUser(nUserId,nvcbid_,-2);
	}
	LOG_PRINT(log_info,"room_tag exec timer 300 end.roomid:%u.", nvcbid_);
}
void CRoomObj::updateTimerid()
{
	char key[32];
	sprintf(key, "%s:%u", KEY_HASH_ROOM_INFO, nvcbid_);
	string skey = key;
	string field = "timerid begintime";
	updateFields(skey,field);
}
void CRoomObj::castSendOnMicState(uint32 userid,uint32 micstate,int32 micindex,int32 MicSec)
{
	std::string strHead;
	std::string strAlias;
	CUserBasicInfo::getUserHeadAndAlias(userid, strHead, strAlias);

	char szOutBuf[1024] = {0};
	ClientGateMask_t * pClientGate;
	CMDUserMicState micInfo;
	micInfo.set_vcbid(nvcbid_);
	micInfo.mutable_runid()->set_userid(0);
	micInfo.mutable_toid()->set_userid(userid);
	micInfo.mutable_toid()->set_head(strHead);
	micInfo.mutable_toid()->set_alias(strAlias);
	micInfo.set_micindex(micindex);
	micInfo.set_micstate(micstate);
	micInfo.set_micsecond(MicSec);

	SERIALIZETOARRAY_BUF(micInfo,respbuf,len);
	int nMsgLen = m_pRoomMgr->Build_NetMsg(szOutBuf, 1024, MDM_Vchat_Room, Sub_Vchat_SetMicStateNotify, &pClientGate, respbuf, len);
	if(nMsgLen > 0)
		castSendMsgXXX((COM_MSG_HEADER*)szOutBuf);
	LOG_PRINT(log_info, "room_tag castSendOnMicState castSendMsg roomid:%u.MicSec:%d", nvcbid_,MicSec);
}
int CRoomObj::checkNextMic()
{
	if(m_micMode != e_QueueMode)
		return -1;
	int nret = -1;
	PubMicState_t* p = getPubMicState();
	for(int i = 0 ; i < MAX_PUBMIC_COUNT ; i++)
	{
		if(p[i].status == 1 && p[i].onmic == 0 && p[i].userid > 0)
		{
			nret = i;
			break;
		}
	}
	if(nret >= 0)
	{
		//find space on mic
		bool bfirst = false;
		for(int i = 0 ; i < MAX_PUBMIC_COUNT ; i++)
		{
			if(p[i].userid == 0 && p[i].status ==1)
			{
				for (int j = i ; j < MAX_PUBMIC_COUNT ; j++)
				{
					if(p[j].userid > 0 && p[j].onmic == 0 && p[j].status == 1)
					{
						p[i].userid = p[j].userid;
						p[i].onmic = p[j].onmic;
						p[i].status = p[j].status;

						p[j].userid = 0;
						p[j].onmic = 0;
						p[j].status = 1;
						if(!bfirst)
						{
							bfirst = true;
							p[i].onmic = 1;
							addTimerid();
							castSendOnMicState(p[i].userid,1,i,getInterval());
						}
						break;
					}
				}
			}
		}
	}
	updateMicState();
//	castSendMicList();
	return 0;
}
void CRoomObj::castSendMicList()
{
	std::vector<PubMicState_t> list;
	getMicList(list);

	CMDMicStateListResp resp;
	ClientGateMask_t * pClientGate;
	char szBuf1[1024] = {0};
	resp.set_vcbid(nvcbid_);
	for(int i = 0 ; i < list.size() ; i++)
	{
		CMDMicStateListResp_PubMic * pm = resp.add_pubmic();
		std::string strHead;
		std::string strAlias;
		CUserBasicInfo::getUserHeadAndAlias(list[i].userid, strHead, strAlias);
		pm->mutable_userinfo()->set_userid(list[i].userid);
		pm->mutable_userinfo()->set_head(strHead);
		pm->mutable_userinfo()->set_alias(strAlias);
		pm->set_status(list[i].status);
		LOG_PRINT(log_info, "room_tag<<<<<< proc_micstatelist_req() end list[i].userid:%d,,%d.strHead:%s,strAlias:%s.",list[i].userid,nvcbid_,strHead.c_str(),strAlias.c_str());
	}
	SERIALIZETOARRAY_BUF(resp, respbuf, len);
	LOG_PRINT(log_info, "room_tag<<<<<< proc_micstatelist_req() end size:%d,,%d. len:%d", list.size(),nvcbid_,len);
	int nMsgLen = m_pRoomMgr->Build_NetMsg(szBuf1, 1024, MDM_Vchat_Room, Sub_Vchat_MicStateListResp, &pClientGate, respbuf, len);
	castSendMsgXXX((COM_MSG_HEADER*)szBuf1);
}

bool CRoomObj::downMics(bool visitor /*= false*/)
{
	int count = 0;
	for (int i = 0; i < nMaxPubMicCount_; ++i)
	{
		unsigned int userid = pubmicstate_[i].userid;
		if (userid && (!visitor || !CUserGroupinfo::isMember(userid, nvcbid_)))
		{
			count++;
			pubmicstate_[i].userid = 0;
			pubmicstate_[i].userendtime = 0;
			pubmicstate_[i].onmic = 0;
			nCurPubMicCount_--;
			LOG_PRINT(log_info, "room_tag downMics, left:%d, user:%u, room:%u", nCurPubMicCount_, userid, nvcbid_);
			clearTimerid();
			castSendOnMicState(userid, 0, i);
		}
	}

	if (count)
		updateMicState();
	changePos(0);
	castSendMicList();
	return count;
}
void CRoomObj::addTimerid()
{
	if(m_micMode == e_QueueMode)
	{
		timerid = Timer::getInstance()->add(MIC_TIME_PER, boost::bind(&CRoomObj::OnTimer,shared_from_this()),1);
		LOG_PRINT(log_info,"room_tag addtimer 30,roomid:%d,this:%p,MIC_TIME_PER:%d,,timerid:%d",nvcbid_,this,MIC_TIME_PER,timerid);
		m_micBeginTime = time(0);
		updateTimerid();
	}
}
void CRoomObj::clearTimerid()
{
	if(m_micMode == e_QueueMode)
	{
		if(timerid >= 0)
		{
			Timer::getInstance()->del(timerid);
			timerid = -1;
			m_micBeginTime = 0;
			updateTimerid();
			LOG_PRINT(log_info,"room_tag del timer 30");
		}
	}
}
void CRoomObj::changePos(int pos)
{
	if(m_micMode != e_QueueMode)
		return ;

	if(pos >= 0)
	{
		PubMicState_t* p = getPubMicState();
		//find space on mic
		bool bfirst = false;
		for(int i = 0 ; i < MAX_PUBMIC_COUNT ; i++)
		{
			if(p[i].userid == 0 && p[i].status ==1)
			{
				for (int j = i ; j < MAX_PUBMIC_COUNT ; j++)
				{
					if(p[j].userid > 0 && p[j].onmic == 0 && p[j].status == 1)
					{
						p[i].userid = p[j].userid;
						p[i].onmic = p[j].onmic;
						p[i].status = p[j].status;

						p[j].userid = 0;
						p[j].onmic = 0;
						p[j].status = 1;
						if(!bfirst)
						{
							bfirst = true;
							p[i].onmic = 1;
							addTimerid();
							castSendOnMicState(p[i].userid,1,i,getInterval());
						}
						break;
					}
				}
			}
		}
	}
	updateMicState();
	LOG_PRINT(log_info, "room_tag changePos, left:%d,, room:%u", nCurPubMicCount_, nvcbid_);
}

void CRoomObj::resetInfo(const stRoomFullInfo &info)
{
	nvcbid_ = info.ncourseid;
	nuser_id = info.nuser_id;							//群主
	nlive_id = info.nlive_id;
	class_name = info.class_name;
	teacher_name = info.teacher_name;
	teacher_info = info.teacher_info;
	img = info.img;
	src_img = info.src_img;
	nprice = info.nprice;
	nlevel = info.nlevel;
	strpwd = info.strpwd;
	invite_code = info.invite_code;
	strremark = info.strremark;
	tags = info.tags;
	goal = info.goal;
	dest_user = info.dest_user;
	requirements = info.requirements;
	nbegin_time = info.nbegin_time;
	nend_time = info.nend_time;
	nstatus = info.nstatus;
	npublish_time = info.npublish_time;
	ncreate_time = info.ncreate_time;
	nupdate_time = info.nupdate_time;
	talk_status = info.talk_status;
	nopenstatus = info.nopenstatus;
	virtual_num = info.virtual_num;
	form=info.form;

//	loadRoomBroadInfo();
}

bool CRoomObj::isNowInGroup(uint32 userid)
{
	return (m_rsMemberInGroup.count(nvcbid_, userid) > 0 || m_rsVisitorInGroup.count(nvcbid_, userid) > 0);
}

bool CRoomObj::inGroup(uint32 userid)
{
	if (m_rsMemberInGroup.count(nvcbid_, userid) <= 0)
	{
		if (m_rsMemberInGroup.insert(nvcbid_, userid))
		{
			CMDMemberEnterGroup enter;
			enter.set_groupid(nvcbid_);
			enter.set_userid(userid);
			unsigned int rspDataLen = SIZE_IVM_HEAD_TOTAL + enter.ByteSize();
			SL_ByteBuffer buff(rspDataLen);
			CMsgComm::Build_COM_MSG_HEADER(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_MemberEnterGroup, rspDataLen);
			enter.SerializeToArray(buff.buffer() + SIZE_IVM_HEAD_TOTAL, enter.ByteSize());
			castSendMsgXXX((COM_MSG_HEADER *)buff.buffer());
		}
	}

	return false;
}

bool CRoomObj::outGroup(uint32 userid)
{
	if (m_rsMemberInGroup.count(nvcbid_, userid) > 0)
	{
		m_rsMemberInGroup.erase(nvcbid_, userid);
		delWaitMicUser(userid); //下麦
		CMDMemberExitGroup exit;
		exit.set_groupid(nvcbid_);
		exit.set_userid(userid);
		unsigned int rspDataLen = SIZE_IVM_HEAD_TOTAL + exit.ByteSize();
		SL_ByteBuffer buff(rspDataLen);
		CMsgComm::Build_COM_MSG_HEADER(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_MemberExitGroup, rspDataLen);
		exit.SerializeToArray(buff.buffer() + SIZE_IVM_HEAD_TOTAL, exit.ByteSize());
		castSendMsgXXX((COM_MSG_HEADER *)buff.buffer(), 0, true);
		return true;
	}

	return false;
}

void CRoomObj::sendOnlineMemberList(task_proc_data *task_node)
{
	if (NULL == task_node)
		return;

	CMDGroupOnlineMemberList list;
	vector<uint32> vList;
	getInGroupList(vList);
	for (int i = 0; i < vList.size(); i++)
	{
		list.add_list(vList[i]);
	}

	task_node->respProtobuf(list, Sub_Vchat_GroupOnlineMemberList);
}
int CRoomObj::getInterval()
{
	int micSec = -1;
	if(m_micMode == e_QueueMode)
	{
		micSec = MIC_TIME_PER - (time(0) - m_micBeginTime);
		LOG_PRINT(log_info, "room_tag <<<<<< getInterval() micsec now:%d,,m_micBeginTime:%d.",time(0),m_micBeginTime);
		if(micSec > MIC_TIME_PER || micSec < 0)
		{
			micSec = -1;
		}
	}
	return micSec;
}

void CRoomObj::sendJoinGroupCondition(task_proc_data * message, uint32 userid)
{
	int ret = ERR_CODE_SUCCESS;

	std::string strJRecList;
	do 
	{
		if (!userid)
		{
			LOG_PRINT(log_error, "query join group condition input error.userid:%u,groupid:%u.", userid, nvcbid_);
			ret = ERR_CODE_INVALID_PARAMETER;
			break;
		}

		unsigned int authType = 0;
		unsigned int redpacketNum = 0;
		unsigned int redpacketTotal = 0;
		int gender = CUserBasicInfo::getUserGender(userid);
		switch(gender)
		{
		case e_MaleGender:
			authType = male_join_group_auth;
			redpacketNum = male_red_packet_num;
			redpacketTotal = male_red_packet_total;
			break;
		case e_FemaleGender:
			authType = female_join_group_auth;
			redpacketNum = female_red_packet_num;
			redpacketTotal = female_red_packet_total;
			break;
		default:
			LOG_PRINT(log_error, "query join group condition error.cannot get user gender.userid:%u.", userid);
			ret = ERR_CODE_FAILED_USERNOTFOUND;
			break;
		}

		if (ret != ERR_CODE_SUCCESS)
		{
			break;
		}

		strJRecList = genJoinGroupCond(authType, nvcbid_, gender, redpacketTotal, redpacketNum);
	} while (0);

	LOG_PRINT(log_debug, "query join group condition result:%s,userid:%u,groupid:%u.", strJRecList.c_str(), userid, nvcbid_);
	CMDQryJoinGroupCondResp oRspData;
	oRspData.mutable_errinfo()->set_errid(ret);
	oRspData.set_userid(userid);
	oRspData.set_groupid(nvcbid_);
	oRspData.set_action(strJRecList);

	message->respProtobuf(oRspData, Sub_Vchat_QryJoinGroupCondResp);
}

std::string CRoomObj::genJoinGroupCond(unsigned int authType, unsigned int groupid, int gender, unsigned int redpacketTotal, unsigned int redpacketNum)
{
	Json::Value root;
	/*
	视频认证入群
	{"action": "open_video_auth", "param": {"groupid": 3}}
	发炮弹入群
	{"action": "send_joingroup_redpacket_auth", "param": {"groupid": 3, "redpacketTotal": 10000, "redpacketNum": 10}}
	直接入群
	{"action": "none_auth", "param": {"groupid": 3}}
	留言入群
	{"action": "msg_auth", "param": {"groupid": 3}}
	禁止入群
	{"action": "forbidden", "param": {"groupid": 3}}
	*/
	switch(authType)
	{
	case NoNeedAuth:
		{
			LOG_PRINT(log_info, "not need to auth to join group.groupid:%u,gender:%d.", groupid, gender);
			Json::Value paramValue;
			paramValue["groupid"] = Json::Value(groupid);
			root["action"] = Json::Value("none_auth");
			root["param"] = paramValue;
		}
		break;
	case RedPacket:
		{
			LOG_PRINT(log_info, "redpacket auth to join group.red packet num:%u,red packet total:%u.groupid:%u,gender:%d.", redpacketNum, redpacketTotal, groupid, gender);
			Json::Value paramValue;
			paramValue["groupid"] = Json::Value(groupid);
			paramValue["redpacketTotal"] = Json::Value(redpacketTotal);
			paramValue["redpacketNum"] = Json::Value(redpacketNum);
			root["action"] = Json::Value("send_joingroup_redpacket_auth");
			root["param"] = paramValue;
		}
		break;
	case VideoAuth:
		{
			LOG_PRINT(log_info, "video auth to join group.groupid:%u,gender:%d.", groupid, gender);
			Json::Value paramValue;
			paramValue["groupid"] = Json::Value(groupid);
			root["action"] = Json::Value("open_video_auth");
			root["param"] = paramValue;
		}
		break;
	case MsgAuth:
		{
			LOG_PRINT(log_info, "msg auth to join group.groupid:%u,gender:%d.", groupid, gender);
			Json::Value paramValue;
			paramValue["groupid"] = Json::Value(groupid);
			root["action"] = Json::Value("msg_auth");
			root["param"] = paramValue;
		}
		break;
	case Forbidden:
		{
			LOG_PRINT(log_info, "forbidden to join group.groupid:%u,gender:%d.", groupid, gender);
			Json::Value paramValue;
			paramValue["groupid"] = Json::Value(groupid);
			root["action"] = Json::Value("forbidden");
			root["param"] = paramValue;
		}
		break;
	default:
		break;
	}

	Json::FastWriter fast_writer;
	std::string strJRecList = fast_writer.write(root);
	LOG_PRINT(log_debug, "genJoinGroupCond result:%s,groupid:%u.", strJRecList.c_str(), groupid);
	return strJRecList;
}

void CRoomObj::getRedisValuesMap(std::map<std::string, std::string> &jval, const string &fields)
{
	if (fields.empty())
	{
		return;
	}
	if (fields.find("courseid") != string::npos)
		jval["courseid"] = bitTostring(nvcbid_);
	if (fields.find("user_id") != string::npos)
		jval["user_id"] = bitTostring(nuser_id);
	if (fields.find("live_id") != string::npos)
		jval["live_id"] = bitTostring(nlive_id);
	if (fields.find("class_name") != string::npos)
		jval["class_name"] = bitTostring(class_name);
	if (fields.find("teacher_name") != string::npos)
		jval["teacher_name"] = bitTostring(teacher_name);
	if (fields.find("teacher_info") != string::npos)
		jval["teacher_info"] = bitTostring(teacher_info);
	if (fields.find("img") != string::npos)
		jval["img"] = bitTostring(img);
	if (fields.find("src_img") != string::npos)
		jval["src_img"] = bitTostring(src_img);
	if (fields.find("price") != string::npos)
		jval["price"] = bitTostring(nprice);
	if (fields.find("level") != string::npos)
		jval["level"] = bitTostring(nlevel);
	if (fields.find("strpwd") != string::npos)
		jval["strpwd"] = bitTostring(strpwd);
	if (fields.find("invite_code") != string::npos)
		jval["invite_code"] = bitTostring(invite_code);
	if (fields.find("strremark") != string::npos)
		jval["strremark"] = bitTostring(strremark);
	if (fields.find("tags") != string::npos)
		jval["tags"] = bitTostring(tags);
	if (fields.find("goal") != string::npos)
		jval["goal"] = bitTostring(goal);
	if (fields.find("dest_user") != string::npos)
		jval["dest_user"] = bitTostring(dest_user);
	if (fields.find("requirements") != string::npos)
		jval["requirements"] = bitTostring(requirements);
	if (fields.find("begin_time") != string::npos)
		jval["begin_time"] = bitTostring(nbegin_time);
	if (fields.find("end_time") != string::npos)
		jval["end_time"] = bitTostring(nend_time);
	if (fields.find("status") != string::npos)
		jval["status"] = bitTostring(nstatus);
	if (fields.find("openstatus") != string::npos)
			jval["openstatus"] = bitTostring(nopenstatus);
	if (fields.find("publish_time") != string::npos)
		jval["publish_time"] = bitTostring(npublish_time);
	if (fields.find("create_time") != string::npos)
		jval["create_time"] = bitTostring(ncreate_time);
	if (fields.find("update_time") != string::npos)
		jval["update_time"] = bitTostring(nupdate_time);
	if (fields.find("class_name") != string::npos)
		jval["class_name"] = bitTostring(class_name);
	if (fields.find("audience_num") != string::npos)
		jval["audience_num"] = bitTostring(audience_num);

	if (fields.find("virtual_num") != string::npos)
		jval["virtual_num"] = bitTostring(virtual_num);


	if (fields.find("talk_status") != string::npos)
		jval["talk_status"] = bitTostring(talk_status);

	if (fields.find("opusers") != string::npos)
	{
		Json::Value val;
		for(int i = 0 ; i < MAX_GUEST ; i++)
		{
			val.append(nopuser[i]);
		}
		jval["opusers"] = Json::FastWriter().write(val);
	}

//	if (fields.find("begintime") != string::npos)
//		jval["begintime"] = bitTostring(m_micBeginTime);
//	if (fields.find("timerid") != string::npos)
//		jval["timerid"] = bitTostring(timerid);
//	if (fields.find("micmode") != string::npos)
//		jval["micmode"] = bitTostring((int)m_micMode);
//	if (fields.find("vcbid") != string::npos)
//		jval["vcbid"] = bitTostring(nvcbid_);
//	if (fields.find("name") != string::npos)
//		jval["name"] = name;
//	if (fields.find("icon") != string::npos)
//		jval["icon"] = icon;
//	if (fields.find("remark") != string::npos)
//		jval["remark"] = remark;
//	if (fields.find("user_num") != string::npos)
//		jval["user_num"] = bitTostring(user_num);
//	if (fields.find("male_num") != string::npos)
//		jval["male_num"] = bitTostring(male_num);
//	if (fields.find("female_num") != string::npos)
//		jval["female_num"] = bitTostring(female_num);
//	if (fields.find("audience_num") != string::npos)
//		jval["audience_num"] = bitTostring(audience_num);
//	if (fields.find("status") != string::npos)
//		jval["status"] = bitTostring(status);
//	if (fields.find(CFieldName::ROOM_GROUPMASTER) != string::npos)
//		jval[CFieldName::ROOM_GROUPMASTER] = bitTostring(group_master);
//	if (fields.find("show_square") != string::npos)
//		jval["show_square"] = bitTostring(show_square);
//	if (fields.find("capacity") != string::npos)
//		jval["capacity"] = bitTostring(capacity);
//	if (fields.find("roomnotice") != string::npos)
//		jval["roomnotice"] = m_roomnotice.m_strRoomNotice;
//	if (fields.find("noticeuserid") != string::npos)
//		jval["noticeuserid"] = bitTostring(m_roomnotice.m_uBroadUserid);
//	if (fields.find("noticetime") != string::npos)
//		jval["noticetime"] = bitTostring(m_roomnotice.m_uBroadTime);
//	if (fields.find("showid") != string::npos)
//		jval["showid"] = bitTostring(showid);
//	if (fields.find(CFieldName::ROOM_UNREAD_MSGNUM.c_str()) != string::npos)
//		jval[CFieldName::ROOM_UNREAD_MSGNUM] = bitTostring(iUnreadMsgNum);
//	if (fields.find(CFieldName::ROOM_ALLOW_VISITGROUP.c_str()) != string::npos)
//		jval[CFieldName::ROOM_ALLOW_VISITGROUP] = bitTostring(uAllowVisitGroup);
//	if (fields.find(CFieldName::ROOM_GAIN_PERCENT.c_str()) != string::npos)
//		jval[CFieldName::ROOM_GAIN_PERCENT] = bitTostring(gainPercent);
//	if (fields.find(CFieldName::ROOM_GAIN_USERRANGE.c_str()) != string::npos)
//		jval[CFieldName::ROOM_GAIN_USERRANGE] = bitTostring(gainUserRange);
//	if (fields.find(CFieldName::ROOM_MALE_JOIN_GROUP_AUTH.c_str()) != string::npos)
//		jval[CFieldName::ROOM_MALE_JOIN_GROUP_AUTH] = bitTostring(male_join_group_auth);
//	if (fields.find(CFieldName::ROOM_MALE_REDPACKET_NUM.c_str()) != string::npos)
//		jval[CFieldName::ROOM_MALE_REDPACKET_NUM] = bitTostring(male_red_packet_num);
//	if (fields.find(CFieldName::ROOM_MALE_REDPACKET_TOTAL.c_str()) != string::npos)
//		jval[CFieldName::ROOM_MALE_REDPACKET_TOTAL] = bitTostring(male_red_packet_total);
//	if (fields.find(CFieldName::ROOM_FEMALE_JOIN_GROUP_AUTH.c_str()) != string::npos)
//		jval[CFieldName::ROOM_FEMALE_JOIN_GROUP_AUTH] = bitTostring(female_join_group_auth);
//	if (fields.find(CFieldName::ROOM_FEMALE_REDPACKET_NUM.c_str()) != string::npos)
//		jval[CFieldName::ROOM_FEMALE_REDPACKET_NUM] = bitTostring(female_red_packet_num);
//	if (fields.find(CFieldName::ROOM_FEMALE_REDPACKET_TOTAL.c_str()) != string::npos)
//		jval[CFieldName::ROOM_FEMALE_REDPACKET_TOTAL] = bitTostring(female_red_packet_total);
//	if (fields.find(CFieldName::ROOM_CONTRIBUTION_SWITCH.c_str()) != string::npos)
//		jval[CFieldName::ROOM_CONTRIBUTION_SWITCH] = bitTostring(contribution_switch);
//	if (fields.find(CFieldName::ROOM_VOICE_CHAT_SWITCH.c_str()) != string::npos)
//		jval[CFieldName::ROOM_VOICE_CHAT_SWITCH] = bitTostring(voice_chat_switch);
//	if (fields.find(CFieldName::ROOM_ALLOW_VISITOR_ON_MIC.c_str()) != string::npos)
//		jval[CFieldName::ROOM_ALLOW_VISITOR_ON_MIC] = bitTostring(allow_visitor_on_mic);
//
//
//	if (fields.find("opstate") != string::npos)
//		jval["opstate"] = bitTostring(nopstate_);
//	if (fields.find("creatorid") != string::npos)
//		jval["creatorid"] = bitTostring(ncreatorid_);
//
//	if (fields.find("waitusers") != string::npos) {
//		Json::Value val;
//		for(int i =0 ; i < vecWaitMics_.size() ; i++)
//		{
//			val.append(vecWaitMics_[i].userid);
//		}
//		jval["waitusers"] = Json::FastWriter().write(val);
//	}
//	if (fields.find("micindex") != string::npos) {//micindex micuserid mictimetype
//		Json::Value val;
//		for(int i =0 ; i < MAX_PUBMIC_COUNT ; i++)
//		{
//			val.append(i);
//		}
//		jval["micindex"] = Json::FastWriter().write(val);
//	}
//	if (fields.find("micuserid") != string::npos) {//micindex micuserid mictimetype
//		Json::Value val;
//		for(int i =0 ; i < MAX_PUBMIC_COUNT ; i++)
//		{
//			val.append(pubmicstate_[i].userid);
//			printf("pubmicUser:%d\n",pubmicstate_[i].userid);
//		}
//		jval["micuserid"] = Json::FastWriter().write(val);
//
//	}
//	if (fields.find("mictimetype") != string::npos) {//micindex micuserid mictimetype onmic
//		Json::Value val;
//		for(int i =0 ; i < MAX_PUBMIC_COUNT ; i++)
//		{
//			val.append(pubmicstate_[i].mictimetype);
//		}
//		jval["mictimetype"] = Json::FastWriter().write(val);
//	}
//	if (fields.find("micstatus") != string::npos) {//micindex micuserid mictimetype onmic
//		Json::Value val;
//		for(int i =0 ; i < MAX_PUBMIC_COUNT ; i++)
//		{
//			val.append(pubmicstate_[i].status);
//		}
//		jval["micstatus"] = Json::FastWriter().write(val);
//	}
//	if (fields.find("onmic") != string::npos) {//micindex micuserid mictimetype onmic
//		Json::Value val;
//		for(int i =0 ; i < MAX_PUBMIC_COUNT ; i++)
//		{
//			val.append(pubmicstate_[i].onmic);
//		}
//		jval["onmic"] = Json::FastWriter().write(val);
//	}
}

bool CRoomObj::updateRedisInfo(std::string fields/* = ""*/)
{
	if (fields.empty())
	{
		fields = getRedisFields();
	}
	std::map<std::string, std::string> mValues;
	getRedisValuesMap(mValues, fields);
	if (!mValues.empty())
	{
		return m_rmGroupInfo.insert(nvcbid_, mValues);
	}

	return false;
}

int CRoomObj::getOnMicCount()
{
	PubMicState_t* p = pubmicstate_;
	int count = 0;
	for(int i = 0 ; i < MAX_PUBMIC_COUNT ; i++)
	{
		if (p[i].userid > 0)
			count++;
	}

	return count;
}
int CRoomObj::getForbidUserChat(int userid)
{
	if (userid > 0)
	{
		char key[32] = { 0 };
		sprintf(key, "%u:%u", nvcbid_, userid);
		string strstatus;
		m_rmRoomuserSetting.get(string(key), "forbitchat", strstatus);
		return atoi(strstatus.c_str());
	}
	return 0;
	
}
int CRoomObj::setForbidUserChat(int runuserid, int userid, int status)
{
	int  ret = ERR_CODE_HASE_SET_FORBIT_CHAT_SWITCH;
	if(userid > 0)
	{
		char key[32] = {0};
		sprintf(key, "%u:%u", nvcbid_, userid);
		string strstatus;
		m_rmRoomuserSetting.get(string(key),"forbitchat",strstatus);
		if(atoi(strstatus.c_str()) != status)
		{
			std::map<string,string> m_fieldvaluemap;
			m_fieldvaluemap["runuserid"] = bitTostring(runuserid); // 20171114, wxc: int -> string
			m_fieldvaluemap["forbitchat"] = bitTostring(status);
			m_fieldvaluemap["datetime"] = bitTostring(int(time(0)));
			m_rmRoomuserSetting.insert(string(key),m_fieldvaluemap);
			m_rmRoomuserSetting.expire(string(key), 24 * 60 * 60); // 20171114, wxc
			ret = 0;
		}
	}
	else
	{
		if(talk_status != status)
		{
			talk_status = status;
			char key[32];
			sprintf(key, "%s:%u", KEY_HASH_ROOM_INFO, nvcbid_);
			string skey = key;
			string field = "talk_status";
			updateFields(skey,field);

			if (!CDBSink().updateForbidUserChat(nvcbid_, status))
			{
				LOG_PRINT(log_error, "room_tag update setForbidUserChat in db fail.groupid:%u,status:%d.", nvcbid_, status);
			}
			ret = 0;
		}
	}
	return ret;
}
void CRoomObj::setCourseFinish()
{
	status = 4;
	nend_time = getTime("%Y-%m-%d %H:%M:%S");
	char key[32];
	sprintf(key, "%s:%u", KEY_HASH_ROOM_INFO, nvcbid_);
	string skey = key;
	string field = "status nend_time";
	updateFields(skey,field);

	if (!CDBSink().updateCourseFinish(nvcbid_))
	{
		LOG_PRINT(log_error, "room_tag update setCourseFinish in db fail.groupid:%u", nvcbid_);
	}

}
bool CRoomObj::findOnlineUser(CRoomManager * pRoomMgr, redisOpt *pRedisOpt, uint32 userId, UserObject_Ref & userobjRef)
{
	if (!pRoomMgr || !pRedisOpt || !userId)
		return false;

	vector<string> vecfield, vecvalue;

	char key[32];
	sprintf(key, "%s:%u", KEY_HASH_USER_INFO, userId);
	{
		redisPipeline pipeline(pRedisOpt);
		pRedisOpt->redis_hgetall(key, vecfield, vecvalue);
		pipeline.getreply(vecfield);
		pipeline.getreply(vecvalue);
	}

	if (0 == vecfield.size() || 0 == vecvalue.size()) {
//		LOG_PRINT(log_error, "no found user:%u", userId);
		return false;
	}

	userobjRef.reset(new CUserObj(vecfield, vecvalue));
	if (userobjRef){
		map<uint32, clienthandler_ptr>::iterator it = pRoomMgr->m_mapConn.find(userobjRef->pConnId_);
		if (it != pRoomMgr->m_mapConn.end())
		{
			clienthandler_ptr pconn = it->second;
			if (pconn && pconn->getgateid() == userobjRef->ngateid) {
				userobjRef->pConn_ = pconn;
			}
		}
	}
	else {
		userobjRef.reset();
		LOG_PRINT(log_error, "[user:%u] Failed to read user info from redis", userId);
	}

	return (userobjRef.get() != NULL);
}

string CRoomObj::getRoomLiveUrl()
{
	string liveUrl;
	CDBTools::getLiveUrlByGroupid(nvcbid_,liveUrl);
	LOG_PRINT(log_info, "liveurl:%s", liveUrl.c_str());
	return liveUrl;
}

bool CRoomObj::isPPTRoom()
{
	return form==3;
}