#include "json/json.h"
#include "ChatMongoMgr.h"
#include "mongo/bson/bsonobj.h"
#include "macro_define.h"
#include "DBTools.h"
#include "DBSink.h"

#define MAX_INT ((int)(~0U>>1))

CChatMongoMgr::CChatMongoMgr(void)
{
}

CChatMongoMgr::~CChatMongoMgr(void)
{
}

bool CChatMongoMgr::getNextSequence(std::string strSeqName, uint32 &value, uint32 inc /*= 1*/)
{
	try
	{
		std::string ns = MONGO_SEQUENCES_DB".sequences";
		mongo::BSONObj query = BSON( MONGO_KEY << strSeqName.c_str() );
		mongo::BSONObj update = BSON( "$inc" << BSON( "value" << inc ) );
		mongo::BSONObj result = CMongoDBConnection()->findAndModify(ns.c_str(), query, update, true, true);

		if (result.hasElement("value"))
		{
			const mongo::BSONElement &bel = result.getField("value");
			value = (uint32)bel.numberInt();
		}
		else
		{
			LOG_PRINT(log_error, "Invalid mongo key[value] of ns[%s]", ns.c_str());
			return false;
		}
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
		return false;
	}

	return true;
}

bool CChatMongoMgr::getCurSequence(const std::string & strSeqName, uint32 &value)
{
	try
	{
		std::string ns = MONGO_SEQUENCES_DB".sequences";
		mongo::BSONObj query = BSON( MONGO_KEY << strSeqName.c_str() );
		mongo::BSONObj result = CMongoDBConnection()->findOne(ns.c_str(), query);

		if (result.hasElement("value"))
		{
			const mongo::BSONElement &bel = result.getField("value");
			value = (uint32)bel.numberInt();
		}
		else
		{
			LOG_PRINT(log_error, "Invalid mongo key[value] of ns[%s]", ns.c_str());
			return false;
		}
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
		return false;
	}

	return true;
}

bool CChatMongoMgr::writePrivateMsg(const CMDPrivateMsgReq &req)
{
	try
	{
		mongo::BSONObjBuilder rootObj;
		const ChatMsg_t &msg = req.msg();
		rootObj.append(MONGO_KEY, msg.msgid());
		rootObj.append(MONGO_CHAT_SRC_USERID, msg.srcuser().userid());
		rootObj.append(MONGO_CHAT_DST_USERID, msg.dstuser().userid());
		rootObj.append(MONGO_CHAT_MSG_TIME, msg.msgtime());
		rootObj.append(MONGO_CHAT_MSG_TYPE, msg.msgtype());
		rootObj.append(MONGO_CHAT_CONTENT, msg.content());
		rootObj.append(MONGO_CHAT_CLIENT_MSGID, (int64)msg.clientmsgid());		

		std::string ns = CChatCommon::getFormatString("%s.%s", MONGO_PRIVATE_CHAT_MSG_DB, CChatCommon::getPChatSeqName(msg.srcuser().userid(), msg.dstuser().userid()).c_str());
		CMongoDBConnection()->insert(ns, rootObj.obj());
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
		return false;
	}

	return true;
}

bool CChatMongoMgr::qryPrivateMsg(uint32 srcuid, uint32 dstuid, uint32 msgid, ChatMsg_t &msg)
{
	try
	{
		mongo::BSONObj query = BSON("query" << BSON(MONGO_KEY << msgid));
		std::string ns = CChatCommon::getFormatString("%s.%s", MONGO_PRIVATE_CHAT_MSG_DB, CChatCommon::getPChatSeqName(srcuid, dstuid).c_str());
		std::auto_ptr<mongo::DBClientCursor> cursor = CMongoDBConnection()->query(ns, mongo::Query(query));
		if (cursor.get() && cursor->more())
		{
			const mongo::BSONObj &obj = cursor->next();
			return parsePrivateMsg(obj, msg);
		}
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
		return false;
	}

	return false;
}

size_t CChatMongoMgr::qryPrivateMsgList(uint32 requid, uint32 dstuid, std::list<ChatMsg_t> &vMsg, uint32 msgid /*= 0*/, int32 count /*= 0*/, bool forward /*= true*/)
{
	if (0 == requid || 0 == dstuid)
	{
		return 0;
	}

	if (0 == msgid && forward)
	{
		msgid = MAX_INT;
	}

	size_t total= 0, result = 0;
	try
	{
		mongo::BSONObj query = BSON("query" << BSON(MONGO_KEY << BSON((forward ? "$lt" : "$gt") << msgid) /*<< 
													"$or" << BSON_ARRAY(BSON(MONGO_CHAT_MSG_TYPE << BSON("$ne" << MSGTYPE_REMIND)) << BSON(MONGO_CHAT_DST_USERID << requid))*/) 
								<< "orderby" << BSON(MONGO_KEY << (forward ? -1 : 1)));

		std::string ns = CChatCommon::getFormatString("%s.%s", MONGO_PRIVATE_CHAT_MSG_DB, CChatCommon::getPChatSeqName(requid, dstuid).c_str());
		std::auto_ptr<mongo::DBClientCursor> cursor = CMongoDBConnection()->query(ns, mongo::Query(query), count);
		while (cursor.get() && cursor->more())
		{
			total++;
			const mongo::BSONObj &obj = cursor->next();
			ChatMsg_t msg;
			if (parsePrivateMsg(obj, msg))
			{
				vMsg.push_back(msg);
				result++;
			}
		}
		LOG_PRINT(log_debug, "ns[%s] query[%s] total[%u] count[%u]", ns.c_str(), query.toString().c_str(), total, result);
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
		return false;
	}

	return result;
}

bool CChatMongoMgr::parsePrivateMsg(const mongo::BSONObj &bsonMsg, ChatMsg_t &msg)
{
	if (bsonMsg.hasElement(MONGO_KEY))
		msg.set_msgid(bsonMsg.getField(MONGO_KEY).numberInt());
	if (bsonMsg.hasElement(MONGO_CHAT_SRC_USERID))
		msg.mutable_srcuser()->set_userid(bsonMsg.getField(MONGO_CHAT_SRC_USERID).numberInt());
	if (bsonMsg.hasElement(MONGO_CHAT_DST_USERID))
		msg.mutable_dstuser()->set_userid(bsonMsg.getField(MONGO_CHAT_DST_USERID).numberInt());
	if (bsonMsg.hasElement(MONGO_CHAT_MSG_TIME))
		msg.set_msgtime(bsonMsg.getField(MONGO_CHAT_MSG_TIME).numberInt());
	if (bsonMsg.hasElement(MONGO_CHAT_MSG_TYPE))
		msg.set_msgtype((e_MsgType)bsonMsg.getField(MONGO_CHAT_MSG_TYPE).numberInt());
	if (bsonMsg.hasElement(MONGO_CHAT_CONTENT))
		msg.set_content(bsonMsg.getField(MONGO_CHAT_CONTENT).String());
	if (bsonMsg.hasElement(MONGO_CHAT_CLIENT_MSGID))
	{
		msg.set_clientmsgid(bsonMsg.getField(MONGO_CHAT_CLIENT_MSGID).numberLong());
	}
	if (bsonMsg.hasElement(MONGO_CHAT_RECALL))
	{
		msg.set_recall(bsonMsg.getField(MONGO_CHAT_RECALL).Bool());
	}

	return true;
}

bool CChatMongoMgr::updPrivateMsgId(uint32 userid, uint32 srcuid, uint32 msgid)
{
	if (0 == userid || 0 == srcuid)
	{
		return false;
	}
	
	try
	{
		mongo::BSONObj query = BSON(MONGO_KEY << srcuid << MONGO_CHAT_READ_MSGID << BSON( "$lt" << msgid ) );
		mongo::BSONObj modify = BSON( "$set" << BSON(MONGO_CHAT_READ_MSGID << msgid) );
		std::string ns = CChatCommon::getFormatString("%s.%u", MONGO_PRIVATE_CHAT_STAT_DB, userid);
		CMongoDBConnection()->update(ns, query, modify, true);
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
		return false;
	}
	return true;
}

bool CChatMongoMgr::qryUserPrivateMsgIdList(uint32 userid, std::map<uint32, uint32> &mMsgId)
{
	if (0 == userid)
	{
		return false;
	}

	size_t total = 0, result = 0;
	try
	{
		mMsgId.clear();
		std::string ns = CChatCommon::getFormatString("%s.%u", MONGO_PRIVATE_CHAT_STAT_DB, userid);
		std::auto_ptr<mongo::DBClientCursor> cursor = CMongoDBConnection()->query(ns);
		while (cursor.get() && cursor->more())
		{
			total++;
			mongo::BSONObj obj = cursor->next();
			if (!obj.hasElement(MONGO_CHAT_DST_USERID) || !obj.hasElement(MONGO_CHAT_READ_MSGID))
			{
				continue;
			}

			mongo::BSONElement dstuid = obj[MONGO_CHAT_DST_USERID];
			mongo::BSONElement msgid = obj[MONGO_CHAT_READ_MSGID];
			mMsgId[dstuid.numberInt()] = msgid.numberInt();
			result++;
		}
		LOG_PRINT(log_debug, "ns[%s] total[%u] count[%u]", ns.c_str(), total, result);
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
		return false;
	}

	return !mMsgId.empty();
}

