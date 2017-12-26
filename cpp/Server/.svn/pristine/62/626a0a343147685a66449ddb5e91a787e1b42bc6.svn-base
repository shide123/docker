// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "string"
#include <time.h>


#include "../src/sl_bytebuffer.h"
#include "../src/sl_Random.h"
#include "../src/sl_sync_cond.h"
#include "../src/sl_thread.h"

#include "../src/SL_Sync_Mutex.h"
#include "../src/SL_Socket_TcpServer.h"
#include "../src/SL_Socket_TcpServer_Handler.h"
#include "../src/SL_Socket_INET_Addr.h"
#include "../src/SL_ObjectPool.h"
#include "../src/SL_Socket_Select_Runner.h"

#include "../src/SL_Task.h"

#include "testHandler.h"
//#include <omp.h>
//#include <string>
//#include <unordered_map>
//#include <hash_map>
//#include <map>

//typedef std::map<uint64,int > std_map_t;
//typedef stdext::hash_map<uint64,int > stdext_hash_map_t;
//typedef std::tr1::unordered_map<uint64,int > stdtr1_unordered_map_t;

#include "testtask.h"

#include "../src/sl_queue.h"
#include "../src/sl_timer.h"
#include "../src/sl_timer_list_time.h"
#include "../src/sl_timer_list_clock.h"
#include "sl_timer_test.h"

#include <vector>
#include <list>
#include <string>


#include "../src/SL_Hash_Fun.h"
//typedef __int64 int64;
//typedef unsigned __int64 uint64;


// uint WINAPI TestThreadProc(void *arg)
// {
// 	int *i = (int*)arg;
// 	while (1)
// 	{
// 		printf("thread test:%d!\n",*i);
// 		Sleep(100);
// 	}

// 	return 0;
// };

