/*
 * redias_map_map_base.h
 *
 *  Created on: 2016年6月01日
 *      Author: zhangshaochen
 */

#ifndef REDIS_MAP_MAP_BASE_H_
#define REDIS_MAP_MAP_BASE_H_

#include "basic_redis.h"

class redis_map_map_base:public basic_redis {
public:
	redis_map_map_base(redisMgr *pRedisMgr);
	redis_map_map_base();
	~redis_map_map_base();  
    bool insert(const string &key, const string &field, const string &val, int db = -1); 
	bool insert(const string &key, const map<string, string> &fields_vals, int db = -1);
	bool find(const string &key, const string &field, string &val, int db = -1);    
    bool erase(const string &key, const string &field, int db = -1);
	bool erase(const string &key, int db = -1);
    bool size(const string &key, int &fieldnum, int db = -1);
	bool exist(const string &key, int db = -1);
	bool hexists(const string &key, const string &field, int db = 0);
	bool hgetall(const string &key, std::vector<std::string> &res, int db = -1);
	bool hget(const string &key, const std::string &field, std::string &field_value, int db = -1);
	int hincrby(const string & key, const std::string & field, int increment, int db = -1);
	bool hkeys(const string & key, std::vector<std::string> &res, int db = -1);
};

#endif /* REDIS_MAP_MAP_BASE_H_ */