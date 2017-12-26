/*
 * CSvrConnMgr.cpp
 *
 *  Created on: Aug 23, 2016
 *      Author: testtest
 */

#include "CSvrConnMgr.h"
#include "CLogThread.h"
#include "CClientItem.h"
#include "GateBase.h"

CSvrConnMgr::CSvrConnMgr()
{
	session_map_.clear();
	m_svrconn.clear();
	m_svr_conhash.clear();
	m_nodes.clear();
	m_svr_nodeid.clear();
	m_svrtype_nodeid.clear();
}

CSvrConnMgr::~CSvrConnMgr()
{
	//boost::mutex::scoped_lock lock(svr_conn_mutex_);

	SVRTYPE_CONHASH_MAP::iterator iter = m_svr_conhash.begin();
	for (; iter != m_svr_conhash.end(); ++iter)
	{
		conhash_s * phash = iter->second;
		if (phash)
		{
			conhash_fini(phash);
		}
	}

	std::map<int, node_s * >::iterator iter_map = m_nodes.begin();
	for (; iter_map != m_nodes.end(); ++iter_map)
	{
		node_s * node_array = iter_map->second;
		if (node_array)
		{
			delete[] node_array;
			node_array = NULL;
		}
	}
}

//add server connection.
void CSvrConnMgr::add_svr_type(int svr_type)
{
	//boost::mutex::scoped_lock lock(svr_conn_mutex_);
	if (m_svr_conhash.end() == m_svr_conhash.find(svr_type))
	{
		conhash_s * phash = conhash_init(NULL);
		if (phash)
		{
			m_svr_conhash.insert(std::make_pair(svr_type, phash));
		}
	}

	std::map<int, SVR_CONNID_SET >::iterator iter_map = m_svrconn.find(svr_type);
	if (m_svrconn.end() == iter_map)
	{
		SVR_CONNID_SET connid_set;
		m_svrconn.insert(std::make_pair(svr_type, connid_set));
	}

	std::map<int, node_s * >::iterator iter_array = m_nodes.find(svr_type);
	if (m_nodes.end() == iter_array)
	{
		node_s * node_array = new node_s[MAX_NODE_NUM + 1];
		m_nodes.insert(std::make_pair(svr_type, node_array));
	}

	std::map<unsigned int, unsigned int>::iterator iter_node = m_svrtype_nodeid.find(svr_type);
	if (m_svrtype_nodeid.end() == iter_node)
	{
		unsigned int arrange_nodeid = 1;
		m_svrtype_nodeid.insert(std::make_pair(svr_type, arrange_nodeid));
	}
}

