FROM centos:centos7
MAINTAINER lishide <376190717@qq.com>

ENV TZ "Asia/Shanghai"


#create config path
RUN mkdir -p /usr/local/myconf

RUN rpm -Uvh http://nginx.org/packages/centos/7/noarch/RPMS/nginx-release-centos-7-0.el7.ngx.noarch.rpm
RUN yum -y install nginx

RUN mkdir /www

#add nginx.conf
ADD nginx/nginx.conf /etc/nginx/nginx.conf
#ADD nginx/fastcgi_params /usr/local/myconf/fastcgi_params
#add start script
#add extend  port
EXPOSE 80 8081 443


#CMD ["/bin/sh","/tmp/start.sh"]
ENTRYPOINT ["nginx","-g", "daemon off;"]

#start container: docker 
##docker run --name nginx -v <config_path>:/usr/local/myconf/ --link php5.6:php5.6 -d <image_name>
