#include "GroupBasicInfo.h"
#include "CLogThread.h"
#include "errcode.h"
#include "message_vchat.h"
#include "macro_define.h"
#include "DBTools.h"

redis_map_map CGroupBasicInfo::m_group_hashmap;
std::map<CGroupBasicInfo::e_FieldType, const char *> CGroupBasicInfo::m_mTypeToField;

CGroupBasicInfo::CGroupBasicInfo()
{
}

bool CGroupBasicInfo::init(redisMgr * pRedisMgr)
{
	if (!pRedisMgr)
	{
		LOG_PRINT(log_error, "redisMgr is null.init failed.");
		return false;
	}
	
	m_group_hashmap.init(DB_ROOM, KEY_HASH_ROOM_INFO":", pRedisMgr);

	initMap();
	return true;
}

void CGroupBasicInfo::initMap()
{
	m_mTypeToField[e_Field_Unread_Msgnum]		= CFieldName::ROOM_UNREAD_MSGNUM.c_str();
	m_mTypeToField[e_Field_Alow_Visit_Group]	= CFieldName::ROOM_ALLOW_VISITGROUP.c_str();
	m_mTypeToField[e_Field_Male_Redpacket_Num]	= CFieldName::ROOM_MALE_REDPACKET_NUM.c_str();
	m_mTypeToField[e_Field_Male_Redpacket_Total]= CFieldName::ROOM_MALE_REDPACKET_TOTAL.c_str();
	m_mTypeToField[e_Field_Contribution_Switch]	= CFieldName::ROOM_CONTRIBUTION_SWITCH.c_str();
	m_mTypeToField[e_Field_Voice_Chat_Switch]	= CFieldName::ROOM_VOICE_CHAT_SWITCH.c_str();
	m_mTypeToField[e_Field_Allow_Visitor_On_Mic]= CFieldName::ROOM_ALLOW_VISITOR_ON_MIC.c_str();
	m_mTypeToField[e_Field_Gain_Percent]		= CFieldName::ROOM_GAIN_PERCENT.c_str();
	m_mTypeToField[e_Field_Gain_Userrange]		= CFieldName::ROOM_GAIN_USERRANGE.c_str();
	m_mTypeToField[e_Group_Name]				= CFieldName::ROOM_ROOMNAME.c_str();
	m_mTypeToField[e_Field_Private_Chat_Switch]	= CFieldName::ROOM_PRIVATE_CHAT_SWITCH.c_str();
}

bool CGroupBasicInfo::exists(uint32 groupid)
{
	return m_group_hashmap.exist(groupid);
}

bool CGroupBasicInfo::modGroupBasicInfo(uint32 groupid, std::map<std::string, std::string> &mValues, bool modDB/* = true*/)
{
	bool ret = false;

	if (modDB)
	{
		ret = CDBSink().updGroupBasicInfo(groupid, mValues);
	}

	if (ret && m_group_hashmap.exist(groupid))
	{
		ret = m_group_hashmap.insert(groupid, mValues);
	}

	return ret;
}

bool CGroupBasicInfo::modGroupPluginInfo(uint32 groupid, std::map<std::string, std::string> &mValues, bool modDB/* = true*/)
{
	bool ret = false;

	if (modDB)
	{
		ret = CDBSink().updGroupPluginSetting(groupid, mValues);
	}

	if (ret && m_group_hashmap.exist(groupid))
	{
		ret = m_group_hashmap.insert(groupid, mValues);
	}

	return ret;
}

bool CGroupBasicInfo::chkFieldType(e_FieldType fieldType)
{
	if (m_mTypeToField.find(fieldType) == m_mTypeToField.end())
	{
		LOG_PRINT(log_error, "type(%d) is not found, check that m_mTypeToField has been initialized.", fieldType);
		return false;
	}

	return true;
}
