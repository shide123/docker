/*
 * DBmongoConnection.cpp
 *
 *  Created on: Oct 12, 2016
 *      Author: boa
 */

#include "DBmongoConnection.h"
#include "CLogThread.h"
#include "utils.h"
#include "macro_define.h"
size_t CMongoDBConnectionPool::m_uSize = 0;
boost::mutex CMongoDBConnectionPool::m_mutex;
std::list<mongodbconnection_ptr> CMongoDBConnectionPool::m_lstPool;

bool CMongoDBConnectionPool::init(Config &config)
{
	if (!config.load())
	{
		LOG_PRINT(log_error, "load mongo config failed,init failed.\n");
		return false;
	}
	bool bUseReplicaSet = config.getInt("mongo", "UseReplicaSet", 0) != 0;
	int poolsize = config.getInt("mongo", "poolsize", 5);
	LOG_PRINT(log_info, "UseReplicaSet flag: %d.", bUseReplicaSet);
	if (bUseReplicaSet)
	{
		std::string strUrl = config.getString("mongo", "url");
		LOG_PRINT(log_info, "Connect to Mongodb, url[%s]", strUrl.c_str());
		return init(strUrl, poolsize);
	}
	else
	{
		std::string ip = config.getString("mongo", "ip");
		int port = config.getInt("mongo", "port");
		std::string user = config.getString("mongo", "user");
		std::string password = config.getString("mongo", "password");
		return init(ip, port, user, password, poolsize);
	}
}

bool CMongoDBConnectionPool::init(std::string strUrl, size_t poolsize /*= 5*/)
{
	LOG_PRINT(log_info, "Connect to Mongodb, url[%s]", strUrl.c_str());
	resetPoolSize(poolsize);

	bool ret = false;
	try {
		mongo::client::initialize();
		do
		{
			int count = 0;
			std::string errmsg;
			mongo::ConnectionString cs = mongo::ConnectionString::parse(strUrl.c_str(), errmsg);
			if (!cs.isValid())
			{
				LOG_PRINT(log_error, "mongo client parse failed! url[%s] errmsg[%s]", strUrl.c_str(), errmsg.c_str());
				break;
			}

			for (int i = 0; i < poolsize; i++)
			{
				mongo::DBClientConnection *m_pMongoClient = (mongo::DBClientConnection*)cs.connect(errmsg);

				if (!m_pMongoClient)
				{
					LOG_PRINT(log_error, "mongo client connect failed! url[%s] errmsg[%s]", strUrl.c_str(), errmsg.c_str());
					break;
				}

				m_lstPool.push_back(mongodbconnection_ptr(m_pMongoClient));
				count++;
			}
			if (count == poolsize)
			{
				ret = true;
				m_uSize = count;
			}
		} while (0);
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "mongo client connect failed!(%s), url[%s]", e.what(), strUrl.c_str());
	}
	catch (...)
	{
		LOG_PRINT(log_error, "mongo client connect failed with unknown execption! url[%s].", strUrl.c_str());
	}

	return ret;
}

bool CMongoDBConnectionPool::init(const std::string ip, int port, const std::string user, const std::string password, size_t poolsize /*= 5*/)
{
	LOG_PRINT(log_info, "Connect to Mongodb, server[%s:%d]", ip.c_str(), port);
	resetPoolSize(poolsize);

	bool ret = false;
	try
	{
		mongo::client::initialize();
		int count = 0;
		for (int i = 0; i < poolsize; i++)
		{
			mongodbconnection_ptr conn(new mongo::DBClientConnection(true));
			char hostinfo[128] = { 0 };
			snprintf(hostinfo, 127, "%s:%d", ip.c_str(), port);

			std::string errmsg;
			conn->connect(hostinfo, errmsg);
			//conn->auth(dbname, user, password, errmsg);

			m_lstPool.push_back(conn);
			count++;
		}
		if (count == poolsize)
		{
			ret = true;
			m_uSize = count;
		}
	}
	catch (mongo::DBException & e)
	{
		LOG_PRINT(log_error, "mongo client connect failed!(Error msg:%s,mongo:%s:%d)\n", e.what(), ip.c_str(), port);
	}

	return ret;
}

