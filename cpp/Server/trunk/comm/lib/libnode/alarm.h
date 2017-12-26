#ifndef _ALARM_INTF_H__
#define _ALARM_INTF_H__

#include "cmd_vchat.h"

#include <string>

class IAlarm
{
public:
	typedef const char * cont_char;

	enum en_conn_type {
		en_conn_tcp,
		en_conn_udp,
	};

	static IAlarm * Create(const std::string & ip, int port, int intval, 
		en_conn_type type = en_conn_tcp);
	
public:
	IAlarm(const std::string & ip, int port, int intval)
		: m_ip(ip)
		, m_port(port)
		, m_interval(intval)
	{}
	
	virtual ~IAlarm() {}

	virtual int send(const char * fmt, ...) = 0;

	virtual void setinfo(cont_char name, cont_char group, cont_char title)
	{
		m_appname 	= name;
		m_group		= group;
		m_title		= title;
	}

protected:
	std::string	m_ip;
	int			m_port;
	
	std::string m_appname;
    std::string m_group;
    std::string m_title;

	int 		m_interval;
};

#endif
