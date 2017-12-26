#include "SL_Socket_CommonAPI.h"
#include "SL_OS_API.h"
#include <time.h>

SL_Socket_CommonAPI::SL_Socket_CommonAPI()
{
}

SL_Socket_CommonAPI::~SL_Socket_CommonAPI()
{
}

int SL_Socket_CommonAPI::socket_init(int version_high, int version_low)
{
#ifdef SOCKETLITE_OS_WINDOWS
	WSADATA wsaData;
    WORD version_requested = MAKEWORD(version_high, version_low);
	return WSAStartup(version_requested, &wsaData);
#endif
	return 0;
}

int SL_Socket_CommonAPI::socket_fini()
{
#ifdef SOCKETLITE_OS_WINDOWS
	return WSACleanup();
#endif
	return 0;
}

int SL_Socket_CommonAPI::socket_get_lasterror()
{
#ifdef SOCKETLITE_OS_WINDOWS
	return GetLastError();
#else
	return errno;
#endif
}

int SL_Socket_CommonAPI::socket_get_lasterror_wsa()
{
#ifdef SOCKETLITE_OS_WINDOWS
	return WSAGetLastError();
#else
	return errno;
#endif
}

int SL_Socket_CommonAPI::socket_get_error(SL_SOCKET fd)
{
	int socket_error = 0;
	int socket_error_len =  sizeof(int);
	if (getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)&socket_error, (socklen_t*)&socket_error_len) < 0)
    {
        return -1;
    }
	return socket_error;
}

SL_SOCKET SL_Socket_CommonAPI::socket_open(int address_family, int type, int protocol)
{
	return socket(address_family, type, protocol);
}

int SL_Socket_CommonAPI::socket_close(SL_SOCKET fd)
{
    if ((fd < 0) || (fd == SL_INVALID_SOCKET))
    {
        return -1;
    }
#ifdef SOCKETLITE_OS_WINDOWS
	return closesocket(fd);
#else
	return close(fd);
#endif
}

int SL_Socket_CommonAPI::socket_shutdown(SL_SOCKET fd, int how)
{
    if ((fd < 0) || (fd == SL_INVALID_SOCKET))
    {
        return -1;
    }
	return shutdown(fd, how);
}

SL_SOCKET SL_Socket_CommonAPI::socket_accept(SL_SOCKET fd, struct sockaddr *addr, int *addrlen)
{
#ifdef SOCKETLITE_OS_WINDOWS
	return WSAAccept(fd, addr, addrlen, NULL, 0);
#else
	return accept(fd, addr, (socklen_t *)addrlen);
#endif
}

int SL_Socket_CommonAPI::socket_bind(SL_SOCKET fd, const struct sockaddr *addr, int addrlen)
{
    return bind(fd, addr, addrlen);
}

int SL_Socket_CommonAPI::socket_listen(SL_SOCKET fd, int backlog)
{
	return listen(fd, backlog);
}

int SL_Socket_CommonAPI::socket_connect(SL_SOCKET fd, const struct sockaddr *addr, int addrlen)
{
	return connect(fd, addr, addrlen);
}

int SL_Socket_CommonAPI::socket_connect(SL_SOCKET fd, const struct sockaddr *addr, int addrlen,  timeval *timeout)
{
	if (NULL == timeout)
    {
        return ::connect(fd, addr, addrlen);
    }
	else
	{
		SL_Socket_CommonAPI::socket_set_block(fd, false);
        int res = connect(fd, addr, addrlen);
        if (res == 0)
        {
            return 0;
        }
        if (res == SL_SOCKET_ERROR) 
        {
            fd_set read_set;
            fd_set write_set;
		    FD_ZERO(&read_set);
		    FD_ZERO(&write_set);
		    FD_SET(fd, &read_set);
            FD_SET(fd, &write_set);
#ifdef SOCKETLITE_OS_WINDOWS
            int n = ::select(0, &read_set, &write_set, NULL , timeout);
#else
            int n = ::select(fd+1, &read_set, &write_set, NULL, timeout);
#endif
		    if (n <= 0)
            {
			    return -1;
            }
		    if (FD_ISSET(fd, &read_set) || FD_ISSET(fd, &write_set))
		    {
                if (socket_get_error(fd) == 0)
                {
                    return 0;
                }
		    }
        }
	}
    return -1;
}

