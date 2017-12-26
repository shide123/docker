#ifndef __CLIENT_HANDLER_H__
#define __CLIENT_HANDLER_H__

//std header file
#include <string>
#include <map>
#include <vector>
using namespace std;
//boost header file
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
using boost::asio::ip::tcp;
using boost::asio::deadline_timer;

#include "SL_ByteBuffer.h"
#include "SL_Sync_Mutex.h"
#include "CLogThread.h"
#include "message_comm.h"
#include "message_vchat.h"

enum {
	en_session_status_unkown    = 0,
	en_session_status_connected = 1,
	en_session_status_ready     = 2,
	en_session_status_needclose = 3,
};
class tcpserver;
class ProtocolsBase;
class IWorkerEvent;
class clienthandler;
typedef boost::shared_ptr<clienthandler> clienthandler_ptr;
const string mask = "d914632f7b5c0e8a";
class clienthandler : public boost::enable_shared_from_this<clienthandler>
{
friend class ProtocolsBase;
friend class Websocket;
public:

	clienthandler(boost::asio::io_service & ioservice, std::string protocol, IWorkerEvent * event,tcpserver* svr = NULL);

    virtual ~clienthandler();

    int open();

	time_t getLastalivetime() const;

	bool isconnected() const { return connected; }
    void setconnid(uint32_t connid) { m_connid = connid; }
    uint32_t getconnid() const { return m_connid; }
    const char* getremote_ip() const { return remote_ip; }
	ushort getremote_port() const { return remote_port; }
	unsigned int getsendqueue_size() const { return m_send_queue.size(); }
	ProtocolsBase *getProtocol(){ return m_protocol; }

	void handle_write_loop(const boost::system::error_code& e);

    enum {ERR_READ = 1, ERR_WRITE, ERR_PACKET};

    void setEvent(IWorkerEvent* event){m_workerEvent = event;}
    virtual void write_message(SL_ByteBuffer& message, bool bforced = true);
    virtual void write_message(const char* pdata, int datalen, bool bforced = true);
    void handle_write(const boost::system::error_code& e, std::size_t bytes_transferred);
    void send_message(SL_ByteBuffer& message, bool bforced);

	virtual void stop();
	virtual void release();
	void start_connect(const char *addr, u_int16_t port);

	void resperrinf(COM_MSG_HEADER * reqHead, ClientGateMask_t * pMask, uint16 errcode);
	void setRemoteSvrIp(const char* ip,unsigned short port);
	void transKey(const string& s,string& d);//only s = md5
	void sendAuthPassmsg();

	void addclientid(unsigned int client_connid);
	void delclientid(unsigned int client_connid);
	unsigned int getclientid_size();
	void getallclientid(std::set<unsigned int> & clientid_set);

	void setgateid(uint16 idgate) { m_ngateid = idgate; }
	uint16 getgateid() const { return m_ngateid; }

	unsigned int getsvrtype() const { return m_svrtype; }
	void setsvrtype(unsigned int svrtype) {	m_svrtype = svrtype; }
protected:
	virtual void cancel();
	virtual void close();
    void handle_read(const boost::system::error_code & e, std::size_t bytes_transferred);

    int build_NetMsg_Svr(char* szBuf,int nBufLen,int mainCmdId, int subCmdId, void* pData, int pDataLen);

    int build_NetMsg_Client(char* szBuf,int nBufLen,int mainCmdId, int subCmdId, void* pData, int pDataLen);

	void handle_connect(const boost::system::error_code& e);
	void handle_timeout(const boost::system::error_code& e);
	int connect();
	void clear_data();
	void sendAuthmsg();

	void addTaskMessage(int msgType, int subType, int errid = 0, const std::string &msg = std::string());
	void onError(int code, const std::string &msg = std::string());

public:
    tcp::socket socket_;
    deadline_timer deadline_;
    deadline_timer writetimer;
    std::vector<SL_ByteBuffer> m_vbuffer;
    boost::mutex messagelstmtx;
    int session_status_;
	uint16_t svrid;
	time_t lastalivetime;
	bool bSayHello;

	tcp::endpoint endpoint_;
	unsigned int last_activetime_;
	int disconn_report_times;

    /*userdata*/
    uint32_t user_id;
    uint32_t room_id;
    void* user_data;
    IWorkerEvent* 		m_workerEvent;

	string m_skey;
	string m_transkey;
	bool m_bAuthPass;

protected:

	//read message buffer.
	char recv_buf_[en_msgbuffersize];
	int  recv_buf_remainlen_;

	std::deque<SL_ByteBuffer > m_send_queue;
	boost::mutex send_messages_mutex_;

	bool connected;
	uint32_t m_connid;
	char    remote_ip[30];
	ushort  remote_port;
	tcpserver  *serv;
	ProtocolsBase* m_protocol;

	//connecthandler
	boost::mutex conn_mutex_;
	uint16 m_ngateid; 
	set<unsigned int> m_client_connid_set;
	unsigned int		m_svrtype;
	int m_swtich;
	bool m_force_stop;
};

class CConnection
{
public:

	CConnection(const string protocolname);

	void setEvent(IWorkerEvent * event)
	{
		m_connecthandler_Ptr->setEvent(event);
	}

	void connect(const char * ip, u_int16_t port);

	void stop();

	virtual ~CConnection();

	clienthandler_ptr get_connecthandle(){ return m_connecthandler_Ptr; }

	void setRemoteSvrIp(const char* ip,unsigned short port);

private:

	clienthandler_ptr m_connecthandler_Ptr;
};
typedef boost::shared_ptr<CConnection> CConnection_ptr;
#endif // __CLIENT_HANDLER_H__
