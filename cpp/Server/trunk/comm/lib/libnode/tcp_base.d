tcp_base.o: tcp_base.cpp tcp_base.hpp ../SocketLite/src/SL_ByteBuffer.h \
 ../SocketLite/src/SL_DataType.h ../../pcommon/log/CLogThread.h \
 ../SocketLite/src/SL_Queue.h ../SocketLite/src/SL_Sync_Guard.h \
 ../SocketLite/src/SL_Log.h ../SocketLite/src/SL_Sync_Mutex.h \
 ../SocketLite/src/SL_Sync_Macros.h ../SocketLite/src/SL_Config.h \
 ../SocketLite/src/SL_Singleton.h ../SocketLite/src/SL_Sync_Condition.h \
 ../SocketLite/src/SL_Thread.h ../../pcommon/public/message_comm.h \
 ../../pcommon/public/yc_datatypes.h
	g++ -Wall -Wno-format -g -DDEBUG -Wno-deprecated -D LIBNODE -DUSE_STL  -c -o tcp_base.o  -I/tmp/Server/trunk/comm/lib/libnode  -I../../pcommon  -I../../pcommon/public  -I../../pcommon/log  -I../SocketLite/src  -I/home/boost_1_58_0/sdk/include tcp_base.cpp
tcp_base.d: tcp_base.cpp tcp_base.hpp ../SocketLite/src/SL_ByteBuffer.h \
 ../SocketLite/src/SL_DataType.h ../../pcommon/log/CLogThread.h \
 ../SocketLite/src/SL_Queue.h ../SocketLite/src/SL_Sync_Guard.h \
 ../SocketLite/src/SL_Log.h ../SocketLite/src/SL_Sync_Mutex.h \
 ../SocketLite/src/SL_Sync_Macros.h ../SocketLite/src/SL_Config.h \
 ../SocketLite/src/SL_Singleton.h ../SocketLite/src/SL_Sync_Condition.h \
 ../SocketLite/src/SL_Thread.h ../../pcommon/public/message_comm.h \
 ../../pcommon/public/yc_datatypes.h
