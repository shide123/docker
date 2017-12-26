#ifndef SOCKETLITE_OS_FILE_H
#define SOCKETLITE_OS_FILE_H
#include "SL_Config.h"
#include <string>

class SL_OS_File
{
public:
    SL_OS_File();
    ~SL_OS_File();

    int open(const char *filename, int flags, int mode);
    int creat(const char *filename, int mode);
    int close();
    int read(void *buf, unsigned int nbytes);
    int readn(void *buf, unsigned int nbytes);
    int write(const void *buf, unsigned int nbytes);
    int writen(const void *buf, unsigned int nbytes);
    int lseek(long offset, int whence);
    int flush();
    int dup();
    int dup2(int fd);

    int  get_fileno() const;
    void set_fileno(int fileno);
    const char* get_filename() const;

protected:
    int fd_;
    std::string filename_;

};

#endif
