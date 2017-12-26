/*
 * redisOpt.h
 *
 *  Created on: 2016年2月18日
 *      Author: shuisheng
 */

#ifndef REDISOPT_H_
#define REDISOPT_H_

#include <map>
using namespace std;
#include <boost/thread/mutex.hpp>
#include "utils.h"
// hiredis
#include "hiredis.h"
#include "async.h"
#include "adapters/libev.h"
#include "redis_def.h"

typedef void (*fn_REDIS_HANDLE_MSG)(const char* channel, const char* msg, int msglen);
typedef void (*fn_REDIS_CONN_EXCEPTION)();
typedef map<string, vector<string> > HVALSNAME_MAP;
#define MAX_CMD_SIZE 4096
class redisPipeline;
class redisOpt {
public:
	friend class redisPipeline;
	redisOpt(const char* ip, uint16_t port, bool bSubPub = false, const char* password = NULL, uint16_t db = 0);
	virtual ~redisOpt();

	static void* thread_proc(void* arg);
	int runloop();
	int async_connect();
	int connect();
	const char* getip() const { return ip_.c_str(); }
	uint16 getport() const { return port_; }
	void setconnstatus(bool connected) {
		boost::mutex::scoped_lock lock(redis_async_mutex_);
		connected_ = connected;
	}
	bool isconnected() {
		boost::mutex::scoped_lock lock(redis_async_mutex_);
		return connected_;
	}

	void redis_SetHandleMsgProc(fn_REDIS_HANDLE_MSG pfn) { redis_handle_msg = pfn; }
	void redis_ConnExceptCallback(fn_REDIS_CONN_EXCEPTION pfn) { redis_conn_exception = pfn; }
	int redis_async_ping();
	int redis_ping();

	int redis_FlushDB();
	int redis_CountRoomUserid(uint32 roomid, uint32 userids);
	int redis_writeMsg(string &msg);
	int redis_writeMsg(const char *msg, int len);
	int redis_readMsg(vector<string> &result);
	int redis_readMsg(char* buf, int &len);
	int redis_llen(uint32 &listlen);
	int redis_publish(const char *msg);
	int redis_publish(const char *msg, size_t len);
    int hincrby( const string &key, const vector<string> &fields, const vector<string> &vals );
    int del( const string &key );
    
    int redis_hvals_init(string &hvalsName, vector<string> &fields, HVALSNAME_MAP &hvalsName_map);
    int redis_hvals_keys(string &hvalsName, vector<string> &vals); 
    int redis_hvals_set(string key, string &hvalsName, vector<string> &vals, vector<string> &fields);
    int redis_hvals_set(string key, string &fields, vector<string> &vals);
    int redis_hvals_set(vector<string> &key_vals, vector<string> &fields);
    int redis_hvals_find    (string key, string &hvalsName, vector<string> &vals, string &field);
    int redis_hvals_find    (string key, vector<string> &vals, string &field);
    int redis_hvals_delete(string key, string &hvalsName);
    
    int redis_run_cmd(const char* fmt, ...);
    int redis_run_cmdv(const char* fmt, va_list ap);
    
    int redis_run_cmd(redisReply** res, const char* fmt, ...);
    int redis_run_cmdv(redisReply** res, const char* fmt, va_list ap);    
    int redis_run_cmdArgv(int argvnum, const char **argv, size_t *argvlen);
    
    boost::mutex& get_redis_mutex();
    int redis_scan_keys(const string &filter, vector<string> &vals);
	int redis_keys(const string &filter, vector<string> &vals);
public:
	int redis_hget(const char *key, const string &field, string &val);
	int redis_hset(const char *key, const string &field, string &val);
	int redis_hset(const char *key, const char *field, const char *val);
    int hmset(const string &key, const vector<string> &fields, const vector<string> &vals );
	int redis_hmset(const string &key, const vector<string> &fields, const vector<string> &vals );
	int redis_hmset(const char *key, const vector<string> &fields, const vector<string> &vals );
	int redis_hmset(const char *key, const map<string, string> &map_field_val );
	int redis_hmget(const string &key, const string &fields, vector<string> &vals);
	int redis_hmget(const char *key, const string &fields, vector<string> &vals);
	int redis_hgetall(const string &key, vector<string> &fields, vector<string> &vals);
	int redis_hgetall(const char *key, vector<string> &fields, vector<string> &vals);
	int redis_hgetall(const char *key, map<string,string> &mapKVs);
	int redis_hgetallfields(const string &key, vector<string> &fields);
	int redis_hgetallvalues(const string &key, vector<string> &vals);
	int redis_hexists(const char *key, const char *field);
	int redis_hincrby(const char *key, const char *field, int increment = 1);
	int redis_hdel(const char *key, const char *field);
	int redis_hlen(const string & key);