int SL_Socket_CommonAPI::socket_set_block(SL_SOCKET fd, bool block)
{
#ifdef SOCKETLITE_OS_WINDOWS
	{
		unsigned long arg = (block ? 0:1);
        return ::ioctlsocket(fd, FIONBIO, &arg);
	}
#else
    //方法一
    int flags = ::fcntl(fd, F_GETFL);
	if (block)
	{
        flags &= ~O_NONBLOCK;
	}
	else
	{
		flags |= O_NONBLOCK;
	}
    if (::fcntl(fd, F_SETFL, flags) == -1) 
	{
		return -1;
	}
    return 0;

    //方法二
    //int arg = (block ? 0:1);
    //return ::ioctl(fd, FIONBIO, &arg);
#endif
}

int SL_Socket_CommonAPI::socket_set_tcpnodelay(SL_SOCKET fd, int flag)
{
    return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int));
}

int SL_Socket_CommonAPI::socket_set_keepalive(SL_SOCKET fd, int flag)
{
    return setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&flag, sizeof(int));
}

int SL_Socket_CommonAPI::socket_set_keepalivevalue(SL_SOCKET fd, ulong keepalivetime, ulong keepaliveinterval)
{
#ifdef SOCKETLITE_OS_WINDOWS 
#if (_WIN32_WINNT>=0x0500)	//window 2000 or later version
	tcp_keepalive in_keepalive	 = {0};
    tcp_keepalive out_keepalive  = {0};
	ulong inlen		= sizeof(tcp_keepalive);
	ulong outlen	= sizeof(tcp_keepalive);
	ulong bytesreturn = 0;
     
    //设置socket的keepalivetime,keepaliveinterval
	in_keepalive.onoff = 1;
    in_keepalive.keepalivetime = keepalivetime;
    in_keepalive.keepaliveinterval = keepaliveinterval;
                   
    //为选定的SOCKET设置Keep Alive，成功后SOCKET可通过KeepAlive自动检测连接是否断开 
    if ( SOCKET_ERROR == WSAIoctl(fd, SIO_KEEPALIVE_VALS,(LPVOID)&in_keepalive, inlen,(LPVOID)&out_keepalive, outlen, &bytesreturn, NULL, NULL) )
    {
		return -1;
    }
#endif
#elif defined(SOCKETLITE_OS_LINUX)
    ///* set first test time */ 
    //setsockopt(fd, SOL_TCP, TCP_KEEPIDLE , (const char*)&iIdleTime , sizeof(iIdleTime)); 
    ///* set test idle time */ 
    //setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, (const char*)&iInterval, sizeof(iInterval)); 
    ///* set test count */ 
    //setsockopt(fd, SOL_TCP, TCP_KEEPCNT, (const char*)&iCount, sizeof(iCount)); 
#endif
    return 0;
}

int SL_Socket_CommonAPI::socket_set_reuseaddr(SL_SOCKET fd, int flag)
{
	return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(int));
}

int SL_Socket_CommonAPI::socket_set_broadcast(SL_SOCKET fd, int flag)
{
	return setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (char*)&flag, sizeof(int));
}

int SL_Socket_CommonAPI::socket_set_send_buffersize(SL_SOCKET fd, int sz)
{
    return setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&sz, sizeof(int));
}

int SL_Socket_CommonAPI::socket_get_send_buffersize(SL_SOCKET fd)
{
    int sz  = 0;
    int len = sizeof(int);
    if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&sz, (socklen_t *)&len) == SL_SOCKET_ERROR)
    {
		return -1;
	}
	return sz;
}

