#include "TRoomSvrHandler.h"
#include "CLogThread.h"
#include "DBTools.h"
#include "msgcommapi.h"
#include "message_vchat.h"
#include "cmd_vchat.h"
#include "AppDelegate.h"
#include "CUserBasicInfo.h"
#include "CUserGroupMgr.h"
#include "GroupBasicInfo.h"
#include "CUserGroupinfo.h"
#include "RoomObj.h"
#include "errcode.h"

TRoomSvrHandler::TRoomSvrHandler()
{
}

TRoomSvrHandler::~TRoomSvrHandler()
{
}

int32_t TRoomSvrHandler::proc_joingroup(const TJoinGroup & oReq)
{
	LOG_PRINT(log_info, "[proc_joingroup]userid:%d,groupid:%d,action:%d,in_way:%d.", \
		oReq.userid, oReq.groupid, (int)oReq.action, (int)oReq.inway);

	unsigned int iRoleType = (oReq.action == 0 ? e_VisitorRole : e_MemberRole);
	return AppDelegate::getInstance()->getServerEvent()->m_logicCore.proc_joingroup_req(oReq.userid, oReq.groupid, iRoleType, false, (int)oReq.inway);
}

int32_t TRoomSvrHandler::proc_blockUserInGroup(const int32_t runid, const int32_t userid, const int32_t groupid, const int32_t reasonid, const int32_t kickout_min)
{
	return AppDelegate::getInstance()->getServerEvent()->m_logicCore.proc_kickoutuser_req(runid, userid, groupid, reasonid, kickout_min);
}

bool TRoomSvrHandler::proc_setGroupOwner(const int32_t groupid, const int32_t ownerid)
{
	return AppDelegate::getInstance()->getServerEvent()->m_logicCore.proc_setGroupOwner(groupid, ownerid);
}

