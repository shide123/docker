#include "TUsermgrHandler.h"
#include "CLogThread.h"
#include "errcode.h"
#include "CUserBasicInfo.h"

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/transport/THttpClient.h>
#include <thrift/async/TEvhttpClientChannel.h>
#include <event.h>
#include <boost/bind.hpp>
#include <boost/function.hpp>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::async;

TUsermgrHandler::TUsermgrHandler()
{
}

TUsermgrHandler::~TUsermgrHandler()
{
}

int32_t TUsermgrHandler::proc_reloadUserInfo(const int32_t userid)
{
	if (!userid)
	{
		LOG_PRINT(log_error, "proc_reloadUserInfo input error.userid:%d.", userid);
		return ERR_CODE_INVALID_PARAMETER;
	}

	return CUserBasicInfo::loadUserBasic2Redis(userid);
}
