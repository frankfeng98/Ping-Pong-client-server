#!/bin/bash

packet_size=65536
count=10000
out="sample_test_result.txt"
server="jade.clear.rice.edu"
port=18076

# Test invalid case
./server 1500 >> $out
./server 20000 >> $out
./client $server 1500 100 10 >> $out
./client $server 20000 100 10 >> $out
./client $server $port 10 10 >> $out
./client $server $port 70000 10 >> $out
./client $server $port 1000 12000 >> $out


# Test the client with different packet size and counts (connecting to the Port 18076 on jade CLEAR server)
for (( i=18; i < packet_size; i+= 500 ))
do
    for (( j=1; j < count; j += 200 ))
    do
        ./client $server $port $i $j >> $out
    done
done
