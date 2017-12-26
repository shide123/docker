#ifndef SOCKETLITE_SOCKET_MESSAGE_HANDLER_H
#define SOCKETLITE_SOCKET_MESSAGE_HANDLER_H

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

class SL_Socket_Message_Handler : public SL_Socket_Handler
{
public:
    SL_Socket_Message_Handler()
        : need_len_(0)
        , last_left_(0)
    {
    }

    virtual ~SL_Socket_Message_Handler() 
    {
    }

    virtual int handle_open(SL_SOCKET fd, SL_Socket_Source *socket_source, SL_Socket_Runner *socket_runner);
    int handle_event(int event_mask);
    int write_message(const char *msg, int len);

    //取得消息长度
    //目前没有考虑跨平台的字节顺序
    //一般字节顺序:BIG Endian，LITTLE Endian
    static int get_msglen(const char *msg, int len, int msglen_bytes, int byteorder);

    int             need_len_;      //组成一个消息包，还需要长度
    int8            last_left_;     //当收数据长度小于消息长度所占长度时
    SL_ByteBuffer   msg_buffer_;    //消息缓存
};

#endif
