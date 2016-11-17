#!/bin/bash
set -o nounset

if [ $# != 1 ]; then
    echo "Invalid arguments."
    echo "Try: $0 <Project Name>"
    exit
fi

curr_dir=`pwd`
project_dir="$curr_dir/$1"
petalinux_dir="$curr_dir/$1/petalinux"
repo_dir="$curr_dir/software/petalinux"

# Doing repo copy
cp -r $repo_dir/apps/* $petalinux_dir/components/apps/
cp -r $repo_dir/libs/* $petalinux_dir/components/libs/
cp -r $repo_dir/modules/* $petalinux_dir/components/modules/
cp -r $repo_dir/../generic/* $petalinux_dir/components/generic/

cd $petalinux_dir

# Compiling and preparing the projects
petalinux-build -c rootfs
# petalinux-build
petalinux-package --image
petalinux-package --boot --fsbl ./images/linux/zynq_fsbl.elf --u-boot --force
# petalinux-package --prebuilt --fpga ../${1}.sdk/system_wrapper_hw_platform_0/download.bit --force

# Boot system
# xsdb "$curr_dir/zynq_rst.tcl"
# petalinux-boot --jtag --prebuilt 3