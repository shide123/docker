#ifndef SOCKETLITE_SOCKET_MESSAGE_HANDLER2_H
#define SOCKETLITE_SOCKET_MESSAGE_HANDLER2_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif
#include "SL_ByteBuffer.h"
#include "SL_Socket_Handler.h"
//SL_Socket_Message_Handler所处理数据包的格式: 数据包长度+数据包内容
//类似于结构
//struct SL_Message 
//{
//    ushort len; 
//    char   content[0];
//}

//该类与SL_Socket_Message_Handler的区别:
//1.使用自己的Recv Buffer, 直接将数据收到Buffer_后进行解码

class SL_Socket_Message_Handler2: public SL_Socket_Handler
{
public:
	SL_Socket_Message_Handler2() 
		:bufLen_(0)
	{
	}
	virtual ~SL_Socket_Message_Handler2()
	{
	}

	virtual int handle_open(SL_SOCKET fd, SL_Socket_Source *socket_source, SL_Socket_Runner *socket_runner);
	int handle_event(int event_mask);
	
	//取得消息长度
	//目前没有考虑跨平台的字节顺序
	//一般字节顺序:BIG Endian，LITTLE Endian
	static int get_msglen(const char *msg, int len, int msglen_bytes, int byteorder);

private:
	char Buffer_[DATA_BUFSIZE * 2];
	int bufLen_;
};

#endif
