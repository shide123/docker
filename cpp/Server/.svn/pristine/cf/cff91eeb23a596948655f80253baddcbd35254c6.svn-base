#include "GroupAssistLogic.h"
#include "ChatServerApp.h"
#include "DBSink.h"
#include "msgcommapi.h"
#include "CUserBasicInfo.h"
#include "GroupBasicInfo.h"
#include "CUserGroupinfo.h"
#include "ChatMongoMgr.h"

std::string CGroupAssistLogic::m_kickoutMsgAdminFormat			= "<a href=\"userid:%u\">%s</a>被移出了<a href=\"groupid:%u\">%s</a>";
std::string CGroupAssistLogic::m_kickoutMsgUserFormat			= "你被移出了<a href=\"groupid:%u\">%s</a>";

std::string CGroupAssistLogic::m_levelchgMemUpAdminFormat		= "<a href=\"userid:%u\">%s</a>在<a href=\"groupid:%u\">%s</a>被提升为正式成员";
std::string CGroupAssistLogic::m_levelchgMemUpUserFormat		= "你在<a href=\"groupid:%u\">%s</a>被提升为正式成员";
std::string CGroupAssistLogic::m_levelchgGuestUpAdminFormat		= "<a href=\"userid:%u\">%s</a>在<a href=\"groupid:%u\">%s</a>被提升为贵宾";
std::string CGroupAssistLogic::m_levelchgGuestUpUserFormat		= "你在<a href=\"groupid:%u\">%s</a>被提升为贵宾";
std::string CGroupAssistLogic::m_levelchgAdminUpAdminFormat		= "<a href=\"userid:%u\">%s</a>在<a href=\"groupid:%u\">%s</a>被提升为管理员";
std::string CGroupAssistLogic::m_levelchgAdminUpUserFormat		= "你在<a href=\"groupid:%u\">%s</a>被提升为管理员";
std::string CGroupAssistLogic::m_levelchgOwnerUpAdminFormat		= "<a href=\"userid:%u\">%s</a>在<a href=\"groupid:%u\">%s</a>被提升为创始人";
std::string CGroupAssistLogic::m_levelchgOwnerUpUserFormat		= "你在<a href=\"groupid:%u\">%s</a>被提升为创始人";

std::string CGroupAssistLogic::m_levelchgQuitGroupAdminFormat	= "<a href=\"userid:%u\">%s</a>退出了<a href=\"groupid:%u\">%s</a>";
std::string CGroupAssistLogic::m_levelchgMemDownAdminFormat		= "<a href=\"userid:%u\">%s</a>在<a href=\"groupid:%u\">%s</a>被降低为正式成员";
std::string CGroupAssistLogic::m_levelchgMemDownUserFormat		= "你在<a href=\"groupid:%u\">%s</a>被降低为正式成员";
std::string CGroupAssistLogic::m_levelchgGuestDownAdminFormat	= "<a href=\"userid:%u\">%s</a>在<a href=\"groupid:%u\">%s</a>被降低为贵宾";
std::string CGroupAssistLogic::m_levelchgGuestDownUserFormat	= "你在<a href=\"groupid:%u\">%s</a>被降低为贵宾";
std::string CGroupAssistLogic::m_levelchgAdminDownAdminFormat	= "<a href=\"userid:%u\">%s</a>在<a href=\"groupid:%u\">%s</a>被降低为管理员";
std::string CGroupAssistLogic::m_levelchgAdminDownUserFormat	= "你在<a href=\"groupid:%u\">%s</a>被降低为管理员";

std::string CGroupAssistLogic::m_pchatOpenAdminFormat			= "<a href=\"userid:%u\">%s</a>在<a href=\"groupid:%u\">%s</a>私聊功能已开启";
std::string CGroupAssistLogic::m_pchatOpenUserFormat			= "你在<a href=\"groupid:%u\">%s</a>私聊功能已开启";
std::string CGroupAssistLogic::m_pchatCloseAdminFormat			= "<a href=\"userid:%u\">%s</a>在<a href=\"groupid:%u\">%s</a>私聊功能已关闭";
std::string CGroupAssistLogic::m_pchatCloseUserFormat			= "你在<a href=\"groupid:%u\">%s</a>私聊功能已关闭";

std::string CGroupAssistLogic::m_viewAuthAdminFormat			= "<a href=\"userid:%u\">%s</a>在<a href=\"groupid:%u\">%s</a>提交了一个视频验证请求";
std::string CGroupAssistLogic::m_viewAuthUserPassFormat			= "你在<a href=\"groupid:%u\">%s</a>的视频审核已通过";
std::string CGroupAssistLogic::m_viewAuthUserRefuseFormat		= "你在<a href=\"groupid:%u\">%s</a>的视频审核被拒绝";

std::string CGroupAssistLogic::m_msgAuthAdminFormat				= "<a href=\"userid:%u\">%s</a>在<a href=\"groupid:%u\">%s</a>提交了一个留言验证请求:";
std::string CGroupAssistLogic::m_msgAuthUserPassFormat			= "你在<a href=\"groupid:%u\">%s</a>的留言审核已通过";
std::string CGroupAssistLogic::m_msgAuthUserRefuseFormat		= "你在<a href=\"groupid:%u\">%s</a>的留言审核被拒绝";

std::string CGroupAssistLogic::m_investmsg_photo				= "来自<a href=\"groupid:%u\">%s</a>的<a href=\"userid:%u\">%s</a>邀请你上传图片";
std::string CGroupAssistLogic::m_investmsg_viewauth				= "来自<a href=\"groupid:%u\">%s</a>的<a href=\"userid:%u\">%s</a>邀请你认证视频";

