#!/usr/bin/env sh

script_file=$0
cpu_num=512

do_hotplug()
{
    echo $0
    online_file=$1
    echo 1 > $online_file
    sleep 10
    while true; do
        echo 0 > $online_file
        sleep $(shuf -i 0-3 -n 1)
        echo 1 > $online_file
        sleep $(shuf -i 0-20 -n 1)
    done
}

if [ $# -eq 1 ]; then
    cpu_num=$1
fi

trap ctrl_c INT

pids=()

function ctrl_c() {
    echo "** Trapped CTRL-C"
    # wait for all pids
    fuser $script_file -k
    for file in /sys/devices/system/cpu/cpu*/online
    do
        echo 1 > $file
    done
}

if [ x$cpu_num == x512 ]; then
    i=0
    for file in /sys/devices/system/cpu/cpu*/online
    do
        cmd="$script_file $file"
        echo "start hotplug on $file"
        $cmd &
        pids[${i}]=$!
        ((i=i+1))
    done
    for pid in ${pids[*]}; do
        wait $pid
    done
else
    do_hotplug $cpu_num
fi
