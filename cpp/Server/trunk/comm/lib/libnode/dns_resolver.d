dns_resolver.o: dns_resolver.cpp dns_resolver.h node_impl.h node.h \
 ../../pcommon/public/yc_datatypes.h tcp_base.hpp \
 ../SocketLite/src/SL_ByteBuffer.h ../SocketLite/src/SL_DataType.h \
 ../../pcommon/log/CLogThread.h ../SocketLite/src/SL_Queue.h \
 ../SocketLite/src/SL_Sync_Guard.h ../SocketLite/src/SL_Log.h \
 ../SocketLite/src/SL_Sync_Mutex.h ../SocketLite/src/SL_Sync_Macros.h \
 ../SocketLite/src/SL_Config.h ../SocketLite/src/SL_Singleton.h \
 ../SocketLite/src/SL_Sync_Condition.h ../SocketLite/src/SL_Thread.h
	g++ -Wall -Wno-format -g -DDEBUG -Wno-deprecated -D LIBNODE -DUSE_STL  -c -o dns_resolver.o  -I/tmp/Server/trunk/comm/lib/libnode  -I../../pcommon  -I../../pcommon/public  -I../../pcommon/log  -I../SocketLite/src  -I/home/boost_1_58_0/sdk/include dns_resolver.cpp
dns_resolver.d: dns_resolver.cpp dns_resolver.h node_impl.h node.h \
 ../../pcommon/public/yc_datatypes.h tcp_base.hpp \
 ../SocketLite/src/SL_ByteBuffer.h ../SocketLite/src/SL_DataType.h \
 ../../pcommon/log/CLogThread.h ../SocketLite/src/SL_Queue.h \
 ../SocketLite/src/SL_Sync_Guard.h ../SocketLite/src/SL_Log.h \
 ../SocketLite/src/SL_Sync_Mutex.h ../SocketLite/src/SL_Sync_Macros.h \
 ../SocketLite/src/SL_Config.h ../SocketLite/src/SL_Singleton.h \
 ../SocketLite/src/SL_Sync_Condition.h ../SocketLite/src/SL_Thread.h