std::string CGroupAssistLogic::m_redpacket_balance				= "你在<a href=\"groupid:%u\">%s</a>发的<a href=\"packetid:%u\">炮弹</a>未被领完，已退回%.2lf元，请在“我的”--“我的钱包”内进行查收";
//std::string CGroupAssistLogic::m_redpacket_balance				= "<a href=\"packetid:%u\">炮弹</a>在%d小时内未被领完，余额已退回储值账户";
std::string CGroupAssistLogic::m_dir_redpacket_balance			= "<a href=\"userid:%u\">%s</a>未在%d小时内领取你的<a href=\"packetid:%u\">炮弹</a>，已退回储值账户";

CGroupAssistLogic::CGroupAssistLogic()
{
	m_UserIDAliasMap.clear();
	m_UserIDHeadMap.clear();
	m_GroupNameMap.clear();
}

CGroupAssistLogic::~CGroupAssistLogic()
{
	m_UserIDAliasMap.clear();
	m_UserIDHeadMap.clear();
	m_GroupNameMap.clear();
}

void CGroupAssistLogic::initMsgFormat(Config & config)
{
	m_kickoutMsgAdminFormat			= config.getString("group_assist_format", "kickoutMsgAdminFormat", m_kickoutMsgAdminFormat);
	m_kickoutMsgUserFormat			= config.getString("group_assist_format", "kickoutMsgUserFormat", m_kickoutMsgUserFormat);

	m_levelchgMemUpAdminFormat		= config.getString("group_assist_format", "levelchgMemUpAdminFormat", m_levelchgMemUpAdminFormat);
	m_levelchgMemUpUserFormat		= config.getString("group_assist_format", "levelchgMemUpUserFormat", m_levelchgMemUpUserFormat);
	m_levelchgGuestUpAdminFormat	= config.getString("group_assist_format", "levelchgGuestUpAdminFormat", m_levelchgGuestUpAdminFormat);
	m_levelchgGuestUpUserFormat		= config.getString("group_assist_format", "levelchgGuestUpUserFormat", m_levelchgGuestUpUserFormat);
	m_levelchgAdminUpAdminFormat	= config.getString("group_assist_format", "levelchgAdminUpAdminFormat", m_levelchgAdminUpAdminFormat);
	m_levelchgAdminUpUserFormat		= config.getString("group_assist_format", "levelchgAdminUpUserFormat", m_levelchgAdminUpUserFormat);
	m_levelchgOwnerUpAdminFormat	= config.getString("group_assist_format", "levelchgOwnerUpAdminFormat", m_levelchgOwnerUpAdminFormat);
	m_levelchgOwnerUpUserFormat		= config.getString("group_assist_format", "levelchgOwnerUpUserFormat", m_levelchgOwnerUpUserFormat);

	m_levelchgQuitGroupAdminFormat	= config.getString("group_assist_format", "levelchgQuitGroupAdminFormat", m_levelchgQuitGroupAdminFormat);
	m_levelchgMemDownAdminFormat	= config.getString("group_assist_format", "levelchgMemDownAdminFormat", m_levelchgMemDownAdminFormat);
	m_levelchgMemDownUserFormat		= config.getString("group_assist_format", "levelchgMemDownUserFormat", m_levelchgMemDownUserFormat);
	m_levelchgGuestDownAdminFormat	= config.getString("group_assist_format", "levelchgGuestDownAdminFormat", m_levelchgGuestDownAdminFormat);
	m_levelchgGuestDownUserFormat	= config.getString("group_assist_format", "levelchgGuestDownUserFormat", m_levelchgGuestDownUserFormat);
	m_levelchgAdminDownAdminFormat	= config.getString("group_assist_format", "levelchgAdminDownAdminFormat", m_levelchgAdminDownAdminFormat);
	m_levelchgAdminDownUserFormat	= config.getString("group_assist_format", "levelchgAdminDownUserFormat", m_levelchgAdminDownUserFormat);

	m_pchatOpenAdminFormat			= config.getString("group_assist_format", "pchatOpenAdminFormat", m_pchatOpenAdminFormat);
	m_pchatOpenUserFormat			= config.getString("group_assist_format", "pchatOpenUserFormat", m_pchatOpenUserFormat);
	m_pchatCloseAdminFormat			= config.getString("group_assist_format", "pchatCloseAdminFormat", m_pchatCloseAdminFormat);
	m_pchatCloseUserFormat			= config.getString("group_assist_format", "pchatCloseUserFormat", m_pchatCloseUserFormat);

	m_viewAuthAdminFormat			= config.getString("group_assist_format", "viewAuthAdminFormat", m_viewAuthAdminFormat);
	m_viewAuthUserPassFormat		= config.getString("group_assist_format", "viewAuthUserPassFormat", m_viewAuthUserPassFormat);
	m_viewAuthUserRefuseFormat		= config.getString("group_assist_format", "viewAuthUserRefuseFormat", m_viewAuthUserRefuseFormat);

	m_msgAuthAdminFormat			= config.getString("group_assist_format", "msgAuthAdminFormat", m_msgAuthAdminFormat);
	m_msgAuthUserPassFormat			= config.getString("group_assist_format", "msgAuthUserPassFormat", m_msgAuthUserPassFormat);
	m_msgAuthUserRefuseFormat		= config.getString("group_assist_format", "msgAuthUserRefuseFormat", m_msgAuthUserRefuseFormat);

	m_investmsg_photo				= config.getString("group_assist_format", "investmsg_photo", m_investmsg_photo);
	m_investmsg_viewauth			= config.getString("group_assist_format", "investmsg_viewauth", m_investmsg_viewauth);
}