//add server hash node
void CSvrConnMgr::add_svr_node(unsigned int svr_type, unsigned int svrconnID, const std::string & ipaddress, unsigned int port)
{
	//boost::mutex::scoped_lock lock(svr_conn_mutex_);
	std::stringstream svr_key;
	svr_key << ipaddress << ':' << port;

	std::map<int, SVR_CONNID_SET >::iterator iter_map = m_svrconn.find(svr_type);
	if (m_svrconn.end() == iter_map)
	{
		LOG_PRINT(log_error, "m_svrconn cannot find this svr_type:%u.pls check config file.", svr_type);
		return;
	}

	SVR_CONNID_SET::iterator iter = iter_map->second.find(svrconnID);
	if (iter_map->second.end() == iter)
	{
		LOG_PRINT(log_info, "save svr_connID:%u,%s.", svrconnID, svr_key.str().c_str());
		iter_map->second.insert(svrconnID);
	}

	//get nodeID for conhash
	unsigned int nodeID = 0;
	std::map<std::string, unsigned int >::iterator iter_nodeid = m_svr_nodeid.find(svr_key.str());
	if (iter_nodeid != m_svr_nodeid.end())
	{
		//nodeID has arranged before.
		nodeID = iter_nodeid->second;
	}
	else
	{
		//nodeID has not arranged before,need to arrange new nodeid in svr_type.
		std::map<unsigned int, unsigned int >::iterator iter_nodeid_arrange = m_svrtype_nodeid.find(svr_type);
		if (iter_nodeid_arrange == m_svrtype_nodeid.end())
		{
			LOG_PRINT(log_error, "m_svrtype_nodeid cannot find this svr_type:%u.pls check config file.", svr_type);
			return;
		}
		else
		{
			nodeID = iter_nodeid_arrange->second;
			++iter_nodeid_arrange->second;
		}
		m_svr_nodeid.insert(std::make_pair(svr_key.str(), nodeID));
	}

	if (nodeID >= MAX_NODE_NUM)
	{
		LOG_PRINT(log_error, "node num of svr_type:%u must not larger than %u,svr_connID:%u,%s.", svr_type, MAX_NODE_NUM, svrconnID, svr_key.str().c_str());
		return;
	}

	if (m_svr_conhash.end() != m_svr_conhash.find(svr_type) && m_nodes.end() != m_nodes.find(svr_type))
	{
		char cIdentify[256] = {0};
		sprintf(cIdentify, "%s_%u", ipaddress.c_str(), port);
		int nvirnum = 30;

		node_s * pnode = m_nodes[svr_type];
		conhash_set_node(&pnode[nodeID], cIdentify, nvirnum);
		LOG_PRINT(log_info, "conhash_set_node success.svr_type:%u,nodeID:%u,nvirnum:%u.", svr_type, nodeID, nvirnum);

		conhash_s * phash = m_svr_conhash[svr_type];
		conhash_add_node(phash, &pnode[nodeID]);

		LOG_PRINT(log_info, "conhash_add_node success.svr_type:%u,nodeID:%u,svr_connID:%u,%s.", svr_type, nodeID, svrconnID, svr_key.str().c_str());
	}
	else
	{
		LOG_PRINT(log_error, "conhash_add_node failed.svr_type:%u,nodeID:%u,svr_connID:%u,%s.", svr_type, nodeID, svrconnID, svr_key.str().c_str());
	}
}

//delete server hash node
void CSvrConnMgr::del_svr_node(unsigned int svr_type, unsigned int svrconnID, const std::string & ipaddress, unsigned int port)
{
	//boost::mutex::scoped_lock lock(svr_conn_mutex_);
	std::stringstream svr_key;
	svr_key << ipaddress << ':' << port;

	std::map<int, SVR_CONNID_SET >::iterator iter_map = m_svrconn.find(svr_type);
	if (m_svrconn.end() == iter_map)
	{
		LOG_PRINT(log_error, "m_svrconn cannot find this svr_type:%u.pls check config file.", svr_type);
		return;
	}

	std::set<unsigned int>::iterator iter = iter_map->second.find(svrconnID);
	if (iter != iter_map->second.end())
	{
		LOG_PRINT(log_info, "delete svr_connID:%u,%s.", svrconnID, svr_key.str().c_str());
		iter_map->second.erase(svrconnID);
	}

	//get nodeID for conhash
	unsigned int nodeID = 0;
	std::map<std::string, unsigned int >::iterator iter_nodeid = m_svr_nodeid.find(svr_key.str());
	if (iter_nodeid == m_svr_nodeid.end())
	{
		return;
	}

	nodeID = iter_nodeid->second;

	if (m_svr_conhash.end() != m_svr_conhash.find(svr_type) && m_nodes.end() != m_nodes.find(svr_type))
	{
		conhash_s * phash = m_svr_conhash[svr_type];
		node_s * pnode = m_nodes[svr_type];
		conhash_del_node(phash, &pnode[nodeID]);
		
		LOG_PRINT(log_info, "conhash_del_node success.svr_type:%u,nodeID:%u,svr_connID:%u,%s.", svr_type, nodeID, svrconnID, svr_key.str().c_str());
	}
}

