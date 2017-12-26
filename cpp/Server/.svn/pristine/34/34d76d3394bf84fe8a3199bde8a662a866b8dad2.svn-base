#ifndef HTTP_CORE_H_
#define HTTP_CORE_H_

#include <string>
#include "http_server.h"
#include "Config.h"

using namespace std;
class HttpCore
{
public:
	HttpCore();
	virtual ~HttpCore();

	static void initSig();
	static void initCfg();
	static void init();
	static void loadBusiness();
	static void run();

	static string getNodeIp(){return _node_ip;}
	static int    getNodePort(){return _node_port;}

	static HttpServer _http_server;

	static string _node_ip;
	static int    _node_port;
	static int    _listen_port;
	static Config _config;
};

#endif