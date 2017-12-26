#ifndef __JJCJ_SDK_MESSAGE_HELPER_H_20160601__
#define __JJCJ_SDK_MESSAGE_HELPER_H_20160601__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "message_comm.h"

#include <new>
#include <unistd.h>

template<typename T>
T * Construct(char * p)
{
    ::new(p) T();
    return (T *)p;
}

template<typename T>
T * GetObject(const char * p)
{
    return (T *)p;
}

template<typename subcmd_type>
subcmd_type * GetAndFillHeader(char * ptr, uint8 version, 
    uint8 checkcode, uint16 maincmd, uint16 subcmd)
{
    COM_MSG_HEADER * header = Construct<COM_MSG_HEADER>(ptr);

    header->version     = version;
	header->checkcode   = checkcode;
	header->maincmd     = maincmd;
	header->subcmd      = subcmd;
    header->length      = sizeof(COM_MSG_HEADER) + sizeof(subcmd_type);
    
    return Construct<subcmd_type>(header->content);
}

void GetAndFillHeader(char * ptr, uint8 version, 
    uint8 checkcode, uint16 maincmd, uint16 subcmd);

int32 GetMessageLength(const char * ptr);

uint16 GetMainCmd(const char * ptr);

uint16 GetSubCmd(const char * ptr);

uint8 GetVersion(const char * ptr);

uint8 GetCheckCode(const char * ptr);

bool CheckMessageLength(const char * ptr, int32 checksize);

template<typename subcmd_type>
bool CheckMessageLength(const char * ptr, int32 addlen = 0)
{
    COM_MSG_HEADER * header = GetObject<COM_MSG_HEADER>(ptr);
    return ((size_t)header->length == (sizeof(COM_MSG_HEADER) + sizeof(subcmd_type) + addlen));
}

template<typename subcmd_type>
subcmd_type * GetSubCmdObject(const char * ptr)
{
    COM_MSG_HEADER * header = GetObject<COM_MSG_HEADER>(ptr);
    return GetObject<subcmd_type>(header->content);
}

#endif //__JJCJ_SDK_MESSAGE_HELPER_H_20160601__
