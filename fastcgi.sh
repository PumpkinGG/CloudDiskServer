#!/bin/bash

PID_PATH=temp
PID_FILE=fcgi.pid
PID=$(ps aux | grep logup.cgi | grep -v grep | awk '{print $2}');

make_dir()
{
	if [ ! -d $1 ]; then
	    mkdir $1
        if [ ! -d $1 ]; then
            echo "Create directory failed!"
            exit 1
        fi
    fi
}

start_fcgi()
{
    if [ -f $1 -a -n "$PID" ]; then
        echo "spawn fcgi is running!"
        exit 1
    fi

    if [ -f $1 ]; then
        unlink $1
    fi

    spawn-fcgi -a 127.0.0.1 -p 9000 -f ./bin/logup.cgi | awk '{print $NF}' >> $1
    if [ $? -ne 0 ]; then
        echo "spawn logup.cgi failed!"
        exit 1
    fi
    spawn-fcgi -a 127.0.0.1 -p 9100 -f ./bin/login.cgi | awk '{print $NF}' >> $1
    if [ $? -ne 0 ]; then
        echo "spawn logup.cgi failed!"
        exit 1
    fi
    spawn-fcgi -a 127.0.0.1 -p 9200 -f ./bin/uploadmd5.cgi | awk '{print $NF}' >> $1
    if [ $? -ne 0 ]; then
        echo "spawn logup.cgi failed!"
        exit 1
    fi
    spawn-fcgi -a 127.0.0.1 -p 9300 -f ./bin/upload.cgi | awk '{print $NF}' >> $1
    if [ $? -ne 0 ]; then
        echo "spawn logup.cgi failed!"
        exit 1
    fi
    spawn-fcgi -a 127.0.0.1 -p 9400 -f ./bin/userfile.cgi | awk '{print $NF}' >> $1
    if [ $? -ne 0 ]; then
        echo "spawn logup.cgi failed!"
        exit 1
    fi
    echo "spawn cgi processes successfully!"
}

stop_fcgi()
{
    if [ ! -f $1 ]; then
        echo "fastcgi is not running!"
        exit 1
    fi
    kill -9 $(cat $1)
    echo "fastcgi stop successfully!"
    unlink $1
}

# main process
echo '=============== fastcgi ==============='

case $1 in
    start)
        make_dir $PID_PATH
        start_fcgi $PID_PATH/$PID_FILE
        ;;
    stop)
        stop_fcgi $PID_PATH/$PID_FILE
        ;;
    *)
        echo 'Please input argument:'
        echo '  "start" -- start spawn fcgi'
        echo '  "stop"  -- stop spawn fcgi'
esac

