#!/bin/sh

mkfifo /tmp/test.fifo

for i in `seq 10000`
do
    echo $i > /tmp/test.fifo
    sleep 1
done