//TODO
std::string CGroupAssistLogic::genAdminNotifyString(const StGPAssistMsg_t & oMsg, const std::string & userHead, const std::string & userAlias, const std::string & groupName)
{
	char szTemp[1024] = {0};
	switch(oMsg.msgSvrType)
	{
	case MSGTYPE_KICKGROUP:
		{
			snprintf(szTemp, sizeof(szTemp), m_kickoutMsgAdminFormat.c_str(), oMsg.userID, userAlias.c_str(), oMsg.groupID, groupName.c_str());
		}
		break;
	case MSGTYPE_LEVEL_CHG:
		{
			LOG_PRINT(log_debug, "msg_userid:%u,msg_groupid:%u,msg_svrlevel:%u,level up:%d.", oMsg.userID, oMsg.groupID, oMsg.msgSvrLevel, (int)oMsg.msgSvrLevelUp);
			if (oMsg.msgSvrLevel == e_VisitorRole && oMsg.msgSvrLevelUp == false)
			{
				snprintf(szTemp, sizeof(szTemp), m_levelchgQuitGroupAdminFormat.c_str(), oMsg.userID, userAlias.c_str(), oMsg.groupID, groupName.c_str());
			}

			if (oMsg.msgSvrLevel == e_MemberRole && oMsg.msgSvrLevelUp)
			{
				snprintf(szTemp, sizeof(szTemp), m_levelchgMemUpAdminFormat.c_str(), oMsg.userID, userAlias.c_str(), oMsg.groupID, groupName.c_str());
			}

			if (oMsg.msgSvrLevel == e_MemberRole && oMsg.msgSvrLevelUp == false)
			{
				snprintf(szTemp, sizeof(szTemp), m_levelchgMemDownAdminFormat.c_str(), oMsg.userID, userAlias.c_str(), oMsg.groupID, groupName.c_str());
			}

			if (oMsg.msgSvrLevel == e_GuestRole && oMsg.msgSvrLevelUp)
			{
				snprintf(szTemp, sizeof(szTemp), m_levelchgGuestUpAdminFormat.c_str(), oMsg.userID, userAlias.c_str(), oMsg.groupID, groupName.c_str());
			}

			if (oMsg.msgSvrLevel == e_GuestRole && oMsg.msgSvrLevelUp == false)
			{
				snprintf(szTemp, sizeof(szTemp), m_levelchgGuestDownAdminFormat.c_str(), oMsg.userID, userAlias.c_str(), oMsg.groupID, groupName.c_str());
			}

			if (oMsg.msgSvrLevel == e_AdminRole && oMsg.msgSvrLevelUp)
			{
				snprintf(szTemp, sizeof(szTemp), m_levelchgAdminUpAdminFormat.c_str(), oMsg.userID, userAlias.c_str(), oMsg.groupID, groupName.c_str());
			}

			if (oMsg.msgSvrLevel == e_AdminRole && oMsg.msgSvrLevelUp == false)
			{
				snprintf(szTemp, sizeof(szTemp), m_levelchgAdminDownAdminFormat.c_str(), oMsg.userID, userAlias.c_str(), oMsg.groupID, groupName.c_str());
			}

			if (oMsg.msgSvrLevel == e_OwnerRole && oMsg.msgSvrLevelUp)
			{
				snprintf(szTemp, sizeof(szTemp), m_levelchgOwnerUpAdminFormat.c_str(), oMsg.userID, userAlias.c_str(), oMsg.groupID, groupName.c_str());
			}
		}
		break;
	case MSGTYPE_PCHAT_STATE:
		{
			LOG_PRINT(log_debug, "msg_userid:%u,msg_groupid:%u,pchat switch:%d.", oMsg.userID, oMsg.groupID, (int)oMsg.msgSvrSwitch);
			if (oMsg.msgSvrSwitch)
			{
				snprintf(szTemp, sizeof(szTemp), m_pchatOpenAdminFormat.c_str(), oMsg.userID, userAlias.c_str(), oMsg.groupID, groupName.c_str());
			}
			else
			{
				snprintf(szTemp, sizeof(szTemp), m_pchatCloseAdminFormat.c_str(), oMsg.userID, userAlias.c_str(), oMsg.groupID, groupName.c_str());
			}
		}
		break;
	case MSGTYPE_VIDEO_AUTH:
		{
			LOG_PRINT(log_debug, "msg_userid:%u,msg_groupid:%u,video auth.", oMsg.userID, oMsg.groupID);
			snprintf(szTemp, sizeof(szTemp), m_viewAuthAdminFormat.c_str(), oMsg.userID, userAlias.c_str(), oMsg.groupID, groupName.c_str());
			LOG_PRINT(log_debug, "[genAdminNotifyString]result:%s.", szTemp);
		}
		break;
	case MSGTYPE_MSG_AUTH:
		{
			LOG_PRINT(log_debug, "msg_userid:%u,msg_groupid:%u,msg auth.", oMsg.userID, oMsg.groupID);
			snprintf(szTemp, sizeof(szTemp), m_msgAuthAdminFormat.c_str(), oMsg.userID, userAlias.c_str(), oMsg.groupID, groupName.c_str());
			std::string resutl = szTemp;
			resutl += oMsg.userContext;
			LOG_PRINT(log_debug, "[genAdminNotifyString]result:%s.", resutl.c_str());
			return resutl;
		}
		break;
	}

	LOG_PRINT(log_debug, "[genAdminNotifyString]result:%s.", szTemp);
	return szTemp;
}

