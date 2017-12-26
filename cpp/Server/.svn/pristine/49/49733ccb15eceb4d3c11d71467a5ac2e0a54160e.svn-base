#ifndef SOCKETLITE_TIMER_LIST_TIME_H
#define SOCKETLITE_TIMER_LIST_TIME_H

#include "SL_Socket_CommonAPI.h"
#include "SL_Timer.h"
#include "SL_Timer_Queue.h"
#include "SL_Sync_Mutex.h"
#include "SL_Thread.h"
#include <list>

template <typename TSyncMutex>
class SL_Timer_List_Time : public SL_Timer_Queue
{
public:
    SL_Timer_List_Time()
        : max_timer_id_(0)
    {
    }

    ~SL_Timer_List_Time()
    {
        max_timer_id_ = 0;
        clear_timer_i();
    }

    int set_config(uint interval = SL_TIMER_DEFULT_INTERVAL, uint repair = SL_TIMER_DEFAUL_REPAIR)
    {
	    interval_           = interval;
	    repair_.tv_sec      = repair / 1000;
	    repair_.tv_usec     = (repair % 1000) * 1000;
        return 0;
    }

    int add_timer(SL_Timer *timer)
    {
        mutex_.lock();
	    add_timer_i(timer);
	    timer->id_ = ++max_timer_id_;
	    mutex_.unlock();
        return 0;
    }

    int del_timer(SL_Timer *timer)
    {
        mutex_.lock();
	    del_timer_i(timer);
	    mutex_.unlock();
        return 0;
    }

    int clear_timer()
    {
        mutex_.lock();
	    clear_timer_i();
	    mutex_.unlock();
        return 0;
    }

    int timer_loop()
    {
        SL_Socket_CommonAPI::util_sleep(interval_);

	    struct timeval now, stand;   	
        SL_Socket_CommonAPI::util_gettimeofday(&now, 0);
        SL_Socket_CommonAPI::util_timeradd(&now, &repair_, &stand);

        mutex_.lock();
	    SL_Timer *temp_timer;
        std::list<SL_Timer*>::iterator iter = timer_list_.begin();
        while (iter != timer_list_.end())
	    {
            temp_timer = *iter;
			if(temp_timer ==0) {
				iter = timer_list_.erase(iter);
				continue;
			}

            if (SL_Socket_CommonAPI::util_timercmp(&stand, &temp_timer->expire_time_.endtime) >= 0)
		    {
				iter = timer_list_.erase(iter);

				temp_timer->state_ = SL_Timer::TIMER_STATE_TIMEOUT; //先做一次,不然不能在这里面add_timer_i()
                temp_timer->timeout();  //因为timeout里面会再次增加定时器,必须在上面先删除

			    if (temp_timer->type_ == SL_Timer::TIMER_TYPE_ONCE)
			    {
				    temp_timer->state_ = SL_Timer::TIMER_STATE_TIMEOUT;
			    }
			    else if (temp_timer->type_ == SL_Timer::TIMER_TYPE_CYCLE)
			    {
	                temp_timer->state_ = SL_Timer::TIMER_STATE_IDLE;
				    add_timer_i(temp_timer);
			    }
		    }
		    else
		    {
			    break;
		    }
	    }
        mutex_.unlock();
        return 0;
    }

    int thread_timer_loop()
    {
        return timer_loop_thread_.start(timer_loop_proc, this);
    }

    int thread_wait()
    {
        timer_loop_thread_.join();
        return 0;
    }

    int thread_stop()
    {
        timer_loop_thread_.stop();
        return 0;
    }

private:	
#ifdef SOCKETLITE_OS_WINDOWS
    static unsigned int WINAPI timer_loop_proc(void *arg)
#else
    static void* timer_loop_proc(void *arg)
#endif
    {
		SL_Timer_List_Time<TSyncMutex> *timer_list = (SL_Timer_List_Time<TSyncMutex>*)arg;
	    while (1)
	    {
            if (!timer_list->timer_loop_thread_.get_running())
            {
                break;
            }
            timer_list->timer_loop();
        }
        return 0;
    }

	int add_timer_i(SL_Timer *timer)
    {
	    if (timer->state_ == SL_Timer::TIMER_STATE_ALIVE)
        {
		    return -1;
        }

	    struct timeval now, interval;
	    timer->state_       = SL_Timer::TIMER_STATE_ALIVE;
	    interval.tv_sec     = timer->interval_ / 1000;
	    interval.tv_usec    = (timer->interval_ % 1000) * 1000;
        SL_Socket_CommonAPI::util_gettimeofday(&now, 0);
        SL_Socket_CommonAPI::util_timeradd(&now, &interval, &timer->expire_time_.endtime);

        if (timer_list_.empty())
        {
            timer_list_.push_back(timer);
        }
        else
        {
            SL_Timer *temp_timer;
            std::list<SL_Timer*>::iterator iter = timer_list_.begin();
            for (; iter != timer_list_.end(); ++iter)
            {
                temp_timer = *iter;
                if (temp_timer!=0 && SL_Socket_CommonAPI::util_timercmp(&timer->expire_time_.endtime, &temp_timer->expire_time_.endtime) <= 0)
                {
                    timer_list_.insert(iter, timer);
                    return 0;
                }
            }
            timer_list_.push_back(timer);
        }
        return 0;
    }

	int del_timer_i(SL_Timer *timer)
    {
		SL_Timer *temp_timer;
		std::list<SL_Timer*>::iterator iter;
		for(iter=timer_list_.begin();iter!=timer_list_.end();iter++)
		{
			temp_timer = *iter;
			if(temp_timer == timer)
			{
				(*iter)=0; //这里只是重置
				break;
			}
		}
	    timer->state_ = SL_Timer::TIMER_STATE_IDLE;
        return 0;
    }

    int clear_timer_i()
    {
        timer_list_.clear();
        return 0;
    }

    TSyncMutex              mutex_;
    std::list<SL_Timer*>    timer_list_;

	struct timeval          repair_;
	uint                    interval_;
	uint                    max_timer_id_;

    SL_Thread<SL_Sync_ThreadMutex>  timer_loop_thread_;
};

#endif
