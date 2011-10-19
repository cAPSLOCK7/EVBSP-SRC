#!/bin/sh
# ---------------------------------------------------------
#  Set the core module defines according to Core Module
# ---------------------------------------------------------
config="$1"
# ---------------------------------------------------------
# Set up the Realview type define
# ---------------------------------------------------------
# Use this until later versions provide distinct core tiles
# ---------------------------------------------------------
cpu=armct_rv
if [ "$1" = "" ]
then
	echo "$0:: No parameters - using arm-linux-gcc realview_pb1176_config"
else
	case "$config" in

	realview_pb1176_config)
	echo "#define CONFIG_REALVIEW_PB1176" > ./include/config.h
	;;

	realview_pb11mp_config)
	echo "#define CONFIG_REALVIEW_PB11MP" > ./include/config.h
	;;

	realview_pba8_config)
	echo "#define CONFIG_MACH_REALVIEW_PBA8" > ./include/config.h
	;;

	realview_pbx_config)
	echo "#define CONFIG_MACH_REALVIEW_PBX" > ./include/config.h
	;;

	*)
	echo "$0:: Unrecognised config - using realview_pb1176_config"
	echo "#define CONFIG_REALVIEW_PB1176" > ./include/config.h
	;;

	esac

fi
# ---------------------------------------------------------
#	Ensure correct core object loaded first in U-Boot image
# ---------------------------------------------------------
sed -r 's/CPU_FILE/cpu\/'$cpu'\/start.o/; s/#.*//' board/armltd/realview_pb/u-boot.lds.template > board/armltd/realview_pb/u-boot.lds
# ---------------------------------------------------------
# Complete the configuration
# ---------------------------------------------------------
./mkconfig -a realview_pb arm $cpu realview_pb armltd
