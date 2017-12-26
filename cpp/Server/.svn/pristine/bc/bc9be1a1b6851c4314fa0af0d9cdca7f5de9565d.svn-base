#****************************************************************************
# This is a GNU make (gmake) makefile
#****************************************************************************

# DEBUG can be set to YES to include debugging info, or NO otherwise
DEBUG          := YES

# PROFILE can be set to YES to include profiling info, or NO otherwise
PROFILE        := NO

# USE_STL can be used to turn on STL support. NO, then STL
# will not be used. YES will include the STL files.
USE_STL := YES

# WIN32_ENV
WIN32_ENV := NO
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

DEBUG_CFLAGS     := -Wall -Wno-format -g -DDEBUG -Wno-deprecated
RELEASE_CFLAGS   := -Wall -Wno-unknown-pragmas -Wno-format -O2 -Wno-deprecated

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

CXXFLAGS := ${CXXFLAGS} -D LIBNODE

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

INC_PATH = $(shell pwd)
LIB_PATH = $(shell pwd)
LIBRARYS = 			

INC_PATH += \
			../../pcommon	\
			../../pcommon/public	\
			../../pcommon/log	\
			../SocketLite/src	\
			/home/boost_1_58_0/sdk/include	\

#LIB_PATH += 
#LIBRARYS += 

#****************************************************************************
# Makefile code common to all platforms
#****************************************************************************

CFLAGS   := ${CFLAGS}   ${DEFS}
CXXFLAGS := ${CXXFLAGS} ${DEFS}
LIBS := -pthread

INC_DIR = $(patsubst %, -I%, $(INC_PATH))
LIB_DIR = $(patsubst %, -L%, $(LIB_PATH))
LIBS += $(patsubst %, -l%, $(LIBRARYS))

#****************************************************************************
# Targets of the build
#****************************************************************************

OUTPUT := libnode.a
#OUTPUT := nodeclient

all: ${OUTPUT}


#****************************************************************************
# Source files
#****************************************************************************

SRCS := \
	node_client_impl.cpp		\
	node_impl.cpp			\
	node_client.cpp			\
	dns_resolver.cpp			\
	alarm_tcp.cpp	\
	alarm.cpp	\
	message_helper.cpp \
	tcp_base.cpp	\
	#main.cpp			\
	#node.cpp			\


DEP_SOURCES = $(notdir $(SRCS))
OUTPUT_DEP_OBJS = $(patsubst %.cpp,%.o, $(DEP_SOURCES))

#****************************************************************************
# Output
#****************************************************************************
all : $(OUTPUT)

$(OUTPUT) : $(OUTPUT_DEP_OBJS)
	$(AR) $@ $^

.PHONY : clean
clean:
	-${RM} $(OUTPUT)
	-${RM} *.o
	-${RM} *.d
	-${RM} *.d.

ifneq "$(MAKECMDGOALS)" "clean"
    include $(DEP_SOURCES:.cpp=.d)  
endif

%.d : 
	@t1=$*; t2=$${t1##*/}; \
    for file in $(SRCS); \
    	do \
    		find=$${file%%$$t2.cpp};        \
			find=$$find$$t2.cpp;		\
			split=$${file##*/};	\
			cmpfile=$$t2.cpp;	\
			if [ $$split = $$cmpfile ] && [ $${find} = $${file} ]; then     \
            	rely_file=$$file;       \
    		fi;     \
    	done;   \
    set -e; rm -f $@; \
    $(CXX) -MM $(CXXFLAGS) $(INC_DIR) $$rely_file > $@.;    \
    cat $@. > $@;   \
    echo "	$(CXX) $(CXXFLAGS) $(SHARED) -c -o $$t2.o $(INC_DIR) $$rely_file" >> $@; \
    sed 's/\.o/\.d/g' < $@. >> $@; \
    rm -f $@.       ;

