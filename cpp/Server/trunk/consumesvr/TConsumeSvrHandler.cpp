#include "TConsumeSvrHandler.h"
#include "CLogThread.h"
#include "GroupBasicInfo.h"
#include "CUserGroupinfo.h"
#include "macro_define.h"
#include "errcode.h"
#include "CFieldName.h"
#include "msgcommapi.h"
#include "ConsumeApp.h"

int32_t TConsumeSvrHandler::proc_modContributionListSwitch(const int32_t runid, const int32_t groupid, const bool Switch)
{
	LOG_PRINT(log_info, "modContributionListSwitch request: runid[%u], groupid[%d] Switch[%d].", runid, groupid, Switch);
	int32_t ret = ERR_CODE_SUCCESS;
	do
	{
		if (runid <= 0 || groupid <= 0)
		{
			LOG_PRINT(log_info, "modContributionListSwitch input invalid: runid[%u], groupid[%d] Switch[%d].", runid, groupid, Switch);
			ret = ERR_CODE_INVALID_PARAMETER;
			break;
		}

		if (!CUserGroupinfo::chkUserRoleBeyond(runid, groupid, e_AdminRole))
		{
			LOG_PRINT(log_warning, "group[%d], runid[%d] has no power to operate.", groupid, runid);
			ret = ERR_CODE_USER_HAS_NO_RIGHT;
			break;
		}
		
		uint32_t value = Switch ? 1: 0;
		if (!CGroupBasicInfo::isValueChanged(groupid, CGroupBasicInfo::e_Field_Contribution_Switch, value))
		{
			break;
		}

		if (!CGroupBasicInfo::modGroupBasicInfo(groupid, CGroupBasicInfo::e_Field_Contribution_Switch, value))
		{
			ret = ERR_CODE_FAILED;
			break;
		}

		//notify all member of group
		CMDGroupSettingStatNotify notify;
		notify.set_runid(runid);
		notify.set_groupid(groupid);
		CMDSettingStat *pStat = notify.add_list();
		pStat->set_type(e_Setting_ContributionList);
		pStat->set_stat(Switch);
		int nProtoLen = notify.ByteSize();
		unsigned int rspDataLen = SIZE_IVM_HEAD_TOTAL + nProtoLen;
		SL_ByteBuffer buff(rspDataLen);
		DEF_IVM_HEADER(pHead, buff.buffer());
		CMsgComm::Build_COM_MSG_HEADER(buff.buffer(), MDM_Vchat_Room, Sub_Vchat_GroupSettingStatNotify, rspDataLen);
		DEF_IVM_CLIENTGATE(pGateMask, buff.buffer());
		CMsgComm::Build_BroadCastRoomGate(pGateMask, e_Notice_AllType, groupid);
		notify.SerializeToArray(buff.buffer() + SIZE_IVM_HEAD_TOTAL, nProtoLen);
		buff.data_end(rspDataLen);
		ConsumeApp::getInstance()->m_room_mgr->castGateway(pHead);
	} while (0);

	LOG_PRINT(log_info, "modContributionListSwitch %s: runid[%u], groupid[%d] Switch[%d].", CErrMsg::strerror(ret).c_str(), runid, groupid, Switch);
	return ret;
}

int32_t TConsumeSvrHandler::proc_modGroupGainSetting(const int32_t runid, const int32_t groupid, const bool Switch, const int8_t rangeType, const int8_t percent)
{
	LOG_PRINT(log_info, "modGroupGainSetting request: runid[%u], groupid[%d] Switch[%d] rangeType[%d] percent[%d].", runid, groupid, Switch, rangeType, percent);
	int32_t ret = ERR_CODE_SUCCESS;
	do
	{
		if (runid <= 0 || groupid <= 0 || rangeType < 0 || percent < 0 || percent > 100)
		{
			LOG_PRINT(log_info, "modGroupGainSetting input invalid: runid[%u], groupid[%d] rangeType[%d] percent[%d].", runid, groupid, rangeType, percent);
			ret = ERR_CODE_INVALID_PARAMETER;
			break;
		}

		if (!CUserGroupinfo::chkUserRoleBeyond(runid, groupid, e_AdminRole))
		{
			LOG_PRINT(log_warning, "group[%d], runid[%d] has no power to operate.", groupid, runid);
			ret = ERR_CODE_USER_HAS_NO_RIGHT;
			break;
		}

		std::map<std::string, std::string> mValues;
		mValues[CFieldName::ROOM_GAIN_SWITCH] = bitTostring((int32_t)Switch);
		mValues[CFieldName::ROOM_GAIN_USERRANGE] = bitTostring((int32_t)rangeType);
		mValues[CFieldName::ROOM_GAIN_PERCENT] = bitTostring((int32_t)percent);
		if (!CGroupBasicInfo::modGroupPluginInfo(groupid, mValues))
		{
			ret = ERR_CODE_FAILED;
			break;
		}
	} while (0);

	LOG_PRINT(log_info, "modGroupGainSetting %s: runid[%u], groupid[%d] Switch[%d] rangeType[%d] percent[%d].", CErrMsg::strerror(ret).c_str(), runid, groupid, Switch, rangeType, percent);
	return ret;
}

int32_t TConsumeSvrHandler::proc_modGroupUserGainSetting(const int32_t runid, const int32_t groupid, const int32_t userid, const int8_t percent)
{
	LOG_PRINT(log_info, "modGroupUserGainSetting request: runid[%u], groupid[%d] userid[%d] percent[%d].", runid, groupid, userid, percent);
	int32_t ret = ERR_CODE_SUCCESS;
	do
	{
		if (runid <= 0 || groupid <= 0 || userid <= 0 || percent < -1 || percent > 100)
		{
			LOG_PRINT(log_info, "modGroupUserGainSetting input invalid: runid[%u], groupid[%d] userid[%d] percent[%d].", runid, groupid, userid, percent);
			ret = ERR_CODE_INVALID_PARAMETER;
			break;
		}

		if (!CUserGroupinfo::chkUserRoleBeyond(runid, groupid, e_AdminRole))
		{
			LOG_PRINT(log_warning, "group[%d], runid[%d] has no power to operate.", groupid, runid);
			ret = ERR_CODE_USER_HAS_NO_RIGHT;
			break;
		}

		if (!CUserGroupinfo::isValueChanged(userid, groupid, CFieldName::USER_GAIN_PERCENT, (int32_t)percent))
		{
			break;
		}


		if (!CUserGroupinfo::modUserGroupInfo(userid, groupid, CFieldName::USER_GAIN_PERCENT, (int32_t)percent))
		{
			ret = ERR_CODE_FAILED;
			break;
		}
	} while (0);

	LOG_PRINT(log_info, "modGroupUserGainSetting %s: runid[%u], groupid[%d] userid[%d] percent[%d].", CErrMsg::strerror(ret).c_str(), runid, groupid, userid, percent);
	return ret;
}
