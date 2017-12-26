#include "SL_Socket_Message_Handler.h"
#include "SL_Socket_CommonAPI.h"
#include "SL_Socket_Source.h"

int SL_Socket_Message_Handler::handle_open(SL_SOCKET fd, SL_Socket_Source *socket_source, SL_Socket_Runner *socket_runner)
{
    SL_Socket_Handler::handle_open(fd, socket_source, socket_runner);
    msg_buffer_.reserve(socket_source_->get_msgbuffer_size());
    msg_buffer_.reset();
    need_len_   = 0;
    last_left_  = 0;
    return 0;
}

int SL_Socket_Message_Handler::handle_event(int event_mask)
{
    if (event_mask & READ_EVENT_MASK)
    {
        int  res = 0;
        int  errorid = 0;
        int  msg_byteorder    = socket_source_->get_msg_byteorder();
        int  msglen_bytes     = socket_source_->get_msglen_bytes();
        int  msgbuffer_size   = socket_source_->get_msgbuffer_size();
        int  recv_buffer_size = socket_runner_->get_recv_buffer_size();
        char *recv_buffer     = socket_runner_->get_recv_buffer();

        int  pos_size;
        char *pos;
        while (1)
        {
            res = SL_Socket_CommonAPI::socket_recv(socket_, recv_buffer, recv_buffer_size, 0, NULL, &errorid);
            pos = recv_buffer;
            pos_size = res;
            if (res > 0)
            {
split_packet:
                if (pos_size > 0)
                {
                    if (need_len_ > 0)
                    {
                        last_left_ = 0;
	                    if (pos_size >= need_len_)
	                    {
                            msg_buffer_.write(pos, need_len_);
                            handle_read(msg_buffer_.data(), msg_buffer_.data_size());
                            msg_buffer_.data_end(0);
                            pos       += need_len_;
                            pos_size  -= need_len_;
                            need_len_  = 0;
                            goto split_packet;
	                    }
                        else
                        {
                            msg_buffer_.write(pos, pos_size);
                            need_len_ -= pos_size;
                            pos_size   = 0;
                        }
                    }
                    else
                    {//nedd_size<=0
                        if (last_left_ == 0)
                        {
                            msg_buffer_.data_end(0);
                            if (pos_size >= msglen_bytes)
                            {
                                int msglen = SL_Socket_Message_Handler::get_msglen(pos, pos_size, msglen_bytes, msg_byteorder);
                                if ((msglen <= 0) || (msglen > msgbuffer_size))
                                {//消息大小无效或超过消息缓冲大小
                                    need_len_  = 0;
                                    last_left_ = 0;
                                    return -1;
                                }
                                if (pos_size >= msglen)
                                {
                                    handle_read(pos, msglen);
                                    pos      += msglen;
                                    pos_size -= msglen;
                                    goto split_packet;
                                }
                                else
                                {
                                    msg_buffer_.write(pos, pos_size);
                                    need_len_ = msglen - pos_size;
                                    pos_size  = 0;
                                }
                            }
                            else
                            {
                                msg_buffer_.write(pos, pos_size);
                                last_left_ = pos_size;
                                pos_size   = 0;
                            }
                        }
                        else
                        {
                            if ((last_left_+pos_size) < msglen_bytes)
                            {
                                msg_buffer_.write(pos, pos_size);
                                last_left_ += pos_size;
                                pos_size    = 0;
                            }
                            else
                            {
                                msg_buffer_.write(pos, msglen_bytes-last_left_);
                                pos      += (msglen_bytes-last_left_);
                                pos_size -= (msglen_bytes-last_left_);
                                int msglen = SL_Socket_Message_Handler::get_msglen(msg_buffer_.data(), msglen_bytes, msglen_bytes, msg_byteorder);
                                if ((msglen <= 0) || (msglen > msgbuffer_size))
                                {//消息大小无效或超过消息缓冲大小
                                    need_len_  = 0;
                                    last_left_ = 0;
                                    return -1;
                                }
                                last_left_ = 0;

                                if ((pos_size+msglen_bytes) >= msglen)
                                {
                                    msg_buffer_.write(pos, msglen-msglen_bytes);
                                    handle_read(msg_buffer_.data(), msglen);
                                    msg_buffer_.data_end(0);
                                    pos      += (msglen-msglen_bytes);
                                    pos_size -= (msglen-msglen_bytes);
                                    goto split_packet;
                                }
                                else
                                {
                                    msg_buffer_.write(pos, pos_size);
                                    need_len_ = msglen-pos_size-msglen_bytes;
                                    pos_size  = 0;
                                }
                            }
                        }
                    }//need_size>0
                }
                if (res < recv_buffer_size)
                {
                    return 0;
                }
                else
                {
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

int SL_Socket_Message_Handler::write_message(const char *msg, int len)
{
    int msglen_bytes = socket_source_->get_msglen_bytes();
    int datalen = msglen_bytes + len;
    if (datalen > msg_buffer_.buffer_size())
    {
        return -1;
    }

    SL_ByteBuffer buf;
    switch (msglen_bytes)
    {
        case 1:
            {
                char i = datalen;
                buf.reserve(datalen);
                buf.write(i);
            }
            break;
        case 2:
            {
                short i = datalen;
                buf.reserve(datalen);
                buf.write(i);
            }
            break;
        case 4:
            {
                buf.reserve(datalen);
                buf.write(datalen);
            }
            break;
        case 8:
            {
                int64 i = datalen;
                buf.reserve(datalen);
                buf.write(i);
            }
            break;
        default:
            return -1;
    }
    buf.write(msg, len);
    if (socket_source_->get_socket_handler() == this)
    {
        return socket_source_->send(buf.data(), buf.data_size(), 0);
    }
    return SL_Socket_CommonAPI::socket_send(socket_, buf.data(), buf.data_size());
}

//取得消息长度
//目前没有考虑跨平台的整数字节顺序
//一般字节顺序:BIG Endian，LITTLE Endian
int SL_Socket_Message_Handler::get_msglen(const char *msg, int len, int msglen_bytes, int byteorder)
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
        break;
    default:
        return -1;
    }
    return msglen;
}
