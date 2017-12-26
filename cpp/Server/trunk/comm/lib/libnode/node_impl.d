node_impl.o: node_impl.cpp node_impl.h node.h \
 ../../pcommon/public/yc_datatypes.h tcp_base.hpp \
 ../SocketLite/src/SL_ByteBuffer.h ../SocketLite/src/SL_DataType.h \
 message_helper.h ../../pcommon/public/message_comm.h \
 ../../pcommon/public/yc_datatypes.h ../../pcommon/public/cmd_nodemgr.h \
 ../../pcommon/public/message_vchat.h ../../pcommon/public/message_comm.h \
 dns_resolver.h ../../pcommon/log/CLogThread.h \
 ../SocketLite/src/SL_Queue.h ../SocketLite/src/SL_Sync_Guard.h \
 ../SocketLite/src/SL_Log.h ../SocketLite/src/SL_Sync_Mutex.h \
 ../SocketLite/src/SL_Sync_Macros.h ../SocketLite/src/SL_Config.h \
 ../SocketLite/src/SL_Singleton.h ../SocketLite/src/SL_Sync_Condition.h \
 ../SocketLite/src/SL_Thread.h
	g++ -Wall -Wno-format -g -DDEBUG -Wno-deprecated -D LIBNODE -DUSE_STL  -c -o node_impl.o  -I/tmp/Server/trunk/comm/lib/libnode  -I../../pcommon  -I../../pcommon/public  -I../../pcommon/log  -I../SocketLite/src  -I/home/boost_1_58_0/sdk/include node_impl.cpp
node_impl.d: node_impl.cpp node_impl.h node.h \
 ../../pcommon/public/yc_datatypes.h tcp_base.hpp \
 ../SocketLite/src/SL_ByteBuffer.h ../SocketLite/src/SL_DataType.h \
 message_helper.h ../../pcommon/public/message_comm.h \
 ../../pcommon/public/yc_datatypes.h ../../pcommon/public/cmd_nodemgr.h \
 ../../pcommon/public/message_vchat.h ../../pcommon/public/message_comm.h \
 dns_resolver.h ../../pcommon/log/CLogThread.h \
 ../SocketLite/src/SL_Queue.h ../SocketLite/src/SL_Sync_Guard.h \
 ../SocketLite/src/SL_Log.h ../SocketLite/src/SL_Sync_Mutex.h \
 ../SocketLite/src/SL_Sync_Macros.h ../SocketLite/src/SL_Config.h \
 ../SocketLite/src/SL_Singleton.h ../SocketLite/src/SL_Sync_Condition.h \
 ../SocketLite/src/SL_Thread.h
