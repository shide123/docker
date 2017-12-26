#include "SyncServerApplication.h"
#include "Config.h"
#include <assert.h>
#include "server.h"
#include "connmanager.h"
#include "utils.h"
#include "message_vchat.h"
#include "basic_redis.h"
#include "redisdataHelper.h"
#include "timestamp.h"

SyncServerApplication::SyncServerApplication(void)
	: m_pSyncsvr(NULL)
	, m_pConnMgr(NULL)
	, m_pRedisMgr(NULL)
	, m_pRedisMsg(NULL)
	, m_pRedisPub(NULL)
	, m_sync_status(SYNC_INIT)
	, m_nAlarmport(0)
	, m_msg_threadnum(1)
	, m_nserverid(0)
	, m_bdaemon(0)
	, m_bsyncing(0)
	, m_listen_port(0)
	, m_msgindex(0)
	, m_bdbinited(0)
	, m_nDBauthedTime(0)
	, m_synclogfile(NULL)
	, m_last_printstackinfotime(0)
	, deadline_(NULL)
{
    m_Sub_Hvals[Sub_Vchat_RedisRoomMgrInfo_Req] = KEY_HASH_ROOM_INFO;
    m_SubReq_SubResp[Sub_Vchat_RedisRoomMgrInfo_Req] = Sub_Vchat_RedisRoomMgrInfo_Resp;
}


SyncServerApplication::~SyncServerApplication(void)
{
	if (m_pRedisMsg)
		delete m_pRedisMsg;

	if (m_pRedisPub)
		delete m_pRedisPub;

	if (m_pRedisMgr)
		delete m_pRedisMgr;

	if (m_pConnMgr)
		delete m_pConnMgr;

	if (m_synclogfile)
		fclose(m_synclogfile);

	if (deadline_)
		delete deadline_;
}

int SyncServerApplication::init()
{
	CAlarmNotify::init(m_sAlarmhost, m_nAlarmport);

	// must first init redis
	if (initRedis() < 0)
		return EXIT_FAILURE;

	//clearRedis();

	m_synclogfile = fopen(m_strsynclogpath.c_str(), "w+");
	if (!m_synclogfile){
		LOG_PRINT(log_error, "Failed to open file: %s, error: %s", strerror(errno));
		return -1;
	}

	m_online_task.open(m_msg_threadnum, 100000, 10);
	string monitor_dir = m_strbinpath + "../etc";
	m_inotify.startup(monitor_dir.c_str());

	m_pConnMgr = new connmgr();

	rdsMsgThread.start(redis_msg_thread_proc, this);
	return 0;
}

int SyncServerApplication::initRedis()
{
	m_pRedisMgr = new redisMgr(m_strRedisHost.c_str(), m_nRedisPort, m_sRedisPass.c_str(), 2);
	if (!m_pRedisMgr)
		return -1;

	m_pRedisMsg = new redisOpt(m_strRedisHost.c_str(), m_nRedisPort, false, m_sRedisPass.c_str());
	if (!m_pRedisMsg)
		return -1;

	if (m_pRedisMsg->runloop() < 0){
		LOG_PRINT(log_error, "Failed to connect to redis server: %s:%u!", m_strRedisHost.c_str(), m_nRedisPort);
		return -1;
	}

	m_pRedisPub = new redisOpt(m_strRedisHost.c_str(), m_nRedisPort, false, m_sRedisPass.c_str());
	if (m_pRedisPub->runloop() < 0){
		LOG_PRINT(log_error, "Failed to connect to redis server: %s:%u!", m_strRedisHost.c_str(), m_nRedisPort);
		return -1;
	}
//
//	m_pRedisData = new redisOpt(m_strRedisHost.c_str(), m_nRedisPort, false, m_sRedisPass.c_str());
//	if (!m_pRedisData)
//		return -1;

    string room_info_field("roomname,busepwd,strpwd,nlevel,nopstate,nvcbid,ngroupid,nseats,ncreatorid,nopuserid0,nopuserid1,nopuserid2,nopuserid3,roomnotice0,roomnotice1,roomnotice2,roomnotice3");
    string room_info_key(KEY_HASH_ROOM_INFO);
    vector<string> fields;
    fields = strToVec(room_info_field, ',');
    m_pRedisMgr->getOne()->redis_hvals_init(room_info_key, fields, m_hvalsName_map);
    
	return 0;
}

