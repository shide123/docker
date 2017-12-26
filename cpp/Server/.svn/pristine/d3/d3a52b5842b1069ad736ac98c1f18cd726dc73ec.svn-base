/*
 * CLogonMgr.h
 *
 *  Created on: Mar 30, 2016
 *      Author: root
 */

#ifndef __CLOGONMGR_H__
#define __CLOGONMGR_H__

#include "ProtocolsBase.h"
#include "db/table2.h"

class CLogonMgr
{
public:
	CLogonMgr();

	~CLogonMgr();

    int proc_logonreq5(task_proc_data * task_node);
    
	int proc_synusertoken_req(task_proc_data * task_node);
    int proc_client_closeSocket_req(task_proc_data * task_node);

	int proc_client_getExitSoftMessage_req(task_proc_data * task_node);

private:

    int check_loginid(char * cloginid, int nLength, int & loginType);

	bool check_mobilenum_valid(const std::string & mobile);

	void cast_user_sendmsg(char * pPkt, int Len, int userid);	

	int BuildUserLogonErrPack(int errid, char * pszBuf, ClientGateMask_t * pMask, int nBufSize, uint32 reqid);
};

#endif /* __CLOGONMGR_H__ */
