#include "node_client.h"
#include "node_client_impl.h"

#include "CLogThread.h"

#include <string.h>

INodeClient * INodeClient::CreateObject(const char * svr, 
		boost::asio::io_service & service,
        INodeNotify * notify, const char * ip, int port, 
        std::string desc)
{
	if (svr == NULL || strlen(svr) == 0 || 
		port == 0) {

		LOG_PRINT(log_error, "invalid svrname or ip or port!");
		return NULL;
	}

	svr_name_type 	tsvr(svr);
	ip_type 		tip(ip);
	port_type 		tport(port);
	//std::string 	tdesc(desc);
        
    return new CNodeClient(tsvr, service, notify, tip, tport, desc);
}

