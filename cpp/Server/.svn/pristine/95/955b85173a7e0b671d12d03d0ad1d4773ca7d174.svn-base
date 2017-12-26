#include "node_client.h"

#include <boost/thread.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

#include "CLogThread.h"

boost::asio::io_service * gios;
bool bexit = false;

void sigfunc(int signo)
{
    if (signo == SIGUSR1) {
        bexit = true;
    }
}

void ftime(const boost::system::error_code& e, boost::asio::deadline_timer* t)
{ 
    if (bexit) {        
        gios->stop();
        return;
    }
    //printf("ftime...!\n");
    t->expires_at(t->expires_at() + boost::posix_time::seconds(10));
	t->async_wait(boost::bind(ftime, 
		boost::asio::placeholders::error, 
		t));
}


class CNodeNotify
    : public INodeNotify
{
public:
    virtual ~CNodeNotify(void) {};

    /*
     * call by nodeclient.
     * need handle, create connection to ip&port if event equals add.
     * otherwise disconnect connection.
     *
     * @svrname     server name. register in nodesvr.
     * @ip          server ip.
     * @port        server port.
     * @event       enumurate, 'add' or 'rem'.
     */
    virtual void onNodeEvent(const std::string & svrname, const std::string & ip, 
        int port, en_node_event event)
    {
        printf("client-->node event, svr:%s, ip:%s, port:%d, event:%d\n",
            svrname.c_str(), ip.c_str(), port, event);
    }
};

int main(int argc, char * argv[])
{
    signal(SIGUSR1, sigfunc);

    if (argc < 4) {
        printf("Usage: %s svrname port desc\n");
        return 0;
    }

    SL_Log log;
    log.init(SL_Log::LOG_MODE_TO_PRINT, SL_Log::LOG_LEVEL_0);
    CLogThread::Instance(&log, false);
    
    gios = new boost::asio::io_service();
    boost::asio::io_service & ios = * gios;

    std::string svrname(argv[1]);//     = "testsvr";
    int         hostport    = atoi(argv[2]);

    // 1. create client object.
    /*
     * @svrname     register name in nodesvr.
     * @ios         io_service. nodesvr net-op thread.
     * @notify      callback.
     * @hostip      current process acceptor bind ip.
     * @hostport    current process acceptor listened port.
     */
    INodeClient * client = INodeClient::CreateObject(argv[1], ios, NULL,
        "", atoi(argv[2]), argv[4] == NULL ? "" : argv[4]);

    // 2. subscribe svr.  such as "roomsvr,gateway,"
    client->subscribe(argv[3]);

    // 3. add nodesvr node. read from configure.
    //client->set_address("127.0.0.1:9999;127.0.0.1:10000;127.0.0.1:10001;");
    client->set_address("127.0.0.1:9999;");

    // 4. set alarm info
    client->set_alarm_param("127.0.0.1", 8887, 40);

    // 5. connect
	client->connect();

    boost::asio::deadline_timer timer(ios, boost::posix_time::seconds(5));
	timer.async_wait(boost::bind(ftime,
		boost::asio::placeholders::error,
		&timer));  

    boost::thread thread(boost::bind(&boost::asio::io_service::run, &ios));
    thread.join();

    delete client;

    delete gios;
    delete CLogThread::Instance();

    sleep(3);
    
    return 0;
}