std::string CGroupAssistLogic::genUserNotifyString(const StGPAssistMsg_t & oMsg, const std::string & groupName, const std::string & userAlias)
{
	char szTemp[512] = {0};
	switch(oMsg.msgSvrType)
	{
	case MSGTYPE_KICKGROUP:
		{
			snprintf(szTemp, sizeof(szTemp), m_kickoutMsgUserFormat.c_str(), oMsg.groupID, groupName.c_str());
		}
		break;
	case MSGTYPE_LEVEL_CHG:
		{
			LOG_PRINT(log_debug, "msg_userid:%u,msg_groupid:%u,msg_svrlevel:%u,level up:%d.", oMsg.userID, oMsg.groupID, oMsg.msgSvrLevel, (int)oMsg.msgSvrLevelUp);
			if (oMsg.msgSvrLevel == e_MemberRole && oMsg.msgSvrLevelUp)
			{
				snprintf(szTemp, sizeof(szTemp), m_levelchgMemUpUserFormat.c_str(), oMsg.groupID, groupName.c_str());
			}

			if (oMsg.msgSvrLevel == e_MemberRole && oMsg.msgSvrLevelUp == false)
			{
				snprintf(szTemp, sizeof(szTemp), m_levelchgMemDownUserFormat.c_str(), oMsg.groupID, groupName.c_str());
			}

			if (oMsg.msgSvrLevel == e_GuestRole && oMsg.msgSvrLevelUp)
			{
				snprintf(szTemp, sizeof(szTemp), m_levelchgGuestUpUserFormat.c_str(), oMsg.groupID, groupName.c_str());
			}

			if (oMsg.msgSvrLevel == e_GuestRole && oMsg.msgSvrLevelUp == false)
			{
				snprintf(szTemp, sizeof(szTemp), m_levelchgGuestDownUserFormat.c_str(), oMsg.groupID, groupName.c_str());
			}

			if (oMsg.msgSvrLevel == e_AdminRole && oMsg.msgSvrLevelUp)
			{
				snprintf(szTemp, sizeof(szTemp), m_levelchgAdminUpUserFormat.c_str(), oMsg.groupID, groupName.c_str());
			}

			if (oMsg.msgSvrLevel == e_AdminRole && oMsg.msgSvrLevelUp == false)
			{
				snprintf(szTemp, sizeof(szTemp), m_levelchgAdminDownUserFormat.c_str(), oMsg.groupID, groupName.c_str());
			}

			if (oMsg.msgSvrLevel == e_OwnerRole && oMsg.msgSvrLevelUp)
			{
				snprintf(szTemp, sizeof(szTemp), m_levelchgOwnerUpUserFormat.c_str(), oMsg.groupID, groupName.c_str());
			}
		}
		break;
	case MSGTYPE_PCHAT_STATE:
		{
			LOG_PRINT(log_debug, "msg_userid:%u,msg_groupid:%u,pchat switch:%d.", oMsg.userID, oMsg.groupID, (int)oMsg.msgSvrSwitch);
			if (oMsg.msgSvrSwitch)
			{
				snprintf(szTemp, sizeof(szTemp), m_pchatOpenUserFormat.c_str(), oMsg.groupID, groupName.c_str());
			}
			else
			{
				snprintf(szTemp, sizeof(szTemp), m_pchatCloseUserFormat.c_str(), oMsg.groupID, groupName.c_str());
			}
		}
		break;
	case MSGTYPE_VIDEO_AUTH:
		{
			LOG_PRINT(log_debug, "msg_userid:%u,msg_groupid:%u,video auth state:%u.", oMsg.userID, oMsg.groupID, oMsg.msgState);
			if (oMsg.msgState == e_AdminAgree)
			{
				snprintf(szTemp, sizeof(szTemp), m_viewAuthUserPassFormat.c_str(), oMsg.groupID, groupName.c_str());
			}
			else if (oMsg.msgState == e_Reject)
			{
				snprintf(szTemp, sizeof(szTemp), m_viewAuthUserRefuseFormat.c_str(), oMsg.groupID, groupName.c_str());
			}
			else if (oMsg.msgState == e_NeedHandle)
			{
				snprintf(szTemp, sizeof(szTemp), m_viewAuthAdminFormat.c_str(), oMsg.userID, userAlias.c_str(), oMsg.groupID, groupName.c_str());
			}
		}
		break;
	case MSGTYPE_MSG_AUTH:
		{
			LOG_PRINT(log_debug, "msg_userid:%u,msg_groupid:%u,msg auth state:%u.", oMsg.userID, oMsg.groupID, oMsg.msgState);
			if (oMsg.msgState == e_AdminAgree)
			{
				snprintf(szTemp, sizeof(szTemp), m_msgAuthUserPassFormat.c_str(), oMsg.groupID, groupName.c_str());
			}
			else if (oMsg.msgState == e_Reject)
			{
				snprintf(szTemp, sizeof(szTemp), m_msgAuthUserRefuseFormat.c_str(), oMsg.groupID, groupName.c_str());
			}
			else if (oMsg.msgState == e_NeedHandle)
			{
				snprintf(szTemp, sizeof(szTemp), m_msgAuthAdminFormat.c_str(), oMsg.userID, userAlias.c_str(), oMsg.groupID, groupName.c_str());
			}
		}
		break;
	case MSGTYPE_INVEST_PHOTO:
		{
			snprintf(szTemp, sizeof(szTemp), m_investmsg_photo.c_str(), oMsg.groupID, groupName.c_str(), oMsg.investUserID, oMsg.investUserAlias.c_str());
		}
		break;
	case MSGTYPE_INVEST_VIEWAUTH:
		{
			snprintf(szTemp, sizeof(szTemp), m_investmsg_viewauth.c_str(), oMsg.groupID, groupName.c_str(), oMsg.investUserID, oMsg.investUserAlias.c_str());
		}
		break;
	case MSGTYPE_REDPACK_BALANCE:
		{
			//你在<a href=\"groupid:%u\">%s</a>发的<a href=\"packetid:%u\">炮弹</a>未被领完，已退回%.2lf元，请在“我的”--“储值账户”内进行查收
			snprintf(szTemp, sizeof(szTemp), m_redpacket_balance.c_str(), oMsg.groupID, groupName.c_str(), oMsg.packetid, (double)oMsg.balance / 100.0);
// 			if (oMsg.investUserID != 0)
// 			{
// 				//<a href=\"userid:%u\">%s</a>未在%d小时内领取你的<a href=\"packetid:%u\">炮弹</a>，已退回储值账户
// 				snprintf(szTemp, sizeof(szTemp), m_dir_redpacket_balance.c_str(), oMsg.investUserID, oMsg.investUserAlias.c_str(), oMsg.relate_msgid, oMsg.relate_msgid);
// 			}
// 			else
// 			{
// 				//<a href=\"packetid:%u\">炮弹</a>在%d小时内未被领完，余额已退回储值账户
// 				snprintf(szTemp, sizeof(szTemp), m_redpacket_balance.c_str(), oMsg.groupID, groupName.c_str(), oMsg.investUserID, oMsg.investUserAlias.c_str());
// 			}
		}
		break;
	}

	LOG_PRINT(log_debug, "[genUserNotifyString]result:%s.", szTemp);
	return szTemp;
}