size_t CChatMongoMgr::qryUnreadPrivateMsg(uint32 userid, uint32 dstuid, std::list<ChatMsg_t> &lstMsg, uint32 msgid /*= 0*/, int32 count /*= 0*/)
{
	if (0 == userid || 0 == dstuid)
	{
		return 0;
	}

	size_t total = 0, result = 0;
	try
	{
		mongo::BSONObj query = BSON("query" << BSON(MONGO_KEY << BSON("$gt" << msgid) /*<< 
											"$or" << BSON_ARRAY(BSON(MONGO_CHAT_MSG_TYPE << BSON("$ne" << MSGTYPE_REMIND)) << BSON(MONGO_CHAT_DST_USERID << userid))*/)
								<< "orderby" << BSON(MONGO_KEY << (count ? -1: 1)));

		uint32 maxMsgId = 0;
		std::string ns = CChatCommon::getFormatString("%s.%s", MONGO_PRIVATE_CHAT_MSG_DB, CChatCommon::getPChatSeqName(userid, dstuid).c_str());
		std::auto_ptr<mongo::DBClientCursor> cursor = CMongoDBConnection()->query(ns, query, count);
		while (cursor.get() && cursor->more())
		{
			total++;
			const mongo::BSONObj &obj = cursor->next();
			ChatMsg_t msg;
			if (parsePrivateMsg(obj, msg))
			{
				if (maxMsgId < msg.msgid())
					maxMsgId = msg.msgid();

				if (msg.srcuser().userid() == userid)
				{
					total++;
				}

				if (count)
					lstMsg.push_front(msg);
				else
					lstMsg.push_back(msg);
				result++;
			}
		}
		if (maxMsgId && total == result) //如果都是自己的聊天消息，则更新msgid即可
		{
			updPrivateMsgId(userid, dstuid, maxMsgId);
		}
		LOG_PRINT(log_debug, "ns[%s] query[%s] total[%u] count[%u]", ns.c_str(), query.toString().c_str(), total, result);
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
		return result;
	}

	return result;

}

bool CChatMongoMgr::writeGroupMsg(const CMDGroupMsgReq &req,bool blobby)
{
	try
	{
		mongo::BSONObjBuilder builder;
		const ChatMsg_t &msg = req.msg();
		builder.append(MONGO_KEY, msg.msgid());
		builder.append(MONGO_CHAT_SRC_USERID, msg.srcuser().userid());
		builder.append(MONGO_CHAT_DST_USERID, msg.dstuser().userid());
		builder.append(MONGO_CHAT_MSG_TIME, msg.msgtime());
		builder.append(MONGO_CHAT_MSG_TYPE, msg.msgtype());
		builder.append(MONGO_CHAT_CONTENT, msg.content());
		builder.append(MONGO_CHAT_CLIENT_MSGID, (int64)msg.clientmsgid());
		builder.append(MONGO_CHAT_MEDIA_LENGTH, msg.medialength());
		builder.append(MONGO_CHAT_MASTER_MSGID, msg.mastermsgid());
		if (msg.extendtype())
		{
			builder.append(MONGO_CHAT_EXTEND_TYPE, msg.extendtype());
		}
		
		

		mongo::BSONArrayBuilder arrBuilder;
		for (int i = 0; i < msg.atlist_size(); i++)
		{
			arrBuilder << msg.atlist(i);
		}
		builder.append(MONGO_CHAT_AT_LIST, arrBuilder.arr());

		std::string ns;
		if(blobby)
		{
			ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_PCHAT_MSG_DB, req.groupid());
		}else
		{
			ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_CHAT_MSG_DB, req.groupid());
		}

		CMongoDBConnection()->insert(ns, builder.obj());
		LOG_PRINT(log_error, "CMongoDBConnection()->insert");
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
		return false;
	}

	return true;
}


bool CChatMongoMgr::writePPTrelationMsg(const CMDGroupMsgReq &req,const std::string &sPicMediaId)
{
	try
	{
		mongo::BSONObjBuilder builder;
		const ChatMsg_t &msg = req.msg();
		builder.append(MONGO_KEY, sPicMediaId);


		mongo::BSONArrayBuilder arrBuilder;

		arrBuilder << msg.msgid();
		
		builder.append(MONGO_GROUP_CHAT_PPT_RELATION_VOICE_LIST, arrBuilder.arr());

		std::string ns;

		ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_CHAT_PPT_RELATION_DB, req.groupid());
		

		CMongoDBConnection()->insert(ns, builder.obj());
		LOG_PRINT(log_info, "CMongoDBConnection()->insert");
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
		return false;
	}

	return true;
}


//新增或者追加数组
bool CChatMongoMgr::updatePPTrelationMsg(const uint32 &groupId,const std::string &sPicMediaId,const std::vector<int32> &vAddMsgId)
{
	try
	{


		for (unsigned int i=0;i<vAddMsgId.size();i++)
		{
			mongo::BSONObj query = BSON(MONGO_KEY << sPicMediaId);
			mongo::BSONObj modify = BSON( "$push" << BSON(MONGO_GROUP_CHAT_PPT_RELATION_VOICE_LIST << vAddMsgId[i] ) );
			std::string ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_CHAT_PPT_RELATION_DB, groupId);
			CMongoDBConnection()->update(ns, query, modify, true);
		}
		
		return true;




		LOG_PRINT(log_info, "CMongoDBConnection()->update");
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
		return false;
	}

	return true;
}

//查询数组
bool CChatMongoMgr::queryPPTrelationMsg(const uint32 &groupId,const std::string &sPicMediaId,std::vector<int32> &vMsgId)
{
	try
	{

		mongo::BSONObj query = BSON(MONGO_KEY << sPicMediaId);

		std::string ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_CHAT_PPT_RELATION_DB, groupId);
		std::auto_ptr<mongo::DBClientCursor> cursor = CMongoDBConnection()->query(ns, mongo::Query(query));
		if (cursor.get() && cursor->more())
		{
			const mongo::BSONObj &bsonMsg = cursor->next();
			
			if (bsonMsg.hasElement(MONGO_GROUP_CHAT_PPT_RELATION_VOICE_LIST))
			{
				mongo::BSONElement ele = bsonMsg.getField(MONGO_GROUP_CHAT_PPT_RELATION_VOICE_LIST);
				mongo::BSONObjIterator i(ele.Obj());
				while (i.more()) 
				{
					const mongo::BSONElement &e = i.next();
					vMsgId.push_back(e.numberInt());
				}
			}
		}




		LOG_PRINT(log_info, "CMongoDBConnection()->query");
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
		return false;
	}

	return true;
}

//删除一行记录
bool CChatMongoMgr::deletePPTrelationMsg(const uint32 &groupId,const std::string &sPicMediaId)
{
	try
	{

		mongo::BSONObj query = BSON(MONGO_KEY << sPicMediaId);
		std::string ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_CHAT_PPT_RELATION_DB, groupId);
		CMongoDBConnection()->remove(ns, query, true);
		

		LOG_PRINT(log_info, "CMongoDBConnection()->remove");
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
		return false;
	}

	return true;
}



bool CChatMongoMgr::qryGroupPMsgCount(uint32 groupid, uint32& count)
{
	try
	{
		std::string ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_PCHAT_MSG_DB, groupid);
		count = CMongoDBConnection()->count(ns,mongo::Query());
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "qryPrivateMsg catch exception: %s.", e.what());
		return false;
	}

	return false;
}

bool CChatMongoMgr::queryCurrentDayGroupMsgCount(uint32 groupid, uint32& count)
{
	try
	{
		long timeBegin = 0,timeEnd = 0;
		long now = time(NULL);
		std::string sDate;
		CChatCommon::unixTime2dateString(now, sDate);
		CChatCommon::dateString2unixTime(sDate+"000000",timeBegin);
		CChatCommon::dateString2unixTime(sDate+"235959",timeEnd);
		mongo::BSONObj query = BSON( "msgtime" << BSON("$gt" << (int)timeBegin << "$lt" << (int)timeEnd)
			<< MONGO_CHAT_MSG_TYPE << BSON("$ne" << MSGTYPE_RECALL) << MONGO_CHAT_RECALL << BSON("$ne" << true) );

		std::string ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_CHAT_MSG_DB, groupid);

		LOG_PRINT(log_debug, "ns[%s] query[%s].", ns.c_str(), query.toString().c_str());
		count = CMongoDBConnection()->count(ns,mongo::Query(query));
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "queryCurrentDayGroupMsgCount catch exception: %s.", e.what());
		return false;
	}

	return true;
}

