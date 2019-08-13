#!/bin/bash

CONFIG_PATH=/etc/fdfs
PID=$(ps aux | grep fdfs | grep -v grep | awk '{print $2}')

echo '=============== fastdfs ==============='

case $1 in
    start)
        if [ ! -z "$PID" ]; then
            echo "fastdfs is running!"
            exit 1
        fi
        fdfs_trackerd /etc/fdfs/tracker.conf
        fdfs_storaged /etc/fdfs/storage.conf
        echo "fastdfs start successfully!"
        ;;
    stop)
        if [ -z "$PID" ]; then
            echo "fastdfs is not running!"
            exit 1
        fi
        fdfs_trackerd /etc/fdfs/tracker.conf stop
        fdfs_storaged /etc/fdfs/storage.conf stop
        echo "fastdfs stop successfully!"
        ;;
    *)
        echo 'Please input argument:'
        echo '  "start" -- start fdfs_tracker && fdfs_storage'
        echo '  "stop"  -- stop fdfs_tracker && fdfs_storage'
        ;;
esac
        
