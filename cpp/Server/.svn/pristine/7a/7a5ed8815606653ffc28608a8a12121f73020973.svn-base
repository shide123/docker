#ifndef _BRIDGE_MGR_H_
#define _BRIDGE_MGR_H_
#include <set>
#include "CLogThread.h"

template<typename value_type> 
class bridgeMgr
{
public:
    bridgeMgr()
    :m_index(0)
    {
    }
    
    ~bridgeMgr()
    {
    }
    
    void insert(const value_type &val)
    {
        boost::mutex::scoped_lock lock(m_bridgeConnMap_mutex);
        m_bridgeConnMap.insert(val);       
    }
    
    void erase(const value_type &val)
    {
        boost::mutex::scoped_lock lock(m_bridgeConnMap_mutex);
        m_bridgeConnMap.erase(val);          
    }
    
    value_type getone()
    {
        boost::mutex::scoped_lock lock(m_bridgeConnMap_mutex);
        if (m_bridgeConnMap.empty()) return NULL;	        	
    	size_t i = m_index++ % m_bridgeConnMap.size();
    	typename set<value_type>::iterator iter = m_bridgeConnMap.begin();
    	for (int index = 0; index < i; ++index)
    	    ++iter;
    	return *iter;
    }

    
    bool send(const char *pPkt, int len, int subcmd)
    {
        if (pPkt == NULL || len < 0) return false;
        LOG_PRINT(log_info, "send %d pkt len=%d to bridgeSvr!", subcmd, len);    
        COM_MSG_HEADER * pOutMsg = (COM_MSG_HEADER*)pPkt;
        pOutMsg->version = MDM_Version_Value;
        pOutMsg->checkcode = 0;
        pOutMsg->maincmd = MDM_Vchat_Bridge;
        pOutMsg->subcmd= subcmd;
        pOutMsg->length = len;    
        return send(pPkt, len);
    }
    
    template<typename pkt_type>
    bool send(pkt_type &pPkt, int subcmd)
    {    
        char buf[sizeof(pkt_type) + sizeof(COM_MSG_HEADER)];    
        COM_MSG_HEADER * pOutMsg = (COM_MSG_HEADER*)buf;
        pOutMsg->version = MDM_Version_Value;
        pOutMsg->checkcode = 0;
        pOutMsg->maincmd = MDM_Vchat_Bridge;
        pOutMsg->subcmd= subcmd;
        pOutMsg->length = sizeof(pkt_type) + sizeof(COM_MSG_HEADER);
        memcpy(pOutMsg->content, &pPkt, sizeof(pkt_type) ); 
        LOG_PRINT(log_info, "send %d pkt len=%d to bridgeSvr!", subcmd, pOutMsg->length);   
        return send((const char *)buf, pOutMsg->length);
    }
    
    bool send(const char * pPkt, int len)
    {
        if (pPkt == NULL || len < 0) return false;
        
        boost::mutex::scoped_lock lock(m_bridgeConnMap_mutex);
        if (m_bridgeConnMap.empty()) return false;	        	
    	size_t i = m_index++ % m_bridgeConnMap.size();
    	typename set<value_type>::iterator iter = m_bridgeConnMap.begin();
    	for (unsigned int index = 0; index < i; ++index)
    	    ++iter;               
        if (*iter == NULL) return false;
        (*iter)->write_message(pPkt, len);
		LOG_PRINT(log_info, "send bridgeSvr success!");
		return true;
    }
    
    bool send(char * pPkt, int len)
    {
		return send((const char * )pPkt, len);
    }

    int count(const value_type& val)
    {
        return m_bridgeConnMap.count(val);
    }  
private:
    set<value_type> m_bridgeConnMap;
    boost::mutex m_bridgeConnMap_mutex;
    unsigned int m_index;
};

#endif // _BRIDGE_MGR_H_