//add one user connection
clienthandler_ptr CSvrConnMgr::add_conn_inf(int svr_type, unsigned int clientconnid, const std::string & distributed_value, unsigned int problem_svr_connID)
{
    //boost::mutex::scoped_lock lock(svr_conn_mutex_);
	clienthandler_ptr pSelectSvrPtr;
	unsigned int select_svrconnID = 0;

    std::map<int, SVR_CONNID_SET >::iterator iter_svrmap = m_svrconn.find(svr_type);
    if (m_svrconn.end() == iter_svrmap)
    {
        LOG_PRINT(log_error, "m_svrconn cannot find this svr_type:%u,client connid:%u", svr_type, clientconnid);
        return pSelectSvrPtr;
    }

    SVR_CONNID_SET & tcpset = iter_svrmap->second;

	if (distributed_value.empty())
	{
		//choose the min connection num
		unsigned int minConnNum = 99999999;
		int isfirst = 0;
		SVR_CONNID_SET::iterator iter = tcpset.begin();
		for (; iter != tcpset.end(); iter++)
		{
			clienthandler_ptr pSvrConnPtr = tcpserver::getconnhandler(*iter);

			if (!pSvrConnPtr)
			{
				continue;
			}

			//check if the connection is ok
			if (!pSvrConnPtr->isconnected())
			{
				continue;
			}

			//find out the minConnNum of server connection.
			if (isfirst == 0)
			{
				isfirst = 1;
				minConnNum = pSvrConnPtr->getclientid_size();
				select_svrconnID = pSvrConnPtr->getconnid();
				pSelectSvrPtr = pSvrConnPtr;
			}
			else if (pSvrConnPtr->getclientid_size() <  minConnNum)
			{
				minConnNum = pSvrConnPtr->getclientid_size();
				select_svrconnID = pSvrConnPtr->getconnid();
				pSelectSvrPtr = pSvrConnPtr;
			}
		}
	}
	else
	{
		//arrange server according to distributed value,use consistent hash
		if (m_svr_conhash.end() == m_svr_conhash.find(svr_type))
		{
			LOG_PRINT(log_error, "m_svr_conhash cannot find this svr_type:%u,client connid:%u", svr_type, clientconnid);
			return pSelectSvrPtr;
		}
		else
		{
			conhash_s * phash = m_svr_conhash[svr_type];
			char cDistributed[256] = {0};
			sprintf(cDistributed, "%s", distributed_value.c_str());
			const struct node_s * node = conhash_lookup(phash, cDistributed);
			if (node)
			{
				char cIdentify[256] = {0};
				strcpy(cIdentify, node->iden);
                LOG_PRINT(log_info, "[conhash_getnode]distribute:%s,identify:%s.", distributed_value.c_str(), cIdentify);
				SVR_CONNID_SET::iterator iter = tcpset.begin();
				for (; iter != tcpset.end(); iter++)
				{
					clienthandler_ptr pSvrConnPtr = tcpserver::getconnhandler(*iter);

					if (!pSvrConnPtr)
					{
						continue;
					}

					//check if the connection is ok
					if (!pSvrConnPtr->isconnected())
					{
						continue;
					}

					std::string ipaddress = pSvrConnPtr->getremote_ip();
					unsigned int port = pSvrConnPtr->getremote_port();
					char sNode[256] = {0};
					sprintf(sNode, "%s_%u", ipaddress.c_str(), port);
					if (strcmp(sNode, cIdentify) == 0)
					{
						select_svrconnID = pSvrConnPtr->getconnid();
						pSelectSvrPtr = pSvrConnPtr;
						break;
					}
				}
			}
		}
	}

    if (select_svrconnID != 0)
    {
		if (problem_svr_connID && problem_svr_connID == select_svrconnID)
		{
			unsigned int new_selectID = 0;
			SVR_CONNID_SET & svr_tcp_set = m_svrconn[svr_type];
			SVR_CONNID_SET::iterator iter = svr_tcp_set.begin();
			for (; iter != tcpset.end(); iter++)
			{
				clienthandler_ptr pSvrConnPtr = tcpserver::getconnhandler(*iter);
				if (!pSvrConnPtr)
				{
					continue;
				}
				else if (!pSvrConnPtr->isconnected()) //check if the connection is ok
				{
					continue;
				}
				else if (pSvrConnPtr->getconnid() == problem_svr_connID) //check if the connection is problem connection
				{
					continue;
				}
				else
				{
					new_selectID = pSvrConnPtr->getconnid();
					if (new_selectID)
					{
						select_svrconnID = new_selectID;
						pSelectSvrPtr = pSvrConnPtr;
					}
					break;
				}
			}
		}

        pSelectSvrPtr->addclientid(clientconnid);
		const std::string svr_ip = pSelectSvrPtr->getremote_ip();
		unsigned int svr_port = pSelectSvrPtr->getremote_port();

        std::map<unsigned int, SVRTYPE_CONNID_MAP >::iterator iter_sessmap = session_map_.find(clientconnid);
        if (session_map_.end() == iter_sessmap)
        {
            SVRTYPE_CONNID_MAP svr_conn;
            svr_conn.insert(std::make_pair(svr_type, select_svrconnID));
            session_map_.insert(std::make_pair(clientconnid, svr_conn));
			LOG_PRINT(log_debug, "Have added one user connection.svr_type:%u,server:%s:%u,client connid:%u.", svr_type, svr_ip.c_str(), svr_port, clientconnid);
        }
        else
        {
            iter_sessmap->second[svr_type] = select_svrconnID;
            LOG_PRINT(log_debug, "Have added one user connection.svr_type:%u,server:%s:%u,client connid:%u.", svr_type, svr_ip.c_str(), svr_port, clientconnid);
        }
    }

    return pSelectSvrPtr;
}