bool CChatMongoMgr::qryGroupMsg(uint32 groupid, uint32 msgid, ChatMsg_t &msg, bool blobby)
{
	try
	{
		LOG_PRINT(log_error, "qryGroupMsg  msgid:%d,blobby:%d",  msgid,blobby);
		mongo::BSONObj query = BSON("query" << BSON(MONGO_KEY << msgid));
		std::string ns = "";
		if(blobby)
		{
			ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_PCHAT_MSG_DB, groupid);
		}else
		{
			ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_CHAT_MSG_DB, groupid);
		}
		LOG_PRINT(log_error, "qryGroupMsg ns:%s.msgid:%d,blobby:%d", ns.c_str(),msgid,blobby);
		std::auto_ptr<mongo::DBClientCursor> cursor = CMongoDBConnection()->query(ns, mongo::Query(query));
		if (cursor.get() && cursor->more())
		{
			const mongo::BSONObj &obj = cursor->next();
			return parseGroupMsg(obj, msg);
		}
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "qryPrivateMsg catch exception: %s.", e.what());
		return false;
	}

	return false;
}
bool CChatMongoMgr::getgroupmsg_t(int groupid,std::list<ChatMsg_t> &lstMsg)
{
	std::string ns;
	ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_CHAT_MSG_DB, groupid);
	LOG_PRINT(log_info, "getgroupmsg_t ns:%s.", ns.c_str());
	std::auto_ptr<mongo::DBClientCursor> cursor = CMongoDBConnection()->query(ns);

	while (cursor->more())
	{
		LOG_PRINT(log_info, "getgroupmsg_t msg ============ ns:%s.", ns.c_str());
		lstMsg.push_back(ChatMsg_t());
		parseGroupMsg(cursor->next(), lstMsg.back());
	}
	return true;
}
size_t CChatMongoMgr::qryGroupMsgList(uint32 userid, uint32 groupid, std::list<ChatMsg_t> &lstMsg, uint32 msgid /*= 0*/, int32 count /*= 0*/, bool forward /*= true*/,bool blobby /*= false*/)
{
	if (0 == groupid)
	{
		return 0;
	}

	if (0 == msgid && forward)
	{
		msgid = MAX_INT;
	}

	uint32 beg_msgid = 0;
	qryUserGroupMsgId(userid, groupid, beg_msgid, MONGO_CHAT_BEG_MSGID);

	LOG_PRINT(log_debug, "forward[%d],msgid[%d],beg_msgid[%d]",forward, msgid,beg_msgid);
	if (forward && msgid <= beg_msgid)
	{
		
		return 0;
	}
		

	if (!forward && beg_msgid > msgid)
	{
		msgid = beg_msgid;
	}

	long timeBegin=0,timeEnd=0;
	int now=time(NULL);
	std::string sDate;
	CChatCommon::unixTime2dateString(now,sDate);
	LOG_PRINT(log_debug, "unixTime2dateString,now=%d,sDate[%s].", now,sDate.c_str());

	CChatCommon::dateString2unixTime(sDate+"000000",timeBegin);
	CChatCommon::dateString2unixTime(sDate+"235959",timeEnd);
	LOG_PRINT(log_debug, "dateString2unixTime,timeBegin=%d,timeEnd=%d.", timeBegin,timeEnd);

	size_t total = 0;
	try
	{
		std::string ns;
		if(blobby)
		{
			ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_PCHAT_MSG_DB, groupid);
		}else
		{
			ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_CHAT_MSG_DB, groupid);
		}
		mongo::BSONObj query = BSON("query" << BSON(MONGO_KEY << BSON((forward ? "$lt" : "$gt") << msgid << "$gt" << beg_msgid) << 
			//"msgtime" << BSON("$gt" << ((int)timeBegin ?(int)timeBegin :0) << "$lt" << ((int)timeEnd ? (int)timeEnd : MAX_INT) ) <<
				MONGO_CHAT_MSG_TYPE << BSON("$ne" << MSGTYPE_RECALL) << MONGO_CHAT_RECALL << BSON("$ne" << true) <<
				"$or" << BSON_ARRAY(BSON(MONGO_CHAT_MSG_TYPE << BSON("$ne" << MSGTYPE_REMIND)) << BSON(MONGO_CHAT_DST_USERID << userid)))
				<< "orderby" << BSON(MONGO_KEY << (forward ? -1 : 1)));


		std::auto_ptr<mongo::DBClientCursor> cursor = CMongoDBConnection()->query(ns, mongo::Query(query), count);

		LOG_PRINT(log_debug, "1,qryGroupMsgList: ns[%s] query[%s] msgid[%u] count[%d].", ns.c_str(), query.toString().c_str(), msgid,count);

		if (cursor.get() && cursor->more())
		{
			while (cursor->more())
			{
				total++;

				if (!forward)
				{
					lstMsg.push_front(ChatMsg_t());
					parseGroupMsg(cursor->next(), lstMsg.front());
				}
				else
				{
					lstMsg.push_back(ChatMsg_t());
					parseGroupMsg(cursor->next(), lstMsg.back());
				}
			}

			LOG_PRINT(log_debug, "qryGroupMsgList: lstMsg.size[%d]  total[%d].",lstMsg.size(), total);
			if (1 == total || (total > 1 && (abs((long)(lstMsg.front().msgid() - lstMsg.back().msgid())) + 1) == total))
			{
				LOG_PRINT(log_debug, "qryGroupMsgList: lstMsg.front().msgid()[%d]  lstMsg.back().msgid()[%d].",lstMsg.front().msgid(), lstMsg.back().msgid());
			}
			else
			{
				total = 0;
				query = BSON("query" << BSON(MONGO_KEY << BSON("$lte" << lstMsg.front().msgid() << "$gte" << lstMsg.back().msgid())
						<< MONGO_CHAT_MSG_TYPE << BSON("$ne" << MSGTYPE_RECALL) << MONGO_CHAT_RECALL << BSON("$ne" << true) )
					<< "orderby" << BSON(MONGO_KEY << -1));
				cursor = CMongoDBConnection()->query(ns, mongo::Query(query), count);
				LOG_PRINT(log_debug, "2,qryGroupMsgList: ns[%s] query[%s] msgid[%u] count[%d].", ns.c_str(), query.toString().c_str(), msgid,count);

				lstMsg.clear();
				while (cursor.get() && cursor->more())
				{
					total++;
					lstMsg.push_back(ChatMsg_t());
					parseGroupMsg(cursor->next(), lstMsg.back());
				}
			}
		}
		//LOG_PRINT(log_debug, "ns[%s] query[%s] total[%u]", ns.c_str(), query.toString().c_str(), total);
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
		return 0;
	}

	return total;
}

//进入直播间时首先拉10条,客户端发过来的参数：forward=1,表示查询记录时对id desc   count=10
//当向上拉旧的聊天记录时，客户端发过来的参数：forward=1,  count=20, msgid=385,msgid表示id要小于385
//进入历史记录时querytime>0
size_t CChatMongoMgr::qryGroupMsgList_V2(uint32 userid, uint32 groupid, std::list<ChatMsg_t> &lstMsg,int64 querytime, uint32 msgid /*= 0*/, int32 count /*= 0*/, bool forward /*= true*/,bool blobby /*= false*/)
{
	if (0 == groupid)
	{
		return 0;
	}

	if (0 == msgid && forward)
	{
		msgid = MAX_INT;
	}

	uint32 beg_msgid = 0;
	qryUserGroupMsgId(userid, groupid, beg_msgid, MONGO_CHAT_BEG_MSGID);

	LOG_PRINT(log_debug, "forward[%d],msgid[%d],beg_msgid[%d]",forward, msgid,beg_msgid);
	if (forward && msgid <= beg_msgid)
	{

		return 0;
	}


	if (!forward && beg_msgid > msgid)
	{
		msgid = beg_msgid;
	}

	mongo::BSONObj query ;
	long timeBegin=0,timeEnd=0;
	//如果是查今天即count!=0并且今天非节假日,则只显示今天发布的内容
	if (count!=0 && querytime==0 && groupid>1000000000)//进入直播间查询聊天记录querytime须为零
	{
		if (!CChatCommon::isWeekEndAndHoliday())
		{
			int now=time(NULL);
			std::string sDate;
			CChatCommon::unixTime2dateString(now,sDate);
			LOG_PRINT(log_debug, "unixTime2dateString,now=%d,sDate[%s].", now,sDate.c_str());

			CChatCommon::dateString2unixTime(sDate+"000000",timeBegin);
			CChatCommon::dateString2unixTime(sDate+"235959",timeEnd);
			LOG_PRINT(log_debug, "dateString2unixTime,timeBegin=%d,timeEnd=%d.", timeBegin,timeEnd);
		}
	}
	else//如果是查历史
	{
		if (querytime>0)//querytime大于零
		{
			std::string sDate;
			CChatCommon::unixTime2dateString(querytime,sDate);
			LOG_PRINT(log_debug, "unixTime2dateString,querytime=%d,sDate[%s].", querytime,sDate.c_str());

			/*
			 * 20171128, wxc:
			 * 备注：需求限定只能一次查一天的记录，所以后台要根据前端传来的querytime值确定自然日的起点和终点，
			 * 而不是根据forward标志来加减一天的秒数取得时间起点和终点。
			 * 如果聊天记录出现日期断层，例如26至28号中，27号无聊天，可通过客户端直接指定日期来查询，而不采用
			 * 屏幕第一条或最后一条的聊天时间，但msgId仍然是取屏幕的第一条或最后一条记录的msgId作为基准进行
			 * 下次的查询。
			 */
#if 1
			CChatCommon::dateString2unixTime(sDate+"000000",timeBegin);
			CChatCommon::dateString2unixTime(sDate+"235959",timeEnd);
#else
			int64 offset = 24 * 60 * 60;
			if (forward)
			{
				timeBegin = querytime - offset;
				timeEnd = querytime;
			}
			else
			{
				timeBegin = querytime;
				timeEnd = querytime + offset;
			}
#endif
			LOG_PRINT(log_debug, "dateString2unixTime,timeBegin=%d,timeEnd=%d.", timeBegin,timeEnd);
		}
	}

	


	size_t total = 0;
	try
	{
		std::string ns;
		if(blobby)
		{
			ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_PCHAT_MSG_DB, groupid);
		}else
		{
			ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_CHAT_MSG_DB, groupid);
		}
		//mongo::BSONObj msgIdCondition = forward ? BSON("$lt" << msgid << "$gt" << beg_msgid) : BSON("$gt" << msgid); // TODO: use it in future
		query = BSON("query" << BSON(MONGO_KEY << BSON((forward ? "$lt" : "$gt") << msgid << "$gt" << beg_msgid) << 
			"msgtime" << BSON("$gt" << ((int)timeBegin ?(int)timeBegin :0) << "$lt" << ((int)timeEnd ? (int)timeEnd : MAX_INT) ) <<
			MONGO_CHAT_MSG_TYPE << BSON("$ne" << MSGTYPE_RECALL) << MONGO_CHAT_RECALL << BSON("$ne" << true) <<
			"$or" << BSON_ARRAY(BSON(MONGO_CHAT_MSG_TYPE << BSON("$ne" << MSGTYPE_CLOSE_LIVE_COURSE)) << BSON(MONGO_CHAT_DST_USERID << userid)))
			<< "orderby" << BSON(MONGO_KEY << (forward ? -1 : 1)));


		std::auto_ptr<mongo::DBClientCursor> cursor = CMongoDBConnection()->query(ns, mongo::Query(query), count);

		LOG_PRINT(log_debug, "1,qryGroupMsgList: ns[%s] query[%s] msgid[%u] count[%d].", ns.c_str(), query.toString().c_str(), msgid,count);

		if (cursor.get() && cursor->more())
		{
			while (cursor->more())
			{
				total++;

				if (!forward)
				{
					lstMsg.push_front(ChatMsg_t());
					parseGroupMsg(cursor->next(), lstMsg.front(), userid);
				}
				else
				{
					lstMsg.push_back(ChatMsg_t());
					parseGroupMsg(cursor->next(), lstMsg.back(), userid);
				}
			}

			LOG_PRINT(log_debug, "qryGroupMsgList: lstMsg.size[%d]  total[%d].",lstMsg.size(), total);
			if (1 == total || (total > 1 && (abs((long)(lstMsg.front().msgid() - lstMsg.back().msgid())) + 1) == total))//一条或者（比如查10条时,id相减刚好十条）
			{
				LOG_PRINT(log_debug, "qryGroupMsgList: lstMsg.front().msgid()[%d]  lstMsg.back().msgid()[%d].",lstMsg.front().msgid(), lstMsg.back().msgid());
			}
			else
			{
				total = 0;
				query = BSON("query" << BSON(MONGO_KEY << BSON("$lte" << lstMsg.front().msgid() << "$gte" << lstMsg.back().msgid())<<
					"msgtime" << BSON("$gt" << ((int)timeBegin ?(int)timeBegin :0) << "$lt" << ((int)timeEnd ? (int)timeEnd : MAX_INT) ) 
					<< MONGO_CHAT_MSG_TYPE << BSON("$ne" << MSGTYPE_RECALL) << MONGO_CHAT_RECALL << BSON("$ne" << true) )
					<< "orderby" << BSON(MONGO_KEY << -1));
				cursor = CMongoDBConnection()->query(ns, mongo::Query(query), count);
				LOG_PRINT(log_debug, "2,qryGroupMsgList: ns[%s] query[%s] msgid[%u] count[%d].", ns.c_str(), query.toString().c_str(), msgid,count);

				lstMsg.clear();
				while (cursor.get() && cursor->more())
				{
					total++;
					lstMsg.push_back(ChatMsg_t());
					parseGroupMsg(cursor->next(), lstMsg.back(), userid);
				}
			}
		}
		//LOG_PRINT(log_debug, "ns[%s] query[%s] total[%u]", ns.c_str(), query.toString().c_str(), total);
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
		return 0;
	}

	return total;
}



