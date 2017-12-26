#ifndef __RUNTIMECLOCK_H__
#define __RUNTIMECLOCK_H__

#include <stdio.h>
#include <sys/time.h>

class CRuntimeClock
{
public:
	CRuntimeClock()
	{
		gettimeofday(&tv_begin, NULL);
	}
	~CRuntimeClock(){}

	void begin()
	{
		gettimeofday(&tv_begin, NULL);
	}
	void end()
	{
		gettimeofday(&tv_end, NULL);
	}

	long runtime_s()
	{
		if (tv_end.tv_sec <= 0)
		{
			end();
		}
		return tv_end.tv_sec - tv_begin.tv_sec;
	}

	long long runtime_ms()
	{
		if (tv_end.tv_sec <= 0)
		{
			end();
		}
		return end_ms() - begin_ms();
	}

	long long runtime_us()
	{
		if (tv_end.tv_sec <= 0)
		{
			end();
		}
		return end_us() - begin_us();
	}

	const char *runtime_ms_str(int decimal = 3)
	{
		sprintf(runtime_str, "%.*lf", decimal, (double)runtime_us()/1000.0);
		return runtime_str;
	}

private:
	char runtime_str[64];
	struct timeval tv_begin;
	struct timeval tv_end;
	unsigned long long begin_ms()
	{
		return tv_begin.tv_sec*1000 + tv_begin.tv_usec/1000;
	}
	unsigned long long end_ms()
	{
		return tv_end.tv_sec*1000 + tv_end.tv_usec/1000;
	}
	unsigned long long begin_us()
	{
		return (unsigned long long)tv_begin.tv_sec*1000*1000 + tv_begin.tv_usec;
	}
	unsigned long long end_us()
	{
		return (unsigned long long)tv_end.tv_sec*1000*1000 + tv_end.tv_usec;
	}
};

#endif
