#****************************************************************************
#
# roomadapter
#
# This is a GNU make (gmake) makefile
#****************************************************************************
include ../etc/MakeComm
PROGRAM := chatsvr

# DEBUG can be set to YES to include debugging info, or NO otherwise
DEBUG          := YES

# PROFILE can be set to YES to include profiling info, or NO otherwise
PROFILE        := NO

# USE_STL can be used to turn on STL support. NO, then STL
# will not be used. YES will include the STL files.
USE_STL := YES

# WIN32_ENV
WIN32_ENV := YES
#****************************************************************************

CC     := gcc
CXX    := g++
LD     := g++
AR     := ar rc
RANLIB := ranlib

# ifeq (YES, ${WIN32_ENV})
#   RM     := del
# else
#   RM     := rm -f
# endif

DEBUG_CFLAGS     := ${WARNING_FLAGS} -g -DDEBUG 
RELEASE_CFLAGS   := ${WARNING_FLAGS} -O2 

DEBUG_CXXFLAGS   := ${DEBUG_CFLAGS} 
RELEASE_CXXFLAGS := ${RELEASE_CFLAGS}

DEBUG_LDFLAGS    := -g
RELEASE_LDFLAGS  := -O3

ifeq (YES, ${DEBUG})
   CFLAGS       := ${DEBUG_CFLAGS}
   CXXFLAGS     := ${DEBUG_CXXFLAGS}
   LDFLAGS      := ${DEBUG_LDFLAGS}
else
   CFLAGS       := ${RELEASE_CFLAGS}
   CXXFLAGS     := ${RELEASE_CXXFLAGS}
   LDFLAGS      := ${RELEASE_LDFLAGS}
endif

ifeq (YES, ${PROFILE})
   CFLAGS   := ${CFLAGS} -pg -O3
   CXXFLAGS := ${CXXFLAGS} -pg -O3
   LDFLAGS  := ${LDFLAGS} -pg
endif

#****************************************************************************
# Preprocessor directives
#****************************************************************************

ifeq (YES, ${USE_STL})
  DEFS := -DUSE_STL
else
  DEFS :=
endif

#****************************************************************************
# Include paths
#****************************************************************************


INCS := ${INCLUDES} -I../comm/pcommon/thrift/chatsvr 

LIBS := ${LIBDIR} -lhiredis -lsocketlite -lconhash -ldb -lcom -lnode -ljsoncpp -L/usr/local/mysql-5.6.30/lib 	\
 -lpthread -lboost_system -lboost_thread -lboost_locale -lboost_regex -lmongoclient -lmysqlclient -lev -lssl -lprotobuf -lspeex

#****************************************************************************
# Makefile code common to all platforms
#****************************************************************************

CFLAGS   := ${CFLAGS}   ${DEFS}
CXXFLAGS := ${CXXFLAGS} ${DEFS}

#****************************************************************************
# Targets of the build
#****************************************************************************

OUTPUT := ${PROGRAM}${SUFFIX}

all: ${OUTPUT}


#****************************************************************************
# Source files
#****************************************************************************

vpath %.cpp ../comm/pcommon/thrift/chatsvr:../comm/pcommon/thrift/tppushsvr

SRCS := main.cpp \
        ChatLogic.cpp \
		GroupAssistLogic.cpp \
        ChatServerApp.cpp \
        GateEvent.cpp \
        ChatCommon.cpp \
        ChatRedisMgr.cpp \
        ChatMongoMgr.cpp \
        ConnectionMgr.cpp \
        KeywordMgr.cpp \
        common.cpp \
        fifo.c \
        TRSpeex.c \

COMSRCS := CLogThread.cpp \
	syslog-nb.cpp \
	Config.cpp \
	CAlarmNotify.cpp \
	cmd_error.cpp \
	utils.cpp \
	DBConnection.cpp \
	DBTools.cpp \
	DBSink.cpp \
	DBmongoConnection.cpp \
	msgcommapi.cpp \
	crc32.cpp \
	md5.cpp \
	trie_tree.cpp \
	ipdatabase.cpp \
	redisOpt.cpp \
	redisMgr.cpp \
	basic_redis.cpp \
	redis_map_map_base.cpp \
	redis_map_set_base.cpp \
	redisdataHelper.cpp \
	CommonLogic.cpp \
	Common.pb.cc \
	ChatSvr.pb.cc \
	RoomSvr.pb.cc \
	TChatSvrIf_constants.cpp \
	TChatSvrIf_types.cpp \
	TChatSvr.cpp \
	TTpPushSvrIf_types.cpp \
	TpPushSvr.cpp \
	TChatSvrHandler.cpp \
	CUserBasicInfo.cpp \
	GroupBasicInfo.cpp \
	CUserGroupinfo \
	UserObj.cpp \
	errcode.cpp \
	CFieldName.cpp \
	HttpClient.cpp 	\
	RoomManager.cpp \
	RoomObj.cpp \
	UserMgrSvr.pb.cc \
	CUserGroupMgr.cpp \
	CThriftSvrMgr.cpp

# Add on the sources for libraries
SRCS := ${SRCS} ${COMSRCS}

OBJS := $(addsuffix .o,$(basename ${SRCS}))
DEPS := $(addsuffix .d,$(basename ${SRCS})) $(addsuffix .d.,$(basename ${SRCS})) 

#****************************************************************************
# Output
#****************************************************************************

${OUTPUT}: ${OBJS} 
	${LD} -o $@ ${LDFLAGS} ${OBJS} ${LIBS} ${EXTRA_LIBS}
#	$(CXX) $@ $(OBJS)
#****************************************************************************
# common rules
#****************************************************************************

# Rules for compiling source files to object files
%.o : %.cpp
	${CXX} -c ${CXXFLAGS} ${INCS} $< -o $@

%.o : %.c
	${CC} -c ${CFLAGS} ${INCS} $< -o $@

%.o : %.cc
	${CXX} -c ${CXXFLAGS} ${INCS} $< -o $@

dist:
	bash makedistlinux

.PHONY : clean
clean:
	${RM} core ${OBJS} ${OUTPUT}
	${RM} ${DEPS}

depend:
	#makedepend ${INCS} ${SRCS}

ifneq "$(MAKECMDGOALS)" "clean"
    include $(addsuffix .d,$(basename ${SRCS}))
endif

%.d: %.cpp
	@set -e; rm -f $@; \
	$(CXX) -MM ${CXXFLAGS} ${INCS}  $< > $@.; \
	sed 's,/($*/)/.o[ :]*,/1.o $@ : ,g' < $@. > $@; \
	rm -f $@.

%.d: %.cc
	@set -e; rm -f $@; \
	$(CXX) -MM ${CXXFLAGS} ${INCS}  $< > $@.; \
	sed 's,/($*/)/.o[ :]*,/1.o $@ : ,g' < $@. > $@; \
	rm -f $@.

%.d: %.c
	@set -e; rm -f $@; \
	$(CXX) -MM ${CXXFLAGS} ${INCS}  $< > $@.; \
	sed 's,/($*/)/.o[ :]*,/1.o $@ : ,g' < $@. > $@; \
	rm -f $@.
