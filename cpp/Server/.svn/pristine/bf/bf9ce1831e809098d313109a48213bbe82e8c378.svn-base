#include "redis_map_map_base.h"
#include "redisOpt.h"


redis_map_map_base::redis_map_map_base(redisMgr *pRedisMgr)
    : basic_redis(pRedisMgr)
{
}

redis_map_map_base::redis_map_map_base()
{
}

redis_map_map_base::~redis_map_map_base()
{
} 

bool redis_map_map_base::insert(const string &key, const string &field, const string &val, int db)
{   
    if (key.empty()||field.empty()||val.empty()) return false;
         
    if (key.size() + field.size() + val.size() + 48 > MAX_CMD_SIZE)
    {
        string query("HSET ");
        query.append(key);
        query.append(" ");
        query.append(field);  
        query.append(" ");
        query.append(val);  
        LOG_PRINT(log_debug, "%s", query.c_str());
        return run_update_cmd(db, query.c_str());
    }
    else
    {
        char query[MAX_CMD_SIZE] = "";
	    snprintf(query, sizeof(query) - 1, "HSET %s %s %s", key.c_str(), field.c_str(), val.c_str());
	    LOG_PRINT(log_debug, "%s", query);
	    return run_update_cmd(db, query);
    }
}

bool redis_map_map_base::insert(const string &key, const map<string, string> &fields_vals, int db /* = -1 */)
{
	if (key.empty() || fields_vals.empty())
	{
		LOG_PRINT(log_error, "insert multi fields and values fail.key:%s,fields_vals size:%u.", key.c_str(), fields_vals.size());
		return false;
	}

	if (m_pRedisMgr == NULL)
		return false;            

	redisOpt * pRedisData = m_pRedisMgr->getOne(db);
	if (!pRedisData) 
		return false; 

	vector<string> fields;
	fields.reserve(fields_vals.size());
	vector<string> vals;
	vals.reserve(fields_vals.size());

	map<string, string>::const_iterator iter = fields_vals.begin();
	for (; iter != fields_vals.end(); ++iter)
	{
		fields.push_back(iter->first);
		vals.push_back(iter->second);
	}

	int ret = pRedisData->redis_hmset(key, fields, vals);  
	if (-1 == ret) return false;
	return true;
}

bool redis_map_map_base::find(const string &key, const string &field, string &val, int db)
{  
    if (key.empty()||field.empty()) return false;   
    if (key.size() + field.size() + val.size() + 48 > MAX_CMD_SIZE)
    {
        string query("HGET ");
        query.append(key);
        query.append(" ");
        query.append(field);
        LOG_PRINT(log_debug, "%s", query.c_str());
        return run_cmd(db, val, query.c_str());
    }
    else
    {
        char query[MAX_CMD_SIZE] = "";
	    snprintf(query, sizeof(query) - 1, "HGET %s %s", key.c_str(), field.c_str());
//	    LOG_PRINT(log_debug, "%s", query);
	    return run_cmd(db, val, query);
    }
}

bool redis_map_map_base::erase(const string &key, const string &field, int db)
{    
    if (key.empty()||field.empty()) return false; 
    if (key.size() + field.size() + 48 > MAX_CMD_SIZE)
    {
        string query("HDEL ");
        query.append(key);
        query.append(" ");
        query.append(field);  
        LOG_PRINT(log_debug, "%s", query.c_str());
        return run_update_cmd(db, query.c_str());
    }
    else
    {
        char query[MAX_CMD_SIZE] = "";
	    snprintf(query, sizeof(query) - 1, "HDEL %s %s", key.c_str(), field.c_str());
	    LOG_PRINT(log_debug, "%s", query);
	    return run_update_cmd(db, query);
    }
}

bool redis_map_map_base::erase(const string &key, int db /*= -1*/)
{
	if (key.empty()) 
	{
		LOG_PRINT(log_warning, "key is empty!no need to del in redis.");
		return false;
	}

	char query[MAX_CMD_SIZE] = "";
	snprintf(query, sizeof(query) - 1, "DEL %s", key.c_str());
	LOG_PRINT(log_debug, "%s", query);
	return run_cmd(db, query);
}

