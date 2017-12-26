
#include "TaskNetMsgProc.h"
#include  "SyncServerApplication.h"

#include "crc32.h"
#include "utils.h"
#include "basic_redis.h"


const uint16_t GROUPSIZE = 20;
//////////////////////////////////////////////////////////////////////////
CTaskNetMsgProc::CTaskNetMsgProc(void)
{
}

CTaskNetMsgProc::~CTaskNetMsgProc(void)
{
}

int CTaskNetMsgProc::svc(SL_ByteBuffer &buf, void *svc_data /* = NULL */, bool *change_svc_data /* = NULL */)
{
	//60秒打印一次队列信息###################
	static int nproc_message_count = 0;
	if(time(0)- appInstance->m_last_printstackinfotime > 30)
	{
		LOG_PRINT(log_debug, "------------------------------");
		LOG_PRINT(log_debug, "runner_handler-size = %d, msg_queue-size = %d, proc_msg_count = %d.",
			sockethandler::conns.size(), this->getq_size(),nproc_message_count );
		LOG_PRINT(log_debug, "------------------------------");
		appInstance->m_last_printstackinfotime = time(0);
		nproc_message_count = 0;
	}
	nproc_message_count ++;
	//########################################

	MSG_PROC_TASK_NODE *task_node = (MSG_PROC_TASK_NODE *)(buf.buffer());
	if (task_node->task_type == TASK_TYPE_REDISSYNC_MSG){
		COM_MSG_INDEX_HEADER* in_msg = (COM_MSG_INDEX_HEADER*)task_node->pdata;
		uint *pError = NULL;
		if (in_msg->maincmd == MDM_Vchat_Redis_Sync){
			switch(in_msg->subcmd){
			case Sub_SyncData_req:
				proc_syncdata_req(task_node);
				break;
			case Sub_SyncData_resp:
			case Sub_SyncData_Last_resp:
				proc_syncdata_resp(task_node);
				break;

			case Sub_Vchat_RedisSync_Err_Resp:
				pError = (uint*)in_msg->content;
				if (ERR_SYNC_NOT_READY == *pError ) {
					LOG_PRINT(log_error, "redis sync error: sync server is not ready");
				}
				else {
					LOG_PRINT(log_error, "redis sync error: %u", *pError);
				}
				break;
			case Sub_Vchat_RedisSync_data_Resp:
			case Sub_Vchat_RedisSync_data_Complete:
				proc_handle_redismsg(task_node);
				break;
			default:
				return 0;
			}
		}
	}

	return 0;
}

