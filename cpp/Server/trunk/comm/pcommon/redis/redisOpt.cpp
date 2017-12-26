/*
 * redisOpt.cpp
 *
 *  Created on: 2016年2月18日
 *      Author: shuisheng
 */

#include "redisOpt.h"
#include "CLogThread.h"
#include "CAlarmNotify.h"

#define REDIS_ZSET_MIN "-inf"
#define REDIS_ZSET_MAX "+inf"

void connectCallback(const redisAsyncContext *c, int status) {
	redisOpt *pRedis = (redisOpt *)c->data;
    if (status != REDIS_OK) {
        LOG_PRINT(log_error, "redis error: %s [%s:%u]", c->errstr, pRedis->getip(), pRedis->getport());

        return;
    }
    pRedis->setconnstatus(true);
    LOG_PRINT(log_info, "redis Connected...[%s:%u]", pRedis->getip(), pRedis->getport());
}

void disconnectCallback(const redisAsyncContext *c, int status) {
	redisOpt *pRedis = (redisOpt *)c->data;
	pRedis->setconnstatus(false);

    if (status != REDIS_OK) {
    	LOG_PRINT(log_error, "redis error: %s [%s:%u]", c->errstr, pRedis->getip(), pRedis->getport());
        return;
    }

    LOG_PRINT(log_info, "redis Disconnected...[%s:%u]", pRedis->getip(), pRedis->getport());
}

void setCallback(redisAsyncContext *c, void *r, void *privdata) {
    redisReply *reply = (redisReply *)r;
    redisOpt *pRedis = (redisOpt *)c->data;
    if (reply == NULL){
    	LOG_PRINT(log_error, "redis error: %s [%s:%u]", c->errstr, pRedis->getip(), pRedis->getport());
    	return;
    }
    //printf("argv[%s]: %s\n", (char*)privdata, reply->str);
}

static void timeout_cb(EV_P_ ev_timer *w, int revents)
{
	redisOpt *predis = (redisOpt*)w->data;
	if (predis)
		predis->redis_async_ping();
}

void onMessage(redisAsyncContext *c, void *reply, void *privdata) {
    redisReply *r = (redisReply*)reply;
    redisOpt *pRedis = (redisOpt*)privdata;
    if (reply == NULL)
    	return;
    if (r->type == REDIS_REPLY_ARRAY) {
        for (size_t j = 0; j < r->elements; j++) {
            printf("%u) %s\n", j, r->element[j]->str);
        }
        if (0 == strcasecmp(r->element[0]->str, "message")){
        	if (pRedis->redis_handle_msg)
        		pRedis->redis_handle_msg(r->element[1]->str, r->element[2]->str, r->element[2]->len);
        }
    }
}

redisOpt::redisOpt(const char* ip, uint16_t port, bool bSubPub, const char *password, uint16_t db)
	: redis_handle_msg(NULL)
	, redis_conn_exception(NULL)
	, bSubPub_(bSubPub)
	, connected_(false)
	, c(NULL)
	, retrynum(0)
	, pipeline_cmdnum(0)
	, ac(NULL)
	, aysnc_retrynum(0)
	, reply(NULL)
	, ip_(ip)
	, port_(port)
	, bReleased(false)
	, bpipelinestart(false)
	, password_(NULL)
	, db_(db)

{
	if (password)
		password_ = strdup(password);
//    m_hvalsName_field[KEY_HASH_ROOM_INFO] = "roomname busepwd strpwd nlevel nopstate nvcbid ngroupid nseats ncreatorid nopuserid0 nopuserid1 nopuserid2 nopuserid3 roomnotice0 roomnotice1 roomnotice2 roomnotice3";
//    m_hvalsName_field[KEY_HASH_USER_INFO] = "vcbid userid nsvrid ngateid nk nb nd pGateObj pGateObjId nviplevel nyiyuanlevel nshoufulevel nzhongshenlevel ncaifulevel lastmonthcostlevel thismonthcostlevel thismonthcostgrade nisxiaoshou ngender ndevtype ncurpublicmicindex ninroomlevel usertype nheadid nstarflag nactivityflag nflowernum nyuanpiaonum bForbidChat calias ipaddr uuid gateip mediaip inroomstate nuserviplevel";
}

redisOpt::~redisOpt() {
	// TODO Auto-generated destructor stub
	if (c){
		redisFree(c);
		c = NULL;
	}
	bReleased = true;
	if (ac && !ac->err)
		redisAsyncDisconnect(ac);

	if (password_)
		free(password_);
}

void* redisOpt::thread_proc(void* arg)
{
	redisOpt* propt = (redisOpt*)arg;
	if (!propt)
		return NULL;

	signal(SIGPIPE, SIG_IGN);

	while (true){
		if (!propt->bReleased){
			propt->async_connect();
			sleep(10);
		}
	}
}

int redisOpt::runloop()
{
	redisThread.start(thread_proc, this);
	return 0;
}

int redisOpt::async_connect()
{
    ac = redisAsyncConnect(ip_.c_str(), port_);
    if (ac->err) {
        LOG_PRINT(log_error, "Error: %s\n", ac->errstr);
        return -1;
    }
    ac->data = this;

    struct ev_loop *epoller = ev_loop_new (EVBACKEND_EPOLL | EVFLAG_NOENV);
    redisLibevAttach(epoller, ac);
    redisAsyncSetConnectCallback(ac, connectCallback);
    redisAsyncSetDisconnectCallback(ac, disconnectCallback);
    if (password_){
    	redisAsyncCommand(ac, setCallback, NULL, "AUTH %s", password_);
    }
    if (bSubPub_)
    	redisAsyncCommand(ac, onMessage, this, "SUBSCRIBE %s", CHANNEL_REDIS_SYNC);
    else{
		ping_timer.data = this;
		ev_timer_init (&ping_timer, timeout_cb, 2, 30);
		ev_timer_start (epoller, &ping_timer);
    }

    ev_loop(epoller, 0);
    ev_loop_destroy(epoller);

    return 0;
}

int redisOpt::connect()
{
	struct timeval timeout = { 0, 500000 }; // 0.5 seconds

	int ntimes = 0;

START:
	ntimes++;
	if (ntimes > 3){
		LOG_PRINT(log_error, "Failed to connect after attempt 3 times");
		if (redis_conn_exception)
			redis_conn_exception();
		return -1;
	}

	if (c && c->err){
        redisFree(c);
        c = NULL;
	}

	if (NULL == c){
	    c = redisConnectWithTimeout(ip_.c_str(), port_, timeout);
	    if (c == NULL || c->err) {
	        if (c) {
	        	LOG_PRINT(log_error, "Connection error: %s", c->errstr);
	            redisFree(c);
	            c = NULL;
	        } else {
	        	LOG_PRINT(log_error, "Connection error: can't allocate redis context");
	        }
	        goto START;
	    }
	    if (password_){
	    	reply = (redisReply*)redisCommand(c, "AUTH %s", password_);
	    	if (NULL == reply){
	    		LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
	    		redisFree(c);
	    		c = NULL;
	    		goto START;
	    	}
	    	freeReplyObject(reply);
	    }
	    if (db_) {
	    	reply = (redisReply*)redisCommand(c, "SELECT %u", db_);
	    	if (NULL == reply){
	    		LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
	    		redisFree(c);
	    		c = NULL;
	    		goto START;
	    	}
	    	freeReplyObject(reply);
	    }
	}
	else {
		reply = (redisReply*)redisCommand(c, "ping");
		if (NULL == reply){
			LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
			redisFree(c);
			c = NULL;
			goto START;
		}
		freeReplyObject(reply);
	}

	return 0;
}

int redisOpt::redis_async_ping()
{
	if (!isconnected())
		return -1;

	return redisAsyncCommand(ac, setCallback, NULL, "ping");
}

int redisOpt::redis_ping()
{
	boost::mutex::scoped_lock lock(redis_mutex_);

	if (connect() < 0)
		return -1;

	reply = (redisReply*)redisCommand(c, "ping");
	if (NULL == reply){
		LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
		redisFree(c);
		c = NULL;
		return -1;
	}
	freeReplyObject(reply);

	return 0;
}

