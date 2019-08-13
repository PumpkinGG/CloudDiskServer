#!/bin/bash

PID_FILE=/usr/local/nginx/logs/nginx.pid

echo '================ nginx ================'

case $1 in
    start)
        if [ -f $PID_FILE ]; then
            sudo nginx -s reload
            echo "nginx reload successfully!"
            exit 0
        fi
        sudo nginx
        echo "nginx start successfully!"
        ;;
    stop)
        if [ ! -f $PID_FILE ]; then
            echo "nginx is not running!"
            exit 1
        fi
        sudo nginx -s quit
        echo "nginx stop successfully!"
        ;;
    *)
        echo "Please input argument:"
        echo '  "start" -- start or reload nginx'
        echo '  "stop"  -- stop nginx'
        ;;
esac