int CTaskNetMsgProc::proc_syncdata_req(MSG_PROC_TASK_NODE *task_node)
{
	COM_MSG_INDEX_HEADER* in_msg = (COM_MSG_INDEX_HEADER*)task_node->pdata;
	if (appInstance->m_sync_status != SYNC_READY){
		LOG_PRINT(log_info, "recv sync data request, but sync server is not ready");
		int newsize = SIZE_IVM_INDEX_HEADER + sizeof(uint);
		SL_ByteBuffer outbuf(newsize+1);
		COM_MSG_INDEX_HEADER* pmsgheader = (COM_MSG_INDEX_HEADER*)outbuf.buffer();
		pmsgheader->version =10;
		pmsgheader->checkcode = PACK_ORDINARY;
		pmsgheader->maincmd = MDM_Vchat_Redis_Sync;
		pmsgheader->subcmd = Sub_Vchat_RedisSync_Err_Resp;
		pmsgheader->index = in_msg->index;
		uint nError = ERR_SYNC_NOT_READY;
		memcpy(pmsgheader->content, &nError, sizeof(uint));
		pmsgheader->length = newsize;
		outbuf.data_end(newsize);

		task_node->write_message(outbuf.data(), outbuf.data_size());
		return 0;
	}

	redisOpt *pRedis = appInstance->m_pRedisMgr->getOne();
	if (!pRedis) {
		LOG_PRINT(log_error, "pRedis is NULL");
		return -1;
	}

	string filter;
	vector<string> vecKey;
	if (pRedis->redis_scan_keys(filter, vecKey) < 0) {
		LOG_PRINT(log_error, "failed to run cmd scan");
		return -1;
	}
	LOG_PRINT(log_debug, "there are %u keys", vecKey.size());

	uint64_t log_index_start = 0;
	sockethandler::conns_Mutex.lock();
	clienthandler* phandler = (clienthandler*)sockethandler::conns[task_node->connid];
	if (phandler && phandler == task_node->from_connectPtr)
		log_index_start = phandler->last_update_logindex;
	sockethandler::conns_Mutex.unlock();

	const int memsize = SIZE_IVM_INDEX_HEADER + sizeof(int) + (GROUPSIZE+1) * MSG_LEN;
	char *pbuff = (char*)malloc(memsize);
	if (!pbuff) {
		LOG_PRINT(log_fatal, "%s", strerror(errno));
		return -1;
	}

	uint32_t actualsize = SIZE_IVM_INDEX_HEADER + sizeof(int);
	uint32_t count = 0;
	string type;
	vector<string> fields, vals;

	COM_MSG_INDEX_HEADER *out_msg = (COM_MSG_INDEX_HEADER*)pbuff;
	out_msg->maincmd = MDM_Vchat_Redis_Sync;
	out_msg->version = 10;
	out_msg->checkcode = PACK_ORDINARY;
	out_msg->index = in_msg->index;
	int *pItemSize = (int *)out_msg->content;
	RedisKeyInfo_t *pItem = (RedisKeyInfo_t *)(pbuff + actualsize);
	for (size_t i=0; i<vecKey.size(); i++) {
		LOG_PRINT(log_debug, "read the type of key: %s", vecKey[i].c_str());
		if (pRedis->redis_type(vecKey[i].c_str(), type) < 0) {
			LOG_PRINT(log_error, "failed to run cmd type: %s", vecKey[i].c_str());
			continue;
		}
		strcpy(pItem->key, vecKey[i].c_str());
		strcpy(pItem->type, type.c_str());
		if (type == "hash") {
			if (pRedis->redis_hgetall(vecKey[i], fields, vals) < 0) {
				LOG_PRINT(log_error, "failed to run cmd hgetall: %s", vecKey[i].c_str());
				continue;
			}
			string strfield = vecToStr(fields);
			string strvalue = vecToStr(vals, REDIS_DELIMITER);
			pItem->length = strfield.size() + strvalue.size() + 2;
			sprintf(pItem->content, "%s|%s", strfield.c_str(), strvalue.c_str());
		}
		else if (type == "set") {
			if (pRedis->redis_smembers(vecKey[i], vals) < 0) {
				LOG_PRINT(log_error, "failed to run cmd hgetall: %s", vecKey[i].c_str());
				continue;
			}
			string strvalue = vecToStr(vals);
			pItem->length = strvalue.size()+1;
			strcpy(pItem->content, strvalue.c_str());
		}

		actualsize += sizeof(RedisKeyInfo_t) + pItem->length;
		pItem = (RedisKeyInfo_t *)(pbuff + actualsize);
		count++;
		if (GROUPSIZE == count || vecKey.size() - 1 == i) {
			*pItemSize = count;
			out_msg->length = actualsize;
			out_msg->subcmd = Sub_SyncData_resp;
			if (vecKey.size() - 1 == i)
				out_msg->subcmd = Sub_SyncData_Last_resp;
			task_node->write_message(pbuff, actualsize);

			count = 0;
			actualsize = SIZE_IVM_INDEX_HEADER + sizeof(int);
			pItem = (RedisKeyInfo_t *)(pbuff + actualsize);
		}
	}

	if (pbuff)
		free(pbuff);

	proc_syncdata_from_log(task_node, log_index_start);

	return 0;
}

