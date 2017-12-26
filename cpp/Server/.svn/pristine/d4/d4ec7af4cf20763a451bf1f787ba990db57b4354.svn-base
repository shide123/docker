#****************************************************************************
#
# worker
#
# This is a GNU make (gmake) makefile
#****************************************************************************
include ../etc/MakeComm
PROGRAM := libworker.a

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

INCS := ${INCLUDES}

#LIBS := -lsocketlite -lhiredis -lpthread -lboost_system -lboost_thread -lev

#****************************************************************************
# Makefile code common to all platforms
#****************************************************************************

CFLAGS   := ${CFLAGS}   ${DEFS}
CXXFLAGS := ${CXXFLAGS} ${DEFS}

#****************************************************************************
# Targets of the build
#****************************************************************************

OUTPUT := libworker.a

all: ${OUTPUT}


#****************************************************************************
# Source files
#****************************************************************************

SRCS := CLogThread.cpp \
	syslog-nb.cpp \
	CAlarmNotify.cpp \
	utils.cpp \
	Config.cpp \
	csha1.cpp \
	base64.cpp \
	CryptoHandler.cpp \
	io_service_pool.cpp \
	timer.cpp \
	Application.cpp \
	tcpserver.cpp \
	clienthandler.cpp \
	rpcserver.cpp \
	rpcclienthandler.cpp \
	fileinotify.cpp \
	GateConnMgr.cpp \
	protocols/ProtocolsBase.cpp \
	protocols/Frame.cpp \
	protocols/Websocket.cpp \
	protocols/DCL.cpp \
	protocols/Http.cpp
	
# Add on the sources for libraries
SRCS := ${SRCS}

OBJS := $(addsuffix .o,$(basename ${SRCS}))
DEPS := $(addsuffix .d,$(basename ${SRCS})) $(addsuffix .d.,$(basename ${SRCS}))

#****************************************************************************
# Output
#****************************************************************************

${OUTPUT}: ${OBJS}
	${AR} $@ ${OBJS}

#****************************************************************************
# common rules
#****************************************************************************

# Rules for compiling source files to object files
%.o : %.cpp
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
    include $(SRCS:.cpp=.d)
endif

%.d: %.cpp 
	@set -e; rm -f $@; \
	$(CXX) -MM ${CXXFLAGS} ${INCS}  $< > $@.; \
	sed 's,/($*/)/.o[ :]*,/1.o $@ : ,g' < $@. > $@; \
	rm -f $@.
