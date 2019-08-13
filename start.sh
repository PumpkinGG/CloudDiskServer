#!/bin/bash

case $1 in
    start)
        ./nginx.sh start
        ./redis.sh start
        ./fastdfs.sh start
        ./fastcgi.sh start
        ;;
    stop)
        ./nginx.sh stop
        ./redis.sh stop
        ./fastdfs.sh stop
        ./fastcgi.sh stop
        ;;
    *)
        echo 'Please input correct argument:'
        echo '  "start" -- start cloud disk server service'
        echo '  "stop" -- stop cloud disk server service'
        ;;
esac
        
