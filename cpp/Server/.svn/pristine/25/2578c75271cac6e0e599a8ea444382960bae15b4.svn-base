#include "nodemgr.h"

CNodeMgr::CNodeMgr(std::string strIp, int nPort, bool bUse, std::string strRedisIp, int nRedisPort, std::string strRedisPwd)
    :m_strBindIp(strIp),
    m_nListenPort(nPort),
    m_bUse(bUse),
    m_strRedisIp(strRedisIp),
    m_nRedisPort(nRedisPort),
    m_strRedisPwd(strRedisPwd)
{
    if(m_bUse)
    {
        m_pRedisMsg = new redisOpt(m_strRedisIp.c_str(), m_nRedisPort, m_strRedisPwd.c_str());    
    }
}

CNodeMgr::~CNodeMgr()
{
    if(m_pRedisMsg)
    {
        delete m_pRedisMsg;
        m_pRedisMsg = NULL;
    }
}

bool CNodeMgr::register_redis(int nServerType)
{
    if(!m_bUse)
    {
        return false;
    }

    char szKey[64] = "";
    snprintf(szKey, 64, "%d:%s:%d", nServerType, m_strBindIp.c_str(), m_nListenPort);
    
    if(m_pRedisMsg->redis_UpdateServerInfo(szKey, 0) < 0)
    {
        return false;
    }
    if(m_pRedisMsg->redis_DelServerConnList(szKey) < 0)
    {
        return false;
    }
    if(m_pRedisMsg->redis_RegisterServer(szKey) < 0)
    {
        return false;
    }

    return true;
}

void CNodeMgr::handle_message(const char *pdata, int len)
{
    if(!m_bUse)
    {
        return;
    }

    if(len < sizeof(COM_MSG_HEADER)+sizeof(CMDNoticeServerInfo))
    {
        LOG_PRINT(log_error, "packet length is wrong.length:%d", len);
        return;
    }
    COM_MSG_HEADER * pHeader = (COM_MSG_HEADER *)pdata;
    CMDNoticeServerInfo * info = (CMDNoticeServerInfo *)pHeader->content;
    switch(pHeader->subcmd)
    {
        case Sub_Vchat_NoticeServerAdd:
            proc_server_add(info->servertype, info->szIp, info->port);
        break;
        case Sub_Vchat_NoticeServerStop:
            proc_server_stop(info->servertype, info->szIp, info->port);
        break;
    }
}

void CNodeMgr::proc_server_add(int servertype, std::string ip, int port)
{

}

void CNodeMgr::proc_server_stop(int servertype, std::string ip, int port)
{

}
