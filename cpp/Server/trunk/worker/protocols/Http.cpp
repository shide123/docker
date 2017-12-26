
#include <assert.h>
#include <string>
#include <sys/wait.h>
#include "Http.h"
#include <iostream>
#include <sstream>
#include "utils.h"
using namespace std;

//注意：当字符串为空时，也会返回一个空字符串
//vector< std::string > split(std::string& s, std::string& delim)
//{
//    size_t last = 0;
//    size_t index=s.find(delim,last);
//    std::vector< std::string > ret;
//    while (index!=std::string::npos)
//    {
//        ret.push_back(s.substr(last,index-last));
//        last=index+1;
//        index=s.find(delim,last);
//    }
//    if (index-last>0)
//    {
//        ret.push_back(s.substr(last,index-last));
//    }
//    return ret;
//}

int Http::decode(char* recv_buf_, int &recv_buf_remainlen_,clienthandler_ptr conn)
{
	char* p = recv_buf_;
	string msg = p;
	transform(msg.begin(),msg.end(),msg.begin(),::tolower);
	size_t found = msg.find("\r\n\r\n");
	if(recv_buf_remainlen_ > en_msgbuffersize)
	{
		recv_buf_remainlen_ = 0;
		return -1;
	}
	//read header
	if (found == string::npos)
	    return 0;
	int msgpacklen = 0;
	if(msg.find("post") == 0)
	{
		// find Content-Length
		int line_start = msg.find("content-length:");
		if (line_start == string::npos)
		{
			return -1;
		}
		string str = msg.substr(line_start,found);
		int line_end = str.find("\r\n",1);
		string content_length_line = str.substr(0,line_end);

		vector< string > content_len = split(content_length_line,":");
		stringstream content;
		content<<content_len[1];
		int len;
		content>>len;
		msgpacklen =found+4 + len;
	}
	else if(msg.find("get")==0)
	{
		msgpacklen = found+4;
	}
	else
	{
		//$conn->send("HTTP/1.1 400 Bad Request\r\n\r\n", true);
		recv_buf_remainlen_ = 0;
		return -1;
	}
	//read package
	if(recv_buf_remainlen_<msgpacklen)
	{
		return 0;
	}

	vector< string > http_header = split(msg,"\r\n\r\n");
	vector< string > header_data = split(http_header[0],"\r\n");
	vector< string > request_uri = split(header_data[0]," ");
//	printf("header:%s\n",msg.c_str());
//	printf("line:%s==%s\n",request_uri[0].c_str(),request_uri[1].c_str());
//	printf("====%s\n",request_uri[1].c_str());
	handle_message(request_uri[1].c_str(),request_uri[1].length() ,conn);
	recv_buf_remainlen_ -= msgpacklen;
	p += msgpacklen;

	if(recv_buf_remainlen_ >= en_msgmaxsize) {
		recv_buf_remainlen_ = 0;
		return -1;
	}

	if(p != recv_buf_ && recv_buf_remainlen_ > 0){
		memmove(recv_buf_, p, recv_buf_remainlen_);
	}
	return 0;
}

void Http::encode(char* data, int &len,SL_ByteBuffer& message)
{
	char c[128];
	sprintf(c, "%d", len);
	string str1 = c;
	string str2 = data;
	string msg = "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nServer: worker1.0.0\r\nContent-Length:"+str1+"\r\n\r\n"+str2;
//	printf("####!!!%s,,,%d\n",msg.c_str(),msg.length());
	message.write(msg.c_str(), msg.length());
}
