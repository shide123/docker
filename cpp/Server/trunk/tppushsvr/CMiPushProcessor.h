/*
 * CMiPushProcessor.h
 *
 *  Created on: 2016年4月9日
 *      Author: root
 */

#ifndef __CMIPUSH_PROCESSOR_H__
#define __CMIPUSH_PROCESSOR_H__

#include <set>
#include <map>
#include "json/json.h"
#include "Config.h"


class CMiPushProcessor
{
public:
	typedef std::map<std::string, std::string> VALUE_MAP;

	CMiPushProcessor();

	~CMiPushProcessor();

    bool Init(Config &cfg, const char* Main = "MiPush");
        
	int send_msg(const std::string & title, const std::string & description, const std::string & notify_id, const std::map<unsigned int, std::string> & regid_set, const std::string & payload, VALUE_MAP extra = VALUE_MAP());
	
	int send_msg_ex(const std::string & title, const std::string & description, const std::string & notify_id, const std::string & regids, const std::string & payload, VALUE_MAP extra = VALUE_MAP());

	unsigned int get_regid_count_config();

	unsigned int get_loadconfig_activetime();

	unsigned int get_checkconfig_activetime();

private:

    int build_regid_msg(const std::string & title, const std::string & description, const std::string & notify_id, const std::string & regids, const std::string & payload, std::string & pkt, VALUE_MAP extra = VALUE_MAP());
    
    std::string m_Authorization;
    
    std::string m_restricted_package_name;

    std::string m_pushmsg_url;
    
    std::string m_notify_type;   
    
    std::string m_pass_through;
    
    unsigned int m_posttime_ms;

	std::string m_time_to_live;

	unsigned int m_regidcount;

	unsigned int m_push_retry_times;

};

#endif /* __CMIPUSH_PROCESSOR_H__ */
