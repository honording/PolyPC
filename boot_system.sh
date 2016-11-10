#!/bin/bash
set -o nounset

if [ $# != 1 ]; then
	echo "Invalid arguments."
	echo "Try: $0 <Project Name>"
	exit
fi

curr_dir=`pwd`
proj_dir="$curr_dir/$1"
petalinux_dir="$curr_dir/$1/petalinux"

cd $petalinux_dir

# petalinux-package --image -c kernel --format zImage
# petalinux-package --image
# petalinux-package --boot --fsbl ./images/linux/zynq_fsbl.elf --u-boot --force

# petalinux-package --prebuilt --fpga $proj_dir/${1}.sdk/system_wrapper_hw_platform_0/download.bit --force
petalinux-package --prebuilt --fpga $proj_dir/${1}.sdk/system_wrapper_full_hw_platform_0/download.bit --force
# petalinux-package --prebuilt --fpga /home/hding/Projects/HaPara/200M_1g4s4hpageranking/200M_1g4s4hpageranking.sdk/download.bit --force
xsdb "$curr_dir/zynq_rst.tcl"
petalinux-boot --jtag --prebuilt 3
