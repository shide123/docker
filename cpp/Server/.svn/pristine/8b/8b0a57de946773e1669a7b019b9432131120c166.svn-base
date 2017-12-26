/*
 *  redis_map_map.h
 *
 *  Created on: 2016年5月25日
 *      Author: zhangshaochen
 */

#ifndef __REDIS_MAP_ZSET_H__
#define __REDIS_MAP_ZSET_H__

#include "redis_map_zset_base.h"
	

class redis_map_zset :public redis_map_zset_base
{
public:

	template<typename NameType>
    redis_map_zset(int db, NameType name, redisMgr * pRedisMgr = NULL, int run_cmd_Mod = NOSYNC, const char * syncChannel = KEY_ROOM_MSG, int syncdb = 0, int basic_sync_cmd = BASIC_SYNC, const char * bscSplit = ","):
	    redis_map_zset_base(pRedisMgr, run_cmd_Mod, syncChannel, syncdb, basic_sync_cmd, bscSplit),
	    m_name(name),
	    m_db(db) 
	{
		stringstream ss;
		ss << name;
		m_name.assign(ss.str());   
	}
	
	redis_map_zset():
	    m_db(0)
	{
	}
	
	~redis_map_zset()
	{
	}
	
	template<typename NameType>
	bool init(int db, NameType & name, redisMgr * pRedisMgr = NULL, int run_cmd_Mod = NOSYNC, const char * syncChannel = KEY_ROOM_MSG, int syncdb = 0, int basic_sync_cmd = BASIC_SYNC, const char * bscSplit = ",")
    {
        m_db = db;
        m_name.assign(name);
	    return redis_map_zset_base::init(pRedisMgr, run_cmd_Mod, syncChannel, syncdb, basic_sync_cmd, bscSplit);
	}
		 
	void init()
	{
	     return redis_map_zset_base::init();
	}
	
	template<typename FieldType, typename ValType>
    bool insert(const FieldType & field, const ValType & score_value)
    {
        string tmp_field;
        bitTostring(field, tmp_field);      
        string tmp_val;
        bitTostring(score_value, tmp_val); 
        return redis_map_zset_base::insert(m_name, tmp_field, tmp_val, m_db);
    }

    template<typename ValType>  
    bool erase(const ValType & min_score, const ValType & max_score)
    {
		string tmp_min_val;
		bitTostring(min_score, tmp_min_val); 
		string tmp_max_val;
		bitTostring(max_score, tmp_max_val); 
        return redis_map_zset_base::erase(m_name, tmp_min_val, tmp_max_val, m_db);
    }

	template<typename ValType>  
	bool getByScoreValue(const ValType & min_score, const ValType & max_score, unsigned long pos, unsigned int batch_num, std::map<std::string, std::string > & value_score_map)
	{
		string tmp_min_val;
		bitTostring(min_score, tmp_min_val); 
		string tmp_max_val;
		bitTostring(max_score, tmp_max_val); 
		return redis_map_zset_base::getByScoreValue(m_name, tmp_min_val, tmp_max_val, pos, batch_num, value_score_map, m_db);
	}

	bool getByPosValue(long start_pos, long end_pos, std::map<std::string, std::string > & value_score_map)
	{
		return redis_map_zset_base::getByPosValue(m_name, start_pos, end_pos, value_score_map, m_db);
	}

private:
    string m_name;
    int m_db;        
};

#endif /* __REDIS_MAP_ZSET_H__ */
