FROM centos:centos7
MAINTAINER lishide <376190717@qq.com>

ENV TZ "Asia/Shanghai"

#add source code,get from svn : svn co <path> --username <username>  

ADD cpp/Server /Server
#RUN chmod -R 755 /Server/*
#RUN yum -y update && yum -y group install "Development Tools" && yum install -y wget && cd /usr/local/ && wget http://ftp.gnu.org/gnu/autoconf/autoconf-2.69.tar.gz && tar xvf autoconf-2.69.tar.gz && cd autoconf-2.69 && ./configure && make && make install && cd /usr/local && wget http://ftp.gnu.org/gnu/automake/automake-1.14.tar.gz && tar xvf automake-1.14.tar.gz && cd automake-1.14 && ./configure && make && make install && cd /usr/local && yum -y install bison &&yum install -y make gcc-c++ automake libtool flex bison pkgconfig boost-devel libevent-devel zlib-devel python-devel ruby-devel openssl-devel speex-devel && cd /usr/local 

#RUN wget https://sourceforge.net/projects/boost/files/boost/1.58.0/boost_1_58_0.tar.gz && tar -xvf boost_1_58_0.tar.gz && cd boost_1_58_0 && ./bootstrap.sh && ./b2 install && yum install -y libcurl-dev libcurl-devel && yum install -y mysql-devel && cp  /usr/lib64/mysql/libmysqlclient.so /usr/lib64/libmysqlclient.so


#RUN tar -xvf /Server/trunk/comm/lib/protobuf-cpp-3.0.2.tar.gz -C /usr/local && cd /usr/local/protobuf-3.0.2 && ./configure && make && make install && cp /usr/local/protobuf-3.0.2/src/protoc /usr/bin

#RUN tar -xvf /Server/trunk/comm/lib/libevent-2.0.22-stable.tar.gz -C /usr/local && cd /usr/local/libevent-2.0.22-stable && ./configure && make clean && make&& make install 

#RUN ln -s /usr/local/lib/libboost_unit_test_framework.a /usr/lib64/libboost_unit_test_framework.a&& cd /Server/trunk/comm/lib/libev-4.20 && make && make install &&/Server/trunk/comm/lib/make_libs.sh && ln -s /Server/trunk/comm/lib/hiredis/libhiredis.so /usr/lib64/libhiredis.so && cd /Server/trunk/comm/lib && tar -xvf thrift-0.9.3.tar.gz &&./configure &&make && make install &&tar -xvf libevent-2.0.22-stable.tar.gz -C /usr/local && cd /usr/local/libevent-2.0.22-stable && ./configure && make clean && make && make install &&cd /Server/trunk/comm/lib/thrift-0.9.3 &&./configure && make && make install 

#RUN yum -y install cmake &&  cd /Server/trunk/comm/lib/jsoncpp-0.10.2 && mkdir -p build/debug && cd build/debug && cmake -DCMAKE_BUILD_TYPE=debug -DBUILD_STATIC_LIBS=OFF -DBUILD_SHARED_LIBS=ON -DARCHIVE_INSTALL_DIR=. -G "Unix Makefiles" ../..&& make

