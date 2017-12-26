#include "ChatCommon.h"
#include <stdio.h>
#include <stdarg.h>

#include <vector>
#include <string>
#include <time.h> 
#include <iostream>
#include <sstream>
#include <algorithm>
using namespace std;

CChatCommon::CChatCommon(void)
{
}

CChatCommon::~CChatCommon(void)
{
}

std::string CChatCommon::getPChatSeqName(uint32 srcid, uint32 dstid)
{
	char szName[64] = {0};
	if (srcid > dstid)
	{
		snprintf(szName, sizeof(szName)-1, "%u_%u", dstid, srcid);
	}
	else
	{
		snprintf(szName, sizeof(szName)-1, "%u_%u", srcid, dstid);
	}

	return std::string(szName);
}

std::string CChatCommon::getGChatSeqName(uint32 groupid)
{
	char szName[64] = {0};
	snprintf(szName, sizeof(szName)-1, "group:%u", groupid);

	return std::string(szName);
}
std::string CChatCommon::getPChatSeqName(uint32 groupid)
{
	char szName[64] = {0};
	snprintf(szName, sizeof(szName)-1, "pgroup:%u", groupid);

	return std::string(szName);
}
std::string CChatCommon::getGPChatSeqName(uint32 groupid, uint32 srcid, uint32 dstid)
{
	char szName[64] = {0};
	if (srcid > dstid)
	{
		snprintf(szName, sizeof(szName)-1, "group%u:%u_%u", groupid, dstid, srcid);
	}
	else
	{
		snprintf(szName, sizeof(szName)-1, "group%u:%u_%u", groupid, srcid, dstid);
	}

	return std::string(szName);
}

std::string CChatCommon::getGPAssistSeqName()
{
	return std::string("group_assist_msg");
}

std::string CChatCommon::getFormatString(const char * fmt, ...)
{
	char szBuff[256] = {0};
	va_list args;
	va_start(args, fmt);
	vsnprintf(szBuff, sizeof(szBuff) - 1, fmt, args);
	va_end(args);

	return std::string(szBuff);
}

bool CChatCommon::chkPMsgValid(const ChatMsg_t & msg)
{
	if ((!msg.srcuser().userid() && msg.msgtype() != MSGTYPE_REMIND) || !msg.dstuser().userid() || msg.content().empty())
		return false;

	return true;
}

bool CChatCommon::chkGMsgValid(const ChatMsg_t & msg)
{
	/*if ((!msg.srcuser().userid() && (MSGTYPE_REMIND != msg.msgtype() && MSGTYPE_LINKS != msg.msgtype() && MSGTYPE_UPLOADWALL != msg.msgtype()
			&& MSGTYPE_STICKER != msg.msgtype() && MSGTYPE_TIP != msg.msgtype())) || (msg.msgtype() == MSGTYPE_REMIND && !msg.dstuser().userid()) || msg.content().empty())*/
	if(msg.content().empty())
		return false;

	return true;
}

void CChatCommon::pickOnlookerChatUTF8(std::string &chatMsg, size_t charLen /*= 22*/)
{
	if (chatMsg.length() <= charLen)
		return;

	size_t count = 0;
	for (size_t i = 0, len = 0; i < chatMsg.length(); i += len)
	{
		unsigned char byte = (unsigned)chatMsg[i];
		if (byte >= 0xFC)
			len = 6;
		else if (byte >= 0xF8)
			len = 5;
		else if (byte >= 0xF0)
			len = 4;
		else if (byte >= 0xE0)
			len = 3;
		else if (byte >= 0xC0)
			len = 2;
		else
			len = 1;


		count += (len > 1 ? 2 : 1);

		if (count > charLen)
			chatMsg = chatMsg.substr(0, i);
	}
}

bool CChatCommon::isHoliday(const string& sDate)
{

	string arrHoliday[]={"20180101","20180215","20180216","20180217","20180218","20180219",
	                     "20180220","20180221","20180405","20180406","20180407","20180430",
						 "20180501","20180618","20180924","20181001","20181002","20181003",
						 "20181004","20181005"
	                     };

/*
	vector<string> vecHoliday={"20180101","20180215","20180216","20180217"};
	for(string n : vecHoliday) 
	{
		std::cout << n << '\n';
	}
*/
	size_t s_count=sizeof(arrHoliday)/sizeof(string);

	vector<string> vHoliday(arrHoliday,arrHoliday+s_count);

	vector<string>::iterator it=std::find(vHoliday.begin(),vHoliday.end(),sDate);

	if(it!=vHoliday.end())
	{
		return true;
	}
	else
	{
		return false;
	}
}

int CChatCommon::CaculateWeekDay(int y, int m, int d)  
{  
	if (m == 1 || m == 2) {  
		m += 12;  
		y--;  
	}  
	int iWeek = (d + 2 * m + 3 * (m + 1) / 5 + y + y / 4 - y / 100 + y / 400) % 7;  

	return iWeek;
	/*switch (iWeek)  
	{  
	case 0: cout << "星期一" << endl; break;  
	case 1: cout << "星期二" << endl; break;  
	case 2: cout << "星期三" << endl; break;  
	case 3: cout << "星期四" << endl; break;  
	case 4: cout << "星期五" << endl; break;  
	case 5: cout << "星期六" << endl; break;  
	case 6: cout << "星期日" << endl; break;  
	}  */
}  


//时间戳转字符串%Y%m%d格式
int CChatCommon::unixTime2dateString(const long lUnixTime, string& sStringDate)
{
	struct tm *p;  
	time_t t=lUnixTime;
	p=gmtime(&t);  
	char s[100];  
	strftime(s, sizeof(s), "%Y%m%d", p);  

	sStringDate=s;

	return 0;
}

//字符串转时间戳
int CChatCommon::dateString2unixTime(const string& sStringDate, long &lUnixTime)
{
	struct tm tmp_time;;  
	strptime(sStringDate.c_str(),"%Y%m%d%H%M%S",&tmp_time);  
	lUnixTime = mktime(&tmp_time);  
	printf("%ld\n",lUnixTime);  

	return 0;  
}


bool CChatCommon::isWeekEndAndHoliday()
{
	
	time_t timep;
	struct tm *p;
	time(&timep);
	p = localtime(&timep);//取得当地时间
	printf ("%d%d%d ",(1900+p->tm_year),(1+p->tm_mon), p->tm_mday);

	if (p->tm_wday==0||p->tm_wday==6)
	{
		return true;
	}

	string year,month,day;

	stringstream ss;
	ss<<1900+p->tm_year;

	year=ss.str();

	ss.str(""); 
	ss<<p->tm_mon+1;
	if (p->tm_mon<9)
	{		
		month="0"+ss.str();
	}
	else
	{
		month=ss.str();
	}

	ss.str(""); 
	ss<<p->tm_mday;
	if (p->tm_mday<10)
	{		
		day="0"+ss.str();
	}
	else
	{
		day=ss.str();
	}
	
	cout<< year<<"|"<<month<<"|"<<day<<endl;

	return isHoliday(year+month+day);

	
}