std::string CGroupAssistLogic::genAssistMsgIcon(unsigned int msgType, const std::string & strUserHead)
{
	if (msgType == ASSISTMSG_SYSTEM)
	{
		return "";
	}
	else
	{
		return strUserHead;
	}
}

std::string CGroupAssistLogic::genAssistMsgTitle(unsigned int msgType, const std::string & strUserAlias)
{
	switch (msgType)
	{
	case ASSISTMSG_SYSTEM:
		return "系统消息";
	default:
		return strUserAlias;
	}
}

bool CGroupAssistLogic::addAssistMsg(StGPAssistMsg_t & oMsg, bool bNotyAdmin/* = true*/, bool bNotyUser/* = true*/)
{
	if (!oMsg.groupID || !oMsg.userID)
	{
		LOG_PRINT(log_error, "add assist msg input error.groupid:%u,userid:%u.", oMsg.groupID, oMsg.userID);
		return false;
	}

	std::stringstream strImportLog;
	strImportLog << "add assist msg:userid:" << oMsg.userID << ",groupid:" << oMsg.groupID << ",msgSvrType:" << oMsg.msgSvrType << ",msgType:" << oMsg.msgType << ",relateMsgID:" << oMsg.relate_msgid;
	unsigned int msgID = 0;
	if (!CChatMongoMgr::addAssistMsg2MongoDB(oMsg, msgID))
	{
		LOG_PRINT(log_error, "add assist msg to mongodb fail.%s.", strImportLog.str().c_str());
		return false;
	}

	strImportLog << ",msgID:" << msgID;
	LOG_PRINT(log_info, "add assist msg to mongodb success.%s.", strImportLog.str().c_str());
	std::set<unsigned int > adminIDSet;

	if (bNotyAdmin)
	{
		CDBSink().getGroupAdminLst_DB(oMsg.groupID, adminIDSet);
	}
	
	if (bNotyUser)
	{
		//add userid itself
		adminIDSet.insert(oMsg.userID);
	}

	unsigned int result = 0;
	std::set<unsigned int >::iterator iter_lst = adminIDSet.begin();
	for (; iter_lst != adminIDSet.end(); ++iter_lst)
	{
		unsigned int adminID = *iter_lst;
		if (adminID)
		{
			if (CChatMongoMgr::addAssistMsg2User(msgID, adminID))
			{
				++result;
			}
			else
			{
				LOG_PRINT(log_error, "add assist msg to administrator failed.%s,adminID:%u.", strImportLog.str().c_str(), adminID);
			}
		}
	}

	LOG_PRINT(log_info, "add assist msg to administrator.%s,result count:%u.", strImportLog.str().c_str(), result);

	std::string strGroupName = "";
	CGroupBasicInfo::getValue(oMsg.groupID, CGroupBasicInfo::e_Group_Name, strGroupName);

	std::string strHead = "";
	std::string strAlias = "";
	CUserBasicInfo::getUserHeadAndAlias(oMsg.userID, strHead, strAlias);

	if (bNotyAdmin)
	{
		//notify all administrator of group
		handleAssistMsgNotify(oMsg, genAdminNotifyString(oMsg, strHead, strAlias, strGroupName), false);
	}

	if (bNotyUser)
	{
		if (oMsg.investUserID)
		{
			oMsg.investUserAlias = CUserBasicInfo::getUserAlias(oMsg.investUserID);
		}

		//notify userid
		handleAssistMsgNotify(oMsg, genUserNotifyString(oMsg, strGroupName, strAlias), true);
	}
	oMsg.msgId = msgID;

	return true;
}

