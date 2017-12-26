#include "My_Task.h"
#include "IMDefine.h"
#include "IMLog.h"
#include "my_tcpserver_handler.h"
#include "My_App.h"

My_Task::My_Task(void)
{
}

My_Task::~My_Task(void)
{
}

int My_Task::svc(SL_ByteBuffer &buf)
{
    IM_MSG_FULL *im_msg = (IM_MSG_FULL*)buf.data();
    IM_Log::print(&My_App::m_log, 5, im_msg);

    //im_msg->transid++;
    //My_Socket_Handler *handler = (My_Socket_Handler*)im_msg->fromid;
    //handler->write_data(buf.data(), buf.data_size());

    return 0;
}