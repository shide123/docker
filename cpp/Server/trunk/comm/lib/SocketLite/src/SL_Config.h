// Copyright (c) 2007-2010, Bolide Zhang <bolidezhang@gmail.com>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SOCKETLITE_CONFIG_H
#define SOCKETLITE_CONFIG_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	#pragma once
#endif

#define SOCKETLITE_MAJOR_VERSION        1
#define SOCKETLITE_MINOR_VERSION        5
#define SOCKETLITE_SUB_MINOR_VERSION    1
#define SOCKETLITE_VERSION_STRING       "1.5.1"

#define DATA_BUFSIZE                    8192

#ifdef _DEBUG
    #define SOCKETLITE_DEBUG
#else
    #define SOCKETLITE_NDEBUG
#endif

//os type
//#define SOCKETLITE_OS_LINUX
//#define SOCKETLITE_OS_WINDOWS
//#define SOCKETLITE_OS_FREEBSD

//compiler type
//#define SOCKETLIET_CC_MSVC
//#define SOCKETLITE_CC_GCC
//#define SOCKETlITE_CC_INTEL

//application type
//#define SOCKETLITE_HAVE_DLL
//#define SOCKETLITE_HAVE_STATICLIB

//dll type declare
//#define SOCKETLITE_DLL_EXPORT
//#define SOCKETLITE_DLL_IMPORT

//IPV6
#define SOCKETLITE_HAVE_IPV6

//c++ TR
//#define SOCKETLITE_USE_CPLUSPLUS_TR1
//#define SOCKETLITE_USE_CPLUSPLUS_TR2

//STL 实现提供商
//#define SOCKETLITE_STL_MS
//#define SOCKETLITE_STL_SGI
//#define SOCKETLITE_STL_STLPORT
//#define SOCKETLITE_APACHE_STDCXX

//STL Extension
#define SOCKETLITE_USE_STLEXTENSION

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32)
    #ifndef _WIN32_WINNT
	    #define _WIN32_WINNT 0x0500
    #endif
    #ifndef WINVER
	    #define WINVER 0x0500
    #endif
	#define SOCKETLITE_OS_WINDOWS
    #define SOCKETLITE_CC_VC
    #define SOCKETlITE_STL_MS
#else
    #define SOCKETLITE_OS_LINUX
    #define SOCKETLITE_CC_GCC
    #define SOCKETlITE_STL_SGI
#endif
#if defined(_WIN32) 
	#define SOCKETLITE_OS_WIN32
#endif
#if defined(_WIN64) 
	#define SOCKETLITE_OS_WIN64
#endif

#ifdef SOCKETLITE_OS_WINDOWS
	#ifdef SOCKETLITE_HAS_DLL
		#ifdef SL_DLL_EXPORT
			#define SOCKETITE_API __declspec(dllexport)
		#else
			#define SOCKETLITE_API __declspec(dllimport)
		#endif
	#else
		#define SOCKETLITE_API
	#endif

	#define FD_SETSIZE 1024
	// Winsock库版本(默认winsock2)
	#if !defined(SOCKETLITE_USE_WINSOCK2)
		#define SOCKETLITE_USE_WINSOCK2 1
	#endif
	#if defined (SOCKETLITE_USE_WINSOCK2) && (SOCKETLITE_USE_WINSOCK2 != 0)
		#define SOCKETLITE_WINSOCK_VERSION 2, 2
		// will also include windows.h, if not present
		#include <winsock2.h>
		#include <ws2tcpip.h>
		#if (_WIN32_WINNT<0x0500)   //earlier versions of Windows 2000
            #include <wspiapi.h>
		#endif
	#else
		#define SOCKETLITE_WINSOCK_VERSION 1, 0
		#include <winsock.h>
	#endif
	#pragma comment(lib, "ws2_32.lib")
	#if (_WIN32_WINNT>=0x0500)  //window 2000 or later version
		#include <mstcpip.h>
	#endif

	// 微软扩展SOCKET API
	#if !defined(_MSWSOCK_)
		#include <mswsock.h>
		#include <nspapi.h>
		#pragma comment(lib, "mswsock.lib")
	#endif

	//常见头文件
	#include <errno.h>
    #include <sys/stat.h>
	#include <sys/types.h>

	typedef HANDLE				SL_HANDLE;
	typedef SOCKET				SL_SOCKET;
	typedef OVERLAPPED			SL_OVERLAPPED;

	struct SL_IOVEC
	{
		u_long	iov_len;
		char*   iov_base;
	};

	#define SL_SOCKET_ERROR		SOCKET_ERROR
	#define SL_INVALID_SOCKET	INVALID_SOCKET
    #define SL_SHUT_RD          SD_RECEIVE
    #define SL_SHUT_WR          SD_SEND
    #define SL_SHUT_RDWR        SD_BOTH

	//一些常见Socket错误
	#define SL_IO_PENDING       WSA_IO_PENDING
	#define SL_EAGAIN           WSAEWOULDBLOCK
	#define SL_EWOULDBLOCK      WSAEWOULDBLOCK
	#define	SL_ENOBUFS			WSAENOBUFS
	#define SL_EINVAL			WSAEINVAL
    #define SL_EINTR            WSAEINTR
    #define SL_WSAENOTSOCK      WSAENOTSOCK
    #define SL_EISCONN          WSAEISCONN
    #define SL_ENOTCONN         WSAENOTCONN
    #define SL_ESHUTDOWN        WSAESHUTDOWN
    #define SL_EMSGSIZE         WSAEMSGSIZE