int redisOpt::redis_FlushDB()
{
	boost::mutex::scoped_lock lock(redis_mutex_);

	if (connect() < 0)
		return -1;

	char cmd[32] = {0};
	sprintf(cmd, "FLUSHDB");
	reply = (redisReply*)redisCommand(c, cmd);
	if (NULL == reply){
		LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
		redisFree(c);
		c = NULL;
		return -1;
	}

	freeReplyObject(reply);

	return 0;
}

typedef struct arg{
	bool breply;
	vector<string> *pdata;
}ARG_t;

void getAllUseridsCallback(redisAsyncContext *c, void *r, void *privdata) {
	ARG_t *parg = (ARG_t *)privdata;

	redisOpt *pRedis = (redisOpt *)c->data;

    redisReply *reply = (redisReply *)r;
    if (reply == NULL){
    	LOG_PRINT(log_error, "redis error: %s [%s:%u]", c->errstr, pRedis->getip(), pRedis->getport());

    	parg->breply = true;
    	return;
    }
    //printf("argv[%s]: %s\n", (char*)privdata, reply->str);

    parg->breply = true;
}

int redisOpt::redis_CountRoomUserid(uint32 roomid, uint32 userids)
{
	boost::mutex::scoped_lock lock(redis_mutex_);

	if (connect() < 0)
		return -1;

	reply = (redisReply*)redisCommand(c,"SISMEMBER %s:%u %u", KEY_SET_ROOM_USERIDS, roomid, userids );
	if (NULL == reply){
		LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
		redisFree(c);
		c = NULL;
		return -1;
	}
	if (REDIS_REPLY_ERROR == reply->type) {
		LOG_PRINT(log_error, "err_str:%s", reply->str);
		freeReplyObject( reply );
		return -1;
	}
	freeReplyObject(reply);

	return reply->integer;
}

int redisOpt::hmset( const string &key, const vector<string> &fields, const vector<string> &vals )
{
	int nret = -1;
	do {
		if (fields.size() != vals.size()){
			LOG_PRINT(log_error, "the length not match for fields and vals");
			break;
		}

		vector<const char *> argv;
		vector<size_t> argvlen;

		static char cmd[] = "HMSET";
		argv.push_back( cmd );
		argvlen.push_back( sizeof(cmd)-1 );

		argv.push_back( key.c_str() );
		argvlen.push_back( key.size() );

		for (size_t i=0; i<fields.size(); i++) {
			argv.push_back(fields[i].c_str());
			argvlen.push_back(fields[i].size());
			argv.push_back(vals[i].c_str());
			argvlen.push_back(vals[i].size());
		}

		if (bpipelinestart){
			if (REDIS_OK != redisAppendCommandArgv(c, argv.size(), &(argv[0]), &(argvlen[0]))) {
				break;
			}
			++pipeline_cmdnum;
		}
		else {
			if (connect() < 0) {
				break;
			}
			reply = (redisReply*)redisCommandArgv(c, argv.size(), &(argv[0]), &(argvlen[0]) );
			if ( NULL == reply ) {
				LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
				redisFree(c);
				c = NULL;
				break;
			}
			if (REDIS_REPLY_ERROR == reply->type) {
				LOG_PRINT(log_error, "err_str:%s", reply->str);
				freeReplyObject( reply );
				break;
			}
			freeReplyObject( reply );
		}
		nret = 0;
	}while(0);

	return nret;
}

int redisOpt::redis_zadd(const std::string & key, const std::map<std::string, std::string > & score_value_map)
{
	int nret = -1;
	do 
	{
		if (score_value_map.empty() || key.empty())
		{
			LOG_PRINT(log_warning, "score_value_map or key is empty.key:%s.", key.c_str());
			break;
		}

		vector<const char *> argv;
		vector<size_t> argvlen;

		std::string zaddcmd = "ZADD";
		argv.push_back( zaddcmd.c_str() );
		argvlen.push_back( zaddcmd.size() );

		argv.push_back( key.c_str() );
		argvlen.push_back( key.size() );

		std::map<string, std::string >::const_iterator iter = score_value_map.begin();
		for (; iter != score_value_map.end(); ++iter)
		{
			argv.push_back(iter->first.c_str());
			argvlen.push_back(iter->first.size());

			argv.push_back(iter->second.c_str());
			argvlen.push_back(iter->second.size());
		}

		if (bpipelinestart)
		{
			if (REDIS_OK != redisAppendCommandArgv(c, argv.size(), &(argv[0]), &(argvlen[0]))) 
			{
				break;
			}

			++pipeline_cmdnum;
		}
		else 
		{
			boost::mutex::scoped_lock lock(redis_mutex_);
			if (connect() < 0) 
			{
				break;
			}

			reply = (redisReply *)redisCommandArgv(c, argv.size(), &(argv[0]), &(argvlen[0]));
			if (NULL == reply)
			{
				LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
				redisFree(c);
				c = NULL;
				break;
			}
			
			if (REDIS_REPLY_ERROR == reply->type) 
			{
				LOG_PRINT(log_error, "err_str:%s", reply->str);
				freeReplyObject(reply);
				break;
			}

			freeReplyObject(reply);
		}
		nret = 0;
	}while(0);

	return nret;
}

int redisOpt::redis_zadd(const std::string & key, const std::string & score_value, const std::string & value)
{
	std::map<std::string, std::string > score_value_map;
	score_value_map.insert(std::make_pair(score_value, value));
	return redis_zadd(key, score_value_map);
}

int redisOpt::redis_zremrange_byscore(const std::string & key, const std::string & min_score, const std::string & max_score)
{
	int nret = -1;
	if (key.empty() || min_score.empty() || max_score.empty())
	{
		LOG_PRINT(log_error, "[zremrange by score]important input empty.key:%s,min score:%s,max score:%s.", key.c_str(), min_score.c_str(), max_score.c_str());
		return nret;
	}

	std::string cmd = "zremrangebyscore " + key + " " + min_score + " " + max_score;

	do {
		if (bpipelinestart){
			if (REDIS_OK != redisAppendCommand(c, cmd.c_str())) {
				break;
			}
			++pipeline_cmdnum;
		}
		else {
			boost::mutex::scoped_lock lock(redis_mutex_);
			if (connect() < 0) {
				break;
			}

			reply = (redisReply *)redisCommand(c, cmd.c_str());
			if (NULL == reply){
				LOG_PRINT(log_error, "redis cmd:%s, err:%d, err_str:%s", cmd.c_str(), c->err, c->errstr);
				redisFree(c);
				c = NULL;
				break;
			}

			if (REDIS_REPLY_ERROR == reply->type) {
				LOG_PRINT(log_error, "redis cmd:%s, err_str:%s", cmd.c_str(), reply->str);
				freeReplyObject( reply );
				break;
			}

			if (reply->integer) {
				/*删除成功*/
				LOG_PRINT(log_debug, "redis cmd:%s, erase record rows:%lld.", cmd.c_str(), reply->integer);
			}
			else {
				LOG_PRINT(log_warning, "redis cmd:%s, records not exists", cmd.c_str());
			}

			freeReplyObject( reply );
		}
		nret = 0;
	}while(0);

	return nret;
}

int redisOpt::redis_zrangebyscores(const std::string & key, const std::string & min_score, const std::string & max_score,\
								   unsigned long pos, unsigned int batch_num, std::map<std::string, std::string > & value_score_map)
{
	return redis_zrangebasebyscores(key, min_score, max_score, pos, batch_num, false, value_score_map);
}

int redisOpt::redis_zrevrangebyscores(const std::string & key, const std::string & min_score, const std::string & max_score,\
									  unsigned long pos, unsigned int batch_num, std::map<std::string, std::string > & value_score_map)
{
	return redis_zrangebasebyscores(key, min_score, max_score, pos, batch_num, true, value_score_map);
}

