#pragma once

#include "../../src/SL_Sync_Mutex.h"
#include "../../src/SL_ObjectPool.h"
#include "../../src/SL_Socket_IOChannel.h"

class Client_Handler : public SL_Socket_IOChannel
{
public:
    Client_Handler(void);
    ~Client_Handler(void);

    static int connect_success_;
private:
    int handle_connect();
    int handle_close();
    int handle_read(const char *msg, int len);


};
