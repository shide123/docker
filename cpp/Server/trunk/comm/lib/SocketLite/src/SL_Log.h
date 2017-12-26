/*
* Copyright (C) 2009-2010 Cai chen qi(caitian_6@hotmail.com)
* All rights reserved.
* 
* 文件名称：SL_Log.h
* 摘    要：日志类
* 当前版本：1.0.0.0210
* 作    者：蔡承启
* 完成日期：2009/02/10
*
*/

#ifndef  SOCKETLIET_LOG_H
#define  SOCKETLIET_LOG_H

#include <stdio.h>
#include <string>
#include "SL_Sync_Mutex.h"

class SL_Log
{
public:
	//log模式
	enum LOG_MODE
	{
		LOG_MODE_NO_WRITE,      //不写日志
		LOG_MODE_TO_PRINT,      //记录日志到控制台
		LOG_MODE_TO_FILE,       //记录日志到文件
	};
	//log级别
	enum LOG_LEVEL
	{
		LOG_LEVEL_0, 
		LOG_LEVEL_1,     
		LOG_LEVEL_2,     
		LOG_LEVEL_3,
		LOG_LEVEL_4,
		LOG_LEVEL_5,
		LOG_LEVEL_6,
		LOG_LEVEL_7,
		LOG_LEVEL_8,
		LOG_LEVEL_LAST  //最高级别
	};
	
	SL_Log();
	~SL_Log();
    int get_level()
    {
        return m_nLevel;
    }
    int get_mode()
    {
        return m_nMode;
    }

	int init(int nMode=LOG_MODE_TO_PRINT, int nLevel=LOG_LEVEL_5, const char *log_file_path=NULL, 
        const char *log_file_name="log", const char *log_extension_name="log", int nIntervalHour=2);
	int print(char *fmt, ...);
	int print(int nLevel, char *fmt, ...); 
    int write(const char *buf, unsigned int len);

    static void localtime(time_t time, long timezone, struct tm *tm_time);

protected: 
	bool buildfile();
	bool closefile();                       //关闭日志文件	
	int  vprint(char *fmt, va_list argptr); //记录日志

	std::string	   m_logfileName;			//日志文件名
	std::string	   m_logextensionName;		//日志后缀名
	std::string    m_logfilePath;			//日志路径（包含全路径 和 文件名）
	std::string    m_logfileLastName;		//包括前缀的文件名
	std::string    m_logLastFullName;		//包含路径，前缀的全文件名(最后记录的)
	std::string    m_logFullName;			//包含路径，前缀的全文件名

	int		m_nLastHour;                    //最后记录的时间
	int		m_nIntervalHour;                //写日志的间隔 以小时为单位
    int		m_nMode;                        //当前日志的模式
	int		m_nLevel;                       //当前日志的等级

	FILE    *m_pFile;                       //日志文件的指针
	SL_Sync_ThreadMutex m_logLock;
	
};

#endif