int SL_Socket_CommonAPI::socket_get_recv_buffersize(SL_SOCKET fd)
{
    int sz  = 0;
    int len = sizeof(int);
    if (getsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&sz, (socklen_t *)&len) == SL_SOCKET_ERROR)
    {
        return -1;
    }
    return sz;
}

int SL_Socket_CommonAPI::socket_set_recv_buffersize(SL_SOCKET fd, int sz)
{
    return setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&sz, sizeof(int));
}

int SL_Socket_CommonAPI::socket_set_recv_timeout(SL_SOCKET fd, int timeout)
{
	return setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(int));
}

int SL_Socket_CommonAPI::socket_set_send_timeout(SL_SOCKET fd, int timeout)
{
    return setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(int));
}

int SL_Socket_CommonAPI::socket_set_linger(SL_SOCKET fd, int onoff, int lingertime)
{
	linger l;
	l.l_onoff  = onoff;
	l.l_linger = lingertime;
	return setsockopt(fd, SOL_SOCKET, SO_LINGER, reinterpret_cast<char*>(&l), sizeof(l));
}

int SL_Socket_CommonAPI::socket_recv(SL_SOCKET fd, char *buf, int len, int flags, SL_OVERLAPPED *overlapped, int *error)
{
	int	   res;
#ifdef SOCKETLITE_OS_WINDOWS
	long   bytes_recv = 0;
	WSABUF wsabuf;
	wsabuf.len = len;
	wsabuf.buf = (char*)buf;
	res = WSARecv(fd, &wsabuf, 1, (ulong*)&bytes_recv, (LPDWORD)&flags, overlapped, 0);
	if (res < 0)
    {
		if (error != 0)
        {
			*error = socket_get_lasterror_wsa();
        }
        return res;
    }
	return bytes_recv;
#else 
	res = recv(fd, buf, len, flags);
	if (res < 0)
    {
		if (error != 0)
        {
			*error = socket_get_lasterror_wsa();
        }
    }
    return res;
#endif
}

int SL_Socket_CommonAPI::socket_recvn(SL_SOCKET fd, char *buf, int len, int flags, SL_OVERLAPPED *overlapped, int *error)
{
	long bytes_recv = 0;
	int  res;
	while (bytes_recv < len )
	{
		res = recv(fd, buf+bytes_recv, len-bytes_recv, flags);
		if (res < 0)
		{
			if (error != 0)
            {
				*error = socket_get_lasterror_wsa();
            }
			return bytes_recv;
		}
		else
		{
			bytes_recv += res;
		}
	}
	return bytes_recv;
}

int SL_Socket_CommonAPI::socket_recvv(SL_SOCKET fd, SL_IOVEC *iov, int iovcnt, int flags, SL_OVERLAPPED *overlapped, int *error)
{
	int	 res;
#ifdef SOCKETLITE_OS_WINDOWS
	long bytes_recv = 0;
	res = WSARecv(fd, (WSABUF*)iov, iovcnt, (ulong*)&bytes_recv, (LPDWORD)&flags, overlapped, 0);
	if (res < 0)
    {
		if (error != 0)
        {
			*error = socket_get_lasterror_wsa();
        }
        return res;
    }
	return bytes_recv;
#else
	res = readv(fd, iov, iovcnt);
	if (res < 0)
    {
		if (error != 0)
        {
			*error = socket_get_lasterror_wsa();
        }
    }
    return res;
#endif
}

int SL_Socket_CommonAPI::socket_send(SL_SOCKET fd, const char *buf, int len, int flags, SL_OVERLAPPED *overlapped, int *error)
{
    int    res;
#ifdef SOCKETLITE_OS_WINDOWS
    long   bytes_send = 0;
    WSABUF wsabuf;
    wsabuf.len = len;
    wsabuf.buf = (char*)buf;
    res = WSASend(fd, &wsabuf, 1, (ulong*)&bytes_send, flags, overlapped, 0);
    if (res < 0)
    {
        if (error != 0)
        {
            *error = socket_get_lasterror_wsa();
        }
        return res;
    }
    return bytes_send;
#else 
    res = send(fd, buf, len, flags);
    if (res < 0)
    {
        if (error != 0)
        {
            *error = socket_get_lasterror_wsa();
        }
    }
    return res;
#endif
}

