#!/bin/sh
pid=$(ps -aux | grep ./server | awk  '{print $2}' | sed -n '1p')
echo $pid
top -p $pid