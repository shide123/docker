/*
 * redias_map_set_base.h
 *
 *  Created on: 2016年5月25日
 *      Author: zhangshaochen
 */

#ifndef _REDIS_MAP_SET_BASE_H_
#define _REDIS_MAP_SET_BASE_H_

#include "basic_redis.h"

class redis_map_set_base:public basic_redis {
public:
	redis_map_set_base(redisMgr *pRedisMgr);
	redis_map_set_base();
	~redis_map_set_base();  
    bool insert(const string &key, const string &val, int db = -1); 
    bool find(const string &key, vector<string> &val, int db = -1);    
    bool erase(const string &key, const string &val, int db = -1);  
    int count(const string &key, const string &val, int db = -1);       
};

#endif /* _REDIS_MAP_SET_BASE_H_ */
