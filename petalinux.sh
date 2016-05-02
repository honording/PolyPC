#!/bin/bash
set -o nounset

if [ $# != 1 ]; then
	echo "Invalid arguments."
	echo "Try: $0 <Project Name>"
	exit
fi


for file in `ls ./software/petalinux/modules`
do
	echo $file
done

name="qinjx"
greeting="hello, "$name" !"
echo $greeting
greeting2="hello, ${name} !"
echo $greeting2

echo $#