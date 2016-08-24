#!/bin/bash
set -o nounset

if [ $# != 1 ]; then
	echo "Invalid arguments."
	echo "Try: $0 <Project Name>"
	exit
fi

curr_dir=`pwd`
project_dir="$curr_dir/$1"
bitstream_dir="$curr_dir/$1/bitstream"

if [ ! -d $bitstream_dir ]; then
	echo "ERROR: No bitstream forlder: $bitstream_dir exists."
	exit
fi

for dir in `ls $bitstream_dir`
do
	if [[ $dir == *.bit ]]; then
		continue
	fi
	bin_dir="$bitstream_dir/$dir"
	num_pr=`ls -l $bin_dir/*.bin | grep -v ^l | wc -l`
	size_pr=`wc -c < $bin_dir/pr00.bin`
	echo $num_pr >> $bin_dir/info
	echo $size_pr >> $bin_dir/info
	echo "Generate $bin_dir/info file"
	end=`expr $num_pr - 1`
	for i in `seq -f "%02g" 0 $end`
	do
		bin_name="$bin_dir/pr${i}.bin"
		echo "Combile $bin_name"
		cat $bin_name >> "$bin_dir/pr.bin"
	done
done