size_t CChatMongoMgr::qryUnreadGroupMsg(uint32 groupid, uint32 userid, std::list<ChatMsg_t> &lstMsg, int32 count /*= 0*/)
{
	if (0 == groupid)
	{
		return 0;
	}

	uint32 msgid = 0;
	qryUserGroupMsgId(userid, groupid, msgid);
	size_t total = 0, result = 0;
	try
	{
		mongo::BSONObj query = BSON("query" << BSON(MONGO_KEY << BSON("$gt" << msgid) /*<<
										"$or" << BSON_ARRAY(BSON(MONGO_CHAT_MSG_TYPE << BSON("$ne" << MSGTYPE_REMIND)) << BSON(MONGO_CHAT_DST_USERID << userid))*/)
								<< "orderby" << BSON(MONGO_KEY << -1));

		uint32 maxMsgId = 0;
		std::string ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_CHAT_MSG_DB, groupid);
		std::auto_ptr<mongo::DBClientCursor> cursor = CMongoDBConnection()->query(ns, mongo::Query(query), count);
		while (cursor.get() && cursor->more())
		{
			const mongo::BSONObj &obj = cursor->next();
			ChatMsg_t msg;
			if (parseGroupMsg(obj, msg))
			{
				if (maxMsgId < msg.msgid())
				{
					maxMsgId = msg.msgid();
				}
				if (msg.srcuser().userid() == userid)
				{
					total++;
				}
				lstMsg.push_back(msg);
				result++;
			}
		}
		if (maxMsgId && total == result)
		{
			updUserGroupMsgId(userid, groupid, maxMsgId);
		}
		LOG_PRINT(log_debug, "ns[%s] query[%s] mysend[%u] count[%u]", ns.c_str(), query.toString().c_str(), total, result);
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
		return result;
	}
	
	return result;
}

static int appendRedPacketExtraInfo(const int packetId, const int userId, Json::Value &jsonHolder, std::string &result)
{
	if (packetId <= 0)
	{
		LOG_PRINT(log_error, "invalid packetId: %d", packetId);
		return -1;
	}

	stRedPacketInfo fullInfo;

	if (CDBTools::getRedPacketBasicInfo(packetId, fullInfo) < 0)
	{
		LOG_PRINT(log_error, "failed to query red packet info, packetId = %d", packetId);
		return -1;
	}

	/*
	 * 追加所需内容
	 */
	jsonHolder["takeNum"] = fullInfo.takeNum;
	jsonHolder["hasTakenPacket"] = (userId <= 0) ? 0 : (0 != CDBSink().getRedPacketLogByTakeUserAndID(packetId, userId));
	LOG_PRINT(log_debug, "userid: %d, appended fields: takeNum[%d], hasTakenPacket[%d]",
		userId, jsonHolder["takeNum"].asInt(), jsonHolder["hasTakenPacket"].asInt());

	Json::FastWriter fast_writer;
	result = fast_writer.write(jsonHolder);

	return 0;
}

bool CChatMongoMgr::parseGroupMsg(const mongo::BSONObj &bsonMsg, ChatMsg_t &msg, const uint32 optionalUserId/* = 0*/)
{
	if (bsonMsg.hasElement(MONGO_KEY))
		msg.set_msgid(bsonMsg.getField(MONGO_KEY).numberInt());
	//LOG_PRINT(log_debug, "parseGroupMsg, msgid[%d].", bsonMsg.getField(MONGO_KEY).numberInt());
	
	if (bsonMsg.hasElement(MONGO_CHAT_SRC_USERID))
		msg.mutable_srcuser()->set_userid(bsonMsg.getField(MONGO_CHAT_SRC_USERID).numberInt());
	if (bsonMsg.hasElement(MONGO_CHAT_DST_USERID))
		msg.mutable_dstuser()->set_userid(bsonMsg.getField(MONGO_CHAT_DST_USERID).numberInt());
	if (bsonMsg.hasElement(MONGO_CHAT_MSG_TIME))
		msg.set_msgtime(bsonMsg.getField(MONGO_CHAT_MSG_TIME).numberInt());
	if (bsonMsg.hasElement(MONGO_CHAT_MSG_TYPE))
		msg.set_msgtype((e_MsgType)bsonMsg.getField(MONGO_CHAT_MSG_TYPE).numberInt());
	//LOG_PRINT(log_debug, "msgtype: %d", msg.msgtype());
	if (bsonMsg.hasElement(MONGO_CHAT_CONTENT))
	{
		std::string msgContent(bsonMsg.getField(MONGO_CHAT_CONTENT).String());

		if (MSGTYPE_REDPACK == msg.msgtype())
		{
			Json::Reader reader(Json::Features::strictMode());
			Json::Value root;

			if (reader.parse(msgContent, root))
			{
				appendRedPacketExtraInfo(root["packetID"].asInt(), optionalUserId, root, msgContent);
				LOG_PRINT(log_debug, "red packet content: %s", msgContent.c_str());
			}
			else
				LOG_PRINT(log_warning, "failed to parse json string: %s", msgContent.c_str());
		}

		msg.set_content(msgContent);
	}
	if (bsonMsg.hasElement(MONGO_CHAT_AT_LIST))
	{
		mongo::BSONElement ele = bsonMsg.getField(MONGO_CHAT_AT_LIST);
		mongo::BSONObjIterator i(ele.Obj());
		while (i.more()) {
			const mongo::BSONElement &e = i.next();
			msg.add_atlist(e.numberInt());
		}
	}
	if (bsonMsg.hasElement(MONGO_CHAT_CLIENT_MSGID))
	{
		msg.set_clientmsgid(bsonMsg.getField(MONGO_CHAT_CLIENT_MSGID).numberLong());
	}
	if (bsonMsg.hasElement(MONGO_CHAT_RECALL))
	{
		msg.set_recall(bsonMsg.getField(MONGO_CHAT_RECALL).Bool());
	}	
	if (bsonMsg.hasElement(MONGO_CHAT_MEDIA_LENGTH))
	{
		msg.set_medialength(bsonMsg.getField(MONGO_CHAT_MEDIA_LENGTH).numberInt());
	}
	if (bsonMsg.hasElement(MONGO_CHAT_MASTER_MSGID))
	{
		msg.set_mastermsgid(bsonMsg.getField(MONGO_CHAT_MASTER_MSGID).numberInt());
	}
	if (bsonMsg.hasElement(MONGO_CHAT_EXTEND_TYPE))
	{
		msg.set_extendtype(bsonMsg.getField(MONGO_CHAT_EXTEND_TYPE).numberInt());
	}
	return true;
}

bool CChatMongoMgr::updUserGroupMsgId(uint32 userid, uint32 groupid, uint32 msgid)
{
	if (0 == userid || 0 == groupid)
	{
		return false;
	}

	if (!CChatMongoMgr::checkMsgIDValid(CChatCommon::getGChatSeqName(groupid), msgid))
	{
		LOG_PRINT(log_error, "update user group msgID fail.userid:%u,groupid:%u,msgid:%u.", userid, groupid, msgid);
		return false;
	}

	try
	{
		mongo::BSONObj query = BSON( MONGO_KEY << userid );
		mongo::BSONObj modify = BSON( "$set" << BSON(MONGO_CHAT_READ_MSGID << msgid) );
		std::string ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_CHAT_STAT_DB, groupid);
		mongo::BSONObj result = CMongoDBConnection()->findAndModify(ns, query, modify, true);

		if (result.hasElement(MONGO_CHAT_READ_MSGID))
		{
			uint32 oldMsgId = result.getField("value").numberInt();
			if (oldMsgId > msgid)
			{
				LOG_PRINT(log_error, "old read msgid is bigger than new msgid.");
				mongo::BSONObj modify = BSON( "$set" << BSON(MONGO_CHAT_READ_MSGID << oldMsgId) );
				CMongoDBConnection()->update(ns, query, modify);
			}
		}
		return true;
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
	}

	return false;
}

