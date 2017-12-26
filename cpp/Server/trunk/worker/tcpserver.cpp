#include "tcpserver.h"
#include "clienthandler.h"
#include "CLogThread.h"
#include "Application.h"
#define SHM_MODE  	0600

tcpserver::CONN_MAP tcpserver::conns;
boost::mutex tcpserver::conns_Mutex;

unsigned int tcpserver::connid_ = 1;
boost::mutex tcpserver::m_condid_mutex;

tcpserver::tcpserver()
	: io_service_pool_(1)
	, io_service_pool_single(1)
	, acceptor_(io_service_pool_single.get_io_service())
	, port_(0)
    , sockfd(-1)
    , totalconns(0)
	, connid(0)
	, svrid(0)
	, m_workerEvent(NULL)
	, pcn(NULL)
	, m_shmId(-1)
	, m_procnum(0)
	, m_index(0)
{
//	LOG_PRINT(log_info, "syncsvr start listen[%s:%d]", endpoint.address().to_string().c_str(), endpoint.port());
}

tcpserver::~tcpserver()
{
    port_ = 0;

    close(sockfd);
    sockfd = -1;
	if (pcn)
	{
		shmdt(pcn);
	}
}

bool tcpserver::initShm(int size)
{
	if((m_shmId = shmget(IPC_PRIVATE,sizeof(PID_CONNS) * size,SHM_MODE)) < 0)
	{
		LOG_PRINT(log_error, "create shared memory failed.");
		perror("create shared memory failed");
		return false;
	}

	pcn = (PID_CONNS *)shmat(m_shmId,0,0);
	memset(pcn, 0, sizeof(PID_CONNS) * size);
	return true;
}

void tcpserver::init(u_int16_t port, int procnum,std::size_t io_service_pool_size,std::string protocol,IWorkerEvent* event,int max_conns)
{
	port_ = port;
	if(port == 0)
		return ;

    if(procnum > 0)
    {   
    	m_procnum = procnum;

		initShm(procnum);

		/*
        if((semid = semget(IPC_PRIVATE,procnum,SHM_MODE)) < 0)
        {
            perror("create semaphore failed");
            return ;
        }
        //init sem;
        semun su;
        su.val = 0;
        if(semctl(semid,0,SETVAL, su) < 0){
            perror("semctl failed");
            return ;
        }
		*/
    }

    m_max_conns = max_conns;
	m_strProtocol = protocol;
	m_workerEvent = event;
	tcp::resolver resolver(acceptor_.get_io_service());
	char sport[8];
	sprintf(sport, "%d", port);
	tcp::resolver::query query("0.0.0.0", sport);
	tcp::resolver::iterator endpoint_iter = resolver.resolve(query);
	tcp::endpoint endpoint = *endpoint_iter;
	acceptor_.open(endpoint.protocol());
	acceptor_.set_option(tcp::acceptor::reuse_address(true));
	acceptor_.bind(endpoint);
	acceptor_.listen();
}

void tcpserver::delshm()
{
	if (pcn)
	{
		shmctl(m_shmId, IPC_RMID, NULL);
		pcn = NULL;
	}
}

void tcpserver::start(int index)
{
	if(port_ == 0)
			return ;

	if(index >= 0 && index < m_procnum && pcn)
	{
		/*
		if(index == 0)
		{
			semun su;
			su.val = 1;
			if(semctl(semid,0,SETVAL, su) < 0){
				perror("semctl failed");
				return ;
			}
		}
		*/

		PID_CONNS& pc = pcn[index];
		pc.pid = getpid();
		pc.index = index;
		pc.conns = tcpserver::getconns_size();
	}

	m_index = index;
	io_service_pool_.run();
	
	io_service_pool_single.get_io_service().post(boost::bind(&tcpserver::start_accept, this));
	io_service_pool_single.run();
}

//void tcpserver::run()
//{
//	io_service_pool_.loop();
//}
//void server::wait(int index)
//{
//	while(index >= 0)
//	{
//		struct sembuf sb;
//		sb.sem_num = index;
//		sb.sem_op = -1;//表示要把信号量减一
//		sb.sem_flg = SEM_UNDO;//
//		//第二个参数是 sembuf [] 类型的，表示数组
//		//第三个参数表示 第二个参数代表的数组的大小
//		if(semop(semid,&sb,1) == 0)
//		{
//			LOG_PRINT(log_debug, "wait break index:%d.", index);
//			break;
//		}
//		else
//		{
//			LOG_PRINT(log_debug, "wait index:%d.", index);
//		}
//	}
//}
//void server::notify(int index)
//{
//	while(index >= 0)
//	{
//		struct sembuf sb;
//		sb.sem_num = index;
//		sb.sem_op = 1;
//		sb.sem_flg = SEM_UNDO;
//		//第二个参数是 sembuf [] 类型的，表示数组
//		//第三个参数表示 第二个参数代表的数组的大小
//		if(semop(semid,&sb,1) == 0)
//		{
//			LOG_PRINT(log_debug, "notify index:%d.", index);
//			break;
//		}
//	}
//}

