/*
 * CCmdGuideMgr.cpp
 *
 *  Created on: Mar 23, 2016
 *      Author: root
 */

#include "CCmdConfigMgr.h"
#include "CLogThread.h"
#include "utils.h"

CCmdConfigMgr::CCmdConfigMgr(const std::string configpath, const std::string procname)
{
    m_configpath = configpath;
	m_strProcName = procname;
	req_rsp_checktime = 20;
	m_cmd_svr_map.clear();
	m_svr_cmdrange_map.clear();
}

CCmdConfigMgr::~CCmdConfigMgr()
{
}

void CCmdConfigMgr::addCmdConfig(const std::string & svr_name, unsigned int svr_type, const std::string & strCmdlst, const std::list<std::string> & cmdRangeLst)
{
    if (!svr_type)
    {
        LOG_PRINT(log_error, "svr_type is 0,error is comm.conf.svr_name:%s,cmdlst:%s.", svr_name.c_str(), strCmdlst.c_str());
        return;
    }

	if (!strCmdlst.empty())
	{
		std::list<unsigned int> cmd_lst;
		splitStrToLst(strCmdlst, ',', cmd_lst);

		std::stringstream strcmdcode_log;

		std::list<unsigned int>::iterator iterlst = cmd_lst.begin();
		for (; iterlst != cmd_lst.end(); ++iterlst)
		{
			unsigned int cmdcode = *iterlst;
			std::map<unsigned int, SVR_TYPE_SET >::iterator itermap = m_cmd_svr_map.find(cmdcode);
			if (m_cmd_svr_map.end() != itermap)
			{
				itermap->second.insert(svr_type);
			} 
			else
			{
				SVR_TYPE_SET svr_type_set;
				svr_type_set.insert(svr_type);
				m_cmd_svr_map.insert(std::make_pair(cmdcode, svr_type_set));
			}
	
			strcmdcode_log << cmdcode << ",";
		}

		LOG_PRINT(log_info, "[add cmdlist config]%s svr_type:%u,cmd:%s.", svr_name.c_str(), svr_type, strcmdcode_log.str().c_str());
	}

	std::list<std::string>::const_iterator iter_range = cmdRangeLst.begin();
	for (; iter_range != cmdRangeLst.end(); ++iter_range)
	{
		const std::string & strCmdRange = *iter_range;
		if (strCmdRange.empty())
		{
			continue;
		}

		std::list<unsigned int> cmd_range;
		//format min:max
		splitStrToLst(strCmdRange, ':', cmd_range);
		if (2 != cmd_range.size())
		{
			LOG_PRINT(log_error, "cmdcode range config format error.svr_name:%s,cmdrange:%s.", \
				svr_name.c_str(), strCmdRange.c_str());
			continue;
		}

		std::list<unsigned int>::iterator iterrange = cmd_range.begin();
		unsigned int mincmd = *iterrange;
		unsigned int maxcmd = *(++iterrange);

		std::map<int, CMD_MIN_MAX>::iterator iter_svr = m_svr_cmdrange_map.find(svr_type);
		if (m_svr_cmdrange_map.end() != iter_svr)
		{
			iter_svr->second[mincmd] = maxcmd;
		}
		else
		{
			CMD_MIN_MAX min_max_map;
			min_max_map.insert(std::make_pair(mincmd, maxcmd));
			m_svr_cmdrange_map.insert(std::make_pair(svr_type, min_max_map));
		}
		LOG_PRINT(log_info, "[add cmdrange config]%s svr_type:%u,mincmd:%u,maxcmd:%u.", svr_name.c_str(), svr_type, mincmd, maxcmd);
	}
}

SVR_TYPE_SET CCmdConfigMgr::getSvrType(unsigned int cmdcode)
{
    {
		//boost::mutex::scoped_lock lock(cmd_svr_mutex);
        std::map<unsigned int, SVR_TYPE_SET>::iterator iter = m_cmd_svr_map.find(cmdcode);
        if (iter != m_cmd_svr_map.end())
        {
            return iter->second;
        }
    }

    //cmdcode is not in m_cmd_svr_map,need to check cmd range
    //LOG_PRINT(log_info, "cmdcode:%u need to check cmd range.", cmdcode);

    bool bfind = false;
    SVR_TYPE_SET svr_type_set;
    int svr_type = 0;
    {
        //boost::mutex::scoped_lock lock(svr_cmdrange_mutex);
        std::map<int, CMD_MIN_MAX>::iterator iter = m_svr_cmdrange_map.begin();
        for (; iter != m_svr_cmdrange_map.end(); ++iter)
        {
            CMD_MIN_MAX & cmd_min_max = iter->second;
            CMD_MIN_MAX::iterator itermap = cmd_min_max.begin();
            for (; itermap != cmd_min_max.end(); ++itermap)
            {
                //key:min cmdcode, value:max cmdcode
                if (cmdcode >= itermap->first && cmdcode <= itermap->second)
                {
                    bfind = true;
                    break;
                }
            }

            if (bfind)
            {
                svr_type = iter->first;
                break;
            }
        }
    }

    if (bfind)
    {
        LOG_PRINT(log_debug, "cmdcode:%u find svr_type:%d", cmdcode, svr_type);
        //boost::mutex::scoped_lock lock(cmd_svr_mutex);
        svr_type_set.insert(svr_type);
        m_cmd_svr_map.insert(std::make_pair(cmdcode, svr_type_set));
    }
    return svr_type_set;
}