bool CChatMongoMgr::updUserGroupChatTime(uint32 userid, uint32 groupid, uint32 chattime)
{
	if (0 == userid || 0 == groupid)
	{
		return false;
	}

	try
	{
		mongo::BSONObj query = BSON(MONGO_KEY << userid);
		mongo::BSONObj modify = BSON( "$set" << BSON(MONGO_CHAT_CHAT_TIME << chattime) );
		std::string ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_CHAT_STAT_DB, groupid);
		CMongoDBConnection()->update(ns, query, modify, true);
		return true;
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
	}

	return false;
}

bool CChatMongoMgr::qryMaxGroupMsgId(uint32 groupid, uint32 &msgid)
{
	if (!groupid)
	{
		return false;
	}

	try
	{
		mongo::BSONObj query = BSON("query" << BSON(MONGO_KEY << BSON("$gt" << 0)) << "orderby" << BSON(MONGO_KEY << -1));
		std::string ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_CHAT_MSG_DB, groupid);

		mongo::BSONObj obj = CMongoDBConnection()->findOne(ns, mongo::Query(query));
		if (!obj.hasElement(MONGO_KEY))
		{
			return false;
		}
		msgid = obj.getField(MONGO_KEY).numberInt();

		LOG_PRINT(log_debug, "ns[%s] query[%s] msgid[%u]", ns.c_str(), query.toString().c_str(), msgid);
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
		return false;
	}

	return true;
}

bool CChatMongoMgr::recallGroupMsg(uint32 groupid, uint32 msgid,bool blobby /*=false*/)
{
	if (!groupid || !msgid) 
	{
		return false;
	}

	try
	{
		mongo::BSONObj query = BSON(MONGO_KEY << msgid);
		mongo::BSONObj modify = BSON( "$set" << BSON(MONGO_CHAT_RECALL << true) );

		std::string ns;
		if(blobby)
		{
			ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_PCHAT_MSG_DB, groupid);
		}else
		{
			ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_CHAT_MSG_DB, groupid);
		}
		CMongoDBConnection()->update(ns, query, modify);
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
		return false;
	}

	return true;
}

bool CChatMongoMgr::qryUserGroupMsgId(uint32 userid, uint32 groupid, uint32 &msgid, const char *field/* = MONGO_CHAT_READ_MSGID*/, uint32 dstuid/* = 0*/)
{
	if (0 == userid || 0 == groupid || NULL == field)
	{
		return false;
	}

	if (!dstuid) //群聊
	{
		std::string ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_CHAT_STAT_DB, groupid);
		mongo::BSONObj query = BSON(MONGO_KEY << userid);
	}
	else
	{
		std::string ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_PCHAT_STAT_DB, userid);
		mongo::BSONObj query = BSON(MONGO_CHAT_GROUP << groupid << MONGO_CHAT_DST_USERID << dstuid); 
	}

	bool ret = false;
	try
	{
		mongo::BSONObj query = BSON(MONGO_KEY << userid);
		std::string ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_CHAT_STAT_DB, groupid);
		mongo::BSONObj obj = CMongoDBConnection()->findOne(ns, query);
		if (obj.hasElement(field))
		{
			msgid = obj[field].numberInt();
		}
		LOG_PRINT(log_debug, "qryUserGroupMsgId: ns[%s] query[%s] msgid[%u].", ns.c_str(), query.toString().c_str(), msgid);
		ret = true;
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
	}

	return ret;
}

bool CChatMongoMgr::updUserGroupBeginMsgId(uint32 userid, uint32 groupid, uint32 msgid)
{
	if (0 == userid || 0 == groupid)
	{
		return false;
	}

	try
	{
		mongo::BSONObj query = BSON(MONGO_KEY << userid);
		mongo::BSONObj modify = BSON("$set" << BSON(MONGO_CHAT_BEG_MSGID << msgid << MONGO_CHAT_READ_MSGID << msgid << MONGO_CHAT_CHAT_TIME << (int)0));
		std::string ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_CHAT_STAT_DB, groupid);
		CMongoDBConnection()->update(ns, query, modify, true);
		return true;
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
	}

	return false;
}

bool CChatMongoMgr::delUserMsgHistory(uint32 groupid, uint32 userid, uint32 dstuid/* = 0*/)
{
	if (!groupid || !userid)
	{
		LOG_PRINT(log_error, "invalid parameter: group[%u] user[%u]", groupid, userid);
		return false;
	}

	std::string seqName, ns;
	mongo::BSONObj query;
	if (!dstuid) //群聊
	{
		seqName = CChatCommon::getGChatSeqName(groupid);
		ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_CHAT_STAT_DB, groupid);
		query = BSON(MONGO_KEY << userid);
	}
	else
	{
		seqName = CChatCommon::getGPChatSeqName(groupid, userid, dstuid);
		ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_PCHAT_STAT_DB, userid);
		query = BSON(MONGO_CHAT_GROUP << groupid << MONGO_CHAT_DST_USERID << dstuid); 
	}
	
	try
	{

		uint32 curMsgID = 0;
		if (!getCurSequence(seqName, curMsgID) || !curMsgID)
		{
			return false;
		}

		mongo::BSONObj modify = BSON("$set" << BSON(MONGO_CHAT_BEG_MSGID << curMsgID));
		CMongoDBConnection()->update(ns, query, modify, true);
		LOG_PRINT(log_debug, "ns[%s] query[%s] modify[%s]", ns.c_str(), query.toString().c_str(), modify.toString().c_str());
		return true;
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
	}

	return false;
}

bool CChatMongoMgr::writeGroupPrivateMsg(const CMDGroupPrivateMsgReq &req)
{
	try
	{
		mongo::BSONObjBuilder rootObj;
		const ChatMsg_t &msg = req.msg();
		rootObj.append(MONGO_KEY, msg.msgid());
		rootObj.append(MONGO_CHAT_SRC_USERID, msg.srcuser().userid());
		rootObj.append(MONGO_CHAT_DST_USERID, msg.dstuser().userid());
		rootObj.append(MONGO_CHAT_MSG_TIME, msg.msgtime());
		rootObj.append(MONGO_CHAT_MSG_TYPE, msg.msgtype());
		rootObj.append(MONGO_CHAT_CONTENT, msg.content());
		rootObj.append(MONGO_CHAT_CLIENT_MSGID, (int64)msg.clientmsgid());

		std::string ns = CChatCommon::getFormatString("%s.%s", MONGO_GROUP_PCHAT_MSG_DB, CChatCommon::getGPChatSeqName(req.groupid(), msg.srcuser().userid(), msg.dstuser().userid()).c_str());
		CMongoDBConnection()->insert(ns, rootObj.obj());
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
		return false;
	}

	return true;
}

bool CChatMongoMgr::qryGroupPrivateMsg(uint32 groupid, uint32 srcuid, uint32 dstuid, uint32 msgid, ChatMsg_t &msg)
{
	try
	{
		mongo::BSONObj query = BSON("query" << BSON(MONGO_KEY << msgid));
		std::string ns = CChatCommon::getFormatString("%s.%s", MONGO_GROUP_PCHAT_MSG_DB, CChatCommon::getGPChatSeqName(groupid, srcuid, dstuid).c_str());
		std::auto_ptr<mongo::DBClientCursor> cursor = CMongoDBConnection()->query(ns, mongo::Query(query));
		if (cursor.get() && cursor->more())
		{
			const mongo::BSONObj &obj = cursor->next();
			return parsePrivateMsg(obj, msg);
		}
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
		return false;
	}

	return false;
}

size_t CChatMongoMgr::qryGroupPrivateMsgList(uint32 groupid, uint32 srcuid, uint32 dstuid, std::list<ChatMsg_t> &lstMsg, uint32 msgid /*= 0*/, int32 count /*= 0*/, bool forward /*= true*/)
{
	if (0 == srcuid || 0 == dstuid)
	{
		return 0;
	}

	uint32 beg_msgid = 0;
	qryUserGroupMsgId(srcuid, groupid, beg_msgid, MONGO_CHAT_BEG_MSGID, dstuid);
	if (forward && msgid <= beg_msgid)
		return 0;

	if (0 == msgid && forward)
	{
		msgid = MAX_INT;
	}

	if (!forward && beg_msgid > msgid)
	{
		msgid = beg_msgid;
	}

	size_t total = 0;
	try
	{
		std::string ns = CChatCommon::getFormatString("%s.%s", MONGO_GROUP_PCHAT_MSG_DB, CChatCommon::getGPChatSeqName(groupid, srcuid, dstuid).c_str());
		mongo::BSONObj query = BSON("query" << BSON(MONGO_KEY << BSON((forward ? "$lt" : "$gt") << msgid << "$gt" << beg_msgid) << 
									"$or" << BSON_ARRAY(BSON(MONGO_CHAT_MSG_TYPE << BSON("$ne" << MSGTYPE_REMIND)) << BSON(MONGO_CHAT_DST_USERID << srcuid))) 
								<< "orderby" << BSON(MONGO_KEY << (forward ? -1 : 1)));

		std::auto_ptr<mongo::DBClientCursor> cursor = CMongoDBConnection()->query(ns, query, count);
		while (cursor.get() && cursor->more())
		{
			while (cursor->more())
			{
				total++;

				if (!forward)
				{
					lstMsg.push_front(ChatMsg_t());
					parseGroupMsg(cursor->next(), lstMsg.front());
				}
				else
				{
					lstMsg.push_back(ChatMsg_t());
					parseGroupMsg(cursor->next(), lstMsg.back());
				}
			}

			if (1 == total || (total > 1 && (abs((long)(lstMsg.front().msgid() - lstMsg.back().msgid())) + 1) == total))
			{
			}
			else
			{
				total = 0;
				query = BSON("query" << BSON(MONGO_KEY << BSON((forward ? "$lt" : "$gt") << msgid))
					<< "orderby" << BSON(MONGO_KEY << (forward ? -1 : 1)));
				cursor = CMongoDBConnection()->query(ns, mongo::Query(query), count);
				lstMsg.clear();
				while (cursor.get() && cursor->more())
				{
					total++;
					lstMsg.push_back(ChatMsg_t());
					parseGroupMsg(cursor->next(), lstMsg.back());
				}
			}
		}
		LOG_PRINT(log_debug, "ns[%s] query[%s] total[%u]", ns.c_str(), query.toString().c_str(), total);
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
		return 0;
	}

	return total;
}

