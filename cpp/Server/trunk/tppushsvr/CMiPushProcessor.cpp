/*
 * CMiPushProcessor.cpp
 *
 *  Created on: 2016.4.9
 *      Author: root
 */

#include "CMiPushProcessor.h"
#include "CLogThread.h"
#include "HttpClient.h"
#include "Config.h"
#include "curl.h"
#include <boost/locale/conversion.hpp>
#include <boost/locale/encoding.hpp>

CMiPushProcessor::CMiPushProcessor()
{	 
}

CMiPushProcessor::~CMiPushProcessor()
{
}

bool CMiPushProcessor::Init(Config &cfg, const char* Main)
{
    LOG_PRINT(log_info, "start:%s", Main);
    do
	{
        std::string Authorization;
        cfg.get(Authorization, Main, "Authorization");   
        cfg.get(m_restricted_package_name, Main, "restricted_package_name"); 
        cfg.get(m_pushmsg_url, Main, "pushmsg_url"); 
        cfg.get(m_notify_type, Main, "notify_type"); 
        cfg.get(m_pass_through, Main, "pass_through"); 
        cfg.get(m_time_to_live, Main, "time_to_live"); //unit s
    	cfg.get(m_posttime_ms, Main, "http_post_timeout_ms", 3000);
    	cfg.get(m_regidcount, Main, "regid_count_once", 800);
    	cfg.get(m_push_retry_times, Main, "push_retry_times", 3);
    	if (Authorization.empty() || m_restricted_package_name.empty() || m_pushmsg_url.empty() ||
    	    m_notify_type.empty() || m_pass_through.empty() || m_time_to_live.empty())   
        {
    	    LOG_PRINT(log_error, "incorrect config");
	        break;  
	    }   	
    	m_Authorization.assign("Authorization: key=");
    	m_Authorization.append(Authorization); 
    	m_time_to_live.append("000");	                             //unit ms               
        return true;    
    } while(0);
 	return false;
}

int CMiPushProcessor::build_regid_msg(const std::string & title, const std::string & description, const std::string & notify_id, const std::string & regids, const std::string & payload, std::string & pkt, VALUE_MAP extra/* = VALUE_MAP()*/)
{
    //设置regid
    if (regids.empty()) return -1;
    pkt.assign(regids);

    //设置标题
	pkt.append("&title=");
	pkt.append(title);
	
	//设置描述
	pkt.append("&description=");
	pkt.append(description);	
	
	//设置包名
	pkt.append("&restricted_package_name=");
	pkt.append(m_restricted_package_name);
    
    //设置提示方式 详情查看小米推送文档
	pkt.append("&notify_type=");
	pkt.append(m_notify_type);
	
    //设置提示方式 详情查看小米推送文档
	pkt.append("&notify_id=");
	pkt.append(notify_id);

    //设置未成功消息保存时长
	pkt.append("&time_to_live=");
	pkt.append(m_time_to_live);
    
    //设置是否透传
	pkt.append("&pass_through=");
	pkt.append(m_pass_through);
	
	//设置消息体	
	pkt.append("&payload=");
	pkt.append(payload);

	for (VALUE_MAP::iterator iter = extra.begin(); iter != extra.end(); iter++)
	{
		pkt.append("&");
		pkt.append(iter->first);
		pkt.append("=");
		pkt.append(iter->second);
	}
    
// 	std::string tmp = boost::locale::conv::between(pkt, "UTF-8", "GBK");
// 	pkt.swap(tmp);               
	return 0;
}

int CMiPushProcessor::send_msg(const std::string & title, const std::string & description, const std::string & notify_id, const std::map<unsigned int, std::string> & regid_set, const std::string & payload, VALUE_MAP extra/* = VALUE_MAP()*/)
{
    //设置regid
    if (regid_set.empty()) return -1;
    std::string regids;
    std::map<unsigned int, std::string>::const_iterator iter = regid_set.begin();
    
    for (; iter != regid_set.end();)
    {   
        int count = 0;
        regids.assign("registration_id=");
        for (; iter != regid_set.end();)
        {
            regids.append(iter->second);
            regids.append(",");
			++iter;
            if (++count >= m_regidcount) break;
        }
        
        if (count == 0)
        {
            LOG_PRINT(log_error, "count = 0, no regid");
            return -1;
        }
        if (regids[regids.size() - 1] == ',')
            regids.erase(regids.size() - 1, 1);
            
        send_msg_ex(title, description, notify_id, regids, payload, extra);
    }
 
	return 0;
}

int CMiPushProcessor::send_msg_ex(const std::string & title, const std::string & description, const std::string & notify_id, const std::string & regids, const std::string & payload, VALUE_MAP extra/* = VALUE_MAP()*/)
{
	std::string pkt;
	int ret = build_regid_msg(title, description, notify_id, regids, payload, pkt, extra);
	if (ret)
	{
		return ret;
	}

	LOG_PRINT(log_info, "post msg: %s", pkt.c_str());

    //每次重试都会增加长超时时长，超时时长和次数都不要太大避免卡主程序
	ret = 1;
	CHttpClient httpClient;
	for (int i = 0; i < m_push_retry_times && ret != 0; ++i)
	{
		std::string strResponse;
		ret = httpClient.Post_Header(m_Authorization, m_pushmsg_url, pkt, strResponse, m_posttime_ms*(i + 1));
		LOG_PRINT(log_info, "[%d]Post msg ret:%d,response:%s,timeout:%d,post:%s", i+1, ret, strResponse.c_str(), m_posttime_ms, pkt.c_str());
	}

	return ret;
}

unsigned int CMiPushProcessor::get_regid_count_config()
{
	return m_regidcount;
}