int SL_Socket_CommonAPI::socket_sendn(SL_SOCKET fd, const char *buf, int len, int flags, SL_OVERLAPPED *overlapped, int *error)
{
    long bytes_send = 0;
    int  res;
    while (bytes_send < len )
    {
        res = send(fd, buf+bytes_send, len-bytes_send, flags);
        if (res < 0)
        {
            if (error != 0)
            {
                *error = socket_get_lasterror_wsa();
            }
            return bytes_send;
        }
        else
        {
            bytes_send += res;
        }
    }
    return bytes_send;
}

int SL_Socket_CommonAPI::socket_sendv(SL_SOCKET fd, const SL_IOVEC *iov, int iovcnt, int flags, SL_OVERLAPPED *overlapped, int *error)
{
    int   res;
#ifdef SOCKETLITE_OS_WINDOWS
    long  bytes_send = 0;
    res = WSASend(fd, (WSABUF*)iov, iovcnt, (ulong*)&bytes_send, flags, overlapped, 0);
    if (res < 0)
    {
        if (error != 0)
        {
            *error = socket_get_lasterror_wsa();
        }
        return res;
    }
    return bytes_send;
#else
    res = writev(fd, iov, iovcnt);
    if (res < 0)
    {
        if (error != 0)
        {
            *error = socket_get_lasterror_wsa();
        }
    }
    return res;
#endif
}

int SL_Socket_CommonAPI::socket_recvfrom(SL_SOCKET fd, char *buf, int len, int flags, struct sockaddr *addr, int *addrlen, SL_OVERLAPPED *overlapped, int *error)
{
    int		res;
#ifdef SOCKETLITE_OS_WINDOWS
    long   bytes_recv = 0;
    WSABUF wsabuf;
    wsabuf.len = len;
    wsabuf.buf = (char*)buf;
    res = WSARecvFrom(fd, &wsabuf, 1, (ulong*)&bytes_recv, (LPDWORD)&flags, addr, addrlen, overlapped, 0);
    if (res < 0)
    {
        if (error != 0)
        {
            *error = socket_get_lasterror_wsa();
        }
        return res;
    }
    return bytes_recv;
#else 
    res = recvfrom(fd, buf, len, flags, addr, (socklen_t *)addrlen);
    if (res < 0)
    {
        if (error != 0)
        {
            *error = socket_get_lasterror_wsa();
        }
    }
    return res;
#endif
}

int SL_Socket_CommonAPI::socket_sendto(SL_SOCKET  fd, const char *buf, int len, int flags, const struct sockaddr *destaddr, int destlen, SL_OVERLAPPED *overlapped, int *error)
{
	int    res;
#ifdef SOCKETLITE_OS_WINDOWS
	long   bytes_send = 0;
	WSABUF wsabuf;
	wsabuf.len = len;
	wsabuf.buf = (char*)buf;
    res = WSASendTo(fd, &wsabuf, 1, (ulong*)&bytes_send, flags, destaddr, destlen, overlapped, 0);
	if (res < 0)
    {
		if (error != 0)
        {
		    *error = socket_get_lasterror_wsa();
        }
        return res;
    }
	return bytes_send;
#else 
	res = sendto(fd, buf, len, flags, destaddr, destlen);
	if (res < 0)
    {
		if (error != 0)
        {
		    *error = socket_get_lasterror_wsa();
        }
    }
    return res;
#endif
}