int tcpserver::getminindex()
{
	int minindex = -1;
	if(m_procnum > 0 && pcn)
	{
		int min = pcn[0].conns;
		minindex = 0;
		for(int i = 1 ; i < m_procnum ; i++)
		{
			if(pcn[i].conns < min)
			{
				min = pcn[i].conns;
				minindex = i;
			}
		}
	}

	return minindex;
}

int tcpserver::getmaxindex()
{
	int maxindex = -1;
	if(m_procnum > 0 && pcn)
	{
		int max = pcn[0].conns;
		maxindex = 0;
		for(int i = 1 ; i < m_procnum ; i++)
		{
			if(pcn[i].conns > max)
			{
				max = pcn[i].conns;
				maxindex = i;
			}
		}
	}

	return maxindex;
}

void tcpserver::start_accept()
{
	//wait(m_index);
	clienthandler_ptr conn(new clienthandler(io_service_pool_.get_io_service(),m_strProtocol,m_workerEvent,this));
	acceptor_.async_accept(conn->socket_, boost::bind(&tcpserver::handle_accept, this, conn, boost::asio::placeholders::error));
}

void tcpserver::client_release()
{
    if(m_index >= 0)
    {
    	if(m_index < m_procnum && pcn)
    	{
    		PID_CONNS& pc = pcn[m_index];
    		pc.conns = tcpserver::getconns_size();
    	}
    
    	{
    		boost::mutex::scoped_lock lock(cond_mutex);
    		cond_.notify_one();
    	}
    }
}

void tcpserver::handle_accept(clienthandler_ptr session, const boost::system::error_code& e)
{
    if(m_index >= 0 && m_index < m_procnum && pcn)
    {
		PID_CONNS& pc = pcn[m_index];
		pc.conns = tcpserver::getconns_size() + 1;
    	//notify(getminindex());
    }
	if(!e)
	{
		if (session.get()){
			session->open();
			totalconns++;
			LOG_PRINT(log_info, "toatal connections number: %d", totalconns);
		}
	}
	else
	{
		LOG_PRINT(log_error, "%s",boost::system::system_error(e).what());
	}
    
	if (m_index >= 0)
	{
		int max_index = getmaxindex();
		if(max_index != -1 && max_index == m_index)
		{
			usleep(100*1000);
		}
	}

	printf("connsize:%d connid:%d\n", tcpserver::conns.size(), session->getconnid());
	while(tcpserver::conns.size() >= m_max_conns)
	{
		printf("wait====\n");
		boost::mutex::scoped_lock lock(cond_mutex);
		cond_.wait(cond_mutex);
		printf("wait++++\n");
	}
    
	io_service_pool_single.get_io_service().post(boost::bind(&tcpserver::start_accept, this));
}

void tcpserver::handle_stop()
{
	io_service_pool_.stop();
}

void tcpserver::fork_notify()
{
	io_service_pool_.fork_notify();
}

void tcpserver::setconnhandler(unsigned int connid, clienthandler_ptr handler)
{
	boost::mutex::scoped_lock lock(conns_Mutex);
	tcpserver::conns[connid] = handler;
}

clienthandler_ptr tcpserver::getconnhandler(unsigned int connid)
{
	clienthandler_ptr handler;

	boost::mutex::scoped_lock lock(conns_Mutex);
	CONN_MAP::iterator it = tcpserver::conns.find(connid);
	if (it != tcpserver::conns.end())
	{
		handler = it->second;
	}
	return handler;
}

void tcpserver::getallconnhandler(std::list<clienthandler_ptr> & lstOut)
{
	lstOut.clear();
	boost::mutex::scoped_lock lock(conns_Mutex);
	CONN_MAP::iterator it = tcpserver::conns.begin();
	for (; it != tcpserver::conns.end(); it++)
	{
		lstOut.push_back(it->second);
	}
}

unsigned int tcpserver::cast_msg_to_allconn(const char * pdata, unsigned int len)
{
	if (NULL == pdata || 0 == len)
	{
		return 0;
	}

	unsigned int nCount = 0;
	std::list<clienthandler_ptr> lstClients;
	getallconnhandler(lstClients);

	std::list<clienthandler_ptr>::iterator it = lstClients.begin();
	while(it != lstClients.end())
	{
		clienthandler_ptr client = *it;
		if (client && client->isconnected() && client->bSayHello)
		{
			client->write_message(pdata, len);
			nCount++;
		}
		++it;
	}

	return nCount;
}

void tcpserver::eraseconnhandler(unsigned int connid)
{
	boost::mutex::scoped_lock lock(conns_Mutex);
	tcpserver::CONN_MAP::iterator it = conns.find(connid);
	if (it != conns.end())
	{
		conns.erase(it);
	}
}

unsigned int tcpserver::getconns_size()
{
	boost::mutex::scoped_lock lock(conns_Mutex);
	return conns.size();
}

unsigned int tcpserver::getconnid()
{
	boost::mutex::scoped_lock lock(m_condid_mutex);
	return connid_++;
}

//void tcpserver::setgateclient(uint16_t gateid, clienthandler *pgateclient)
//{
//	boost::mutex::scoped_lock(client_mutex_);
//	gateclientmap[gateid] = pgateclient;
//
//}

//clienthandler* server::getgateclient(uint16_t gateid)
//{
//	boost::mutex::scoped_lock(client_mutex_);
//	return gateclientmap[gateid];
//}
