#ifndef __TCONSUMESVR_HANDLER_H__
#define __TCONSUMESVR_HANDLER_H__

#include "consumesvr/TConsumeSvr.h"

class TConsumeSvrHandler: public TConsumeSvrIf
{
public:
	TConsumeSvrHandler(){}
	~TConsumeSvrHandler(){}

	virtual int32_t proc_modContributionListSwitch(const int32_t runid, const int32_t groupid, const bool Switch);                         
	virtual int32_t proc_modGroupGainSetting(const int32_t runid, const int32_t groupid, const bool Switch, const int8_t rangeType, const int8_t percent);
	virtual int32_t proc_modGroupUserGainSetting(const int32_t runid, const int32_t groupid, const int32_t userid, const int8_t percent);
};


#endif //__TCONSUMESVR_HANDLER_H__