int CTaskNetMsgProc::proc_syncdata_resp(MSG_PROC_TASK_NODE *task_node)
{
	COM_MSG_INDEX_HEADER* in_msg = (COM_MSG_INDEX_HEADER*)task_node->pdata;

	redisOpt *pRedis = appInstance->m_pRedisMgr->getOne();
	if (!pRedis) {
		LOG_PRINT(log_error, "pRedis is NULL");
		return -1;
	}

	int * pItemcount =  (int*)in_msg->content;
	char *p = in_msg->content + sizeof(int);
	RedisKeyInfo_t *pItem = (RedisKeyInfo_t *)p;

	LOG_PRINT(log_debug, "received item count:%d", *pItemcount);

	vector<string> vecfield, vecvalue;
	string strfield, strvalue;
	for (int i=0; i<*pItemcount; i++) {
		LOG_PRINT(log_debug, "received key:%s", pItem->key);
		if (0 == strcasecmp(pItem->type, "hash")) {
			char *pseq = strchr(pItem->content, '|');
			if (pseq) {
				*pseq++ = 0;
				vecfield = strToVec(pItem->content);
				vecvalue = strToVec(pseq, REDIS_DELIMITER);
			}
			if (vecfield.size() != vecvalue.size()) {
				p += sizeof(RedisKeyInfo_t) + pItem->length;
				pItem = (RedisKeyInfo_t *)p;
				continue;
			}
			string key = pItem->key;
			if (pRedis->redis_hmset(key, vecfield, vecvalue) < 0) {
				LOG_PRINT(log_error, "failed to run cmd hmset, key:%s", key.c_str());
			}
		}
		else if (0 == strcasecmp(pItem->type, "set")) {
			if (pRedis->redis_sadd(pItem->key, pItem->content) < 0) {
				LOG_PRINT(log_error, "failed to run cmd sadd, key:%s", pItem->key);
			}
		}
		p += sizeof(RedisKeyInfo_t) + pItem->length;
		pItem = (RedisKeyInfo_t *)p;
	}

	if (Sub_SyncData_Last_resp == in_msg->subcmd) {
		appInstance->m_sync_status = SYNC_SEMI_READY;
		LOG_PRINT(log_info, "got all existed redis data, sync status: SYNC_SEMI_READY");
	}
}

int CTaskNetMsgProc::proc_syncdata_from_log(MSG_PROC_TASK_NODE *task_node, uint64_t log_index)
{
	FILE* logfile = fopen(appInstance->m_strsynclogpath.c_str(), "rb");
	if (!logfile)
		return -1;

	const uint GROUP = 30;
	uint32_t memsize = SIZE_IVM_INDEX_HEADER + sizeof(int) + (sizeof(CMDRedisData_t) + MSG_LEN) *GROUP;
	SL_ByteBuffer outbuf(memsize);
	char *pMsg = outbuf.buffer() + SIZE_IVM_INDEX_HEADER + sizeof(int);
	char *pstart = pMsg;
	CMDRedisData_t *pitem = (CMDRedisData_t*)pstart;
	uint32_t index = 0;
	uint32_t actualmsgsize = 0;
	int length = 0;
	uint64_t lineno = 0;
	char line[MSG_LEN];
	while(fread(&length, 1, sizeof(length), logfile)){
		int linesize = fread(line, 1, length, logfile);
		if (++lineno < log_index || linesize != length)
			continue;

		if (linesize){
			pitem->length = length;
			actualmsgsize += pitem->length;
			memcpy(pitem->content, line, pitem->length);
			pstart += pitem->length;
			pitem = (CMDRedisData_t*)pstart;
			index++;
			if (index == GROUP){
				uint itemsize = sizeof(CMDRedisData_t)*index + actualmsgsize;
				uint newsize = SIZE_IVM_INDEX_HEADER + sizeof(int) + itemsize;
				if (newsize > memsize) {
					LOG_PRINT(log_info, "the size of data to sync send over the max MSG LEN, newsize:%u, memsize:%u", newsize, memsize);
					return -1;
				}
				COM_MSG_INDEX_HEADER* pmsgheader = (COM_MSG_INDEX_HEADER*)outbuf.buffer();
				pmsgheader->version =10;
				pmsgheader->checkcode = PACK_REQ;
				pmsgheader->maincmd = MDM_Vchat_Redis_Sync;
				pmsgheader->subcmd = Sub_Vchat_RedisSync_data_Resp;
				pmsgheader->index = appInstance->new_index();
				memcpy(pmsgheader->content, &index, sizeof(index));
				pmsgheader->length = newsize;
				outbuf.data_end(newsize);
				task_node->write_message(outbuf.data(), outbuf.data_size());
				index = 0;
				pstart = pMsg;
				pitem = (CMDRedisData_t*)pstart;
			}
		}
	}
	fclose(logfile);
	//set remote to SYNC_READY status
	//appInstance->m_sync_redis_Mutex.lock();
	sockethandler::conns_Mutex.lock();
	clienthandler* phandler = (clienthandler*)sockethandler::conns[task_node->connid];
	if (phandler && phandler == task_node->from_connectPtr) {
		phandler->remote_sync_status = SYNC_READY;
		LOG_PRINT(log_info, "set remote sync status: SYNC_READY");
	}
	sockethandler::conns_Mutex.unlock();
	//appInstance->m_sync_redis_Mutex.unlock();

	uint itemsize = sizeof(CMDRedisData_t)*index + actualmsgsize;
	uint newsize = SIZE_IVM_INDEX_HEADER + sizeof(int) + itemsize;
	COM_MSG_INDEX_HEADER* pmsgheader = (COM_MSG_INDEX_HEADER*)outbuf.buffer();
	pmsgheader->version =10;
	pmsgheader->checkcode = PACK_REQ;
	pmsgheader->maincmd = MDM_Vchat_Redis_Sync;
	pmsgheader->subcmd = Sub_Vchat_RedisSync_data_Resp;
	pmsgheader->index = appInstance->new_index();
	memcpy(pmsgheader->content, &index, sizeof(index));
	pmsgheader->length = newsize;
	outbuf.data_end(newsize);
	task_node->write_message(outbuf.data(), outbuf.data_size());

	newsize = SIZE_IVM_INDEX_HEADER;
	pmsgheader->version =10;
	pmsgheader->checkcode = PACK_REQ;
	pmsgheader->maincmd = MDM_Vchat_Redis_Sync;
	pmsgheader->subcmd = Sub_Vchat_RedisSync_data_Complete;
	pmsgheader->length = newsize;
	outbuf.data_end(newsize);
	task_node->write_message(outbuf.data(), outbuf.data_size());

	return 0;
}