int SyncServerApplication::connect_sync_server(boost::asio::io_service &ioservice, server *pserver)
{
	deadline_ = new deadline_timer(ioservice);
	deadline_->expires_from_now(boost::posix_time::seconds(10)); //最小间隔 en_checkactivetime 检查
	deadline_->async_wait(boost::bind(&SyncServerApplication::check_deadline, this, boost::asio::placeholders::error));

	vector<string> addrvec = strToVec(m_sSyncsvrAddr, ' ');
	for (size_t i = 0; i < addrvec.size(); i++)
	{
		string ip;
		uint16 port;
		size_t pos = addrvec[i].find(':');
		if (pos != string::npos){
			ip = addrvec[i].substr(0, pos);
			port = std::strtol(addrvec[i].substr(pos+1).c_str(), NULL, 0);
			if (0 == port)
				continue;

			connecthandler_ptr svr_client_;
			svr_client_.reset(new connecthandler(ioservice));
			svr_client_->start_connect(ip.c_str(), port, pserver);
		}
	}
	return 0;
}

int SyncServerApplication::clearRedis()
{
	return m_pRedisMgr->getOne()->redis_FlushDB();
	return 0;
}

int SyncServerApplication::loadconfig(const char *confile)
{
	if (!confile || *confile == 0)
		return -1;

	int line;
	Config config(confile);
	if(!config.load(line)){
		std::cerr << "load configure file " << confile << " failed, error: " << strerror(errno) << endl;
		return -1;
	}
	// 自己相关的
	m_nserverid = config.getInt("self","svrid");
	m_listen_port = config.getInt("self","port");
	m_bdaemon = config.getInt("self", "daemon");
	m_msg_threadnum = config.getInt("self","workthread");
	m_appPath = config.getString("launch_app", "path");

	m_sSyncsvrAddr = config.getString("sync_node","addr");

	// alarm
	m_sAlarmhost = config.getString("alarm", "ip");
	m_nAlarmport = config.getInt("alarm", "port");

	// redis配置信息
	m_strRedisHost = config.getString("redis", "ip");
	m_nRedisPort = config.getInt("redis", "port");
	m_sRedisPass = config.getString("redis", "password");

	//初始化日志线程
	m_nlogmode = config.getInt("log", "logmode");
	m_nloglevel = config.getInt("log", "loglevel");
	m_nusesyslog = config.getInt("log", "usesyslog");

	return 0;
}

clienthandler* SyncServerApplication::getgateclient(uint16_t gateid)
{
	if (m_pSyncsvr)
		return m_pSyncsvr->getgateclient(gateid);

	return NULL;
}

void* SyncServerApplication::redis_msg_thread_proc(void* arg)
{
	SyncServerApplication *pApp = (SyncServerApplication*)arg;
	if (!pApp)
		return NULL;

	char buffer[MSG_LEN];
	int buflen = 0;
	int msgid, datatype;
	uint32_t nflags = 0;
	while(true){
		if (0 == pApp->m_pRedisMsg->redis_readMsg(buffer, buflen)){
			if (buflen <= 0)
				continue;

			Json::Reader reader(Json::Features::strictMode());
			Json::Value root;
			if (!reader.parse(buffer, root)) {
				LOG_PRINT(log_error, "invalid Json format: %s", buffer);
				continue;
			}

			Json::Value head = root["head"];
			nflags = head["flags"].asUInt();
			if (appInstance->done_redis_msg(buffer, buflen, root, nflags & WRITE_LOCAL_REDIS) < 0)
				continue;
			if (nflags & SYNC_REDIS_MSG) {
				pApp->syncsvr_notify(buffer, buflen);
			}
			if (nflags & PUBLISH_LOCAL) {
				pApp->m_pRedisPub->redis_publish(buffer, buflen);
			}

		}
	}
}

