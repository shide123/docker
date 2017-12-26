#include "ServerMsgMgr.h"
#include "AppPush.h"
#include "CLogThread.h"
#include "json/json.h"
#include "utils.h"
#include <boost/locale/conversion.hpp>
#include <boost/locale/encoding.hpp>

enum{
	e_fans_call = 1,
};

enum 
{
    e_JPush         = 0,
    e_MiPush        = 1,
    e_HWPush        = 2,
    e_IOSPush       = 3,
	e_BDPush		= 4,
	e_OPPush		= 5,
	e_MZPush		= 6,

	e_MAXPush,//alway last
};

ServerMsgMgr::ServerMsgMgr(void)
{
}

ServerMsgMgr::~ServerMsgMgr(void)
{
}

bool ServerMsgMgr::Init(Config &cfg, const char* Main)
{
    LOG_PRINT(log_info, "start:%s", Main);
    do
	{
        if (!m_apush_processor.Init(cfg))
	        break;
        if (!m_mipush_processor.Init(cfg))
	        break;	
        if (!m_bdpush_processor.Init(cfg))
        	break;
        if (!m_mzpush_processor.Init(cfg))
            break;
        if (!m_hwpush_processor.Init(cfg))
            break;
        return true;    
    } while(0);
 	return false;
} 

bool ServerMsgMgr::onConnect(clienthandler_ptr connection)
{    
	LOG_PRINT(log_info, "conn:%u %s:%u", connection->getconnid(), connection->getremote_ip(), connection->getremote_port());	
	return true;
}

bool ServerMsgMgr::onMessage(task_proc_data* message)
{
    COM_MSG_HEADER* in_msg = (COM_MSG_HEADER*)message->pdata;    
	LOG_PRINT(log_info, "message msgtye:%d,len:%d,main:%d:sub:%d",message->msgtye,message->datalen,in_msg->maincmd,in_msg->subcmd);
	if (in_msg->subcmd == Sub_Vchat_ClientHello)
	{    
        return true;
    }
	
	switch(in_msg->subcmd)
	{	
	default:
		LOG_PRINT(log_warning, "Unknown subcmd:%u.", in_msg->subcmd);
		break;
	}
    
	return true;
}

bool ServerMsgMgr::onClose(clienthandler_ptr connection)
{
	LOG_PRINT(log_info, "conn:%u %s:%u", connection->getconnid(), connection->getremote_ip(), connection->getremote_port());	
	return true;
}

bool ServerMsgMgr::onError(clienthandler_ptr connection, int code, string msg)
{
    LOG_PRINT(log_error, "conn:%u %s:%u,error code:%d, %s.", connection->getconnid(), connection->getremote_ip(), connection->getremote_port(), code, msg.c_str() );
	return true;
}
enum
{
	e_ATLIST_INDEX = 0,
	e_FOLLOW_INDEX = 1,
	e_NORMAL_INDEX = 2,
	e_MAX_INDEX,
};

