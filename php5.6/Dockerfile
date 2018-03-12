FROM centos:centos7
MAINTAINER lishide <376189717@qq.com>

ENV TZ "Asia/Shanghai"
#env install
RUN yum -y install epel-release
RUN rpm -ivh http://rpms.famillecollet.com/enterprise/remi-release-7.rpm
RUN yum -y install --enablerepo=remi --enablerepo=remi-php56 php php-opcache php-devel php-mbstring php-mcrypt php-mysqlnd php-phpunit-PHPUnit php-pecl-xdebug php-pecl-xhprof php-redis php-mongodb php-fpm

RUN mkdir /www
RUN yum install nfs-utils -y 
#ADD php5.6/php-fpm.conf /usr/local/myconf/php-fpm.conf
#ADD php5.6/php.ini /usr/local/myconf/php.ini
ADD php5.6/php-fpm.d/www.conf /etc/php-fpm.d/www.conf

EXPOSE 9000

#start container
#docker run --name php5.6  -v /www:/www -v /usr/local/lnmp/docker-file/php5.6:/usr/local/myconf -d php5.6/php5.6

ENTRYPOINT ["php-fpm", "-F"]