//get server conn ptr
clienthandler_ptr CSvrConnMgr::get_conn_inf(int svr_type, unsigned int clientconnid, const std::string & distributed_value, unsigned int problem_svr_connID)
{
	clienthandler_ptr pSelectSvrPtr;
	if (0 == svr_type)
	{
		LOG_PRINT(log_error, "svr_type is 0.client connid:%u.", clientconnid);
		return pSelectSvrPtr;
	}

	do 
	{
		std::map<unsigned int, SVRTYPE_CONNID_MAP >::iterator iter = session_map_.find(clientconnid);
		if (iter == session_map_.end())
		{
			//LOG_PRINT(log_warning, "cannot find this connectid.svr_type:%d,client connid:%u.", svr_type, clientconnid);
			break;
		}

		SVRTYPE_CONNID_MAP & svr_map = iter->second;
		SVRTYPE_CONNID_MAP::iterator iter_svr = svr_map.find(svr_type);
		if (iter_svr == svr_map.end())
		{
			//LOG_PRINT(log_warning, "cannot find this type of server connectid.svr_type:%d,client connid:%u.", svr_type, clientconnid);
			break;
		}

		//get server connect id
		unsigned int iSvrConn = iter_svr->second;
		pSelectSvrPtr = tcpserver::getconnhandler(iSvrConn);
		if (!pSelectSvrPtr)
		{
			break;
		}

		//check if server connection is ok
		if (!pSelectSvrPtr->isconnected())
		{
			break;
		}
		else
		{
			return pSelectSvrPtr;
		}

	} while (0);

	return this->add_conn_inf(svr_type, clientconnid, distributed_value, problem_svr_connID);
}

//delete client connection
void CSvrConnMgr::del_conn_inf(unsigned int clientconnid)
{
	//boost::mutex::scoped_lock lock(svr_conn_mutex_);
	do 
	{
		std::map<unsigned int, SVRTYPE_CONNID_MAP >::iterator iter = session_map_.find(clientconnid);
		if (iter == session_map_.end())
		{
			return;
		}

		SVRTYPE_CONNID_MAP & svr_map = iter->second;
		SVRTYPE_CONNID_MAP::iterator iter_svr = svr_map.begin();
		for (; iter_svr != svr_map.end(); ++iter_svr)
		{
			unsigned int iSvrConn = iter_svr->second;

			//use svr_type and iSvrConn to find server
			clienthandler_ptr pSelectSvrPtr = tcpserver::getconnhandler(iSvrConn);
			if (!pSelectSvrPtr)
			{
				continue;
			}
			else
			{
				pSelectSvrPtr->delclientid(clientconnid);
			}
		}
	} while (0);

	session_map_.erase(clientconnid);
	return;
}

void CSvrConnMgr::del_conn_inf_by_type(unsigned int clientconnid, unsigned int svr_type)
{
	//boost::mutex::scoped_lock lock(svr_conn_mutex_);
	std::map<unsigned int, SVRTYPE_CONNID_MAP >::iterator iter = session_map_.find(clientconnid);
	if (iter != session_map_.end())
	{
		iter->second.erase(svr_type);
	}

	return;
}

