#include <stdlib.h>
#include "httptest.h"



#include "thriftUtil.h"
#include "TChatSvr.h"

#include "CLogThread.h"


HttpTest::HttpTest()
{

}

HttpTest::~HttpTest()
{

}


pthread_key_t g_tp_key;

void HttpTest::hello(Request &request, Json::Value &root) {
	root["hello"] = "world";

	//LOG_INFO("get client ip:%s", request.get_client_ip()->c_str());
	LOG_PRINT(log_info,"get client ip:%s", request.get_client_ip()->c_str());
	pthread_t t = pthread_self();
	int *tmp = (int*)pthread_getspecific(g_tp_key);
	if (tmp == NULL) {
		//LOG_INFO("not thread data, tid:%u", t);
		LOG_PRINT(log_error,"not thread data, tid:%u", t);
		return;
	}
	//LOG_INFO("get thread data:%lu", *tmp);
	LOG_PRINT(log_info,"get thread data:%lu", *tmp);
}


void HttpTest::sayhello(Request &request, Json::Value &root)
{
	std::string sGid = request.get_param("gid");
	std::string sUid = request.get_param("uid");
	std::string sContent = request.get_param("content");

	int iGid = atoi(sGid.c_str());
	int iUid= atoi(sUid.c_str());

	root["gid"] = iGid;
	root["uid"] = iUid;
	root["say"] = sContent;

	TChatSvrConcurrentClient *px =thriftUtil::geRpcClientByName<TChatSvrConcurrentClient>(string("chatsvr_ql_rpc"));

	if (!px)
	{
		LOG_PRINT(log_error, "Error ,geRpcClientByName(chatsvr_ql_rpc) ret null");
		return;
	}

	TChatMsg chatMsg;
	chatMsg.srcUId = iUid;
	chatMsg.dstUId = 0;
	chatMsg.msgType = 0;
	chatMsg.content = sContent;
	TChatMsgExtra extra;
	extra.isPrivate = false;
	if (!px->procGroupChatReq(iGid, chatMsg, extra))
	{
		LOG_PRINT(log_error, "Error callSendChatRPC.because ChatSvr return send msg fail,src_userid:%u,dst_userid:%u,groupid:%u,chatmsg:%s,private:%d.", \
			iUid, 0, iGid, sContent.c_str(), (int)0);

	}

	
	
}

void HttpTest::login(Request &request, Json::Value &root) {
	std::string name = request.get_param("name");
	std::string pwd = request.get_param("pwd");

	//LOG_DEBUG("login user which name:%s, pwd:%s", name.c_str(), pwd.c_str());
	LOG_PRINT(log_debug,"login user which name:%s, pwd:%s", name.c_str(), pwd.c_str());
	root["code"] = 0;
	root["msg"] = "login success!";
}

void HttpTest::usleep(Request &request, Response &response) {
	Json::Value root;
	std::string sleep_time = request.get_param("usleep");
	if (sleep_time.empty()) {
		root["msg"] = "usleep is empty!";
		response.set_body(root);
		return;
	}
	::usleep(atoi(sleep_time.c_str()));
	root["code"] = 0;
	root["msg"] = "success!";
	response.set_body(root);
}