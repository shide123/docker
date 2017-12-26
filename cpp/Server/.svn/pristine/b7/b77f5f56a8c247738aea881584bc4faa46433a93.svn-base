
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include <iostream>
#include <fstream>
#include <string>

#include "SL_Socket_CommonAPI.h"
#include "AndroidUserManager.h"
#include "CLogThread.h"
#include "utils.h"
#include "RoomManager.h"

//#include "TaskNetMsgProc.h"


CAndroidUserManager::CAndroidUserManager()
{
   m_nLastTimeCheckFile=0;
   m_nLastTimeCheckAndroidInRoom=0;
}

CAndroidUserManager::~CAndroidUserManager()
{

}
void CAndroidUserManager::SetConfigFile(const char* lpszFileName)
{
   m_strConfigFileName =lpszFileName;
}

void CAndroidUserManager::LoadConfig()
{
	char szTemp[256]={0};
	time_t tnow=time(0);
	if(tnow - m_nLastTimeCheckFile > 10)  //10s间隔
	{
		m_nLastTimeCheckFile = tnow;
		std::fstream file(m_strConfigFileName.c_str(), std::ios_base::in);
		if(file)
		{
			file.getline(szTemp,255);
			file.close();
			if(strcmp(m_strFileMask.c_str(), szTemp)!=0)
			{
				m_strFileMask =szTemp;
				_ReadConfig();
			}
		}
		else {
			LOG_PRINT(log_error, "cannot load robot config file");
		}
	}
}

void CAndroidUserManager::setRoomMgr(CRoomManager* pRoomMgr)
{
	m_pRoomMgr = pRoomMgr;
}

void CAndroidUserManager::_ReadConfig()
{
	LOG_PRINT(log_info, "read robot config file...");
	const int LINE_LENGTH = 1024;
	char szTemp[LINE_LENGTH];

	bool bIsNewRoom;
	AndroidParam_t androidParam;
	int nLoadRoomCount=0, nLoadAndroidCount=0;

	std::ifstream fin(m_strConfigFileName.c_str());
	while(fin.getline(szTemp, LINE_LENGTH))
	{
		if(szTemp[0]=='/'||szTemp[0]=='#')
			continue;
		std::string strTemp;
		std::vector<std::string> vecStrings;
		SplitStringA(szTemp,',',vecStrings);
		if(vecStrings.size()==4) //4个参数
		{
			//room param
			//r,roomId,useandroid,androidnum
			strTemp = StringTrimA(vecStrings[0]);
			if(strcmp(strTemp.c_str(), "r") ==0)
			{
				int vcbId=atoi(vecStrings[1].c_str());
				int usedFlag=atoi(vecStrings[2].c_str());
				int androidNum=atoi(vecStrings[3].c_str());
				RoomAndroidParam_t* pRoomParam = FindRoomAndroid(vcbId);
				if(pRoomParam ==0) {
					bIsNewRoom =true;
					pRoomParam = new RoomAndroidParam_t;
				}
				pRoomParam->nVcbId =vcbId;
				pRoomParam->nUseAndroid = usedFlag;
				pRoomParam->nAndroidLimit = androidNum;

				if(bIsNewRoom) {
					nLoadRoomCount++;
					m_mapRoomAndroidParams_mutex.lock();
					m_mapRoomAndroidParams.insert(std::make_pair(vcbId, pRoomParam));
					m_mapRoomAndroidParams_mutex.unlock();
				}
			}
		}
		else if(vecStrings.size()==10) //10个参数
		{
			//android param
			//u,roomId,userId,username,gender,viplevel,roomlevel,carid,carname,lifetime(mins)
			strTemp = StringTrimA(vecStrings[0]);
			if(strcmp(strTemp.c_str(), "u") == 0)
			{
				int vcbId = atoi(vecStrings[1].c_str());
				androidParam.vcbid =vcbId;
				androidParam.userid = atoi(vecStrings[2].c_str());
				if(androidParam.userid >0)
				{
					strcpy(androidParam.szuseralias, vecStrings[3].c_str());
					androidParam.ngender=atoi(vecStrings[4].c_str());
					androidParam.viplevel =atoi(vecStrings[5].c_str());
					androidParam.roomlevel=atoi(vecStrings[6].c_str());
					androidParam.carid = atoi(vecStrings[7].c_str());
					strcpy(androidParam.szcarname, vecStrings[8].c_str());
					androidParam.lifetime = atoi(vecStrings[9].c_str());

					nLoadAndroidCount++;
					AddAndroidParam(vcbId, androidParam);
				}
			}
		}
	}
	fin.close();
	LOG_PRINT(log_debug, "read room number [%d], read robot number[%d]", nLoadRoomCount, nLoadAndroidCount);
}