//used for loading request and response cmd list in config
void CCmdConfigMgr::loadCmdCheckConfig()
{
	if(m_configpath.empty())
	{
		LOG_PRINT(log_debug, "config path is empty.");
		return;
	}

	std::string cmd_check_config = m_configpath + m_strProcName + ".conf";
	LOG_PRINT(log_debug, "config path:%s", cmd_check_config.c_str());

	Config config(cmd_check_config);
	int line = 0;
	if(!config.load(line))
	{
		LOG_PRINT(log_error, "config path:%s", cmd_check_config.c_str());
		return;
	}

	std::string svrnames = config.getString("checkcmd", "svr_name");
	std::list<std::string> svr_lst;
	splitStrToLst(svrnames, ',', svr_lst);
	if (svr_lst.empty())
	{
		LOG_PRINT(log_warning, "svr_name is empty.config path:%s", cmd_check_config.c_str());
		return;
	}

	std::list<std::string>::iterator iter = svr_lst.begin();
	for (; iter != svr_lst.end(); ++iter)
	{
		const std::string & svr_name = *iter;

		std::string strCmdlst = config.getString("checkcmd", svr_name);
		std::list<unsigned int> cmd_lst;
		splitStrToLst(strCmdlst, ',', cmd_lst);
		std::list<unsigned int>::iterator iter = cmd_lst.begin();
		for (; iter != cmd_lst.end(); ++iter)
		{
			unsigned int req_cmd = *iter;
			char cCmd[32] = {0};
			sprintf(cCmd, "%u", req_cmd);
			std::string strRspLst = config.getString("checkcmd", cCmd);

			std::list<unsigned int> rsp_lst;
			splitStrToLst(strRspLst, ',', rsp_lst);

			std::list<unsigned int>::iterator iter_rsp = rsp_lst.begin();
			for (; iter_rsp != rsp_lst.end(); ++iter_rsp)
			{
				m_rsp_cmd_map.insert(std::make_pair(*iter_rsp, req_cmd));
			}

			if (m_svr_name_cmd_map.end() != m_svr_name_cmd_map.find(svr_name))
			{
				m_svr_name_cmd_map[svr_name].insert(req_cmd);
			}
			else
			{
				std::set<unsigned int> req_set;
				req_set.insert(req_cmd);
				m_svr_name_cmd_map.insert(std::make_pair(svr_name, req_set));
			}
		}
	}

	unsigned int alarm_cmd_timeout = config.getInt("checkcmd", "alarm_cmd_timeout");
	LOG_PRINT(log_info, "[checkcmd]alarm_cmd_timeout:%u,svr_name:%s.", alarm_cmd_timeout, svrnames.c_str());
	if (alarm_cmd_timeout)
	{
		req_rsp_checktime = alarm_cmd_timeout;
	}
}

//get request cmd by response cmd
unsigned int CCmdConfigMgr::getReqbyRspCmd(unsigned int rsp_cmd)
{
	std::map<unsigned int, unsigned int>::iterator iter = m_rsp_cmd_map.find(rsp_cmd);
	if (iter != m_rsp_cmd_map.end())
	{
		return iter->second;
	}
	else
	{
		return 0;
	}
}

//get set of request cmd by server name
void CCmdConfigMgr::getReqbySvrName(const std::string & svr_name, std::set<unsigned int> & req_set)
{
	req_set.clear();
	if (m_svr_name_cmd_map.end() != m_svr_name_cmd_map.find(svr_name))
	{
		req_set.insert(m_svr_name_cmd_map[svr_name].begin(), m_svr_name_cmd_map[svr_name].end());
	}
}

//check if the request cmd need to check.
bool CCmdConfigMgr::needCheckRspTime(unsigned int rsp_cmd)
{
	std::map<unsigned int, unsigned int>::iterator iter = m_rsp_cmd_map.find(rsp_cmd);
	if (iter != m_rsp_cmd_map.end())
	{
		return true;
	}
	else
	{
		return false;
	}
}

unsigned int CCmdConfigMgr::getReqRspCheckTime()
{
	return req_rsp_checktime;
}
