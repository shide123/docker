
#ifndef __GLOBAL_SETTING_HH__
#define __GLOBAL_SETTING_HH__

#include <pthread.h>
#include "LogonServerApplication.h"
#include "CLogThread.h"
#include "CAlarmNotify.h"

class CGlobalSetting
{
public:
	CGlobalSetting();
	virtual ~CGlobalSetting();

public:
	static LogonServerApplication * m_app;
};

#endif //__GLOBAL_SETTING_HH__