int redisOpt::redis_zrangebasebyscores(const std::string & key, const std::string & min_score, const std::string & max_score,\
									   unsigned long pos, unsigned int batch_num, bool desc, std::map<std::string, std::string > & value_score_map)
{
	int nret = -1;
	do 
	{
		value_score_map.clear();

		std::string zrangecmd = "";
		if (desc)
		{
			zrangecmd = "ZREVRANGEBYSCORE";
		}
		else
		{
			zrangecmd = "ZRANGEBYSCORE";
		}

		vector<const char *> argv;
		vector<size_t> argvlen;
		argv.push_back( zrangecmd.c_str() );
		argvlen.push_back( zrangecmd.size() );

		argv.push_back( key.c_str() );
		argvlen.push_back( key.size() );

		std::string min_pos = min_score;
		std::string max_pos = max_score;
		if (min_pos.empty())
		{
			min_pos = REDIS_ZSET_MIN;
		}

		if (max_pos.empty())
		{
			max_pos = REDIS_ZSET_MAX;
		}

		if (desc)
		{
			argv.push_back( max_pos.c_str() );
			argvlen.push_back( max_pos.size() );

			argv.push_back( min_pos.c_str() );
			argvlen.push_back( min_pos.size() );
		}
		else
		{
			argv.push_back( min_pos.c_str() );
			argvlen.push_back( min_pos.size() );

			argv.push_back( max_pos.c_str() );
			argvlen.push_back( max_pos.size() );
		}

		std::string withscorescmd = "withscores";
		argv.push_back( withscorescmd.c_str() );
		argvlen.push_back( withscorescmd.size() );

		if (batch_num)
		{
			std::string zlimitcmd = "limit";
			argv.push_back( zlimitcmd.c_str() );
			argvlen.push_back( zlimitcmd.size() );
	
			char cPos[64] = {0};
			snprintf(cPos, sizeof(cPos), "%lu", pos);
			argv.push_back( cPos );
			argvlen.push_back( sizeof(cPos) );
			
			char cBatchNum[32] = {0};
			snprintf(cBatchNum, sizeof(cBatchNum), "%u", batch_num);
			argv.push_back( cBatchNum );
			argvlen.push_back( sizeof(cBatchNum) );
		}

		if (bpipelinestart)
		{
			if (REDIS_OK != redisAppendCommandArgv(c, argv.size(), &(argv[0]), &(argvlen[0]))) 
			{
				break;
			}

			++pipeline_cmdnum;
		}
		else 
		{
			boost::mutex::scoped_lock lock(redis_mutex_);
			if (connect() < 0) 
			{
				break;;
			}

			reply = (redisReply *)redisCommandArgv(c, argv.size(), &(argv[0]), &(argvlen[0]));
			if (NULL == reply)
			{
				LOG_PRINT(log_error, "[redis_zrangebasebyscores]desc:%d, err:%d, err_str:%s", (int)desc, c->err, c->errstr);
				redisFree(c);
				c = NULL;
				break;
			}

			if (REDIS_REPLY_ERROR == reply->type) 
			{
				LOG_PRINT(log_error, "[redis_zrangebasebyscores]desc:%d, err_str:%s", (int)desc, reply->str);
				freeReplyObject(reply);
				break;
			}

			for (size_t i = 0; i + 1 < reply->elements; i += 2)
			{
				if (reply->element[i]->type ==  REDIS_REPLY_STRING && reply->element[i+1]->type ==  REDIS_REPLY_STRING)
				{
					//i is key string,i+1 is score value.
					value_score_map[reply->element[i]->str] = reply->element[i + 1]->str;
				}
			}

			freeReplyObject(reply);
		}
		nret = 0;
	}while(0);

	return nret;
}

int redisOpt::redis_zrangebase(const std::string & key, long start_pos, long end_pos, std::map<std::string, std::string > & value_score_map)
{
	int nret = -1;
	do 
	{
		value_score_map.clear();

		std::string zrangecmd = "ZRANGE";

		vector<const char *> argv;
		vector<size_t> argvlen;
		argv.push_back( zrangecmd.c_str() );
		argvlen.push_back( zrangecmd.size() );

		argv.push_back( key.c_str() );
		argvlen.push_back( key.size() );

		char cStartPos[128] = {0};
		snprintf(cStartPos, sizeof(cStartPos), "%ld", start_pos);

		argv.push_back( cStartPos );
		argvlen.push_back( sizeof(cStartPos) );

		char cEndPos[128] = {0};
		snprintf(cEndPos, sizeof(cEndPos), "%ld", end_pos);

		argv.push_back( cEndPos );
		argvlen.push_back( sizeof(cEndPos) );

		std::string withscorescmd = "withscores";
		argv.push_back( withscorescmd.c_str() );
		argvlen.push_back( withscorescmd.size() );

		if (bpipelinestart)
		{
			if (REDIS_OK != redisAppendCommandArgv(c, argv.size(), &(argv[0]), &(argvlen[0]))) 
			{
				break;
			}

			++pipeline_cmdnum;
		}
		else 
		{
			boost::mutex::scoped_lock lock(redis_mutex_);
			if (connect() < 0) 
			{
				break;;
			}

			reply = (redisReply *)redisCommandArgv(c, argv.size(), &(argv[0]), &(argvlen[0]));
			if (NULL == reply)
			{
				LOG_PRINT(log_error, "[redis_zrangebase]key:%s,start_pos:%ld,end_pos:%ld,err:%d,err_str:%s.", key.c_str(), start_pos, end_pos, c->err, c->errstr);
				redisFree(c);
				c = NULL;
				break;
			}

			if (REDIS_REPLY_ERROR == reply->type) 
			{
				LOG_PRINT(log_error, "[redis_zrangebase]key:%s,start_pos:%ld,end_pos:%ld,err_str:%s.", key.c_str(), start_pos, end_pos, reply->str);
				freeReplyObject(reply);
				break;
			}

			for (size_t i = 0; i + 1 < reply->elements; i += 2)
			{
				if (reply->element[i]->type ==  REDIS_REPLY_STRING && reply->element[i+1]->type ==  REDIS_REPLY_STRING)
				{
					//i is key string,i+1 is score value.
					value_score_map[reply->element[i]->str] = reply->element[i+1]->str;
				}
			}

			freeReplyObject(reply);
		}
		nret = 0;
	}while(0);

	return nret;
}

int redisOpt::redis_zrangefirst(const std::string & key, std::map<std::string, std::string > & value_score_map)
{
	return redis_zrangebase(key, 0, 0, value_score_map);
}

int redisOpt::redis_zrevrangefirst(const std::string & key, std::map<std::string, std::string > & value_score_map)
{
	return redis_zrangebase(key, -1, -1, value_score_map);
}

int redisOpt::redis_hmset(const string &key, const vector<string> &fields, const vector<string> &vals )
{
	if (!bpipelinestart) {
		boost::mutex::scoped_lock lock(redis_mutex_);
		return hmset(key, fields, vals);
	}
	return hmset(key, fields, vals);
}

int redisOpt::redis_hmset(const char *key, const vector<string> &fields, const vector<string> &vals )
{
	string skey = key;
	return redis_hmset(skey, fields, vals);
}

int redisOpt::redis_hmset(const char *key, const map<string, string> &map_field_val )
{
	vector<string> vecfield, vecvalue;
	map<string, string>::const_iterator it = map_field_val.begin();
	for (; it != map_field_val.end(); it++) {
		vecfield.push_back(it->first);
		vecvalue.push_back(it->second);
	}

	return redis_hmset(key, vecfield, vecvalue);
}

int redisOpt::redis_hget(const char *key, const string &field, string &val)
{
	int nret = -1;
	char str[128];
	sprintf(str, "hget %s %s", key, field.c_str());

	do {
		if (bpipelinestart){
			if (REDIS_OK != redisAppendCommand(c, str)) {
				break;
			}
			++pipeline_cmdnum;
		}
		else {
			boost::mutex::scoped_lock lock(redis_mutex_);
			if (connect() < 0) {
				break;
			}

			reply = (redisReply*)redisCommand(c, str);
			if (NULL == reply){
				LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
				redisFree(c);
				c = NULL;
				break;
			}

			if (REDIS_REPLY_ERROR == reply->type || REDIS_REPLY_NIL == reply->type) {
				LOG_PRINT(log_error, "err_str:%s", reply->str);
				freeReplyObject( reply );
				break;
			}

			if (REDIS_REPLY_STRING == reply->type)
				val = reply->str;
			freeReplyObject(reply);
		}
		nret = 0;
	}while(0);

	return nret;
}

