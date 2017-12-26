
#include "connmanager.h"
#include "connecthandler.h"
#include <iostream>

#include "CLogThread.h"

connmgr::connmgr()
{
	svr_sessions_.clear();
}

connmgr::~connmgr()
{
}

//增加服务器的链接
void connmgr::add_svr_conn(connecthandler_ptr connection)
{
	boost::mutex::scoped_lock lock(svr_session_mutex_);
	svr_sessions_.push_back(connection);

	LOG_PRINT(log_info, "add_svr_conn succuss ");
	return;
}


//删除用户的链接
void connmgr::del_conn_svr(connecthandler_ptr connection)
{
	boost::mutex::scoped_lock lock(svr_session_mutex_);
	vector<connecthandler_ptr>::iterator it = svr_sessions_.begin();
	while (it != svr_sessions_.end()){
		if (*it == connection){
			svr_sessions_.erase(it);
			break;
		}
		++it;
	}
}

//获取服务器链接信息
connecthandler_ptr connmgr::get_conn_svr(uint32 userid)
{   
	boost::mutex::scoped_lock lock(svr_session_mutex_);
	connecthandler_ptr connptr = conn_handler[userid];
	if (connptr)
		return connptr;

	size_t size = svr_sessions_.size();
	if (!size)
		return connptr;

	uint64 modid = userid % size;
	connptr = svr_sessions_[modid];
	connptr->addclientid(userid);
	return connptr;
}



//打印连接信息
void  connmgr::print_conn_svr()
{
	vector<connecthandler_ptr>::iterator iter = svr_sessions_.begin();
	for (; iter != svr_sessions_.end(); iter++)
	{
		connecthandler_ptr connptr = *iter;
		LOG_PRINT(log_debug, "print_conn_inf conn_ssn %u ", connptr->getconnid());
	}
}

connecthandler_ptr connmgr::get_conn_by_ip(const char* ip)
{
	connecthandler_ptr connptr;
	vector<connecthandler_ptr>::iterator iter = svr_sessions_.begin();
	for (; iter != svr_sessions_.end(); iter++)
	{
		connptr = *iter;
		if (0 == strcasecmp(connptr->getremote_ip(), ip))
			return connptr;
	}
	return connptr;
}

void connmgr::write_message(char* pMsg, int nlen)
{
	vector<connecthandler_ptr>::iterator iter = svr_sessions_.begin();
	for (; iter != svr_sessions_.end(); iter++)
	{
		connecthandler_ptr connptr = *iter;
		if (connptr && connptr->isconnected())
			connptr->write_message(pMsg, nlen);
		LOG_PRINT(log_debug, "print_conn_inf conn_ssn %u ", connptr->getconnid());
	}
}

/*************************************************************** 文件结束 *********************************************************************/