bool ServerMsgMgr::procNotifyChatMsg(const TChatMsgPush& msg, const int32_t groupid, const bool notify_offline)
{
	std::stringstream os;
	msg.printTo(os);
	LOG_PRINT(log_info, "notify chat msg: {%s}", os.str().c_str());

	if (msg.title.empty() || msg.description.empty() || !msg.srcuid || groupid < 0)
		return false;

	if ((msg.msg_type != e_PushMsgType::PrivateChat && 0 == groupid) 
	 || (msg.msg_type != e_PushMsgType::GroupChat && 0 == msg.userids.size()))
		return false;


	CDBSink sink;
	Result res;
	int row = 0;
	std::map<uint32, std::string> pushToken_set[e_MAX_INDEX][e_MAXPush];
	std::set<uint32> filterSet;
	std::stringstream sql;
	std::string strSql;
	bool atAllMember = false;
	if (e_PushMsgType::GroupChat == msg.msg_type)	//群聊
	{
		if (msg.userids.size())
		{
			std::string cond;
			for (int i = 0; i < msg.userids.size(); i++)
			{
				if (msg.userids[i] <= 0)
				{
					atAllMember = true;
					strSql = stringFormat(" c and gm.group_id = %d", groupid);
					break;
				}
				else if (cond.size())
				{
					cond += ",";
				}
				cond += bitTostring(msg.userids[i]);
			}

			if (!atAllMember)
			{
				strSql = stringFormat(" select ti.user_id,push_type,push_token from mc_push_token_inf ti, mc_group_member gm "
									   " where ti.user_id = gm.user_id and gm.is_del = 0 and ti.status = 1 and gm.user_id in( %s ) and gm.group_id = %d", cond.c_str(), groupid);
			}

			//@人
			row = 0;
			if (sink.run_sql(strSql.c_str(), &res, &row) && row > 0)
			{
				getPushSet(sink, res, row, pushToken_set[e_ATLIST_INDEX], e_MAXPush, msg.srcuid, filterSet, notify_offline);
				logPushSet(pushToken_set[e_ATLIST_INDEX], e_MAXPush, "有人@我");
			}
			sink.CleanRes(&res);
		}

		if (!atAllMember)
		{
			//特别关心
			strSql = stringFormat(" select ti.user_id,push_type,push_token from mc_push_token_inf ti, mc_group_member gm "
								   " where ti.user_id = gm.user_id and gm.is_del = 0 and ti.status = 1 "
								   " and exists(select 1 from mc_user_friendship uf where uf.user_id = ti.user_id and uf.relation = 1 and uf.friend_id = %d) "
								   " and gm.group_id = %d", msg.srcuid, groupid);
			row = 0;
			if (sink.run_sql(strSql.c_str(), &res, &row) && row > 0)
			{
				getPushSet(sink, res, row, pushToken_set[e_FOLLOW_INDEX], e_MAXPush, msg.srcuid, filterSet, notify_offline);
				logPushSet(pushToken_set[e_FOLLOW_INDEX], e_MAXPush, "特别关心");
			}
			sink.CleanRes(&res);

			//普通没有开启免打扰的用户
			strSql = stringFormat(" select ti.user_id,push_type,push_token from mc_push_token_inf ti, mc_group_member gm "
								   " where ti.user_id = gm.user_id and gm.is_del = 0 and ti.status = 1 "
								   " and gm.group_id = %d ", groupid);
			if (!msg.include_muted)
			{
				strSql += " and gm.mute_notifications = 0 ";
			}
			row = 0;
			if (sink.run_sql(strSql.c_str(), &res, &row) && row > 0)
			{
				getPushSet(sink, res, row, pushToken_set[e_NORMAL_INDEX], e_MAXPush, msg.srcuid, filterSet, notify_offline);
				logPushSet(pushToken_set[e_NORMAL_INDEX], e_MAXPush, "普通推送");
			}
			sink.CleanRes(&res);
		}
	}
	else
	{
		strSql = stringFormat("select ti.user_id,ti.push_type,ti.push_token from mc_push_token_inf ti where ti.user_id = %d and ti.status = 1 ", msg.userids[0]);
		if (sink.run_sql(strSql.c_str(), &res, &row) && row > 0)
		{
			getPushSet(sink, res, row, pushToken_set[e_NORMAL_INDEX], e_MAXPush, msg.srcuid, filterSet, notify_offline);
			logPushSet(pushToken_set[e_NORMAL_INDEX], e_MAXPush, "普通私聊");
		}
		sink.CleanRes(&res);
	}

	std::stringstream userList;
	sink.CleanRes(&res);

	//LOG_PRINT(log_info, "group[%u] send_user[%u] msg_type[%d], notification user list [%s]", groupid, msg.srcuid, msg.msg_type, userList.str().c_str());
	//generate action json
	Json::Value jsonPayload;
	jsonPayload["msg_type"] = msg.msg_type;
// 	jsonPayload["title"] = msg.title;
// 	jsonPayload["description"] = msg.description;
	jsonPayload["create_time"] = (uint32)msg.create_time;
	Json::Value jsonActionParam;
	std::string strNotifyId;
	switch (msg.msg_type)
	{
	case e_PushMsgType::GroupChat:
		strNotifyId = stringFormat("%05u", groupid%10000);
		jsonPayload["action"] = "open_group_chat";
		jsonActionParam["groupid"] = groupid;
		break;
	case e_PushMsgType::GroupPrivateChat:
		strNotifyId = stringFormat("%05u", msg.srcuid%10000);
		jsonPayload["action"] = "open_group_private_chat";
		jsonActionParam["groupid"] = groupid;
		jsonActionParam["to_userid"] = msg.srcuid;
		break;
	case e_PushMsgType::PrivateChat:
		strNotifyId = stringFormat("%05u", msg.srcuid%10000);
		jsonPayload["action"] = "open_private_chat";
		jsonActionParam["to_userid"] = msg.srcuid;
		break;
	default:
		return false;
	}
	if (!msg.notify_id.empty())
	{
		strNotifyId = msg.notify_id;
	}

	jsonPayload["param"] = jsonActionParam;
	std::string strPayload = Json::FastWriter().write(jsonPayload);
	CMiPushProcessor::VALUE_MAP mValues;

	std::string allDesc[e_MAX_INDEX];
//	allDesc[e_ATLIST_INDEX]= stringFormat("【有人@你】%s: %s", msg.title.c_str(), msg.description.c_str());
//	allDesc[e_FOLLOW_INDEX]= stringFormat("【特别关心】%s: %s", msg.title.c_str(), msg.description.c_str());
//	allDesc[e_NORMAL_INDEX]= stringFormat("%s: %s", msg.title.c_str(), msg.description.c_str());
	allDesc[e_ATLIST_INDEX]= stringFormat("【有人@你】%s", msg.title.c_str());
	allDesc[e_FOLLOW_INDEX]= stringFormat("【特别关心】%s", msg.title.c_str());
	allDesc[e_NORMAL_INDEX]= stringFormat("%s", msg.title.c_str());
	for (int i = 0; i < e_MAX_INDEX; i++)
	{
		if (!pushToken_set[i][e_MiPush].empty())
		{
			mValues["extra.notify_foreground"] = "0";
			Application::get_io_service().post(boost::bind(&CMiPushProcessor::send_msg, 
						&m_mipush_processor, allDesc[i], msg.description, strNotifyId, pushToken_set[i][e_MiPush], strPayload, mValues));
		}
		if (!pushToken_set[i][e_BDPush].empty())
		{
			Application::get_io_service().post(boost::bind(&CBDPushProcessor::send_msg,
						&m_bdpush_processor, allDesc[i], msg.description, strNotifyId, pushToken_set[i][e_BDPush], strPayload, mValues));
		}
		if (!pushToken_set[i][e_MZPush].empty())
		{
			Application::get_io_service().post(boost::bind(&CMZPushProcessor::send_msg,
						&m_mzpush_processor, allDesc[i], msg.description, strNotifyId, pushToken_set[i][e_MZPush], strPayload, mValues));
		}
		if (!pushToken_set[i][e_HWPush].empty())
		{
			Application::get_io_service().post(boost::bind(&CHWPushProcessor::send_msg,
						&m_hwpush_processor, allDesc[i], msg.description, strNotifyId, pushToken_set[i][e_HWPush], strPayload, mValues));
		}
		if (!pushToken_set[i][e_IOSPush].empty())
		{
			Application::get_io_service().post(boost::bind(&CApplePushProcessor::send_msg, 
						&m_apush_processor,	allDesc[i] + "\n" + msg.description, pushToken_set[i][e_IOSPush], jsonPayload));
		}
	}


	LOG_PRINT(log_info, "NotifyChatMsg end. title[%s] description[%s] payload[%s].", msg.title.c_str(), msg.description.c_str(), strPayload.c_str());
}

