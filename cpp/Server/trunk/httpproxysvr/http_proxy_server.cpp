/*
 * http_proxy_server.cpp
 *
 *  Created on: Dec 5, 2017
 *      Author: hzh
 */
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include "simple_log.h"
#include "http_server.h"
#include "threadpool.h"

#include "CLogThread.h"
#include "http_core.h"


int g_daemon=0;

void show_help()
{
	printf("Usage : stabird [-h][-v][-d]\n");
	printf("        -h Show help\n");
	printf("        -v Show version\n");
	printf("        -d start as daemon process!\n");
}

void show_version()
{
	printf("version: %s\n", "1.0");
	return;
}
void close_all_fd()
{
	uint32_t fdlimit = sysconf(_SC_OPEN_MAX);

	for (uint32_t fd=0;fd<fdlimit;fd++)
	{
		close(fd);
	}
}

int init_daemon()
{
	if (g_daemon)
	{
		switch (fork())
		{
		case 0:  break;
		case -1: return -1;
		default: exit(0);          
		}

		if (setsid() < 0)
		{
			return -1;
		}

		//leader exit,so the second child can't open shell
		switch (fork())
		{
		case 0:  break;
		case -1: return -1;
		default: exit(0);
		}

		umask(0);

		close_all_fd();
	}

	return 0;

}

void set_daemon_flag(int flag)
{
	g_daemon = flag;

	return;
}


int get_options(int argc, char* argv[])
{
	int optch;
	extern char *optarg;

	static char optstring[] = "hvd";
	while((optch = getopt(argc, argv, optstring)) != -1)
	{
		switch (optch)
		{
		case 'h':
			show_help();
			exit(0);
		case 'v':
			show_version();
			exit(0);
		case 'd':
			set_daemon_flag(1);
			break;
		}
	}

	return 0;
}

int main(int argc, char **args) 
{
    
	get_options(argc,args);

	init_daemon();
    //pthread_key_create(&g_tp_key,NULL);
    //
    //ThreadPool tp;
    //tp.set_thread_start_cb(a_test_fn);
    //tp.set_pool_size(4);

	HttpCore::init();

	HttpCore::loadBusiness();
    
	HttpCore::run();


    return 0;
}
