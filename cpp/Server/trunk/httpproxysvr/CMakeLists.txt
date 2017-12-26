cmake_minimum_required(VERSION 2.8)
project(httpproxysvr)

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS "-g -Wall  -O0")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall  -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -Wall  -O0")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

aux_source_directory(./  SRC_LIST)
aux_source_directory(./deps/multipart-parser-c  BASE_SRC_LIST)
aux_source_directory(./cmd  CMD_SRC_LIST)
aux_source_directory(./../comm/pcommon/log  LOG)
aux_source_directory(./../comm/pcommon/thrift/chatsvr/client  CHAT_THRIFT)
aux_source_directory(./../comm/pcommon/static  STATIC)


list(APPEND SRC_LIST ${BASE_SRC_LIST}  )
list(APPEND SRC_LIST ${CMD_SRC_LIST}  )
list(APPEND SRC_LIST ${LOG}  )
list(APPEND SRC_LIST ${CHAT_THRIFT} )
list(APPEND SRC_LIST ${STATIC} )


include_directories(./)
#include_directories(${PROJECT_SOURCE_DIR}/deps/json-cpp/include)
include_directories(${PROJECT_SOURCE_DIR}/../comm/lib/jsoncpp-0.10.2/include)
include_directories(${PROJECT_SOURCE_DIR}/../comm/pcommon/log)
include_directories(${PROJECT_SOURCE_DIR}/deps/http-parser)
include_directories(${PROJECT_SOURCE_DIR}/deps/multipart-parser-c)
include_directories(${PROJECT_SOURCE_DIR}/../comm/lib/SocketLite/src)
include_directories(${PROJECT_SOURCE_DIR}/cmd)

include_directories(${PROJECT_SOURCE_DIR}/../comm/lib/libnode)
include_directories(${PROJECT_SOURCE_DIR}/../comm/pcommon/public)
include_directories(${PROJECT_SOURCE_DIR}/../comm/pcommon/thrift/chatsvr/client)
include_directories(${PROJECT_SOURCE_DIR}/../comm/pcommon/static)

link_directories(${PROJECT_SOURCE_DIR}/deps/http-parser/)
#link_directories(${PROJECT_SOURCE_DIR}/deps/json-cpp/)

link_directories(${PROJECT_SOURCE_DIR}/../comm/lib/libnode)
link_directories(/usr/local/lib)
link_directories(${PROJECT_SOURCE_DIR}/../comm/lib/SocketLite/src)
link_directories(/usr/local/lib)

add_executable(httpproxysvr ${SRC_LIST})
target_link_libraries(httpproxysvr libhttp_parser.a  libnode.a libsocketlite.a  libthrift.a -ljsoncpp  -lpthread -lssl -lboost_system -lboost_thread -lboost_locale -lboost_regex)
