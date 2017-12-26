#include <stdlib.h>
#include<signal.h>
#include "http_core.h"
#include "httptest.h"
#include "CLogThread.h"
#include "utils.h"

HttpServer HttpCore:: _http_server;



string HttpCore:: _node_ip;
int    HttpCore:: _node_port;
int    HttpCore:: _listen_port;
Config HttpCore:: _config;
HttpCore::HttpCore()
{

}

HttpCore::~HttpCore()
{

}



void splitStrToVec(const std::string & stringInput, std::string splitChar, std::vector<std::string> & str_vec)
{
	str_vec.clear();
	if (stringInput.empty())
	{
		return;
	}

	std::string word = stringInput;
	std::string::size_type pos = word.find(splitChar);
	while (pos != std::string::npos) 
	{
		std::string sub = word.substr(0, pos);
		str_vec.push_back(sub);
		word = word.substr(pos + 1);
		pos = word.find(splitChar);
	}

	if (!word.empty())
	{
		str_vec.push_back(word);
	}
}

void handle_pipe(int sig)
{
	LOG_PRINT(log_info, "recv SIGPIPE");
	return ;
}

void HttpCore::initCfg()
{
	int line;
	std::string conf_file ="../etc/httpproxy.conf";
	_config.loadfile(conf_file.c_str());
	if(!_config.load(line))
	{
		printf( "load configure file ,%s,failed, error:%s\n",conf_file.c_str(), strerror(errno) );
		exit(1) ;
	}

	string sNodeInfo = _config.getString("self","node");
	string sPort = _config.getString("self", "port");

	if (sPort.empty())
	{
		printf( "port is null\n" );
		exit(1) ;
	}

	_listen_port=atoi(sPort.c_str());

	std::vector<std::string>  vec;
	splitStrToVec(sNodeInfo, ":",vec );

	if (vec.size()==2)
	{
		_node_ip=vec[0];
		_node_port=atoi(vec[1].c_str());
	}
	else
	{
		printf( "node ip info error \n" );
		exit(1) ;
	}

	printf(  "_node_ip=%s,_node_port=%d \n",_node_ip.c_str(),_node_port );
}

void HttpCore::initSig()
{
	struct sigaction action;
	action.sa_handler = handle_pipe;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	sigaction(SIGPIPE, &action, NULL);

}

void HttpCore::init()
{

	initSig();
	initCfg();

	char logfile[256];
	sprintf(logfile,"%s_log", "httpproxy");

	SL_Log *m_log = new SL_Log();
	m_log->init(2, 1, "/../log/", logfile, "txt", 2);
	CLogThread::Instance(m_log, 1);

	LOG_PRINT(log_info, "httpproxy is running");
	LOG_PRINT(log_info, "log name[%s] mode[%d:%d]",logfile, 2, 1);
	

}


void HttpCore::loadBusiness()
{
	_http_server.add_mapping("/hello", HttpTest::hello);
	_http_server.add_mapping("/usleep", HttpTest::usleep);
	_http_server.add_mapping("/sayhello", HttpTest::sayhello);
	_http_server.add_mapping("/login", HttpTest::login, GET_METHOD | POST_METHOD);
}

void HttpCore::run()
{
	_http_server.add_bind_ip("0.0.0.0");
	_http_server.set_port(_listen_port);
	_http_server.set_backlog(100000);
	_http_server.set_max_events(100000);
	//http_server.add_bind_ip("192.168.238.158");
	_http_server.start_async();
	//sleep(1);
	//http_server.stop();
	_http_server.join();
}