#include "KeywordMgr.h"
#include "DBTools.h"
#include "macro_define.h"
#include <boost/regex.hpp>

redis_map_map CKeywordMgr::m_keyword_hashmap;
redis_set CKeywordMgr::m_keyword_set;

CKeywordMgr::CKeywordMgr(void)
{
}

CKeywordMgr::~CKeywordMgr(void)
{
}
int CKeywordMgr::init(redisMgr * pRedisMgr)
{
	if (!pRedisMgr)
	{
		LOG_PRINT(log_error, "redisMgr is null.init failed.");
		return -1;
	}

	m_keyword_hashmap.init(DB_ROOM, KEY_HASH_KEYWORD_INFO, pRedisMgr);
	m_keyword_set.init(DB_ROOM, "talk_keyword", pRedisMgr);

	return 0;
}
void CKeywordMgr::loadKeyword()
{
	int recnum = 0;
	const int linenum = 10000;
	int offset = 0;

	std::vector<CMDAdKeywordInfo_t> vKeyword;
	while(1){
		vKeyword.clear();
		recnum = CDBTools::getAdKeywordList(offset, linenum, vKeyword);
		LOG_PRINT(log_info, "keyword num %d", recnum); 
		for (int i = 0; i < vKeyword.size(); i++)
		{
			CMDAdKeywordInfo_t &ri = vKeyword[i];
			AddKeyword(ri);
			LOG_PRINT(log_debug, "load keyword:naction:%d,ntype:%d,nrunerid:%d,createtime:%s,keyword:%s",
				ri.naction,ri.ntype,ri.nrunerid,ri.createtime,ri.keyword);
		}

		if (recnum < linenum)
		{
			LOG_PRINT(log_info, "keyword break");
			break;
		}
		offset += recnum;
	}
}

void CKeywordMgr::AddKeyword(CMDAdKeywordInfo_t &KeywordInfo)
{
#if 0
	std::map<std::string, std::string> oFieldValueMap;
	oFieldValueMap[KeywordInfo.keyword] = KeywordInfo.replace;
	m_keyword_hashmap.insert(1,oFieldValueMap);
#else
	m_keyword_set.insert(KeywordInfo.keyword);
#endif
}

void CKeywordMgr::DelKeyword(std::string pKeyword)
{
	if (pKeyword == "")
		return;
#if 0
	m_keyword_hashmap.erase(1,pKeyword);
#else
	m_keyword_set.erase(pKeyword);
#endif
}

void CKeywordMgr::getKeyword(std::map<std::string,std::string>& keyword_map)
{
	do
	{
		IF_METHOD_FALSE_BREAK_EX(m_keyword_hashmap.getall(1,keyword_map), "fail to getKeyword info from redis.");
		if (!keyword_map.empty())
		{
			return ;
		}
	}while(0);
	loadKeyword();
}

void CKeywordMgr::getKeyword(std::vector<std::string>& keyword_vec)
{
	if (!m_keyword_set.find(keyword_vec))
	{
		LOG_PRINT(log_error, "fail to getKeyword info from redis.");
		return;
	}

	if (!keyword_vec.empty())
		return;

	CDBTools::getAdKeywordList(0, 10000, keyword_vec);
}

void CKeywordMgr::replaceKey(std::string& str)
{
#if 0 // 20171114, wxc: old keywords
	std::map<std::string,std::string> mapkeyword;
	getKeyword(mapkeyword);
	LOG_PRINT(log_warning, "dealGroupMsgReq size:%d",mapkeyword.size());
	std::map<std::string,std::string>::iterator it = mapkeyword.begin();
	for(; it != mapkeyword.end() ; it++)
	{
		string::size_type pos=0;
		std::string replace = it->second;
		std::string word = it->first;
		LOG_PRINT(log_warning, "dealGroupMsgReq first:%s,second:%s",word.c_str(),replace.c_str());
		while((pos = str.find(word,pos))!=string::npos)
		{
			str.replace(pos,word.size(),replace);
			LOG_PRINT(log_warning, "dealGroupMsgReq pos:%d,word.size:%d,str:%s,replace:%s",pos,word.size(),str.c_str(),replace.c_str());
			pos += replace.size();
		}
		LOG_PRINT(log_warning, "dealGroupMsgReq for replaceKey:%s",str.c_str());
	}
	LOG_PRINT(log_warning, "dealGroupMsgReq replaceKey:%s",str.c_str());
#else // 20171114, wxc: new keywords
	std::vector<std::string> keywordVec;
	std::string replace("****");
	std::string::size_type replaceSize = replace.size();

	getKeyword(keywordVec);
	LOG_PRINT(log_warning, "keyword count: %d", keywordVec.size());

	for (std::vector<std::string>::iterator it = keywordVec.begin(); it != keywordVec.end(); ++it)
	{
		std::string::size_type pos = 0;

		while (std::string::npos != (pos = str.find(*it, pos)))
		{
			str.replace(pos, it->size(), replace);
			LOG_PRINT(log_warning, "dealGroupMsgReq pos:%d,word.size:%d,str:%s,replace:%s",pos,it->size(),it->c_str(),replace.c_str());
			pos += replaceSize;
		}
	}
	LOG_PRINT(log_warning, "dealGroupMsgReq replaceKey:%s",str.c_str());
#endif
}
