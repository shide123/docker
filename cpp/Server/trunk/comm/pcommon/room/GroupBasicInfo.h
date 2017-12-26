#ifndef __GROUP_BASIC_INFO_HH__
#define __GROUP_BASIC_INFO_HH__
#include <set>
#include "yc_datatypes.h"
#include "table2.h"
#include "redisMgr.h"
#include "redis_map_map.h"
#include "CFieldName.h"
#include "DBTools.h"
#include "DBSink.h"

class CGroupBasicInfo
{
public:
    CGroupBasicInfo();

	enum e_FieldType
	{
		//from table mc_group
		e_Group_Name = 0,
		e_Field_Unread_Msgnum,
		e_Group_Basic_Type_Max = e_Field_Unread_Msgnum,	//mc_group 枚举 最大值

		//from table mc_group_plugin_setting_detail
		e_Field_Alow_Visit_Group,
		e_Field_Male_Redpacket_Num,
		e_Field_Male_Redpacket_Total,
		e_Field_Contribution_Switch,
		e_Field_Voice_Chat_Switch,
		e_Field_Allow_Visitor_On_Mic,
		e_Field_Gain_Percent,
		e_Field_Gain_Userrange,
		e_Field_Private_Chat_Switch,
		e_Group_Plugin_Type_Max = e_Field_Private_Chat_Switch,//mc_group_plugin_setting_detail 枚举 最大值
	};

	static bool init(redisMgr * pRedisMgr);
	static bool exists(uint32 groupid);
	template<class Type>
	static bool isValueChanged(uint32 groupid, e_FieldType fieldType, Type value);

	template<class Type>
	static bool getValue(uint32 groupid, e_FieldType fieldType, Type &value);
	template<class Type>
	static bool getValue(uint32 groupid, const std::set<e_FieldType > & oSetfieldType, std::map<e_FieldType, Type> & valueMap);
	template<class Type>
	static bool getValueFromDB(uint32 groupid, e_FieldType fieldType, Type &value);

	template<class Type>
	static bool modGroupBasicInfo(uint32 groupid, e_FieldType fieldType, Type value, bool modDB = true);
	static bool modGroupBasicInfo(uint32 groupid, std::map<std::string, std::string> &mValues, bool modDB = true);
	static bool modGroupPluginInfo(uint32 groupid, std::map<std::string, std::string> &mValues, bool modDB = true);

private:
	static void initMap();
	static bool chkFieldType(e_FieldType fieldType);

private:
	static redis_map_map m_group_hashmap;
	static std::map<e_FieldType, const char *> m_mTypeToField;
};


template<class Type>
bool CGroupBasicInfo::isValueChanged(uint32 groupid, e_FieldType fieldType, Type value)
{
	std::string oldValue;
	if (getValue(groupid, fieldType, oldValue) && oldValue == bitTostring(value))
	{
		LOG_PRINT(log_info, "group[%u] field[%s] value[%s], modification is the same as old value", groupid, m_mTypeToField[fieldType], bitTostring(value).c_str());
		return false;
	}

	return true;
}

template<class Type>
bool CGroupBasicInfo::getValue(uint32 groupid, e_FieldType fieldType, Type &value)
{
	if (!chkFieldType(fieldType)) return false;

	std::string strValue;
	if (m_group_hashmap.get(groupid, m_mTypeToField[fieldType], strValue) && !strValue.empty())
	{
		stringTobit(value, strValue);
		return true;
	}

	if (getValueFromDB(groupid, fieldType, value))
	{
		return true;
	}

	return false;
}

template<class Type>
bool CGroupBasicInfo::getValue(uint32 groupid, const std::set<e_FieldType > & oSetfieldType, std::map<e_FieldType, Type> & valueMap)
{
	map<std::string, std::string> field_value_map;
	if (m_group_hashmap.getall(groupid, field_value_map) && !field_value_map.empty())
	{
		std::set<e_FieldType >::const_iterator iter = oSetfieldType.begin();
		for (; iter != oSetfieldType.end(); ++iter)
		{
			if (!chkFieldType(*iter))
				continue;

			std::string fieldName = m_mTypeToField[*iter];
			if (field_value_map.end() != field_value_map.find(fieldName))
			{
				Type value;
				stringTobit(value, field_value_map[fieldName]);
				valueMap[*iter] = value;
			}
			else
			{
				LOG_PRINT(log_warning, "redis has not this fieldName:%s in group:%u.", fieldName.c_str(), groupid);
			}
		}
		return true;
	}
	else
	{
		return false;
	}
}

template<class Type>
bool CGroupBasicInfo::getValueFromDB(uint32 groupid, e_FieldType fieldType, Type &value)
{
	if (!chkFieldType(fieldType)) return false;

	if (fieldType <= e_Group_Basic_Type_Max)
		return CDBSink().getGroupBasicInfo(groupid, m_mTypeToField[fieldType], value);
	else if(fieldType > e_Group_Basic_Type_Max && fieldType <= e_Group_Plugin_Type_Max)
		return CDBSink().getGroupPluginSetting(groupid, m_mTypeToField[fieldType], value);

	return false;
}

template<class Type>
bool CGroupBasicInfo::modGroupBasicInfo(uint32 groupid, e_FieldType fieldType, Type value, bool modDB /*= true*/)
{
	if (!chkFieldType(fieldType)) return false;

	const char *field = m_mTypeToField[fieldType];
	bool ret = true;
	if (modDB)
	{
		if (fieldType <= e_Group_Basic_Type_Max)
			ret = CDBSink().updGroupBasicInfo(groupid, field, value);
		else if (fieldType > e_Group_Basic_Type_Max && fieldType <= e_Group_Plugin_Type_Max)
			ret = CDBSink().updGroupPluginSetting(groupid, field, value);
		else
			ret = false;
	}
	if (ret && m_group_hashmap.exist(groupid))
	{
		ret = m_group_hashmap.insert(groupid, field, value);
	}
	return ret;
}

#endif //__GROUP_BASIC_INFO_HH__
