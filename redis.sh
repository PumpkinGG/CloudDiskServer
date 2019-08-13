#!/bin/bash

PID=$(ps aux | grep redis-server | grep -v grep | awk '{print $2}')

echo '================ redis ================'

case $1 in
    start)
        if [ ! -z "$PID" ]; then 
            echo "redis server is running!";
            exit 1
        fi
        redis-server ./config/redis.conf
        if [ $? -eq 0 ]; then
            echo "redis-server start successfully!"
        fi
        ;;
    stop)
        if [ -z "$PID" ]; then
            echo "redis is not running!"
            exit 1
        fi
        kill -9 $PID
        echo "redis stop successfully!"
        ;;
    *)
        echo "Please input argument:"
        echo '  "start" -- start redis server'
        echo '  "stop"  -- stop redis server'
        ;;
esac