bool redis_map_map_base::size(const string &key, int &fieldnum, int db)
{
	fieldnum = 0;
    if (key.empty())
		return false; 
	
	if (m_pRedisMgr == NULL)
		return false;

	redisOpt * pRedisData = m_pRedisMgr->getOne(db);
	if (!pRedisData) 
		return false; 

	int ret = pRedisData->redis_hlen(key);
	if (-1 == ret)
	{
		return false;
	}
	else
	{
		fieldnum = ret;
		return true;
	}
}

bool redis_map_map_base::exist(const string &key, int db /*= -1*/)
{
	if (key.empty()) 
	{
		LOG_PRINT(log_warning, "key is empty!no need to query redis.");
		return false;
	}

	if (m_pRedisMgr == NULL)
		return false;            

	redisOpt * pRedisData = m_pRedisMgr->getOne(db);
	if (!pRedisData) 
		return false; 

	int ret = pRedisData->redis_existkey(key.c_str());  
	if (-1 == ret)
		return false;
	bool bRet = (bool)ret;
	return bRet;
}

bool redis_map_map_base::hexists(const string &key, const string &field, int db /*= 0*/)
{
	if (key.empty() || field.empty())
	{
		LOG_PRINT(log_warning, "key[%s] or field[%s] is empty!no need to query redis.", key.c_str(), field.c_str());
		return false;
	}

	if (m_pRedisMgr == NULL)
		return false;

	redisOpt * pRedisData = m_pRedisMgr->getOne(db);
	if (!pRedisData)
		return false;

	int ret = pRedisData->redis_hexists(key.c_str(), field.c_str());

	return ret > 0;
}

bool redis_map_map_base::hgetall(const string &key, std::vector<std::string> &res, int db /*= -1*/)
{
	if (key.empty()) 
	{
		LOG_PRINT(log_warning, "key is empty!no need to query redis.");
		return false;
	}

	char query[MAX_CMD_SIZE] = "";
	snprintf(query, sizeof(query) - 1, "HGETALL %s", key.c_str());
//	LG_PRINT(log_debug, "%s", query);
	return run_cmd(db, res, query);
}

bool redis_map_map_base::hget( const string &key, const std::string &field, std::string &field_value, int db /*= -1*/ )
{
	if (key.empty() || field.empty()) 
	{
		LOG_PRINT(log_warning, "key or field is empty!no need to query redis.key:%s,field:%s.", key.c_str(), field.c_str());
		return false;
	}

	char query[MAX_CMD_SIZE] = "";
	snprintf(query, sizeof(query) - 1, "HGET %s %s", key.c_str(), field.c_str());
//	LOG_PRINT(log_debug, "%s", query);
	return run_cmd(db, field_value, query);
}

int redis_map_map_base::hincrby(const string & key, const std::string & field, int increment, int db /*= -1*/)
{
	if (key.empty() || field.empty()) 
	{
		LOG_PRINT(log_error, "key or field is empty!no need to hincrby redis.key:%s,field:%s.", key.c_str(), field.c_str());
		return -1;
	}

	if (m_pRedisMgr == NULL)
		return -1;            

	redisOpt * pRedisData = m_pRedisMgr->getOne(db);
	if (!pRedisData) 
		return -1; 

	int ret = pRedisData->redis_hincrby(key.c_str(), field.c_str(), increment);
	return ret;
}

bool redis_map_map_base::hkeys(const string & key, std::vector<std::string> &res, int db /*= -1*/)
{
	if (key.empty())
	{
		LOG_PRINT(log_warning, "key is empty!no need to query redis.");
		return false;
	}

	char query[MAX_CMD_SIZE] = "";
	snprintf(query, sizeof(query)-1, "HKEYS %s", key.c_str());
//	LOG_PRINT(log_debug, "%s", query);
	return run_cmd(db, res, query);
}
