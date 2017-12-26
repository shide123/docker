#ifndef __UTILS_HH_20150210__
#define __UTILS_HH_20150210__
#include <string>
#include <vector>
#include <set>
#include <map>
#include <sstream>
#include <time.h>
#include "CLogThread.h"

using namespace std;

bool isspecial(char c);
vector<string> strToVec(const string &s, const char delimiter = ' ');
std::string vecToStr(const std::vector<std::string> &vec, const char delimiter = ' ');

vector<string> strToVec(const string &s, const char* delimiter);
std::string vecToStr(const std::vector<std::string> &vec, const char* delimiter);

int replace(string& LineData, string &src_Split, string &dest_Split, int max = 0);

template<typename BitType>
inline void stringTobit(BitType &bit, string &str)
{
    bit = str;
}

template<>
inline void stringTobit<string>(string &bit, string &str)
{
    bit = str;
}

template<>
inline void stringTobit<unsigned int>(unsigned int &bit, string &str)
{
    bit = atoi(str.c_str());
}

template<>
inline void stringTobit<uint16_t>(uint16_t &bit, string &str)
{
    bit = atoi(str.c_str());
}

template<>
inline void stringTobit<int16_t>(int16_t &bit, string &str)
{
    bit = atoi(str.c_str());
}

template<>
inline void stringTobit<int>(int &bit, string &str)
{
    bit = atoi(str.c_str());
}

template<>
inline void stringTobit<unsigned long long>(unsigned long long &bit, string &str)
{
    bit = atoll(str.c_str());
}

template<>
inline void stringTobit<int64>(int64 &bit, string &str)
{
    bit = atoll(str.c_str());
}

template<>
inline void stringTobit<unsigned char>(unsigned char &bit, string &str)
{
	bit = (unsigned char)atoi(str.c_str());
}

template<>
inline void stringTobit<char>(char &bit, string &str)
{
	bit = (char)atoi(str.c_str());
}

template<typename BitType>
inline void bitTostring(const BitType &bit, string &str)
{
   bit.str().swap(str); 
}

template<>
inline void bitTostring<string>(const string &bit, string &str)
{
    str = bit;
}

inline void bitTostring(const char *bit, string &str)
{
    str = bit;
}

template<>
inline void bitTostring<unsigned int>(const unsigned int &bit, string &str)
{
    stringstream ss;
    ss << bit;
    str.assign(ss.str()); 
}

template<>
inline void bitTostring<int>(const int &bit, string &str)
{
    stringstream ss;
    ss << bit;
    str.assign(ss.str());
}

template<>
inline void bitTostring<uint64>(const uint64 &bit, string &str)
{
    stringstream ss;
    ss << bit;
    str.assign(ss.str());
}

template<>
inline void bitTostring<int64>(const int64 &bit, string &str)
{
    stringstream ss;
    ss << bit;
    str.assign(ss.str());
}

template<typename BitType>
inline string bitTostring(const BitType &bit)
{
    return bit.str();
}

template<>
inline string bitTostring<string>(const string &bit)
{
    return bit;
}

template<>
inline string bitTostring<unsigned int>(const unsigned int &bit)
{
    stringstream ss;
    ss << bit;
    return ss.str();
}

inline string bitTostring(const short &bit)
{
	stringstream ss;
	ss << bit;
	return ss.str();
}

inline string bitTostring(const unsigned short &bit)
{
	stringstream ss;
	ss << bit;
	return ss.str();
}

inline string bitTostring(const char &bit)
{
	stringstream ss;
	ss << bit;
	return ss.str();
}

inline string bitTostring(const unsigned char &bit)
{
	stringstream ss;
	ss << bit;
	return ss.str();
}

template<>
inline string bitTostring<int>(const int &bit)
{
    stringstream ss;
    ss << bit;
    return ss.str();
}

template<>
inline string bitTostring<uint64>(const uint64 &bit)
{
	stringstream ss;
	ss << bit;
	return ss.str();
}
template<>
inline string bitTostring<double>(const double &bit)
{
	char buf[32] = { 0 };
	sprintf(buf, "%lf", bit);
	stringstream ss;
	ss << buf;
	return ss.str();
}
template<typename Type>
inline void getDataFromVec(Type &dest, vector<string> &vals, int pos)
{	
    if (pos > vals.size() - 1) return;
    dest = atoi(vals[pos].c_str());	
}

template<>
inline void getDataFromVec<uint64>(uint64 &dest, vector<string> &vals, int pos) 
{	
    if (pos > vals.size() - 1) return;
    dest = atoll(vals[pos].c_str());	
}

template<>
inline void getDataFromVec<string>(string &dest, vector<string> &vals, int pos) 
{	
    if (pos > vals.size() - 1) return;
    dest = vals[pos];	
}

template<>
inline void getDataFromVec<char>(char &dest, vector<string> &vals, int pos) 
{	
    if (pos > vals.size() - 1) return;
    if (!vals[pos].empty())    
        dest = *vals[pos].begin();	
}

inline void getDataFromVec(char *dest, vector<string> &vals, int pos, int len) 
{	
    if (pos > vals.size() - 1) return;
    if (!vals[pos].empty())
        strncpy(dest, vals[pos].c_str(), len);    
}

std::vector<std::string> split(std::string str,std::string pattern);
void splitStrToLst(const std::string & stringInput, char splitChar, std::list<std::string> & string_lst);
void splitStrToLst(const std::string & stringInput, char splitChar, std::list<unsigned int> & int_lst);
void splitStrToSet(const std::string & stringInput, std::string splitChar, std::set<unsigned int> & int_set);
void splitStrToSet(const std::string & stringInput, std::string splitChar, std::set<std::string> & int_set);

void splitStrToVec(const std::string & stringInput, std::string splitChar, std::vector<std::string> & str_vec);

void SplitStringA(const char *pSrc, char chMark, std::vector<std::string> &vecStrings, int bOnce=0);
std::string StringTrimA(std::string& s);
void SplitAndTrim(const char *pSrc, char delim, std::vector<std::string> &vecStrings);
std::string trim(const std::string & strInput);

#define SECRET_MAGIC 0x15

void simp_encrypt(char* src, int len, char magic = SECRET_MAGIC);
void simp_decrypt(char* src, int len, char magic = SECRET_MAGIC);

void ffunc01i(unsigned int *v, unsigned int *k);
void ffunc02i(unsigned int *v, unsigned int *k);

void ffunc1(char* pbuf, int nlen, unsigned int *k);
void ffunc2(char* pbuf, int nlen, unsigned int *k);

bool IsInVisitorId(unsigned int userId);

//支持3个级别的版本号（v1.v2.v3）转换为U64
uint64 convVersionStringToU64(const std::string strVersion);
std::string convVersionU64ToString(uint64 uVersion);
//return value: 0(v1 == v2), 1(v1 > v2), -1(v1 < v2)
int cmpVersionString(const std::string v1, const std::string v2);


//datetime
std::string getDateStringFromTimestamp(time_t t);

//string format, return format string
std::string stringFormat(const char *fmt, ...);
std::string getTime(string format,int add = 0);
std::string getTime_H(string format,int add = 0);
std::string stamp_to_standard(int stampTime,const char* format);

class Geohash
{
public:
	Geohash();
	virtual ~Geohash();
	string getGeohash(double j, double w);
	void geohash_search(string base32);
private:
	string geohash_w_bin(double w,double left,double right,int step,int max_step);
	string geohash_merge(string j, string w);
	string geohash_base32(string s);
	string jiequ(string s, int l, int r);
private:
	map<string, string> base32;
};
#endif //__UTILS_HH_20150210__
