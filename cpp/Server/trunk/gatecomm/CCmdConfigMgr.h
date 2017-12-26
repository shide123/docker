/*
 * CCmdGuideMgr.h
 *
 *  Created on: Mar 23, 2016
 *      Author: root
 */

#ifndef CCMDGUIDEMGR_H_
#define CCMDGUIDEMGR_H_

#include <map>
#include <string>
#include <list>
#include <set>
//#include <boost/thread/mutex.hpp>
#include "Config.h"

typedef std::map<unsigned int, unsigned int> CMD_MIN_MAX;
typedef std::set<int> SVR_TYPE_SET;

class CCmdConfigMgr
{
public:

	CCmdConfigMgr(const std::string configpath, const std::string procname);

	~CCmdConfigMgr();

	void addCmdConfig(const std::string & svr_name, unsigned int svr_type, const std::string & strCmdlst, const std::list<std::string> & cmdRangeLst);

	//used for loading request and response cmd list in config
	void loadCmdCheckConfig();

    SVR_TYPE_SET getSvrType(unsigned int cmdcode);

	//get request cmd
	unsigned int getReqbyRspCmd(unsigned int rsp_cmd);

	//get set of request cmd by server name
	void getReqbySvrName(const std::string & svr_name, std::set<unsigned int> & req_set);

	//check if the request cmd need to check.
	bool needCheckRspTime(unsigned int request_cmd);

	unsigned int getReqRspCheckTime();

protected:
	std::string m_configpath;
	std::string m_strProcName;

private:

	unsigned int req_rsp_checktime;

    //key:cmd, value:set of server type
    std::map<unsigned int, SVR_TYPE_SET> m_cmd_svr_map;

    //key:server type, value:map<min,max>
    std::map<int, CMD_MIN_MAX> m_svr_cmdrange_map;


	//key:response cmd which need to be checked,value:request cmd which need to be checked
	std::map<unsigned int, unsigned int> m_rsp_cmd_map;

	//key:svr_name,like:roomsvr ,value:request cmd which need to be checked
	std::map<std::string, std::set<unsigned int> > m_svr_name_cmd_map;

};

#endif /* CCMDGUIDEMGR_H_ */
