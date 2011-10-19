/*
 * (C) Copyright 2008
 * NEC Electronics Corporation.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#if defined(CONFIG_EMXX_EMMCBOOT) || defined(CONFIG_EMXX_SDBOOT)
#define CONFIG_SKIP_RELOCATE_UBOOT
#define CONFIG_EMXX_MMCBOOT
#endif
#if defined(CONFIG_EM1_BOARD_DKIT)
#define CONFIG_EM1_EMMC_1Piece		1	/* 0:eMMC 2pieces 1:eMMC 1piece */
#else
#define CONFIG_EM1_EMMC_1Piece		0	/* 0:eMMC 2pieces 1:eMMC 1piece */
#endif

/*
 * High Level Configuration Options
 */
#define CONFIG_ARM1136			1	/* This is an arm1136 CPU core  */
#define CONFIG_EMXX			1	/* EMMA Mobile series */
#define CONFIG_MACH_EM1			1

#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */

#undef CONFIG_USE_IRQ			/* no support for IRQs */

#define CONFIG_DISPLAY_BOARDINFO
#define CONFIG_AUTO_COMPLETE

#define CONFIG_SYS_HELP_CMD_WIDTH	10


/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 128*1024)
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */


/*-----------------------------------------------------------------------
 * Hardware drivers
 */

/* ---- Configuration for the Ethernet Driver (LAN9118) ---- */
#if defined(CONFIG_EM1_BOARD_DKIT)
#define CONFIG_DRIVER_SMSC9118
#define CONFIG_SMSC9118_BASE		0x20000000
#else
#define CONFIG_DRIVER_SMC91111
#define CONFIG_SMC91111_BASE		0x20000300
#endif

/* ---- Configuration for the Console Driver (NS16550) ---- */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(4)
#ifdef CONFIG_EM1_PLL3_238MHZ
#define CONFIG_SYS_NS16550_CLK		(237568000/2)	/* PLL3(237.567MHz) divided by 2 */
#else
#define CONFIG_SYS_NS16550_CLK		(229376000/2)	/* PLL3(229.376MHz) divided by 2 */
#endif
#define CONFIG_SYS_NS16550_COM1		0x50000000
#define CONFIG_CONS_INDEX		1

#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE


/*-----------------------------------------------------------------------
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING

#undef CONFIG_CMD_FPGA		/* FPGA configuration Support	*/
#undef CONFIG_CMD_SETGETDCR	/* DCR support on 4xx		*/


/*-----------------------------------------------------------------------
 * Environment information
 */
#define CONFIG_BOOTDELAY	3

#if defined(CONFIG_EMXX_EMMCBOOT)
#define CONFIG_EXT3_ROOT	"/dev/mmcblk0p3"	/* emmc-boot */
#elif CONFIG_EM1_EMMC_1Piece
#define CONFIG_EXT3_ROOT	"/dev/mmcblk1p3"	/* sd-boot (emmc 1 device) */
#else
#define CONFIG_EXT3_ROOT	"/dev/mmcblk2p3"	/* sd-boot (emmc 2 device) */
#endif

#ifdef CONFIG_EM1_DDR_1GBit	/* defined DDR 1Gbit(128MByte) */
#define CONFIG_DDR		"mem=32M@0x30000000 mem=32M@0x34000000 mem=32M@0x36000000"	/* 96MByte */

#define CONFIG_BOOTARGS		"root=/dev/null noinitrd init=/linuxrc console=ttyS0,115200n8n SELINUX_INIT=no \$(cfg_ddr) ro video=qfb: ip=none rootflags=physaddr=0x00400000"
#else
#define CONFIG_DDR		"mem=32M@0x30000000"	/* 32MByte */

#define CONFIG_BOOTARGS		"root=/dev/null noinitrd init=/linuxrc console=ttyS0,115200n8n SELINUX_INIT=no mem=32M@0x30000000 ro video=qfb: ip=none rootflags=physaddr=0x00400000"
#endif
#define CONFIG_CRAMFSCMD	"setenv bootargs root=/dev/null noinitrd init=/linuxrc console=ttyS0,115200n8n SELINUX_INIT=no \$(cfg_ddr) ro video=qfb: ip=none rootflags=physaddr=0x00400000\;bootm 00080000"
#define CONFIG_NFSCMD		"setenv bootargs root=/dev/nfs noinitrd init=/linuxrc console=ttyS0,115200n8n SELINUX_INIT=no \$(cfg_ddr) ro video=qfb: nfsroot=\$(serverip):\$(rootpath),timeo=30 ip=\$(ipaddr):\$(serverip):\$(gatewayip):\$(netmask):\$(hostname):eth0:off\;bootm 00080000"
#ifdef CONFIG_MP200_SDBOOT_LINE	/* SD boot linesystem */
#define CONFIG_EXT3CMD		"setenv bootargs root=/dev/null noinitrd init=/linuxrc console=ttyS0,115200n8n SELINUX_INIT=no mem=32M@0x30000000 rw video=qfb: ip=none rootflags=physaddr=0x34000000\;bootm 30007fc0"
#else
#define CONFIG_EXT3CMD		"setenv bootargs root=\$(ext3_root) noinitrd init=/linuxrc console=ttyS0,115200n8n SELINUX_INIT=no \$(cfg_ddr) rw video=qfb: ip=none rootfstype=ext3 rootwait\;bootm 30007fc0"
#endif