int redisOpt::redis_hdel(const char *key, const char *field)
{
	int nret = -1;
	char str[128];
	sprintf(str, "hdel %s %s", key, field);

	do {
		if (bpipelinestart){
			if (REDIS_OK != redisAppendCommand(c, str)) {
				break;
			}
			++pipeline_cmdnum;
		}
		else {
			boost::mutex::scoped_lock lock(redis_mutex_);
			if (connect() < 0) {
				break;
			}

			reply = (redisReply*)redisCommand(c, str);
			if (NULL == reply){
				LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
				redisFree(c);
				c = NULL;
				break;
			}

			if (REDIS_REPLY_ERROR == reply->type) {
				LOG_PRINT(log_error, "err_str:%s", reply->str);
				freeReplyObject( reply );
				break;
			}

			freeReplyObject(reply);
		}
		nret = 0;
	}while(0);

	return nret;
}

int redisOpt::redis_hset(const char *key, const string &field, string &val)
{
	int nret = -1;
	char str[128];
	sprintf(str, "hset %s %s %s", key, field.c_str(), val.c_str());

	do {
		if (bpipelinestart){
			if (REDIS_OK != redisAppendCommand(c, str)) {
				break;
			}
			++pipeline_cmdnum;
		}
		else {
			boost::mutex::scoped_lock lock(redis_mutex_);
			if (connect() < 0) {
				break;
			}

			reply = (redisReply*)redisCommand(c, str);
			if (NULL == reply){
				LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
				redisFree(c);
				c = NULL;
				break;
			}

			if (REDIS_REPLY_ERROR == reply->type) {
				LOG_PRINT(log_error, "err_str:%s", reply->str);
				freeReplyObject( reply );
				break;
			}

			freeReplyObject(reply);
		}
		nret = 0;
	}while(0);

	return nret;
}

int redisOpt::redis_hset(const char *key, const char *field, const char *val)
{
	if (!field || !val)
		return -1;

	string sfield = field;
	string sval = val;

	return redis_hset(key, sfield, sval);
}

int redisOpt::redis_hmget(const string &key, const string &fields, vector<string> &vals)
{
	int nret = -1;
	vals.clear();
	char *str = (char*)malloc(fields.size() + 32);
	sprintf(str, "hmget %s %s", key.c_str(), fields.c_str());

	do {
		if (bpipelinestart){
			if (REDIS_OK != redisAppendCommand(c, str)) {
				break;;
			}
			++pipeline_cmdnum;
		}
		else {
			boost::mutex::scoped_lock lock(redis_mutex_);
			if (connect() < 0) {
				break;
			}

			reply = (redisReply*)redisCommand(c, str);
			if (NULL == reply){
				LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
				redisFree(c);
				c = NULL;
				break;
			}

			if (REDIS_REPLY_ERROR == reply->type) {
				LOG_PRINT(log_error, "err_str:%s", reply->str);
				freeReplyObject( reply );
				break;
			}

			for (size_t i=0; i<reply->elements; i++){
				if (reply->element[i]->type ==  REDIS_REPLY_STRING)
					vals.push_back(reply->element[i]->str);
			}
			freeReplyObject(reply);
		}
		nret = 0;
	}while(0);

	if (str)
		free(str);

	return nret;
}

int redisOpt::redis_hmget(const char *key, const string &fields, vector<string> &vals)
{
	string skey = key;
	return redis_hmget(skey, fields, vals);
}

int redisOpt::redis_hgetall(const string &key, vector<string> &fields, vector<string> &vals)
{
	int ret = redis_hgetallfields(key, fields);
	ret = redis_hgetallvalues(key, vals);
	return ret;
}

int redisOpt::redis_hgetall(const char *key, vector<string> &fields, vector<string> &vals)
{
	string skey = key;
	return redis_hgetall(skey, fields, vals);
}

int redisOpt::redis_hgetall(const char *key, map<string,string> &mapKVs)
{
	int nret = -1;
	do {
		if (bpipelinestart) {
			if (REDIS_OK != redisAppendCommand(c, "HGETALL %s", key)) {
				break;
			}
			++pipeline_cmdnum;
		}
		else {
			mapKVs.clear();
			boost::mutex::scoped_lock lock(redis_mutex_);
			if (connect() < 0) {
				break;
			}

			reply = (redisReply*)redisCommand(c,"HGETALL %s", key);
			if (NULL == reply) {
				LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
				redisFree(c);
				c = NULL;
				break;
			}

			if (REDIS_REPLY_ERROR == reply->type) {
				LOG_PRINT(log_error, "err_str:%s", reply->str);
				freeReplyObject(reply);
				break;
			}

			for (size_t i=0; i<reply->elements; i+=2){
				if (reply->element[i]->type ==  REDIS_REPLY_STRING && reply->element[i+1]->type ==  REDIS_REPLY_STRING)
					mapKVs[reply->element[i]->str] = reply->element[i+1]->str;
			}
			freeReplyObject(reply);
		}
		nret = 0;
	} while(0);

	return nret;
}

int redisOpt::redis_hgetallfields(const string &key, vector<string> &fields)
{
	int nret = -1;
	do {
		if (bpipelinestart) {
			if (REDIS_OK != redisAppendCommand(c, "HKEYS %s", key.c_str())) {
				break;
			}
			++pipeline_cmdnum;
		}
		else {
			boost::mutex::scoped_lock lock(redis_mutex_);
			fields.clear();

			if (connect() < 0) {
				break;
			}

			reply = (redisReply*)redisCommand(c,"HKEYS %s", key.c_str());
			if (NULL == reply){
				LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
				redisFree(c);
				c = NULL;
				break;;
			}

			if (REDIS_REPLY_ERROR == reply->type) {
				LOG_PRINT(log_error, "err_str:%s", reply->str);
				freeReplyObject( reply );
				break;
			}

			for (size_t i=0; i<reply->elements; i++){
				if (reply->element[i]->type ==  REDIS_REPLY_STRING)
					fields.push_back(reply->element[i]->str);
			}
			freeReplyObject(reply);
		}
		nret = 0;
	}while(0);

	return nret;
}

int redisOpt::redis_hgetallvalues(const string &key, vector<string> &vals)
{
	int nret = -1;
	do {
		if (bpipelinestart) {
			if (REDIS_OK != redisAppendCommand(c, "HVALS %s", key.c_str())) {
				break;
			}
			++pipeline_cmdnum;
		}
		else {
			vals.clear();
			boost::mutex::scoped_lock lock(redis_mutex_);
			if (connect() < 0) {
				break;
			}

			reply = (redisReply*)redisCommand(c,"HVALS %s", key.c_str());
			if (NULL == reply) {
				LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
				redisFree(c);
				c = NULL;
				break;
			}

			if (REDIS_REPLY_ERROR == reply->type) {
				LOG_PRINT(log_error, "err_str:%s", reply->str);
				freeReplyObject(reply);
				break;
			}

			for (size_t i=0; i<reply->elements; i++){
				if (reply->element[i]->type ==  REDIS_REPLY_STRING)
					vals.push_back(reply->element[i]->str);
			}
			freeReplyObject(reply);
		}
		nret = 0;
	} while(0);

	return nret;
}

int redisOpt::redis_hexists(const char *key, const char *field)
{
	int nret = -1;
	do {
		if (bpipelinestart) {
			if (REDIS_OK != redisAppendCommand(c, "HEXISTS %s %s", key, field)) {
				break;
			}
			++pipeline_cmdnum;
		}
		else {
			boost::mutex::scoped_lock lock(redis_mutex_);
			if (connect() < 0) {
				break;
			}

			reply = (redisReply*)redisCommand(c, "HEXISTS %s %s", key, field);
			if (NULL == reply) {
				LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
				redisFree(c);
				c = NULL;
				break;
			}

			if (REDIS_REPLY_ERROR == reply->type) {
				LOG_PRINT(log_error, "err_str:%s", reply->str);
				freeReplyObject(reply);
				break;
			}

			if (REDIS_REPLY_INTEGER == reply->type) {
				nret = reply->integer;
			}
			freeReplyObject(reply);
		}
	} while(0);

	return nret;
}

