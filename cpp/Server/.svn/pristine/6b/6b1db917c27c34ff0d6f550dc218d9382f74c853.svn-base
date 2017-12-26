#pragma once
#include "../../src/SL_Socket_UdpSource_Handler.h"

class My_UdpHandler : public SL_Socket_UdpSource_Handler
{
public:
    My_UdpHandler(void);
    ~My_UdpHandler(void);

    int process_message(const char *buf, int len, SL_Socket_INET_Addr &inet_addr)
    {
        printf("%s%\n",buf);        
        write_message(buf,len, inet_addr);
        return 0;
    }
    int handle_close()
    {
        return 0;
    }
};