#else
    #include <string.h>
	#include <errno.h>
	#include <unistd.h>
	#include <fcntl.h>
    #include <sys/stat.h>
	#include <sys/types.h>
	#include <sys/uio.h>
	#include <sys/socket.h>
	#include <sys/times.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
	#include <netdb.h>
	#include <arpa/inet.h>
    #include <pthread.h>
    #include <signal.h>

	typedef int		SL_HANDLE;
	typedef int		SL_SOCKET;
	typedef iovec	SL_IOVEC;
	typedef void	SL_OVERLAPPED;

	#define SOCKETLITE_API
	#define SOCKETLITE_HAVE_PTHREADS
	#define SL_SOCKET_ERROR		(-1)
	#define SL_INVALID_SOCKET	(-1)
    #define SL_SHUT_RD          SHUT_RD
    #define SL_SHUT_WR          SHUT_WR
    #define SL_SHUT_RDWR        SHUT_RDWR

	//一些常见Socket错误
	#define SL_IO_PENDING       EAGAIN
	#define SL_EAGAIN           EAGAIN
	#define SL_EWOULDBLOCK      EWOULDBLOCK
	#define	SL_ENOBUFS			ENOBUFS
	#define SL_EINVAL			EINVAL
    #define SL_EINTR            EINTR
    #define SL_WSAENOTSOCK      ENOTSOCK
    #define SL_EISCONN          EISCONN
    #define SL_ENOTCONN         ENOTCONN
    #define SL_ESHUTDOWN        ESHUTDOWN
    #define SL_EMSGSIZE         EMSGSIZE
#endif

#ifdef SOCKETLITE_OS_LINUX
#endif
#ifdef SOCKETLITE_OS_FREEBSD
#endif

#ifndef SOCKETLITE_USE_CPLUSPLUS_TR1
    #ifdef SOCKETlITE_STL_MS
        #include <hash_map>
        #include <hash_set>
        #define SL_STDEXT_NAMESPACE stdext
        #define SL_HASH_REHASH(c,n) {}
    #else
        #ifdef SOCKETLITE_CC_GCC
            #include <ext/hash_map>
            #include <ext/hash_set>
        #else
            #include <hash_map>
            #include <hash_set>
        #endif
        #define SL_STDEXT_NAMESPACE __gnu_cxx
        #define SL_HASH_REHASH(c,n) c.resize(n)
    #endif
    #define SL_HASH_MAP         SL_STDEXT_NAMESPACE::hash_map
    #define SL_HASH_SET         SL_STDEXT_NAMESPACE::hash_set
    #define SL_HASH_MULTIMAP    SL_STDEXT_NAMESPACE::hash_multimap
    #define SL_HASH_MULTISET    SL_STDEXT_NAMESPACE::hash_multiset
#else
    #include <unordered_map>
    #include <unordered_set>
    #define SL_HASH_MAP         std::tr1::unordered_map
    #define SL_HASH_SET         std::tr1::unordered_set
    #define SL_HASH_MULTIMAP    std::tr1::unordered_multimap
    #define SL_HASH_MULTISET    std::tr1::unordered_multiset
    #define SL_HASH_REHASH(c,n) c.rehash(n)
#endif

#include <stdio.h>
#include <stdlib.h>
#ifdef SOCKETLITE_OS_WINDOWS
    #include <tchar.h>
    #define SL_T(x)         _T(x)
    #define SL_TSPRINTF     _stprintf
    #define SL_TFOPEN       _tfopen
    #define SL_TFPRINTF     _ftprintf
    #define SL_TVFPRINTF    _vftprintf
#else
    #define SL_T(x)         x
    #define SL_TSPRINTF     sprintf
    #define SL_TFOPEN       fopen
    #define SL_TFPRINTF     fprintf
    #define SL_TVFPRINTF    vfprintf
#endif

#include "SL_DataType.h"
#endif

