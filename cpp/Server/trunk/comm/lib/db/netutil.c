
#include "netutil.h"

#include <stdio.h>
#include <stdlib.h>

#ifndef WIN32
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <pthread.h>
#include <signal.h>
#else
#include <WinSock2.h>
#endif

int ipchar2int(const char* ipstr)
{
	return inet_addr(ipstr);
}

void ipint2char(int ipint, char* ipstr)
{
	struct in_addr ipaddr;
	char* szip;
	if(ipstr == 0 || ipint == 0)
		return;

	ipaddr.s_addr = ipint;
	szip = inet_ntoa(ipaddr);
	strcpy(ipstr, szip);
}