bool CGroupAssistLogic::approveAssistMsg(unsigned int userID, unsigned int groupID, unsigned int msgID, bool agree, unsigned int apply_userID)
{
	if (!userID || !groupID || !msgID || !apply_userID)
	{
		LOG_PRINT(log_error, "approve assist msg input error.userID:%u,groupID:%u,msgID:%u.apply_userID:%u.", userID, groupID, msgID, apply_userID);
		return false;
	}

	LOG_PRINT(log_info, "approve assist msg info input runid:%u,groupID:%u,msgID:%u,state:%d.", userID, groupID, msgID, (int)agree);
	if (!CUserGroupinfo::chkUserRoleBeyond(userID, groupID, e_AdminRole))
	{
		LOG_PRINT(log_warning, "group[%u],runid[%u] has no power to operate.", groupID, userID);
		return false;
	}

	StGPAssistMsg_t oMsg;
	if (!CChatMongoMgr::qryAssistMsgInfo(msgID, oMsg))
	{
		LOG_PRINT(log_error, "get assist msg by msgID error from mongodb.msgID:%u.", msgID);
		return false;
	}

	if (oMsg.groupID != groupID || oMsg.userID != apply_userID)
	{
		LOG_PRINT(log_error, "this assist msg has not belonged to this group or not belong to this apply userID error.msgID:%u,msgGroupID:%u,request groupID:%u,msgUserID:%u,request userID:%u.", \
			msgID, oMsg.groupID, groupID, oMsg.userID, apply_userID);
		return false;
	}

	e_MsgState value = (agree == true ? e_AdminAgree : e_Reject);
	if (!CChatMongoMgr::updateAssistMsgState(msgID, value))
	{
		LOG_PRINT(log_error, "update assist msg by msgID error.msgID:%u,agree:%d.", msgID, (int)agree);
		return false;
	}

	//notify msg state
	MsgState_t notify;
	notify.set_msgid(msgID);
	notify.set_msgtype(MSGTYPE_ASSIST);
	notify.set_state(value);

	unsigned int notyDataLen = SIZE_IVM_HEAD_TOTAL + notify.ByteSize();
	SL_ByteBuffer buff(notyDataLen);
	buff.data_end(notyDataLen);

	CMsgComm::Build_COM_MSG_HEADER(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_GPAssistMsgStateNotify, notyDataLen);
	DEF_IVM_CLIENTGATE(pGateMask, buff.buffer());

	CMsgComm::Build_BroadCastRoomRoleType(pGateMask, oMsg.groupID, e_AdminRole, 0, e_Notice_AllType, 0);

	notify.SerializeToArray(buff.buffer() + SIZE_IVM_HEAD_TOTAL, notify.ByteSize());
	AppInstance()->m_GateConnMgr.sendMsgToGate(buff.buffer(), notyDataLen);

	//generate one new msg to notify user
	StGPAssistMsg_t oNewMsg;
	oNewMsg.userID = oMsg.userID;
	oNewMsg.groupID = oMsg.groupID;
	oNewMsg.msgSvrType = oMsg.msgSvrType;
	oNewMsg.msgSvrSwitch = oMsg.msgSvrSwitch;
	oNewMsg.msgSvrLevel = oMsg.msgSvrLevel;
	oNewMsg.msgSvrLevelUp = oMsg.msgSvrLevelUp;
	oNewMsg.msgState = value;
	oNewMsg.msgType = ASSISTMSG_SYSTEM;
	oNewMsg.relate_msgid = oMsg.msgId;	//just mark relate msgID
	oNewMsg.msgTime = time(NULL);
	return addAssistMsg(oNewMsg, false, true);
}

void CGroupAssistLogic::procInvestMsgReq(task_proc_data & task_data)
{
	CMDGroupInvestUserReq req;
	if (!CMsgComm::ParseProtoMessage(task_data.pdata, task_data.datalen, req))
	{
		LOG_PRINT(log_warning, "parse [%s]  failed.", req.GetTypeName().c_str());
		return;
	}

	LOG_PRINT(log_info, "proc invest msg req:run userid:%u,groupid:%u,to userid:%u,invest type:%d.", req.run_userid(), req.groupid(), req.to_userid(), (int)req.investtype());
	
	int ret = ERR_CODE_SUCCESS;
	do 
	{
		if (!req.groupid() || !req.to_userid() || !req.run_userid())
		{
			LOG_PRINT(log_error, "proc invest msg fail,input parameter is wrong.groupid:%u,to userid:%u,run userid:%u.", req.groupid(), req.to_userid(), req.run_userid());
			ret = ERR_CODE_INVALID_PARAMETER;
			break;
		}

		unsigned int msgSvrType = 0;
		switch(req.investtype())
		{
		case e_Photo:
			msgSvrType = MSGTYPE_INVEST_PHOTO;
			break;
		case e_ViewAuth:
			msgSvrType = MSGTYPE_INVEST_VIEWAUTH;
			break;
		default:
			LOG_PRINT(log_error, "proc invest msg fail,input invest type is wrong.groupid:%u,to userid:%u,investtype:%d.", req.groupid(), req.to_userid(), (int)req.investtype());
			ret = ERR_CODE_INVALID_PARAMETER;
			break;
		}

		if (ret != ERR_CODE_SUCCESS)
		{
			break;
		}

		StGPAssistMsg_t oMsg;
		oMsg.userID = req.to_userid();
		oMsg.groupID = req.groupid();
		oMsg.msgSvrType = msgSvrType;
		oMsg.msgState = e_NotNeed;
		oMsg.msgType = ASSISTMSG_SYSTEM;
		oMsg.msgTime = time(NULL);
		oMsg.investUserID = req.run_userid();
		if (!CGroupAssistLogic::addAssistMsg(oMsg, false, true))
		{
			ret = ERR_CODE_GROUP_INVEST_FAIL;
			break;
		}

	} while (0);

	CMDGroupInvestUserResp resp;
	resp.mutable_errinfo()->set_errid(ret);
	resp.set_groupid(req.groupid());
	resp.set_investtype(req.investtype());
	resp.set_to_userid(req.to_userid());
	task_data.respProtobuf(resp, Sub_Vchat_GroupInvestUserResp);
	return;
}

