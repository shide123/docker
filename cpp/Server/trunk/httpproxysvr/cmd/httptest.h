#ifndef HTTP_TEST_H_
#define HTTP_TEST_H_

#include "http_server.h"

class HttpTest
{
public:
	HttpTest();
	virtual ~HttpTest();

	static void hello(Request &request, Json::Value &root);
	static void sayhello(Request &request, Json::Value &root);
	static void login(Request &request, Json::Value &root);
	static void usleep(Request &request, Response &response);

};

#endif