	int redis_sadd(const string &key, string &members);
	int redis_sadd(const char *key, const char *members);
	int redis_srem(const string &key, string &member);
	int redis_srem(const char *key, const char *member);
	int redis_smembers(const string &key, vector<string> &members);
	int redis_smembers(const char *key, vector<string> &members);
	int redis_scard(const string & key);
	int redis_sismember(const string &key, string &member);

	int redis_delkey(const char *key);
	int redis_delkey(const string &key);
	int redis_delkey(const vector<string> &keys);
	int redis_existkey(const char* key);
	int redis_type(const char* key, string &type);

	int redis_zadd(const std::string & key, const std::map<std::string, std::string > & score_value_map);
	int redis_zadd(const std::string & key, const std::string & score_value, const std::string & value);
	int redis_zremrange_byscore(const std::string & key, const std::string & min_score, const std::string & max_score);
	
	/*
		min_score is string of number1,empty "" means minimum
		max_score is string of number2,empty "" means maximum
	*/
	int redis_zrangebyscores(const std::string & key, const std::string & min_score, const std::string & max_score,\
							unsigned long pos, unsigned int batch_num, std::map<std::string, std::string > & value_score_map);

	/*
		min_score is string of number1,empty "" means minimum
		max_score is string of number2,empty "" means maximum
	*/
	int redis_zrevrangebyscores(const std::string & key, const std::string & min_score, const std::string & max_score,\
							unsigned long pos, unsigned int batch_num, std::map<std::string, std::string > & value_score_map);
	/*
		asc order and return the first one.
	*/
	int redis_zrangefirst(const std::string & key, std::map<std::string, std::string > & value_score_map);

	/*
		desc order and return the first one.
	*/
	int redis_zrevrangefirst(const std::string & key, std::map<std::string, std::string > & value_score_map);

	/*
		zrange by pos not by score.
		start pos: 0 mean the first one, 1 means the second one, -1 means the last one, -2 means the last second one.
		end pos: 0 mean the first one, 1 means the second one, -1 means the last one, -2 means the last second one.
		for example:
		zrange key -10 -1 withscores, means from the last tenth to the last one.
		zrange key 0 -10 withscores, means from the first one to the tenth.
	*/
	int redis_zrangebase(const std::string & key, long start_pos, long end_pos, std::map<std::string, std::string > & value_score_map);

	boost::mutex& locker() { return redis_mutex_; }

public:
	fn_REDIS_HANDLE_MSG redis_handle_msg;
	fn_REDIS_CONN_EXCEPTION redis_conn_exception;

protected:

	/*
		min_score is string of number1,empty "" means minimum
		max_score is string of number2,empty "" means maximum
	*/
	int redis_zrangebasebyscores(const std::string & key, const std::string & min_score, const std::string & max_score,\
							unsigned long pos, unsigned int batch_num, bool desc, std::map<std::string, std::string > & value_score_map);

	SL_Thread<SL_Sync_ThreadMutex> redisThread;
	bool bSubPub_;
	ev_timer ping_timer;
	boost::mutex redis_mutex_;
	boost::mutex redis_async_mutex_;
	bool connected_;
    redisContext *c;
    uint16 retrynum;
    uint16 pipeline_cmdnum;
    redisAsyncContext *ac;
    uint16 aysnc_retrynum;
    redisReply *reply;
	string ip_;
	uint16_t port_;
	bool bReleased;
	bool bpipelinestart;
	char *password_;
	uint16_t db_;
};

typedef boost::shared_ptr<redisOpt> redisOpt_ptr;
#define NO_MORE_REDIS_REPLY 0xFFFF

/* for redis pipeline operation */
class redisPipeline
{
public:
	redisPipeline(redisOpt* pRedis)
		:pRedis_(pRedis)
	{
		if (pRedis_) {
			pRedis_->locker().lock();
			pRedis->bpipelinestart = true;
			pRedis->pipeline_cmdnum = 0;
			pRedis->connect();
		}
	}
	~redisPipeline(){
		if (pRedis_->bpipelinestart) {
			pRedis_->bpipelinestart = false;
			pRedis_->pipeline_cmdnum = 0;
			pRedis_->locker().unlock();
		}
	}

	int getreply(vector<string> &vec);
	int getallreply(map<string, string> &mValues);
	int getreply();

	int hmset(const string &key, const vector<string> &fields, const vector<string> &vals);
	int hmset(const string &key, const map<string, string> &mValues);

private:
	redisOpt *pRedis_;
};

#endif /* REDISOPT_H_ */
