#ifndef __LINK_LIST_H__
#define __LINK_LIST_H__

#include "CLogThread.h"
#include "Config.h"
#include "utils.h"
#include "cmd_vchat.h"
#include <string>
#include <list>
#include <set>
#include <vector>

using namespace std;

struct SimEndPoint
{
    SimEndPoint():port(0)
    {
    }
    SimEndPoint(const string &iip, int iport)
            :ip(iip)
            ,port(iport)
    {
    }
    string ip;
    int port;
};

class LinkList
{
public:
    LinkList()
    {
        m_EndPointList_iter = m_EndPointList.end();
    }
    
    bool Init(Config &cfg, const char* Main = "GoldTrade")
	{
        string connlist = cfg.getString(Main, "connlist"); 
        LOG_PRINT(log_info, "connlist:%s", connlist.c_str());
        return push_back_all(connlist, ',', ":");
	}
	
    bool push_back_all(const string & connlist, char mainsplit, const string & subsplit)
    {
        if (connlist.empty()) {
            LOG_PRINT(log_error, "connlist is empty");
            return false;
        }
        vector<string> vecconnlist = strToVec(connlist, mainsplit);
        if (vecconnlist.empty()) {
            LOG_PRINT(log_error, "connlist is empty");
            return false;
        }
        for (int index = 0; index < vecconnlist.size(); ++index)
        {
            size_t pos = vecconnlist[index].find(subsplit);
            if (pos == string::npos) {
                LOG_PRINT(log_error, "conn format is incorrect:%s", vecconnlist[index].c_str());
                return false;
            }           
            string ip(vecconnlist[index], 0, pos);
            string port(vecconnlist[index], pos + 1); 
            push_back(ip, port);      
        }
        return true;
    }
    
    void push_back(const string & ip, const string & port)
    {
        if (ip.empty() || port.empty()) return;
        m_EndPointList.push_back(SimEndPoint(ip, atoi(port.c_str())));
        if (m_EndPointList.size() == 1) m_EndPointList_iter = m_EndPointList.begin();  
    }    

    void push_back(const string & ip, int port)
    {
        if (ip.empty() || port == 0) return;
        m_EndPointList.push_back(SimEndPoint(ip, port));
        if (m_EndPointList.size() == 1) m_EndPointList_iter = m_EndPointList.begin();  
    }
    
    const char * ip()
    {
        if (m_EndPointList_iter != m_EndPointList.end())                
            return m_EndPointList_iter->ip.c_str();
        else
            return NULL;
    }
    
    int port()
    {
        if (m_EndPointList_iter != m_EndPointList.end())                
            return m_EndPointList_iter->port;
        else
            return NULL;
    }
    
    LinkList& next()
    {
        if (m_EndPointList_iter != m_EndPointList.end())  
        {
            ++m_EndPointList_iter;                
        }
        
        if (m_EndPointList_iter == m_EndPointList.end())  
        {              
            if (m_EndPointList.empty()) return *this;
            m_EndPointList_iter = m_EndPointList.begin();  
        }
        return *this;
    }
    
    void reset()
    {
        m_EndPointList_iter = m_EndPointList.end();
    }
    list<SimEndPoint> m_EndPointList;
    list<SimEndPoint>::iterator m_EndPointList_iter;
};

enum
{
    E_GATEWAY = 0,
};

#define Hello_Param_Size 4

//map的比较函数
struct cmp_CMDGateHello_t
{
    bool operator()(const CMDGateHello_t &k1, const CMDGateHello_t &k2)const
    {
        if (k1.param1 != k2.param1)
        {
            return k1.param1 < k2.param1;
        } 
        if (k1.param2 != k2.param2)
        {
            return k1.param2 < k2.param2;
        }        
        if (k1.param3 != k2.param3)
        {
            return k1.param3 < k2.param3;
        }
        if (k1.param4 != k2.param4)
        {
            return k1.param4 < k2.param4;
        }
        return false;
    }
};

template<typename LINK_TYPE>
class LinkPool
{

public:    

    LinkPool(size_t n = 0)
        : m_LinkPool(n)
        , m_MaxLinkType(0)
    {
    }
    