int main(int argc, char* argv[])
{
	
    //std::vector<int> testList;
    //testList.reserve(1000000);
    //printf("%d,%d\n",testList.size(),testList.capacity());
    //for (int i=0;i<100000;i++)
    //{
    //    testList.push_back(i);
    //}
    //printf("%d,%d\n",testList.size(),testList.capacity());
    //for (int i=0;i<100;i++)
    //{
    //    testList.push_back(i);
    //}
    //printf("%d,%d\n",testList.size(),testList.capacity());
    //for (int i=0;i<100;i++)
    //{
    //    testList.pop_back();
    //}
    //printf("%d,%d\n",testList.size(),testList.capacity());
    //for (int i=0;i<100000;i++)
    //{
    //    testList.pop_back();
    //}
    //printf("%d,%d\n",testList.size(),testList.capacity());

	//{
	//	{
	//		SL_Random::randomize(0);

	//		int ret;
	//		stdext_hash_map_t h1;
	//		stdtr1_unordered_map_t h2;
	//		std_map_t h3;

	//		stdext_hash_map_t::iterator it1;
	//		stdtr1_unordered_map_t::iterator it2;
	//		std_map_t::iterator it3;

	//		ret = ::GetTickCount();
	//		h1.rehash(2000000);
	//		printf("h1.rehash: %d ms\n",::GetTickCount()-ret);

	//		ret = ::GetTickCount();
	//		h2.rehash(2000000);
	//		printf("h2.rehash: %d ms\n",::GetTickCount()-ret);

	//		//h1 insert
	//		int value;
	//		ret = ::GetTickCount();
	//		for (int i=0;i<2000000;i++)
	//		{
	//			//value = SL_Random::randomInteger(0,999999999);
	//			//h1.insert(stdext_hash_map_t::value_type(value,value));
	//			h1.insert(stdext_hash_map_t::value_type(i,i));
	//		}
	//		printf("h1.insert: %d ms size:%ld\n",::GetTickCount()-ret,h1.size());

	//		//h2 insert
	//		ret = ::GetTickCount();
	//		for (int i=0;i<2000000;i++)
	//		{
	//			//value = SL_Random::randomInteger(0,999999999);
	//			//h2.insert(stdtr1_unordered_map_t::value_type(value,value));
	//			h2.insert(stdtr1_unordered_map_t::value_type(i,i));
	//		}
	//		printf("h2.insert: %d ms size:%ld\n",::GetTickCount()-ret,h2.size());

	//		//h3 insert
	//		ret = ::GetTickCount();
	//		for (int i=0;i<2000000;i++)
	//		{
	//			//value = SL_Random::randomInteger(0,999999999);
	//			//h3.insert(std_map_t::value_type(value,value));
	//			h3.insert(std_map_t::value_type(i,i));
	//		}
	//		printf("h3.insert: %d ms size:%ld\n",::GetTickCount()-ret,h3.size());

	//		int findsuccess_count = 0;
	//		//h1 find
	//		findsuccess_count = 0;
	//		ret = ::GetTickCount();
	//		for (int i=0;i<20000000;i++)
	//		{
	//			value = SL_Random::randomInteger(0,999999999);
	//			it1 = h1.find(value);
	//			if (it1 != h1.end())
	//				findsuccess_count++;
	//		}
	//		printf("h1.find: %d ms findsucces_count:%d\n",::GetTickCount()-ret,findsuccess_count);

	//		//h2 find
	//		findsuccess_count = 0;
	//		ret = ::GetTickCount();
	//		for (int i=0;i<20000000;i++)
	//		{
	//			value = SL_Random::randomInteger(0,999999999);
	//			it2 = h2.find(value);
	//			if (it2 != h2.end())
	//				findsuccess_count++;
	//		}
	//		printf("h2.find: %d ms findsucces_count:%d\n",::GetTickCount()-ret,findsuccess_count);

	//		//h3 find
	//		findsuccess_count = 0;
	//		ret = ::GetTickCount();
	//		for (int i=0;i<20000000;i++)
	//		{
	//			value = SL_Random::randomInteger(0,999999999);
	//			it3 = h3.find(value);
	//			if (it3 != h3.end())
	//				findsuccess_count++;
	//		}
	//		printf("h3.find: %d ms findsucces_count:%d\n",::GetTickCount()-ret,findsuccess_count);

	//		ret = ::GetTickCount();
	//		h1.clear();
	//		printf("h1.clear: %d ms\n",::GetTickCount()-ret);

	//		ret = ::GetTickCount();
	//		h2.clear();
	//		printf("h2.clear: %d ms\n",::GetTickCount()-ret);

	//		ret = ::GetTickCount();
	//		h3.clear();
	//		printf("h3.clear: %d ms\n",::GetTickCount()-ret);
	//	}
	//}

	//h2.insert(stdtr1_unordered_map_t<int64,string>::value_type(100,"ABB"));
	//h2.insert(stdtr1_unordered_map_t<int64,string>::value_type(102,"zby"));

    //SL_ByteBuffer buffer;
    //buffer.clear();
    //bool bret = buffer.empty();
    //bret = buffer.write("ZBY is bolidezhang");
    //int i = buffer.capacity();
    //int j = buffer.buffer_size();
    //int k = buffer.data_size();
    //buffer.write(j);
    //i = buffer.capacity();
    //j = buffer.buffer_size();
    //k = buffer.data_size();

	//char *p = buffer.buffer();
	//p = buffer.data();

	//SL_Random::randomize();
	//int a = SL_Random::randomInteger();

	////SL_Sync_NullMutex_t SL_Sync_NullMutex::instance();
	//{
	//	SL_Sync_ThreadMutex mutex;
	//	SL_Thread<SL_Sync_ThreadMutex> t1(mutex);
	//	int i = 256;
	//	t1.start(TestThreadProc,&i);
	//	t1.sleep(100);
	//}

	//printf("%d\n", _OPENMP);
	//#pragma omp parallel
	//{
	//	printf("Hello from thread %d, nthreads %d\n", omp_get_thread_num(), omp_get_num_threads());
	//}
	//ulong currenttime = ::time(NULL);
	
	//int sum = 0;
	//#pragma omp parallel num_threads(2)
	//{
	//	//int i = omp_get_thread_num();
	//	//printf_s("Hello from thread %d\n", i);
	//	for (int i=1;i<10;i++)
	//	{
	//		sum += i;
	//		printf("Hello from thread %d, nthreads %d\n", omp_get_thread_num(), omp_get_num_threads());
	//		printf("sum=%ld\n",sum);
	//	}
	//}
	//printf("total sum=%ld\n",sum);

//
//<<<<<<< .mine
//	SL_Socket_CommonAPI::socket_init(SOCKETLITE_WINSOCK_VERSION);
//	SL_Socket_Select_Runner select_runner1;
//	SL_Socket_TcpServer<SL_Socket_TcpServer_Handler> tcp_server1(&select_runner1);
//	//tcp_server1.open(6000,"192.168.185.59",100,false); 
//	tcp_server1.open(6000,100,0,false); 
//=======
//	//SL_Socket_CommonAPI::socket_init(SOCKETLITE_WINSOCK_VERSION);
// //   SL_Socket_Select_Runner select_runner;
// //   SL_ObjectPool_SimpleEx<testHandler,SL_Sync_NullMutex> poolex;
// //   SL_Socket_TcpServer_Handler_Impl<testHandler,SL_ObjectPool_SimpleEx<testHandler,SL_Sync_NullMutex> > tcpserver_handler(&poolex);
//>>>>>>> .r872

 //   select_runner.open(SL_Socket_Handler::ALL_EVENT_MASK);
	//SL_Socket_TcpServer tcp_server(&tcpserver_handler,&select_runner);
	//tcp_server.open(6000,100,0,false); 

    
    //SL_ByteBuffer buf;
    //std::string str = "dfadsf";
    //buf.write(str.c_str(),str.size());
    ////SL_ByteBuffer buf2;
    ////buf2 = buf;


    //std::list<SL_ByteBuffer> testList;
    //testList.push_back(buf);
    ////buf.owner(false);
    //SL_Queue<SL_ByteBuffer,SL_Sync_NullMutex> queue;
    //queue.push_back(buf);
    //int i;
    //i = 200;

    //CTestTask task;
    //task.open(1);
    //SL_ByteBuffer buf;
    //std::string str = "123456";
    //buf.write(str.data(),str.size()+1);
    //task.putq(buf);
    //SL_ByteBuffer buf2;
    //str = "abcdedf";
    //buf2.write(str.data(),str.size()+1);
    //task.putq(buf2);
    //::Sleep(20000);

	//char ip[50]={0};
	//tcp_server1.get_addr()->get_ip_addr(ip,50);
	//printf("ip:(%s)\n",ip);

	//ushort port_number = 0;
	//memset(ip,0,50);
	//int ret = SL_Socket_INET_Addr::get_ip_local_s(tcp_server1.get_handler()->get_socket(), ip, 50, &port_number);
	//printf("ip:(%s),port:%d\n",ip,port_number);
	//int error = WSAGetLastError();

	//SL_Socket_INET_Addr remoteAddr;
	//SL_Socket_INET_Addr::get_addr_s("www.baidu.com",80,remoteAddr.get_addr(),remoteAddr.get_addr_size());

	//memset(ip,0,50);
	//remoteAddr.get_ip_addr(ip,50);
	//printf("ip:(%s),port:%d\n",ip,remoteAddr.get_port_number());

	//SL_Sync_Event<SL_Sync_NullMutex> event1(SL_Sync_NullMutex_T::instance());
	//event1.wait();

	//timeval tm;
	//tm.tv_sec = 5;
	//tm.tv_usec = 0;
	//remoteAddr.set("www.baidu.com",8001);
	//SL_SOCKET fd = SL_Socket_CommonAPI::socket_open(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//SL_Socket_CommonAPI::socket_connect(fd,remoteAddr,remoteAddr.get_addr_size(),&tm);
	
	
    //SL_Socket_Select_Runner<SL_Sync_NullMutex> select_runner;
    //char path[260];
    ////MAX_PATH
    //int len = SL_Socket_CommonAPI::util_get_application_path(path, 260);
    //memset(path,0,260);
    //len = SL_Socket_CommonAPI::util_get_application_name(path, 260);

    //SL_Thread<SL_Sync_ThreadMutex> th;
    //SL_Sync_ThreadMutex &m=th.mutex();
    
    clock_t ct = clock();
    SL_Timer_Test timer_test;
    SL_Timer_Test timer_test2;
    SL_Timer_Test timer_test3;
    timer_test.set_config(1,500,NULL, SL_Timer::TIMER_TYPE_CYCLE);
    timer_test2.set_config(1,50, NULL, SL_Timer::TIMER_TYPE_CYCLE);
    timer_test3.set_config(1,10, NULL, SL_Timer::TIMER_TYPE_CYCLE);

    SL_Timer_List_Clock<SL_Sync_NullMutex> timer_list;
    SL_Timer_List_Time<SL_Sync_NullMutex>  timer_list2;
    timer_list.set_config(1,1);
    timer_list.add_timer(&timer_test);
    //timer_list.add_timer(&timer_test2);
    //timer_list.add_timer(&timer_test3);
    timer_list.thread_timer_loop();

    //timer_list2.set_config(1,10);
    //timer_list2.add_timer(&timer_test);
    //timer_list2.add_timer(&timer_test2);
    //timer_list2.add_timer(&timer_test3);
    //timer_list2.thread_timer_loop();


    timer_list.thread_wait();
	return 1;
}

//NI_MAXHOST


