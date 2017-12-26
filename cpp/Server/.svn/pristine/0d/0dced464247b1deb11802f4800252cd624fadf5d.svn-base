
#ifndef __USER_MANAGER_HH__
#define __USER_MANAGER_HH__

#include "ProtocolsBase.h"

class CUserManager
{
public:
	CUserManager();

	virtual ~CUserManager();

	static int BuildUserTokenPack(unsigned int userid, char * pszBuf, COM_MSG_HEADER * pReqHead, ClientGateMask_t * pReqMask, int nBufSize, std::string & strSessionToken, std::string & strValidTime);

	int GetVisitorId(char *_uuid);

    void ResetVistorInfo(char *_uuid, uint32 userid); 

public:
	uint m_visitor_genid;

};



#endif  //__USER_MANAGER_HH__