    bool Init(Config &cfg, const char* Main = "LinkPool")
	{        
        string LinkTypeList = cfg.getString(Main, "LinkTypeList"); 
        if (LinkTypeList.empty()) {
            LOG_PRINT(log_error, "LinkTypeList is empty");
            return false;
        }
        LOG_PRINT(log_info, "LinkTypeList:%s", LinkTypeList.c_str());
        vector<string> veclist = strToVec(LinkTypeList, ";");
        if (veclist.empty()) {
            LOG_PRINT(log_error, "LinkTypeList is empty");
            return false;
        }
        for (int index = 0; index < veclist.size(); ++index)
        {
            size_t pos = veclist[index].find(":");
            if (pos == string::npos) {
                LOG_PRINT(log_error, "format is incorrect:%s", veclist[index].c_str());
                return false;
            }           
            string type(veclist[index], 0, pos);
            string helloparam(veclist[index], pos + 1);           
            vector<string> vechello = strToVec(helloparam, ".");
            if (vechello.size() != Hello_Param_Size) {
                LOG_PRINT(log_error, "format is incorrect:%s", veclist[index].c_str());
                return false;
            }
            int ntype = atoi(type.c_str());
            CMDGateHello_t Hello;
            memset(&Hello, 0 ,sizeof(CMDGateHello_t));
            Hello.param1 = atoi(vechello[0].c_str());
            Hello.param2 = atoi(vechello[1].c_str());
            Hello.param3 = atoi(vechello[2].c_str());
            Hello.param4 = atoi(vechello[3].c_str());        
            m_LinkType[Hello] = ntype; 
            m_MaxLinkType = m_MaxLinkType > ntype ? m_MaxLinkType : ntype; 
        }
        
        Init(m_MaxLinkType);
        return true;

	}
    
    void Init(size_t n)
    {
        m_LinkPool.reserve(n);
    }
    
    void insert(LINK_TYPE link, const char * data, size_t len)
    {
        CMDGateHello_t* pHelloMsg = (CMDGateHello_t *)data;
        if (!data || len < sizeof(pHelloMsg->param1) + sizeof(pHelloMsg->param2) + sizeof(pHelloMsg->param3)) 
            return;     
        map<CMDGateHello_t, int>::iterator iter = m_LinkType.find(*pHelloMsg);
        if (iter != m_LinkType.end())         
       		insert(link, iter->second);
       	else
       	{
       	   LOG_PRINT(log_warning, "unknown hello type:%d,%d,%d,%d", (int)pHelloMsg->param1, (int)pHelloMsg->param2, (int)pHelloMsg->param3, (int)pHelloMsg->param4);
       	}  	
    }
    
    bool insert(LINK_TYPE link, int type = E_GATEWAY)
    {  
        if (link == NULL || type < 0 || type >= m_LinkPool.size()) return false; 
        LOG_PRINT(log_info, "type:%d conn:%u %s:%u", type, link->getconnid(), link->getremote_ip(), link->getremote_port());      
        m_LinkPool[type].insert(link);
        return true;
    }
    
    bool count(LINK_TYPE link)
    {
        if (link == NULL) return false; 
        for (int type = 0; type < m_LinkPool.size(); ++type)
            if (m_LinkPool[type].count(link) != 0) 
                return true;
        return false;
    }
    
    bool GetType(LINK_TYPE link, int &type)
    {  
        if (link == NULL) return false; 
        for (type = 0; type < m_LinkPool.size(); ++type)
            if (m_LinkPool[type].count(link) != 0) 
                return true;
        return false;
    }
    
    void Cast(const char * data, size_t len, int type = E_GATEWAY)
    {   
        if (data == NULL || len == 0 || type < 0 || type >= m_LinkPool.size()) return;
        LOG_PRINT(log_info, "Cast to %d, link num: %u", type, m_LinkPool[type].size());
        typename set<LINK_TYPE>::iterator iter;
        iter = m_LinkPool[type].begin();
        for (; iter != m_LinkPool[type].end(); ++iter)
        {        
            LOG_PRINT(log_info, "conn:%u %s:%u", (*iter)->getconnid(), (*iter)->getremote_ip(), (*iter)->getremote_port());   
            (*iter)->write_message(data, len);
        }
    }
    
    bool erase(LINK_TYPE link)
    {  
        if (link == NULL) return false; 
        LOG_PRINT(log_info, "conn:%u %s:%u", link->getconnid(), link->getremote_ip(), link->getremote_port());      
        for (int type = 0; type < m_LinkPool.size(); ++type)
            m_LinkPool[type].erase(link);
        return true;
    }
    
    vector<set<LINK_TYPE> > m_LinkPool; 
    size_t m_MaxLinkType;
    map<CMDGateHello_t, int, cmp_CMDGateHello_t> m_LinkType;      
};

#endif //__LINK_LIST_H__
