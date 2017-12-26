/*
 * CMZPushProcessor.cpp
 *
 *  Created on: 2016.4.9
 *      Author: root
 */

#include "CMZPushProcessor.h"
#include "CLogThread.h"
#include "HttpClient.h"
#include "Config.h"
#include "curl.h"
#include <boost/locale/conversion.hpp>
#include <boost/locale/encoding.hpp>
#include "CryptoHandler.h"
CMZPushProcessor::CMZPushProcessor()
{	 
}

CMZPushProcessor::~CMZPushProcessor()
{
}

bool CMZPushProcessor::Init(Config &cfg, const char* Main)
{
    LOG_PRINT(log_info, "start:%s", Main);
    do
	{
    	std::string contenttype;
    	std::string userAgent;
    	cfg.get(contenttype, Main, "Content-Type");
    	cfg.get(userAgent, Main, "User-Agent");

        cfg.get(m_pushmsg_url, Main, "pushmsg_url");
        cfg.get(m_appid,Main,"appid");
        cfg.get(m_secretkey,Main,"secretkey");
        cfg.get(m_apikey,Main,"apikey");
        cfg.get(m_notify_type, Main, "notify_type");
        cfg.get(m_pass_through, Main, "pass_through");
        cfg.get(m_time_to_live, Main, "time_to_live"); //unit s
    	cfg.get(m_posttime_ms, Main, "http_post_timeout_ms", 3000);
    	cfg.get(m_regidcount, Main, "regid_count_once", 800);
    	cfg.get(m_push_retry_times, Main, "push_retry_times", 3);
    	string header;
    	header.assign("Content-Type:");
    	header.append(contenttype);
    	m_vheader.push_back(header);
        return true;    
    } while(0);
 	return false;
}

int CMZPushProcessor::build_regid_msg(const std::string & title, const std::string & description, const std::string & notify_id, const std::string & regids, const std::string & payload, std::string & pkt, VALUE_MAP extra/* = VALUE_MAP()*/)
{
	if (m_regids.empty()) return -1;

	CHttpClient httpClient;
	pkt.append("appId=");
	pkt.append(m_appid);

//	string title1 = "123";
//	string description1 = "258";
	string msg = stringFormat("{\"noticeBarInfo\":{\"title\":\"%s\",\"content\":\"%s\"}}",title.c_str(),description.c_str());
	pkt.append("&messageJson=");
	pkt.append(httpClient.Urlencode(msg));

	pkt.append(regids);
	pkt.append(httpClient.Urlencode(m_regids));

	pkt.append("&sign=");
	pkt.append(genSign(msg));

	return 0;
}

int CMZPushProcessor::send_msg(const std::string & title, const std::string & description, const std::string & notify_id, const std::map<unsigned int, std::string> & regid_set, const std::string & payload, VALUE_MAP extra/* = VALUE_MAP()*/)
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
        regids.assign("&pushIds=");
        m_regids.clear();
        for (; iter != regid_set.end();)
        {
        	m_regids.append(iter->second);
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
        send_msg_ex(title, description, notify_id, regids , payload, extra);
    }
	return 0;
}
int CMZPushProcessor::send_msg_ex(const std::string & title, const std::string & description, const std::string & notify_id, const std::string & regids, const std::string & payload, VALUE_MAP extra/* = VALUE_MAP()*/)
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

unsigned int CMZPushProcessor::get_regid_count_config()
{
	return m_regidcount;
}
string CMZPushProcessor::genSign(std::string msg)
{
	string methodUpper;
	string sign;
	sign.append("appId=");
	sign.append(m_appid);
	sign.append("messageJson=");
	sign.append(msg);
	sign.append("pushIds=");
	sign.append(m_regids);
	sign.append(m_secretkey);

//	CHttpClient httpClient;
//	string encodeStr = httpClient.Urlencode(sign);
	string md5str;
	cryptohandler::md5hash(sign,md5str);
	transform(md5str.begin(),md5str.end(),md5str.begin(),::tolower);
	LOG_PRINT(log_info, "[%d]Post genSign sign:%s,md5str:%s", sign.c_str(),md5str.c_str());
	return md5str;
}
