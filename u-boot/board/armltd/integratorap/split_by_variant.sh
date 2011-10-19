#!/bin/sh
# ---------------------------------------------------------
# Set the platform defines
# ---------------------------------------------------------
echo -n	"/* Integrator configuration implied "	 > tmp.fil
echo	" by Makefile target */"		>> tmp.fil
echo -n	"#define CONFIG_INTEGRATOR"		>> tmp.fil
echo	" /* Integrator board */"		>> tmp.fil
echo -n	"#define CONFIG_ARCH_INTEGRATOR"	>> tmp.fil
echo	" 1 /* Integrator/AP	 */"		>> tmp.fil
# ---------------------------------------------------------
#	Set the core module defines according to Core Module
# ---------------------------------------------------------
cpu="arm_intcm"
variant="integratorap"

if [ "$1" = "" ]
then
	echo "$0:: No parameters - using arm_intcm"
else
	variant=`echo $1 | sed -r s/_config//`

	case "$1" in
	ap7t_config)
	cpu="arm_intcm"
	;;

	ap966_config)
	cpu="arm_intcm"
	;;

	ap922_config)
	cpu="arm_intcm"
	;;

	integratorap_config)
	cpu="arm_intcm"
	;;

	ap720t_config)
	cpu="arm720t"
	echo -n	"#define CONFIG_CM720T"		>> tmp.fil
	echo	" 1 /* CPU core is ARM720T */ "	>> tmp.fil
	;;

	ap922_XA10_config)
	cpu="arm_intcm"
	echo -n	"#define CONFIG_CM922T_XA10" 		>> tmp.fil
	echo	" 1 /* CPU core is ARM922T_XA10 */" 	>> tmp.fil
	;;

	ap920t_config)
	cpu="arm920t"
	echo -n	"#define CONFIG_CM920T" 		>> tmp.fil
	echo	" 1 /* CPU core is ARM920T */"		>> tmp.fil
	;;

	ap926ejs_config)
	cpu="arm926ejs"
	echo -n	"#define CONFIG_CM926EJ_S"		>> tmp.fil
	echo	" 1 /* CPU core is ARM926EJ-S */ "	>> tmp.fil
	;;

	ap946es_config)
	cpu="arm946es"
	echo -n	"#define CONFIG_CM946E_S"		>> tmp.fil
	echo	" 1 /* CPU core is ARM946E-S */ "	>> tmp.fil
	;;

	*)
	echo "$0:: Unknown core module"
	variant="integratorap"
	cpu="arm_intcm"
	;;

	esac
fi
# ---------------------------------------------------------
# Link variant header to integratorap.h
# ---------------------------------------------------------
if [ "$variant" != "integratorap" ] 
then
	if [ -h ./include/configs/$variant.h ] 
	then
		rm ./include/configs/$variant.h
	fi
	ln -s ./integratorap.h ./include/configs/$variant.h
fi

if [ "$cpu" = "arm_intcm" ]
then
	echo "/* Core module undefined/not ported */"	>> tmp.fil
	echo "#define CONFIG_ARM_INTCM 1"		>> tmp.fil
	echo -n	"#undef CONFIG_CM_MULTIPLE_SSRAM"	>> tmp.fil
	echo -n	"	/* CM may not have "		>> tmp.fil
	echo	"multiple SSRAM mapping */"		>> tmp.fil
	echo -n	"#undef CONFIG_CM_SPD_DETECT "		>> tmp.fil
	echo -n	" /* CM may not support SPD "		>> tmp.fil
	echo	"query */"				>> tmp.fil
	echo -n	"#undef CONFIG_CM_REMAP	"		>> tmp.fil
	echo -n	" /* CM may not support "		>> tmp.fil
	echo	"remapping */"				>> tmp.fil
	echo -n	"#undef CONFIG_CM_INIT	"		>> tmp.fil
	echo -n	" /* CM may not have	"		>> tmp.fil
	echo	"initialization reg */"			>> tmp.fil
	echo -n	"#undef CONFIG_CM_TCRAM	"		>> tmp.fil
	echo	" /* CM may not have TCRAM */"		>> tmp.fil
fi

mkdir -p ${obj}include
mkdir -p ${obj}board/armltd/integratorap
mv tmp.fil ${obj}include/config.h
# ---------------------------------------------------------
#	Ensure correct core object loaded first in U-Boot image
# ---------------------------------------------------------
sed -r 's/CPU_FILE/cpu\/'$cpu'\/start.o/; s/#.*//' ${src}board/armltd/integratorap/u-boot.lds.template > ${obj}board/armltd/integratorap/u-boot.lds
# ---------------------------------------------------------
# Complete the configuration
# ---------------------------------------------------------
$MKCONFIG -a $variant arm $cpu integratorap armltd;
echo "Variant:: $variant with core $cpu"

