#include "CUserinfo.h"
#include <time.h>

const char * CUserinfo::LOGIN_TIME = "login_time";
const char * CUserinfo::DEV_TYPE = "devtype";
const char * CUserinfo::GATE_CONNID = "gateconnid";

CUserinfo::CUserinfo():DevType(ePC_type),userid(0),connid(0)
{
    loginTime = time(NULL);
}

CUserinfo::CUserinfo(unsigned int userid, DEVTYPE loginType, time_t loginTime, uint32 connid)
{
    this->userid = userid;
    this->DevType = loginType;
    this->loginTime = loginTime;
	this->connid = connid;
}

CUserinfo::CUserinfo(const CUserinfo & obj)
{
    userid = obj.userid;
    DevType = obj.DevType;
    loginTime = obj.loginTime;
	connid = obj.connid;
}

CUserinfo::~CUserinfo()
{
}

CUserinfo & CUserinfo::operator =(const CUserinfo & obj)
{
    if (this != &obj)
    {
        userid = obj.userid;
        DevType = obj.DevType;
        loginTime = obj.loginTime;
		connid = obj.connid;
    }

    return *this;
}

bool CUserinfo::operator ==(const CUserinfo & obj)const
{
    return (this->userid == obj.userid && this->DevType == obj.DevType && this->loginTime == obj.loginTime && this->connid == obj.connid);
}

bool CUserinfo::operator <(const CUserinfo & obj)const
{
    if(this->userid < obj.userid)
    {
        return true;
    }
	else if(this->userid > obj.userid)
    {
        return false;
    }
	else if (this->DevType < obj.DevType)
    {
        //userid is equal
        return true;
    }
	else if (this->DevType > obj.DevType)
    {
        //userid is equal
        return false;
    }
	else if (this->connid < obj.connid)
	{
		//userid,devtype are equal
		return true;
	}
	else if (this->connid > obj.connid)
	{
		//userid,devtype are equal
		return false;
	}
    else
    {
        //userid,devtype,connid are equal
        return this->loginTime < obj.loginTime;
    }
}

time_t CUserinfo::getLoginTime()const
{
    return loginTime;
}

void CUserinfo::setLoginTime(time_t loginTime)
{
	this->loginTime = loginTime;
}

DEVTYPE CUserinfo::getDevType()const
{
	return DevType;
}

void CUserinfo::setDevType(DEVTYPE loginType)
{
	this->DevType = loginType;
}

unsigned int CUserinfo::getUserid()const
{
	return userid;
}

void CUserinfo::setUserid(unsigned int userid)
{
	this->userid = userid;
}

unsigned int CUserinfo::getConnID() const
{
	return connid;
}

void CUserinfo::setConnID(unsigned int connid)
{
	this->connid = connid;
}

bool CUserinfo::isMobileType()const
{
    return DevType == eAndroid_type || DevType == eIOS_type;
}

bool CUserinfo::sameLoginType(const CUserinfo & obj)
{
    if (this->DevType == obj.DevType)
    {
        return true;
    }
    else
    {
        if (this->isMobileType() && obj.isMobileType())
        {
            return true;
        }
        else
        {
            return false;
        }
    }        
}
