#include <cstdarg>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <ctype.h>
#include <iostream>
#include <functional>
#include <algorithm>
#include <time.h>
#include <vector>
#include <list>
#include <set>
#include "utils.h"

std::string StringTrimA(std::string& s)
{
	const std::string drop = " ";
	// trim right
	s.erase(s.find_last_not_of(drop)+1);
	// trim left
	return s.erase(0,s.find_first_not_of(drop));
}

void SplitStringA(const char *pSrc, char chMark, std::vector<std::string> &vecStrings, int bOnce/*=0*/)
{
	vecStrings.clear();
	if (!pSrc)
		return;

	char *pFront = (char*)pSrc;
	char *pBack = (char*)pSrc;
	char *pEnd = (char*)pSrc + (strlen(pSrc)-1);
	if (!bOnce)
	{
		for (; pBack<=pEnd; ++pBack)
		{
			if (*pBack == chMark)
			{
				char chTmp = *pBack;
				*pBack = '\0';
				vecStrings.push_back(pFront);
				*pBack = chTmp;
				pFront = pBack+1;
			}
			else if (pBack == pEnd)
			{
				vecStrings.push_back(pFront);
				break;
			}
		}
	}
	else
	{
		for (; pBack<=pEnd; ++pBack)
		{
			if (*pBack == chMark)
			{
				*pBack = '\0';
				vecStrings.push_back(pFront);
				pFront = pBack+1;
				vecStrings.push_back(pFront);
				break;
			}
		}
	}
}

void SplitAndTrim(const char *pSrc, char delim, std::vector<std::string> &vecStrings)
{
    std::string::size_type begin = 0;
    std::string::size_type pos   = 0;
    std::string str(pSrc);

    while (begin < str.size() && str[begin] == delim && ++ begin);

    while ((pos = str.find(delim, begin)) != std::string::npos) {
        vecStrings.push_back(str.substr(begin, pos - begin));
        begin = pos + 1;

        while (begin < str.size() && str[begin] == delim && ++ begin);
    }
    if (begin < str.size()) {
        pos = str.size();
        vecStrings.push_back(str.substr(begin, pos - begin));
    }
}


bool IsInVisitorId(unsigned int userId)
{
	//检查是不是游客,游客的号码段
	unsigned int m_visitor_maxid = 1500000000;
	unsigned int m_visitor_minid = 900000000;
	if(userId >= m_visitor_minid && userId <= m_visitor_maxid)
		return true;
	return false;
}

bool isspecial(char c) {
	if (c =='-' || c == '_' || c == ' ' || c == '|')
		return 1;
	return 0;
}

vector<string> strToVec(const string &s, const char delimiter)
{
	vector<string> vec;
	size_t last = 0;
	size_t next = 0;
	while ((next = s.find(delimiter, last)) != string::npos) {
		vec.push_back(s.substr(last, next - last));
		last = next + 1;
	}
	vec.push_back(s.substr(last));
	return vec;
}

string vecToStr(const vector<string> &vec, const char delimiter) {
  string str;
  for (size_t i = 0; i < vec.size() - 1; i++)
    str += vec[i] + delimiter;
  str += vec[vec.size() - 1];
  return str;
}

vector<string> strToVec(const string &s, const char* delimiter)
{
	vector<string> vec;
	size_t last = 0;
	size_t next = 0;
	size_t skip = strlen(delimiter);
	while ((next = s.find(delimiter, last)) != string::npos) {
		vec.push_back(s.substr(last, next - last));
		last = next + skip;
	}
	vec.push_back(s.substr(last));
	return vec;
}

std::string vecToStr(const std::vector<std::string> &vec, const char* delimiter)
{
	  string str;
	  for (size_t i = 0; i < vec.size() - 1; i++)
	    str += vec[i] + delimiter;
	  str += vec[vec.size() - 1];
	  return str;
}

int replace(string& LineData, string &src_Split, string &dest_Split, int max) {
    size_t Loc = 0;  
    int index = 0;  
    do {        
        Loc = LineData.find(src_Split, Loc);
        if (Loc != string::npos)
            LineData.replace(Loc, src_Split.size(), dest_Split);
        else
            break;
        Loc = Loc + abs((int)src_Split.size() - (int)dest_Split.size()) + 1;
        
        ++index;
        if (max > 0 && index >= max) break;
            
    } while(1); 
    return index;            
}

