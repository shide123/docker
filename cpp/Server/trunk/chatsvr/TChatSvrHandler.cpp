#include "TChatSvrHandler.h"
#include <sstream>
#include "ChatLogic.h"
#include "macro_define.h"
#include "errcode.h"
#include "GroupAssistLogic.h"
#include "ChatMongoMgr.h"
#include "KeywordMgr.h"
TChatSvrHandler::TChatSvrHandler()
{
}


TChatSvrHandler::~TChatSvrHandler()
{
}
bool TChatSvrHandler::handle_keyword_action(const int32_t action, const std::string& value, const std::string& replace)
{
	//1:add 2:modify 3:del
	LOG_PRINT(log_info, "handle_keyword_action, action[%d] value[%s] replace[%s]", action, value.c_str(),replace.c_str());
	switch(action)
	{
	case 1:
		{
			CMDAdKeywordInfo_t info;
			info.naction = action;
			strcpy(info.keyword , value.c_str());
			strcpy(info.replace , replace.c_str());
			CKeywordMgr::AddKeyword(info);
		}
		break;
	case 2:
		{
			CKeywordMgr::DelKeyword((char*)value.c_str());
			CMDAdKeywordInfo_t info;
			info.naction = action;
			strcpy(info.keyword , value.c_str());
			strcpy(info.replace , replace.c_str());
			CKeywordMgr::AddKeyword(info);
		}
		break;
	case 3:
		{
			CKeywordMgr::DelKeyword((char*)value.c_str());
		}
		break;
	}

	return true;
}
bool TChatSvrHandler::handle_voice_callback(const std::string& inputkey, const int32_t code, const std::string& key)
{
	LOG_PRINT(log_info, "handle_voice_callback, inputkey[%s] key[%s] code[%d]", inputkey.c_str(), key.c_str(),code);
	if(code == 0)
	{
		CMDGroupMsgReq  req;
		if(CChatLogic::getVoiceInfo(inputkey, req))
		//	std::map<std::string, CMDGroupMsgReq>::iterator it = CChatLogic::getVoiceInfo(inputkey,req);//m_VoiceCompleteInfo.find(inputkey);
		//if(it != CChatLogic::m_VoiceCompleteInfo.end())
		{
			std::string content = "http://os700oap7.bkt.clouddn.com/";
			content += key;

			//ppt课程需要解释json
			if (req.msg().msgtype()==MSGTYPE_PPT_VOICE)
			{
				Json::Value root;
				/*
				Json::Reader reader(Json::Features::strictMode());
				if (reader.parse(req.msg().content(), root))
				{
					if (root.isMember("media_id"))
					{
						root["media_id"]=content;
						Json::FastWriter writer;  
						content=writer.write(root);
						req.mutable_msg()->set_content(content);
					}
					
				}*/

				std::string sVoiceMediaId;
				std::string sPicMediaId;
				if (!CChatLogic::getMediaId(req.msg().content(),sVoiceMediaId,sPicMediaId))
				{
					LOG_PRINT(log_error, "MSGTYPE_PPT_VOICE  parse sVoiceMediaId error, groupid: %u,req.msg().content()=%s", req.groupid(),req.msg().content().c_str());
					return false;
				}

				root["media_id"]=content;
				root["picmediaid"]=sPicMediaId;
				Json::FastWriter writer;  
				content=writer.write(root);
				req.mutable_msg()->set_content(content);

			}
			else
			{
				req.mutable_msg()->set_content(content);
			}
			
			
			CChatMongoMgr::updateMsgContent(req.msg().msgid(), req.groupid(), content);
			CChatLogic::notifyGroupMsg(req, false, CChatLogic::isMsgPushOnlineOnly(req.msg()));
			CChatLogic::delVoiceInfo(inputkey);
			LOG_PRINT(log_info, "handle_voice_callback,complete inputkey[%s] key[%s] code[%d]", inputkey.c_str(), key.c_str(),code);

			unsigned int ntime_begin = 0, ntime_end = 0;
			ntime_begin = CChatLogic::m_req_last[inputkey];
			ntime_end = SL_Socket_CommonAPI::util_process_clock_ms();
			LOG_PRINT(log_info, "------ Sub_Vchat_handle_voice_callback ms = %d.", ntime_end - ntime_begin);
			CChatLogic::m_req_last.erase(inputkey);
		}
	}

	return true;
}
bool TChatSvrHandler::procGroupChatReq(const int32_t groupid, const TChatMsg& chatMsg, const TChatMsgExtra& extra)
{
	std::ostringstream stream;
	stream << chatMsg << " " << extra;
	LOG_PRINT(log_info, "procGroupChatReq request, group[%d] msgtype[%d] msg[%s]", groupid,chatMsg.msgType, stream.str().c_str());
	if (!groupid || (!chatMsg.srcUId && (MSGTYPE_REMIND != chatMsg.msgType && MSGTYPE_LINKS != chatMsg.msgType && MSGTYPE_CLOSE_LIVE_COURSE != chatMsg.msgType
			&& MSGTYPE_STICKER != chatMsg.msgType && MSGTYPE_TIP != chatMsg.msgType)) || chatMsg.content.empty())
	{
		LOG_PRINT(log_error, "check param failed..");
		return false;
	}

	ChatMsg_t msg;
	msg.mutable_srcuser()->set_userid(chatMsg.srcUId);
	msg.mutable_dstuser()->set_userid(chatMsg.dstUId);
	msg.set_msgtype((e_MsgType)chatMsg.msgType);
	msg.set_content(chatMsg.content);
	bool ret = false;
	if (extra.isPrivate)
	{
		CMDGroupPrivateMsgReq req;
		req.set_groupid(groupid);
		req.mutable_msg()->CopyFrom(msg);
		ret = extra.pushOnline || (CChatLogic::dealGroupPMsgReq(req) == ERR_CODE_SUCCESS);
		if (ret)
		{
			CChatLogic::notifyGroupPMsg(req, extra.pushOnline);
		}
	}
	else
	{
		CMDGroupMsgReq req;
		req.set_groupid(groupid);
		req.mutable_msg()->CopyFrom(msg);
		int retcode = CChatLogic::dealGroupMsgReq(req);
		LOG_PRINT(log_warning, "dealGroupMsgReq %s, group[%u] srcuid[%u].extra.pushOnline [%d]",
				CErrMsg::strerror(ret).c_str(), req.groupid(), req.msg().srcuser().userid(),extra.pushOnline);
		ret = extra.pushOnline || (ERR_CODE_SUCCESS == retcode);
		if (ret)
		{
			CChatLogic::notifyGroupMsg(req, false, extra.pushOnline, extra.memberOnly);
		}
	}

	LOG_PRINT(log_info, "procGroupChatReq %s, group[%d] msg[%s]", BOOL_TO_RESULTSTR(ret), groupid, stream.str().c_str());

	return ret;
}