int redisOpt::redis_hincrby(const char *key, const char *field, int increment)
{
	int nret = -1;
	do {
		if (bpipelinestart) {
			if (REDIS_OK != redisAppendCommand(c, "HINCRBY %s %s %d", key, field, increment)) {
				break;
			}
			++pipeline_cmdnum;
		}
		else {
			boost::mutex::scoped_lock lock(redis_mutex_);
			if (connect() < 0) {
				break;
			}

			reply = (redisReply*)redisCommand(c, "HINCRBY %s %s %d", key, field, increment);
			if (NULL == reply) {
				LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
				redisFree(c);
				c = NULL;
				break;
			}

			if (REDIS_REPLY_ERROR == reply->type) {
				LOG_PRINT(log_error, "err_str:%s", reply->str);
				freeReplyObject(reply);
				break;
			}

			if (REDIS_REPLY_INTEGER == reply->type) {
				nret = reply->integer;
			}
			freeReplyObject(reply);
		}
	} while(0);

	return nret;
}

int redisOpt::redis_sadd(const string &key, string &members)
{
	int nret = -1;
	char *str = (char*)malloc(members.size() + 32);
	sprintf(str, "sadd %s %s", key.c_str(), members.c_str());

	do {
		if (bpipelinestart) {
			if (REDIS_OK != redisAppendCommand(c, str))
				break;
			++pipeline_cmdnum;
		}
		else {
			boost::mutex::scoped_lock lock(redis_mutex_);
			if (connect() < 0)
				break;

			reply = (redisReply*)redisCommand(c, str);
			if (NULL == reply){
				LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
				redisFree(c);
				c = NULL;
				break;
			}

			if (REDIS_REPLY_ERROR == reply->type) {
				LOG_PRINT(log_error, "err_str:%s", reply->str);
				freeReplyObject(reply);
				break;
			}
			freeReplyObject(reply);
		}
		nret = 0;
	} while(0);

	if (str) {
		free(str);
	}

	return nret;
}

int redisOpt::redis_sadd(const char *key, const char *members)
{
	string skey = key;
	string smembers = members;

	return redis_sadd(skey, smembers);
}

int redisOpt::redis_srem(const string &key, string &member)
{
	int nret = -1;
	do {
		if (bpipelinestart) {
			if (REDIS_OK != redisAppendCommand(c, "srem %s %s", key.c_str(), member.c_str())) {
				break;
			}
			++pipeline_cmdnum;
			nret = 0;
		}
		else {
			boost::mutex::scoped_lock lock(redis_mutex_);
			if (connect() < 0)
				break;

			reply = (redisReply*)redisCommand(c, "srem %s %s", key.c_str(), member.c_str());
			if (NULL == reply){
				LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
				redisFree(c);
				c = NULL;
				break;
			}

			if (REDIS_REPLY_ERROR == reply->type) {
				LOG_PRINT(log_error, "err_str:%s", reply->str);
				freeReplyObject( reply );
				break;
			}
			if (reply->integer) {
				nret = 0;
			}
			freeReplyObject(reply);
		}
	}while(0);

	return nret;
}

int redisOpt::redis_srem(const char *key, const char *member)
{
	string skey = key;
	string smember = member;
	return redis_srem(skey, smember);
}

int redisOpt::redis_smembers(const string &key, vector<string> &members)
{
	int nret = -1;
	do {
		if (bpipelinestart) {
			if (REDIS_OK != redisAppendCommand(c, "SMEMBERS %s", key.c_str())) {
				break;
			}
			++pipeline_cmdnum;
		}
		else {
			boost::mutex::scoped_lock lock(redis_mutex_);
			members.clear();

			if (connect() < 0)
				break;

			reply = (redisReply*)redisCommand(c, "SMEMBERS %s", key.c_str());
			if (NULL == reply){
				LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
				redisFree(c);
				c = NULL;
				break;
			}

			if (REDIS_REPLY_ERROR == reply->type) {
				LOG_PRINT(log_error, "err_str:%s", reply->str);
				freeReplyObject( reply );
				break;
			}

			for (size_t i=0; i<reply->elements; i++){
				if (reply->element[i]->type ==  REDIS_REPLY_STRING)
					members.push_back(reply->element[i]->str);
			}
			freeReplyObject(reply);
		}
		nret = 0;
	}while(0);

	return nret;
}

int redisOpt::redis_smembers(const char *key, vector<string> &members)
{
	string sKey = key;
	return redis_smembers(sKey, members);
}

int redisOpt::redis_scard(const string & key)
{
	std::string cmd = "SCARD " + key;
	int nret = -1;

	do {
		if (bpipelinestart) {
			if (REDIS_OK != redisAppendCommand(c, cmd.c_str())) {
				break;
			}
			++pipeline_cmdnum;
		}
		else {
			boost::mutex::scoped_lock lock(redis_mutex_);
			if (connect() < 0)
				break;

			reply = (redisReply *)redisCommand(c, cmd.c_str());
			if (NULL == reply){
				LOG_PRINT(log_error, "redis cmd:%s err:%d, err_str:%s", cmd.c_str(), c->err, c->errstr);
				redisFree(c);
				c = NULL;
				break;
			}

			if (REDIS_REPLY_ERROR == reply->type) {
				LOG_PRINT(log_error, "redis cmd:%s err_str:%s", cmd.c_str(), reply->str);
				freeReplyObject(reply);
				break;
			}

			nret = reply->integer;
			freeReplyObject(reply);
		}

	}while(0);

	return nret;
}

int redisOpt::redis_hlen(const string & key)
{
	std::string cmd = "HLEN " + key;
	int nret = -1;

	do {
		if (bpipelinestart) {
			if (REDIS_OK != redisAppendCommand(c, cmd.c_str())) {
				break;
			}
			++pipeline_cmdnum;
		}
		else {
			boost::mutex::scoped_lock lock(redis_mutex_);
			if (connect() < 0)
				break;

			reply = (redisReply *)redisCommand(c, cmd.c_str());
			if (NULL == reply){
				LOG_PRINT(log_error, "redis cmd:%s err:%d, err_str:%s", cmd.c_str(), c->err, c->errstr);
				redisFree(c);
				c = NULL;
				break;
			}

			if (REDIS_REPLY_ERROR == reply->type) {
				LOG_PRINT(log_error, "redis cmd:%s err_str:%s", cmd.c_str(), reply->str);
				freeReplyObject(reply);
				break;
			}

			nret = reply->integer;
			freeReplyObject(reply);
		}

	}while(0);

	return nret;
}

int redisOpt::redis_sismember(const string &key, string &member)
{
	int nret = -1;
	do {
		if (bpipelinestart) {
			if (REDIS_OK != redisAppendCommand(c, "SISMEMBER %s %s", key.c_str(), member.c_str())) {
				break;
			}
			nret = 0;
			++pipeline_cmdnum;
		}
		else {
			boost::mutex::scoped_lock lock(redis_mutex_);
			if (connect() < 0)
				break;

			reply = (redisReply*)redisCommand(c, "SISMEMBER %s %s", key.c_str(), member.c_str());
			if (NULL == reply){
				LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
				redisFree(c);
				c = NULL;
				break;
			}

			if (REDIS_REPLY_ERROR == reply->type) {
				LOG_PRINT(log_error, "err_str:%s", reply->str);
				freeReplyObject( reply );
				break;
			}
			nret = reply->integer;	/*1: 存在, 0: 不存在*/
			freeReplyObject(reply);
		}

	}while(0);

	return nret;
}

int redisOpt::redis_delkey(const char *key)
{
	string skey = key;
	return redis_delkey(skey);
}

int redisOpt::redis_delkey(const string &key)
{
	int nret = -1;
	do {
		if (bpipelinestart) {
			if (REDIS_OK != redisAppendCommand(c, "del %s", key.c_str())) {
				break;
			}
			++pipeline_cmdnum;
			nret = 0;
		}
		else {
			boost::mutex::scoped_lock lock(redis_mutex_);
			if (connect() < 0)
				break;

			reply = (redisReply*)redisCommand(c, "del %s", key.c_str());
			if (NULL == reply){
				LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
				redisFree(c);
				c = NULL;
				break;
			}

			if (REDIS_REPLY_ERROR == reply->type) {
				LOG_PRINT(log_error, "err_str:%s", reply->str);
				freeReplyObject( reply );
				break;
			}
			if (reply->integer) { /*删除成功*/
				nret = 0;
			}
			else {
				LOG_PRINT(log_error, "key:%s not exists", key.c_str());
			}

			freeReplyObject(reply);
		}

	}while(0);

	return nret;
}


