alarm.o: alarm.cpp alarm.h ../../pcommon/public/cmd_vchat.h \
 ../../pcommon/public/yc_datatypes.h ../../pcommon/public/message_vchat.h \
 ../../pcommon/public/message_comm.h alarm_tcp.h
	g++ -Wall -Wno-format -g -DDEBUG -Wno-deprecated -D LIBNODE -DUSE_STL  -c -o alarm.o  -I/tmp/Server/trunk/comm/lib/libnode  -I../../pcommon  -I../../pcommon/public  -I../../pcommon/log  -I../SocketLite/src  -I/home/boost_1_58_0/sdk/include alarm.cpp
alarm.d: alarm.cpp alarm.h ../../pcommon/public/cmd_vchat.h \
 ../../pcommon/public/yc_datatypes.h ../../pcommon/public/message_vchat.h \
 ../../pcommon/public/message_comm.h alarm_tcp.h
