/*
 * fileinotify.cpp
 *
 *  Created on: 2016年3月11日
 *      Author: shuisheng
 */

#include "fileinotify.h"
#include <sys/inotify.h>
#include "CLogThread.h"

#define EVENT_SIZE  		( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     	( 1024 * ( EVENT_SIZE + 16 ) )

#define GIFT_CONF "giftsvr.conf"
#define ROBOT_CONF "robot.conf"

fileinotify::fileinotify() :
		path_(NULL) {

}

fileinotify::~fileinotify() {
	if (path_)
		free (path_);
}

void fileinotify::startup(const char* path) {
	if (!path || 0 == *path)
		return;

	if (path_)
		free(path_);

	path_ = strdup(path);
	monitorthread.start(fileinotify::file_inotify_proc, this);
}

void* fileinotify::file_inotify_proc(void* arg) {
	int length, i;
	char buffer[EVENT_BUF_LEN];

	fileinotify *fi = (fileinotify*) arg;
	if (!fi) {
		LOG_PRINT(log_error, "invalid thread arg for file inotify");
		return NULL;
	}
	int fd = inotify_init();
	if (fd < 0) {
		LOG_PRINT(log_error, "Failed to inotify_init");
		return NULL;
	}

	int wd = inotify_add_watch(fd, fi->path_, IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVE | IN_MOVED_FROM | IN_MOVED_TO);
	while (true) {
		i = 0;
		if ((length = read(fd, buffer, EVENT_BUF_LEN)) < 0) {
			LOG_PRINT(log_error, "Failed to read data for file inotify");
			return NULL;
		}

		while (i < length) {
			struct inotify_event *event = (struct inotify_event *) &buffer[i];
			if (event->len) {
				if (event->mask & IN_CREATE) {
					if (event->mask & IN_ISDIR) {
						LOG_PRINT(log_debug, "New directory %s created.\n", event->name);
					} else {
						LOG_PRINT(log_debug, "New file %s created.\n", event->name);
					}
				} else if (event->mask & IN_DELETE) {
					if (event->mask & IN_ISDIR) {
						LOG_PRINT(log_debug, "Directory %s deleted.\n",	event->name);
					} else {
						LOG_PRINT(log_debug, "File %s deleted.\n", event->name);
					}
				} else if (event->mask & IN_MODIFY) {
					if (event->mask & IN_ISDIR) {
						LOG_PRINT(log_debug, "Directory %s modified.\n", event->name);
					} else {
						LOG_PRINT(log_debug, "File %s modified.\n", event->name);
					}
				} else if (event->mask & IN_MOVED_TO) {
					if (event->mask & IN_ISDIR) {
						LOG_PRINT(log_debug, "Directory %s moved.\n", event->name);
					}
					else {
						LOG_PRINT(log_info, "File %s moved.\n", event->name);
						if (strcasecmp(event->name, GIFT_CONF)) {

						}
						else if (strcasecmp(event->name, ROBOT_CONF)){

						}
					}
				}
			}
			i += EVENT_SIZE + event->len;
		}
	}

	inotify_rm_watch(fd, wd);
	close(fd);
}
