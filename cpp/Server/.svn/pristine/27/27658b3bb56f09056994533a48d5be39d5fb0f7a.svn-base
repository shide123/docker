#ifndef SOCKETLITE_TEST_LOG_H
#define SOCKETLITE_TEST_LOG_H
#include "SL_Config.h"
#include "SL_OS_File.h"
#include <fcntl.h>
#include <sys/stat.h>

class SL_Test_Log
{
public:
    //log模式
    enum LOG_MODE
    {
        LOG_MODE_NO_WRITE,      //不写日志
        LOG_MODE_TO_STDOUT,     //记录日志到stdout
        LOG_MODE_TO_STDERR,     //记录日志到stderr
        LOG_MODE_TO_FILE        //记录日志到文件
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
        LOG_LEVEL_LAST          //最高级别
    };

    SL_Test_Log()
        : log_mode_(LOG_MODE_TO_STDOUT)
        , log_level_(LOG_LEVEL_5)
    {
    }

    ~SL_Test_Log() 
    {
    }

    virtual int open(const char *log_filename, const char *open_mode, int log_mode, int log_level) 
    { 
        return 0; 
    }

    virtual int close() { return 0; }
    virtual int print(char *fmt, ...) { return 0; }
    virtual int print(int level, char *fmt, ...) { return 0; }
    virtual int write(const char *buf, unsigned int len) { return 0; }
    virtual int write(int level, const char *buf, unsigned int len) { return 0; }

protected:
    int     		log_mode_;  //日志模式
    int     		log_level_; //日志等级
};

class SL_Test_StdioLog : public SL_Test_Log
{
public:
    inline SL_Test_StdioLog()
        : log_file_(stdout)
    {
    }

    inline ~SL_Test_StdioLog()
    {
        close();
    }

    inline int open(const char *log_filename, 
        const char *open_mode="a+", 
        int log_mode=LOG_MODE_TO_FILE, 
        int log_level=LOG_LEVEL_5)
    {
        close();

        //设置日志模式
        if (log_mode<LOG_MODE_NO_WRITE || log_mode>LOG_MODE_TO_FILE)
        {
            log_mode_ = LOG_MODE_TO_STDOUT;
        }
        else
        {
            log_mode_ = log_mode;
        }
        switch (log_mode)
        {
        case LOG_MODE_TO_STDOUT:
            log_file_ = stdout;
            break;
        case LOG_MODE_TO_STDERR:
            log_file_ = stderr;
            break;
        default:
            break;
        }
        //设置日志等级
        if (log_level<LOG_LEVEL_0 || log_level>LOG_LEVEL_LAST)
        {
            log_level_ = LOG_LEVEL_5;
        }
        else
        {
            log_level_ = log_level;
        }

        if (NULL == log_filename)
        {
            if (LOG_MODE_TO_FILE == log_mode)
            {
                log_mode_ = LOG_MODE_TO_STDOUT;
                log_file_ = stdout;
            }
        }
        else
        {
            log_file_ = fopen(log_filename, open_mode);
            if (NULL == log_file_)
            {
                log_mode_ = LOG_MODE_TO_STDOUT;
                log_file_ = stdout;
                return -1;
            }
            log_mode_ = LOG_MODE_TO_FILE;
        }
        return 0;
    }

    inline int close()
    {
        if ((LOG_MODE_TO_FILE == log_mode_) && (NULL != log_file_))
        {
            fclose(log_file_);
            log_file_ = NULL;
        }
        return 0;
    }

    inline int print(char *fmt, ...)
    {
        va_list ap;
        va_start(ap, fmt);
        int rc = vprint(fmt,ap);
        va_end(ap);
        return rc;
    }

    inline int print(int level, char *fmt, ...)
    {
        if (log_level_ > level)
        {
            return -1;
        }
        va_list ap;
        va_start(ap, fmt);
        int rc = vprint(fmt,ap);
        va_end(ap);
        return rc;
    }

    inline int write(const char *buf, unsigned int len)
    {
        return fwrite(buf, len, 1, log_file_);
    }

