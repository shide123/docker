#include "timer.h"
#include "protocols/ProtocolsBase.h"
#include "Application.h"
#include "clienthandler.h"
using namespace std;
Timer::Timer():
timer_idx(1)
{
}

Timer::~Timer()
{
	for(unsigned int i = 0 ; i < _tasks.size(); i++)
	{
		boost::system::error_code e;
//		_tasks[i].m_Timer->cancel(e);
		delete _tasks[i].m_timer;
	}
}

int Timer::add(int time_interval,FUNC func,int times)
{
	if (time_interval < 0 || times == 0) {
		printf("bad time_interval or times");
		return false;
	}
	
	boost::asio::deadline_timer* t = new boost::asio::deadline_timer(Application::get_io_service(), boost::posix_time::seconds(time_interval));
	Task task;
	task.func = func;
	task.interval = time_interval;
	task.times = times;
	task.m_timer = t;
	
	//閸旂姴鍙嗛崚浼存Е閸掞拷

	if (_tasks.empty()) {
		//
	}
	int timerid = timer_idx++;
	_tasks[timerid] = task;

	t->async_wait(boost::bind(&Timer::handle,this,timerid));
	// 閸旂姴鍙嗛崚鐧皁ost
	return timerid;
}

bool Timer::del(int Timerid)
{
	//娴犲酣妲﹂崚妤呭櫡閸掔娀娅庢禒璇插
	TASK_MAP::iterator it = _tasks.find(Timerid);
	if (it != _tasks.end())
	{
		//Task task = _tasks[timerid];
		//boost::system::error_code e;
		//	task.m_timer->cancel(e);
		boost::system::error_code e;
		it->second.m_timer->cancel(e);
		delete it->second.m_timer;
		_tasks.erase(it);
		return true;
	}

	return false;
}

void Timer::handle(int timerid)
{
	//閸欐垿锟戒焦绉烽幁顖氬煂濞戝牊浼呴梼鐔峰灙,濞夈劍鍓扮憰浣哥敨娑撳ィimerid
	task_proc_data* task_data = new task_proc_data;
	task_proc_param param ;
	param.message = MSG_TYPE_TIMER;
	param.param1 = timerid;
	param.param2 = 1;
	task_data->msgtye=TASK_MSGTYPE_TIMER;
	task_data->pdata = new char[sizeof(task_proc_param)];
	task_data->datalen = sizeof(task_proc_param);
	memcpy(task_data->pdata, &param, sizeof(task_proc_param));
	clienthandler_ptr ptr;
	task_data->connection = ptr;
//	task_data->event = NULL;
	Application::getInstance()->add_message(task_data);
}

void Timer::trigger(int timerid)
{
	//鐟欙箑褰傞梼鐔峰灙娴犺濮?
	TASK_MAP::iterator it  = _tasks.find(timerid);
	if(it != _tasks.end())
	{
		Task &task = it->second;
		if(task.func)
		{
			task.func();
		}

		//func may delete this timerid,so check it second time.
		if (_tasks.end() == _tasks.find(timerid))
		{
			return;
		}

		task.times--;
		if(task.times==0)
		{
			delete _tasks[timerid].m_timer;
			_tasks.erase(timerid);
		}
		else
		{
			boost::asio::deadline_timer* t = task.m_timer;
			t->expires_at(t->expires_at() + boost::posix_time::seconds(task.interval));
			t->async_wait(boost::bind(&Timer::handle,this,timerid));
		}
	}
}
