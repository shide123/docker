#!/bin/bash
#nginx
docker run  --name nginx -p 80:80 -p 8081:8081 -p 443:443 -v /www:/www --link php5.6 -d nginx/nginx
#php
docker run --name php5.6 -p 9000:9000 -v /www:/www -v /usr/local/lnmp/docker-file/php5.6:/usr/local/myconf -d php5.6/php5.6

