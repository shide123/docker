#ifndef INC_H
#define INC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

// for boost
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/lexical_cast.hpp>
using boost::asio::deadline_timer;
using boost::asio::ip::tcp;

#include "../lib/com/slist.h"
#include "../lib/com/hash.h"
#include "../lib/com/lock_def.h"

#include "../lib/db/sink.h"
#include "../lib/db/table2.h"
#include "../lib/db/table.h"
#include "../lib/db/table2.h"
#include "../lib/db/optable2.h"

#include "../lib/db/sink.h"
#include "../lib/db/pool.h"
#include "../lib/db/timestamp.h"


// for std c++
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <map>

using namespace std;
using namespace __gnu_cxx;

// for socketlite
#include "../lib/SocketLite/src/SL_Queue.h"
#include "../lib/SocketLite/src/SL_ByteBuffer.h"
#include "../lib/SocketLite/src/SL_Sync_Mutex.h"
#include "../lib/SocketLite/src/SL_ObjectPool.h"
#include "../lib/SocketLite/src/SL_Thread.h"
#include "../lib/SocketLite/src/SL_Task.h"
#include "../lib/SocketLite/src/SL_Socket_INET_Addr.h"
#include "../lib/SocketLite/src/SL_Socket_CommonAPI.h"


#include "message_comm.h"
#include "message_vchat.h"
#include "cmd_vchat.h"
#include "macro_define.h"
#include "CLogThread.h"
#include "singleton.h"


#ifndef SSMAXCONN
#define SSMAXCONN 1024
#endif

#ifndef UINT64_MAX
#define UINT64_MAX 18446744073709551615ULL
#endif

#endif // INC_H
