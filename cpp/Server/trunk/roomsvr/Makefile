#****************************************************************************
#
# roomsvr
#
# This is a GNU make (gmake) makefile
#****************************************************************************
include ../etc/MakeComm
PROGRAM := roomsvr

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
vpath %.cpp ../comm/pcommon/thrift/commonif:../comm/pcommon/thrift/chatsvr/:../comm/pcommon/thrift/roomsvr:../comm/pcommon/thrift/tppushsvr
INCS := ${INCLUDES} -I../comm/pcommon/thrift/ -I../comm/pcommon/thrift/commonif -I../comm/pcommon/thrift/chatsvr/ -I../comm/pcommon/thrift/roomsvr \
	-I../comm/pcommon/thrift/tppushsvr/

LIBS := ${LIBDIR} -ldb -lcom -lsocketlite -lnode -ljsoncpp -lnode -lhiredis \
	-lpthread -lcurl -lboost_system -lboost_thread  -lboost_locale -lboost_regex -lev -lmysqlclient -lssl -lcrypto -lprotobuf 	\
	-lthrift -lthriftnb  -lcryptopp -lmongoclient

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

SRCS := AppDelegate.cpp \
	main.cpp \
	serverEvent.cpp \
	LogicCore.cpp \
	ipdatabase.cpp \
	ipdb99cj.cpp \
	DBConnection.cpp \
	CLogThread.cpp \
	syslog-nb.cpp \
	CAlarmNotify.cpp \
	cmd_error.cpp \
	crc32.cpp \
	md5.cpp \
	utils.cpp \
	redisOpt.cpp \
	redisMgr.cpp \
	redis_map_set_base.cpp \
	redis_map_map_base.cpp \
	basic_redis.cpp \
	redisdataHelper.cpp \
	HttpClient.cpp \
	Config.cpp \
	CFieldName.cpp \
	UserObj.cpp \
	RoomObj.cpp \
	RoomManager.cpp \
	GroupBasicInfo.cpp \
	CUserBasicInfo.cpp \
	CUserGroupinfo.cpp \
	CUserGroupMgr.cpp \
	CommonLogic.cpp \
	AndroidUserManager.cpp \
	CThriftSvrMgr.cpp \
	DBTools.cpp \
	DBSink.cpp \
	msgcommapi.cpp \
	Common.pb.cc \
	UserMgrSvr.pb.cc \
	RoomSvr.pb.cc \
	TCommonIf_types.cpp \
	TCommonIf_constants.cpp \
	TChatSvr.cpp \
	TChatSvrIf_constants.cpp \
	TChatSvrIf_types.cpp \
	TRoomSvr.cpp \
	TRoomSvrIf_constants.cpp \
	TRoomSvrIf_types.cpp \
	TRoomSvrHandler.cpp \
	TTpPushSvrIf_types.cpp \
	TpPushSvr.cpp \
	errcode.cpp	
	
COMSRCS := DBmongoConnection.cpp

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
