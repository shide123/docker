/*
 * tcp_epoll.h
 *
 *  Created on: Nov 10, 2014
 *      Author: liao
 */

#ifndef TCP_EPOLL_H_
#define TCP_EPOLL_H_

#include "sys/epoll.h"
#include <vector>
#include <set>
#include <string>
#include "threadpool.h"

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#define SS_WRITE_BUFFER_SIZE 4096
#define SS_READ_BUFFER_SIZE 4096

#define WRITE_CONN_ALIVE 0
#define WRITE_CONN_CLOSE 1
#define WRITE_CONN_CONTINUE 2

#define READ_OVER 0
#define READ_CONTINUE 1
#define READ_CLOSE -1





class EpollContext {
    public:
        void *ptr;
        int fd;
        std::string client_ip;
};

typedef void (*ScheduleHandlerPtr)();

class EpollSocketWatcher {
    public:
        virtual int on_accept(EpollContext &epoll_context) = 0;

        virtual int on_readable(int &epollfd, epoll_event &event) = 0;

        /**
         * return :
         * if return value == 1, we will close the connection
         * if return value == 2, we will continue to write
         */
        virtual int on_writeable(EpollContext &epoll_context) = 0;

        virtual int on_close(EpollContext &epoll_context) = 0;

};


enum EpollSocketStatus {
    S_RUN = 0,
    S_REJECT_CONN = 1,
    S_STOP = 2
};

class EpollSocket {
    private:
        int setNonblocking(int fd);

        int accept_socket(int sockfd, std::string &client_ip);

        int bind_on(unsigned int ip);

        int listen_on();

        int handle_accept_event(int &epollfd, epoll_event &event, EpollSocketWatcher &socket_watcher);

        int handle_writeable_event(int &epollfd, epoll_event &event, EpollSocketWatcher &socket_watcher);

        int close_and_release(epoll_event &event);

        int init_default_tp();

        

        int add_listen_sock_to_epoll();

        int handle_event(epoll_event &e);

        int start_event_loop();

		
		
        std::vector<std::string> _bind_ips;
        int _backlog;
        int _max_events;
        int _port;
        int _epollfd;
        std::set<int> _listen_sockets;
        pthread_mutex_t _client_lock;
        volatile int _clients;
        
        ThreadPool *_thread_pool;
        bool _use_default_tp;
        volatile int _status;
        EpollSocketWatcher *_watcher;
    public:
        EpollSocket();
       
        ~EpollSocket();
        
		int init_tp();

        int create_epoll();

		EpollSocketWatcher *getWatcher(){return _watcher;}

        int handle_readable_event(epoll_event &event);

        int start_epoll();
        
        int stop_epoll();

        void set_thread_pool(ThreadPool *tp);

        void set_port(int port);

        void set_watcher(EpollSocketWatcher *w);

        void set_backlog(int backlog);

        void set_max_events(int max_events);

        void set_schedule(ScheduleHandlerPtr h);

        void add_bind_ip(std::string ip);

		//////////////////////////////////////////////////////////////////////////

		int handle_node_readable_event(epoll_event &event);
		int connect_remote(const std::string &sServerIp,int iPort,int & iClientFd);
		void reconnect_node();
		//void* start_client_epoll(void *);
		int add_sock_to_epoll(int sockfd);
		int handle_client_event(epoll_event &e) ;
		int start_client_event_loop() ;
		int start_client();
		static void set_nodefd(int fd){_nodefd=fd;}
		static int  get_nodefd(){return _nodefd;}
		int start_timer();
		static int tcp_send(int fd, const char *buf, int len);
		static void set_node_epoll_adr(void* arg){_node_epoll_adr=arg;}
		static void* get_node_epoll_adr(){return _node_epoll_adr;}
		static int _nodefd;
		static void* _node_epoll_adr;
		static boost::mutex m_mutex;
		//////////////////////////////////////////////////////////////////////////

};

struct TaskData {
    epoll_event event;
    EpollSocket *es;
};


#endif /* TCP_EPOLL_H_ */
