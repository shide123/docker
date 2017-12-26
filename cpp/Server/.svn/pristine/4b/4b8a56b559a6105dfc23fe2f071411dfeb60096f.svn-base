#include "redis_map_zset_base.h"
#include "redisOpt.h"

redis_map_zset_base::redis_map_zset_base(redisMgr * pRedisMgr)
    : basic_redis(pRedisMgr)
{
}

redis_map_zset_base::redis_map_zset_base()
{
}

redis_map_zset_base::~redis_map_zset_base()
{
} 

bool redis_map_zset_base::insert(const string & key, const string & field, const string & score_value, int db /*= -1*/)
{
	std::string strLog = "key:" + key + ",field:" + field + ",score value:" + score_value;
	if (key.empty() || field.empty() || score_value.empty())
	{
		LOG_PRINT(log_warning, "input is empty! no need to add zset to redis.%s.", strLog.c_str());
		return false;
	}

	if (!m_pRedisMgr)
	{
		LOG_PRINT(log_error, "redis mgr is null.not need to add zset to redis.%s.", strLog.c_str());
		return false;
	}

	redisOpt * pRedisData = m_pRedisMgr->getOne(db);
	if (!pRedisData) 
	{
		LOG_PRINT(log_error, "redis opt is null.not need to add zset to redis.%s.", strLog.c_str());
		return false;
	}

	if (-1 == pRedisData->redis_zadd(key, score_value, field)) 
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool redis_map_zset_base::erase(const string & key, const string & min_score, const string & max_score, int db /* = -1 */)
{
	std::string strLog = "key:" + key + ",min_score:" + min_score + ",max_score:" + max_score;
	if (key.empty() || min_score.empty() || max_score.empty())
	{
		LOG_PRINT(log_warning, "input is empty! no need to erase record in zset.%s.", strLog.c_str());
		return false;
	}

	if (!m_pRedisMgr)
	{
		LOG_PRINT(log_error, "redis mgr is null.not need to erase record in zset.%s.", strLog.c_str());
		return false;
	}

	redisOpt * pRedisData = m_pRedisMgr->getOne(db);
	if (!pRedisData) 
	{
		LOG_PRINT(log_error, "redis opt is null.not need to erase record in zset.%s.", strLog.c_str());
		return false;
	}

	if (-1 == pRedisData->redis_zremrange_byscore(key, min_score, max_score))
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool redis_map_zset_base::getByScoreValue(const string & key, const string & min_score, const string & max_score, \
										  unsigned long pos, unsigned int batch_num, std::map<std::string, std::string > & value_score_map, int db /*= -1*/)
{
	value_score_map.clear();
	if (key.empty())
	{
		LOG_PRINT(log_warning, "input key is empty!no need to get record from zset.");
		return false;
	}

	if (!m_pRedisMgr)
	{
		LOG_PRINT(log_error, "redis mgr is null.not need to get record from zset.");
		return false;
	}

	redisOpt * pRedisData = m_pRedisMgr->getOne(db);
	if (!pRedisData) 
	{
		LOG_PRINT(log_error, "redis opt is null.not need to get record from zset.");
		return false;
	}

	if (-1 == pRedisData->redis_zrangebyscores(key, min_score, max_score, pos, batch_num, value_score_map))
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool redis_map_zset_base::getByPosValue(const string & key, long start_pos, long end_pos, std::map<std::string, std::string > & value_score_map, int db /*= -1*/)
{
	value_score_map.clear();
	if (key.empty())
	{
		LOG_PRINT(log_warning, "input key is empty!no need to get record from zset.");
		return false;
	}

	if (!m_pRedisMgr)
	{
		LOG_PRINT(log_error, "redis mgr is null.not need to get record from zset.");
		return false;
	}

	redisOpt * pRedisData = m_pRedisMgr->getOne(db);
	if (!pRedisData) 
	{
		LOG_PRINT(log_error, "redis opt is null.not need to get record from zset.");
		return false;
	}

	if (-1 == pRedisData->redis_zrangebase(key, start_pos, end_pos, value_score_map))
	{
		return false;
	}
	else
	{
		return true;
	}
}
