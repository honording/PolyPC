#!/bin/bash
set -o nounset

if [ $# != 3 ]
then
    echo "Invalid arguments."
    echo "Try: $0 <PE per Group> <Acc Name> <Prefix>"
    exit
fi

# buf_size=(1 8 16 32 64 128)
buf_size=(1)
group_size=(1 2 3 4 5 6 7 8)
# group_size=1
pe_per_group=$1
# data_size=(4 8 16 32 64 128 256)
data_size=(2 4 8 16 32 63)

dir_name="/mnt/sys_bit"
output_file="/mnt/testscript/performance.txt"

num_loops=8

for buf in ${buf_size[@]}
do
    for group in ${group_size[@]}
    do
        hw_num=$((${pe_per_group}*${group}))
        proj_name=${3}${group}g${pe_per_group}s${hw_num}h${2}${buf}buf
        dest_bit=${dir_name}/${proj_name}/download.bit
        cat ${dest_bit} > /dev/xdevcfg
        soft_group=1
        echo "Downloaded $proj_name"
        echo "$proj_name" >> $output_file
        while [ $soft_group -lt $group ]
        do
            soft_group=$((${soft_group}*2))
        done
        echo "App Group Num: ${soft_group}"
        echo "App Group Num: ${soft_group}" >> $output_file
        for size in ${data_size[@]}
        do
            cumulative_time=0.0
            for i in `seq 1 ${num_loops}`
            do
                temp_time=$(/bin/apptest ${soft_group} ${size} ${buf})
                cumulative_time=$(echo $temp_time $cumulative_time | awk '{printf "%f\n" ,$1*1000000+$2}')
            done
            average_time=$(echo $cumulative_time $num_loops | awk '{printf "%0.0f\n" ,$1/$2}')
            echo "${size}K: ${average_time}us"
            echo "${average_time}" >> $output_file
        done
    done
done

