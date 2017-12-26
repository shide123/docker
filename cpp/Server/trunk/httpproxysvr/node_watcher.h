#ifndef NODE_WATCHER_H_
#define NODE_WATCHER_H_

#include "epoll_socket.h"
#include "message_vchat.h"
#include "node_client.h"
#include <boost/thread.hpp>

//#include "CThriftSvrMgr.h"
using namespace std;

struct ServerIpInfo
{
	string sServerName;
	string sIp;
	int    iPort;
};

class NodeEpollWatcher : public EpollSocketWatcher 
{
private:
	
public:

	NodeEpollWatcher() 
	{
		m_svrname="httpproxysvr";
		m_nRecvLeftLen=0;
		memset(m_szRecvBuffer,0,en_msgbuffersize);
	};
	virtual ~NodeEpollWatcher() {}



	virtual int on_readable(int &epollfd, epoll_event &event) ;

	//virtual void on_timer(const boost::system::error_code& /*e*/) ;

	virtual int on_writeable(EpollContext &epoll_context){} ;
	virtual int on_close(EpollContext &epoll_context) ;
	virtual int on_accept(EpollContext&){};

	int  handleMessage(int fd,const char* pdata, int msglen);

	uint64  getKey(const int32 & ip, const int32 & port);
	int32   getip(uint64 key);
	int32   getport(uint64 key);

	static  bool getServerInfoByServerName(const string &sServerName,ServerIpInfo & tServerIpInfo);

	std::string u322ip(const uint32 ip);
	template<typename T> int  getIpPort(T *svr,std::string &sIp,int &iPort);
	void onNodeEvent(const std::string & svrname, const std::string & ip, int port, INodeNotify::en_node_event event, std::string desc /*= ""*/);
	std::string m_svrname;
	static std::map<string,map<string,ServerIpInfo> > m_sSubRpcServer;
	char            m_szRecvBuffer[en_msgbuffersize];
	int             m_nRecvLeftLen;

	static  boost::mutex    m_mutex;

};


#endif /* NODE_WATCHER_H_ */