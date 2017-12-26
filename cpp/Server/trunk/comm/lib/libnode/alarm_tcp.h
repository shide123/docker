#ifndef __ALARM_TCP_H__
#define __ALARM_TCP_H__

#include "alarm.h"

#include <sys/time.h>
#include <netinet/in.h>

#include <vector>

class CTcpAlarm
	: public IAlarm
{
public:
	CTcpAlarm(const std::string & ip, int port, int intval);

	virtual ~CTcpAlarm();

	virtual int send(const char * fmt, ...);

	virtual void setinfo(cont_char name, cont_char group, cont_char title);
	
private:
	int sendmsg(const char * msg, int size);
	
	int copy(char * dst, int dlen, const char * src, int slen);

private:
	struct timeval 		m_timeout;
	struct sockaddr_in 	m_sockaddr;

	time_t 				m_lastsend;
	std::vector<char>	m_buf;
	CmdAlarmNotify * 	m_req;
	int					m_sendsize;
};

#endif