#ifndef THRIFTUTIL_H
#define THRIFTUTIL_H

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

#include <boost/shared_ptr.hpp>
#include "node_watcher.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

namespace thriftUtil{

	template<typename T> T*  geRpcClientByName(const std::string &sRpcServerName)
	{
		T *px=NULL;

		ServerIpInfo tServerIpInfo;

		if (NodeEpollWatcher::getServerInfoByServerName(sRpcServerName,tServerIpInfo))
		{
			boost::shared_ptr<TTransport> transport;
			boost::shared_ptr<TProtocol> protocol;


			transport.reset(new THttpClient(tServerIpInfo.sIp, tServerIpInfo.iPort, "/"));
			protocol.reset(new TBinaryProtocol(transport));
			px = new T(protocol);
			transport->open();

		}

		return px;
	}

}



#endif