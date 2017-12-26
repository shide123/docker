/*
 * CConnInfo.h
 *
 *  Created on: Feb 17, 2016
 *      Author: root
 */

#ifndef __CALARM_NOTIFY_H__
#define __CALARM_NOTIFY_H__

#define ALARMNOTIFY
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "yc_datatypes.h"
#include "cmd_vchat.h"

class CAlarmNotify
{
private:
	CAlarmNotify(std::string & ip, unsigned int port);
	~CAlarmNotify();
public:
	static void init(std::string & ip, unsigned int port);
    //content only support char[512]
    static int sendAlarmNoty(E_NOTICE_TYPE notitype, E_ALARM_TYPE alarmtype, const std::string & appname, const std::string & title, const std::string & groups, const std::string & content);

    //content only support char[512]
    static int sendAlarmNoty(E_NOTICE_TYPE notitype, E_ALARM_TYPE alarmtype, const std::string & appname, const std::string & title, const std::string & groups, const char * format, ...);

private:
    static std::string m_ipaddress;
    static unsigned int m_port;
};

#undef ALARMNOTIFY
#endif /* __CALARM_NOTIFY_H__ */