int redisOpt::redis_delkey(const vector<string> &keys)
{
	int nret = -1;
	do {
		if (keys.size() <= 0){
			LOG_PRINT(log_error, "the size of keys is error.");
			break;
		}

		vector<const char *> argv;
		vector<size_t> argvlen;

		static char cmd[] = "DEL";
		argv.push_back( cmd );
		argvlen.push_back( sizeof(cmd)-1 );
		
		string log;
		for (int i = 0; i < keys.size(); i++)
		{
			log += keys[i];
			log += ' ';
			argv.push_back( keys[i].c_str() );
			argvlen.push_back( keys[i].length());
		}
		
		LOG_PRINT(log_debug, "%s %s", cmd, log.c_str());

		if (bpipelinestart){
			if (REDIS_OK != redisAppendCommandArgv(c, argv.size(), &(argv[0]), &(argvlen[0]))) {
				break;
			}
			++pipeline_cmdnum;
		}
		else {
			if (connect() < 0) {
				break;
			}
			reply = (redisReply*)redisCommandArgv(c, argv.size(), &(argv[0]), &(argvlen[0]) );
			if ( NULL == reply ) {
				LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
				redisFree(c);
				c = NULL;
				break;
			}

			if (REDIS_REPLY_ERROR == reply->type) {
				LOG_PRINT(log_error, "err_str:%s", reply->str);
				freeReplyObject( reply );
				break;
			}

			if (reply->integer) { /*删除成功*/
				nret = 0;
			}
			else {
				LOG_PRINT(log_error, "keys[%s] not exists", log.c_str());
			}

			freeReplyObject( reply );
		}
		nret = 0;
	}while(0);

	return nret;
}

int redisOpt::redis_existkey(const char* key)
{
	int nret = -1;
	do {
		if (bpipelinestart) {
			if (REDIS_OK != redisAppendCommand(c, "EXISTS %s", key)) {
				break;
			}
			++pipeline_cmdnum;
		}
		else {
			boost::mutex::scoped_lock lock(redis_mutex_);
			if (connect() < 0)
				break;

			reply = (redisReply*)redisCommand(c, "EXISTS %s", key);
			if (NULL == reply){
				LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
				redisFree(c);
				c = NULL;
				break;
			}

			if (REDIS_REPLY_ERROR == reply->type) {
				LOG_PRINT(log_error, "err_str:%s", reply->str);
				freeReplyObject( reply );
				break;
			}
			nret = reply->integer;	/*1: 存在, 0: 不存在*/
			freeReplyObject(reply);
		}

	}while(0);

	return nret;
}

int redisOpt::redis_type(const char* key, string &type)
{
	int nret = -1;
	do {
		if (bpipelinestart) {
			if (REDIS_OK != redisAppendCommand(c, "TYPE %s", key)) {
				break;
			}
			++pipeline_cmdnum;
		}
		else {
			boost::mutex::scoped_lock lock(redis_mutex_);
			if (connect() < 0)
				break;

			reply = (redisReply*)redisCommand(c, "TYPE %s", key);
			if (NULL == reply){
				LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
				redisFree(c);
				c = NULL;
				break;
			}

			if (REDIS_REPLY_ERROR == reply->type) {
				LOG_PRINT(log_error, "err_str:%s", reply->str);
				freeReplyObject( reply );
				break;
			}
			type = reply->str;
			freeReplyObject(reply);
			nret = 0;
		}

	}while(0);

	return nret;
}

int redisOpt::del( const string &key )
{
	vector<const char *> argv;
    vector<size_t> argvlen;

    static char cmd[] = "DEL";
	argv.push_back( cmd );
	argvlen.push_back( sizeof(cmd)-1 );

    argv.push_back( key.c_str() );
	argvlen.push_back( key.size() );

	if (connect() < 0)
		return -1;
	reply = (redisReply*)redisCommandArgv(c, argv.size(), &(argv[0]), &(argvlen[0]) );
	if ( NULL == reply ){
		LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
		redisFree(c);
		c = NULL;
		return -1;
	}

	if (REDIS_REPLY_ERROR == reply->type) {
		LOG_PRINT(log_error, "err_str:%s", reply->str);
		freeReplyObject( reply );
		return -1;
	}

	freeReplyObject( reply );
	return 0;
}

int redisOpt::hincrby( const string &key, const vector<string> &fields, const vector<string> &vals )
{
	if (fields.size() != vals.size()){
		LOG_PRINT(log_error, "the length not match for fields and vals");
		return -1;
	}

	vector<const char *> argv;
	vector<size_t> argvlen;

	static char cmd[] = "HINCRBY";
	argv.push_back( cmd );
	argvlen.push_back( sizeof(cmd)-1 );

	argv.push_back( key.c_str() );
	argvlen.push_back( key.size() );

	for (int i=0; i<fields.size(); i++){
		argv.push_back(fields[i].c_str());
		argvlen.push_back(fields[i].size());
		argv.push_back(vals[i].c_str());
		argvlen.push_back(vals[i].size());
	}


	if (connect() < 0)
		return -1;
	reply = (redisReply*)redisCommandArgv(c, argv.size(), &(argv[0]), &(argvlen[0]) );
	if ( NULL == reply ){
		LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
		redisFree(c);
		c = NULL;
		return -1;
	}

	if (REDIS_REPLY_ERROR == reply->type) {
		LOG_PRINT(log_error, "err_str:%s", reply->str);
		freeReplyObject( reply );
		return -1;
	}

	freeReplyObject( reply );
	return 0;
}

int redisOpt::redis_writeMsg(string &msg)
{
	LOG_PRINT(log_debug, "write msg to redis: %s", msg.c_str());
	boost::mutex::scoped_lock lock(redis_mutex_);

	if (connect() < 0)
		return -1;
	reply = (redisReply*)redisCommand(c,"lpush %s %s", KEY_ROOM_MSG, msg.c_str());
	if (NULL == reply){
		LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
		redisFree(c);
		c = NULL;
		return -1;
	}

	if (REDIS_REPLY_ERROR == reply->type) {
		LOG_PRINT(log_error, "err_str:%s", reply->str);
		freeReplyObject( reply );
		return -1;
	}

	freeReplyObject(reply);
	return 0;
}

int redisOpt::redis_writeMsg(const char *msg, int len)
{
	boost::mutex::scoped_lock lock(redis_mutex_);

	if (connect() < 0)
		return -1;
	reply = (redisReply*)redisCommand(c,"lpush %s %b", KEY_ROOM_MSG, msg, len);
	if (NULL == reply){
		LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
		redisFree(c);
		c = NULL;
		return -1;
	}

	if (REDIS_REPLY_ERROR == reply->type) {
		LOG_PRINT(log_error, "err_str:%s", reply->str);
		freeReplyObject( reply );
		return -1;
	}

	freeReplyObject(reply);
	return 0;
}

int redisOpt::redis_readMsg(vector<string> &result)
{
	boost::mutex::scoped_lock lock(redis_mutex_);
	result.clear();
	if (connect() < 0)
		return -1;
	reply = (redisReply*)redisCommand(c,"brpop %s 0", KEY_ROOM_MSG);
	if (NULL == reply){
		LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
		redisFree(c);
		c = NULL;
		return -1;
	}

	if (REDIS_REPLY_ERROR == reply->type) {
		LOG_PRINT(log_error, "err_str:%s", reply->str);
		freeReplyObject( reply );
		return -1;
	}

	for(size_t i=0; i<reply->elements; i++){
		result.push_back(reply->element[i]->str);
	}
	freeReplyObject(reply);

	return 0;
}

int redisOpt::redis_readMsg(char* buf, int &len)
{
	boost::mutex::scoped_lock lock(redis_mutex_);
	if (connect() < 0)
		return -1;
	reply = (redisReply*)redisCommand(c,"brpop %s 0", KEY_ROOM_MSG);
	if (NULL == reply){
		LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
		redisFree(c);
		c = NULL;
		return -1;
	}

	if (REDIS_REPLY_ERROR == reply->type) {
		LOG_PRINT(log_error, "err_str:%s", reply->str);
		freeReplyObject( reply );
		return -1;
	}

	if (2 == reply->elements) {
		memcpy(buf, reply->element[1]->str, reply->element[1]->len);
		len = reply->element[1]->len;
	}

	freeReplyObject(reply);

	return 0;
}

