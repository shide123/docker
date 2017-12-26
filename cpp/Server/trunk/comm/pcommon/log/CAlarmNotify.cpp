#include "CAlarmNotify.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory.h>
#include <sys/ioctl.h>
#include <stdarg.h>

std::string CAlarmNotify::m_ipaddress = "";
unsigned int CAlarmNotify::m_port = 0;

CAlarmNotify::CAlarmNotify(std::string & ip, unsigned int port)
{
    m_ipaddress = ip;
    m_port = port;
}

CAlarmNotify::~CAlarmNotify()
{
}

void CAlarmNotify::init(std::string & ip, unsigned int port)
{
	m_ipaddress = ip;
	m_port = port;
}

int CAlarmNotify::sendAlarmNoty(E_NOTICE_TYPE notitype, E_ALARM_TYPE alarmtype, const std::string & appname, const std::string & title, const std::string & groups, const char * format, ...)
{
    char ccontent[512] = {0};

    va_list ap;
    va_start(ap, format);
    vsprintf(ccontent, format, ap);
    va_end(ap);
    
    std::string content_str(ccontent);
    return sendAlarmNoty(notitype, alarmtype, appname, title, groups, content_str);
}

int CAlarmNotify::sendAlarmNoty(E_NOTICE_TYPE notitype, E_ALARM_TYPE alarmtype, const std::string & appname, const std::string & title, const std::string & groups, const std::string & content)
{
    if (m_ipaddress.empty() || 0 == m_port)
    {
        return -1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr  = inet_addr(m_ipaddress.c_str());
    sockAddr.sin_port         = htons(m_port);

    //100 ms
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100*1000;

    //set to no block
    unsigned long ul = 1;
    ioctl(sock, FIONBIO, &ul);

    bool bconn = false;

    if(connect(sock, (struct sockaddr*)&sockAddr, sizeof(sockAddr)) < 0)
    {
        fd_set set;
        FD_ZERO(&set);
        FD_SET(sock, &set);
        if(select(sock + 1, NULL, &set, NULL, &timeout) > 0)
        {
            int error = -1;
            int len = sizeof(int);
            getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
            if(error == 0)
            {
                bconn = true;
            }
        }
    }
    else
    {
        //connect immediately success
        bconn = true;
    }

    if (!bconn)
    {
        close(sock);
        return -1;
    }

    //set to block state
    ul = 0;
    ioctl(sock, FIONBIO, &ul);
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    char szBuf[1024] = {0};
    COM_MSG_HEADER * pOutMsg = (COM_MSG_HEADER *)szBuf;
    pOutMsg->version = MDM_Version_Value;
    pOutMsg->checkcode = 0;
    pOutMsg->maincmd = MDM_Vchat_Alarm;
    pOutMsg->subcmd = Sub_Vchat_notifyReq;
    pOutMsg->length = sizeof(COM_MSG_HEADER) + sizeof(CmdAlarmNotify);

    CmdAlarmNotify * pData = (CmdAlarmNotify *)pOutMsg->content;
    memset(pData, 0, sizeof(CmdAlarmNotify));
    pData->type = notitype;
    pData->alarmtype = alarmtype;
    strncpy(pData->title, title.c_str(), sizeof(pData->title) - 1);
    strncpy(pData->groups, groups.c_str(), sizeof(pData->groups) - 1);
	snprintf(pData->content, sizeof(pData->content) - 1, "[%s]%s", appname.c_str(), content.c_str());

    if(send(sock, szBuf, pOutMsg->length, 0) <= 0)
    {
        close(sock);
        return -1;
    }

    close(sock);
    return 0;
}

