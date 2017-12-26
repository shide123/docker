/*
 * DBmongoConnection.h
 *
 *  Created on: Oct 12, 2016
 *      Author: boa
 */

#ifndef __DB_MONGO_CONNECTION_H__
#define __DB_MONGO_CONNECTION_H__

#include "mongo/client/dbclient.h"
#include <mongo/bson/bson.h>
#include <mongo/bson/bsonobj.h>
#include <mongo/client/init.h>
#include "Config.h"

typedef boost::shared_ptr<mongo::DBClientConnection> mongodbconnection_ptr;
class CMongoDBConnectionPool
{
public:
	static bool init(Config &config);
	static bool init(std::string url, size_t poolsize = 5);
	static bool init(const std::string ip, int port, const std::string user, const std::string password, size_t poolsize = 5);

	class COneConnection
	{
	public:
		COneConnection(){ m_conn = CMongoDBConnectionPool::get(); }
		~COneConnection() { CMongoDBConnectionPool::put(m_conn); }

		mongodbconnection_ptr operator ->()
		{
			return m_conn;
		}

		operator bool() const
		{
			return NULL != m_conn.get();
		}

		bool operator !() const
		{
			return NULL == m_conn.get();
		}

	private:
		mongodbconnection_ptr m_conn;
	};

private:
	static void resetPoolSize(size_t &poolsize);
	static mongodbconnection_ptr get();
	static void put(mongodbconnection_ptr conn);

private:
	static size_t m_uSize;
	static boost::mutex m_mutex;
	static std::list<mongodbconnection_ptr> m_lstPool;
};

typedef CMongoDBConnectionPool::COneConnection CMongoDBConnection;

class DBmongoConnection 
{
public:

	DBmongoConnection();

	virtual ~DBmongoConnection();

	static mongo::DBClientConnection * getInstance();

	static Config m_config;

	static int updateRoomCount(int vcbid,int hour,int count);
	static int updateUserOnlineTime(int userid,int groupid,int date,int minutes);
	static int queryUserOnlineTime(int userid,int groupid,int date,int& minutes);
	static int queryYesterdayMaxUserCount(std::vector<std::string> vroomids, std::map<int,int>& mguc);
	static int updateRoomMaxCount(const std::string& date,const std::map<int,int>& mguc);
	static int queryYesterdayAvgOnline(std::vector<std::string> vroomids, std::map<int,int>& avg);
	static int updateAvgOnline(const std::string& date,const std::map<int,int>& avg);
	static int queryYesterdayAvgCount(std::vector<std::string> vroomids, std::map<int,int>& avg);
	static int updateAvgCount(const std::string& date, const std::map<int,int>& avg);
private:

	static mongo::DBClientConnection * m_pMongoClient;
};

#endif /* __DB_MONGO_CONNECTION_H__ */
