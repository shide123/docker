#include "CLogThread.h"
#include "syslog-nb.h"
#include <stdarg.h>

CLogThread *CLogThread::s_instance = NULL;
bool CLogThread::m_use_syslog = false;
CLogThread * CLogThread::Instance(bool use_syslog)
{
	if (!s_instance)
	{
		s_instance = new CLogThread();
	}
	
	if(use_syslog)
	{
		openlog_nb(NULL, LOG_PID, LOG_LOCAL0);
		m_use_syslog = use_syslog;
	}
	return s_instance;
}

CLogThread * CLogThread::Instance(SL_Log *log, bool use_syslog)
{
	if (!s_instance)
	{
		s_instance = new CLogThread();
		s_instance->set_config(log);
	}
	
	if(use_syslog)
	{
		openlog_nb(NULL, LOG_PID, LOG_LOCAL0);
		m_use_syslog = use_syslog;
	}
	return s_instance;
}


CLogThread::CLogThread(bool use_syslog):m_log(NULL)
{
	CLogThread::m_use_syslog = use_syslog;
	if(use_syslog)
	{
		openlog_nb(NULL, LOG_PID, LOG_LOCAL0);
	}
}

CLogThread::CLogThread(SL_Log *log, bool use_syslog):m_log(log)
{
	CLogThread::m_use_syslog = use_syslog;
	
	if(use_syslog)
	{
		openlog_nb(NULL, LOG_PID, LOG_LOCAL0);
	}
}

CLogThread::~CLogThread()
{
	if(m_use_syslog)
	{
		closelog_nb();
	}
}

int CLogThread::push_back(logentry_t entry)
{
	int res = m_queue.push_back(entry);
	m_condition.signal();
	return res;
}

int CLogThread::print(SL_Log::LOG_LEVEL level,char *fmt,...)
{
	if(m_log == 0 || 
		m_log->get_level() > level || 
		m_log->get_mode() == SL_Log::LOG_MODE_NO_WRITE) {
		if (!m_log)
			fprintf(stderr, "failed to print log, please make sure log instance initialize correctly.");
		return 1;
	}
	
	if(!m_use_syslog)
	{
		SL_ByteBuffer buf;
		if (!buf.reserve(2048)){
			perror("malloc memory failed");
			return 0;
		}

		va_list ap;
		va_start(ap, fmt);
		vsnprintf(buf.buffer(), 2048-1, fmt, ap);
		va_end(ap);
		logentry_t entry;
		entry.level = level;
		entry.buf = buf;

		push_back(entry);
	}
	else
	{
		int syslog_level = 0;
		//turn level to syslog_level
		if(level >= log_debug0 && level <= log_debug)
		{
			syslog_level = LOG_DEBUG;
		}
		else if(level == log_normal)
		{
			syslog_level = LOG_INFO;
		}
		else if(level == log_info)
		{
			syslog_level = LOG_NOTICE;
		}
		else if(level == log_warning)
		{
			syslog_level = LOG_WARNING;
		}
		else if(level >= log_error0 && level <= log_error)
		{
			syslog_level = LOG_ERR;
		}
		else if(level == log_fatal)
		{
			syslog_level = LOG_CRIT;
		}

		va_list ap;
		va_start(ap, fmt);
		vsyslog_nb(level, fmt, ap);
		va_end(ap);
	}
	return 1;
}


#ifdef SOCKETLITE_OS_WINDOWS
unsigned int WINAPI CLogThread::work_proc(void *arg)
#else
void * CLogThread::work_proc(void * arg)
#endif
{
	CLogThread *log_thread = (CLogThread *)arg;
	while(1)
	{
		if( !log_thread->m_work_thread.get_running())
		{
			break;
		}
		logentry_t entry;
		if(log_thread->m_queue.pop_front(entry) < 0)
		{
			(log_thread->m_queue.mutex()).lock();
			log_thread->m_condition.wait(&(log_thread->m_queue.mutex()));
			(log_thread->m_queue.mutex()).unlock();
		}
		
		if (log_thread->m_log != NULL)
		{
			log_thread->m_log->print(entry.level,entry.buf.data());
		}
		else
		{
			//printf("%s",entry.buf.data());
		}
	}
	return 0;
}