size_t CChatMongoMgr::qryUnreadGroupPrivateMsg(uint32 groupid, uint32 userid, uint32 dstuid, std::list<ChatMsg_t> &lstMsg, uint32 msgid /*= 0*/, int32 count /*= 0*/)
{
	if (0 == groupid || 0 == userid || 0 == dstuid)
	{
		return 0;
	}

	size_t total = 0, result = 0;
	try
	{
		mongo::BSONObj query = BSON("query" << BSON(MONGO_KEY << BSON("$gt" << msgid) /*<< 
									"$or" << BSON_ARRAY(BSON(MONGO_CHAT_MSG_TYPE << BSON("$ne" << MSGTYPE_REMIND)) << BSON(MONGO_CHAT_DST_USERID << userid))*/) 
								<< "orderby" << BSON(MONGO_KEY << -1));

		uint32 maxMsgId = 0;
		std::string ns = CChatCommon::getFormatString("%s.%s", MONGO_GROUP_PCHAT_MSG_DB, CChatCommon::getGPChatSeqName(groupid, userid, dstuid).c_str());
		std::auto_ptr<mongo::DBClientCursor> cursor = CMongoDBConnection()->query(ns, query, count);
		while (cursor.get() && cursor->more())
		{
			const mongo::BSONObj &obj = cursor->next();
			ChatMsg_t msg;
			if (parsePrivateMsg(obj, msg))
			{
				if (maxMsgId < msg.msgid())
					maxMsgId = msg.msgid();

				if (msg.srcuser().userid() == userid)
				{
					total++;
				}

				lstMsg.push_back(msg);
				result++;
			}
		}
		if (maxMsgId && total == result) //如果都是自己的聊天消息，则更新msgid即可
		{
			updUserGroupPMsgId(userid, groupid, dstuid, maxMsgId);
		}
		LOG_PRINT(log_debug, "ns[%s] query[%s] total[%u] count[%u]", ns.c_str(), query.toString().c_str(), total, result);
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
		return result;
	}

	return result;
}


bool CChatMongoMgr::updUserGroupPMsgId(uint32 userid, uint32 groupid, uint32 dstuid, uint32 msgid)
{
	if (0 == userid || 0 == groupid || 0 == dstuid)
	{
		return false;
	}

	if (!CChatMongoMgr::checkMsgIDValid(CChatCommon::getGPChatSeqName(groupid, userid, dstuid), msgid))
	{
		LOG_PRINT(log_error, "update user group pmsgID fail.userid:%u,groupid:%u,dstuid:%u,msgid:%u.", userid, groupid, dstuid, msgid);
		return false;
	}
	
	try
	{
		std::string ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_PCHAT_STAT_DB, userid);
		mongo::BSONObj query = BSON(MONGO_CHAT_GROUP << groupid << MONGO_CHAT_DST_USERID << dstuid); 

		mongo::BSONObj modify = BSON( "$set" << BSON(MONGO_CHAT_READ_MSGID << msgid) );
		CMongoDBConnection()->update(ns, query, modify, true);
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
		return false;
	}

	return true;
}

bool CChatMongoMgr::qryUserGroupPMsgIdList(uint32 userid, uint32 groupid, std::map<uint32, uint32> &mMsgId)
{
	if (0 == userid || 0 == groupid)
	{
		return false;
	}

	size_t total = 0, result = 0;
	try
	{
		mMsgId.clear();
		std::string ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_PCHAT_STAT_DB, userid);
		mongo::BSONObj query = BSON(MONGO_CHAT_GROUP << groupid);
		std::auto_ptr<mongo::DBClientCursor> cursor = CMongoDBConnection()->query(ns, query);
		while (cursor.get() && cursor->more())
		{
			total++;
			mongo::BSONObj obj = cursor->next();
			if (!obj.hasElement(MONGO_CHAT_DST_USERID) || !obj.hasElement(MONGO_CHAT_READ_MSGID))
			{
				continue;
			}

			//int32 to uint32
			mongo::BSONElement dstuid = obj[MONGO_CHAT_DST_USERID];
			mongo::BSONElement msgid = obj[MONGO_CHAT_READ_MSGID];
			mMsgId[dstuid.numberInt()] = msgid.numberInt();
			result++;
		}
		LOG_PRINT(log_debug, "ns[%s] query[%s] total[%u] count[%u]", ns.c_str(), query.toString().c_str(), total, result);
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
		return false;
	}

	return !mMsgId.empty();
}

bool CChatMongoMgr::recallGroupPrivateMsg(uint32 groupid, uint32 srcuid, uint32 dstuid, uint32 msgid)
{
	if (!groupid || !msgid) 
	{
		return false;
	}

	try
	{
		mongo::BSONObj query = BSON(MONGO_KEY << msgid);
		mongo::BSONObj modify = BSON( "$set" << BSON(MONGO_CHAT_RECALL << true) );

		std::string ns = CChatCommon::getFormatString("%s.%s", MONGO_GROUP_PCHAT_MSG_DB, CChatCommon::getGPChatSeqName(groupid, srcuid, dstuid).c_str());
		CMongoDBConnection()->update(ns, query, modify);
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
		return false;
	}

	return true;
}

bool CChatMongoMgr::addAssistMsg2MongoDB(StGPAssistMsg_t & oMsg, unsigned int & msgID)
{
	msgID = 0;
	std::string strSeqKey = CChatCommon::getGPAssistSeqName();
	uint32 msgid = 0;
	if (!CChatMongoMgr::getNextSequence(strSeqKey, msgid) || !msgid)
	{
		LOG_PRINT(log_error, "getNextSequence error,sequeue:%s.msgid:%u.", strSeqKey.c_str(), msgid);
		return false;
	}

	try
	{
		mongo::BSONObjBuilder rootObj;
		rootObj.append(MONGO_KEY,				msgid);
		rootObj.append(MONGO_ASSIST_USERID,		oMsg.userID);
		rootObj.append(MONGO_ASSIST_GROUPID,	oMsg.groupID);
		rootObj.append(MONGO_ASSIST_SVRTYPE,	(int)oMsg.msgSvrType);
		rootObj.append(MONGO_ASSIST_SVRSWITCH,	(int)oMsg.msgSvrSwitch);
		rootObj.append(MONGO_ASSIST_SVRLEVELUP,	(int)oMsg.msgSvrLevelUp);
		rootObj.append(MONGO_ASSIST_SVRLEVEL,	oMsg.msgSvrLevel);
		rootObj.append(MONGO_ASSIST_STATE,		(int)oMsg.msgState);
		rootObj.append(MONGO_ASSIST_MSGTIME,	oMsg.msgTime);
		rootObj.append(MONGO_ASSIST_URL,		oMsg.url);
		rootObj.append(MONGO_ASSIST_MSGTYPE,	oMsg.msgType);
		rootObj.append(MONGO_ASSIST_RELATE_MSGID,	oMsg.relate_msgid);
		rootObj.append(MONGO_ASSIST_USERCONTEXT,	oMsg.userContext);
		rootObj.append(MONGO_ASSIST_AUTHID,		oMsg.authID);
		rootObj.append(MONGO_ASSIST_INVEST_USERID, oMsg.investUserID);
		mongo::BSONObjBuilder extra;
		if (oMsg.packetid > 0)
			extra.append(MONGO_ASSIST_EXTRA_PACKETID, (int)oMsg.packetid);
		if (oMsg.balance > 0)
			extra.append(MONGO_ASSIST_EXTRA_BALANCE, (int)oMsg.balance);
		rootObj.append(MONGO_ASSIST_EXTRA_INFO, extra.obj());

		std::string ns = CChatCommon::getFormatString("%s.%s", MONGO_GROUP_ASSIST_DB, strSeqKey.c_str());
		CMongoDBConnection()->insert(ns, rootObj.obj());
		msgID = msgid;
		oMsg.msgId = msgid;
		return true;
	}
	catch (mongo::DBException & e)
	{
		LOG_PRINT(log_error, "DBException: %s.add assist msg error.msgid:%u,userid:%u,groupid:%u.", e.what(), msgid, oMsg.userID, oMsg.groupID);
		return false;
	}
}

bool CChatMongoMgr::updateAssistMsgState(unsigned int msgID, e_MsgState state)
{
	if (!msgID)
	{
		LOG_PRINT(log_error, "update assist msg state input error.");
		return false;
	}

	LOG_PRINT(log_debug, "update assist msg state input.msgID:%u,task state:%d.", msgID, (int)state);
	try
	{
		mongo::BSONObj query = BSON( MONGO_KEY << msgID );
		mongo::BSONObj modify = BSON( "$set" << BSON( MONGO_ASSIST_STATE << (int)state ) );
		
		std::string ns = CChatCommon::getFormatString("%s.%s", MONGO_GROUP_ASSIST_DB, CChatCommon::getGPAssistSeqName().c_str());
		CMongoDBConnection()->update(ns, query, modify);
	}
	catch (mongo::DBException & e)
	{
		LOG_PRINT(log_error, "DBException: %s.update assist msg state error.msgid:%u,task state:%d.", e.what(), msgID, (int)state);
		return false;
	}

	return true;
}

