message_helper.o: message_helper.cpp message_helper.h \
 ../../pcommon/public/message_comm.h ../../pcommon/public/yc_datatypes.h
	g++ -Wall -Wno-format -g -DDEBUG -Wno-deprecated -D LIBNODE -DUSE_STL  -c -o message_helper.o  -I/tmp/Server/trunk/comm/lib/libnode  -I../../pcommon  -I../../pcommon/public  -I../../pcommon/log  -I../SocketLite/src  -I/home/boost_1_58_0/sdk/include message_helper.cpp
message_helper.d: message_helper.cpp message_helper.h \
 ../../pcommon/public/message_comm.h ../../pcommon/public/yc_datatypes.h
