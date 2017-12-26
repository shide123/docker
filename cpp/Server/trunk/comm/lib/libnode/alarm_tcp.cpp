#include "alarm_tcp.h"

#include "message_helper.h"
//#include "CLogThread.h"

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

CTcpAlarm::CTcpAlarm(const std::string & ip, int port, int intval)
	: IAlarm(ip, port, intval)
	, m_lastsend(0)
	, m_buf()
	, m_req(NULL)
{
	//connect & send timeout. 100ms
	m_timeout.tv_sec 	= 0;
    m_timeout.tv_usec 	= 100 * 1000;

    m_sockaddr.sin_family 		= AF_INET;
    m_sockaddr.sin_addr.s_addr  = inet_addr(ip.c_str());
    m_sockaddr.sin_port         = htons(port);

	m_buf.resize(1024);	//sizeof(COM_MSG_HEADER) + sizeof(CmdAlarmNotify) = 722

	m_req = GetAndFillHeader<CmdAlarmNotify>((char *)&m_buf[0], 
		MDM_Version_Value, 0, MDM_Version_Value, Sub_Vchat_notifyReq);

    m_req->type 		= e_all_notitype;
    m_req->alarmtype 	= e_network_conn;

	m_sendsize			= GetMessageLength((char *)&m_buf[0]);
}

CTcpAlarm::~CTcpAlarm()
{
	m_req = NULL;
	m_buf.clear();
}

void CTcpAlarm::setinfo(cont_char name, cont_char group, cont_char title)
{
	IAlarm::setinfo(name, group, title);

	copy(m_req->title, sizeof(m_req->title), m_title.c_str(), m_title.size());
	copy(m_req->groups, sizeof(m_req->groups), m_group.c_str(), m_group.size());

	//printf("title:%s, group:%s\n", m_req->title, m_req->groups);
}

int CTcpAlarm::copy(char * dst, int dlen, const char * src, int slen)
{
	if (dst == NULL || src == NULL) {
		return 0;
	}
	
	int cpylen = slen;
	
	if (dlen < slen) {
		cpylen = dlen - 1;
	}

	strncpy(dst, src, cpylen);

	*(dst + cpylen) = '\0';

	return cpylen;
}

int CTcpAlarm::send(const char * fmt, ...)
{
    if (m_ip.empty() || m_port <= 0) {
        return -1;
    }
    
	time_t now = time(NULL);
	if (now - m_lastsend < m_interval) {
		return -1;
	}

	char * content	= m_req->content;

	int off  = 0;
	int maxo = sizeof(m_req->content);
	int ret  = 0;
	
	if ((ret = snprintf (content + off, maxo - off, 
			"[%s]", m_appname.c_str())) < 0) {
		return -1;
	}
	off += ret;
	
	va_list ap;
    va_start(ap, fmt);
    ret = vsnprintf(content + off, maxo - off, fmt, ap);
    va_end(ap);
	
	if (ret < 0) {
		return -1;
	}
	off += ret;

	content[off] = '\0';
	
	//LOG_PRINT(log_info, "alarm:%s", content);

	if ((ret = sendmsg(&m_buf[0], m_sendsize)) > 0) {
		m_lastsend = now;
	}
	
	return ret;
}

int CTcpAlarm::sendmsg(const char * msg, int size)
{
	// 1. create socket.
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	// 2. set nonblock, use select to detect timeout.
	unsigned long ul = 1;
    ioctl(sockfd, FIONBIO, &ul);

	bool bconn = false;

	// 3. connect and detect.
	if (connect(sockfd, (struct sockaddr *)&m_sockaddr, sizeof(m_sockaddr)) < 0) {
		//printf("errno:%d, EINPROGRESS:%d\n", errno, EINPROGRESS);
		if (errno == EINPROGRESS) {
			fd_set set;
	        FD_ZERO(&set);
	        FD_SET(sockfd, &set);

			if(select(sockfd + 1, NULL, &set, NULL, &m_timeout) > 0) {
				int error 	= -1;
	            int len 	= sizeof(int);
	            getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
	            if(error == 0) {
	                bconn = true;
	            }
			}
		}
    } else {
        //connect immediately success
        bconn = true;
    }
	//printf("conn status:%d, send:%d\n", bconn, size);

	// 4. if connect failed, return.
	if (!bconn) {
		close(sockfd);
		return -1;
	}

	// 5. set block mode.
	ul = 0;
    ioctl(sockfd, FIONBIO, &ul);
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &m_timeout, sizeof(m_timeout));

	// 6. send data
	int sendn = write(sockfd, msg, size); // ::send(sockfd, msg, size, 0);
	
	// 7. close
	close(sockfd);

	return sendn;
}