void SyncServerApplication::syncsvr_notify(const char* data, int len)
{
	SL_ByteBuffer buf(SIZE_IVM_INDEX_HEADER + sizeof(uint) + sizeof(CMDRedisData_t) + len);
	COM_MSG_INDEX_HEADER* pHeader = (COM_MSG_INDEX_HEADER*)buf.buffer();
	pHeader->version = 10;
	pHeader->checkcode = PACK_REQ;
	pHeader->maincmd = MDM_Vchat_Redis_Sync;
	pHeader->subcmd = Sub_Vchat_RedisSync_data_Resp;
	pHeader->index = new_index();
	uint itemsize = 1;
	memcpy(pHeader->content, &itemsize, sizeof(uint));
	CMDRedisData_t *pMsg = (CMDRedisData_t *)(pHeader->content + sizeof(uint));
	pMsg->length = len;
	memcpy(pMsg->content, data, len);
	pHeader->length = SIZE_IVM_INDEX_HEADER + sizeof(uint) + sizeof(CMDRedisData_t) + len;
	buf.data_end(pHeader->length);

    sockethandler::conns_Mutex.lock();
    sockethandler::CONN_MAP::iterator it = sockethandler::conns.begin();
    while (it != sockethandler::conns.end()){
    	sockethandler* phandler = it->second;
    	if (phandler && !phandler->isactived()){
    		clienthandler* pclient = (clienthandler*)phandler;
    		if (pclient && pclient->remote_sync_status == SYNC_READY){
    			pclient->write_message(buf.data(), buf.data_size());
    			struct timeval tv;
    			char stime[32];
    			gettimeofday(&tv, 0);
    			toStringTimestamp6(&tv, stime);
				LOG_PRINT(log_info, "[index:%u,subcmd:%u,send time:%s]send sync data to %s:%u, {%s}", pHeader->index, pHeader->subcmd, stime,
			    						pclient->getremote_ip(), pclient->getremote_port(), pMsg->content);

    		}
    	}
    	++it;
    }
    sockethandler::conns_Mutex.unlock();
}

int SyncServerApplication::done_redis_msg(const char *buffer, int buflen, Json::Value &root, bool bwrite)
{
	Json::Value head = root["head"];
	Json::Value body = root["body"];
	if (head.empty() || body.empty()) {
		LOG_PRINT(log_info, "Json head or body is empty");
		return -1;
	}
	Json::Int msgid = head["msgid"].asInt();
	Json::Int datatype = head["datatype"].asInt();
	Json::Int action = head["action"].asInt();
	uint32 nflags = head["flags"].asUInt();
	int db = head["redisdb"].asInt();
	Json::FastWriter writer;
	string out = writer.write(root);
	LOG_PRINT(log_info, "[msgid:%u] start handling redis msg: {%s}", msgid, out.c_str());
	bool bwritelog = false;

	string vcbid, userid, nsvrid, skey;
	vector<string> vecField, vecValue;
	skey = head["key"].asString();
	JsonToVec(body, vecField, vecValue);
	redisOpt *pRedis = appInstance->m_pRedisMgr->getOne(db);
	if (!pRedis) {
		LOG_PRINT(log_error, "pRedis is NULL");
		return -1;
	}
	// write redis data
	if (bwrite) {
		switch(datatype){
		case REDIS_HASH_CMD: {
			if (REDIS_ADD == action) {
				if (pRedis->redis_hmset(skey, vecField, vecValue) < 0) {
					LOG_PRINT(log_error, "Failed to write data to redis, key:%s", skey.c_str());
					return -1;
				}
			}
			else if (REDIS_UPDATE == action) {
				if (1 == pRedis->redis_existkey(skey.c_str())) {
					if (pRedis->redis_hmset(skey, vecField, vecValue) < 0) {
						LOG_PRINT(log_error, "Failed to write data to redis, key:%s", skey.c_str());
						return -1;
					}
				}
			}
			else if (REDIS_REM == action) {
				if (pRedis->redis_delkey(skey) < 0) {
					LOG_PRINT(log_error, "Failed to delete data from redis, key:%s", skey.c_str());
					return -1;
				}
			}
			else if (REDIS_INCRBY == action) {
				bwritelog = false;
				for (size_t i=0; i<vecField.size(); i++) {
					string value;
					if (0 == pRedis->redis_hget(skey.c_str(), vecField[i].c_str(), value)) {
						if ("0" == value && atoi(vecValue[i].c_str()) < 0)
							continue;
						if (pRedis->redis_hincrby(skey.c_str(), vecField[i].c_str(), atoi(vecValue[i].c_str())) < 0) {
							LOG_PRINT(log_error, "Failed to hincrby cmd, key:%s,field:%s,inceasement:%d", skey.c_str(), vecField[i].c_str(),
									atoi(vecValue[i].c_str()));
							return -1;
						}
					}
				}
			}
			break;
		}
		case REDIS_SET_CMD: {
			bwritelog = true;
			string members = vecToStr(vecValue);
			if (REDIS_ADD == action) {
				if (pRedis->redis_sadd(skey, members) < 0) {
					LOG_PRINT(log_error, "Failed to write user id to redis room key:%s, members:%s", skey.c_str(), members.c_str());
					return -1;
				}
			}
			else if (REDIS_REM == action) {
				if (pRedis->redis_srem(skey, members) < 0) {
					LOG_PRINT(log_error, "Failed to delete user id from redis room key:%s, members:%s", skey.c_str(), members.c_str());
					return -1;
				}
			}
			break;
		}
		}
	}


	switch(msgid){
	case BASIC_SYNC:
		{
			string cmd = body["cmd"].asString();
			redisOpt *pRedis = appInstance->m_pRedisMgr->getOne(db);
			LOG_PRINT(log_info, "db: %d cmd: %s",db, cmd.c_str());
			pRedis->redis_run_cmd(cmd.c_str());
			break;
		}
	}

	if (bwritelog) {
		size_t len = 0;
		appInstance->new_synclog_seq();
		appInstance->m_synclogfile_Mutex.lock();
		len = fwrite(&buflen, 1, sizeof(buflen), appInstance->m_synclogfile);
		len = fwrite(buffer, 1, buflen, appInstance->m_synclogfile);
		fflush(appInstance->m_synclogfile);
		appInstance->m_synclogfile_Mutex.unlock();
	}

	return 0;
}