void CMongoDBConnectionPool::resetPoolSize(size_t &poolsize)
{
	if (poolsize <= 0) poolsize = 5;
	if (poolsize > 50) poolsize = 50;
}

mongodbconnection_ptr CMongoDBConnectionPool::get()
{
	mongodbconnection_ptr conn;
	while (m_uSize > 0)
	{
		{
			boost::mutex::scoped_lock lock(m_mutex);
			if (m_lstPool.begin() != m_lstPool.end())
			{
				conn = m_lstPool.front();
				m_lstPool.pop_front();
				break;
			}
		}

		usleep(5000);
	}

	return conn;
}

void CMongoDBConnectionPool::put(mongodbconnection_ptr conn)
{
	if (conn)
	{
		boost::mutex::scoped_lock lock(m_mutex);
		m_lstPool.push_back(conn);
	}
}


Config DBmongoConnection::m_config;
mongo::DBClientConnection * DBmongoConnection::m_pMongoClient = NULL;

DBmongoConnection::DBmongoConnection()
{
}

DBmongoConnection::~DBmongoConnection()
{
	if (m_pMongoClient) 
	{
		delete m_pMongoClient;
		m_pMongoClient = NULL;
	}
}

mongo::DBClientConnection * DBmongoConnection::getInstance()
{
	if (m_pMongoClient)
	{
		return m_pMongoClient;
	}
	else
	{
		mongo::client::GlobalInstance instance;
		mongo::OID::gen();

		//first initial.
		int line;
		if(!m_config.load(line))
		{
			LOG_PRINT(log_error, "load mongo config failed,init failed.\n");
			return NULL;
		}

		bool bUseReplicaSet = m_config.getInt("mongo", "UseReplicaSet", 0) != 0;
		LOG_PRINT(log_info, "UseReplicaSet flag: %d.", bUseReplicaSet);
		//use mongodb replica set
		if (bUseReplicaSet)
		{
			std::string errmsg;
			std::string strUrl = m_config.getString("mongo", "url");

			LOG_PRINT(log_info, "Connect to Mongodb, url[%s]", strUrl.c_str());

			try {
				do 
				{
					mongo::ConnectionString cs = mongo::ConnectionString::parse( strUrl.c_str(), errmsg );
					if(!cs.isValid())
					{
						LOG_PRINT(log_error, "mongo client parse failed! url[%s] errmsg[%s]", strUrl.c_str(), errmsg.c_str());
						break;
					}

					m_pMongoClient = (mongo::DBClientConnection* )cs.connect( errmsg );
					
					if (!m_pMongoClient)
					{
						LOG_PRINT(log_error, "mongo client connect failed! url[%s] errmsg[%s]", strUrl.c_str(), errmsg.c_str());
					}
				} while (0);
			}
			catch(mongo::DBException &e)
			{
				LOG_PRINT(log_error, "mongo client connect failed!(%s), url[%s]", e.what(), strUrl.c_str());
			}
			catch(...)
			{
				LOG_PRINT(log_error, "mongo client connect failed with unknown execption! url[%s].", strUrl.c_str());
			}
		}
		else
		{
			std::string ip = m_config.getString("mongo", "ip");
			int port = m_config.getInt("mongo", "port");
			std::string user = m_config.getString("mongo", "user");
			std::string password = m_config.getString("mongo", "password");

			LOG_PRINT(log_info, "Connect to Mongodb, server[%s:%d]", ip.c_str(), port);
			m_pMongoClient = new mongo::DBClientConnection(true);
			try 
			{
				char hostinfo[128] = {0};
				snprintf(hostinfo, 127, "%s:%d", ip.c_str(), port);

				std::string errmsg;
				m_pMongoClient->connect(hostinfo, errmsg);
				//m_pMongoClient->auth(dbname, user, password, errmsg);
			}
			catch(mongo::DBException & e)  
			{
				LOG_PRINT(log_error, "mongo client connect failed!(Error msg:%s,mongo:%s:%d)\n", e.what(), ip.c_str(), port);
				if (m_pMongoClient)
				{
					delete m_pMongoClient;
					m_pMongoClient = NULL;
				}
			}
		}
	}

	return m_pMongoClient;
}

