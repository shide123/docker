/*
 *  redis_map_map.h
 *
 *  Created on: 2016年5月25日
 *      Author: zhangshaochen
 */

#ifndef REDIS_MAP_MAP_H_
#define REDIS_MAP_MAP_H_

#include "redis_map_map_base.h"
	

class redis_map_map :public redis_map_map_base{
public:

	template<typename NameType>
    redis_map_map(int db, NameType name, redisMgr *pRedisMgr = NULL, int run_cmd_Mod = NOSYNC, const char *syncChannel = KEY_ROOM_MSG, int syncdb = 0, int basic_sync_cmd = BASIC_SYNC, const char *bscSplit = ","):
	    redis_map_map_base(pRedisMgr, run_cmd_Mod, syncChannel, syncdb, basic_sync_cmd, bscSplit),
	    m_name(name),
	    m_db(db) 
	{	    
	}
	
	redis_map_map():
	    m_db(0)
	{
	}
	
	~redis_map_map()
	{
	}
	
	template<typename NameType>
	bool init(int db, NameType &name, redisMgr *pRedisMgr = NULL, int run_cmd_Mod = NOSYNC, const char *syncChannel = KEY_ROOM_MSG, int syncdb = 0, int basic_sync_cmd = BASIC_SYNC, const char *bscSplit = ",")
    {
        m_db = db;
        m_name.assign(name);
	    return redis_map_map_base::init(pRedisMgr, run_cmd_Mod, syncChannel, syncdb, basic_sync_cmd, bscSplit);
	}
		 
	void init()
	{
	     return redis_map_map_base::init();
	}
	
	template<typename KeyType, typename FieldType, typename ValType>
    bool insert(const KeyType &key, const FieldType &field, const ValType &val, bool isname = true)
    {
        string tmp_key;
        build_key(tmp_key, key, isname); 
        string tmp_field;
        bitTostring(field, tmp_field);      
        string tmp_val;
        bitTostring(val, tmp_val); 
        return redis_map_map_base::insert(tmp_key, tmp_field, tmp_val, m_db);     
    }

	template<typename KeyType>
	bool insert(const KeyType &key, const map<string, string> &fields_vals, bool isname = true)
	{
		string tmp_key;
		build_key(tmp_key, key, isname);
		return redis_map_map_base::insert(tmp_key, fields_vals, m_db);     
	}

    template<typename KeyType, typename FieldType>
    bool find(const KeyType &key, const FieldType &field, bool isname = true)
    {
        string tmp_key;        
        build_key(tmp_key, key, isname);
        string tmp_field;
        bitTostring(field, tmp_field);                   
        string tmp_val;
        
        if (!redis_map_map_base::find(tmp_key, tmp_field, tmp_val, m_db) )
            return false;        
        if (tmp_val.empty())
            return false;
        return true;
    }

    template<typename KeyType, typename FieldType, typename ValType>
    bool find(const KeyType &key, const FieldType &field, ValType &val, bool isname = true)
    {
        string tmp_key;        
        build_key(tmp_key, key, isname);
        string tmp_field;
        bitTostring(field, tmp_field);                   
        string tmp_val;
        
        if (!redis_map_map_base::find(tmp_key, tmp_field, tmp_val, m_db) )
            return false;        
        if (tmp_val.empty())
            return false;
        stringTobit(val, tmp_val);    
        return true;
    }
    
    template<typename KeyType, typename FieldType>  
    bool erase(const KeyType &key, const FieldType &field, bool isname = true)
    {
        string tmp_key; 
        build_key(tmp_key, key, isname);
        string tmp_field;
        bitTostring(field, tmp_field);                  
        return redis_map_map_base::erase(tmp_key, tmp_field, m_db);
    }

	template<typename KeyType>
	bool erase(const KeyType &key, bool isname = true)
	{
		string tmp_key;
		build_key(tmp_key, key, isname);  
		return redis_map_map_base::erase(tmp_key, m_db);
	}
    
