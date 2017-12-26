/*
 * CConnInfo.h
 *
 *  Created on: Feb 17, 2016
 *      Author: root
 */

#ifndef CCONNINFO_H_
#define CCONNINFO_H_
#include <string>

class CConnInfo
{
public:
    CConnInfo();

    CConnInfo(const CConnInfo & obj);

    CConnInfo(const std::string & ip, int port, int gateid = 0);

    ~CConnInfo();

    CConnInfo & operator =(const CConnInfo & obj);

    bool operator ==(const CConnInfo & obj)const;

    bool operator <(const CConnInfo & obj)const;

    std::string getIpaddress()const;
    void setIpaddress(const std::string & ipaddress);

    unsigned int getPort()const;
    void setPort(unsigned int port);

	int getGateid() const ;
	void setGateid(int gateid);

public:
	static const char * GATE_IP;
	static const char * GATE_PORT;
	static const char * GATE_ID;

private:
    unsigned int port;
    std::string ipaddress;
	int gateid;
};

#endif /* CCONNINFO_H_ */
