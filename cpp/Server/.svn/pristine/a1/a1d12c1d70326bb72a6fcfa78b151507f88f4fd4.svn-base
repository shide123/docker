
#include <stdarg.h>   
#include <time.h>
#include <sys/types.h>
#include "SL_Log.h"

#ifdef SOCKETLITE_OS_WINDOWS
    #include <direct.h> 
#else
    #include <sys/stat.h>
#endif

const char SL_DAYS[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
void  SL_Log::localtime(time_t time, long timezone, struct tm *tm_time)
{
	unsigned  int n32_Pass4year;
	int n32_hpery;

	//计算时差
	time=time + (timezone * 60 * 60);

	if (time < 0)
	{
		time = 0;
	}
	//取秒时间
	tm_time->tm_sec=(int)(time % 60);
	time /= 60;
	//取分钟时间
	tm_time->tm_min=(int)(time % 60);
	time /= 60;
	//取过去多少个四年，每四年有 1461*24 小时
	n32_Pass4year=((unsigned int)time / (1461L * 24L));
	//计算年份
	tm_time->tm_year=(n32_Pass4year << 2)+70;
	//四年中剩下的小时数
	time %= 1461L * 24L;
	//校正闰年影响的年份，计算一年中剩下的小时数
	for (;;)
	{
		//一年的小时数
		n32_hpery = 365 * 24;
		//判断闰年
		if ((tm_time->tm_year & 3) == 0)
		{
			//是闰年，一年则多24小时，即一天
			n32_hpery += 24;
		}
		if (time < n32_hpery)
		{
			break;
		}
		++tm_time->tm_year;
		time -= n32_hpery;
	}
	//小时数
	tm_time->tm_hour=(int)(time % 24);
	//一年中剩下的天数
	time /= 24;
	//假定为闰年
	++time;
	//校正润年的误差，计算月份，日期
	if ((tm_time->tm_year & 3) == 0)
	{
		if (time > 60)
		{
			--time;
		}
		else
		{
			if (time == 60)
			{
				tm_time->tm_mon = 1;
				tm_time->tm_mday = 29;
				return;
			}
		}
	}
	//计算月日
	for (tm_time->tm_mon=0; SL_DAYS[tm_time->tm_mon]<time; ++tm_time->tm_mon)
	{
		time -= SL_DAYS[tm_time->tm_mon];
	}
	tm_time->tm_mday = (int)(time);

	return;
}

SL_Log::SL_Log()
{
	m_pFile 	= NULL;
	m_nMode 	= LOG_MODE_TO_PRINT;
	m_nLevel 	= LOG_LEVEL_5;
	m_nIntervalHour = 2;
	m_nLastHour     = -25;
}

SL_Log::~SL_Log()
{
	if (m_nMode == LOG_MODE_TO_FILE)
    {
		closefile();
    }
		
	m_logfileName.clear();			
	m_logextensionName.clear();			
	m_logfilePath.clear();			
	m_logfileLastName.clear();		
	m_logLastFullName.clear();		
	m_logFullName.clear();			
}

int SL_Log::init(int nMode,
				  int nLevel,
                  const char *log_file_path, 
				  const char *log_file_name,
				  const char *log_extension_name,
				  int nIntervalHour)
{
	//设置日志模式
	if ((nMode<LOG_MODE_NO_WRITE) || (nMode>LOG_MODE_TO_FILE))
	{
		nMode = LOG_MODE_TO_FILE;
	}
	m_nMode = nMode;
	if (m_nMode == LOG_MODE_TO_PRINT)
	{
		m_pFile = stdout;
	}
	//设置日志等级
	if (nLevel<LOG_LEVEL_0 || nLevel>LOG_LEVEL_LAST)
	{
		m_nLevel = LOG_LEVEL_5;
	}
    else
    {
	    m_nLevel = nLevel;
    }

	//设置间隔时间
	if (nIntervalHour > 24 || nIntervalHour < 1)
	{
		nIntervalHour = 2;
	}
	m_nIntervalHour = nIntervalHour;

    if (m_nMode==LOG_MODE_TO_FILE)
    {
	    //记录日志路径，日志名，日志扩展名
	    if (log_file_path && log_file_name && log_extension_name)
	    {
		    m_logfilePath = log_file_path;
		    m_logfileName = log_file_name;
		    m_logextensionName = log_extension_name;
		    m_logFullName =  m_logfilePath + "/" + m_logfileName + "." + m_logextensionName;
	    }            
	    //创建目录
#ifdef SOCKETLITE_OS_WINDOWS
        CreateDirectoryA(m_logfilePath.c_str(), NULL);
#else
	    mkdir(m_logfilePath.c_str(), S_IRWXU);
#endif
    }
	return 0;

}

int SL_Log::vprint(char *fmt, va_list argptr)
{
	if (m_nMode == LOG_MODE_NO_WRITE)
	{
		return 0;
	}
	if (m_nMode == LOG_MODE_TO_FILE)
	{
		if(!buildfile())
		{
			return -1;
		}
	}

	m_logLock.lock();   //保证记录都能正确记录

	//取当前时间
	//time_t now_time = time(0);
    //struct tm *now_tm = this->localtime(&now_time);
    time_t now_time = time(0);
    struct tm _tm;
    this->localtime(now_time, 8, &_tm);
    struct tm *now_tm = &_tm;
    if (now_tm == NULL)
    {
        m_logLock.unlock();
        return -2;
    }

	char buf[30]={0};
	sprintf(buf, "%4d%02d%02d %02d:%02d:%02d ", now_tm->tm_year+1900, now_tm->tm_mon+1,
		now_tm->tm_mday, now_tm->tm_hour, now_tm->tm_min, now_tm->tm_sec);

	
	fprintf(m_pFile, buf);
	vfprintf(m_pFile, fmt, argptr);
	fflush(m_pFile);
	
	m_logLock.unlock(); //释放锁
	return 0;
}

int SL_Log::print(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int rc = vprint(fmt,ap);
	va_end(ap);
	return rc;
}

int SL_Log::print(int nLevel, char *fmt, ...)
{
	if (m_nLevel > nLevel)
	{
		return -1;
	}

	va_list ap;
	va_start(ap, fmt);
	int rc = vprint(fmt,ap);
	va_end(ap);
	return rc;
}

inline int SL_Log::write(const char *buf, unsigned int len)
{
    return fwrite(buf, len, 1, m_pFile);
}

bool SL_Log::buildfile()
{
    m_logLock.lock();

	//time_t now_time = time(0);
    //struct tm *now_tm = ::localtime(&now_time);

    time_t now_time = time(0);
    struct tm _tm;
    this->localtime(now_time, 8, &_tm);
    struct tm *now_tm = &_tm;

    if (now_tm == NULL)
    {
        return false;
    }

	if (abs(now_tm->tm_hour-m_nLastHour) > m_nIntervalHour)
	{//当前时间大于间隔时间
	    char szTime[30] = {0};
		m_nLastHour = now_tm->tm_hour;
		sprintf(szTime, "_%4d%02d%02d_%02d", now_tm->tm_year+1900, now_tm->tm_mon+1,
			now_tm->tm_mday, now_tm->tm_hour);
		m_logfileLastName = m_logfileName +  szTime + "." + m_logextensionName;
		m_logFullName = m_logfilePath + "/" + m_logfileLastName;
	}

	if (m_pFile == NULL)
	{
		m_pFile = fopen(m_logFullName.c_str(), "a+");
	}
	else
	{
		if (m_logFullName != m_logLastFullName)
		{//产生新的Log文件
			closefile();
			m_logLastFullName = m_logFullName;
			m_pFile = fopen(m_logFullName.c_str(), "a+");
		}
	}

    m_logLock.unlock();
	if (m_pFile == NULL)
	{
		return false;
	}
	return true;
}

bool SL_Log::closefile()
{
    if ((LOG_MODE_TO_FILE == m_nMode) && (NULL != m_pFile))
    {
        if (!fclose(m_pFile))
        {
            return false;
        }
        m_pFile = NULL;
    }
	return true;
}
