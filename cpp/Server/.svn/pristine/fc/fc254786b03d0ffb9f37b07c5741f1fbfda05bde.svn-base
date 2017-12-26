/*
 * redisMgr.cpp
 *
 *  Created on: 2016年3月30日
 *      Author: shuisheng
 */

#include "redisMgr.h"
#include "redisOpt.h"
#include <limits.h>

redisMgr::redisMgr(const char* ip, uint16 port, const char* passwd, size_t num)
	: ip_(ip)
	, port_(port)
	, passwd_(passwd)
	, num_(num)
{
	spawnRedis(0);

}

redisMgr::~redisMgr() {
	map<int, vector<redisOpt*> >::const_iterator it = map_redis.begin();
	while(it != map_redis.end()) {
		size_t num = it->second.size();
		for (size_t i=0; i<num; i++){
			delete it->second[i];
		}
		++it;
	}
}

void redisMgr::spawnRedis(int db)
{
	vector<redisOpt*> vecRedis;
	for(size_t i=0; i<num_; i++){
		redisOpt *predis = new redisOpt(ip_.c_str(), port_, false, passwd_.c_str(), db);
		if (predis){
			predis->runloop();
			vecRedis.push_back(predis);
		}
	}
	map_redis[db] = vecRedis;
	map_index[db] = 0;
}

redisOpt* redisMgr::selectOne(int db)
{
	uint32_t index = map_index[db];
	if (index >= UINT_MAX - 1)
		index = 0;

	map_index[db] = index + 1;

	return map_redis[db][index % num_];
}

void redisMgr::redis_ConnExceptCallback(fn_REDIS_CONN_EXCEPTION pfn)
{
	map<int, vector<redisOpt*> >::const_iterator it = map_redis.begin();
	while(it != map_redis.end()) {
		size_t num = it->second.size();
		for (size_t i=0; i<num; i++){
			it->second[i]->redis_ConnExceptCallback(pfn);
		}
		++it;
	}
}

redisOpt* redisMgr::getOne(int db)
{
	boost::mutex::scoped_lock lock(inc_mutex_);

	vector<redisOpt*> vecRedis;
	map<int, vector<redisOpt*> >::iterator it = map_redis.find(db);
	if (it == map_redis.end()) {
		spawnRedis(db);
	}

	return selectOne(db);
}