void SL_Socket_CommonAPI::util_sleep(ulong timeout)
{
#ifdef SOCKETLITE_OS_WINDOWS
    Sleep(timeout);
#else
    //timeout: 毫秒ms

    //方法一
    usleep(timeout*1000);

    //方法二
    //struct timespec ts;
    //ts.tv_sec	= timeout/1000;
    //ts.tv_nsec	= (timeout%1000)*1000000L;
    //nanosleep(&ts, 0);
#endif
}

int64 SL_Socket_CommonAPI::util_atoi64(const char *str)
{
    //方法一
    //_atoi64(atoll)

    //方法二
    //_strtoi64(strtoll)

#ifdef SOCKETLITE_OS_WINDOWS
    return _atoi64(str);
#else
    return atoll(str);
#endif
}

uint64 SL_Socket_CommonAPI::util_atoui64(const char *str)
{
#ifdef SOCKETLITE_OS_WINDOWS
    return _strtoui64(str, NULL, 10);
#else
    return strtoull(str, NULL, 10);
#endif
}

int SL_Socket_CommonAPI::util_i64toa(int64 i, char *str, int radix)
{   
#ifdef SOCKETLITE_OS_WINDOWS
    _i64toa(i, str, radix);
#else
    //lltoa(i, str, radix);
    sprintf(str, "%lld", i);
#endif
    return 0;
}

int SL_Socket_CommonAPI::util_ui64toa(uint64 i, char *str, int radix)
{
#ifdef SOCKETLITE_OS_WINDOWS
    _ui64toa(i, str, radix);
#else
    //ulltoa(i, str, radix);
    sprintf(str, "%llu", i);
#endif
    return 0;
}

int SL_Socket_CommonAPI::util_get_application_path(char *path, int path_len)
{
	int pos = 0;
#ifdef SOCKETLITE_OS_WINDOWS
	int real_len = GetModuleFileNameA(NULL, path, path_len);
	for (pos=real_len; pos>0; --pos)
	{
		if ('\\' == path[pos])
        {
			break;
        }
	}
#elif defined(SOCKETLITE_OS_LINUX)
    char proc_path[128] = {0};
    sprintf(proc_path, "/proc/%d/exe", getpid());
    int real_len = readlink(proc_path, path, path_len);
	for (pos=real_len; pos>0; --pos)
	{
		if ('/' == path[pos])
        {
		    break;
        }
	}
#endif
    if (path_len > pos)
    {
	    path[pos+1] = 0;
    }
    return pos;
}

int SL_Socket_CommonAPI::util_get_application_name(char *app_name, int name_len)
{
	int real_len = 0;
#ifdef SOCKETLITE_OS_WINDOWS
	real_len = GetModuleFileNameA(NULL, app_name, name_len);
#elif defined(SOCKETLITE_OS_LINUX)
    char proc_path[128] = {0};
    sprintf(proc_path, "/proc/%d/exe", getpid());
    real_len = readlink(proc_path, app_name, name_len);
#endif
    if (name_len > real_len)
    {
	    app_name[real_len+1] = 0;
    }
    return real_len;
}

ulong SL_Socket_CommonAPI::util_time()
{
    return time(NULL);
}

int SL_Socket_CommonAPI::util_gettimeofday(struct timeval *tv, struct timezone *tz)
{
#ifdef SOCKETLITE_OS_WINDOWS
    struct _timeb tb;
    _ftime (&tb);

    tv->tv_sec  = (long)tb.time;
    tv->tv_usec = ((long)tb.millitm) * 1000;
#else
    gettimeofday(tv, tz);
#endif

    return 0;
}

void SL_Socket_CommonAPI::util_timeradd(struct timeval *tvp, struct timeval *uvp, struct timeval *vvp)
{
	vvp->tv_sec  = tvp->tv_sec + uvp->tv_sec;
	vvp->tv_usec = tvp->tv_usec + uvp->tv_usec;
	if (vvp->tv_usec >= 1000000)
    {
	    ++vvp->tv_sec;
		vvp->tv_usec -= 1000000;
	}
}

