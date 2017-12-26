#include "message_helper.h"

#include <unistd.h>

void GetAndFillHeader(char * ptr, uint8 version, 
    uint8 checkcode, uint16 maincmd, uint16 subcmd)
{
    COM_MSG_HEADER * header = Construct<COM_MSG_HEADER>(ptr);

    header->version     = version;
	header->checkcode   = checkcode;
	header->maincmd     = maincmd;
	header->subcmd      = subcmd;
    header->length      = sizeof(COM_MSG_HEADER);
}

int32 GetMessageLength(const char * ptr)
{
    COM_MSG_HEADER * header = GetObject<COM_MSG_HEADER>(ptr);
    return header->length;
}

uint16 GetMainCmd(const char * ptr)
{
    COM_MSG_HEADER * header = GetObject<COM_MSG_HEADER>(ptr);
    return header->maincmd;
}

uint16 GetSubCmd(const char * ptr)
{
    COM_MSG_HEADER * header = GetObject<COM_MSG_HEADER>(ptr);
    return header->subcmd;
}

uint8 GetVersion(const char * ptr)
{
    COM_MSG_HEADER * header = GetObject<COM_MSG_HEADER>(ptr);
    return header->version;
}

uint8 GetCheckCode(const char * ptr)
{
    COM_MSG_HEADER * header = GetObject<COM_MSG_HEADER>(ptr);
    return header->checkcode;
}

bool CheckMessageLength(const char * ptr, int32 checksize)
{
    int32 realsize = GetMessageLength(ptr);
    if ((size_t)checksize < sizeof(COM_MSG_HEADER) || realsize != checksize) {
        return false;
    }
    return true;
}
