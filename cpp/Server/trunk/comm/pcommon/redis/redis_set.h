/*
 *  redis_map.h
 *
 *  Created on: 2016年5月25日
 *      Author: zhangshaochen
 */

#ifndef _REDIS_SET_H_
#define _REDIS_SET_H_

#include "redis_map_set_base.h"

	

class redis_set :public redis_map_set_base {
public:

	template<typename NameType>
    redis_set(int db, NameType name, redisMgr *pRedisMgr = NULL, int run_cmd_Mod = NOSYNC, const char *syncChannel = KEY_ROOM_MSG, int syncdb = 0, int basic_sync_cmd = BASIC_SYNC, const char *bscSplit = ","):
	    redis_map_set_base(pRedisMgr, run_cmd_Mod, syncChannel, syncdb, basic_sync_cmd, bscSplit),
	    m_db(db) 
	{	    
        stringstream ss;
	    ss<<name;
	    m_name.assign(ss.str());  
	}
	
	redis_set():
	    m_db(0)
	{
	}
	
	~redis_set()
	{
	}
	
	template<typename NameType>
	bool init(int db, NameType &name, redisMgr *pRedisMgr = NULL, int run_cmd_Mod = NOSYNC, const char *syncChannel = KEY_ROOM_MSG, int syncdb = 0, int basic_sync_cmd = BASIC_SYNC, const char *bscSplit = ",")
    {
        m_db = db;
        stringstream ss;
	    ss<<name;
	    m_name.assign(ss.str());  
	    return redis_map_set_base::init(pRedisMgr, run_cmd_Mod, syncChannel, syncdb, basic_sync_cmd, bscSplit);
	}
		 
	void init()
	{
	     return redis_map_set_base::init();
	}
	
	template<typename KeyType>
    bool insert(const KeyType &key)
    {
        string tmp_key;
        bitTostring(key, tmp_key);
        return redis_map_set_base::insert(m_name, tmp_key, m_db);     
    }
    
    template<typename KeyType> 
    bool find(vector<KeyType> &keys)
    {  
        
        vector<string> tmp_keys;
        if (!redis_map_set_base::find(m_name, tmp_keys, m_db) )
            return false;        
        if (tmp_keys.empty())
            return false;
        vector<KeyType> res(tmp_keys.size(), KeyType());
        for (int index = 0; index < tmp_keys.size(); ++index)
        {
           res[index] = tmp_keys[index]; 
        }
        keys.swap(res);
        return true;
    }
    
    template<typename KeyType>
    int count(const KeyType &key)
    {
        string tmp_key;
        bitTostring(key, tmp_key);
        return redis_map_set_base::count(m_name, tmp_key, m_db);      
    }
    
    template<typename KeyType>    
    bool erase(const KeyType &key)
    {
        string tmp_key;
        bitTostring(key, tmp_key);
        return redis_map_set_base::erase(m_name, tmp_key, m_db);
    }
     
    bool clear()
    {        
        vector<string> tmp_keys;
        redis_map_set_base::find(m_name, tmp_keys, m_db);
        if (tmp_keys.empty())
            return true;
        for (int index = 0; index < tmp_keys.size(); ++index)
        {
           if(!redis_map_set_base::erase(m_name, tmp_keys[index], m_db))
            return false;
        }
        return true;        
    }
    
private:
    string m_name;
    int m_db;        
};

#endif /* REDIS_MAP_H_ */