void SL_Socket_CommonAPI::util_timersub(struct timeval *tvp, struct timeval *uvp, struct timeval *vvp)
{
    vvp->tv_sec  = tvp->tv_sec - uvp->tv_sec;
    vvp->tv_usec = tvp->tv_usec - uvp->tv_usec;
    if (vvp->tv_usec < 0)
    {
	    --vvp->tv_sec;
	    vvp->tv_usec += 1000000;
    }
}

int SL_Socket_CommonAPI::util_timercmp(struct timeval *tvp, struct timeval *uvp)
{
    if (tvp->tv_sec > uvp->tv_sec)
    {
        return 1;
    }
    else
    {
        if (tvp->tv_sec < uvp->tv_sec)
        {
            return -1;
        }
        if (tvp->tv_usec > uvp->tv_usec)
        {
            return 1;
        }
        else if (tvp->tv_usec < uvp->tv_usec)
        {
            return -1;
        }
    }
    return 0;
}

bool SL_Socket_CommonAPI::util_timerisset(struct timeval *tvp)
{
    return (tvp->tv_sec || tvp->tv_usec);
}

void SL_Socket_CommonAPI::util_timerclear(struct timeval *tvp)
{
    tvp->tv_sec = tvp->tv_usec = 0;
}

ulong SL_Socket_CommonAPI::util_process_clock()
{
#ifdef SOCKETLITE_OS_WINDOWS
    return ::GetTickCount();
#else
    struct tms process_clock;
    return times(&process_clock);
#endif
}

ulong SL_Socket_CommonAPI::util_process_clock_ms()
{
#ifdef SOCKETLITE_OS_WINDOWS
    return ::GetTickCount();
#else

    //方法(1) 
    //static long clock_tick = 0;
    //if (clock_tick <= 0)
    //{
    //    clock_tick = sysconf(_SC_CLK_TCK);
    //}
    //ulong real_clock;
    //struct tms process_clock;
    //real_clock = times(&process_clock);
    //return real_clock*1000/clock_tick;

    //方法(2) clock

    //方法(3)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec*1000 + tv.tv_usec/1000);
#endif
}

long SL_Socket_CommonAPI::util_process_clock_difference(ulong end_clock, ulong start_clock)
{
#ifdef SOCKETLITE_OS_WINDOWS
    return (end_clock-start_clock);
#else
    static long clock_tick = 0;
    if (clock_tick <= 0)
    {
        clock_tick = sysconf(_SC_CLK_TCK);
    }
    return (end_clock-start_clock)*1000/clock_tick;
#endif
}

void* SL_Socket_CommonAPI::util_memcpy(void *dest, const void *src, size_t n)
{
    void *ret = dest;

    int unit_len = sizeof(int64);
    int64 *dest_align = (int64*)dest;
    int64 *src_align  = (int64*)src;
    for (; n >= unit_len; n -= unit_len)
    {   
        *dest_align++ = *src_align++;
    }
    
    char *dest_char =(char*)dest_align;
    char *src_char =(char*)src_align;
    for (unit_len=n, n=0; n<unit_len; ++n)
    {
        *dest_char++ = *src_char++;
    }
    return ret; 
}

//内存拷贝
void* SL_Socket_CommonAPI::util_memcpy_char(void *dest, const void *src, size_t n)
{
    void *ret = dest;

    char *char_dest = (char*)dest;
    char *char_src  = (char*)src;
    while (n--) 
    {
        *char_dest++ = *char_src++;
    }
    return ret;
}

bool SL_Socket_CommonAPI::util_is_exists_file(const char *pathname)
{
    return SL_OS_API::futil_exists(pathname);
}

int SL_Socket_CommonAPI::util_get_file_size(const char *filename, unsigned long &filesize)
{
    return SL_OS_API::futil_filesize(filename, filesize);
}

int SL_Socket_CommonAPI::util_get_errno()
{
    return errno;
}
