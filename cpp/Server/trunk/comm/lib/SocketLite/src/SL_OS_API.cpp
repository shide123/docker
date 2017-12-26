#include "SL_OS_API.h"
#ifdef SOCKETLITE_OS_WINDOWS

#include <io.h>
#include <direct.h>
#include "Shlwapi.h"
#include <stdarg.h>    // for va_list, va_start, va_end
#pragma comment(lib, "shlwapi.lib")

// Calls the windows _vsnprintf, but always NUL-terminate.
int snprintf(char *str, size_t size, const char *format, ...) 
{
    if (0 == size)          // not even room for a \0?
    {
        return -1;          // not what C99 says to do, but what windows does
    }
    str[size-1] = '\0';
    va_list ap;
    va_start(ap, format);
    const int rc = _vsnprintf(str, size-1, format, ap);
    va_end(ap);
    return rc;
}

#endif

SL_OS_API::SL_OS_API()
{
}

SL_OS_API::~SL_OS_API()
{
}

int SL_OS_API::futil_stat(const char *filename, struct stat *buf)
{
    return stat(filename, buf);
}

int SL_OS_API::futil_access(const char *pathname, int mode)
{
#ifdef SOCKETLITE_OS_WINDOWS
    return ::_access(pathname, mode);
#else
    return ::access(pathname, mode);
#endif
}

int SL_OS_API::futil_chmod(const char *pathname, int mode)
{
#ifdef SOCKETLITE_OS_WINDOWS
    return ::_chmod(pathname, mode);
#else
    return ::chmod(pathname, mode);
#endif
}

int SL_OS_API::futil_rename(const char *oldname, const char *newname)
{
    return ::rename(oldname, newname);
}

int SL_OS_API::futil_remove(const char *filename)
{
    return ::remove(filename);
}

int SL_OS_API::futil_unlink(const char *filename)
{
#ifdef SOCKETLITE_OS_WINDOWS
    return ::_unlink(filename);
#else
    return ::unlink(filename);
#endif
}

int SL_OS_API::futil_chdir(const char *dirname)
{
#ifdef SOCKETLITE_OS_WINDOWS
    return ::_chdir(dirname);
#else
    return ::chdir(dirname);
#endif
}

int SL_OS_API::futil_mkdir(const char *dirname, int mode)
{
#ifdef SOCKETLITE_OS_WINDOWS
    return ::_mkdir(dirname);
#else
    return ::mkdir(dirname, mode);
#endif
}

int SL_OS_API::futil_rmdir(const char *dirname)
{
#ifdef SOCKETLITE_OS_WINDOWS
    return ::_rmdir(dirname);
#else
    return ::rmdir(dirname);
#endif
}

bool SL_OS_API::futil_exists(const char *pathname)
{
#ifdef SOCKETLITE_OS_WINDOWS
    //方法一
    return ::PathFileExistsA(pathname);

    //方法二 access,_taccess(_access/_waccess)
    //return access(pathname, 0);
#else
    int rc = access(pathname, F_OK);
    if (rc < 0)
    {
        return false;
    }
    return true;
#endif
}

int SL_OS_API::futil_filesize(const char *filename, unsigned long &filesize)
{
#ifdef SOCKETLITE_OS_WINDOWS
    //方法一:
    //HANDLE handle = ::CreateFileA(file_name, FILE_READ_EA, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    //if (handle != INVALID_HANDLE_VALUE)
    //{
    //    file_size = GetFileSize(handle, NULL);
    //    ::CloseHandle(handle);
    //    return 0;
    //}
    //return -1;

    //方法二 stat,_tstat(_stat/_wstat)
    //Get data associated with the file descriptor.
    struct stat statbuf;
    int ret     = stat(filename, &statbuf);
    filesize    = statbuf.st_size;
    return ret;
#else
    struct stat statbuf;    
    int ret     = stat(filename, &statbuf);
    filesize    = statbuf.st_size;
    return ret;
#endif
}

int SL_OS_API::futil_filelength(const char *filename, unsigned long &filesize)
{
    return futil_filesize(filename, filesize);
}