bool TChatSvrHandler::procUserJoinGroupNotify(const int32_t userid, const int32_t groupid, const int32_t roletype)
{
	LOG_PRINT(log_info, "UserJoinGroupNotify request, group[%d] user[%d] roletype[%d].", groupid, userid, roletype);
	bool ret = CChatLogic::procUserJoinGroupNotify(groupid, userid, roletype);
	LOG_PRINT(log_info, "UserJoinGroupNotify %s, group[%d] user[%d].", BOOL_TO_RESULTSTR(ret), groupid, userid);
	return ret;
}

int32_t TChatSvrHandler::proc_addGroupAssistMsg(const TGroupAssistMsg & assistMsg, const bool bNotyAdmin, const bool bNotyUser)
{
	StGPAssistMsg_t oMsg;
	oMsg.userID = assistMsg.userID;
	oMsg.groupID = assistMsg.groupID;
	oMsg.msgSvrType = assistMsg.svrType;
	oMsg.msgSvrSwitch = assistMsg.svrSwitch;
	oMsg.msgSvrLevel = assistMsg.svrLevel;
	oMsg.msgSvrLevelUp = assistMsg.svrLevelUp;
	oMsg.msgState = assistMsg.state;
	oMsg.url = assistMsg.url;
	oMsg.msgType = assistMsg.msgType;
	oMsg.msgTime = time(NULL);
	oMsg.userContext = assistMsg.msg;
	oMsg.authID = assistMsg.authID;
	oMsg.investUserID = assistMsg.investuserID;
	oMsg.packetid = assistMsg.packetID;
	oMsg.balance = assistMsg.balance;

	if (CGroupAssistLogic::addAssistMsg(oMsg, bNotyAdmin, bNotyUser))
	{
		return oMsg.msgId;
	}
	else
	{
		return 0;
	}
}

bool TChatSvrHandler::handle_approveGroupAssistMsg(const int32_t handle_userid, const int32_t groupid, const int32_t msgid, const bool agree, const int32_t apply_userid)
{
	return CGroupAssistLogic::approveAssistMsg(handle_userid, groupid, msgid, agree, apply_userid);
}


bool TChatSvrHandler::proc_optPPTPic( const std::vector<TPPTPicInfo>& vecPicId,const int32_t optType) 
{
	LOG_PRINT(log_info, "from php req,proc_optPPTPic request, vecPicId.size=%d,optType[%d].", vecPicId.size(),optType);
	bool ret = CChatLogic::procOptPPTPic(vecPicId, optType);

	return ret;
}

bool TChatSvrHandler::proc_commentAudit( const TChatAuditMsg &tTChatMsg) 
{
	LOG_PRINT(log_info, "from php req,proc_commentAudit, srcUId=%d,groupid=%d,msgtype=%d,notifyType=%d,msgid=%d,pmsgid=%d,updateid=%d,coentent[%s].", 
		tTChatMsg.srcUId,tTChatMsg.groupId,tTChatMsg.msgType,tTChatMsg.notifyType,tTChatMsg.msgId,tTChatMsg.mastermsgId,tTChatMsg.updateId,tTChatMsg.content.c_str());
	if (!tTChatMsg.srcUId|| !tTChatMsg.groupId || !tTChatMsg.notifyType||tTChatMsg.content.empty())
	{
		return false;
	}

	return CChatLogic::procCommentAudit(tTChatMsg);
}
