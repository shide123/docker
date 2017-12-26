#ifndef __KEYWORDMGR_H__
#define __KEYWORDMGR_H__

#include "time.h"
#include "cmd_vchat.h"
#include "trie_tree.h"
#include <boost/thread/shared_mutex.hpp>
#include <map>
#include <vector>
#include "redis_map_map.h"
#include "redis_set.h"
#include <string>

class CKeywordMgr
{
public:
	CKeywordMgr(void);
	~CKeywordMgr(void);
	static int init(redisMgr * pRedisMgr);

	static void loadKeyword();
	static void AddKeyword(CMDAdKeywordInfo_t &KeywordInfo);
	static void DelKeyword(std::string pKeyword);
	static void getKeyword(std::map<std::string,std::string>& keyword_map);
	static void getKeyword(std::vector<std::string>& keyword_vec);
	static void replaceKey(std::string& str);

private:
	static redis_map_map m_keyword_hashmap;
	static redis_set m_keyword_set;
};

#endif //__KEYWORDMGR_H__