void CGroupAssistLogic::handleAssistMsgNotify(const StGPAssistMsg_t & oMsg, const std::string & strNotifyMsg, bool notyUser)
{
	CMDGroupAssistMsgNotify notify;
	GPAssistMsg_t * pAssistMsg = notify.mutable_msg();
	pAssistMsg->set_msgid(oMsg.msgId);
	pAssistMsg->set_msgtime(oMsg.msgTime);
	pAssistMsg->set_state((e_MsgState)oMsg.msgState);
	pAssistMsg->set_url(oMsg.url);
	pAssistMsg->set_msg(strNotifyMsg);
	pAssistMsg->set_msgtype((e_AssistMsgType)oMsg.msgType);
	pAssistMsg->set_authid(oMsg.authID);

	std::string strHead = "";
	std::string strAlias = "";
	CUserBasicInfo::getUserHeadAndAlias(oMsg.userID, strHead, strAlias);
	pAssistMsg->set_title(genAssistMsgTitle(oMsg.msgType, strAlias));
	pAssistMsg->set_icon(genAssistMsgIcon(oMsg.msgType, strHead));

	int nNotifyLen = notify.ByteSize();
	unsigned int notyDataLen = SIZE_IVM_HEAD_TOTAL + nNotifyLen;
	SL_ByteBuffer buff(notyDataLen);
	buff.data_end(notyDataLen);

	CMsgComm::Build_COM_MSG_HEADER(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_GPAssistMsgNotify, notyDataLen);
	DEF_IVM_CLIENTGATE(pGateMask, buff.buffer());

	if (notyUser)
	{
		CMsgComm::Build_BroadCastUser_Gate(pGateMask, oMsg.userID);
	}
	else
	{
		CMsgComm::Build_BroadCastRoomRoleType(pGateMask, oMsg.groupID, e_AdminRole, 0, e_Notice_AllType, oMsg.userID);
	}
	
	notify.SerializeToArray(buff.buffer() + SIZE_IVM_HEAD_TOTAL, nNotifyLen);
	AppInstance()->m_GateConnMgr.sendMsgToGate(buff.buffer(), notyDataLen);
}

void CGroupAssistLogic::procUnreadMsgReq(task_proc_data & task_data)
{
	CMDUnreadAssistMsgReq req;
	if (!CMsgComm::ParseProtoMessage(task_data.pdata, task_data.datalen, req))
	{
		LOG_PRINT(log_warning, "parse [%s]  failed.", req.GetTypeName().c_str());
		return;
	}

	LOG_PRINT(log_info, "proc user unread assist msg req.userID:%u.", req.userid());
	std::list<StGPAssistMsg_t > unreadMsgLst;
	CChatMongoMgr::qryUnreadAssistMsgLst(req.userid(), unreadMsgLst);
	
	//send unread msg to user
	int page_cnt = 0;
	CGroupAssistLogic cache;
	CMDUnreadAssistMsgNotify msgLst;
	std::list<StGPAssistMsg_t >::iterator iter_lst = unreadMsgLst.begin();
	for (; iter_lst != unreadMsgLst.end(); ++iter_lst)
	{
		GPAssistMsg_t * pMsg = msgLst.add_msgs();
		pMsg->set_msgid(iter_lst->msgId);
		pMsg->set_msgtime(iter_lst->msgTime);
		pMsg->set_url(iter_lst->url);
		pMsg->set_state((e_MsgState)iter_lst->msgState);
		pMsg->set_msgtype((e_AssistMsgType)iter_lst->msgType);
		pMsg->set_authid(iter_lst->authID);

		unsigned int groupID = iter_lst->groupID;
		unsigned int userID = iter_lst->userID;
		std::string userAlias = "";
		std::string userHead = "";
		std::string groupName = "";
		//create CGroupAssistLogic obj to use cache.
		cache.getGroupNameFromCache(groupID, groupName);
		cache.getUserInfoFromCache(userID, userAlias, userHead);

		if (req.userid() == iter_lst->userID)
		{
			//user itself request.
			if (iter_lst->investUserID)
			{
				std::string investHead;
				cache.getUserInfoFromCache(iter_lst->investUserID, iter_lst->investUserAlias, investHead);
			}
			pMsg->set_msg(genUserNotifyString(*iter_lst, groupName, userAlias));
		}
		else
		{
			pMsg->set_msg(genAdminNotifyString(*iter_lst, userHead, userAlias, groupName));
		}

		pMsg->set_title(genAssistMsgTitle(iter_lst->msgType, userAlias));
		pMsg->set_icon(genAssistMsgIcon(iter_lst->msgType, userHead));
		
		++page_cnt;
		if (page_cnt == 50)
		{
			msgLst.set_unreadcount(page_cnt);
			task_data.respProtobuf(msgLst, Sub_Vchat_UnreadGPAssistMsgNotify);
			LOG_PRINT(log_info, "[procUnreadMsgReq]response user data:userID:%u,count:%d.", req.userid(), page_cnt);
			page_cnt = 0;
			msgLst.clear_msgs();
		}
	}

	if (page_cnt)
	{
		msgLst.set_unreadcount(page_cnt);
		task_data.respProtobuf(msgLst, Sub_Vchat_UnreadGPAssistMsgNotify);
		LOG_PRINT(log_info, "[procUnreadMsgReq]response user data:userID:%u,count:%d.", req.userid(), page_cnt);
	}

	return;
}

void CGroupAssistLogic::getUserInfoFromCache(unsigned int userID, std::string & strUserAlias, std::string & strUserHead)
{
	if (m_UserIDAliasMap.end() == m_UserIDAliasMap.find(userID) || m_UserIDHeadMap.end() == m_UserIDHeadMap.find(userID))
	{
		CUserBasicInfo::getUserHeadAndAlias(userID, strUserHead, strUserAlias);
		m_UserIDHeadMap[userID] = strUserHead;
		m_UserIDAliasMap[userID] = strUserAlias;
	}
	else
	{
		strUserHead = m_UserIDHeadMap[userID];
		strUserAlias = m_UserIDAliasMap[userID];
	}
}

void CGroupAssistLogic::getGroupNameFromCache(unsigned int groupID, std::string & groupName)
{
	if (m_GroupNameMap.end() == m_GroupNameMap.find(groupID))
	{
		CGroupBasicInfo::getValue(groupID, CGroupBasicInfo::e_Group_Name, groupName);
		m_GroupNameMap[groupID] = groupName;
	}
	else
	{
		groupName = m_GroupNameMap[groupID];
	}
}