void CChatMongoMgr::qryAssistMsgState(const std::list<unsigned int > & msgIDLst, std::list<MsgState_t > & msgStateLst)
{
	try
	{
		mongo::BSONArrayBuilder arrBuild;
		std::list<unsigned int >::const_iterator iter_id = msgIDLst.begin();
		for (; iter_id != msgIDLst.end(); ++iter_id)
		{
			arrBuild.append(*iter_id);
		}

		mongo::BSONObj query = BSON("query" << BSON( MONGO_KEY << BSON( "$in" << arrBuild.arr() ) ) << "orderby" << BSON( MONGO_KEY << 1 ));
		std::string ns = CChatCommon::getFormatString("%s.%s", MONGO_GROUP_ASSIST_DB, CChatCommon::getGPAssistSeqName().c_str());
		std::auto_ptr<mongo::DBClientCursor> cursor = CMongoDBConnection()->query(ns, query);
		while (cursor.get() && cursor->more())
		{
			const mongo::BSONObj & bsonMsg = cursor->next();
			if (bsonMsg.hasElement(MONGO_KEY) && bsonMsg.hasElement(MONGO_ASSIST_STATE))
			{
				MsgState_t oState;
				oState.set_msgid(bsonMsg.getField(MONGO_KEY).numberInt());
				oState.set_state((e_MsgState)bsonMsg.getField(MONGO_ASSIST_STATE).numberInt());
				oState.set_msgtype(MSGTYPE_ASSIST);
				msgStateLst.push_back(oState);
			}
		}
	}
	catch (mongo::DBException & e)
	{
		LOG_PRINT(log_error, "DBException: %s.qurey AssistMsgState error.", e.what());
	}
}

bool CChatMongoMgr::addAssistMsg2User(unsigned int msgID, unsigned int userID)
{
	if (!msgID || !userID)
	{
		LOG_PRINT(log_error, "add assist msg to user input error.msgID:%u,userID:%u.", msgID, userID);
		return false;
	}

	LOG_PRINT(log_debug, "add assist msg input.msgID:%u,userID:%u.", msgID, userID);
	try
	{
		mongo::BSONObjBuilder rootObj;
		rootObj.append(MONGO_KEY, msgID);
		std::string ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_ASSIST_STAT_DB, userID);
		CMongoDBConnection()->insert(ns, rootObj.obj());
	}
	catch (mongo::DBException & e)
	{
		LOG_PRINT(log_error, "DBException: %s.add assist msg to user error.msgid:%u,userid:%u.", e.what(), msgID, userID);
		return false;
	}

	return true;
}

bool CChatMongoMgr::checkMsgIDValid(const std::string & strSeqName, uint32 msgID)
{
	uint32 curMsgID = 0;
	if (CChatMongoMgr::getCurSequence(strSeqName, curMsgID))
	{
		if (curMsgID == 0)
		{
			LOG_PRINT(log_error, "current %s msgID is 0.Not need to handle it.request msgID:%u,curMsgID:%u.", \
				strSeqName.c_str(), msgID, curMsgID);
			return false;
		}
		else if (msgID > curMsgID)
		{
			LOG_PRINT(log_error, "client send read msgID is bigger than current %s msgID.Not need to handle it.request msgID:%u,curMsgID:%u.", \
				strSeqName.c_str(), msgID, curMsgID);
			return false;
		}
		else
		{
//			LOG_PRINT(log_debug, "request msgID is correct.need to handle it.%s curMsgID:%u,request msgID:%u.", 
//				strSeqName.c_str(), curMsgID, msgID);
			return true;
		}
	}
	else
	{
		LOG_PRINT(log_error, "get current %s msgID fail.Not need to handle update msgid.request msgID:%u.", strSeqName.c_str(), msgID);
		return false;
	}
}
bool CChatMongoMgr::updateMsgContent(unsigned int msgid,unsigned int groupid,std::string content)
{
	try
	{
		bool bUpdateSet = false;
		if (msgid == 0)
		{
			bUpdateSet = true;
		}

		mongo::BSONObj query = BSON(MONGO_KEY << msgid);
		mongo::BSONObj modify = BSON("$set" << BSON( MONGO_CHAT_CONTENT << content ));
		std::string ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_CHAT_MSG_DB, groupid);
		CMongoDBConnection()->update(ns, query, modify, bUpdateSet);
		return true;
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.update  msgId Content error.msgID:%u,groupid:%u.", e.what(), msgid, groupid);
	}
	return false;
}

bool CChatMongoMgr::updateMsgExtendType(unsigned int msgid,unsigned int groupid,int  extendtype)
{
	try
	{
		bool bUpdateSet = false;
		if (msgid == 0)
		{
			bUpdateSet = true;
		}

		mongo::BSONObj query = BSON(MONGO_KEY << msgid);
		mongo::BSONObj modify = BSON("$set" << BSON( MONGO_CHAT_EXTEND_TYPE << extendtype ));
		std::string ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_CHAT_MSG_DB, groupid);
		CMongoDBConnection()->update(ns, query, modify, bUpdateSet);
		return true;
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.update  msgId  ExtendType  error.msgID:%u,groupid:%u.", e.what(), msgid, groupid);
	}
	return false;
}


bool CChatMongoMgr::updateUserAssistReadMsgId(unsigned int userID, unsigned int msgID)
{
	if (!msgID || !userID)
	{
		LOG_PRINT(log_error, "update user read assist msgId input error.msgID:%u,userID:%u.", msgID, userID);
		return false;
	}

	LOG_PRINT(log_debug, "update assist read msgId input.msgID:%u,userID:%u.", msgID, userID);

	if (!CChatMongoMgr::checkMsgIDValid(CChatCommon::getGPAssistSeqName(), msgID))
	{
		LOG_PRINT(log_error, "update user assist read msgID fail.userID:%u,request msgID:%u.", userID, msgID);
		return false;
	}

	uint32 msgIDDB = 0;
	qryUserReadAssistMsgId(userID, msgIDDB);
	if (msgIDDB >= msgID)
	{
		LOG_PRINT(log_warning, "user has read msgID in db is bigger than or equal input msgID.msgID in db:%u,msgID in request:%u.Not need to handle it.", msgIDDB, msgID);
		return false;
	}

	try
	{
		bool bUpdateSet = false;
		if (msgIDDB == 0)
		{
			bUpdateSet = true;
		}

		mongo::BSONObj query = BSON(MONGO_KEY << userID << MONGO_CHAT_READ_MSGID << BSON( "$lt" << msgID ) );
		mongo::BSONObj modify = BSON("$set" << BSON( MONGO_CHAT_READ_MSGID << msgID ));
		std::string ns = CChatCommon::getFormatString("%s.%u_read", MONGO_GROUP_ASSIST_STAT_DB, userID);
		CMongoDBConnection()->update(ns, query, modify, bUpdateSet);
		return true;
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.update user read assist msgId error.msgID:%u,userID:%u.", e.what(), msgID, userID);
	}

	return false;
}

bool CChatMongoMgr::qryUserReadAssistMsgId(uint32 userID, uint32 & msgid)
{
	msgid = 0;
	if (!userID)
	{
		LOG_PRINT(log_error, "query user read assist msgId input error.userID is 0.");
		return false;
	}

	try
	{
		mongo::BSONObj query = BSON( MONGO_KEY << userID );
		std::string ns = CChatCommon::getFormatString("%s.%u_read", MONGO_GROUP_ASSIST_STAT_DB, userID);
		mongo::BSONObj obj = CMongoDBConnection()->findOne(ns, query);
		if (!obj.hasElement(MONGO_CHAT_READ_MSGID))
		{
			LOG_PRINT(log_debug, "query user read assist msgId user has no %s.", MONGO_CHAT_READ_MSGID);
			return true;
		}

		msgid = obj.getField(MONGO_CHAT_READ_MSGID).numberInt();
		LOG_PRINT(log_debug, "query user read assist msgId: ns[%s] query[%s] msgid[%u].", ns.c_str(), query.toString().c_str(), msgid);
		return true;
	}
	catch (mongo::DBException & e)
	{
		LOG_PRINT(log_error, "DBException: %s.query user read assist msgId,userid:%u.", e.what(), userID);
	}

	return false;
}

size_t CChatMongoMgr::qryUserAssistMsgIdLst(uint32 userID, uint32 msgid, std::list<uint32> & msgIdLst, int32 count, bool forward)
{
	if (!userID)
	{
		LOG_PRINT(log_error, "query user assist msgId list input error.userID:%u.", userID);
		return 0;
	}

	LOG_PRINT(log_debug, "query user assist msgId list input.userID:%u.msgid:%u.count:%d,forward:%d.", userID, msgid, count, (int)forward);
	size_t total = 0;
	try
	{
		mongo::BSONObj query = BSON( "query" << BSON( MONGO_KEY << BSON( (forward ? "$lt" : "$gt") << msgid) ) << "orderby" << BSON( MONGO_KEY << -1 ) );

		std::string ns = CChatCommon::getFormatString("%s.%u", MONGO_GROUP_ASSIST_STAT_DB, userID);
		std::auto_ptr<mongo::DBClientCursor> cursor = CMongoDBConnection()->query(ns, query, count);
		while (cursor.get() && cursor->more())
		{
			const mongo::BSONObj & obj = cursor->next();
			if (obj.hasElement(MONGO_KEY))
			{
				msgIdLst.push_back(obj.getField(MONGO_KEY).numberInt());
				++total;
			}
		}

		LOG_PRINT(log_debug, "query user assist msgId list end.userID:%u,msgid:%u,count:%d,forward:%d,total:%u.", userID, msgid, count, (int)forward, total);
	}
	catch (mongo::DBException & e)
	{
		LOG_PRINT(log_error, "DBException: %s.query user assist msgId list,userID:%u,msgid:%u,total:%u.", e.what(), userID, msgid, total);
	}

	return total;
}