    template<typename KeyType>
    int size(const KeyType &key, bool isname = true)
    {
        string tmp_key;
        build_key(tmp_key, key, isname);     
        int tmp_size;
        if (!redis_map_map_base::size(tmp_key, tmp_size, m_db))
        {
            return 0;
        }
        return tmp_size;            
    }
    
    template<typename KeyType>
    bool getallkey(vector<KeyType>& keys)
    {
        string filter(m_name);
        filter.append("*");
        return redis_scan_keys(filter, keys, m_db);
    } 
    
	template<typename KeyType>
	bool gethashkeys(const KeyType &key, vector<std::string>& keys, bool isname = true)
	{
		string tmp_key;
		build_key(tmp_key, key, isname);
		return redis_map_map_base::hkeys(tmp_key, keys, m_db);
	}

    template<typename KeyType>
    inline void build_key(string &tmp_key, const KeyType &key, bool isname = true)
    {
        if (isname)
            tmp_key = m_name + bitTostring(key); 
        else
            tmp_key = bitTostring(key); 
    }

	template<typename KeyType>
	bool exist(const KeyType &key, bool isname = true)
	{
		string tmp_key;
		build_key(tmp_key, key, isname);  
		return redis_map_map_base::exist(tmp_key, m_db);
	}

	template<typename KeyType, typename FieldType>
	bool hexists(const KeyType &key, const FieldType &field, bool isname = true)
	{
		string tmp_key, tmp_field;
		build_key(tmp_key, key, isname);
		build_key(tmp_field, field, isname);
		return redis_map_map_base::hexists(tmp_key, tmp_field, m_db);
	}

	template<typename KeyType>
	bool getall(const KeyType &key, vector<std::string> &res, bool isname = true)
	{
		string tmp_key;
		build_key(tmp_key, key, isname);  
		return redis_map_map_base::hgetall(tmp_key, res, m_db);
	}

	template<typename KeyType>
	bool getall(const KeyType &key, map<std::string, std::string> &field_value_map, bool isname = true)
	{
		field_value_map.clear();
		vector<std::string> res;
		string tmp_key;
		build_key(tmp_key, key, isname);  
		bool ret = redis_map_map_base::hgetall(tmp_key, res, m_db);
		if (ret)
		{
			for (int i = 0; i + 1 < res.size(); i += 2)
			{
				field_value_map[res[i]] = res[i+1];
			}
		}
		return ret;
	}

	template<typename KeyType, typename FieldType>
	bool get(const KeyType &key, const FieldType &field, std::string &res, bool isname = true)
	{
		string tmp_key;
		build_key(tmp_key, key, isname);
		string tmp_field;
		bitTostring(field, tmp_field);
		return redis_map_map_base::hget(tmp_key, tmp_field, res, m_db);
	}

	template<typename KeyType, typename FieldType>
	int hincrby(const KeyType &key, const FieldType &field, int increment, bool isname = true)
	{
		string tmp_key;
		build_key(tmp_key, key, isname);
		string tmp_field;
		bitTostring(field, tmp_field);
		return redis_map_map_base::hincrby(tmp_key, tmp_field, increment, m_db);
	}

	template<typename KeyType>
	bool expire(const KeyType &key, unsigned int live, bool isname = true)
	{
		string tmp_key;
		build_key(tmp_key, key, isname);  
		return basic_redis::expire(tmp_key, live, m_db);
	}

	template<typename KeyType>
	bool expireat(const KeyType &key, unsigned int expiration, bool isname = true)
	{
		string tmp_key;
		build_key(tmp_key, key, isname);  
		return basic_redis::expireat(tmp_key, expiration, m_db);
	}

	template<typename KeyType>
	bool del(const KeyType &key, bool isname = true)
	{
		string tmp_key;
		build_key(tmp_key, key, isname);  
		return basic_redis::del(tmp_key, m_db);
	}
private:
    string m_name;
    int m_db;        
};

#endif /* REDIS_MAP_MAP_H_ */