void CGroupAssistLogic::procMsgNotifyRecv(task_proc_data & task_data)
{
	CMDAssistMsgNotifyRecv rcv;
	if (!CMsgComm::ParseProtoMessage(task_data.pdata, task_data.datalen, rcv))
	{
		LOG_PRINT(log_warning, "parse [%s]  failed.", rcv.GetTypeName().c_str());
		return;
	}

	LOG_PRINT(log_info, "proc user receive assist msg.userID:%u.receive msgID:%u.", rcv.rcvuid(), rcv.msgid());
	if (!rcv.rcvuid() || !rcv.msgid())
	{
		LOG_PRINT(log_warning, "check param failed.proc user receive assist msg.userID:%u.receive msgID:%u.", rcv.rcvuid(), rcv.msgid());
		return;
	}

	CChatMongoMgr::updateUserAssistReadMsgId(rcv.rcvuid(), rcv.msgid());
}

void CGroupAssistLogic::procSyncTaskStatusReq(task_proc_data & task_data)
{
	CMDGroupAssistMsgStateReq req;
	if (!CMsgComm::ParseProtoMessage(task_data.pdata, task_data.datalen, req))
	{
		LOG_PRINT(log_warning, "parse [%s]  failed.", req.GetTypeName().c_str());
		return;
	}

	LOG_PRINT(log_info, "proc user sync assist msg state request.userID:%u.", req.userid());

	CMDGroupAssistMsgStateResp resp;
	resp.set_userid(req.userid());

	do 
	{
		size_t count = 0;
		std::list<unsigned int > msgIDLst;
		for (int i = 0; i != req.msgids_size(); ++i)
		{
			msgIDLst.push_back(req.mutable_msgids()->Get(i));
			++count;
		}

		if (count == 0)
		{
			LOG_PRINT(log_warning, "msgID list is empty in user sync assist msg state request.userID:%u.", req.userid());
			break;
		}

		LOG_PRINT(log_info, "msgID list size:%u in user sync assist msg state request.userID:%u.", count, req.userid());

		std::list<MsgState_t > msgStateLst;
		CChatMongoMgr::qryAssistMsgState(msgIDLst, msgStateLst);
		std::list<MsgState_t >::iterator iter_lst = msgStateLst.begin();
		for (; iter_lst != msgStateLst.end(); ++iter_lst)
		{
			MsgState_t * pMsgState = resp.add_msgidstates();
			pMsgState->CopyFrom(*iter_lst);
		}

	} while (0);

	task_data.respProtobuf(resp, Sub_Vchat_GPAssistMsgStateResp);
}

void CGroupAssistLogic::procAssistMsgHisReq(task_proc_data & task_data)
{
	CMDGPAssistMsgHisReq req;
	if (!CMsgComm::ParseProtoMessage(task_data.pdata, task_data.datalen, req))
	{
		LOG_PRINT(log_warning, "parse [%s] failed.", req.GetTypeName().c_str());
		return;
	}

	if (!req.userid())
	{
		task_data.resperrinf(ERR_CODE_INVALID_PARAMETER);
		LOG_PRINT(log_info, "check param failed, user[%u].", req.userid());
		return;
	}

	LOG_PRINT(log_info, "receive assist msg history request: user[%u] forward[%d] msgid[%u] count[%d]", \
		req.userid(), (int)req.forward(), req.msgid(), req.count());

	if (0 == req.count() || req.count() > 50)
	{
		req.set_count(50);
	}

	CMDGPAssistMsgHisResp resp;
	resp.set_userid(req.userid());

	CGroupAssistLogic cache;
	int count = 0;
	std::list<StGPAssistMsg_t > lstAssistMsg;
	if (CChatMongoMgr::qryAssistMsgList(req.userid(), lstAssistMsg, req.msgid(), req.count(), req.forward()) > 0)
	{
		std::list<StGPAssistMsg_t >::iterator iter_lst = lstAssistMsg.begin();
		for (; iter_lst != lstAssistMsg.end(); ++iter_lst)
		{
			GPAssistMsg_t * pMsg = resp.add_msgs();
			pMsg->set_msgid(iter_lst->msgId);
			pMsg->set_msgtime(iter_lst->msgTime);
			pMsg->set_url(iter_lst->url);
			pMsg->set_state((e_MsgState)iter_lst->msgState);
			pMsg->set_msgtype((e_AssistMsgType)iter_lst->msgType);
			pMsg->set_authid(iter_lst->authID);

			unsigned int groupID = iter_lst->groupID;
			unsigned int userID = iter_lst->userID;
			std::string userAlias = "";
			std::string userHead = "";
			std::string groupName = "";
			//create CGroupAssistLogic obj to use cache.
			cache.getGroupNameFromCache(groupID, groupName);
			cache.getUserInfoFromCache(userID, userAlias, userHead);

			if (req.userid() == iter_lst->userID)
			{
				//user itself request.
				if (iter_lst->investUserID)
				{
					std::string investHead;
					cache.getUserInfoFromCache(iter_lst->investUserID, iter_lst->investUserAlias, investHead);
				}
				pMsg->set_msg(genUserNotifyString(*iter_lst, groupName, userAlias));
			}
			else
			{
				pMsg->set_msg(genAdminNotifyString(*iter_lst, userHead, userAlias, groupName));
			}

			pMsg->set_title(genAssistMsgTitle(iter_lst->msgType, userAlias));
			pMsg->set_icon(genAssistMsgIcon(iter_lst->msgType, userHead));

			++count;
		}
	}

	task_data.respProtobuf(resp, Sub_Vchat_GPAssistMsgHisResp);
	LOG_PRINT(log_info, "send assist msg history list: user[%u] count[%d]", req.userid(), count);
}
