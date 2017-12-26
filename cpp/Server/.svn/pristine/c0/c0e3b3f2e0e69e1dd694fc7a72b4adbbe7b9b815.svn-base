
#ifndef __SOCKET_MANAGER_HH_
#define __SOCKET_MANAGER_HH_

//#include <list>
//#include <set>
//#include <stdlib.h>
//#include <boost/thread/mutex.hpp>
//#include <boost/smart_ptr/detail/spinlock.hpp>
#include "connecthandler.h"

class connmgr
{
public:
	connmgr(void);
	virtual ~connmgr(void);
    
	//增加服务
	void add_svr_conn(connecthandler_ptr connection);
	//获取链接服务
    connecthandler_ptr get_conn_svr(uint32 userid);
	//删除用户的链接
    void del_conn_svr(connecthandler_ptr connection);
	//打印连接信息
    void  print_conn_svr();
    connecthandler_ptr get_conn_by_ip(const char* ip);

    void write_message(char* pMsg, int nlen);
    size_t conn_number() const { return conn_handler.size(); }
private:
	boost::mutex svr_session_mutex_;
	map <u_int32_t, connecthandler_ptr> conn_handler; // [userid, roomsvr_conn] pair
	std::vector<connecthandler_ptr> svr_sessions_;
};

#endif  //__SVR_SESSION_MANAGER_HH_


