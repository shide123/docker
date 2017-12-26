
#ifndef __CLIENTMGR_H__
#define __CLIENTMGR_H__

#include <set>
//#include <boost/thread/mutex.hpp>
//#include <boost/smart_ptr/detail/spinlock.hpp>
#include "clienthandler.h"
#include "cmd_vchat.h"

struct REQ_SVR_INFO
{
	char svr_type;
	unsigned int svr_connID;
	std::string svr_ip;
	unsigned int svr_port;
	time_t req_time;
};

//key: request cmd, value: request server info
typedef std::map<unsigned int, REQ_SVR_INFO> REQ_TIME_MAP;

class CClientManager
{
public:
	CClientManager();
	~CClientManager();
	void init();
	void del_client(clienthandler_ptr connection);
	void update_client_map(clienthandler_ptr connection);
	clienthandler_ptr find_client_map(unsigned int connect_id);
	//send msg to client connection by connID,0 means success, others means fail.
	int send_msg_to_client_by_connID(unsigned int connect_id, SL_ByteBuffer & buff, e_NoticeDevType e_dev_type = e_Notice_AllType);
	clienthandler_ptr find_user_conn(unsigned int iduser, byte nmoblie, unsigned int nlogintime);
	void setuserconn(unsigned int userid, byte nmobile, unsigned int nlogintime, unsigned int idconn);
	unsigned int get_last_alarmnotify_time(){return last_alarmnotify_time_;}
	void set_last_alarmnotify_time(unsigned int t){last_alarmnotify_time_ = t;}
	void broadcast_all_client(byte termtype, SL_ByteBuffer & buff, unsigned int except_connid = 0);
	void broadcast_all_subscribe_client(e_NoticeDevType termtype, unsigned int subcmd, SL_ByteBuffer & buff);
	void broadcast_user_subscribe_client(unsigned int userid, e_NoticeDevType termtype, unsigned int subcmd, SL_ByteBuffer & buff);
	void broadcast_room_all_connects(int cmd, unsigned int roomid, unsigned int own_id, SL_ByteBuffer & buff, e_NoticeDevType e_dev_type = e_Notice_AllType, bool castInGroupOnly = false);
	void broadcast_all_room_all_connects(unsigned int own_id, SL_ByteBuffer & buff, e_NoticeDevType e_dev_type = e_Notice_AllType);
	void broadcast_room_gender_connects(unsigned int roomid, SL_ByteBuffer & buff, USER_GENDER_TYPE gender);
	void broadcast_all_room_gender_connects(SL_ByteBuffer & buff, USER_GENDER_TYPE gender);
	void broadcast_user_all_connects(unsigned int userid, SL_ByteBuffer & buff, e_NoticeDevType e_dev_type = e_Notice_AllType, unsigned int except_connid = 0);
	void insert_req_check(unsigned int connect_id, unsigned int req_cmd, const REQ_SVR_INFO & oInfo);
	void del_req_check(unsigned int connect_id, unsigned int req_cmd);
	void del_req_check_by_conn_set(const std::set<unsigned int> & client_connid_set, unsigned int req_cmd);
	void get_req_check_connid(std::map<unsigned int, REQ_TIME_MAP > & connid_time_map);
	void check_req_time();
	void getallconnptr(std::vector<clienthandler_ptr> & vecconn);
	void get_user_all_connects(unsigned int userid, std::set<unsigned int > & conn_setID);
	void get_room_user_conn_by_roletype_filter(std::set<clienthandler_ptr> &sConn, unsigned int roomid, unsigned int minRole, unsigned int maxRole = 0, e_NoticeDevType e_dev_type = e_Notice_AllType);

	//service interface
	void save_user_invite_mic(unsigned int userid, unsigned int runnerID, unsigned int touserID, int sessionid);
	void reset_user_invite_mic(unsigned int userid);

private:
	//boost::mutex client_session_mutex_;

	//key:client id, value:clienthandler_ptr
	std::map<unsigned int, clienthandler_ptr > session_map_;

	//key:userid_nmobile_logintime, value:client connect id
	std::map<std::string, unsigned int> m_mapuser;

	//key:userid, value:set of client connect id
	std::map<unsigned int, std::set<unsigned int> > m_user_connid_map;

	//key:client connect id, value:map<request cmd, time_t>
	std::map<unsigned int, REQ_TIME_MAP > m_connid_time_map;

	unsigned int last_alarmnotify_time_;

	time_t last_alarm_cmd_time_;

	int alarm_interval_;

	int m_nTimerId;
};

#endif //__CLIENTMGR_H__
