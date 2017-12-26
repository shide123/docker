#****************************************************************************
#
# pushmsg
#
# This is a GNU make (gmake) makefile
#****************************************************************************
include ../etc/MakeComm
PROGRAM := pushmsg

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

INCS := ${INCLUDES} -I../comm/pcommon/thrift/commonif -I../comm/pcommon/thrift/pushmsg
		
LIBS := ${LIBDIR} -lworker -ldb -lcom -lsocketlite -lnode -lcurl -lhiredis -ljsoncpp -lprotobuf\
 -lpthread -lboost_system -lboost_thread -lboost_regex -lmysqlclient -lev -lssl -lcrypto  -lthrift -lthriftnb -levent 

#****************************************************************************
# Makefile code common to all platforms
#****************************************************************************

CFLAGS   := ${CFLAGS}   ${DEFS}
CXXFLAGS := ${CXXFLAGS} ${DEFS}

#****************************************************************************
# Targets of the build
#****************************************************************************
vpath %.cpp ../comm/pcommon/thrift/commonif:../comm/pcommon/thrift/pushmsg:../comm/pcommon/thrift/roomsvr:../comm/pcommon/thrift/chatsvr:../comm/pcommon/thrift/consumesvr:../comm/pcommon/thrift/usermgrsvr

OUTPUT := ${PROGRAM}${SUFFIX}

all: ${OUTPUT}


#****************************************************************************
# Source files
#****************************************************************************

SRCS :=main.cpp \
	PushmsgLogic.cpp \
	msgcommapi.cpp \
	redisMgr.cpp \
	redisOpt.cpp \
	utils.cpp \
	DBConnection.cpp \
	DBSink.cpp \
	ConnectionMgr.cpp \
	TPushMsg.cpp \
	TPushMsgIf_constants.cpp \
	TPushMsgIf_types.cpp \
	TPushMsgHandler.cpp	\
	AppDelegate.cpp \
	CThriftSvrMgr.cpp \
	TCommonIf_constants.cpp \
	TCommonIf_types.cpp \
	TRoomSvr.cpp \
	TRoomSvrIf_types.cpp \
	TChatSvr.cpp \
	TChatSvrIf_types.cpp \
	TConsumeSvr.cpp \
	TConsumeSvrIf_types.cpp \
	TUserMsgSvr.cpp \
	TUsermgrSvrIf_types.cpp \
	TUsermgrSvrIf_constants.cpp \
	errcode.cpp \
	Common.pb.cc \
	ChatSvr.pb.cc \
	RoomSvr.pb.cc
	
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

%.o : %.cc
	${CXX} -c ${CXXFLAGS} ${INCS} $< -o $@

%.o : %.c
	${CC} -c ${CFLAGS} ${INCS} $< -o $@

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
