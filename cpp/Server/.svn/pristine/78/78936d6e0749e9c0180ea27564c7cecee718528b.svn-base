#include "basic_redis.h"
#include "redisOpt.h"
#include "json/json.h"

basic_redis::basic_redis(redisMgr *pRedisMgr, int run_cmd_Mod, const char *syncChannel, int syncdb, int basic_sync_cmd, const char *bscSplit)
    : m_pRedisMgr(pRedisMgr),
      m_basic_sync_cmd(basic_sync_cmd),
      m_bscSplit(bscSplit),
      m_syncChannel(syncChannel),
      m_syncdb(syncdb),  
      m_run_cmd_Mod(run_cmd_Mod),
      m_run_cmd(2)
{
    m_run_cmd[NOSYNC]   = &basic_redis::run_cmd;
    m_run_cmd[SYNC]     = &basic_redis::sync_run_cmd;        
}

basic_redis::basic_redis()
    : m_pRedisMgr(NULL),      
      m_run_cmd_Mod(NOSYNC),
      m_run_cmd(2)
{
    m_run_cmd[NOSYNC]   = &basic_redis::run_cmd;
    m_run_cmd[SYNC]     = &basic_redis::sync_run_cmd;         
}

basic_redis::~basic_redis()
{
} 

void basic_redis::init()
{       
}

bool basic_redis::init(redisMgr *pRedisMgr, int run_cmd_Mod, const char *syncChannel, int syncdb, int basic_sync_cmd, const char *bscSplit)
{
    if (pRedisMgr == NULL) return false;    
    m_pRedisMgr = pRedisMgr;
    m_syncChannel = syncChannel,  
    m_basic_sync_cmd = basic_sync_cmd;   
    m_bscSplit.assign(bscSplit);
    m_run_cmd_Mod = run_cmd_Mod;
    m_syncdb = syncdb;       

	return true;
} 

bool basic_redis::setSyncMod(int run_cmd_Mod)
{
    m_run_cmd_Mod = run_cmd_Mod;
    return true;
}

bool basic_redis::setdb(int db, redisOpt *pRedisData, bool Islock)
{
    if (db < 0) return false;
    char query[MAX_CMD_SIZE] = "";
    snprintf(query, sizeof(query) - 1, "SELECT %d", db);
    LOG_PRINT(log_debug, "%s", query);
    if (Islock) {
        if (-1 == pRedisData->redis_run_cmd(query)) 
            return false;
    } else {
        redisReply* tmp_res;
        int ret = pRedisData->redis_run_cmd(&tmp_res, query);
        freeReplyObject(tmp_res); 
        if (-1 == ret) return false;
    }
	return true;	
}

bool basic_redis::cleandb(int db)
{
    return run_cmd(db, "FLUSHDB");
}

bool basic_redis::cleandb()
{
    return run_cmd(-1, "FLUSHDB");
}

bool basic_redis::run_update_cmd(int db, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);	
    int ret = (this->*m_run_cmd[m_run_cmd_Mod])(db, fmt, args); 
    va_end(args);  
    return ret;    
}

bool basic_redis::sync_run_cmd(int db, const char* query, ...)
{
    return sync_run_cmd(m_syncChannel.c_str(), m_syncdb, db, query);
}

bool basic_redis::sync_run_cmd(const char *syncchannel, int syncdb, int db, const char* query)
{
	Json::FastWriter writer;
	Json::Value root, head, body;
	head["msgid"] = m_basic_sync_cmd;
	head["flags"] = SYNC_REDIS_MSG;
	head["redisdb"] = db;
	body["cmd"] = query;
	root["head"] = head;
	root["body"] = body;

    static const char* pSyncFmt = "lpush %s %b";

    string squery = writer.write(root);
    LOG_PRINT(log_debug, "lpush %s %s", syncchannel, squery.c_str());
    return run_cmd(syncdb, pSyncFmt, syncchannel, squery.c_str(), squery.size());
}

bool basic_redis::run_cmd(int db, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);	
    int ret = run_cmdv(db, fmt, ap); 
    va_end(ap);  
    return ret;
}

bool basic_redis::run_cmdv(int db, const char* fmt, va_list ap)
{
    redisReply* tmp_res = NULL;
    bool ret = run_cmdv(db, &tmp_res, fmt, ap);
	freeReplyObject(tmp_res);
	return ret;     
}

bool basic_redis::run_cmd(int db, string &res, const char* fmt, ...)  
{
    vector<string> tmp_res; 
    va_list ap;
    va_start(ap, fmt);	
    int ret = run_cmdv(db, tmp_res, fmt, ap); 
    va_end(ap);
    if (!tmp_res.empty()) tmp_res[0].swap(res);  
    return ret;    
}

bool basic_redis::run_cmd(int db, vector<string> &res, const char* fmt, ...)  
{
    va_list ap;
    va_start(ap, fmt);	
    int ret = run_cmdv(db, res, fmt, ap); 
    va_end(ap);  
    return ret;    
}
  
