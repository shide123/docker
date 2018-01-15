FROM centos:centos7
MAINTAINER lishide <376189717@qq.com>

ENV TZ "Asia/Shanghai"
#env install
RUN rpm -Uvh https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
RUN yum install -y php php-opcache php-pecl-apcu php-devel php-mbstring php-mcrypt php-mysqlnd php-phpunit-PHPUnit php-pecl-xdebug php-pecl-xhprof php-pdo php-pear php-fpm php-cli php-xml php-bcmath php-process php-gd php-common php-mongodb php-redis

#make dir
RUN mkdir -p /usr/local/php/etc/php-fpm.d/  && mkdir -p /var/log/php-fpm/ && mkdir -p /var/run/php-fpm/  && mkdir -p /usr/local/myconf/php-fpm.d/

#ADD php confile
ADD php5.6/php-fpm.conf /usr/local/myconf/php-fpm.conf
ADD php5.6/php.ini /usr/local/myconf/php.ini
ADD php5.6/php-fpm.d/www.conf /usr/local/myconf/php-fpm.d/www.conf


EXPOSE 9000

#start container
#docker run --name php5.6  -v /www:/www -v /usr/local/lnmp/docker-file/php5.6:/usr/local/myconf -d php5.6/php5.6

ENTRYPOINT ["php-fpm", "-F","-c","/usr/local/myconf/php.ini", "-y","/usr/local/myconf/php-fpm.conf"]
