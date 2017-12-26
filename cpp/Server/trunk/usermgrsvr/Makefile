#****************************************************************************
#
# usermgrsvr
#
# This is a GNU make (gmake) makefile
#****************************************************************************
include ../etc/MakeComm
PROGRAM := usermgrsvr

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
vpath %.cpp ../comm/pcommon/thrift/usermgrsvr

INCS := ${INCLUDES} -I../comm/pcommon/thrift/usermgrsvr

LIBS := ${LIBDIR} -ldb -lcom -lnode -lhiredis -ljsoncpp \
	 -lsocketlite -lboost_system -lboost_thread -lboost_regex -lev -lpthread -lcurl -lmysqlclient -lcrypto -lssl -lprotobuf -lthrift -lthriftnb

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

SRCS := basic_redis.cpp \
	redisMgr.cpp \
	redis_map_map_base.cpp \
	redis_map_set_base.cpp \
	redisOpt.cpp \
	redisdataHelper.cpp \
	DBConnection.cpp \
	msgcommapi.cpp \
	utils.cpp \
	UsermgrApp.cpp \
	CConnInfo.cpp \
	ClientConnectionMgr.cpp \
	CUserinfo.cpp \
	CUserMgr.cpp \
	main.cpp \
	UsermgrEvent.cpp \
	DBTools.cpp \
	DBSink.cpp \
	CThriftSvrMgr.cpp \
	CUserGroupinfo.cpp \
	CUserBasicInfo.cpp \
	GroupBasicInfo.cpp \
	CFieldName.cpp \
	UserObj.cpp \
	Common.pb.cc \
	UserMgrSvr.pb.cc \
	RoomSvr.pb.cc \
	TUsermgrSvrIf_constants.cpp \
	TUsermgrSvrIf_types.cpp \
	TUserMsgSvr.cpp \
	TUsermgrHandler.cpp \
	HttpClient.cpp

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