int DBmongoConnection::updateRoomCount(int vcbid,int date,int count)
{
	try
	{
		mongo::BSONObj query = BSON("date" << date);
		mongo::BSONObj modify = BSON( "$set" << BSON("visitorcount" << count) );
		std::string ns = stringFormat("%s.%u", "online_room_stat", vcbid);
		CMongoDBConnection()->update(ns, query, modify, true);
		return 1;
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
	}

	return 0;
}
int DBmongoConnection::updateUserOnlineTime(int userid,int groupid,int date,int minutes)
{
	try
	{
		int m = 0;
		queryUserOnlineTime(userid,groupid,date,m);
		LOG_PRINT(log_error, "room_tag m[%d].", m);
		minutes += m;
		mongo::BSONObj query = BSON("date" << date << "userid" << userid);
		mongo::BSONObj modify = BSON( "$set" << BSON("minutes" << minutes) );
		std::string ns = stringFormat("%s.%u", "online_user_stat", groupid);
		CMongoDBConnection()->update(ns, query, modify, true);
		return 1;
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
	}

	return 0;
}
int DBmongoConnection::queryUserOnlineTime(int userid,int groupid,int date,int& minutes)
{
	try
	{
		mongo::BSONObj query = BSON("date" << date << "userid" << userid);
		std::string ns = stringFormat("%s.%u", "online_user_stat", groupid);
		mongo::BSONObj result = CMongoDBConnection()->findOne(ns.c_str(), query);
		if (result.hasElement("minutes"))
		{
			const mongo::BSONElement &bel = result.getField("minutes");
			minutes = (uint32)bel.numberInt();
		}
		else
		{
			LOG_PRINT(log_error, "Invalid mongo key[value] of ns[%s]", ns.c_str());
			return false;
		}
		return 1;
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
	}

	return 0;
}
int DBmongoConnection::queryYesterdayMaxUserCount(std::vector<string> vroomids, std::map<int,int>& mguc)
{
	try
	{
		int maxcount = 0;
		for(int i = 0 ; i < vroomids.size() ; i++)
		{
			int roomid = atoi(vroomids[i].c_str());
			if(roomid < BROADCAST_ROOM_BEGIN_NUM) continue;
			string date = getTime("%Y%m%d",-24);
			int mindate = atoi(date.c_str())*100 + 0;
			int maxdate = atoi(date.c_str())*100 + 23;
			mongo::BSONObj query = BSON("query" << BSON("date" << BSON("$lte" << maxdate << "$gte" << mindate)));
			std::string ns = stringFormat("%s.%u", "online_room_stat", roomid);
			std::auto_ptr<mongo::DBClientCursor> cursor = CMongoDBConnection()->query(ns.c_str(), mongo::Query(query));
			while (cursor.get() && cursor->more())
			{
				const mongo::BSONObj &bsonMsg = cursor->next();
				if (bsonMsg.hasElement("visitorcount"))
				{
					int count = bsonMsg.getField("visitorcount").numberInt();
					if(maxcount < count)
					{
						maxcount = count;
					}
				}
			}
			mguc[roomid] = maxcount;
		}
		return 1;
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
	}

	return 0;
}
int DBmongoConnection::updateRoomMaxCount(const std::string& date,const std::map<int,int>& mguc)
{
	try
	{
		for(std::map<int,int>::const_iterator it = mguc.begin() ; it != mguc.end() ; it++)
		{
			mongo::BSONObj query = BSON("date" << date << "type" << 0);
			mongo::BSONObj modify = BSON( "$set" << BSON("value" << it->second) );
			std::string ns = stringFormat("%s.%u", "online_room_info_stat", it->first);
			CMongoDBConnection()->update(ns, query, modify, true);
		}
		return 1;
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
	}

	return 0;
}
int DBmongoConnection::queryYesterdayAvgOnline(std::vector<std::string> vroomids, std::map<int,int>& avg)
{
	try
	{
		int allmin = 0;
		int count = 1;
		for(int i = 0 ; i < vroomids.size() ; i++)
		{
			int roomid = atoi(vroomids[i].c_str());
			if(roomid < BROADCAST_ROOM_BEGIN_NUM) continue;
			string date = getTime("%Y%m%d",-24);
			mongo::BSONObj query = BSON("query" << BSON("date" << date));
			std::string ns = stringFormat("%s.%u", "online_user_stat", roomid);
			std::auto_ptr<mongo::DBClientCursor> cursor = CMongoDBConnection()->query(ns.c_str(), mongo::Query(query));
			while (cursor.get() && cursor->more())
			{
				const mongo::BSONObj &bsonMsg = cursor->next();
				if (bsonMsg.hasElement("minutes"))
				{
					allmin += bsonMsg.getField("minutes").numberInt();
					count++;
				}
			}
			avg[roomid] = allmin / count;
		}
		return 1;
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
	}

	return 0;
}
int DBmongoConnection::updateAvgOnline(const std::string& date,const std::map<int,int>& avg)
{
	try
	{
		for(std::map<int,int>::const_iterator it = avg.begin() ; it != avg.end() ; it++)
		{
			mongo::BSONObj query = BSON("date" << date << "type" << 1);
			mongo::BSONObj modify = BSON( "$set" << BSON("value" << it->second) );
			std::string ns = stringFormat("%s.%u", "online_room_info_stat", it->first);
			CMongoDBConnection()->update(ns, query, modify, true);
		}
		return 1;
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
	}

	return 0;
}
int DBmongoConnection::queryYesterdayAvgCount(std::vector<std::string> vroomids, std::map<int,int>& avg)
{
	try
	{
		int allcount = 0;
		for(int i = 0 ; i < vroomids.size() ; i++)
		{
			int roomid = atoi(vroomids[i].c_str());
			if(roomid < BROADCAST_ROOM_BEGIN_NUM) continue;
			string date = getTime("%Y%m%d",-24);
			int mindate = atoi(date.c_str())*100 + 0;
			int maxdate = atoi(date.c_str())*100 + 23;
			mongo::BSONObj query = BSON("query" << BSON("date" << BSON("$lte" << maxdate << "$gte" << mindate)));
			std::string ns = stringFormat("%s.%u", "online_room_stat", roomid);
			std::auto_ptr<mongo::DBClientCursor> cursor = CMongoDBConnection()->query(ns.c_str(), mongo::Query(query));
			while (cursor.get() && cursor->more())
			{
				const mongo::BSONObj &bsonMsg = cursor->next();
				if (bsonMsg.hasElement("visitorcount"))
				{
					allcount += bsonMsg.getField("visitorcount").numberInt();
				}
			}
			avg[roomid] = allcount/24;
		}
		return 1;
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
	}

	return 0;
}
int DBmongoConnection::updateAvgCount(const std::string& date, const std::map<int,int>& avg)
{
	try
	{
		for(std::map<int,int>::const_iterator it = avg.begin() ; it != avg.end() ; it++)
		{
			mongo::BSONObj query = BSON("date" << date << "type" << 2);
			mongo::BSONObj modify = BSON( "$set" << BSON("value" << it->second) );
			std::string ns = stringFormat("%s.%u", "online_room_info_stat", it->first);
			CMongoDBConnection()->update(ns, query, modify, true);
		}
		return 1;
	}
	catch (mongo::DBException &e)
	{
		LOG_PRINT(log_error, "DBException: %s.", e.what());
	}

	return 0;
}
