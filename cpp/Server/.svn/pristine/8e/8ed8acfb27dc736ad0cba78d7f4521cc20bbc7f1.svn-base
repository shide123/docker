#include "timestamp.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef WIN32
#include "com/config_win32.h"
#endif


time_t toNumericTimestamp(const char *ts)
{
    //format "2003-08-08 08:08:08"
    int  year, month, day, hour, minute, second;
    struct tm tm;
	sscanf(ts, "%d-%d-%d %d:%d:%d", 
		&year,
		&month,
		&day,
		&hour,
		&minute,
		&second);
	
    tm.tm_year = year - 1900;
    tm.tm_mon  = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min  = minute;
    tm.tm_sec  = second;
    return mktime(&tm);
//  return NumericTime(DateTime(str));
}

time_t toNumericDate(const char *ts)
{
	//format "2003-08-08"
	int  year, month, day;
	struct tm tm;
	sscanf(ts, "%d-%d-%d", 
		&year,
		&month,
		&day);

	tm.tm_year = year - 1900;
	tm.tm_mon  = month - 1;
	tm.tm_mday = day;
	tm.tm_hour = 0;
	tm.tm_min  = 0;
	tm.tm_sec  = 0;

	return mktime(&tm);
}

void toStringTimestamp(time_t t, char *ts)
{
    struct tm *tm = localtime(&t);
    struct timeval  tv;
    struct timezone tz;
    char            diff;
    gettimeofday(&tv, &tz);
    
    if(tz.tz_minuteswest < 0){
	diff = '+';
    } else {
	diff = '-';
    }

    //sprintf(ts, 
	//    "%d-%d-%d %d:%d:%d%c%d",
	//    tm->tm_year + 1900,
	//    tm->tm_mon  + 1,
	//    tm->tm_mday,
	//    tm->tm_hour,
	//    tm->tm_min,
	//    tm->tm_sec,
	//    diff,
	//    tzone/*abs(tz.tz_minuteswest/60)*/);

	sprintf(ts, 
		    "%d-%d-%d %d:%d:%d",
		    tm->tm_year + 1900,
		    tm->tm_mon  + 1,
		    tm->tm_mday,
		    tm->tm_hour,
		    tm->tm_min,
		    tm->tm_sec);
}

void toStringTimestamp2(time_t t, char* ts)
{
	struct tm *tm = localtime(&t); 

	sprintf(ts, 
		"%d-%d-%d 00:00:00",
		tm->tm_year + 1900,
		tm->tm_mon  + 1,
		tm->tm_mday);
}

void toStringDate(time_t t, char *ts)
{
	struct tm *tm = localtime(&t);
	
	sprintf(ts, 
		"%d-%02d-%02d",
		tm->tm_year + 1900,
		tm->tm_mon  + 1,
		tm->tm_mday);
}

void toStringTimestamp3(time_t t, char* ts)
{
    struct tm *tm = localtime(&t);
	sprintf(ts,
		    "%d%02d%02d%02d%02d%02d",
		    tm->tm_year + 1900,
		    tm->tm_mon  + 1,
		    tm->tm_mday,
		    tm->tm_hour,
		    tm->tm_min,
		    tm->tm_sec);
}

void toStringTimestamp4(time_t t, char* ts)
{
    struct tm *tm = localtime(&t);
	sprintf(ts,
		    "%d%02d%02d",
		    tm->tm_year + 1900,
		    tm->tm_mon  + 1,
		    tm->tm_mday);
}

void toStringTimestamp5(time_t t, char* ts)
{
    struct tm *tm = localtime(&t);
	sprintf(ts,
		    "%d-%02d-%02d %02d:%02d:%02d",
		    tm->tm_year + 1900,
		    tm->tm_mon  + 1,
		    tm->tm_mday,
		    tm->tm_hour,
		    tm->tm_min,
		    tm->tm_sec);
}

void toStringTimestamp6(struct timeval *tv, char* ts)
{
	struct tm *tm = localtime(&(tv->tv_sec));
	sprintf(ts,
		    "%d-%02d-%02d %02d:%02d:%02d.%03d",
		    tm->tm_year + 1900,
		    tm->tm_mon  + 1,
		    tm->tm_mday,
		    tm->tm_hour,
		    tm->tm_min,
		    tm->tm_sec,
			tv->tv_usec/1000);
}

void toStringTimestamp7(time_t t, char* ts)
{
	struct tm *tm = localtime(&t);
	sprintf(ts,
		"%02d:%02d:%02d",
		tm->tm_hour,
		tm->tm_min,
		tm->tm_sec);
}
