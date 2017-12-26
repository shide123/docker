#include "my_tcpserver_handler.h"
#include "My_App.h"
#include "SL_Socket_INET_Addr.h"
#include "SL_Socket_CommonAPI.h"
#include "SL_Socket_Runner.h"
#include "IMDefine.h"
#include "IMLog.h"
#include "SL_Sync_Guard.h"

#if 0
int My_Socket_TcpServer_Handler::handle_accept()
{
    struct sockaddr_in client_addr;
    int len=sizeof(sockaddr_in);
    int client= SL_Socket_CommonAPI::socket_accept(socket_, (struct sockaddr *)&client_addr, &len);
    
    if(client<0)
        return -1;

    SL_Socket_Handler  *client_handle=new My_Socket_Handler;
    client_handle->set_socket(client);
    client_handle->set_socket_source(socket_source_);
    client_handle->set_socket_runner(socket_runner_);
    
    client_handle->handle_open();

    int event_mask=READ_EVENT_MASK|EXCEPT_EVENT_MASK;
    
    int res=socket_runner_->add_handle(client_handle,event_mask);
    if(res<0)
    {
        client_handle->handle_close();
        SL_Socket_CommonAPI::socket_close(client_handle->get_socket());
        delete client_handle;
        return -2;
    }

    return 0;
}

#endif

int My_Socket_Handler::handle_open(SL_SOCKET fd, SL_Socket_Source *socket_source, SL_Socket_Runner *socket_runner)
{
    SL_Socket_IOChannel::handle_open(fd,socket_source,socket_runner);

    char ip[64] = {0};
    unsigned short port;
    SL_Socket_INET_Addr::get_ip_remote_s(fd, ip, 64,&port);
    printf("get socket from %s: %d\n",ip,port);

    return 0;
};
    
int My_Socket_Handler::handle_close()
{
    char ip[64]={0};
    unsigned short port;
    SL_Socket_INET_Addr::get_ip_remote_s(socket_, ip, 64,&port);
    printf("close socket- (%s:%d)\n",ip,port);

    return 0;
}

int My_Socket_Handler::process_message(const char *msg, int len)
{
    //socket_source_->get_log()->print("recv msg: %d,%s socket:%ld\n",len,msg+4,socket_);
    //write_data(msg,len);

    IM_MSG_FULL *im_msg = (IM_MSG_FULL*)msg;
    IM_Log::print(get_socket_source()->get_log(), SL_Log::LOG_LEVEL_5, im_msg);

    SL_Sync_Guard<SL_Sync_ThreadMutex> lock(this->m_mtx);
    im_msg->transid++;
    SL_ByteBuffer buf(IM_MSGHEAD_FULL_LEN);
    buf.write(msg, len);
    write_data(buf.data(), buf.data_size());

    //My_App::m_task.putq(buf);

    return 0;
}

//int My_Socket_Handler::handle_event(int event_mask)
//{
//    char ip[64]={0};
//    unsigned short port;
//    SL_Socket_INET_Addr::get_ip_remote_s(socket_, ip, 64,&port);
//
//    char buf[1024]={0};
//    if (event_mask & READ_EVENT_MASK)
//    {
//        int len=SL_Socket_CommonAPI::socket_recv(socket_,buf,1024,0);
//        if(len>0)
//        {
//            printf("recive data :%s from %s: %d\n",buf,ip, port);
//            SL_Socket_CommonAPI::socket_send(socket_,buf,len,0);
//        }
//        else if(len==0)
//        {
//            printf("recive no data from %s: %d\n",ip, port);
//            socket_runner_->del_handle(this);
//            SL_Socket_CommonAPI::socket_close(socket_);
//            socket_ = SL_INVALID_SOCKET;
//        }
//        else 
//        {
//            printf("error from %s: %d\n ",ip,port);
//            socket_runner_->del_handle(this);
//            SL_Socket_CommonAPI::socket_close(socket_);
//            return -1;
//        }
//    }
//    if (event_mask &  EXCEPT_EVENT_MASK)
//    {
//        printf("expected event from %s:%d\n",ip,port);
//    }
//
//    if (event_mask &  WRITE_EVENT_MASK)
//    {
//        printf("write event from %s: %d\n ",ip,port);
//    }
//    return 0;
//}