#ifndef __GATEBASE_H__
#define __GATEBASE_H__

#include <string>
#include "RoomClientMgr.h"
#include "CClientManager.h"
#include "CSvrConnMgr.h"
#include "CCmdConfigMgr.h"
#include "CSvrEvent.h"
#include "CRoomEvent.h"
#include "CUsermgrEvent.h"
#include "Application.h"
#include "libnode/node_client.h"
#include "ConnectionMgr.h"

typedef enum{
	GATE_TYPE_GATEWAY,
	GATE_TYPE_WEBGATE
}GATE_TYPE;

class CGateBase: public Application, public INodeNotify
{
public:
	CGateBase();
	virtual ~CGateBase();
	static unsigned int getAlarmQueueSize();
	static unsigned int getClientTimeout();
	static unsigned int getOnMicClientTimeout();
	static unsigned int getAlarmNotifyInterval();
	static unsigned int getReqRespCheckTime();
	static unsigned short Port();
	static std::string ProcName();
	static void setGateType(GATE_TYPE type);
	static GATE_TYPE getGateType();
	
	static std::string change_type_to_svrname(const unsigned int svr_type);
	static unsigned int change_svrname_to_type(std::string svr_name);
protected:
	void initNode();
	void unInitNode();
	virtual void onNodeEvent(const std::string & svrname, const std::string & ip, 
		int port, en_node_event event, std::string desc = "");

public:
	static CClientManager m_ClientMgr;
	static CRoomClientMgr m_RoomClientMgr;
	static CSvrConnMgr m_SvrConnMgr;
	static CCmdConfigMgr * m_pCmdConfigMgr;
	static ConnectionMgr m_ConnectionMgr;
	static unsigned int m_uServerTimeout;

protected: 
	static unsigned int m_uAlarmQueueSize;
	static unsigned int m_uClientTimeout;
	static unsigned int m_uOnMicClientTimeout;
	static unsigned int m_uAlarmNotifyInterval;

	CSvrEvent	m_svrevent;
	CRoomEvent	m_roomevent;
	CUsermgrEvent m_usermgrevent;

	INodeClient * m_pNodeClient;
	uint16 m_usGateId;

private:
	static GATE_TYPE m_eGateType;
    static std::map<std::string, unsigned int> m_svrname_type;
    static std::map<unsigned int, std::string> m_type_svrname;
};

#endif //__GATEBASE_H__
