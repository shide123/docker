#include "SL_Socket_TcpClient_AutoConnect.h"
#include "SL_Socket_CommonAPI.h"

SL_Socket_TcpClient_AutoConnect::SL_Socket_TcpClient_AutoConnect()
{
}

SL_Socket_TcpClient_AutoConnect::~SL_Socket_TcpClient_AutoConnect()
{
}

int SL_Socket_TcpClient_AutoConnect::open(uint check_interval)
{
    if (check_interval == 0)
    {
        check_interval_ = 5000;
    }
    else
    {
        check_interval_ = check_interval;
    }
    tcpclient_set_.clear();
    return autoconnect_thread_.start(SL_Socket_TcpClient_AutoConnect::autoconnect_proc, this);
}

int SL_Socket_TcpClient_AutoConnect::close()
{
    autoconnect_thread_.stop();
    autoconnect_thread_.join();
    return 0;
}

int SL_Socket_TcpClient_AutoConnect::add_tcpclient(SL_Socket_TcpClient *tcpclient)
{
    mutex_.lock();
    tcpclient_set_.insert(tcpclient);
    mutex_.unlock();
    return 0;
}

int SL_Socket_TcpClient_AutoConnect::del_tcpclient(SL_Socket_TcpClient *tcpclient)
{
    mutex_.lock();
    tcpclient_set_.erase(tcpclient);
    mutex_.unlock();
    return 0;
}

int SL_Socket_TcpClient_AutoConnect::have_tcpclient(SL_Socket_TcpClient *tcpclient)
{
	int found = 0;
	mutex_.lock();
	std::set<SL_Socket_TcpClient*>::iterator iter = tcpclient_set_.find(tcpclient);
	if(iter != tcpclient_set_.end())
		found = 1;
	mutex_.unlock();
	return found;
}

#ifdef SOCKETLITE_OS_WINDOWS
unsigned int WINAPI SL_Socket_TcpClient_AutoConnect::autoconnect_proc(void *arg)
#else
void* SL_Socket_TcpClient_AutoConnect::autoconnect_proc(void *arg)
#endif
{
    SL_Socket_TcpClient_AutoConnect *autoconnect = (SL_Socket_TcpClient_AutoConnect*)arg;
    SL_Socket_TcpClient *tcpclient;
    std::set<SL_Socket_TcpClient*>::iterator iter;
    std::set<SL_Socket_TcpClient*>::iterator temp_iter;

    while (1)
    {
        if (!autoconnect->autoconnect_thread_.get_running())
        {
            return 0;
        }

        iter = autoconnect->tcpclient_set_.begin();
        while (iter != autoconnect->tcpclient_set_.end())
        {
            if (!autoconnect->autoconnect_thread_.get_running())
            {
                return 0;
            }

            tcpclient = *iter;
            if (!tcpclient->get_is_autoconnect())
            {
                temp_iter = iter++;

                autoconnect->mutex_.lock();
                autoconnect->tcpclient_set_.erase(temp_iter);
                autoconnect->mutex_.unlock();
                continue;
            }

            if (!tcpclient->get_is_connected())
            {
				//printf("begin connect...\n");
                if (tcpclient->get_socket_handler()->handle_connecting() >= 0)
                {
                    if(tcpclient->connect() != 0)
					{
						//connect error
						tcpclient->get_socket_handler()->handle_connecterr();
					}
                }
            }
            ++iter;
        }
        SL_Socket_CommonAPI::util_sleep(autoconnect->check_interval_);
    }
    return 0;
}
