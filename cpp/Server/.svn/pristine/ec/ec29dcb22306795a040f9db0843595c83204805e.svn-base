#ifndef __USERINFO_HH__
#define __USERINFO_HH__
#include <time.h>
#include "yc_datatypes.h"

typedef enum
{
    ePC_type = 0,
    eAndroid_type = 1,
    eIOS_type = 2,
    eWeb_type = 3
}DEVTYPE;

class CUserinfo
{
public:
    CUserinfo();

    CUserinfo(const CUserinfo & obj);

	CUserinfo(unsigned int userid, DEVTYPE loginType, time_t loginTime, uint32 connid);

	~CUserinfo();

    bool operator ==(const CUserinfo & obj)const;

    CUserinfo & operator =(const CUserinfo & obj);

    bool operator <(const CUserinfo & obj)const;

    time_t getLoginTime()const;

	void setLoginTime(time_t loginTime);

	DEVTYPE getDevType()const;

	void setDevType(DEVTYPE loginType);

	unsigned int getUserid()const;

	void setUserid(unsigned int userid);

    bool sameLoginType(const CUserinfo & obj);

    bool isMobileType()const;

	unsigned int getConnID()const;

	void setConnID(unsigned int connid);

public:
	static const char * LOGIN_TIME;
	static const char * DEV_TYPE;
	static const char * GATE_CONNID;

private:
	uint32 connid;

    time_t loginTime;
    DEVTYPE DevType;
    unsigned int userid;
};


#endif //__USERINFO_HH__
