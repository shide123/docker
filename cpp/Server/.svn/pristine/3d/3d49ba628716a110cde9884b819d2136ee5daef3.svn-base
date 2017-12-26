/*
 * CBDPushProcessor.h
 *
 *  Created on: 2016年4月9日
 *      Author: root
 */

#ifndef __CBDPUSH_PROCESSOR_H__
#define __CBDPUSH_PROCESSOR_H__

#include <set>
#include <map>
#include "json/json.h"
#include "Config.h"
#include "comm_struct.h"
#include "utils.h"
#include <curl.h>
#include "CryptoHandler.h"
using namespace std;
const string PUSH_SIGNLE_DEVICE = "push/single_device";                    //推送消息到单台设备
const string PUSH_ALL = "push/all";                                        //推送广播消息
const string PUSH_TAGS = "push/tags";                                      //推送组播消息
const string PUSH_BATCH_DEVICE = "push/batch_device";                      //推送消息到给定的一组设备(批量单播)
const string REPORT_QUERY_MSG_STATUS = "report/query_msg_status";          //查询消息的发送状态
const string REPORT_QUERY_TIMER_RECORDS = "report/query_timer_records";    //查询定时消息的发送记录
const string REPORT_QUERY_TOPIC_RECORDS = "report/query_topic_records";    //查询指定分类主题的发送记录
const string APP_QUERY_TAGS = "app/query_tags";                            //查询标签组列表
const string APP_CREATE_TAG = "app/create_tag";                            //创建标签组
const string APP_DEL_TAG = "app/del_tag";                                  //删除标签组
const string TAG_ADD_DEVICES = "tag/add_devices";                          //添加设备到标签组
const string TAG_DEL_DEVICES = "tag/del_devices";                          //将设备从标签组中移除
const string TAG_DEVICE_NUM = "tag/device_num";                            //查询标签组设备数量
const string TIMER_QUERY_LIST = "timer/query_list";                        //查询定时任务列表
const string TIMER_CANCEL = "timer/cancel";                                //取消定时任务
const string TOPIC_QUERY_LIST = "topic/query_list";                        //查询分类主题列表
const string REPORT_STATISTIC_MSG = "report/statistic_msg";                //当前应用的消息统计信息
const string REPORT_STATISTIC_DEVICE = "report/statistic_device";          //当前应用的设备统计信息
const string REPORT_STATISTIC_TOPIC = "report/statistic_topic";            //查询分类主题统计信息

class CBDPushProcessor
{
public:
	typedef std::map<std::string, std::string> VALUE_MAP;

	CBDPushProcessor();

	~CBDPushProcessor();

    bool Init(Config &cfg, const char* Main = "BDPush");
        
	int send_msg(const std::string & title, const std::string & description, const std::string & notify_id, const std::map<unsigned int, std::string> & regid_set, const std::string & payload, VALUE_MAP extra = VALUE_MAP());
	
	int send_msg_ex(const std::string & title, const std::string & description, const std::string & notify_id, const std::string & regids, const std::string & payload, VALUE_MAP extra = VALUE_MAP());

	unsigned int get_regid_count_config();

	unsigned int get_loadconfig_activetime();

	unsigned int get_checkconfig_activetime();

	string genSign(std::string msg);

private:

    int build_regid_msg(const std::string & title, const std::string & description, const std::string & notify_id, const std::string & regids, const std::string & payload, std::string & pkt, VALUE_MAP extra = VALUE_MAP());
    
    std::vector<std::string> m_vheader;

    std::string m_appid;

    std::string m_secretkey;

    std::string m_apikey;

    std::string m_expires;

    std::string m_timestamp;
    std::string m_method;

    std::string m_Authorization;
    
    std::string m_restricted_package_name;

    std::string m_pushmsg_url;
    
    std::string m_notify_type;   
    
    std::string m_pass_through;
    
    unsigned int m_posttime_ms;

	std::string m_time_to_live;

	unsigned int m_regidcount;

	unsigned int m_push_retry_times;

	std::string m_regids;

};

#endif /* __CBDPUSH_PROCESSOR_H__ */