bool basic_redis::run_cmdv(int db, vector<string> &res, const char* fmt, va_list ap)
{	
    redisReply* tmp_res = NULL;
    bool ret = run_cmdv(db, &tmp_res, fmt, ap);
    if (ret) {
        for (size_t i=0; i<tmp_res->elements; i++) {
    		if (tmp_res->element[i]->type ==  REDIS_REPLY_STRING)
    			res.push_back(tmp_res->element[i]->str);
    	}
    	if (tmp_res->elements == 0) {
    	    if (tmp_res->type ==  REDIS_REPLY_STRING)
    			res.push_back(tmp_res->str);
    	}
//    	LOG_PRINT(log_debug, "success!");
    	
	}
	freeReplyObject(tmp_res);
	return ret;     
}

bool basic_redis::run_cmd(int db, redisReply** res, const char* fmt, ...)  
{
    va_list ap;
    va_start(ap, fmt);	
    bool ret = run_cmdv(db, res, fmt, ap); 
    va_end(ap);  
    return ret;    
}

bool basic_redis::run_cmdv(int db, redisReply** res, const char* fmt, va_list ap)
{
    if (m_pRedisMgr == NULL) return false;            
    redisOpt *pRedisData = m_pRedisMgr->getOne(db);
	if (!pRedisData) return false; 
   	
    int ret = pRedisData->redis_run_cmdv(res, fmt, ap);  
	if (-1 == ret) return false;
	return true;	    
}

bool basic_redis::redis_scan_keys(const string &filter, vector<string> &val, int db)
{
    if (m_pRedisMgr == NULL)
    {
        init();
        if (m_pRedisMgr == NULL) return false;
    }
            
    redisOpt *pRedisData = m_pRedisMgr->getOne(db);
	if (!pRedisData)
        return false;
    
    
    vector<string> tmp_val;    
	int ret = pRedisData->redis_scan_keys(filter, tmp_val);
	if (!tmp_val.empty()) tmp_val.swap(val);
	if (-1 == ret) return false;       
	return true;    
}
bool basic_redis::redis_keys(const string &filter, vector<string> &val, int db)
{
	if (m_pRedisMgr == NULL)
	{
		init();
		if (m_pRedisMgr == NULL) return false;
	}

	redisOpt *pRedisData = m_pRedisMgr->getOne(db);
	if (!pRedisData)
		return false;


	vector<string> tmp_val;
	int ret = pRedisData->redis_keys(filter, tmp_val);
	if (!tmp_val.empty()) tmp_val.swap(val);
	if (-1 == ret) return false;
	return true;
}
int basic_redis::count(const string &key, int db)
{   
    redisReply* reply = NULL;
    bool ret = false;
    if (key.size() + 48 > MAX_CMD_SIZE)
    {
        string query("EXISTS ");
        query.append(key);
        LOG_PRINT(log_debug, "%s", query.c_str());  
        ret = run_cmd(db, &reply, query.c_str());
    }
    else
    {
        char query[MAX_CMD_SIZE] = "";
	    snprintf(query, sizeof(query) - 1, "EXISTS %s", key.c_str());
	    LOG_PRINT(log_debug, "%s", query);
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

bool basic_redis::expire(const string &key, unsigned int live, int db)
{
	redisReply* reply = NULL;
	bool ret = false;
	if (key.size() + 48 > MAX_CMD_SIZE)
	{
		stringstream query;
		query << "EXPIRE " << key << " " << live;
		LOG_PRINT(log_debug, "%s", query.str().c_str());  
		ret = run_cmd(db, &reply, query.str().c_str());
	}
	else
	{
		char query[MAX_CMD_SIZE] = "";
		snprintf(query, sizeof(query) - 1, "EXPIRE %s %u", key.c_str(), live);
		LOG_PRINT(log_debug, "%s", query);
		ret = run_cmd(db, &reply, query);    
	}

	if (ret && reply) 
	{       
		int num = reply->integer;
		freeReplyObject(reply);
		return num > 0;
	}
	else 
	{
		return false;
	}
}

bool basic_redis::expireat(const string &key, unsigned int expiration, int db)
{
	redisReply* reply = NULL;
	bool ret = false;
	if (key.size() + 48 > MAX_CMD_SIZE)
	{
		stringstream query;
		query << "EXPIREAT " << key << " " << expiration;
		LOG_PRINT(log_debug, "%s", query.str().c_str());  
		ret = run_cmd(db, &reply, query.str().c_str());
	}
	else
	{
		char query[MAX_CMD_SIZE] = "";
		snprintf(query, sizeof(query) - 1, "EXPIREAT %s %u", key.c_str(), expiration);
		LOG_PRINT(log_debug, "%s", query);
		ret = run_cmd(db, &reply, query);    
	}

	if (ret && reply) 
	{       
		int num = reply->integer;
		freeReplyObject(reply);
		return num > 0;
	}
	else 
	{
		return false;
	}
}

bool basic_redis::del(const string &key, int db)
{
	char query[MAX_CMD_SIZE] = "";
	snprintf(query, sizeof(query) - 1, "DEL %s %u", key.c_str());
	LOG_PRINT(log_debug, "%s", query);

	redisReply* reply = NULL;
	bool ret = run_cmd(db, &reply, query);    

	if (ret && reply) 
	{       
		int num = reply->integer;
		freeReplyObject(reply);
		return num > 0;
	}
	else 
	{
		return false;
	}
}