int CTaskNetMsgProc::proc_handle_redismsg(MSG_PROC_TASK_NODE *task_node)
{
	COM_MSG_INDEX_HEADER* in_msg = (COM_MSG_INDEX_HEADER*)task_node->pdata;
	
	if (in_msg->subcmd == Sub_Vchat_RedisSync_data_Complete){
		appInstance->m_sync_status = SYNC_READY;
		LOG_PRINT(log_info, "redis sync complete, sync status: SYNC_READY.")
		appInstance->launchapp("roomsvr");
		appInstance->m_bsyncing = false;
		return 0;
	}

	int nret = 0;
	uint *pitemsize = (uint*)in_msg->content;
	char *pstart = in_msg->content + sizeof(uint);
	CMDRedisData_t *pItem = (CMDRedisData_t *)(pstart);
	vector<string> vecField, vecValue;

	for (size_t i=0; i<*pitemsize; i++) {
		if (!pItem->content || 0 == strlen(pItem->content))
			continue;

		LOG_PRINT(log_info, "[index:%u,len:%u,length:%u]receive sync msg: %s", in_msg->index, pItem->length, in_msg->length, pItem->content);
		const char* msg = pItem->content;
		Json::Reader reader(Json::Features::strictMode());
		Json::Value root;
		if (!reader.parse(msg, root)) {
			LOG_PRINT(log_error, "invalid Json format: %s", msg);
			return -1;
		}
		Json::Value head = root["head"];
		Json::Value body = root["body"];
		uint32 nflags = 0;
		nflags = head["flags"].asUInt();
		appInstance->done_redis_msg(msg, pItem->length, root, nflags & WRITE_REMOTE_REDIS);
		if (nflags & PUBLISH_REMOTE) {
			appInstance->m_pRedisPub->redis_publish(msg, pItem->length);
		}
		pstart += pItem->length;
		pItem = (CMDRedisData_t*)pstart;
	}
	
	if (PACK_REQ == in_msg->checkcode)
		in_msg->checkcode = PACK_RESP;
	in_msg->length = SIZE_IVM_INDEX_HEADER;
	task_node->write_message(task_node->pdata, in_msg->length);

	return 0;
}