    inline int write(int level, const char *buf, unsigned int len)
    {
        if (log_level_ > level)
        {
            return -1;
        }
        return fwrite(buf, len, 1, log_file_);
    }

private:
    inline int vprint(char *fmt, va_list argptr)
    {
        if (LOG_MODE_NO_WRITE == log_mode_)
        {
            return -1;
        }
        if ((log_mode_==LOG_MODE_TO_FILE) && (NULL==log_file_))
        {
            return -2;
        }
        vfprintf(log_file_, fmt, argptr);
        fflush(log_file_);
        return 0;
    }

protected:
    std::string log_fileName_;  //日志文件名
    FILE        *log_file_;      //日志文件指针
};

class SL_Test_OSFileLog : public SL_Test_Log
{
public:
    inline SL_Test_OSFileLog()
        : log_file_()
    {
    }

    inline ~SL_Test_OSFileLog()
    {
        close();
    }

    inline int open(const char *log_filename, 
        int log_oflag=O_APPEND|O_CREAT|O_RDWR, 
        int log_pmode=S_IREAD|S_IWRITE, 
        int log_mode=LOG_MODE_TO_FILE, 
        int log_level=LOG_LEVEL_5)
    {
        close();

        //设置日志模式
        if (log_mode<LOG_MODE_NO_WRITE || log_mode>LOG_MODE_TO_FILE)
        {
            log_mode_ = LOG_MODE_TO_STDOUT;
        }
        else
        {
            log_mode_ = log_mode;
        }
        switch (log_mode)
        {
        case LOG_MODE_TO_STDOUT:
            log_file_.set_fileno(SL_STDOUT_FILENO);
            break;
        case LOG_MODE_TO_STDERR:
            log_file_.set_fileno(SL_STDERR_FILENO);
            break;
        default:
            break;
        }
        //设置日志等级
        if (log_level<LOG_LEVEL_0 || log_level>LOG_LEVEL_LAST)
        {
            log_level_ = LOG_LEVEL_5;
        }
        else
        {
            log_level_ = log_level;
        }

        if (NULL == log_filename)
        {
            if (LOG_MODE_TO_FILE == log_mode)
            {
                log_mode_ = LOG_MODE_TO_STDOUT;
                log_file_.set_fileno(SL_STDOUT_FILENO);
            }
        }
        else
        {
            if (log_file_.open(log_filename, log_oflag, log_pmode) < 0)
            {
                log_mode_ = LOG_MODE_TO_STDOUT;
                log_file_.set_fileno(SL_STDOUT_FILENO);
                return -1;
            }
            log_mode_ = LOG_MODE_TO_FILE;
        }
        return 0;
    }

    inline int close()
    {
        return log_file_.close();
    }

    inline int print(char *fmt, ...)
    {
        va_list ap;
        va_start(ap, fmt);
        int rc = vprint(fmt,ap);
        va_end(ap);
        return rc;
    }

    inline int print(int level, char *fmt, ...)
    {
        if (log_level_ > level)
        {
            return -1;
        }
        va_list ap;
        va_start(ap, fmt);
        int rc = vprint(fmt,ap);
        va_end(ap);
        return rc;
    }

    inline int write(const char *buf, unsigned int len)
    {
        return log_file_.write(buf, len);
    }

    inline int write(int level, const char *buf, unsigned int len)
    {
        if (log_level_ > level)
        {
            return -1;
        }
        return log_file_.write(buf, len);
    }

private:
    inline int vprint(char *fmt, va_list argptr)
    {
        if (LOG_MODE_NO_WRITE == log_mode_)
        {
            return -1;
        }
        char buf[SL_LOG_MAX_LINE_LEN];
        buf[SL_LOG_MAX_LINE_LEN-1] = '\0';
        int rc = vsnprintf(buf, SL_LOG_MAX_LINE_LEN-1, fmt, argptr);
        if (rc <= 0)
        {
            return -1;
        }
        return log_file_.write(buf, rc);
    }

protected:
    SL_OS_File      log_file_;  //日志文件
};

#endif