void SyncServerApplication::set_sync_master_ip(const char* ip)
{
	boost::mutex::scoped_lock lock(m_sync_master_ip_lock);
	if (ip && *ip && m_sync_master_ip.empty()){
		m_sync_master_ip = ip;
	}
}

const string &SyncServerApplication::get_sync_master_ip()
{
	boost::mutex::scoped_lock lock(m_sync_master_ip_lock);
	return m_sync_master_ip;
}

connecthandler_ptr SyncServerApplication::get_conn_by_ip(const char* ip)
{
	return m_pConnMgr->get_conn_by_ip(ip);
}

redisHost SyncServerApplication::get_redishosts(const string &addrinfo)
{
	redisHost host;
	if (addrinfo.empty()) {
		return host;
	}
	vector<string> vecHostInfo = strToVec(addrinfo, ':');
	host = { vecHostInfo[0], atoi(vecHostInfo[1].c_str()), vecHostInfo[2] };
	return host;
}

void SyncServerApplication::check_deadline(const boost::system::error_code& e)
{
	if(!e)
	{
		if (sockethandler::conns.size() == 0 && SYNC_READY != m_sync_status){
			m_sync_status = SYNC_READY;
			LOG_PRINT(log_info, "set local sync status: SYNC_READY");
		}
		deadline_->expires_from_now(boost::posix_time::seconds(1)); //最小间隔 en_checkactivetime 检查
		deadline_->async_wait(boost::bind(&SyncServerApplication::check_deadline, this, boost::asio::placeholders::error));

		time_t now = time(NULL);
		if (now % 30 == 0) {
			uint32 llen = 0;
			if (0 == m_pRedisMgr->getOne()->redis_llen(llen)) {
				LOG_PRINT(log_info, "redis msg(list) queue length: %u", llen);
			}
		}
		if (0 == m_pConnMgr->conn_number()){
			launchapp("roomsvr");
		}
	}
	else if(e) //error
	{
	}
}

void SyncServerApplication::launchapp(const char* appname)
{
//	FILE *fp = NULL;
//	char cmd[64] = {0};
//	char buf[128] = {0};
//	sprintf(cmd, "pidof %s", appname);
//	if(NULL == (fp = popen(cmd, "r")) ){
//		LOG_PRINT(log_error, "failed to call fopen");
//		return;
//	}
//
//	int rtn;
//	fgets(buf, sizeof(buf), fp);
//	pclose(fp);
//	if (0 == buf[0] && !m_appPath.empty()){
//		pid_t pid = fork();
//		if (pid < 0){
//			LOG_PRINT(log_error, "failed to fork");
//		}
//		else if (0 == pid){
//			execle(m_appPath.c_str(), m_appPath.c_str(), NULL, NULL);
//			exit(0);
//		}
//		wait(&rtn);
//	}
}

