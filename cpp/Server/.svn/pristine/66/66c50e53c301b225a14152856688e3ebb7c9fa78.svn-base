#include "StdAfx.h"
#include "Client_Handler.h"
#include "../../src/SL_Socket_CommonAPI.h"
#include "../../../Include/IMDefine.h"
#include "../../../Include/IMLog.h"
#include "SL_Socket_Source.h"

int Client_Handler::connect_success_ = 0;
Client_Handler::Client_Handler(void)
{
}

Client_Handler::~Client_Handler(void)
{
}


int Client_Handler::handle_connect()
{
    printf("connect success!\n");
    //std::string strTest="beging a packet!";
  /*  for (int i=0;i<1000;i++)
    {
        this->write_message(strTest.c_str(),strTest.size()+1);
        if (i%100 == 0)
            SL_Socket_CommonAPI::util_sleep(50);
    }*/
    //strTest="123456789,123456789,123456789";
    //write_message(strTest.c_str(),strTest.size()+1);

    //IM_MSG_FULL im_msg = {0};
    //im_msg.length  = IM_MSGHEAD_FULL_LEN;

    connect_success_++;
    struct MSG
    {
        int len;
        char msgcontent[100];
    };
    MSG msg;
    strncpy(msg.msgcontent, "张保元abc11", 100);
    msg.len = strlen(msg.msgcontent)+sizeof(int)+1;
    write_data((char*)&msg, msg.len);

    strncpy(msg.msgcontent, "张保元abc2222", 100);
    msg.len = strlen(msg.msgcontent)+sizeof(int)+1;
    write_data((char*)&msg, msg.len);

    //SL_Socket_CommonAPI::util_sleep(1000);

    strncpy(msg.msgcontent, "张保元abc333333", 100);
    msg.len = strlen(msg.msgcontent)+sizeof(int)+1;
    write_data((char*)&msg, msg.len);

    strncpy(msg.msgcontent, "张保元abc44444444", 100);
    msg.len = strlen(msg.msgcontent)+sizeof(int)+1;
    write_data((char*)&msg, msg.len);

    strncpy(msg.msgcontent, "张保元abc5555555555", 100);
    msg.len = strlen(msg.msgcontent)+sizeof(int)+1;
    write_data((char*)&msg, msg.len);

    return 0;
}

int Client_Handler::handle_close()
{
    printf("socket:%d connect close!\n", socket_);
    return 0;
}

int Client_Handler::handle_read(const char *msg, int len)
{
    //printf("%d,%s socket:%ld\n",len, msg+4, socket_);
    //IM_Log::print(this->get_socket_source()->get_log(), 5, &im_msg);

    struct MSG
    {
        int len;
        char msgcontent[100];
    };
    MSG *msg_real = (MSG*)msg;
    //printf("socket:%d, msg: %s\n", socket_, msg_real->msgcontent);

    //MSG msg;

    //IM_MSG_FULL *im_msg = (IM_MSG_FULL*)msg;
    //IM_Log::print(this->get_socket_source()->get_log(), 5, im_msg);
    write_data(msg, len);

    //::Sleep(1);

    return 0;
}
