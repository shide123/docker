#include "sockethandler.h"
#include "CLogThread.h"
#include "server.h"
#include "CAlarmNotify.h"
#include "timestamp.h"


sockethandler::CONN_MAP sockethandler::conns;
SL_Sync_Mutex sockethandler::conns_Mutex;

sockethandler::sockethandler(boost::asio::io_service& ioservice)
    : socket_(ioservice)
	, deadline_(ioservice)
	, sfd(-1)
	, recv_buf_remainlen_(0)
    , connected(false)
	, actived(false)
    , connid_(0)
    , remote_port(0)
	, serv(NULL)
	, ntime_msgsend_timeout(0)
{
}

sockethandler::~sockethandler()
{  

}

void sockethandler::setconnhandler(uint64_t connid, sockethandler* handler)
{
	sockethandler::conns_Mutex.lock();
    sockethandler::conns[connid] = handler;
    sockethandler::conns_Mutex.unlock();
}

sockethandler* sockethandler::getconnhandler(uint64_t connid)
{
	sockethandler *handler = NULL;

	sockethandler::conns_Mutex.lock();
	handler = sockethandler::conns[connid];
    sockethandler::conns_Mutex.unlock();
    return handler;
}

void sockethandler::close()
{
	socket_.close();
    sfd = -1;
    connected = false;

    sockethandler::conns_Mutex.lock();
    sockethandler::conns[connid_] = NULL;
    sockethandler::CONN_MAP::iterator it = sockethandler::conns.find(connid_);
    if (it != sockethandler::conns.end())
    	sockethandler::conns.erase(it);
    sockethandler::conns_Mutex.unlock();

    connid_ = 0;
    boost::mutex::scoped_lock lock(send_messages_mutex_);
    m_send_queue.clear();
}


unsigned int getmillisecondspan(struct timeval &tv1, struct timeval &tv2)
{
	unsigned int ms1 = tv1.tv_sec * 1000 + tv1.tv_usec / 1000;
	unsigned int ms2 = tv2.tv_sec * 1000 + tv2.tv_usec / 1000;
	return ms1 - ms2;
}

void sockethandler::handle_msgsend_record(const char* pdata)
{
	/* record send message, check receive response or timeout */
	COM_MSG_INDEX_HEADER *pmsg = (COM_MSG_INDEX_HEADER*) pdata;
	if (PACK_REQ != pmsg->checkcode)
		return;

	struct timeval tv;
	gettimeofday(&tv, NULL);
	MSG_SEND_TIME msg = { pmsg->subcmd, tv };

	boost::mutex::scoped_lock lock(map_msgsend_lock);
	mapMsgsend[pmsg->index] = msg;
}

int sockethandler::handle_msgsend_timeout(const char* pdata)
{
	boost::mutex::scoped_lock lock(map_msgsend_lock);

	struct timeval tv;
	char stime[32];
	time_t now = time(NULL);
	if ( now - ntime_msgsend_timeout > 30) {
		LOG_PRINT(log_debug, "the size of mapMsgsend is :%u", mapMsgsend.size());
		ntime_msgsend_timeout = now;
	}

	if (pdata) {
		COM_MSG_INDEX_HEADER* in_msg = (COM_MSG_INDEX_HEADER*)pdata;
		map<uint32, MSG_SEND_TIME>::iterator it = mapMsgsend.find(in_msg->index);
		if (it != mapMsgsend.end()) {
			if (Sub_Vchat_RedisSync_data_Resp == in_msg->subcmd) {
				gettimeofday(&tv, 0);
				toStringTimestamp6(&tv, stime);
				LOG_PRINT(log_error, "[index:%u,subcmd:%u,recv time:%s] receive msg response!", it->first, it->second.subcmd, stime);
			}
			mapMsgsend.erase(it);
			return -1;
		}
	}
	else{
		map<uint32, MSG_SEND_TIME>::iterator it = mapMsgsend.begin();

		while (it != mapMsgsend.end()) {
			gettimeofday(&tv, 0);
			toStringTimestamp6(&tv, stime);
			if (getmillisecondspan(tv, it->second.tv) > 3000) {
				//TODO should alarm here
				LOG_PRINT(log_error, "[index:%u,subcmd:%u,timeout time:%s] no receive msg response!", it->first, it->second.subcmd, stime);
				CAlarmNotify::sendAlarmNoty(e_all_notitype, e_data_sync, "syncsvr", "sync data time out", "Yunwei,Room",
										"sync data time out to [%s:%u]", getremote_ip(), getremote_port());
				mapMsgsend.erase(it++);
				continue;
			}
			++it;
		}
	}
	return 0;
}

int sockethandler::parse_message()
{
	char* p = recv_buf_;

	while(recv_buf_remainlen_ > 4) {
		msgpacklen = *((int*)p);
		if(msgpacklen <= 0 || msgpacklen > en_msgbuffersize) {
			recv_buf_remainlen_ = 0;
			return -1;
		}
		else if(recv_buf_remainlen_ < msgpacklen) {
			break;
		}
		else {
			if(handle_message(p, msgpacklen) == -1) {
				recv_buf_remainlen_ = 0;
				return -1;
			}
			recv_buf_remainlen_ -= msgpacklen;
			p += msgpacklen;
		}
	}

	if(recv_buf_remainlen_ >= en_msgmaxsize) {
		recv_buf_remainlen_ = 0;
		return -1;
	}

	if(p != recv_buf_ && recv_buf_remainlen_ > 0){
		memmove(recv_buf_, p, recv_buf_remainlen_);
	}
	return 0;
}


