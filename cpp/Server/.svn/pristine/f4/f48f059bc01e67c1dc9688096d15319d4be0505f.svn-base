#include "rpcclienthandler.h"
#include "rpcserver.h"
#include <vector>
#include <iostream>
#include <sstream>
#include "utils.h"
#include "CLogThread.h"
using namespace std;

rpcclienthandler::rpcclienthandler(boost::asio::io_service & ioservice,TProcessor* processor)
:m_processor(processor),ioservice_(ioservice),socket_(ioservice)
{
	recv_buf_remainlen_ = 0;
}

rpcclienthandler::~rpcclienthandler()
{
	//
}

int rpcclienthandler::open(int fd)
{
	t_socket.reset(new TSocket());
	t_socket->setSocketFD(fd);

	inputMemoryBuffer.reset(new TMemoryBuffer(NULL, 0));
	inputtransports.reset(new THttpServer(inputMemoryBuffer));
	inputprotocol.reset(new TBinaryProtocol(inputtransports));

	outputtransports.reset(new THttpServer(t_socket));
	outputprotocol.reset(new TBinaryProtocol(outputtransports));

	socket_.async_read_some(boost::asio::buffer((void *)recv_buf_, en_msgbuffersize),
		boost::bind(&rpcclienthandler::handle_read, shared_from_this(),
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
	return 0;
}

void rpcclienthandler::close()
{
	printf("close\n");
	t_socket->close();

	// close any factory produced transports
	inputMemoryBuffer->close();
	inputtransports->close();
	outputtransports->close();

	// release processor and handler
//	processor_.reset();
}

std::string printString2Hex(const char * cArray, unsigned int cArraySize)
{
	std::stringstream strLog;
	for (unsigned int i = 0; i < cArraySize; ++i)
	{
		unsigned char c = *(cArray + i);
		char binary[32] = {0};
		sprintf(binary, "%02x ", c);
		strLog << binary;
	}
	return strLog.str();
}

int rpcclienthandler::parsePkg()
{
	char* p = recv_buf_;
	string msg = p;
	transform(msg.begin(),msg.end(),msg.begin(),::tolower);
	size_t found = msg.find("\r\n\r\n");
	//read header
	if (found == string::npos)
	{
		return 0;
	}
	int msgpacklen = 0;
	if(msg.find("post") == 0)
	{
		// to catch the content length
		// find first pos of Content-Length
		msg = msg.substr(0, found + 4);
		int line_start = msg.find("content-length:");
		if (line_start == string::npos)
		{
			memmove(recv_buf_, p + found + 4, recv_buf_remainlen_ - found - 4); 
			LOG_PRINT(log_error, "msg packet is wrong.msg binary:%s.", printString2Hex(msg.c_str(), msg.size()).c_str());
			return 0;
		}
		
		string str = msg.substr(line_start, found);
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
	}
	
	if(recv_buf_remainlen_<msgpacklen)
	{
		return 0;
	}
	inputMemoryBuffer->resetBuffer((uint8_t*)recv_buf_, msgpacklen);
	transition();
	recv_buf_remainlen_ -= msgpacklen;
	p += msgpacklen;
	memmove(recv_buf_, p, recv_buf_remainlen_);
	recv_buf_[recv_buf_remainlen_] = 0;
	return 0;
}

void rpcclienthandler::handle_read(const boost::system::error_code & e, std::size_t bytes_transferred)
{
	if(!e) {
		recv_buf_remainlen_ += bytes_transferred;
		recv_buf_[recv_buf_remainlen_] = 0;
		int ret = parsePkg();
		if (ret == -1)
		{
			close();
			return;
		}
		
		socket_.async_read_some(boost::asio::buffer(recv_buf_+recv_buf_remainlen_, en_msgbuffersize - 1),
			boost::bind(&rpcclienthandler::handle_read, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}
	else if(e)
	{
		if(e != boost::asio::error::try_again && e != boost::asio::error::would_block && e != boost::asio::error::operation_aborted)
		{
			close();
		}
	}
}

void rpcclienthandler::transition()
{

	if(m_processor)
	{
//		try
//		{
//			m_processor->process(inputprotocol,outputprotocol,0);
//		}
//		catch(...)
//		{
//			printf("parse msg error! \n");
//		}
		m_processor->process(inputprotocol,outputprotocol,0);
	}
}