int redisOpt::redis_llen(uint32 &listlen)
{
	boost::mutex::scoped_lock lock(redis_mutex_);
	if (connect() < 0)
		return -1;
	reply = (redisReply*)redisCommand(c,"llen %s", KEY_ROOM_MSG);
	if (NULL == reply){
		LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
		redisFree(c);
		c = NULL;
		return -1;
	}

	if (REDIS_REPLY_ERROR == reply->type) {
		LOG_PRINT(log_error, "err_str:%s", reply->str);
		freeReplyObject( reply );
		return -1;
	}

	if (REDIS_REPLY_INTEGER == reply->type) {
		listlen = reply->integer;
	}

	freeReplyObject(reply);

	return 0;
}


int redisOpt::redis_publish(const char *msg)
{
	boost::mutex::scoped_lock lock(redis_mutex_);

	if (connect() < 0)
		return -1;
	reply = (redisReply*)redisCommand(c,"publish %s %s", CHANNEL_REDIS_SYNC, msg);
	if (NULL == reply){
		LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
		redisFree(c);
		c = NULL;
		return -1;
	}

	if (REDIS_REPLY_ERROR == reply->type) {
		LOG_PRINT(log_error, "err_str:%s", reply->str);
		freeReplyObject( reply );
		return -1;
	}

	freeReplyObject(reply);

	return 0;
}

int redisOpt::redis_publish(const char *msg, size_t len)
{
	boost::mutex::scoped_lock lock(redis_mutex_);

	if (connect() < 0)
		return -1;
	reply = (redisReply*)redisCommand(c,"publish %s %b", CHANNEL_REDIS_SYNC, msg, len);
	if (NULL == reply){
		LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
		redisFree(c);
		c = NULL;
		return -1;
	}

	if (REDIS_REPLY_ERROR == reply->type) {
		LOG_PRINT(log_error, "err_str:%s", reply->str);
		freeReplyObject( reply );
		return -1;
	}

	freeReplyObject(reply);

	return 0;
}

int redisOpt::redis_hvals_set(vector<string> &key_vals, vector<string> &fields)
{
	boost::mutex::scoped_lock lock(redis_mutex_);
    if (key_vals.empty()) return -1;       
    string key;
    key.swap(key_vals[0]);
    key_vals.erase(key_vals.begin());
	return hmset(key, fields, key_vals);
}
      
int redisOpt::redis_hvals_set(string key, string &hvalsName, vector<string> &vals, vector<string> &fields)
{
	boost::mutex::scoped_lock lock(redis_mutex_);
    key = hvalsName + ":" + key;
	return hmset(key, fields, vals);
}

int redisOpt::redis_hvals_set(string key, string &fields, vector<string> &vals)
{
	boost::mutex::scoped_lock lock(redis_mutex_);
	vector<string> vecfields = strToVec(fields, ',');

	return hmset(key, vecfields, vals);
}

int redisOpt::redis_hvals_find(string key, string &hvalsName, vector<string> &vals, string &field)
{
    key = hvalsName + ":" + key;
    return redis_hvals_find(key, vals, field);
}

int redisOpt::redis_hvals_find(string key, vector<string> &vals, string &field)
{
	boost::mutex::scoped_lock lock(redis_mutex_);
	vals.clear();
	if (connect() < 0)
		return -1;

	char *str = (char*)malloc(field.size() + 32);
	sprintf(str, "hmget %s %s", key.c_str(), field.c_str());

	reply = (redisReply*)redisCommand(c, str);
	if (NULL == reply){
		LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
		redisFree(c);
		c = NULL;
		if (str)
			free(str);
		return -1;
	}

	if (REDIS_REPLY_ERROR == reply->type) {
		LOG_PRINT(log_error, "err_str:%s", reply->str);
		freeReplyObject( reply );
		return -1;
	}

	for (size_t i=0; i<reply->elements; i++){
		if (reply->element[i]->type ==  REDIS_REPLY_STRING)
			vals.push_back(reply->element[i]->str);
	}
	freeReplyObject(reply);

	if (str)
		free(str);
	return 0;
}


int redisOpt::redis_hvals_delete(string key, string &hvalsName)
{

	boost::mutex::scoped_lock lock(redis_mutex_);

	if (connect() < 0)
		return -1;

	reply = (redisReply*)redisCommand(c,"del %s:%s", hvalsName.c_str(), key.c_str());
	if (NULL == reply){
		LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
		redisFree(c);
		c = NULL;
		return -1;
	}

	if (REDIS_REPLY_ERROR == reply->type) {
		LOG_PRINT(log_error, "err_str:%s", reply->str);
		freeReplyObject( reply );
		return -1;
	}

	freeReplyObject(reply);

	return 0;
}

int redisOpt::redis_hvals_init(string &hvalsName, vector<string> &fields, HVALSNAME_MAP &hvalsName_map)
{
    pair<HVALSNAME_MAP::iterator,bool> ret;
    ret = hvalsName_map.insert(make_pair(hvalsName, vector<string>(fields)));
    if (!ret.second)
        return -1;
	return 0;
}

int redisOpt::redis_hvals_keys(string &hvalsName, vector<string> &vals)
{
	boost::mutex::scoped_lock lock(redis_mutex_);
	if (connect() < 0)
		return -1;
	int cursor = 0;
	do {	      
    	reply = (redisReply*)redisCommand(c,"SCAN %d match %s:*", cursor, hvalsName.c_str());
    	if (NULL == reply){
    		LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
    		redisFree(c);
    		c = NULL;
    		return -1;
    	}
    	
    	if (reply->type ==  REDIS_REPLY_ARRAY && reply->elements > 1) {    	    
    	    cursor = atoi(reply->element[0]->str);
    	} else {
    	    freeReplyObject(reply);
            return -1;
    	}   
    	    
    	for (size_t i=0; i<reply->element[1]->elements; i++){
    		if (reply->element[1]->element[i]->type ==  REDIS_REPLY_STRING)
    			vals.push_back(reply->element[1]->element[i]->str);
    	}
    } while (cursor);
    
	freeReplyObject(reply);

	return 0;
}


int redisOpt::redis_run_cmdArgv(int argvnum, const char **argv, size_t *argvlen)
{
	if (argvnum < 0 || argv == NULL || argvlen == NULL){
		LOG_PRINT(log_error, "wrong param");
		return -1;
	}
	
	if (connect() < 0)
		return -1;
	reply = (redisReply*)redisCommandArgv(c, argvnum, argv, argvlen);
	if ( NULL == reply ){
		LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
		redisFree(c);
		c = NULL;
		return -1;
	}
	if (REDIS_REPLY_ERROR == reply->type) {
		LOG_PRINT(log_error, "err_str:%s", reply->str);
		freeReplyObject( reply );
		return -1;
	}
	freeReplyObject( reply );
	return 0;
}

int redisOpt::redis_run_cmd(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);	
    int ret = redis_run_cmdv(fmt, ap); 
    va_end(ap);  
    return ret;
}
      
int redisOpt::redis_run_cmdv(const char* fmt, va_list ap)
{

    redisReply* tmp_res = NULL;
    bool ret = redis_run_cmdv(&tmp_res, fmt, ap);
	freeReplyObject(tmp_res);
	return ret; 

	return 0;
}

int redisOpt::redis_run_cmd(redisReply** res, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);	
    int ret = redis_run_cmdv(res, fmt, ap); 
    va_end(ap);  
    return ret;
}

int redisOpt::redis_run_cmdv(redisReply** res, const char* fmt, va_list ap)
{
    boost::mutex::scoped_lock lock(redis_mutex_);    
	if (connect() < 0) return -1;   
	*res = (redisReply*)redisvCommand(c, fmt, ap);
	if (NULL == *res) {
		LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
		redisFree(c);
		c = NULL;
		return -1;
	}	
	if (REDIS_REPLY_ERROR == (*res)->type) {
		LOG_PRINT(log_error, "err_str:%s", (*res)->str);
		return -1;
	}
	return 0;
}