bool TRoomSvrHandler::proc_loadGroup(const int32_t groupid, const std::string& host)
{
	LOG_PRINT(log_info, "load group[%u] host[%s].", groupid, host.c_str());
	if (AppDelegate::getInstance()->getServerEvent()->m_roomMgr.LoadActiveRooms(groupid, true, host) == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool TRoomSvrHandler::proc_modGroup(const int32_t groupid, const std::string& host)
{
	LOG_PRINT(log_info, "modify group[%u] host[%s].", groupid, host.c_str());
	AppDelegate::getInstance()->getServerEvent()->m_roomMgr.LoadModRoom(groupid);
	return true;
}

int32_t TRoomSvrHandler::proc_modGroupSettings(const int32_t runid, const int32_t groupid, const std::map< ::E_THRIFT_GROUP_SETTING::type, std::string> & mSetting)
{
	LOG_PRINT(log_info, "proc_modGroupSettings request: runid[%d], groupid[%d]", runid, groupid);
	int32_t ret = ERR_CODE_SUCCESS;
	
	CDBSink sink;
	do
	{
		if (runid <= 0 || groupid <= 0 || mSetting.empty())
		{
			LOG_PRINT(log_info, "proc_modGroupSettings input invalid: runid[%d] groupid[%d] setting size[%u].", runid, groupid, mSetting.size());
			ret =  ERR_CODE_INVALID_PARAMETER;
			break;
		}

		if (!CUserGroupinfo::chkUserRoleBeyond(runid, groupid, e_AdminRole))
		{
			ret = ERR_CODE_USER_HAS_NO_RIGHT;
			break;
		}

		sink.transBegin();
		CMDGroupSettingStatNotify notify;
		std::map<std::string, std::string> mBasicSetting;
		std::map<std::string, std::string> mPluginSetting;
		bool kickOnlooker = false;
		
		std::map< ::E_THRIFT_GROUP_SETTING::type, std::string>::const_iterator it = mSetting.begin();
		for (;it != mSetting.end(); it++)
		{
			int value = atoi(it->second.c_str());
			switch (it->first)
			{
				case ::E_THRIFT_GROUP_SETTING::VISIT_SWITCH:
					mPluginSetting[CFieldName::ROOM_ALLOW_VISITGROUP] = it->second;
					setSettingStat(*notify.add_list(), e_Setting_JoinGroup, atoi(it->second.c_str()));
					break;
				case ::E_THRIFT_GROUP_SETTING::KICK_ALL_VISITOR:
					kickOnlooker = true;
					setSettingStat(*notify.add_list(), e_Setting_KillVisitor, kickOnlooker);
					break;
				case ::E_THRIFT_GROUP_SETTING::MALE_JOIN_AUTH:
					break;
				case ::E_THRIFT_GROUP_SETTING::MALE_JOIN_REDPACKET_NUM:
					break;
				case ::E_THRIFT_GROUP_SETTING::MALE_JOIN_REDPACKET_AMOUNT:
					break;
				case ::E_THRIFT_GROUP_SETTING::FEMALE_JOIN_AUTH:
					break;
				case ::E_THRIFT_GROUP_SETTING::FEMALE_JOIN_REDPACKET_NUM:
					break;
				case ::E_THRIFT_GROUP_SETTING::FEMALE_JOIN_REDPACKET_AMOUNT:
					break;
				case ::E_THRIFT_GROUP_SETTING::CONTRIBUTION_LIST_SWITCH:
					break;
				case ::E_THRIFT_GROUP_SETTING::VOICE_CHAT_SWITCH:
					break;
				case ::E_THRIFT_GROUP_SETTING::ALLOW_VISITOR_ON_MIC:
					break;
				case ::E_THRIFT_GROUP_SETTING::GAIN_RANGE_TYPE:
					break;
				case ::E_THRIFT_GROUP_SETTING::GAIN_PERCENT:
					break;
				case ::E_THRIFT_GROUP_SETTING::GAIN_SWITCH:
					break;
				case ::E_THRIFT_GROUP_SETTING::GAIN_GROUP:
					{
						int gain_group = atoi(it->second.c_str());
						if (gain_group < 0 && gain_group > 100)
						{
							return ERR_CODE_INVALID_PARAMETER;
						}
						mPluginSetting[CFieldName::ROOM_GAIN_GROUP] = it->second;
					}
					break;
				case ::E_THRIFT_GROUP_SETTING::PRIVATE_CHAT_SWITCH:
					mPluginSetting[CFieldName::ROOM_PRIVATE_CHAT_SWITCH] = it->second;
					setSettingStat(*notify.add_list(), e_Setting_PChat_Switch, atoi(it->second.c_str()));
					break;
				case ::E_THRIFT_GROUP_SETTING::PCHAT_MIN_THRESHOLD_MALE:
					mPluginSetting[CFieldName::ROOM_MALE_PCHAT_MIN_THRESHOLD] = it->second;
					if (value > 0)
						sink.updWhenGroupPChatThresholdMod(groupid, value, true);
					break;
				case ::E_THRIFT_GROUP_SETTING::PCHAT_MIN_THRESHOLD_FEMALE:
					mPluginSetting[CFieldName::ROOM_FEMALE_PCHAT_MIN_THRESHOLD] = it->second;
					if (value > 0)
						sink.updWhenGroupPChatThresholdMod(groupid, value, false);
					break;
				default:
					break;
			}

			if (ERR_CODE_SUCCESS != ret)
				break;
		}

		if (ERR_CODE_SUCCESS != ret)
			break;

		if ((!mBasicSetting.empty() && !sink.updGroupBasicInfo(groupid, mBasicSetting))
			|| (!mPluginSetting.empty() && !sink.updGroupPluginSetting(groupid, mPluginSetting)))
		{
			LOG_PRINT(log_error, "updGroupSetting failed, group[%d]", groupid);
			ret = ERR_CODE_FAILED_DB;
			break;
		}

		if (kickOnlooker)
			AppDelegate::getInstance()->getServerEvent()->m_logicCore.proc_kickAllOnlookers(runid, groupid);

		if (notify.list_size())
		{
			int nProtoLen = notify.ByteSize();
			unsigned int rspDataLen = SIZE_IVM_HEAD_TOTAL + nProtoLen;
			SL_ByteBuffer buff(rspDataLen);
			DEF_IVM_HEADER(pHead, buff.buffer());
			CMsgComm::Build_COM_MSG_HEADER(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_GroupSettingStatNotify, rspDataLen);
			DEF_IVM_CLIENTGATE(pGateMask, buff.buffer());
			CMsgComm::Build_BroadCastRoomGate(pGateMask, e_Notice_AllType, groupid);
			notify.SerializeToArray(buff.buffer() + SIZE_IVM_HEAD_TOTAL, nProtoLen);
			AppDelegate::getInstance()->getServerEvent()->getRoomMgr()->castGateway(pHead);
		}
	} while (0);

	if (ERR_CODE_SUCCESS == ret)
		sink.transCommit();
	else
		sink.transRollBack();

	LOG_PRINT(log_info, "proc_modGroupSettings %s: runid[%d], group[%d].", CErrMsg::strerror(ret).c_str(), runid, groupid);
	return ret;
}

int32_t TRoomSvrHandler::proc_modVisitSwitch(const int32_t runid, const int32_t groupid, const bool Switch, const bool kickOnlooker)
{
	LOG_PRINT(log_info, "proc_modVisitSwitch request: runid[%d], group[%d] Switch[%d] kickOnlooker[%d].", runid, groupid, Switch, kickOnlooker);
	int32_t ret = ERR_CODE_SUCCESS;
	do
	{
		if (runid <= 0 || groupid <= 0)
		{
//			LOG_PRINT(log_info, "modJoinGroupSetting input invalid: runid[%u], groupid[%d] Switch[%d] redpacket[num:%d amount:%d].", runid, groupid, Switch, redPacketNum, redPacketAmount);
			ret = ERR_CODE_INVALID_PARAMETER;
			break;
		}
	
		if (!CUserGroupinfo::chkUserRoleBeyond(runid, groupid, e_AdminRole))
		{
//			LOG_PRINT(log_warning, "group[%d], runid[%d] has no power to operate.", groupid, runid);
			ret = ERR_CODE_USER_HAS_NO_RIGHT;
			break;
		}

		std::map<std::string, std::string> mValues;
		mValues[CFieldName::ROOM_ALLOW_VISITGROUP] = bitTostring(Switch ? 1: 0);
		if (!CGroupBasicInfo::modGroupBasicInfo(groupid, CGroupBasicInfo::e_Field_Alow_Visit_Group, (Switch ? 1: 0)))
		{
			ret = ERR_CODE_FAILED;
			break;
		}

		//notify all member of group
		CMDGroupSettingStatNotify notify;
		notify.set_runid(runid);
		notify.set_groupid(groupid);
		CMDSettingStat *pStat = notify.add_list();
		pStat->set_type(e_Setting_JoinGroup);
		pStat->set_stat(Switch);

		if (!Switch)
		{
			if (kickOnlooker)
				AppDelegate::getInstance()->getServerEvent()->m_logicCore.proc_kickAllOnlookers(runid, groupid);
			pStat = notify.add_list();
			pStat->set_type(e_Setting_KillVisitor);
			pStat->set_stat(kickOnlooker);
		}

		int nProtoLen = notify.ByteSize();
		unsigned int rspDataLen = SIZE_IVM_HEAD_TOTAL + nProtoLen;
		SL_ByteBuffer buff(rspDataLen);
		DEF_IVM_HEADER(pHead, buff.buffer());
		CMsgComm::Build_COM_MSG_HEADER(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_GroupSettingStatNotify, rspDataLen);
		DEF_IVM_CLIENTGATE(pGateMask, buff.buffer());
//		CMsgComm::Build_BroadCastRoomRoleType(pGateMask, groupid, e_MemberRole);
		CMsgComm::Build_BroadCastRoomGate(pGateMask, e_Notice_AllType, groupid);
		notify.SerializeToArray(buff.buffer() + SIZE_IVM_HEAD_TOTAL, nProtoLen);
		AppDelegate::getInstance()->getServerEvent()->getRoomMgr()->castGateway(pHead);
	} while(0);

	LOG_PRINT(log_info, "proc_modVisitSwitch %s: runid[%d], group[%d] Switch[%d] kickOnlooker[%d].", CErrMsg::strerror(ret).c_str(), runid, groupid, Switch, kickOnlooker);
	return ret;
}

int32_t TRoomSvrHandler::proc_modJoinGroupMode(const int32_t runid, const int32_t groupid, const  ::TJoinGroupCondition& maleCondition, const  ::TJoinGroupCondition& femaleCondition)
{
	std::stringstream os;
	os << maleCondition << "||" << femaleCondition;
	LOG_PRINT(log_info, "proc_modJoinGroupMode request: runid[%d], groupid[%u], %s.", runid, groupid, os.str().c_str());
	int32_t ret = ERR_CODE_SUCCESS;
	do
	{
		ServerEvent * pServerEvent = AppDelegate::getInstance()->getServerEvent();
		if (!pServerEvent || runid <= 0 || groupid <= 0 || !checkJoinGroupConditionValid(maleCondition) || !checkJoinGroupConditionValid(femaleCondition))
		{
//			LOG_PRINT(log_info, "modJoinGroupSetting input invalid: runid[%u], groupid[%d] Switch[%d] redpacket[num:%d amount:%d].", runid, groupid, Switch, redPacketNum, redPacketAmount);
			ret = ERR_CODE_INVALID_PARAMETER;
			break;
		}
	
		if (!CUserGroupinfo::chkUserRoleBeyond(runid, groupid, e_AdminRole))
		{
//			LOG_PRINT(log_warning, "group[%d], runid[%d] has no power to operate.", groupid, runid);
			ret = ERR_CODE_USER_HAS_NO_RIGHT;
			break;
		}

		std::map<std::string, std::string> mValues;
		mValues[CFieldName::ROOM_MALE_JOIN_GROUP_AUTH] = bitTostring((int32_t)maleCondition.type);
		mValues[CFieldName::ROOM_FEMALE_JOIN_GROUP_AUTH] = bitTostring((int32_t)femaleCondition.type);
		mValues[CFieldName::ROOM_MALE_REDPACKET_NUM] = bitTostring((int32_t)maleCondition.redPacketNum);
		mValues[CFieldName::ROOM_MALE_REDPACKET_TOTAL] = bitTostring(maleCondition.redPacketAmount);
		mValues[CFieldName::ROOM_FEMALE_REDPACKET_NUM] = bitTostring((int32_t)femaleCondition.redPacketNum);
		mValues[CFieldName::ROOM_FEMALE_REDPACKET_TOTAL] = bitTostring(femaleCondition.redPacketAmount);
		if (!CGroupBasicInfo::modGroupPluginInfo(groupid, mValues))
		{
			ret = ERR_CODE_FAILED;
			break;
		}

		CMDQryJoinGroupCondResp notify;
		notify.mutable_errinfo()->set_errid(0);
		notify.set_groupid(groupid);
		std::string maleCondi = CRoomObj::genJoinGroupCond((int32_t)maleCondition.type, groupid, e_MaleGender, (unsigned int)maleCondition.redPacketAmount, (unsigned int)maleCondition.redPacketNum);
		if (!maleCondi.empty())
		{
			notify.set_action(maleCondi);
			unsigned int notyLen = notify.ByteSize() + SIZE_IVM_HEAD_TOTAL;
			SL_ByteBuffer buff(notyLen);
			buff.data_end(notyLen);

			ClientGateMask_t * pClientGate = (ClientGateMask_t *)CMsgComm::Build_COM_MSG_HEADER(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_QryJoinGroupCondResp, notyLen);
			CMsgComm::Build_BroadCastRoomGender_Gate(pClientGate, groupid, (unsigned int)e_MaleGender);
			notify.SerializeToArray(buff.buffer() + SIZE_IVM_HEAD_TOTAL, notify.ByteSize());
			pServerEvent->getRoomMgr()->castGateway((COM_MSG_HEADER *)buff.buffer());
		}

		std::string famaleCondi = CRoomObj::genJoinGroupCond((int32_t)femaleCondition.type, groupid, e_FemaleGender, (unsigned int)femaleCondition.redPacketAmount, (unsigned int)femaleCondition.redPacketNum);
		if (!famaleCondi.empty())
		{
			notify.set_action(famaleCondi);
			unsigned int notyLen = notify.ByteSize() + SIZE_IVM_HEAD_TOTAL;
			SL_ByteBuffer buff(notyLen);
			buff.data_end(notyLen);

			ClientGateMask_t * pClientGate = (ClientGateMask_t *)CMsgComm::Build_COM_MSG_HEADER(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_QryJoinGroupCondResp, notyLen);
			CMsgComm::Build_BroadCastRoomGender_Gate(pClientGate, groupid, (unsigned int)e_FemaleGender);
			notify.SerializeToArray(buff.buffer() + SIZE_IVM_HEAD_TOTAL, notify.ByteSize());
			pServerEvent->getRoomMgr()->castGateway((COM_MSG_HEADER *)buff.buffer());
		}
	} while(0);

	LOG_PRINT(log_info, "proc_modJoinGroupMode %s: runid[%d], groupid[%d] %s.", CErrMsg::strerror(ret).c_str(), runid, groupid, os.str().c_str());
	return ret;
}

int32_t TRoomSvrHandler::proc_kickAllOnlookers(const int32_t runid, const int32_t groupid)
{
	LOG_PRINT(log_info, "kickAllOnlookers request: runid[%d] groupid[%d].", runid, groupid);
	int32_t ret = ERR_CODE_SUCCESS;
	do
	{
		if (runid <= 0 || groupid <= 0)
		{
			LOG_PRINT(log_info, "kickAllOnlookers input invalid: runid[%d] groupid[%d].", runid, groupid);
			ret =  ERR_CODE_INVALID_PARAMETER;
			break;
		}

		if (!CUserGroupinfo::chkUserRoleBeyond(runid, groupid, e_AdminRole))
		{
			//			LOG_PRINT(log_warning, "group[%d], runid[%d] has no power to operate.", groupid, runid);
			ret = ERR_CODE_USER_HAS_NO_RIGHT;
			break;
		}

		ret = AppDelegate::getInstance()->getServerEvent()->m_logicCore.proc_kickAllOnlookers(runid, groupid);
		//TODO
		
	} while (0);

	LOG_PRINT(log_info, "kickAllOnlookers %s: runid[%d] groupid[%d].", CErrMsg::strerror(ret).c_str(), runid, groupid);
	return ret;
}

int32_t TRoomSvrHandler::proc_modVoiceChatSwitch(const int32_t runid, const int32_t groupid, const bool Switch, const bool visitOnMic)
{
	LOG_PRINT(log_info, "modVoiceChatSwitch request: runid[%u], groupid[%d] Switch[%d] visitOnMic[%d].", runid, groupid, Switch, visitOnMic);
	int ret = ERR_CODE_SUCCESS;

	do
	{
		if (runid <= 0 || groupid <= 0)
		{
//			LOG_PRINT(log_info, "modVoiceChatSwitch input invalid: runid[%u], groupid[%d] Switch[%d].", runid, groupid, Switch);
			ret = ERR_CODE_INVALID_PARAMETER;
			break;
		}
	
		if (!CUserGroupinfo::chkUserRoleBeyond(runid, groupid, e_AdminRole))
		{
//			LOG_PRINT(log_warning, "group[%d], runid[%d] has no power to operate.", groupid, runid);
			ret = ERR_CODE_USER_HAS_NO_RIGHT;
			break;
		}
/*	
		if (!CGroupBasicInfo::isValueChanged((uint32_t)groupid, CGroupBasicInfo::e_Field_Voice_Chat_Switch, Switch ? 1: 0)
		 && !CGroupBasicInfo::isValueChanged((uint32_t)groupid, CGroupBasicInfo::e_Field_Allow_Visitor_On_Mic, Switch ? 1: 0))
			break;
*/
		std::map<std::string, std::string> mValues;
		mValues[CFieldName::ROOM_VOICE_CHAT_SWITCH] = (Switch ? "1": "0");
		mValues[CFieldName::ROOM_ALLOW_VISITOR_ON_MIC] = (visitOnMic ? "1": "0");

		if (!CGroupBasicInfo::modGroupPluginInfo(groupid, mValues))
		{
			ret = ERR_CODE_FAILED;
			break;
		}

		if (!Switch || !visitOnMic)
		{
			RoomObject_Ref roomObj = AppDelegate::getInstance()->getServerEvent()->m_roomMgr.FindRoom(groupid);
			if (roomObj)
			{
				roomObj->downMics(Switch);
			}
		}
		//notify all member of group
		CMDGroupSettingStatNotify notify;
		notify.set_runid(runid);
		notify.set_groupid(groupid);
		CMDSettingStat *pStat = notify.add_list();
		pStat->set_type(e_Setting_VoiceChat);
		pStat->set_stat(Switch);
		if (Switch)
		{
			pStat = notify.add_list();
			pStat->set_type(e_Setting_AllowVisitorOnMic);
			pStat->set_stat((visitOnMic ? 1: 0));
		}
		int nProtoLen = notify.ByteSize();
		unsigned int rspDataLen = SIZE_IVM_HEAD_TOTAL + nProtoLen;
		SL_ByteBuffer buff(rspDataLen);
		DEF_IVM_HEADER(pHead, buff.buffer());
		CMsgComm::Build_COM_MSG_HEADER(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_GroupSettingStatNotify, rspDataLen);
		DEF_IVM_CLIENTGATE(pGateMask, buff.buffer());
		CMsgComm::Build_BroadCastRoomGate(pGateMask, e_Notice_AllType, groupid);
		notify.SerializeToArray(buff.buffer() + SIZE_IVM_HEAD_TOTAL, nProtoLen);
		AppDelegate::getInstance()->getServerEvent()->getRoomMgr()->castGateway(pHead);
	} while(0);

	LOG_PRINT(log_info, "modVoiceChatSwitch %s: runid[%u], groupid[%d] Switch[%d].", CErrMsg::strerror(ret).c_str(), runid, groupid, Switch);
	return ret;
}

int32_t TRoomSvrHandler::proc_modVideoAuthSwitch(const int32_t runid, const int32_t groupid, const bool Switch)
{
	LOG_PRINT(log_info, "modVideoAuthSwitch discarded: runid[%u], groupid[%d] Switch[%d].", runid, groupid, Switch);
/*
	LOG_PRINT(log_info, "modVideoAuthSwitch request: runid[%u], groupid[%d] Switch[%d].", runid, groupid, Switch);
	int32_t ret = ERR_CODE_SUCCESS;
	do
	{
		if (runid <= 0 || groupid <= 0)
		{
			LOG_PRINT(log_info, "modVideoAuthSwitch input invalid: runid[%u], groupid[%d] Switch[%d].", runid, groupid, Switch);
			ret = ERR_CODE_INVALID_PARAMETER;
			break;
		}
	
		if (!CUserGroupinfo::chkUserRoleBeyond(runid, groupid, e_AdminRole))
		{
			LOG_PRINT(log_warning, "group[%d], runid[%d] has no power to operate.", groupid, runid);
			ret = ERR_CODE_USER_HAS_NO_RIGHT;
			break;
		}

		if (!CGroupBasicInfo::isValueChanged((uint32_t)groupid, CGroupBasicInfo::e_Field_Video_Auth_Switch, Switch ? 1: 0))
			break;

		if (!CGroupBasicInfo::modGroupBasicInfo(groupid, CGroupBasicInfo::e_Field_Video_Auth_Switch, Switch ? 1: 0))
		{
			ret = ERR_CODE_FAILED;
			break;
		}
		
		//notify all member of group
		CMDGroupSettingStatNotify notify;
		notify.set_runid(runid);
		notify.set_groupid(groupid);
		CMDSettingStat *pStat = notify.add_list();
		pStat->set_type(e_Setting_VideoAuth);
		pStat->set_stat(Switch);
		int nProtoLen = notify.ByteSize();
		unsigned int rspDataLen = SIZE_IVM_HEAD_TOTAL + nProtoLen;
		SL_ByteBuffer buff(rspDataLen);
		DEF_IVM_HEADER(pHead, buff.buffer());
		CMsgComm::Build_COM_MSG_HEADER(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_GroupSettingStatNotify, rspDataLen);
		DEF_IVM_CLIENTGATE(pGateMask, buff.buffer());
		CMsgComm::Build_BroadCastRoomGate(pGateMask, e_Notice_AllType, groupid);
		notify.SerializeToArray(buff.buffer() + SIZE_IVM_HEAD_TOTAL, nProtoLen);
		AppDelegate::getInstance()->getServerEvent()->getRoomMgr()->castGateway(pHead);
	} while (0);

	LOG_PRINT(log_info, "modVideoAuthSwitch %s: runid[%u], groupid[%d] Switch[%d].", CErrMsg::strerror(ret).c_str(), runid, groupid, Switch);
*/
	return ERR_CODE_FAILED;
}

int32_t TRoomSvrHandler::proc_modGroupUserRole(const int32_t runid, const int32_t groupid, const int32_t userid, const int32_t roleType)
{
	LOG_PRINT(log_info, "modGroupUserRole request: runid[%u], groupid[%d] userid[%d] roleType[%d].", runid, groupid, userid, roleType);
	int32_t ret = ERR_CODE_SUCCESS;
	do
	{
		if (runid <= 0 || groupid <= 0 || userid <= 0)
		{
			LOG_PRINT(log_info, "modGroupUserRole input invalid: runid[%u], groupid[%d] userid[%d] roleType[%d].", runid, groupid, userid, roleType);
			ret = ERR_CODE_INVALID_PARAMETER;
			break;
		}
	
		ret = AppDelegate::getInstance()->getServerEvent()->m_logicCore.proc_setuserpriority_req(runid, userid, groupid, roleType, true);
		if (ret < 0)
		{
			ret = ERR_CODE_FAILED;
			break;
		}
	} while (0);

	LOG_PRINT(log_info, "modGroupUserRole %s: runid[%u], groupid[%d] userid[%d] roleType[%d].", CErrMsg::strerror(ret).c_str(), runid, groupid, userid, roleType);
	return ret;
}

void TRoomSvrHandler::proc_getGroupVisitorLst(std::vector< ::TUserGroupInfo> & _return, const int32_t groupid)
{
	_return.clear();
	LOG_PRINT(log_info, "getGroupVisitorLst request: groupid[%d].", groupid);
	if (groupid <= 0)
	{
		LOG_PRINT(log_error, "getGroupVisitorLst input invalid.groupid[%d]", groupid);
		return;
	}

	AppDelegate::getInstance()->getServerEvent()->m_logicCore.proc_getGroupVisitorLst(groupid, _return);
	return;
}

bool TRoomSvrHandler::checkJoinGroupConditionValid(const TJoinGroupCondition &condition)
{
	if (e_JoinGroupAuthType::RedPacket == condition.type && (condition.redPacketAmount < condition.redPacketNum || condition.redPacketNum < 0))
		return false;

	return true;
}
void TRoomSvrHandler::proc_queryInGroupList(std::map< ::e_INGROUP_TYPE::type, std::vector<int32_t> > & _return, const int32_t userid, const int32_t groupid)
{
	LOG_PRINT(log_info, "proc_queryInGroupList begin: user[%d] group[%d].", userid, groupid);
	int ret = AppDelegate::getInstance()->getServerEvent()->m_logicCore.queryInGroupList(_return, userid, groupid);
	LOG_PRINT(log_info, "proc_queryInGroupList %s: user[%d] group[%d].", CErrMsg::strerror(ret).c_str(), userid, groupid);
}
int32_t TRoomSvrHandler::proc_give_tip(const int32_t srcid, const int32_t toid, const int32_t groupid, const std::string& gifname, const int32_t tiptype,
	const std::string& srcname, const std::string& toname, const std::string& gifpicture)
{
	LOG_PRINT(log_info, "proc_give_tip begin: srcuser[%d] touser[%d] groupid[%d] gifname[%s] type[%d] gifpicture[%s].", srcid, toid, groupid, gifname.c_str(), tiptype, gifpicture.c_str());
	int ret = AppDelegate::getInstance()->getServerEvent()->m_logicCore.procGiveTip(srcid, toid, groupid, gifname, tiptype, srcname, toname, gifpicture);
	LOG_PRINT(log_info, "proc_give_tip end: srcuser[%d] touser[%d] groupid[%d] gifname[%s] type[%d] gifpicture[%s].", srcid, toid, groupid, gifname.c_str(), tiptype, gifpicture.c_str());
	return ret;
}
int32_t TRoomSvrHandler::proc_send_link(const int32_t groupid, const std::string& title, const std::string& source, const std::string& summary, const int32_t bTip,
	const int32_t amount, const int32_t pointId, const std::string& head_add, const int32_t recommendid, const int32_t user_id)
{
	LOG_PRINT(log_info, "procSendLink begin: groupid[%d] title[%s] source[%s] summary[%s] btip[%d] amount[%d] pointId[%d] recommendid[%u] user_id[%u] ", groupid, title.c_str(),
		source.c_str(), summary.c_str(), bTip, amount, pointId, recommendid, user_id);
	int ret = AppDelegate::getInstance()->getServerEvent()->m_logicCore.procSendLink(groupid, title, source, summary, bTip, amount, pointId, head_add, recommendid, user_id);
	LOG_PRINT(log_info, "procSendLink end: groupid[%d] title[%s] source[%s] summary[%s] btip[%d] amount[%d] pointId[%d].", groupid, title.c_str(),
				source.c_str(),summary.c_str(),bTip,amount,pointId);
	return ret;
}
int32_t TRoomSvrHandler::proc_send_notice(const std::string& notice)
{
	LOG_PRINT(log_info, "proc_send_notice begin: notice[%s].", notice.c_str());
	int ret = AppDelegate::getInstance()->getServerEvent()->m_logicCore.procNotice(notice);
	LOG_PRINT(log_info, "proc_send_notice end: notice[%s].", notice.c_str());
	return ret;
}
int32_t TRoomSvrHandler::proc_send_course(const int32_t groupid, const std::string& title, const std::string& source, const std::string& summary, const int32_t bTip,
	const int32_t amount, const int32_t courseId, const std::string& head_add, const std::string& teachername, const std::string& srcimg, const int32_t recommendid, const int32_t user_id)
{
	LOG_PRINT(log_info, "proc_send_course begin: groupid[%d] title[%s] source[%s] summary[%s] btip[%d] amount[%d] courseId[%d] recommendid[%u]  user_id[%u]", groupid, title.c_str(),
		source.c_str(), summary.c_str(), bTip, amount, courseId, recommendid);
	int ret = AppDelegate::getInstance()->getServerEvent()->m_logicCore.procSendCourse(groupid, title, source, summary, bTip, amount, courseId, head_add, teachername, srcimg, recommendid, user_id);
	LOG_PRINT(log_info, "proc_send_course end: groupid[%d] title[%s] source[%s] summary[%s] btip[%d] amount[%d] courseId[%d].", groupid, title.c_str(),
				source.c_str(),summary.c_str(),bTip,amount,courseId);
	return ret;
}
int32_t TRoomSvrHandler::proc_reloadUserInfo(const int32_t userid)
{
	if (!userid)
	{
		LOG_PRINT(log_error, "proc_reloadUserInfo input error.userid:%d.", userid);
		return ERR_CODE_INVALID_PARAMETER;
	}
	LOG_PRINT(log_info, "proc_reloadUserInfo,userid:%u", userid)
	return CUserBasicInfo::loadUserBasic2Redis(userid);
	 
}
int32_t TRoomSvrHandler::proc_reloadRoomVirtualNumInfo(const int32_t liveid, const int32_t numbers)
{
	LOG_PRINT(log_info, "proc_reloadRoomVirtualNumInfo begin: liveid[%u]  numbers[%u] ", liveid, numbers);
	int ret = AppDelegate::getInstance()->getServerEvent()->m_logicCore.proc_reloadRoomVirtualNumInfo(liveid, numbers);
	LOG_PRINT(log_info, "proc_reloadRoomVirtualNumInfo end: notice[%d].", ret);
	return ret;
}

int32_t TRoomSvrHandler::proc_CloseLiveAndCourse(const int32_t runuserid, const int32_t liveid, const int32_t status, const int32_t type)
{
	LOG_PRINT(log_info, "proc_CloseLiveAndCourse begin: runuserid[%u] liveid[%u] status[%u] type[%u] ", runuserid, liveid, status, type);
	int ret = AppDelegate::getInstance()->getServerEvent()->m_logicCore.proc_CloseLiveAndCourse(runuserid, liveid, status, type);
	LOG_PRINT(log_info, "proc_CloseLiveAndCourse end: notice[%d].", ret);
	return ret;
}
//流名:用户uid_课程id
int32_t TRoomSvrHandler::proc_liveStart(const std::string& streamID)
{
	LOG_PRINT(log_info, "proc_liveStart streamID[%s] ", streamID.c_str());
	int ret = AppDelegate::getInstance()->getServerEvent()->m_logicCore.proc_liveStart(streamID);
	return ret;

}
int32_t TRoomSvrHandler:: proc_liveStop(const std::string& streamID)
{
	LOG_PRINT(log_info, "proc_liveStop streamID[%s] ", streamID.c_str());
	int ret = AppDelegate::getInstance()->getServerEvent()->m_logicCore.proc_liveStop(streamID);
	return ret;
}

int32_t TRoomSvrHandler::proc_sendNewSubCourseNotice(const int32_t courseId, const int32_t subCourseId, const int32_t userId, const std::string& courseName, const std::string& subCourseName, const std::string& beginTime)
{
	LOG_PRINT(log_info, "courseId[%d], subCourseId[%d], userId[%d]", courseId, subCourseId, userId);
	int ret = AppDelegate::getInstance()->getServerEvent()->m_logicCore.proc_sendNewSubCourseNotice(courseId, subCourseId, userId, courseName, subCourseName, beginTime);
	return ret;
}

int32_t TRoomSvrHandler::proc_sendNewCourseNotice(const int32_t courseId, const bool isSubCourse)
{
	int courseType = 0;
	uint32 parentCourseId = 0;

	if (CDBSink().queryCourseTypeAndParentId(courseId, courseType, parentCourseId) < 0)
	{
		LOG_PRINT(log_error, "failed to query course type and parent course id, courseId = %d", courseId);
		return -1;
	}
	LOG_PRINT(log_debug, "course[%d]: type = %d, parent course id = %d", courseId, courseType, parentCourseId);

	/*
	 * 注：（1）PHP不一定传isSubCourse，所以需要自行查数据库
	 *   （2）除系列课主课的type是2之外，其余课（包括系列课子课）的type都是1
	 */
	if (/*isSubCourse*/ /*2 == courseType &&*/ parentCourseId > 0)
	{
		const int32_t kUnusedId = 0;
		const std::string kUnusedString("");

		return proc_sendNewSubCourseNotice(parentCourseId, courseId, kUnusedId, kUnusedString, kUnusedString, kUnusedString);
	}

	return AppDelegate::getInstance()->getServerEvent()->m_logicCore.proc_sendNewCourseNotice(courseId);
}