//print connection
void CSvrConnMgr::print_conn_inf()
{
	//boost::mutex::scoped_lock lock(svr_conn_mutex_);
	std::map<int, SVR_CONNID_SET >::iterator iter = m_svrconn.begin();
	for (; iter != m_svrconn.end(); ++iter)
	{
		int svrtype = iter->first;
		SVR_CONNID_SET & tcpset = iter->second;
		SVR_CONNID_SET::iterator iter_set = tcpset.begin();
		for (; iter_set != tcpset.end(); ++iter_set)
		{
			clienthandler_ptr pSvrConnPtr = tcpserver::getconnhandler(*iter_set);
			if (!pSvrConnPtr)
			{
				continue;
			}

			//check if server connection is ok
			if (!pSvrConnPtr->isconnected())
			{
				continue;
			}

			LOG_PRINT(log_debug, "print_conn_inf svr_type:%d,svr_conn:%u,conn_num:%d.", svrtype, pSvrConnPtr->getconnid(), pSvrConnPtr->getclientid_size());
		}
	}
}

int CSvrConnMgr::send_msg_to_service_type(SL_ByteBuffer & buff, e_SvrType svrType)
{
	int count = 0;

	std::map<int, SVR_CONNID_SET >::iterator iter = m_svrconn.find(svrType);
	if (iter != m_svrconn.end())
	{
		SVR_CONNID_SET & tcpset = iter->second;
		SVR_CONNID_SET::iterator iter_tcp = tcpset.begin();
		for (; iter_tcp != tcpset.end(); ++iter_tcp)
		{
			clienthandler_ptr pSvrConnPtr = tcpserver::getconnhandler(*iter_tcp);
			if (!pSvrConnPtr)
			{
				continue;
			}

			//check if server connection is ok
			if (!pSvrConnPtr->isconnected())
			{
				LOG_PRINT(log_error, "usermgr[%s:%u] is not connected.", pSvrConnPtr->getremote_ip(), pSvrConnPtr->getremote_port());
				continue;
			}

			pSvrConnPtr->write_message(buff.data(), buff.data_size());
			++count;
		}
	}

	return count;
}

void CSvrConnMgr::post_user_login(unsigned int userid, byte nmobile, unsigned int nlogintime, unsigned int client_connid, const std::string & uuid)
{
	if (userid == 0)
		return;

	CMDLogonClientInfLst userInfo;
	CMDLogonClientInf * logoninf = userInfo.add_userinflst();
	logoninf->set_userid(userid);
	logoninf->set_mobile((unsigned int)nmobile);
	logoninf->set_logontime(nlogintime);
	logoninf->set_connid(client_connid);
	logoninf->set_uuid(uuid);

	unsigned int dataLen = SIZE_IVM_HEADER + userInfo.ByteSize();
	SL_ByteBuffer buff(dataLen);
	buff.data_end(dataLen);

	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)buff.buffer();
	pHead->version = MDM_Version_Value;
	pHead->checkcode = CHECKCODE;
	pHead->maincmd = MDM_Vchat_Usermgr;
	pHead->subcmd = Sub_Vchat_LogonNot;
	pHead->length = dataLen;

	//user info
	userInfo.SerializeToArray(pHead->content, userInfo.ByteSize());
	int ret = send_msg_to_service_type(buff, e_usermgrsvr_type);

	LOG_PRINT(log_info, "post_user_login:client connid:%u,userid %u,nmobile:%d,logintime:%u,uuid:%s,usermgr count:%d.", client_connid, userid, (int)nmobile, nlogintime, uuid.c_str(), ret);
	return;
}

void CSvrConnMgr::post_user_login_bat(const std::vector<CMDLogonClientInf> & vecClientInf, clienthandler_ptr usermgr_conn)
{
	if (vecClientInf.size() == 0 || !usermgr_conn)
	{
		LOG_PRINT(log_error, "post_user_login_bat fail,size of client vector:%u.", vecClientInf.size());
		return;
	}

	int npostnum = vecClientInf.size();

	CMDLogonClientInfLst userLst;
	for (unsigned int i = 0; i < npostnum; i++)
	{
		CMDLogonClientInf * pUserInfo = userLst.add_userinflst();
		pUserInfo->CopyFrom(vecClientInf[i]);
	}

	unsigned int msglen = SIZE_IVM_HEADER + userLst.ByteSize();
	SL_ByteBuffer buff(msglen);
	buff.data_end(msglen);
	COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)buff.buffer();
	pHead->version = MDM_Version_Value;
	pHead->checkcode = CHECKCODE;
	pHead->maincmd = MDM_Vchat_Usermgr;
	pHead->subcmd = Sub_Vchat_LogonNot;
	pHead->length = msglen;
	userLst.SerializeToArray(pHead->content, userLst.ByteSize());

	if (usermgr_conn->isconnected())
	{
		usermgr_conn->write_message(buff);
	}
	else
	{
		LOG_PRINT(log_error, "post_user_login_bat fail,usermgr[%s:%u] is not connected.", usermgr_conn->getremote_ip(), usermgr_conn->getremote_port());
	}

	return;
}

