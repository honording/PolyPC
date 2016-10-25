#!/bin/bash
set -o nounset

if [ $# != 2 ]
then
    echo "Invalid arguments."
    echo "Try: $0 <PE per Group> <Acc Name>"
    exit
fi

buf_size=(16 32 64 128)
group_size=(1 2 3 4 5 6 7 8)
pe_per_group=$1
# data_size=(4 8 16 32 64 128 256)

dir_name="sys_bit"

for group in ${group_size[@]}
do
    for buf in ${buf_size[@]}
    do
        hw_num=$((${pe_per_group}*${group}))
        proj_name=${group}g${pe_per_group}s${hw_num}h${2}${buf}buf
        dest_dir=${dir_name}/${proj_name}/
        source_dir=${proj_name}/${proj_name}.sdk/system_wrapper_full_hw_platform_0/download.bit
        mkdir -p ${dest_dir}
        cp -f ${source_dir} ${dest_dir}
    done
done