//字符串分割函数
std::vector<std::string> split(std::string str,std::string pattern)
{
    std::string::size_type pos;
    std::vector<std::string> result;
    if(str.empty())
    	return result;
    str+=pattern;//扩展字符串以方便操作
    int size=str.size();

    for(int i=0; i<size; i++)
    {
        pos=str.find(pattern,i);
        if(pos<size)
        {
            std::string s=str.substr(i,pos-i);
            result.push_back(s);
            i=pos+pattern.size()-1;
        }
    }
    return result;
}


void splitStrToVec(const std::string & stringInput, std::string splitChar, std::vector<std::string> & str_vec)
{
	str_vec.clear();
	if (stringInput.empty())
	{
		return;
	}

	std::string word = stringInput;
	std::string::size_type pos = word.find(splitChar);
	while (pos != std::string::npos) 
	{
		std::string sub = word.substr(0, pos);
		str_vec.push_back(sub);
		word = word.substr(pos + 1);
		pos = word.find(splitChar);
	}

	if (!word.empty())
	{
		str_vec.push_back(word);
	}
}
void splitStrToLst(const std::string & stringInput, char splitChar, std::list<std::string> & string_lst)
{
	string_lst.clear();
	if (stringInput.empty())
	{
		return;
	}

	std::string word = stringInput;
	std::string::size_type pos = word.find(splitChar);
	while (pos != std::string::npos) 
	{
		std::string sub = word.substr(0, pos);
		string_lst.push_back(sub);
		word = word.substr(pos + 1);
		pos = word.find(splitChar);
	}

	if (!word.empty())
	{
		string_lst.push_back(word);
	}
}

void splitStrToLst(const std::string & stringInput, char splitChar, std::list<unsigned int> & int_lst)
{
	int_lst.clear();
	if (stringInput.empty())
	{
		return;
	}

	unsigned int subInt = 0;
	std::string word = stringInput;
	std::string::size_type pos = word.find(splitChar);
	while (pos != std::string::npos) 
	{
		std::string sub = word.substr(0, pos);
		subInt = strtoul(sub.c_str(), NULL, 0);
		int_lst.push_back(subInt);
		word = word.substr(pos + 1);
		pos = word.find(splitChar);
	}

	if (!word.empty())
	{
		subInt = strtoul(word.c_str(), NULL, 0);
		int_lst.push_back(subInt);
	}
}
void splitStrToSet(const std::string & stringInput, std::string splitChar, std::set<unsigned int> & int_set)
{
	int_set.clear();
	if (stringInput.empty())
	{
		return;
	}

	unsigned int subInt = 0;
	std::string word = stringInput;
	std::string::size_type pos = word.find(splitChar);
	while (pos != std::string::npos) 
	{
		std::string sub = word.substr(0, pos);
		subInt = strtoul(sub.c_str(), NULL, 0);
		int_set.insert(subInt);
		word = word.substr(pos + splitChar.size());
		pos = word.find(splitChar);
	}

	if (!word.empty())
	{
		subInt = strtoul(word.c_str(), NULL, 0);
		int_set.insert(subInt);
	}
}

void splitStrToSet(const std::string & stringInput, std::string splitChar, std::set<std::string> & str_set)
{
	str_set.clear();
	if (stringInput.empty())
	{
		return;
	}

	std::string word = stringInput;
	std::string::size_type pos = word.find(splitChar);
	while (pos != std::string::npos)
	{
		std::string sub = word.substr(0, pos);
		str_set.insert(sub);
		word = word.substr(pos + splitChar.size());
		pos = word.find(splitChar);
	}

	if (!word.empty())
	{
		str_set.insert(word);
	}
}

/*
  Date:2011年10月30日
  by LiuYongsheng
  
  说明：简单加解密程序。加解密方法如下。
  ABCDEFG  (原始数据)
  ^^^^^^^     |  ^
  Mzyxwvu     v  |
  zyxwvut  (密文数据）
*/
void simp_encrypt(char* src, int len, char magic)
{
	if(!src)
		return;

	int i;
	src[0] ^= magic;

	for(i = 1; i < len; i++)
	{
		src[i] ^= src[i-1];
	}
	return;
}

void simp_decrypt(char* src,int len, char magic)
{
	if(!src)
		return;

	int i;
	for(i = len - 1; i > 0; i--)
	{
		src[i] ^= src[i-1];
	}
	src[0] ^= magic;
}

