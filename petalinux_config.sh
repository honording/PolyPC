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

if [ ! -d $petalinux_dir ]; then
	echo "Warning: No petalinux forlder: $petalinux_dir exists."
	echo "Creating petalinux project folder."
	cd $project_dir
	petalinux-create -t project -n petalinux --template zynq
	cd $petalinux_dir
	petalinux-config --get-hw-description=../${1}.sdk --oldconfig
fi

# Creating generic/include
petalinux-create -t generic -n include --enable --force

# Creating modules
for module in `ls $repo_dir/modules`
do
	petalinux-create -t modules -n $module --enable --force
done

# Creating libs
for lib in `ls $repo_dir/libs`
do
	petalinux-create -t libs -n $lib --enable --force
done

# Creating apps
for app in `ls $repo_dir/apps`
do
	petalinux-create -t apps -n $app --enable --force
done

# Doing repo copy
cp -r $repo_dir/apps/* $petalinux_dir/components/apps/
cp -r $repo_dir/libs/* $petalinux_dir/components/libs/
cp -r $repo_dir/modules/* $petalinux_dir/components/modules/
cp -r $repo_dir/../generic/* $petalinux_dir/components/generic/

echo "Copy Finished."

# Copy psinit files
cp $petalinux_dir/subsystems/linux/hw-description/psinit/* $petalinux_dir/subsystems/linux/hw-description/

# Compiling and preparing the projects
petalinux-build
petalinux-package --image
petalinux-package --boot --fsbl ./images/linux/zynq_fsbl.elf --u-boot --force
# petalinux-package --prebuilt --fpga ../${1}.sdk/system_wrapper_hw_platform_0/download.bit --force

echo "Petalinux Compiling Done."
