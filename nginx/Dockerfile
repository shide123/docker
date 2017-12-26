FROM centos:centos7
MAINTAINER lishide <376190717@qq.com>

ENV TZ "Asia/Shanghai"

#install env
RUN yum -y update && yum install -y httpd gcc automake autoconf libtool make gcc-c++ vixie-cron  wget zlib  file openssl-devel sharutils zip  bash vim cyrus-sasl-devel libmemcached libmemcached-devel libyaml libyaml-devel unzip libvpx-devel openssl-devel ImageMagick-devel  autoconf  tar gcc libxml2-devel gd-devel libmcrypt-devel libmcrypt mcrypt mhash libmcrypt libmcrypt-devel libxml2 libxml2-devel bzip2 bzip2-devel curl curl-devel libjpeg libjpeg-devel libpng libpng-devel freetype-devel bison libtool-ltdl-devel net-tools && yum clean all

#create config path
RUN mkdir -p /usr/local/myconf

# install jdk1.8
RUN mkdir /var/tmp/jdk
RUN wget --no-check-certificate --no-cookies --header "Cookie: oraclelicense=accept-securebackup-cookie"  -P /var/tmp/jdk http://download.oracle.com/otn-pub/java/jdk/8u151-b12/e758a0de34e24606bca991d704f6dcbf/jdk-8u151-linux-x64.tar.gz
RUN tar xzf /var/tmp/jdk/jdk-8u151-linux-x64.tar.gz -C /var/tmp/jdk && rm -rf /var/tmp/jdk/jdk-8u151-linux-x64.tar.gz


#env setting
ENV JAVA_HOME /var/tmp/jdk/jdk1.8.0_151
ENV PATH=$JAVA_HOME/bin:$PATH
ENV CLASSPATH=.:$JAVA_HOME/lib/dt.jar:$JAVA_HOME/lib/tools.jar 

#install Nginx
RUN cd /tmp && wget http://nginx.org/packages/centos/7/noarch/RPMS/nginx-release-centos-7-0.el7.ngx.noarch.rpm && rpm -ivh nginx-release-centos-7-0.el7.ngx.noarch.rpm  && yum -y install nginx

#install logstash
RUN cd /usr/local && wget https://artifacts.elastic.co/downloads/logstash/logstash-6.1.1.tar.gz && cd /usr/local && tar -xvf logstash-6.1.1.tar.gz && mv logstash-6.1.1 logstash 

#add logstash.conf
ADD nginx/logstash.conf /usr/local/myconf/logstash.conf
#add nginx.conf
ADD nginx/nginx.conf /usr/local/myconf/nginx.conf
ADD nginx/fastcgi_params /usr/local/myconf/fastcgi_params
#add start script
ADD nginx/start.sh /tmp/start.sh

#add extend  port
EXPOSE 80 8081 443

#start app
#RUN nginx -c /usr/local/myconf/nginx.conf
#RUN nohup /usr/local/logstash/bin/logstash -f /usr/local/myconf/logstash.conf &

CMD ["/bin/sh","/tmp/start.sh"]

#start container: docker 
##docker run --name nginx -v <config_path>:/usr/local/myconf/ --link php5.6:php5.6 -d <image_name>
