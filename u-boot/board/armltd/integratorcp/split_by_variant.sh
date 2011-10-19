#!/bin/sh
# ---------------------------------------------------------
# Set the platform defines
# ---------------------------------------------------------
echo -n "/* Integrator configuration implied "   > tmp.fil
echo    " by Makefile target */"		>> tmp.fil
echo -n "#define CONFIG_INTEGRATOR"		>> tmp.fil
echo	 " /* Integrator board */"		>> tmp.fil
echo -n "#define CONFIG_ARCH_CINTEGRATOR"	>> tmp.fil
echo     " 1 /* Integrator/CP   */"  		>> tmp.fil
# ---------------------------------------------------------
#	Set the core module defines according to Core Module
# ---------------------------------------------------------
cpu="arm_intcm"
variant="integratorcp"

if [ "$1" = "" ]
then
	echo "$0:: No parameters - using arm_intcm"
else
	variant=`echo $1 | sed -r s/_config//`
	
	case "$1" in
	cp966_config)
	cpu="arm_intcm"
	;;

	cp922_config)
	cpu="arm_intcm"
	;;

	integratorcp_config)
	cpu="arm_intcm"
	;;

	cp922_XA10_config)
	cpu="arm_intcm"
	echo -n "#define CONFIG_CM922T_XA10" 		>> tmp.fil
	echo    " 1 /* CPU core is ARM922T_XA10 */" 	>> tmp.fil
	;;

	cp920t_config)
	cpu="arm920t"
	echo -n "#define CONFIG_CM920T" 		>> tmp.fil
	echo    " 1 /* CPU core is ARM920T */"		>> tmp.fil
	;;

	cp926ejs_config)
	cpu="arm926ejs"
	echo -n "#define CONFIG_CM926EJ_S"		>> tmp.fil
	echo    " 1 /* CPU core is ARM926EJ-S */ "	>> tmp.fil
	;;


	cp946es_config)
	cpu="arm946es"
	echo -n "#define CONFIG_CM946E_S"		>> tmp.fil
	echo    " 1 /* CPU core is ARM946E-S */ "	>> tmp.fil
	;;

	cp1026_config)
	cpu="arm_intcm"
	echo -n "#define CONFIG_CM1026EJ_S"		>> tmp.fil
	echo    " 1 /* CPU core is ARM1026J-S */ "	>> tmp.fil
	;;

	cp1136_config)
	cpu="arm1136"
	echo -n "#define CONFIG_CM1136EJF_S"		>> tmp.fil
	echo    " 1 /* CPU core is ARM1136JF-S */ "	>> tmp.fil
	;;

	cp1176_rt_config)
	cpu="arm_intcm"
	echo -n "#define CONFIG_CM1176JFZ_S"		>> tmp.fil
	echo    " 1 /* CPU core is ARM1136JF-S */ "	>> tmp.fil
	echo -n "#define ARM_1176_CP_RT"		>> tmp.fil
	echo    " 1 /* FastSIM model */ "	>> tmp.fil
	;;

	
	*)
	echo "$0:: Unknown core module"
	cpu="arm_intcm"
	;;

	esac

fi

# ---------------------------------------------------------
# Link variant header to integratorcp.h
# ---------------------------------------------------------
if [ "$variant" != "integratorcp" ] 
then
	if [ -h ./include/configs/$variant.h ] 
	then
		rm ./include/configs/$variant.h
	fi
	ln -s ./integratorcp.h ./include/configs/$variant.h
fi

if [ "$cpu" = "arm_intcm" ]
then
	echo "/* Core module undefined/not ported */"	>> tmp.fil
	echo "#define CONFIG_ARM_INTCM 1"		>> tmp.fil
	echo -n "#undef CONFIG_CM_MULTIPLE_SSRAM"	>> tmp.fil
	echo -n "  /* CM may not have "			>> tmp.fil
	echo    "multiple SSRAM mapping */"		>> tmp.fil
	echo -n "#undef CONFIG_CM_SPD_DETECT "		>> tmp.fil
	echo -n " /* CM may not support SPD "		>> tmp.fil
	echo    "query */"				>> tmp.fil
	echo -n "#undef CONFIG_CM_REMAP  "		>> tmp.fil
	echo -n " /* CM may not support "		>> tmp.fil
	echo    "remapping */"				>> tmp.fil
	echo -n "#undef CONFIG_CM_INIT  "		>> tmp.fil
	echo -n " /* CM may not have  "			>> tmp.fil
	echo    "initialization reg */"			>> tmp.fil
	echo -n "#undef CONFIG_CM_TCRAM  "		>> tmp.fil
	echo    " /* CM may not have TCRAM */"		>> tmp.fil
fi

mkdir -p ${obj}include
mkdir -p ${obj}board/armltd/integratorcp
mv tmp.fil ${obj}include/config.h
# ---------------------------------------------------------
#  Ensure correct core object loaded first in U-Boot image
# ---------------------------------------------------------
sed -r 's/CPU_FILE/cpu\/'$cpu'\/start.o/; s/#.*//' ${src}board/armltd/integratorcp/u-boot.lds.template > ${obj}board/armltd/integratorcp/u-boot.lds
# ---------------------------------------------------------
# Complete the configuration
# ---------------------------------------------------------
$MKCONFIG -a $variant arm $cpu integratorcp armltd;
echo "Variant:: $variant with core $cpu"

