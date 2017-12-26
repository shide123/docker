/*
 * CSvrConnMgr.h
 *
 *  Created on: Aug 23, 2016
 *      Author: testtest
 */

#ifndef __CSVRCONNMGR_H__
#define __CSVRCONNMGR_H__

#include <map>
#include "clienthandler.h"
#include "conhash.h"
#include "conhash_inter.h"
#include "cmd_vchat.h"
#include "UserMgrSvr.pb.h"

typedef std::map<int, unsigned int> SVRTYPE_CONNID_MAP;
typedef std::map<unsigned int, conhash_s * > SVRTYPE_CONHASH_MAP;
typedef std::set<unsigned int> SVR_CONNID_SET;

#define MAX_NODE_NUM 1024

class CSvrConnMgr
{
public:

	CSvrConnMgr();

	virtual ~CSvrConnMgr();

	//add server connection.
	void add_svr_type(int svr_type);

	//add server hash node.
	void add_svr_node(unsigned int svr_type, unsigned int svrconnID, const std::string & ipaddress, unsigned int port);

	//delete server hash node.
	void del_svr_node(unsigned int svr_type, unsigned int svrconnID, const std::string & ipaddress, unsigned int port);

	void del_conn_inf(unsigned int clientconnid);

	void del_conn_inf_by_type(unsigned int clientconnid, unsigned int svr_type);

	//get server conn ptr
	clienthandler_ptr get_conn_inf(int svr_type, unsigned int clientconnid, const std::string & distributed_value = "", unsigned int problem_svr_connID = 0);

	void print_conn_inf();

	//for usermgr
	int send_msg_to_service_type(SL_ByteBuffer & buff, e_SvrType svrType);

	void post_user_login(unsigned int userid, byte nmobile, unsigned int nlogintime, unsigned int client_connid, const std::string & uuid);

	void post_all_user_login(clienthandler_ptr usermgr_conn);

	void post_user_logout(unsigned int userid, byte nmobile, unsigned int nlogintime, unsigned int client_connid, const std::string & uuid);

private:

	void post_user_login_bat(const std::vector<CMDLogonClientInf> & vecClientInf, clienthandler_ptr usermgr_conn);

	//return server conn ptr
	clienthandler_ptr add_conn_inf(int svr_type, unsigned int clientconnid, const std::string & distributed_value = "", unsigned int problem_svr_connID = 0);

	//boost::mutex svr_conn_mutex_;

	//key:client connection id, value:<svr_type,server connect id>
	std::map<unsigned int, SVRTYPE_CONNID_MAP > session_map_;

	//key:svr_type, value:set of server connect id
	std::map<int, SVR_CONNID_SET > m_svrconn;

	//key:svr_type, value:array of node_s
	std::map<int, node_s * > m_nodes;

	//key:svr_type,value:conhash_s
	SVRTYPE_CONHASH_MAP m_svr_conhash;

	//key: server info:ip:port,value: node id, this is used for conhash.
	std::map<std::string, unsigned int > m_svr_nodeid;
	//key: svr_type,value: node id which is arranged, this is used for conhash.
	std::map<unsigned int, unsigned int > m_svrtype_nodeid;
};

#endif /* __CSVRCONNMGR_H__ */
