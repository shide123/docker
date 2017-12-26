#ifndef SOCKETLITE_SOCKET_COMMONAPI_H
#define SOCKETLITE_SOCKET_COMMONAPI_H

#include "SL_Config.h"
#ifdef SOCKETLITE_OS_WINDOWS
    #include <io.h>
    #include "Shlwapi.h"
    #pragma comment(lib, "shlwapi.lib")

    /* timeb.h is actually xsi legacy functionality */
    #include <sys/timeb.h>
#else
    #include <sys/time.h>
    #include <sys/ioctl.h>
#endif

class SOCKETLITE_API SL_Socket_CommonAPI
{
private:
	SL_Socket_CommonAPI();
	~SL_Socket_CommonAPI();
	
public:
	static int socket_init(int version_high, int version_low);
	static int socket_fini();
	static int socket_get_lasterror();
	static int socket_get_lasterror_wsa();
	static int socket_get_error(SL_SOCKET fd);

	static SL_SOCKET socket_open(int address_family, int type, int protocol);
	static int socket_close(SL_SOCKET fd);
	static int socket_shutdown(SL_SOCKET fd, int how);

	static SL_SOCKET socket_accept(SL_SOCKET fd, struct sockaddr *addr, int *addrlen);
	static int socket_bind(SL_SOCKET fd, const struct sockaddr *addr, int addrlen);
	static int socket_listen(SL_SOCKET fd, int backlog);
	static int socket_connect(SL_SOCKET fd, const struct sockaddr *addr, int addrlen);
	static int socket_connect(SL_SOCKET fd, const struct sockaddr *addr, int addrlen, timeval *timeout);

	static int socket_set_block(SL_SOCKET fd, bool block);
	static int socket_set_tcpnodelay(SL_SOCKET fd, int flag);
	static int socket_set_keepalive(SL_SOCKET fd, int flag);
	static int socket_set_keepalivevalue(SL_SOCKET fd, ulong keepalivetime=7200000, ulong keepaliveinterval=1000);
	static int socket_set_reuseaddr(SL_SOCKET fd, int flag);
	static int socket_set_broadcast(SL_SOCKET fd, int flag);
    static int socket_set_recv_buffersize(SL_SOCKET fd, int sz);
	static int socket_set_send_buffersize(SL_SOCKET fd, int sz);
    static int socket_set_recv_timeout(SL_SOCKET fd, int timeout);
    static int socket_set_send_timeout(SL_SOCKET fd, int timeout);
	static int socket_set_linger(SL_SOCKET fd, int onoff, int linger);
    static int socket_get_recv_buffersize(SL_SOCKET fd);
	static int socket_get_send_buffersize(SL_SOCKET fd);

	static int socket_recv(SL_SOCKET fd, char *buf, int len, int flags=0, SL_OVERLAPPED *overlapped=NULL, int *error=0);
	static int socket_recvn(SL_SOCKET fd, char *buf, int len, int flags=0, SL_OVERLAPPED *overlapped=NULL, int *error=0);
	static int socket_recvv(SL_SOCKET fd, SL_IOVEC *iov, int iovcnt, int flags=0, SL_OVERLAPPED *overlapped=NULL, int *error=0);
    static int socket_send(SL_SOCKET fd, const char *buf, int len, int flags=0, SL_OVERLAPPED *overlapped=NULL, int *error=0);
    static int socket_sendn(SL_SOCKET fd, const char *buf, int len, int flags=0, SL_OVERLAPPED *overlapped=NULL, int *error=0);
    static int socket_sendv(SL_SOCKET fd, const SL_IOVEC *iov, int iovcnt, int flags=0, SL_OVERLAPPED *overlapped=NULL, int *error=0);

    static int socket_recvfrom(SL_SOCKET fd, char *buf, int len, int flags, struct sockaddr *addr, int *addrlen, SL_OVERLAPPED *overlapped=0, int *error=0);
    static int socket_sendto(SL_SOCKET fd, const char *buf, int len, int flags, const struct sockaddr *destaddr, int destlen, SL_OVERLAPPED *overlapped=0, int *error=0);

	static void     util_sleep(ulong timeout);  //sleep以毫秒为单位 
    static int64    util_atoi64(const char *str);
    static uint64   util_atoui64(const char *str);
    static int      util_i64toa(int64 i, char *str, int radix);
    static int      util_ui64toa(uint64 i, char *str, int radix);
    static int      util_get_application_path(char *app_path, int len);
    static int      util_get_application_name(char *app_name, int len);

    //取得当前系统时间(自公元1970/1/1 00:00:00以来经过秒数)
    static ulong    util_time();

    //取得当前系统时间(自公元1970/1/1 00:00:00以来经过秒数,可以精确到微秒)
    static int      util_gettimeofday(struct timeval *tv, struct timezone *tz);

    //时间操作函数
    static void     util_timeradd(struct timeval *tvp, struct timeval *uvp, struct timeval *vvp);
    static void     util_timersub(struct timeval *tvp, struct timeval *uvp, struct timeval *vvp);
    static bool     util_timerisset(struct timeval *tvp);
    static void     util_timerclear(struct timeval *tvp);

    //时间比较函数
    //The return value for each of these functions indicates the lexicographic relation of tvp to uvp
    //<0 tvp less than uvp
    //=0 tvp identical to uvp
    //>0 tvp greater than uvp
    static int      util_timercmp(struct timeval *tvp, struct timeval *uvp);

    //取得进程时钟滴答
    static ulong    util_process_clock(); 

    //取得进程时钟滴答(毫秒:ms)
    static ulong    util_process_clock_ms();

    //两个进程时钟差(毫秒:ms)
    static long     util_process_clock_difference(ulong end, ulong start);

    //内存拷贝(使用内存对齐)
    static void* util_memcpy(void *dest, const void *src, size_t n);

    //内存拷贝
    static void* util_memcpy_char(void *dest, const void *src, size_t n);

    //检测文件是否存在
    static bool util_is_exists_file(const char *pathname);

    //取得文件大小
    static int util_get_file_size(const char *filename, unsigned long &filesize);

    //取得错误号
    static int util_get_errno();
};

#endif
