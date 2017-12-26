#include "SL_Socket_Message_Handler2.h"
#include "SL_Socket_CommonAPI.h"
#include "SL_Socket_Source.h"

int SL_Socket_Message_Handler2::handle_open(SL_SOCKET fd, SL_Socket_Source *socket_source, SL_Socket_Runner *socket_runner)
{
	SL_Socket_Handler::handle_open(fd, socket_source, socket_runner);
	bufLen_ = 0;
	return 0;
}

int SL_Socket_Message_Handler2::handle_event(int event_mask)
{
	if (event_mask & READ_EVENT_MASK)
	{
		int  res = 0;
		int  errorid = 0;
		int  msg_byteorder    = socket_source_->get_msg_byteorder();
		int  msglen_bytes     = socket_source_->get_msglen_bytes();
		int  msgbuffer_size   = socket_source_->get_msgbuffer_size();

		//接受数据使用的内存，这里与SL_Socket_Message_Handler不同的是使用自己的内存。
		int  recv_buffer_size = DATA_BUFSIZE * 2 - bufLen_;    //可用长度
		char *recv_buffer     = Buffer_ + bufLen_;             //内存地址

		int msglen;
		char* p;

		while (1)
		{
			res = SL_Socket_CommonAPI::socket_recv(socket_, recv_buffer, recv_buffer_size, 0, NULL, &errorid);
			if(res >0)
			{
				//总共消息长度
				bufLen_ += res;
				p = Buffer_;

				//尝试取出数据
				while(bufLen_ > msglen_bytes)
				{
					msglen = SL_Socket_Message_Handler2::get_msglen(p, bufLen_, msglen_bytes, msg_byteorder);
					if ((msglen <= 0) || (msglen > msgbuffer_size))
					{//消息大小无效或超过消息缓冲大小
						bufLen_ = 0;
						return -1;
					}
					else if(bufLen_< msglen)   //长度不够
						break;
					else
					{
						//取出数据
						handle_read(p, msglen);
						bufLen_ -= msglen;
						p += msglen;
					}
				}
				//取出数据后还能剩余一个最大消息长度,肯定有问题
				if(bufLen_ > msgbuffer_size)
				{
					bufLen_ = 0;
					return -1;
				}
				//如果有数据读出,将剩余数据前移动
				if(p != Buffer_)
				{
					memmove(Buffer_, p, bufLen_);
				}

				//
				if (res < recv_buffer_size)
				{
					return 0;
				}
				else
				{
					//更新接受数据内存地址
					recv_buffer_size = DATA_BUFSIZE * 2 - bufLen_;
					recv_buffer = Buffer_ + bufLen_;
					continue;
				}
			}                
			else
			{//出现异常(如连接已关闭)
				if (SL_EWOULDBLOCK == errorid)
				{//非阻塞模式下正常情况
					return 0;
				}
				return -1;
			}
		}
	}
	return 0;
}


//取得消息长度
//目前没有考虑跨平台的整数字节顺序
//一般字节顺序:BIG Endian，LITTLE Endian
int SL_Socket_Message_Handler2::get_msglen(const char *msg, int len, int msglen_bytes, int byteorder)
{
	if (len < msglen_bytes)
	{
		return -1;
	}

	int msglen;
	switch (msglen_bytes)
	{
	case 1:
		msglen = *msg;
		break;
	case 2:
		msglen = *((int16*)msg);
		break;
	case 4:
		msglen = *((int32*)msg);
		break;
	case 8:
		msglen = *((int64*)msg);
	default:
		return -1;
	}
	return msglen;
}
