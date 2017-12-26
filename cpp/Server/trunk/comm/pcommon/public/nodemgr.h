#ifndef _NODE_MGR_H
#define _NODE_MGR_H

#include "redisOpt.h"

class CNodeMgr
{
    public:
        CNodeMgr(std::string strIp, int nPort, bool bUse, std::string strRedisIp, int nRedisPort, std::string strRedisPwd);
        ~CNodeMgr();
        
        bool is_use(){return m_bUse;}
        bool register_redis(int nServerType);
        
        void handle_message(const char *pdata, int len);
        virtual void proc_server_add(int servertype, std::string ip, int port);
        virtual void proc_server_stop(int servertype, std::string ip, int port);
    private:
        bool m_bUse;
        std::string m_strBindIp;
        int m_nListenPort;
        std::string m_strRedisIp;
        int m_nRedisPort;
        std::string m_strRedisPwd;
    private:
        redisOpt *m_pRedisMsg;
};

#endif //_NODE_MGR_H
