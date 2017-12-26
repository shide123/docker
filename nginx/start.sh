#!/bin/bash
nginx -c /usr/local/myconf/nginx.conf && nohup /usr/local/logstash/bin/logstash -f /usr/local/myconf/logstash.conf &
