#ifndef __THRIFT_SVR_MGR_H__
#define __THRIFT_SVR_MGR_H__

#include <map>
#include <string>
#include "Config.h"
#include "message_vchat.h"
#include <set>

struct stClientInfo
{
	stClientInfo();
	stClientInfo(std::string ip, unsigned int port);
	bool operator ==(const stClientInfo & obj)const;
	stClientInfo & operator =(const stClientInfo & obj);
	bool operator <(const stClientInfo & obj)const;

	std::string strIP;
	unsigned int iPort;
};

class CThriftSvrMgr
{
public:
	CThriftSvrMgr();
	virtual ~CThriftSvrMgr();
	static void init(Config * pConfig);
	static void addThriftClient(const std::string & strIP, unsigned int iPort, e_SvrType serviceType, bool bNative);
	static void delThriftClient(const std::string & strIP, unsigned int iPort, e_SvrType serviceType);
	static bool getThriftClient(e_SvrType serviceType, stClientInfo & oClientOutPut);
	static unsigned int change_svrname_to_type(std::string svr_name);
	
private:

	static std::map<e_SvrType, std::set< stClientInfo> > m_native_clientinfo;
	static std::map<e_SvrType, std::set< stClientInfo> > m_remote_clientinfo;
	static std::map<std::string, unsigned int> m_svrname_type;
	static std::map<unsigned int, std::string> m_type_svrname;
	static Config * m_commConfig;
};


#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/transport/THttpClient.h>
#include <thrift/TToString.h>
using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

#include <boost/shared_ptr.hpp>
#include "CLogThread.h"

template<class ClientType>
class CThriftHttpClient
{
public:
	CThriftHttpClient(e_SvrType svrType):px(NULL)
	{
		stClientInfo host;
		if (CThriftSvrMgr::getThriftClient(svrType, host))
		{
			open(host.strIP, host.iPort);
		}
		else
		{
			LOG_PRINT(log_warning, "can not get server host, server type: %d", (int)svrType);
		}
	}

	CThriftHttpClient(std::string strIP, int iPort)
	{
		open(strIP, iPort);
	}

	virtual ~CThriftHttpClient(void)
	{
		reset();
	}

	ClientType *operator-> ()
	{
		return px;
	}

	ClientType *get()
	{
		return px;
	}

	operator bool() const
	{
		return NULL != px;
	}

	bool operator !() const
	{
		return NULL == px;
	}

	void reset()
	{
		try
		{
			if (px)
			{
				delete px;
				px = NULL;
			}
			if (protocol)
			{
				protocol.reset();
			}
			if (transport)
			{
				transport->close();
				transport.reset();
			}
		}
		catch(TException &e)
		{
			LOG_PRINT(log_error, "catch exception: %s", e.what());
		}
	}

private:
	bool open(std::string strIP, int iPort)
	{
		try
		{
			if (!strIP.empty() && iPort > 0)
			{
				transport.reset(new THttpClient(strIP, iPort, "/"));
				protocol.reset(new TBinaryProtocol(transport));
				px = new ClientType(protocol);
				transport->open();
				return true;
			}
			else
			{
				LOG_PRINT(log_error, "input host is invalid: host[%s:%d]", strIP.c_str(), iPort);
			}
		}
		catch (TTransportException &e)
		{
			LOG_PRINT(log_error, "catch exception: %s", e.what());
		}
		catch (TException &e)
		{
			LOG_PRINT(log_error, "catch exception: %s", e.what());
		}
		catch (exception &e)
		{
			LOG_PRINT(log_error, "catch exception: %s", e.what());
		}
		catch (...)
		{
			LOG_PRINT(log_error, "catch unknown exception");
		}
		reset();
		return false;
	}

private:
	boost::shared_ptr<TTransport> transport;
	boost::shared_ptr<TProtocol> protocol;
	ClientType *px;
};

#endif //__THRIFT_SVR_MGR_H__