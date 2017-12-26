
#ifndef __TIMER_HH__
#define __TIMER_HH__

#include <stdarg.h>
//#include <boost/asio.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
using namespace std;

#define TIMES_UNLIMIT -1

//typedef void* (*FUNC)(void*);
//
//class taskdata{
//public:
//	FUNC callback;
//	void* userdata;
//	int timerid;
//};
typedef boost::function<void()> FUNC;
typedef struct tag_Task
{
	FUNC func;
	boost::asio::deadline_timer* m_timer;
	int interval;
	int times;
} Task;



class Timer:public boost::enable_shared_from_this<Timer>
{
public:
	Timer();
	~Timer();
	static Timer* getInstance()
	{
		static Timer t;
		return &t;
	}
	int add(int time_interval,FUNC func,int times = TIMES_UNLIMIT);
	bool del(int timerid);
	void trigger(int timerid);
private:
	typedef map<int,Task> TASK_MAP;
	void handle(int timerid);
	TASK_MAP _tasks;
	int timer_idx;

};
typedef boost::shared_ptr<Timer> Timer_ptr;
#endif //__TIMER_HH__