void CSvrConnMgr::post_all_user_login(clienthandler_ptr usermgr_conn)
{
	if (!usermgr_conn)
	{
		LOG_PRINT(log_error, "post_all_user_login fail,usermgr_conn is null.");
		return;
	}

	//first get all online clienthandler
	std::vector<clienthandler_ptr> vecconn;
	CGateBase::m_ClientMgr.getallconnptr(vecconn);

	int count = 0;
	std::vector<CMDLogonClientInf> vecpostclient;

	int nbatnum = 160;
	for (int i = 0; i < vecconn.size(); i++)
	{
		//client connection
		clienthandler_ptr connptr = vecconn[i];
		if (!connptr || !connptr->user_data)
		{
			continue;
		}

		CClientItem * pClientItem = (CClientItem *)connptr->user_data;
		//check if online
		if (pClientItem->m_last_login_time == 0 || connptr->user_id == 0)
			continue;

		CMDLogonClientInf cClientInf;
		cClientInf.set_userid(connptr->user_id);
		cClientInf.set_mobile(pClientItem->m_mobile);
		cClientInf.set_logontime(pClientItem->m_last_login_time);
		cClientInf.set_connid(connptr->getconnid());
		cClientInf.set_uuid(pClientItem->m_uuid);

		++count;
		if (i % nbatnum != 0 || i == 0)
		{
			vecpostclient.push_back(cClientInf);
		}
		else
		{
			if (vecpostclient.size() > 0)
			{
				post_user_login_bat(vecpostclient, usermgr_conn);
				vecpostclient.clear();
			}
			vecpostclient.push_back(cClientInf);
		}
	}

	if (vecpostclient.size() > 0)
	{
		post_user_login_bat(vecpostclient, usermgr_conn);
	}

	LOG_PRINT(log_info, "post_all_user_login() post_all_user_login end.size:%d.usermgr[%s:%u].", count, usermgr_conn->getremote_ip(), usermgr_conn->getremote_port());

	return;
}

void CSvrConnMgr::post_user_logout(unsigned int userid, byte nmobile, unsigned int nlogintime, unsigned int client_connid, const std::string & uuid)
{
	if (userid == 0)
		return;

	int ret = 0;
	clienthandler_ptr pSvrConnPtr = get_conn_inf(e_usermgrsvr_type, client_connid);
	if (pSvrConnPtr)
	{
		CMDLogonClientInfLst userLst;
		CMDLogonClientInf * pUserInfo = userLst.add_userinflst();
		pUserInfo->set_userid(userid);
		pUserInfo->set_mobile(nmobile);
		pUserInfo->set_logontime(nlogintime);
		pUserInfo->set_connid(client_connid);
		pUserInfo->set_uuid(uuid);

		unsigned int dataLen = SIZE_IVM_HEADER + userLst.ByteSize();
		SL_ByteBuffer buff(dataLen);
		buff.data_end(dataLen);

		COM_MSG_HEADER * pHead = (COM_MSG_HEADER *)buff.buffer();
		pHead->version = MDM_Version_Value;
		pHead->checkcode = CHECKCODE;
		pHead->maincmd = MDM_Vchat_Usermgr;
		pHead->subcmd = Sub_Vchat_LogoutNot;
		pHead->length = dataLen;
		userLst.SerializeToArray(pHead->content, userLst.ByteSize());
		ret = 1;
		pSvrConnPtr->write_message(buff);
	}
//	ret = send_msg_to_service_type(buff, e_usermgrsvr_type);
	LOG_PRINT(log_info, "post_user_logout:userid %u,client connid:%u,nmobile:%d,logintime:%u,uuid:%s,usermgr count:%d.", userid, client_connid, (int)nmobile, nlogintime, uuid.c_str(), ret);
	return;
}