void ffunc01i(unsigned int *v, unsigned int *k) 
{
   unsigned int y=v[0], z=v[1], sum=0, i;
   unsigned int delta=0x9e9d79ea;
   unsigned int a=k[0],b=k[1],c=k[2],d=k[3];
   for(i=0; i<32; i++) {
	   sum += delta;
	   y += ((z<<4) + a)^(z+sum)^((z>>5) +b);
	   z += ((y<<4) + c)^(y+sum)^((y>>5) +d);
   }
   v[0]=y;
   v[1]=z;
}

void ffunc02i(unsigned int *v, unsigned int *k)
{
	unsigned int y=v[0],z=v[1],sum=0xd3af3d40, i;
	unsigned int delta=0x9e9d79ea;
	unsigned int a=k[0],b=k[1],c=k[2],d=k[3];
	for(i=0; i<32; i++) {
		z -=((y<<4)+c)^(y+sum)^((y>>5)+d);
		y -=((z<<4)+a)^(z+sum)^((z>>5)+b);
		sum -= delta;
	}
	v[0]=y;
	v[1]=z;
}

void ffunc1(char* pbuf, int nlen, unsigned int *k)
{
  int nlen2 = ((nlen +7)/8) * 8;
  char* pbuf2 = new char[nlen2];
  memset(pbuf2, 0, sizeof(char) * nlen2);
  memcpy(pbuf2, pbuf, nlen);

  unsigned int* pIntVal = (unsigned int*)pbuf2;
  for(int i=0; i<nlen2/4; i+=2)
  {
	  ffunc01i(&pIntVal[i], (unsigned int*)k);
  }
  memcpy(pbuf, pbuf2, nlen);
  delete[] pbuf2;
}

void ffunc2(char* pbuf, int nlen, unsigned int *k)
{
	int nlen2 = ((nlen +7)/8) * 8;
	char* pbuf2 = new char[nlen2];
	memset(pbuf2, 0, sizeof(char) * nlen2);
	memcpy(pbuf2, pbuf, nlen);

	unsigned int* pIntVal = (unsigned int*)pbuf2;
	for(int i=0; i<nlen2/4; i+=2)
	{
		ffunc02i(&pIntVal[i], (unsigned int*)k);
	}
	memcpy(pbuf, pbuf2, nlen);
	delete[] pbuf2;
}

uint64 convVersionStringToU64(const std::string strVersion)
{
	uint32 v1 = 0, v2 = 0, v3 = 0;
	sscanf(strVersion.c_str(), "%u.%u.%u", &v1, &v2, &v3);
	uint64 uVersion = 0;
	short *pVersion = (short *)&uVersion;
	pVersion[0] = v1;
	pVersion[1] = v2;
	pVersion[2] = v3;

	return uVersion;
}

std::string convVersionU64ToString(uint64 uVersion)
{
	short *pVersion = (short *)&uVersion;
	char szVersion[64] = {};
	snprintf(szVersion, sizeof(szVersion)-1, "%d.%d.%d", pVersion[0], pVersion[1], pVersion[2]);

	return szVersion;
}

int cmpVersionString(const std::string v1, const std::string v2)
{
	uint64 uV1 = convVersionStringToU64(v1);
	uint64 uV2 = convVersionStringToU64(v2);
	short *pV1 = (short *)uV1;
	short *pV2 = (short *)uV2;

	for (int i = 0; i < 3; i++)
	{
		if (pV1[i] > pV2[i])
			return 1;
		else if (pV1[i] < pV2[i])
			return -1;
	}

	return 0;
}

std::string getDateStringFromTimestamp(time_t t)
{
	struct tm *tm = localtime(&t);
	char ts[32] = {};
	sprintf(ts,
		"%d-%02d-%02d %02d:%02d:%02d",
		tm->tm_year + 1900,
		tm->tm_mon  + 1,
		tm->tm_mday,
		tm->tm_hour,
		tm->tm_min,
		tm->tm_sec);

	return ts;
}

std::string trim(const std::string & strInput)
{
	if (strInput.empty())
	{
		return strInput;
	}

	std::string result = strInput;
	result.erase(0, result.find_first_not_of(" "));
	result.erase(result.find_last_not_of(" ") + 1);
	return result;
}

