/*
 * redis_map_zset_base.h
 *
 *  Created on: 2016年6月01日
 *      Author: zhangshaochen
 */

#ifndef __REDIS_MAP_ZSET_BASE_H__
#define __REDIS_MAP_ZSET_BASE_H__

#include "basic_redis.h"

class redis_map_zset_base: public basic_redis 
{
protected:
	redis_map_zset_base(redisMgr * pRedisMgr);
	redis_map_zset_base();
	~redis_map_zset_base();

    bool insert(const string & key, const string & field, const string & score_value, int db = -1);

	bool erase(const string & key, const string & min_score, const string & max_score, int db = -1);

	bool getByScoreValue(const string & key, const string & min_score, const string & max_score, \
		unsigned long pos, unsigned int batch_num, std::map<std::string, std::string > & value_score_map, int db = -1);

	bool getByPosValue(const string & key, long start_pos, long end_pos, std::map<std::string, std::string > & value_score_map, int db = -1);
};

#endif /* __REDIS_MAP_ZSET_BASE_H__ */