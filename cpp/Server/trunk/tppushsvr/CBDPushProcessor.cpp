/*
 * CBDPushProcessor.cpp
 *
 *  Created on: 2016.4.9
 *      Author: root
 */

#include "CBDPushProcessor.h"
#include "CLogThread.h"
#include "HttpClient.h"
#include "Config.h"
#include "curl.h"
#include <boost/locale/conversion.hpp>
#include <boost/locale/encoding.hpp>
#include "md5.h"
CBDPushProcessor::CBDPushProcessor()
{	 
}

CBDPushProcessor::~CBDPushProcessor()
{
}

bool CBDPushProcessor::Init(Config &cfg, const char* Main)
{
    LOG_PRINT(log_info, "start:%s", Main);
    do
	{
    	std::string contenttype;
    	std::string userAgent;
    	cfg.get(contenttype, Main, "Content-Type");
    	cfg.get(userAgent, Main, "User-Agent");

        cfg.get(m_pushmsg_url, Main, "pushmsg_url");
        m_pushmsg_url.append(PUSH_BATCH_DEVICE);
        cfg.get(m_appid,Main,"appid");
        cfg.get(m_secretkey,Main,"secretkey");
        cfg.get(m_apikey,Main,"apikey");
        cfg.get(m_notify_type, Main, "notify_type");
        cfg.get(m_pass_through, Main, "pass_through");
        cfg.get(m_time_to_live, Main, "time_to_live"); //unit s
    	cfg.get(m_posttime_ms, Main, "http_post_timeout_ms", 3000);
    	cfg.get(m_regidcount, Main, "regid_count_once", 800);
    	cfg.get(m_push_retry_times, Main, "push_retry_times", 3);
//    	if (m_restricted_package_name.empty() || m_pushmsg_url.empty() ||
//    	    m_notify_type.empty() || m_pass_through.empty() || m_time_to_live.empty())
//        {
//    	    LOG_PRINT(log_error, "incorrect config");
//	        break;
//	    }
    	string header;
    	header.assign("Content-Type:");
    	header.append(contenttype);
    	m_vheader.push_back(header);
    	header.assign("User-Agent:");
    	header.append(userAgent);
    	m_vheader.push_back(header);
    	m_time_to_live.append("000");	                             //unit ms               
        return true;    
    } while(0);
 	return false;
}

int CBDPushProcessor::build_regid_msg(const std::string & title, const std::string & description, const std::string & notify_id, const std::string & regids, const std::string & payload, std::string & pkt, VALUE_MAP extra/* = VALUE_MAP()*/)
{
    //设置regid
    if (regids.empty()) return -1;
    	pkt.assign(regids);
    CHttpClient httpClient;
    pkt.append(httpClient.Urlencode(m_regids));
    string msg = stringFormat("{\"title\":\"%s\",\"description\":\"%s\"}",title.c_str(),description.c_str());

	pkt.append("&msg=");
	pkt.append(httpClient.Urlencode(msg));
    pkt.append("&msg_type=");
    pkt.append("1");

    pkt.append("&apikey=");
    pkt.append(m_apikey);

    pkt.append("&device_type=");
    pkt.append("3");

    m_timestamp = stringFormat("%u",time(0));
    pkt.append("&timestamp=");
    pkt.append(m_timestamp);
    pkt.append("&sign=");
    pkt.append(genSign(msg));

	return 0;
}

int CBDPushProcessor::send_msg(const std::string & title, const std::string & description, const std::string & notify_id, const std::map<unsigned int, std::string> & regid_set, const std::string & payload, VALUE_MAP extra/* = VALUE_MAP()*/)
{
    //设置regid
    if (regid_set.empty()) return -1;
    std::map<unsigned int, std::string>::const_iterator iter = regid_set.begin();
    
    string regids;
    Json::Value jval;
    for (; iter != regid_set.end();)
    {
    	jval.clear();
        int count = 0;
        regids.assign("channel_ids=");
        m_regids.assign("[");
        for (; iter != regid_set.end();)
        {
        	m_regids.append("\"");
        	m_regids.append(iter->second);
        	m_regids.append("\"");
        	m_regids.append(",");
			++iter;
            if (++count >= m_regidcount) break;
        }
        if (count == 0)
        {
            LOG_PRINT(log_error, "count = 0, no regid");
            return -1;
        }
        if (m_regids[m_regids.size() - 1] == ',')
        	m_regids.erase(m_regids.size() - 1, 1);
        m_regids.append("]");
        send_msg_ex(title, description, notify_id, regids , payload, extra);
    }
	return 0;
}
int CBDPushProcessor::send_msg_ex(const std::string & title, const std::string & description, const std::string & notify_id, const std::string & regids, const std::string & payload, VALUE_MAP extra/* = VALUE_MAP()*/)
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
		ret = httpClient.Post_Header2(m_vheader, m_pushmsg_url,pkt , strResponse, m_posttime_ms*(i + 1));
		LOG_PRINT(log_info, "[%d]Post msg ret:%d,response:%s,timeout:%d,post:%s", i+1, ret, strResponse.c_str(), m_posttime_ms, pkt.c_str());
	}

	return ret;
}

unsigned int CBDPushProcessor::get_regid_count_config()
{
	return m_regidcount;
}
string CBDPushProcessor::genSign(std::string msg)
{
	string methodUpper;
	string sign = "POST";
	sign.append(m_pushmsg_url);
	sign.append("apikey=");
	sign.append(m_apikey);
	sign.append("channel_ids=");
	sign.append(m_regids);
	sign.append("device_type=");
	sign.append("3");
	sign.append("msg=");
	sign.append(msg);
	sign.append("msg_type=");
	sign.append("1");
	sign.append("timestamp=");
	sign.append(m_timestamp);
	sign.append(m_secretkey);

	CHttpClient httpClient;
	string encodeStr = httpClient.Urlencode(sign);
//	MD5 md5(encodeStr);
	string md5str;// = md5.toString();
	cryptohandler::md5hash(encodeStr,md5str);
	transform(md5str.begin(),md5str.end(),md5str.begin(),::tolower);
	LOG_PRINT(log_info, "[%d]Post genSign sign:%s,md5str:%s", sign.c_str(),md5str.c_str());
	return md5str;
}
