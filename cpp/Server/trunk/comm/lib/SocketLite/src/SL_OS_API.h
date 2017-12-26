#ifndef SOCKETLITE_OS_API_H
#define SOCKETLITE_OS_API_H
#include "SL_Config.h"

#ifdef SOCKETLITE_OS_WINDOWS

// We can't just use _snprintf as a drop-in replacement, because it
// doesn't always NUL-terminate. :-(
extern int snprintf(char *str, size_t size, const char *format, ...);

#endif

class SOCKETLITE_API SL_OS_API
{
private:
    SL_OS_API();
    ~SL_OS_API();

public:
    static int futil_stat(const char *pathname, struct stat *buf);
    static int futil_access(const char *pathname, int mode);
    static int futil_chmod(const char  *pathname, int mode);
    static int futil_rename(const char *oldname, const char *newname);
    static int futil_remove(const char *filename);
    static int futil_unlink(const char *filename);

    static int futil_chdir(const char *dirname);
    static int futil_mkdir(const char *dirname, int mode);
    static int futil_rmdir(const char *dirname);

    //检测文件是否存在
    static bool futil_exists(const char *pathname);

    //取得文件大小
    static int futil_filesize(const char *filename, unsigned long &filesize);
    static int futil_filelength(const char *filename, unsigned long &filesize);

};

#endif
