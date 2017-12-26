#include <assert.h>
#include <string>
#include <sys/wait.h>
#include "Frame.h"
using namespace std;

//ProtocolsBase* ProtocolsBase::app = 0;
Frame::Frame(void)
{
}


Frame::~Frame(void)
{

}

//void Frame::input(char* recv_buffer, int &len)
//{
//	//
//}

int Frame::decode(char* recv_buf_, int &recv_buf_remainlen_,clienthandler_ptr conn)
{
	char* p = recv_buf_;

	while(recv_buf_remainlen_ > 4) 
	{
		int msgpacklen = *((int*)p);
		if(msgpacklen <= 0 || msgpacklen > en_msgbuffersize) 
		{
			LOG_PRINT(log_error, "msgpacklen invalid.msgpacklen:%d.", msgpacklen);
			recv_buf_remainlen_ = 0;
			return -1;
		}
		else if(recv_buf_remainlen_ < msgpacklen) 
		{
			break;
		}
		else 
		{
			handle_message(p, msgpacklen,conn);
			recv_buf_remainlen_ -= msgpacklen;
			p += msgpacklen;
		}
	}

	if(p != recv_buf_ && recv_buf_remainlen_ > 0)
	{
		memmove(recv_buf_, p, recv_buf_remainlen_);
	}
	return 0;
}

void Frame::encode(char* data, int &len,SL_ByteBuffer& message)
{
	message.write(data, len);
}
