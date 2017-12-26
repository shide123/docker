#include "alarm.h"
#include "alarm_tcp.h"

IAlarm * IAlarm::Create(const std::string & ip, int port, int intval, 
		en_conn_type type)
{
	IAlarm * alarm = NULL;

	switch (type) {
		case en_conn_tcp:
			alarm = new CTcpAlarm(ip, port, intval);
			break;
		default:
			break;
	}

	return alarm;
}