boost::mutex& redisOpt::get_redis_mutex()
{
    return redis_mutex_;
}

int redisOpt::redis_scan_keys(const string &filter, vector<string> &vals)
{
	boost::mutex::scoped_lock lock(redis_mutex_);
	if (connect() < 0)
		return -1;
	int cursor = 0;
	do {	      
		char szcmd[128] = {0};
		if (filter.size())
			sprintf(szcmd, "SCAN %d match %s", cursor, filter.c_str());
		else
			sprintf(szcmd, "SCAN %d", cursor);
    	reply = (redisReply*)redisCommand(c, szcmd);
    	if (NULL == reply){
    		LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
    		redisFree(c);
    		c = NULL;
    		return -1;
    	}

    	if (REDIS_REPLY_ERROR == reply->type) {
    		LOG_PRINT(log_error, "err_str:%s", reply->str);
    		freeReplyObject( reply );
    		return -1;
    	}
    	
    	if (reply->type ==  REDIS_REPLY_ARRAY && reply->elements > 1) {    	    
    	    cursor = atoi(reply->element[0]->str);
    	} else {
    	    freeReplyObject(reply);
            return -1;
    	}   
    	    
    	for (size_t i=0; i<reply->element[1]->elements; i++){
    		if (reply->element[1]->element[i]->type ==  REDIS_REPLY_STRING)
    			vals.push_back(reply->element[1]->element[i]->str);
    	}
    } while (cursor);
    
	freeReplyObject(reply);

	return 0;
}
int redisOpt::redis_keys(const string &filter, vector<string> &vals)
{
	boost::mutex::scoped_lock lock(redis_mutex_);
	if (connect() < 0)
		return -1;
	//int cursor = 0;
	//do {
		char szcmd[128] = { 0 };
		if (filter.size())
			sprintf(szcmd, "KEYS %s*", filter.c_str());
		else
			sprintf(szcmd, "KEYS *");
		reply = (redisReply*)redisCommand(c, szcmd);
		if (NULL == reply){
			LOG_PRINT(log_error, "err:%d, err_str:%s", c->err, c->errstr);
			redisFree(c);
			c = NULL;
			return -1;
		}

		if (REDIS_REPLY_ERROR == reply->type) {
			LOG_PRINT(log_error, "err_str:%s", reply->str);
			freeReplyObject(reply);
			return -1;
		}

		/*if (reply->type == REDIS_REPLY_ARRAY && reply->elements > 1) {
			cursor = atoi(reply->element[0]->str);
		}
		else {
			freeReplyObject(reply);
			return -1;
		}*/

		for (size_t i = 0; i < reply->elements; i++)
		{
			if (reply->element[i]->type == REDIS_REPLY_STRING)
			{
				vals.push_back(reply->element[i]->str);
			}

		}
	//} while (cursor);

	freeReplyObject(reply);

	return 0;
}
int redisPipeline::getreply(vector<string> &vec)
{
	vec.clear();
	if (pRedis_->pipeline_cmdnum <= 0)
		return NO_MORE_REDIS_REPLY;
	pRedis_->pipeline_cmdnum--;

	redisReply *reply;
	int r = redisGetReply(pRedis_->c, (void **)&reply);
	if (r == REDIS_ERR)
	{
		LOG_PRINT(log_error, "REDIS_ERR: %s", pRedis_->c->errstr);
		return -1;
	}
	if (REDIS_REPLY_ERROR == reply->type) {
		LOG_PRINT(log_error, "REDIS_REPLY_ERROR: %s", reply->str);
		freeReplyObject(reply);
		return -1;
	}
	if (REDIS_REPLY_ARRAY == reply->type) {
		for (size_t i = 0; i < reply->elements; i++){
			if (reply->element[i]->type == REDIS_REPLY_STRING)
				vec.push_back(reply->element[i]->str);
		}
	}
	freeReplyObject(reply);
	return 0;
}


int redisPipeline::getreply()
{
	if (pRedis_->pipeline_cmdnum <= 0)
		return NO_MORE_REDIS_REPLY;
	pRedis_->pipeline_cmdnum--;

	redisReply *reply;
	int r = redisGetReply(pRedis_->c, (void **)&reply);
	if (r == REDIS_ERR)
	{
		LOG_PRINT(log_error, "REDIS_ERR: %s", pRedis_->c->errstr);
		return -1;
	}
	if (REDIS_REPLY_ERROR == reply->type) {
		LOG_PRINT(log_error, "REDIS_REPLY_ERROR: %s", reply->str);
		freeReplyObject(reply);
		return -1;
	}
	int ret = 0;
	if (REDIS_REPLY_INTEGER == reply->type) {
		if (reply->integer)
			ret = 0;
		else
			ret = -1;
	}
	else {
		ret = 0;
	}
	freeReplyObject(reply);
	return ret;
}

int redisPipeline::getallreply(map<string, string> &mValues)
{
	if (pRedis_->pipeline_cmdnum <= 0)
		return NO_MORE_REDIS_REPLY;

	while (pRedis_->pipeline_cmdnum--)
	{

		redisReply *reply;
		int r = redisGetReply(pRedis_->c, (void **)&reply);
		if (r == REDIS_ERR)
		{
			LOG_PRINT(log_error, "REDIS_ERR: %s", pRedis_->c->errstr);
			return -1;
		}
		if (REDIS_REPLY_ERROR == reply->type) {
			LOG_PRINT(log_error, "REDIS_REPLY_ERROR: %s", reply->str);
			freeReplyObject(reply);
			return -1;
		}
		int ret = 0;
		if (REDIS_REPLY_INTEGER == reply->type) {
			ret = 0;
		}
		else if (REDIS_REPLY_ARRAY == reply->type) {
			for (size_t i = 0; i < reply->elements / 2 * 2; i += 2){
				if (reply->element[i]->type == REDIS_REPLY_STRING && reply->element[i + 1]->type == REDIS_REPLY_STRING)
					mValues[reply->element[i]->str] = reply->element[i + 1]->str;
			}
		}
		freeReplyObject(reply);
	}
	return 0;
}

int redisPipeline::hmset(const string &key, const vector<string> &fields, const vector<string> &vals)
{
	int nret = -1;
	do {
		if (fields.size() != vals.size()){
			LOG_PRINT(log_error, "the length not match for fields and vals");
			break;
		}

		vector<const char *> argv;
		vector<size_t> argvlen;

		argv.push_back("HMSET");
		argvlen.push_back(sizeof("HMSET") - 1);
		argv.push_back(key.c_str());
		argvlen.push_back(key.size());

		for (size_t i = 0; i < fields.size(); i++) {
			argv.push_back(fields[i].c_str());
			argvlen.push_back(fields[i].size());
			argv.push_back(vals[i].c_str());
			argvlen.push_back(vals[i].size());
		}

		if (REDIS_OK != redisAppendCommandArgv(pRedis_->c, argv.size(), &(argv[0]), &(argvlen[0]))) {
			break;
		}
		++pRedis_->pipeline_cmdnum;
		nret = 0;
	} while (0);

	return nret;
}

int redisPipeline::hmset(const string &key, const map<string, string> &mValues)
{
	int nret = -1;
	do {
		if (mValues.empty()){
			LOG_PRINT(log_error, "mValues is empty");
			break;
		}

		vector<const char *> argv;
		vector<size_t> argvlen;

		argv.push_back("HMSET");
		argvlen.push_back(sizeof("HMSET") - 1);
		argv.push_back(key.c_str());
		argvlen.push_back(key.size());

		map<string, string>::const_iterator iter = mValues.begin();
		for (; iter != mValues.end(); iter++) {
			argv.push_back(iter->first.c_str());
			argvlen.push_back(iter->first.size());
			argv.push_back(iter->second.c_str());
			argvlen.push_back(iter->second.size());
		}

		if (REDIS_OK != redisAppendCommandArgv(pRedis_->c, argv.size(), &(argv[0]), &(argvlen[0]))) {
			break;
		}
		++pRedis_->pipeline_cmdnum;
		nret = 0;
	} while (0);

	return nret;
}

