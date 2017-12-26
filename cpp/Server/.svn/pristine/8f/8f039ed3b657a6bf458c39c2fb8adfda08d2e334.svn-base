/*
 * fileinotify.h
 *
 *  Created on: 2016年3月11日
 *      Author: shuisheng
 */

#ifndef FILEINOTIFY_H_
#define FILEINOTIFY_H_
#include <SL_Headers.h>

class fileinotify {
public:
	fileinotify();
	virtual ~fileinotify();

	void startup(const char* path);
protected:
	static void* file_inotify_proc(void* arg);
private:
	SL_Thread<SL_Sync_ThreadMutex> monitorthread;
	char *path_;
};

#endif /* FILEINOTIFY_H_ */
