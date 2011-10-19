#!/bin/sh
# ---------------------------------------------------------
#  Set the core module defines according to Core Module
# ---------------------------------------------------------
# ---------------------------------------------------------
# Set up the Versatile type define
# ---------------------------------------------------------
mkdir -p ${obj}include
variant=versatilepb

if [ "$1" == "" ]
then
	echo "$0:: No parameters - using versatilepb_config"
	echo "#define CONFIG_ARCH_VERSATILE_PB" > ${obj}include/config.h
	variant=versatilepb
else
	case "$1" in
	versatile_config)
	echo "#define CONFIG_ARCH_VERSATILE_PB" > ${obj}include/config.h
	;;

	versatilepb_config)
	echo "#define CONFIG_ARCH_VERSATILE_PB" > ${obj}include/config.h
	;;

	versatileab_config)
	echo "#define CONFIG_ARCH_VERSATILE_AB" > ${obj}include/config.h
	variant=versatileab
	;;


	*)
	echo "$0:: Unrecognised config - using versatilepb_config"
	echo "#define CONFIG_ARCH_VERSATILE_PB" > ${obj}include/config.h
	variant=versatilepb
	;;

	esac

fi


# ---------------------------------------------------------
# Link variant header to versatile.h
# ---------------------------------------------------------
if [ "$variant" != "versatile" ] 
then
	if [ -h ./include/configs/$variant.h ] 
	then
		rm ./include/configs/$variant.h
	fi
	ln -s ./versatile.h ./include/configs/$variant.h
fi

# ---------------------------------------------------------
# Complete the configuration
# ---------------------------------------------------------
$MKCONFIG -a versatile arm arm926ejs versatile armltd versatile
echo "Variant:: $variant"