size_t CChatMongoMgr::qryUnreadAssistMsgLst(unsigned int userID, std::list<StGPAssistMsg_t > & unreadMsgLst, int32 count /*= 0*/)
{
	if (!userID)
	{
		LOG_PRINT(log_error, "query unread assist msg list input error.userID:%u.", userID);
		return 0;
	}

	LOG_PRINT(log_debug, "query unread assist msg list input.userID:%u.request count:%d.", userID, count);
	uint32 msgID = 0;
	if (!qryUserReadAssistMsgId(userID, msgID))
	{
		return 0;
	}

	std::list<uint32 > msgIdLst;
	if (!qryUserAssistMsgIdLst(userID, msgID, msgIdLst, count, false))
	{
		return 0;
	}

	size_t batch_num = 50;
	std::list<mongo::BSONArray > inConditionLst;
	int i = 0;
	mongo::BSONArrayBuilder * pArrBuild = new mongo::BSONArrayBuilder();
	std::list<uint32>::iterator iter_lst = msgIdLst.begin();
	for (; iter_lst != msgIdLst.end(); ++iter_lst, ++i)
	{
		if (i < batch_num)
		{
			pArrBuild->append(*iter_lst);
		}
		else
		{
			if (pArrBuild->isArray())
			{
				inConditionLst.push_back(pArrBuild->arr());
			}
			
			i = 0;
			DELETE_POINT(pArrBuild);
			pArrBuild = new mongo::BSONArrayBuilder();
			pArrBuild->append(*iter_lst);
		}
	}
	if (pArrBuild && pArrBuild->isArray())
	{
		inConditionLst.push_back(pArrBuild->arr());
	}
	DELETE_POINT(pArrBuild);

	size_t result = 0;
	try
	{
		std::string ns = CChatCommon::getFormatString("%s.%s", MONGO_GROUP_ASSIST_DB, CChatCommon::getGPAssistSeqName().c_str());
		std::list<mongo::BSONArray >::iterator iter_cond = inConditionLst.begin();
		for (; iter_cond != inConditionLst.end(); ++iter_cond)
		{
			mongo::BSONObj query = BSON("query" << BSON( MONGO_KEY << BSON( "$in" << *iter_cond ) ) << "orderby" << BSON(MONGO_KEY << -1));
			std::auto_ptr<mongo::DBClientCursor> cursor = CMongoDBConnection()->query(ns, query);
			while (cursor.get() && cursor->more())
			{
				const mongo::BSONObj & obj = cursor->next();
				StGPAssistMsg_t msg;
				if (parseAssistMsg(obj, msg))
				{
					unreadMsgLst.push_back(msg);
					++result;
				}
			}
			LOG_PRINT(log_debug, "query unread assist msg list ns[%s] query[%s] userID:%u request count[%d] result:%u.", ns.c_str(), query.toString().c_str(), userID, count, result);
		}
		
		LOG_PRINT(log_debug, "query unread assist msg list result:userID:%u.request count:%d.result:%u.", userID, count, result);	
	}
	catch (mongo::DBException & e)
	{
		LOG_PRINT(log_error, "DBException: %s.query unread assist msg lst input.userID:%u.request count:%d.", e.what(), userID, count);
		return result;
	}

	return result;
}

size_t CChatMongoMgr::qryAssistMsgList(uint32 userID, std::list<StGPAssistMsg_t > & lstMsg, uint32 msgID /*= 0*/, int32 count /*= 0*/, bool forward /*= true*/)
{
	if (!userID)
	{
		LOG_PRINT(log_error, "query AssistMsg history list input error.userID:%u.", userID);
		return 0;
	}

	if (0 == msgID && forward)
	{
		msgID = MAX_INT;
	}

	std::list<uint32 > msgIdLst;
	if (!qryUserAssistMsgIdLst(userID, msgID, msgIdLst, count, forward))
	{
		LOG_PRINT(log_warning, "userID:%u has no unread assist msg list.", userID);
		return 0;
	}

	mongo::BSONArrayBuilder oArrBuild;
	std::list<uint32>::iterator iter_lst = msgIdLst.begin();
	for (; iter_lst != msgIdLst.end(); ++iter_lst)
	{
		oArrBuild.append(*iter_lst);
	}

	size_t result = 0;
	try
	{
		mongo::BSONObj query = BSON("query" << BSON( MONGO_KEY << BSON( "$in" << oArrBuild.arr() ) ) << "orderby" << BSON(MONGO_KEY << -1));
		std::string ns = CChatCommon::getFormatString("%s.%s", MONGO_GROUP_ASSIST_DB, CChatCommon::getGPAssistSeqName().c_str());
		std::auto_ptr<mongo::DBClientCursor> cursor = CMongoDBConnection()->query(ns, query);
		while (cursor.get() && cursor->more())
		{
			const mongo::BSONObj & obj = cursor->next();
			StGPAssistMsg_t msg;
			if (parseAssistMsg(obj, msg))
			{
				lstMsg.push_back(msg);
				++result;
			}
		}
		LOG_PRINT(log_debug, "query AssistMsg history list ns[%s] query[%s] userID:%u request count[%d] result:%u.", ns.c_str(), query.toString().c_str(), userID, count, result);
	}
	catch (mongo::DBException & e)
	{
		LOG_PRINT(log_error, "DBException: %s.query AssistMsg history list input.userID:%u.request count:%d.", e.what(), userID, count);
		return result;
	}
	return result;
}

bool CChatMongoMgr::qryAssistMsgInfo(uint32 msgID, StGPAssistMsg_t & oMsg)
{
	mongo::BSONArrayBuilder oArrBuild;
	oArrBuild.append(msgID);
	try
	{
		mongo::BSONObj query = BSON("query" << BSON( MONGO_KEY << BSON( "$in" << oArrBuild.arr() ) ) );
		std::string ns = CChatCommon::getFormatString("%s.%s", MONGO_GROUP_ASSIST_DB, CChatCommon::getGPAssistSeqName().c_str());
		std::auto_ptr<mongo::DBClientCursor> cursor = CMongoDBConnection()->query(ns, query);
		while (cursor.get() && cursor->more())
		{
			const mongo::BSONObj & obj = cursor->next();
			if (obj.hasElement(MONGO_KEY) && obj.getField(MONGO_KEY).numberInt() == msgID)
			{
				if (parseAssistMsg(obj, oMsg))
				{
					return true;
				}
			}
		}
	}
	catch (mongo::DBException & e)
	{
		LOG_PRINT(log_error, "DBException: %s.query AssistMsg input.msgID:%u.", e.what(), msgID);
	}
	return false;
}

bool CChatMongoMgr::parseAssistMsg(const mongo::BSONObj & bsonMsg, StGPAssistMsg_t & msg)
{
	if (!bsonMsg.hasElement(MONGO_KEY) ||
		!bsonMsg.hasElement(MONGO_ASSIST_USERID) ||
		!bsonMsg.hasElement(MONGO_ASSIST_GROUPID) ||
		!bsonMsg.hasElement(MONGO_ASSIST_SVRTYPE) ||
		!bsonMsg.hasElement(MONGO_ASSIST_SVRSWITCH) ||
		!bsonMsg.hasElement(MONGO_ASSIST_SVRLEVEL) ||
		!bsonMsg.hasElement(MONGO_ASSIST_SVRLEVELUP) ||
		!bsonMsg.hasElement(MONGO_ASSIST_STATE) ||
		!bsonMsg.hasElement(MONGO_ASSIST_MSGTIME) ||
		!bsonMsg.hasElement(MONGO_ASSIST_URL) ||
		!bsonMsg.hasElement(MONGO_ASSIST_MSGTYPE))
	{
		LOG_PRINT(log_warning, "invalid assist msg record: BSON{%s}", bsonMsg.toString().c_str());
		return false;
	}

	msg.msgId = bsonMsg.getField(MONGO_KEY).numberInt();
	msg.userID = bsonMsg.getField(MONGO_ASSIST_USERID).numberInt();
	msg.groupID = bsonMsg.getField(MONGO_ASSIST_GROUPID).numberInt();
	msg.msgSvrType = (e_MsgSvrType)bsonMsg.getField(MONGO_ASSIST_SVRTYPE).numberInt();
	msg.msgSvrSwitch = (bool)bsonMsg.getField(MONGO_ASSIST_SVRSWITCH).numberInt();
	msg.msgSvrLevel = bsonMsg.getField(MONGO_ASSIST_SVRLEVEL).numberInt();
	msg.msgSvrLevelUp = (bool)bsonMsg.getField(MONGO_ASSIST_SVRLEVELUP).numberInt();
	msg.msgState = (e_MsgState)bsonMsg.getField(MONGO_ASSIST_STATE).numberInt();
	msg.msgTime = bsonMsg.getField(MONGO_ASSIST_MSGTIME).numberInt();
	msg.url = bsonMsg.getField(MONGO_ASSIST_URL).str();
	msg.msgType = bsonMsg.getField(MONGO_ASSIST_MSGTYPE).numberInt();

	if (bsonMsg.hasElement(MONGO_ASSIST_USERCONTEXT))
	{
		msg.userContext = bsonMsg.getField(MONGO_ASSIST_USERCONTEXT).str();
	}

	if (bsonMsg.hasElement(MONGO_ASSIST_AUTHID))
	{
		msg.authID = bsonMsg.getField(MONGO_ASSIST_AUTHID).numberInt();
	}

	if (bsonMsg.hasElement(MONGO_ASSIST_INVEST_USERID))
	{
		msg.investUserID = bsonMsg.getField(MONGO_ASSIST_INVEST_USERID).numberInt();
	}

	if (bsonMsg.hasElement(MONGO_ASSIST_EXTRA_INFO))
	{
		mongo::BSONObj ele = bsonMsg.getField(MONGO_ASSIST_EXTRA_INFO).Obj();
		if (ele.hasElement(MONGO_ASSIST_EXTRA_PACKETID))
		{
			msg.packetid = ele.getField(MONGO_ASSIST_EXTRA_PACKETID).numberInt();
		}
		if (ele.hasElement(MONGO_ASSIST_EXTRA_BALANCE))
		{
			msg.balance = ele.getField(MONGO_ASSIST_EXTRA_BALANCE).numberInt();
		}
	}

	return true;
}
