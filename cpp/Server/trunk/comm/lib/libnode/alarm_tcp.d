alarm_tcp.o: alarm_tcp.cpp alarm_tcp.h alarm.h \
 ../../pcommon/public/cmd_vchat.h ../../pcommon/public/yc_datatypes.h \
 ../../pcommon/public/message_vchat.h ../../pcommon/public/message_comm.h \
 message_helper.h ../../pcommon/public/message_comm.h
	g++ -Wall -Wno-format -g -DDEBUG -Wno-deprecated -D LIBNODE -DUSE_STL  -c -o alarm_tcp.o  -I/tmp/Server/trunk/comm/lib/libnode  -I../../pcommon  -I../../pcommon/public  -I../../pcommon/log  -I../SocketLite/src  -I/home/boost_1_58_0/sdk/include alarm_tcp.cpp
alarm_tcp.d: alarm_tcp.cpp alarm_tcp.h alarm.h \
 ../../pcommon/public/cmd_vchat.h ../../pcommon/public/yc_datatypes.h \
 ../../pcommon/public/message_vchat.h ../../pcommon/public/message_comm.h \
 message_helper.h ../../pcommon/public/message_comm.h
