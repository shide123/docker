#include "CThriftSvrMgr.h"
#include <stdio.h>
#include <algorithm>
#include "CLogThread.h"

stClientInfo::stClientInfo()
{
	strIP = "";
	iPort = 0;
}

stClientInfo::stClientInfo(std::string ip, unsigned int port)
{
	strIP = ip;
	iPort = port;
}

stClientInfo & stClientInfo::operator=(const stClientInfo & obj)
{
	if (this != &obj)
	{
		strIP = obj.strIP;
		iPort = obj.iPort;
	}

	return *this;
}

bool stClientInfo::operator<(const stClientInfo & obj) const
{
	if (this->strIP < obj.strIP)
	{
		return true;
	}
	else if (this->strIP > obj.strIP)
	{
		return false;
	}
	else
	{
		return this->iPort < obj.iPort;
	}
}

bool stClientInfo::operator==(const stClientInfo & obj) const
{
	return (this->strIP == obj.strIP && this->iPort == obj.iPort);
}

std::map<e_SvrType, std::set< stClientInfo> > CThriftSvrMgr::m_native_clientinfo;
std::map<e_SvrType, std::set< stClientInfo> > CThriftSvrMgr::m_remote_clientinfo;

std::map<std::string, unsigned int> CThriftSvrMgr::m_svrname_type;
std::map<unsigned int, std::string> CThriftSvrMgr::m_type_svrname;
Config * CThriftSvrMgr::m_commConfig = NULL;

CThriftSvrMgr::CThriftSvrMgr()
{
	m_native_clientinfo.clear();
	m_remote_clientinfo.clear();
}

CThriftSvrMgr::~CThriftSvrMgr()
{
}

void CThriftSvrMgr::init(Config * pConfig)
{
	if (pConfig)
	{
		m_commConfig = pConfig;
	}
}

void CThriftSvrMgr::addThriftClient(const std::string & strIP, unsigned int iPort, e_SvrType serviceType, bool bNative)
{
	stClientInfo oClientObj(strIP, iPort);
	if (bNative)
	{
		m_native_clientinfo[serviceType].insert(oClientObj);
	}
	else
	{
		m_remote_clientinfo[serviceType].insert(oClientObj);
	}
}

void CThriftSvrMgr::delThriftClient(const std::string & strIP, unsigned int iPort, e_SvrType serviceType)
{
	stClientInfo oClientObj(strIP, iPort);
	//native
	std::map<e_SvrType, std::set< stClientInfo> >::iterator iter = m_native_clientinfo.find(serviceType);
	if (iter != m_native_clientinfo.end())
	{
		if (iter->second.end() != iter->second.find(oClientObj))
		{
			iter->second.erase(oClientObj);
		}
		
		if (iter->second.empty())
		{
			m_native_clientinfo.erase(iter);
		}
	}

	//remote
	std::map<e_SvrType, std::set< stClientInfo> >::iterator iter_remote = m_remote_clientinfo.find(serviceType);
	if (iter_remote != m_remote_clientinfo.end())
	{
		if (iter_remote->second.end() != iter_remote->second.find(oClientObj))
		{
			iter_remote->second.erase(oClientObj);
		}

		if (iter_remote->second.empty())
		{
			m_remote_clientinfo.erase(iter_remote);
		}
	}
}

bool CThriftSvrMgr::getThriftClient(e_SvrType serviceType, stClientInfo & oClientOutPut)
{
	//first find in native client info map, second find in remote client info map.
	std::map<e_SvrType, std::set< stClientInfo> >::iterator iter = m_native_clientinfo.find(serviceType);
	if (iter != m_native_clientinfo.end() && !iter->second.empty())
	{
		oClientOutPut = *(iter->second.begin());
		return true;
	}
	else
	{
		//remote client info map
		std::map<e_SvrType, std::set< stClientInfo> >::iterator iter_remote = m_remote_clientinfo.find(serviceType);
		if (iter_remote != m_remote_clientinfo.end() && !iter_remote->second.empty())
		{
			oClientOutPut = *(iter_remote->second.begin());
			return true;
		}
		else
		{
			return false;
		}
	}
}

unsigned int CThriftSvrMgr::change_svrname_to_type(std::string svr_name)
{
	std::string find_svrname = svr_name;
	std::transform(find_svrname.begin(), find_svrname.end(), find_svrname.begin(), ::tolower);

	std::map<std::string, unsigned int>::iterator iter = m_svrname_type.find(find_svrname);
	if (m_svrname_type.end() != iter)
	{
		return iter->second;
	}
	else
	{
		if (!m_commConfig)
		{
			LOG_PRINT(log_error, "m_commConfig is null, so cannot find this svr_type of %s,please initial comm.conf first.", svr_name.c_str());
			return 0;
		}

		unsigned int svr_type = m_commConfig->getInt("server", find_svrname);
		LOG_PRINT(log_info, "find config:%s.", find_svrname.c_str());
		if (!svr_type)
		{
			LOG_PRINT(log_error, "cannot find this svr_type of %s,please check comm.conf", svr_name.c_str());
			return 0;
		}

		m_svrname_type[svr_name] = svr_type;
		m_type_svrname[svr_type] = svr_name;
		return svr_type;
	}
}
