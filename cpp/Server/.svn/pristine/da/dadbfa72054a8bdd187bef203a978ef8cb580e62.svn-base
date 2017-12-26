/*
 * redisMgr.h
 *
 *  Created on: 2016年3月30日
 *      Author: shuisheng
 */

#ifndef REDISMGR_H_
#define REDISMGR_H_
#include <map>
#include <vector>
using namespace std;
#include <boost/thread/mutex.hpp>
#include "redisOpt.h"

class redisMgr {
public:
	redisMgr(const char* ip, uint16 port, const char* passwd, size_t num);
	void redis_ConnExceptCallback(fn_REDIS_CONN_EXCEPTION pfn);
	virtual ~redisMgr();

	redisOpt* getOne(int db=0);
protected:
	void spawnRedis(int db);
	redisOpt* selectOne(int db=0);
private:
	map<int, vector<redisOpt*> > map_redis;
	map<int, uint32_t> map_index;
	string ip_;
	uint16 port_;
	string passwd_;
	size_t num_;
	boost::mutex inc_mutex_;
};

#endif /* REDISMGR_H_ */
