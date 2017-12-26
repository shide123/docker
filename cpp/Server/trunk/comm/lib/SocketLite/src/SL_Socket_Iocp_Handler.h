#ifndef SOCKETLITE_SOCKET_IOCP_HANDLER_H
#define SOCKETLITE_SOCKET_IOCP_HANDLER_H

#include "SL_Config.h"
#include "SL_Socket_CommonAPI.h"
#include "SL_Socket_Handler.h"

#ifdef SOCKETLITE_OS_WINDOWS

class SL_Socket_Iocp_Handler : public SL_Socket_Handler
{
public:
    enum OPERTYPE
    {
        RECV_POSTED     = 1,
        SEND_POSTED     = 2,
        ACCEPT_POSTED   = 3
    };

    //��I/O��������
    struct PER_IO_DATA
    {
        //�ص��ṹ
        OVERLAPPED    overlapped;

        //�������ͱ�ʾ
        OPERTYPE      opertype;

        //������
        WSABUF        buffer;
        SL_ByteBuffer data_buffer;
    };

    SL_Socket_Iocp_Handler() 
    {
    }

    virtual ~SL_Socket_Iocp_Handler() 
    {
    }

    virtual int handle_open(SL_SOCKET fd, SL_Socket_Source *socket_source, SL_Socket_Runner *socket_runner)
    {
        SL_Socket_Handler::handle_open(fd, socket_source, socket_runner);
        per_io_data_.data_buffer.reserve(socket_source_->get_recvbuffer_size());
        per_io_data_.data_buffer.reset();
        per_io_data_.buffer.buf = per_io_data_.data_buffer.buffer();
        per_io_data_.buffer.len = per_io_data_.data_buffer.buffer_size();
        per_io_data_.opertype   = RECV_POSTED;
        return 0;
    }

    //Ͷ�ݽ��ղ���
    virtual int post_recv()
    {
        //����IO��������
        DWORD  flag = 0;
        DWORD  recv_byte = 0;
        ZeroMemory(&(per_io_data_.overlapped), sizeof(OVERLAPPED));

        //�ύWSARecv����
        int nRet = WSARecv(socket_, &(per_io_data_.buffer), 1, &recv_byte, &flag, &(per_io_data_.overlapped), NULL);
        if (nRet == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSA_IO_PENDING)
            {//����ʧ��,һ��Socket�Ͽ�����
                return -1;
            }
        }
        return 0;
    }

    //Ͷ�ݷ����¼�
    virtual int post_send()
    {
        return 0;
    }

protected:
    PER_IO_DATA per_io_data_;
};

#endif

#endif