#ifndef __GNUC__ 
std::string stringFormat(const char *fmt, ...) 
{ 
	std::string strResult=""; 
	if (NULL != fmt) 
	{ 
		va_list marker = NULL; 
		va_start(marker, fmt); 

		size_t nLength = _vscprintf(fmt, marker) + 1; 
		std::vector<char> vBuffer(nLength, '\0'); 

		int nRet = _vsnprintf_s(&vBuffer[0], vBuffer.size(), nLength, fmt, marker); 
		if (nRet > 0) 
		{
			strResult = &vBuffer[0];  

		} 

		va_end(marker); 
	} 
	return strResult; 
} 
#else 
std::string stringFormat(const char *fmt,...) 
{ 
	std::string strResult=""; 
	if (NULL != fmt) 
	{ 
		va_list marker; 
		va_start(marker, fmt); 

		char *buf = 0; 
		int result = vasprintf (&buf, fmt, marker); 

		if (!buf) 
		{ 
			va_end(marker); 
			return strResult; 
		} 

		if (result < 0) 
		{ 
			free (buf); 
			va_end(marker); 
			return strResult; 
		} 

		result = strlen (buf); 
		strResult.append(buf,result); 

		free(buf); 
		va_end(marker); 
	} 
	return strResult; 
} 
std::string stamp_to_standard(int stampTime,const char* format)
{
    time_t tick = (time_t)stampTime;
    struct tm tm;
    char s[100];

    tm = *localtime(&tick);
    strftime(s, sizeof(s), format, &tm);
    printf("%d: %s\n", (int)tick, s);

    return s;
}
Geohash::Geohash()
{
    base32["00000"] = "0";
    base32["00001"] = "1";

    base32["00010"] = "2";
    base32["00011"] = "3";

    base32["00100"] = "4";
    base32["00101"] = "5";
    base32["00110"] = "6";
    base32["00111"] = "7";

    base32["01000"] = "8";
    base32["01001"] = "9";
    base32["01010"] = "b";//10
    base32["01011"] = "c";//11
    base32["01100"] = "d";//12
    base32["01101"] = "e";//13
    base32["01110"] = "f";//14
    base32["01111"] = "g";//15

    base32["10000"] = "h";//16
    base32["10001"] = "j";//17
    base32["10010"] = "k";//18
    base32["10011"] = "m";//19
    base32["10100"] = "n";//20
    base32["10101"] = "p";//21
    base32["10110"] = "q";//22
    base32["10111"] = "r";//23
    base32["11000"] = "s";//24
    base32["11001"] = "t";//25
    base32["11010"] = "u";//26
    base32["11011"] = "v";//27
    base32["11100"] = "w";//28
    base32["11101"] = "x";//29
    base32["11110"] = "y";//30
    base32["11111"] = "z";//31
}
Geohash::~Geohash()
{

}
string Geohash::getGeohash(double j, double w)
{
    string s_w="", s_j="";
    string s="", ss="";

    s_w = geohash_w_bin(w, -90, 90, 1, 30);
    s_j = geohash_w_bin(j, -180, 180, 1, 30);

    reverse(s_w.begin(), s_w.end());
    reverse(s_j.begin(), s_j.end());

    s = geohash_merge(s_j, s_w);
    s= geohash_base32(s);
    return s;
}
void Geohash::geohash_search(string base32)
{

}
string Geohash::geohash_w_bin(double w,double left,double right,int step,int max_step)
{
	if (step > max_step)
	{
		return "";
	}
	double mid = (left + right)*1.0 / 2;

	if (w >= left && w <= mid)
	{
		return geohash_w_bin(w, left, mid,step+1,max_step)+"0";
	}
	if (w >= mid && w <= right)
	{
		return geohash_w_bin(w, mid, right,step+1,max_step)+"1";
	}
}
string Geohash::geohash_merge(string j, string w)
{
	string s;
	for (int i = 0; i <j.size(); i++)
	{
		s += j[i];
		s += w[i];
	}
	return s;
}
string Geohash::geohash_base32(string s)
{
    string temp;
    string ans;
    for (int i = 0; i < s.size(); i++) {
        temp += s[i];
        if ((i + 1) % 5 == 0) {
            ans += base32[temp];
            temp = "";
        }
    }

    return ans;
}
string Geohash::jiequ(string s, int l, int r)
{
    string temp;
    for (int i = 0; i < r; i++)
    {
        temp += s[i];
    }
    return temp;
}
#endif
std::string getTime(string format,int add)
{
	char ctime[255] = {0};
	time_t Today;
	tm Time;

	Today = time(NULL);
	Today += add * 60 * 60 ;
	localtime_r(&Today,&Time);
	strftime(ctime,255,format.c_str(),&Time);
	return ctime;
}

std::string getTime_H(string format,int add)
{
	char ctime[255] = {0};
	time_t Today;
	tm Time;

	Today = time(NULL);
	Today += add * 60 ;
	localtime_r(&Today,&Time);
	strftime(ctime,255,format.c_str(),&Time);
	return ctime;
}