void ServerMsgMgr::getPushSet(CDBSink &sink, Result &res, int row, std::map<uint32, string> *pSet, int nSize, uint32 filterUser, std::set<uint32> &filterSet, bool notify_offline/* = true*/)
{
	for (int i = 0; i < row; i++)
	{
		unsigned int userid;
		sink.GetFromRes(userid, &res, i, 0);
		if (filterSet.count(userid) > 0)
			continue;

		int pushtype;
		string pushToken;
		sink.GetFromRes(pushtype, &res, i, 1);
		sink.GetFromRes(pushToken, &res, i, 2);

		if (pushToken.empty() || pushtype > nSize - 1)
		{
			LOG_PRINT(log_warning, "pushToken:%s, pushtype:%u, continue.", pushToken.c_str(), pushtype); 
			continue;
		}

		if (userid == filterUser)
			continue;

		if (notify_offline && AppInstance()->m_rdsOnlineUser.exist(userid))
			continue;

		pSet[pushtype][userid] = pushToken;
		filterSet.insert(userid);
	}
}

const char *pushName[] = 
{
    "JPush",
    "MiPush",
    "HWPush",
    "IOSPush",
	"BDPush",
	"OPPush",
	"MZPush"
};

void ServerMsgMgr::logPushSet(std::map<uint32, string> *pSet, int nSize, const char *preLog/* = ""*/)
{
	std::string userList;
	for (int i = 0; i < nSize; i++)
	{
		if (!pSet[i].empty())
		{
			userList.clear();
			std::map<uint32, string>::iterator iter = pSet[i].begin();
			for (; iter != pSet[i].end(); iter++)
			{
				if (userList.length())
				{
					userList += ",";
				}
				userList += bitTostring(iter->first);
			}
			LOG_PRINT(log_debug, "[%s] %s:{%s}", i < sizeof(pushName) ? pushName[i]: "Unknown Push", preLog, userList.c_str());
		}
	}
}
