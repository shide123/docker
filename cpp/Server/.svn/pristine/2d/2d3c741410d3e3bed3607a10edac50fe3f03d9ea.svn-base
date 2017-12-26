#include "redis_map_set_base.h"
#include "redisOpt.h"


redis_map_set_base::redis_map_set_base(redisMgr *pRedisMgr)
    : basic_redis(pRedisMgr)
{
}

redis_map_set_base::redis_map_set_base()
{
}

redis_map_set_base::~redis_map_set_base()
{
} 

bool redis_map_set_base::insert(const string &key, const string &val, int db)
{    
    if (key.size() + val.size() + 48 > MAX_CMD_SIZE)
    {
        string query("sadd ");
        query.append(key);
        query.append(" ");
        query.append(val);
        LOG_PRINT(log_debug, "%s", query.c_str());  
        return run_update_cmd(db, query.c_str());
    }
    else
    {
        char query[MAX_CMD_SIZE] = "";
	    snprintf(query, sizeof(query) - 1, "sadd %s %s", key.c_str(), val.c_str());
	    LOG_PRINT(log_debug, "%s", query);
	    return run_update_cmd(db, query);
    }
}

bool redis_map_set_base::find(const string &key, vector<string> &val, int db)
{    
    if (key.size() + val.size() + 48 > MAX_CMD_SIZE)
    {
        string query("SMEMBERS ");
        query.append(key);
//        LOG_PRINT(log_debug, "%s", query.c_str());  
        return run_cmd(db, val, query.c_str());
    }
    else
    {
        char query[MAX_CMD_SIZE] = "";
	    snprintf(query, sizeof(query) - 1, "SMEMBERS %s", key.c_str());
//	    LOG_PRINT(log_debug, "%s", query);
	    return run_cmd(db, val, query);
    }
}

int redis_map_set_base::count(const string &key, const string &val, int db)
{   
    redisReply* reply = NULL;
    bool ret = false;
    if (key.size() + val.size() + 48 > MAX_CMD_SIZE)
    {
        string query("SISMEMBER ");
        query.append(key);
        query.append(" ");
        query.append(val);
//        LOG_PRINT(log_debug, "%s", query.c_str());  
        ret = run_cmd(db, &reply, query.c_str());
    }
    else
    {
        char query[MAX_CMD_SIZE] = "";
	    snprintf(query, sizeof(query) - 1, "SISMEMBER %s %s", key.c_str(), val.c_str());
//	    LOG_PRINT(log_debug, "%s", query);
	    ret = run_cmd(db, &reply, query);    
    }
    
    if (ret&&reply) 
    {       
        int num = reply->integer;
        freeReplyObject(reply);
        return num;
    }
    else 
    {
        return -1;
    }
}

bool redis_map_set_base::erase(const string &key, const string &val, int db)
{    
    if (key.size() + val.size() + 48 > MAX_CMD_SIZE)
    {
        string query("SREM ");
        query.append(key);
        query.append(" ");
        query.append(val);
        LOG_PRINT(log_debug, "%s", query.c_str());   
        return run_update_cmd(db, query.c_str());
    }
    else
    {
        char query[MAX_CMD_SIZE] = "";
	    snprintf(query, sizeof(query) - 1, "SREM %s %s", key.c_str(), val.c_str());
	    LOG_PRINT(log_debug, "%s", query);
	    return run_update_cmd(db, query);
    }
}
