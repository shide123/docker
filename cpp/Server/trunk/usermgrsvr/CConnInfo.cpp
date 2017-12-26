/*
 * CConnInfo.cpp
 *
 *  Created on: Feb 17, 2016
 *      Author: root
 */

#include "CConnInfo.h"

const char * CConnInfo::GATE_IP = "gateip";
const char * CConnInfo::GATE_PORT = "gateport";
const char * CConnInfo::GATE_ID = "gateid";

CConnInfo::CConnInfo():port(0), ipaddress(""), gateid(0)
{
}

CConnInfo::CConnInfo(const CConnInfo & obj)
{
    port = obj.port;
    ipaddress = obj.ipaddress;
}

CConnInfo::CConnInfo(const std::string & ip, int port, int gateid/* = 0*/)
{
    this->ipaddress = ip;
    this->port = port;
	this->gateid = gateid;
}

CConnInfo::~CConnInfo()
{
}

CConnInfo & CConnInfo::operator =(const CConnInfo & obj)
{
    if (this != &obj)
    {
        port = obj.port;
        ipaddress = obj.ipaddress;
		gateid = obj.gateid;
    }

    return *this;
}

bool CConnInfo::operator ==(const CConnInfo & obj)const
{
    return (gateid == obj.gateid && ipaddress == obj.ipaddress);
}

bool CConnInfo::operator <(const CConnInfo & obj)const
{
    if (ipaddress < obj.ipaddress)
    {
        return true;
    }
	else if (ipaddress > obj.ipaddress)
    {
        return false;
    } 
    else
    {
		//equal ipaddress
		return port < obj.port;
    }
}

std::string CConnInfo::getIpaddress()const
{
    return ipaddress;
}

void CConnInfo::setIpaddress(const std::string & ipaddress)
{
    this->ipaddress = ipaddress;
}

unsigned int CConnInfo::getPort()const
{
    return port;
}

void CConnInfo::setPort(unsigned int port)
{
    this->port = port;
}

int CConnInfo::getGateid() const
{
	return gateid;
}

void CConnInfo::setGateid(int gateid)
{
	this->gateid = gateid;
}
