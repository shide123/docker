#ifndef __ANDROID_USER_MANAGER__HH__
#define __ANDROID_USER_MANAGER__HH__

#include <map>
#include <list>
#include "yc_datatypes.h"
#include "SL_Sync_Mutex.h"
#include "message_vchat.h"

typedef std::list<AndroidParam_t> ANDROIDPARAM_LIST;

typedef struct _tag_RoomAndroidParam
{
	int nVcbId;
	int nUseAndroid;
	int nAndroidLimit;  //最大数目
	ANDROIDPARAM_LIST lstAndroidParams;
}RoomAndroidParam_t;

typedef std::map<int, RoomAndroidParam_t*> ROOMANDROID_MAP;


class CRoomManager;
//////////////////////////////////////////////////////////////////////////
class CAndroidUserManager
{
public:
	CAndroidUserManager();
	~CAndroidUserManager();

public:
	void SetConfigFile(const char* lpszFileName);
	void LoadConfig(void);
	void setRoomMgr(CRoomManager* pRoomMgr);

	void CheckAndroidInRoom(void);
	void AddAndroidParam(int vcbId, AndroidParam_t& androidParam);

	void Clear();

private:
	void _ReadConfig();
	RoomAndroidParam_t* FindRoomAndroid(int vcbId);
	RoomAndroidParam_t* FindRoomAndroid_i(int vcbId);


private:
	SL_Sync_Mutex m_mapRoomAndroidParams_mutex;
	ROOMANDROID_MAP m_mapRoomAndroidParams;

	std::string m_strConfigFileName;
	std::string m_strFileMask;
	unsigned int m_nLastTimeCheckFile;
	unsigned int m_nLastTimeCheckAndroidInRoom;
	CRoomManager *m_pRoomMgr;

};

#endif //__ANDROID_USER_MANAGER__HH__

