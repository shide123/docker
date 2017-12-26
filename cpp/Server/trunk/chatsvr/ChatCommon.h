#ifndef __CCHAT_COMM_H__
#define __CCHAT_COMM_H__

#include "yc_datatypes.h"
#include <string>
#include "ChatSvr.pb.h"

#define FIRST_SEQUENCE_VALUE 1

class CChatCommon
{
public:
	CChatCommon(void);
	virtual ~CChatCommon(void);

	static std::string getPChatSeqName(uint32 srcid, uint32 dstid);

	static std::string getPChatSeqName(uint32 groupid);
	static std::string getGChatSeqName(uint32 groupid);
	static std::string getGPChatSeqName(uint32 groupid, uint32 srcid, uint32 dstid);

	static std::string getGPAssistSeqName();

	static std::string getFormatString(const char * fmt, ...);

	static bool chkPMsgValid(const ChatMsg_t & msg);
	static bool chkGMsgValid(const ChatMsg_t & msg);

	//一个中文当做2个字节，英文&数字&符号当做1个字节
	static void pickOnlookerChatUTF8(std::string &chatMsg, size_t charLen = 22);

	static bool isHoliday(const std::string& sDate);
	static int  CaculateWeekDay(int y, int m, int d);
	static bool isWeekEndAndHoliday();
	static int  unixTime2dateString(const long lUnixTime, std::string& sStringDate);
	static int  dateString2unixTime(const std::string& sStringDate, long &lUnixTime);
};

#endif //__CCHAT_COMM_H__