RoomAndroidParam_t* CAndroidUserManager::FindRoomAndroid(int vcbId)
{
	RoomAndroidParam_t* pRoomParam=0;
	m_mapRoomAndroidParams_mutex.lock();

	pRoomParam=FindRoomAndroid_i(vcbId);
	
	m_mapRoomAndroidParams_mutex.unlock();
	return pRoomParam;
}

RoomAndroidParam_t* CAndroidUserManager::FindRoomAndroid_i(int vcbId)
{
	RoomAndroidParam_t* pRoomParam=0;
	ROOMANDROID_MAP::iterator iter = m_mapRoomAndroidParams.find(vcbId);
	if(iter !=m_mapRoomAndroidParams.end())
	{
		pRoomParam = iter->second;
	}
    return pRoomParam;
}

void CAndroidUserManager::CheckAndroidInRoom()
{
	time_t tnow=time(0);
	if(tnow - m_nLastTimeCheckAndroidInRoom <5) //intval time 5s
       return;
	m_nLastTimeCheckAndroidInRoom = tnow;

	//CLogThread::Instance()->print(SL_Log::LOG_LEVEL_3, "检查机器人进房间...");

	//遍历每个房间，每次增加一个机器人。防止死锁,采用先取出再放入的方式

	RoomAndroidParam_t* pRoomParam=0;
	ROOMANDROID_MAP::iterator iter;
	ANDROIDPARAM_LIST tmp_lstAndroidParams; //临时缓存

	m_mapRoomAndroidParams_mutex.lock();
	for(iter= m_mapRoomAndroidParams.begin(); iter!=m_mapRoomAndroidParams.end(); iter++)
	{
		pRoomParam = iter->second;
		if(m_pRoomMgr->IsRoomExist(pRoomParam->nVcbId) && pRoomParam->nUseAndroid && pRoomParam->lstAndroidParams.size()>0)
		{
			AndroidParam_t androidParam=pRoomParam->lstAndroidParams.front();
			pRoomParam->lstAndroidParams.pop_front();
			androidParam.temp_maxaddnum = pRoomParam->nAndroidLimit;
			tmp_lstAndroidParams.push_back(androidParam);  //临时缓存
		}
	}
	m_mapRoomAndroidParams_mutex.unlock();

	//CLogThread::Instance()->print(SL_Log::LOG_LEVEL_3, "将把 [%d]个 机器人加入到房间中...",tmp_lstAndroidParams.size());
	//加入到房间中
	if(tmp_lstAndroidParams.size()>0)
	{
		ANDROIDPARAM_LIST::iterator iter2;
		for(iter2 =tmp_lstAndroidParams.begin(); iter2 !=tmp_lstAndroidParams.end(); iter2++)
		{
			AndroidParam_t& param=(*iter2);
			//模拟消息，扔到工作线程中去处理？ 消息类型:TASK_TYPE_LOADA4USER, 参数 wparam:0,lparam:0,content:AndroidParam_t
		}
		tmp_lstAndroidParams.clear();
	}
}

void CAndroidUserManager::AddAndroidParam(int vcbId, AndroidParam_t& androidParam)
{
	m_mapRoomAndroidParams_mutex.lock();

	RoomAndroidParam_t* pRoomParam = FindRoomAndroid_i(vcbId);
	bool bfound=false;
	ANDROIDPARAM_LIST::iterator iter;
	for(iter = pRoomParam->lstAndroidParams.begin(); iter != pRoomParam->lstAndroidParams.end(); iter++)
	{
		if((*iter).userid == androidParam.userid) {
			bfound = true;
			break;
		}
	}
	if(!bfound)
		pRoomParam->lstAndroidParams.push_back(androidParam);

	m_mapRoomAndroidParams_mutex.unlock();
}

void CAndroidUserManager::Clear()
{
	RoomAndroidParam_t* pRoomParam=0;
	m_mapRoomAndroidParams_mutex.lock();

	ROOMANDROID_MAP::iterator iter;
	for(iter= m_mapRoomAndroidParams.begin(); iter!=m_mapRoomAndroidParams.end(); iter++)
	{
		pRoomParam = iter->second;
		pRoomParam->lstAndroidParams.clear();
		delete pRoomParam;
	}
	m_mapRoomAndroidParams.clear();

	m_mapRoomAndroidParams_mutex.unlock();
}