#if defined(CONFIG_EMXX_MMCBOOT)
#define CONFIG_BOOTCOMMAND	"run ext3cmd"
#else
#define CONFIG_BOOTCOMMAND	"run cramfscmd"
#endif

#define CONFIG_EXTRA_ENV_SETTINGS	\
	"cramfscmd="	CONFIG_CRAMFSCMD	"\0"	\
	"ext3cmd="	CONFIG_EXT3CMD		"\0"	\
	"nfscmd="	CONFIG_NFSCMD		"\0"	\
	"cfg_ddr="	CONFIG_DDR		"\0"	\
	"ext3_root="	CONFIG_EXT3_ROOT	"\0"


#define CONFIG_IPADDR		192.168.1.100
#define CONFIG_SERVERIP		192.168.1.1
#define CONFIG_BOOTFILE		uImage

/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		/* undef to save memory     */
#define CONFIG_SYS_PROMPT		"EM1 # "	/* the primary prompt string */
#define CONFIG_SYS_CBSIZE		384			/* Console I/O Buffer Size  */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		24			/* max number of command args   */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size    */

#define CONFIG_SYS_MEMTEST_START	0x33000000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x33ffffff	/* 64 MB in DRAM    */

/* MEM TEST */
#define CONFIG_SYS_ALT_MEMTEST		1

#undef	CONFIG_SYS_CLKS_IN_HZ		/* everything, incl board info, in Hz */
#define CONFIG_SYS_LOAD_ADDR		0x30007fc0	/* default load address */


/*-----------------------------------------------------------------------
 * Timer
 */
#define CFG_HZ				32768
#define CONFIG_SYS_HZ			CFG_HZ
#define CONFIG_SYS_TIMERBASE		0xc0000000	/* use TI0 */

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE		(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ		(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ		(4*1024)	/* FIQ stack */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS		2		/* we have 2 bank of DRAM */
#ifdef CONFIG_EM1_DDR_1GBit	/* defined DDR 1Gbit(128MByte) */
#define PHYS_SDRAM_SIZE			0x04000000	/* 64 MB */
#else
#define PHYS_SDRAM_SIZE			0x02000000	/* 32 MB */
#endif
#define PHYS_SDRAM_1			0x30000000	/* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE		PHYS_SDRAM_SIZE
#define PHYS_SDRAM_2			(PHYS_SDRAM_1 + PHYS_SDRAM_1_SIZE)	/* SDRAM Bank #2 */
#define PHYS_SDRAM_2_SIZE		PHYS_SDRAM_SIZE

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_CFI_FLASH_USE_WEAK_ACCESSORS
#define CONFIG_SYS_FLASH_PROTECTION
#define CONFIG_FLASH_SHOW_PROGRESS	45

#define CONFIG_SYS_FLASH_BASE		0x00000000
#define CONFIG_SYS_MAX_FLASH_BANKS	1		/* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_SECT	(512)		/* max number of sectors on one chip */

#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE

#define FLASH_SECTOR_SIZE		0x00020000	/* 128 KB sectors */
#define CONFIG_ENV_SIZE			FLASH_SECTOR_SIZE


#if defined(CONFIG_EMXX_MMCBOOT)
#define CONFIG_ENV_IS_IN_MMC

/* mmc common configuration */
#define CONFIG_MMC_PARTS		4		/* number of memory partitions */
#define CONFIG_MMC_BLOCKS		0x800000	/* 4GB */
#define CONFIG_MMC_SECT_OF_BLOCKS	0x1000		/* 1[sector] == 4096[block] * 512[byte] */
#define CONFIG_MMC_SECTS		(CONFIG_MMC_BLOCKS / CONFIG_MMC_SECT_OF_BLOCKS)	/* 0x800[sector] */

#define CONFIG_MMC_PROTECT_TBL		0xa0000000		/* protect table address */
#define CONFIG_MMC_PROTECT_TBL_SIZE	CONFIG_MMC_SECTS	/* protect table size (0x800[byte]) */
#define CONFIG_ENV_TMP_ADDR		0x310C0000		/* 0x410C0000 - 0x410FFFFF */

#if defined(CONFIG_EMXX_EMMCBOOT)
/* addr of environment */
#define CONFIG_ENV_OFFSET		0x200			/* environment starts p1 + here  */
#elif defined(CONFIG_EMXX_SDBOOT)
/* addr of environment */
#define CONFIG_ENV_OFFSET		0x0			/* environment starts p2 + here  */
#endif

#else
#define CONFIG_ENV_IS_IN_FLASH	1	/* env in flash */
#define CONFIG_ENV_OFFSET		0x00040000
#define CONFIG_ENV_ADDR			(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
#endif


/*-----------------------------------------------------------------------
 * SRAM
 */
#define	SRAM_HWINFO_BOOT		(0xa001ff00)
#define	SRAM_HWINFO_KERNL		(0xa001fffc)

/*-----------------------------------------------------------------------*/

#endif	/* __CONFIG_H */
