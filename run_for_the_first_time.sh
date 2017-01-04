#!/bin/bash
set -o nounset

if [ $# != 5 ] && [ $# != 4 ]
then
	echo "Invalid arguments."
	echo "Try: $0 <Project Name> <Number of Group> <Number of Total Slaves per Group> <Number of Total HW Slaves> (Name of HW Slave)"
	exit
fi

echo "██╗  ██╗ █████╗ ██████╗  █████╗ ██████╗  █████╗      ██████╗ ███████╗███╗   ██╗"
echo "██║  ██║██╔══██╗██╔══██╗██╔══██╗██╔══██╗██╔══██╗    ██╔════╝ ██╔════╝████╗  ██║"
echo "███████║███████║██████╔╝███████║██████╔╝███████║    ██║  ███╗█████╗  ██╔██╗ ██║"
echo "██╔══██║██╔══██║██╔═══╝ ██╔══██║██╔══██╗██╔══██║    ██║   ██║██╔══╝  ██║╚██╗██║"
echo "██║  ██║██║  ██║██║     ██║  ██║██║  ██║██║  ██║    ╚██████╔╝███████╗██║ ╚████║"
echo "╚═╝  ╚═╝╚═╝  ╚═╝╚═╝     ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝     ╚═════╝ ╚══════╝╚═╝  ╚═══╝"

if [ -d ./$1 ]; then
	echo "Warning: Project $1 exists."
	exit
fi

echo "██╗  ██╗██╗     ███████╗     ██████╗ ███████╗███╗   ██╗"
echo "██║  ██║██║     ██╔════╝    ██╔════╝ ██╔════╝████╗  ██║"
echo "███████║██║     ███████╗    ██║  ███╗█████╗  ██╔██╗ ██║"
echo "██╔══██║██║     ╚════██║    ██║   ██║██╔══╝  ██║╚██╗██║"
echo "██║  ██║███████╗███████║    ╚██████╔╝███████╗██║ ╚████║"
echo "╚═╝  ╚═╝╚══════╝╚══════╝     ╚═════╝ ╚══════╝╚═╝  ╚═══╝"

# vivado_hls -f hls_config.tcl all


echo "██╗  ██╗██╗    ██╗    ███████╗██████╗  █████╗ ███╗   ███╗███████╗██╗    ██╗ ██████╗ ██████╗ ██╗  ██╗"
echo "██║  ██║██║    ██║    ██╔════╝██╔══██╗██╔══██╗████╗ ████║██╔════╝██║    ██║██╔═══██╗██╔══██╗██║ ██╔╝"
echo "███████║██║ █╗ ██║    █████╗  ██████╔╝███████║██╔████╔██║█████╗  ██║ █╗ ██║██║   ██║██████╔╝█████╔╝ "
echo "██╔══██║██║███╗██║    ██╔══╝  ██╔══██╗██╔══██║██║╚██╔╝██║██╔══╝  ██║███╗██║██║   ██║██╔══██╗██╔═██╗ "
echo "██║  ██║╚███╔███╔╝    ██║     ██║  ██║██║  ██║██║ ╚═╝ ██║███████╗╚███╔███╔╝╚██████╔╝██║  ██║██║  ██╗"
echo "╚═╝  ╚═╝ ╚══╝╚══╝     ╚═╝     ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝ ╚══╝╚══╝  ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝"

if [ $# == 4 ]
then
    echo "Generate Framework with PR"
    vivado -mode batch -source generate.tcl -tclargs $1 $2 $3 $4
else 
    echo "Generate Framework without PR. ACC Name: $5"
    vivado -mode batch -source generate.tcl -tclargs $1 $2 $3 $4 $5
fi

echo "██████╗ ██████╗      ██████╗ ██████╗ ███╗   ███╗██████╗ ██╗███╗   ██╗███████╗"
echo "██╔══██╗██╔══██╗    ██╔════╝██╔═══██╗████╗ ████║██╔══██╗██║████╗  ██║██╔════╝"
echo "██████╔╝██████╔╝    ██║     ██║   ██║██╔████╔██║██████╔╝██║██╔██╗ ██║█████╗  "
echo "██╔═══╝ ██╔══██╗    ██║     ██║   ██║██║╚██╔╝██║██╔══██╗██║██║╚██╗██║██╔══╝  "
echo "██║     ██║  ██║    ╚██████╗╚██████╔╝██║ ╚═╝ ██║██████╔╝██║██║ ╚████║███████╗"
echo "╚═╝     ╚═╝  ╚═╝     ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚═════╝ ╚═╝╚═╝  ╚═══╝╚══════╝"

./combine_pr.sh $1

echo "███████╗██████╗ ██╗  ██╗     ██████╗ ███████╗███╗   ██╗"
echo "██╔════╝██╔══██╗██║ ██╔╝    ██╔════╝ ██╔════╝████╗  ██║"
echo "███████╗██║  ██║█████╔╝     ██║  ███╗█████╗  ██╔██╗ ██║"
echo "╚════██║██║  ██║██╔═██╗     ██║   ██║██╔══╝  ██║╚██╗██║"
echo "███████║██████╔╝██║  ██╗    ╚██████╔╝███████╗██║ ╚████║"
echo "╚══════╝╚═════╝ ╚═╝  ╚═╝     ╚═════╝ ╚══════╝╚═╝  ╚═══╝"

xsdk -batch -source sdk_config.tcl $1 $2 $3 $4

echo "██████╗ ███████╗████████╗ █████╗ ██╗     ██╗███╗   ██╗██╗   ██╗██╗  ██╗     ██████╗ ███████╗███╗   ██╗"
echo "██╔══██╗██╔════╝╚══██╔══╝██╔══██╗██║     ██║████╗  ██║██║   ██║╚██╗██╔╝    ██╔════╝ ██╔════╝████╗  ██║"
echo "██████╔╝█████╗     ██║   ███████║██║     ██║██╔██╗ ██║██║   ██║ ╚███╔╝     ██║  ███╗█████╗  ██╔██╗ ██║"
echo "██╔═══╝ ██╔══╝     ██║   ██╔══██║██║     ██║██║╚██╗██║██║   ██║ ██╔██╗     ██║   ██║██╔══╝  ██║╚██╗██║"
echo "██║     ███████╗   ██║   ██║  ██║███████╗██║██║ ╚████║╚██████╔╝██╔╝ ██╗    ╚██████╔╝███████╗██║ ╚████║"
echo "╚═╝     ╚══════╝   ╚═╝   ╚═╝  ╚═╝╚══════╝╚═╝╚═╝  ╚═══╝ ╚═════╝ ╚═╝  ╚═╝     ╚═════╝ ╚══════╝╚═╝  ╚═══╝"


./petalinux_config.sh $1

echo " ██████╗██╗     ███████╗ █████╗ ███╗   ██╗██╗   ██╗██████╗ "
echo "██╔════╝██║     ██╔════╝██╔══██╗████╗  ██║██║   ██║██╔══██╗"
echo "██║     ██║     █████╗  ███████║██╔██╗ ██║██║   ██║██████╔╝"
echo "██║     ██║     ██╔══╝  ██╔══██║██║╚██╗██║██║   ██║██╔═══╝ "
echo "╚██████╗███████╗███████╗██║  ██║██║ ╚████║╚██████╔╝██║     "
echo " ╚═════╝╚══════╝╚══════╝╚═╝  ╚═╝╚═╝  ╚═══╝ ╚═════╝ ╚═╝     "

# ./clean_up.sh
