/*
  FileName: CLogThread.h
  Date:2011年11月10日
  by LiuYongsheng

  DESC:All server logs are written by this thread,that can promote the effect of the program.
  Because this thread only handle the IO request,which does not occupy the cpu time.
*/

#ifndef _LOG_THREAD_H_
#define _LOG_THREAD_H_

#include "SL_Queue.h"
#include "SL_Log.h"
#include "SL_ByteBuffer.h"
#include "SL_Sync_Condition.h"
#include "SL_Thread.h"

#define sys_log      CLogThread::Instance()->print
#define log_debug0   SL_Log::LOG_LEVEL_0
#define log_debug1   SL_Log::LOG_LEVEL_1
#define log_debug    SL_Log::LOG_LEVEL_2
#define log_normal   SL_Log::LOG_LEVEL_3
#define log_info     SL_Log::LOG_LEVEL_4
#define log_warning  SL_Log::LOG_LEVEL_5
#define log_error0   SL_Log::LOG_LEVEL_6
#define log_error    SL_Log::LOG_LEVEL_7
#define log_fatal    SL_Log::LOG_LEVEL_8

#define LOG_PRINT(level, format, args...)\
	switch(level){ \
	case log_debug0: \
	case log_debug1: \
	case log_debug: \
		CLogThread::Instance()->print(SL_Log::LOG_LEVEL_1, (char*)"[DEBUG][%s %s:%d] " format "\n", __FILE__, __func__, __LINE__, ##args); \
		break; \
	case log_normal: \
		CLogThread::Instance()->print(SL_Log::LOG_LEVEL_3, (char*)"[NORMAL][%s %s:%d] " format "\n", __FILE__, __func__, __LINE__, ##args); \
		break; \
	case log_info: \
		CLogThread::Instance()->print(SL_Log::LOG_LEVEL_4, (char*)"[INFO][%s %s:%d] " format "\n", __FILE__, __func__, __LINE__, ##args); \
		break; \
	case log_warning: \
		CLogThread::Instance()->print(SL_Log::LOG_LEVEL_5, (char*)"[WARNING][%s %s:%d] " format "\n", __FILE__, __func__, __LINE__, ##args); \
		break; \
	case log_error0: \
	case log_error: \
		CLogThread::Instance()->print(SL_Log::LOG_LEVEL_6, (char*)"[ERROR][%s %s:%d] " format "\n", __FILE__, __func__, __LINE__, ##args); \
		break; \
	case log_fatal: \
		CLogThread::Instance()->print(SL_Log::LOG_LEVEL_8, (char*)"[FATAL][%s %s:%d] " format "\n", __FILE__, __func__, __LINE__, ##args); \
		break; \
	}

#define LOG_PRINT_TH(level, format, args...) LOG_PRINT(level, "[%lu]" format, pthread_self(), ##args)

typedef struct tag_logentry
{
	SL_Log::LOG_LEVEL	level;
	SL_ByteBuffer		buf;
	tag_logentry():level(SL_Log::LOG_LEVEL_0) {};
	tag_logentry(const tag_logentry&o):level(o.level),buf(o.buf) {};
}logentry_t;

class CLogThread
{
public:
	~CLogThread();

	void set_config(SL_Log *log)
	{
		m_log = log;
		m_work_thread.start(work_proc,this);
	}
	static CLogThread * Instance(bool use_syslog = false);
	static CLogThread * Instance(SL_Log *log, bool use_syslog = false);
	static void reset() { s_instance = NULL; }

	int push_back(logentry_t entry);
	int print(SL_Log::LOG_LEVEL level,char *fmt,...);

	void stop() {m_work_thread.stop();m_work_thread.join();}

protected:
	CLogThread(bool use_syslog = false);
	CLogThread(const CLogThread &o, bool use_syslog = false);
	CLogThread(SL_Log *log, bool use_syslog = false);
	CLogThread & operator=(const CLogThread &o);

protected:
	SL_Log  *m_log;
	SL_Queue<logentry_t,SL_Sync_ThreadMutex>	m_queue;
	SL_Sync_Condition		m_condition;
	//工作线程
	SL_Thread<SL_Sync_ThreadMutex> m_work_thread;
	static bool m_use_syslog;
	static	CLogThread *s_instance;
#ifdef SOCKETLITE_OS_WINDOWS
	static unsigned int WINAPI work_proc(void *arg);
#else
	static void* work_proc(void *arg);
#endif
	
private:
};

#endif

