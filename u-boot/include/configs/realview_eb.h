/*
 * (C) Copyright 2003
 * Texas Instruments.
 * Kshitij Gupta <kshitij@ti.com>
 * Configuation settings for the TI OMAP Innovator board.
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 * Configuration for Versatile PB.
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
/*
 *  Code, etc. common to all ARM supplied development boards
 */
#include <armsupplied.h>
/*
 * Board info register
 */
#define SYS_ID  (0x10000000)
#define ARM_SUPPLIED_REVISION_REGISTER SYS_ID

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_ARCH_REALVIEW_EB   1

#define CONFIG_SYS_MEMTEST_START	0x100000
#define CONFIG_SYS_MEMTEST_END		0x10000000
#define CFG_HZ	       			(1000)
#define CONFIG_SYS_HZ			CFG_HZ
#define CONFIG_SYS_HZ_CLOCK		1000000		/* Timers clocked at 1Mhz */
#define CONFIG_SYS_TIMERBASE		0x10011000	/* Timer 0 and 1 base	*/
#define CONFIG_SYS_TIMER_RELOAD	0xFFFFFFFF
#define TIMER_LOAD_VAL		CONFIG_SYS_TIMER_RELOAD

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_MISC_INIT_R		1	/* call misc_init_r during start up */
/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	(CONFIG_ENV_SIZE + 128*1024)
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/*
 * Hardware drivers
 */
#define REALVIEW_EB_SCTL_BASE	0x10001000	/* System controller */
/*
 * System controller (SP810) register offsets & bit assignment
 */
#define SP804_REFCLK	0
#define SP804_TIMCLK	1
#define SP804_TIMER0_EnSel	15
#define SP804_TIMER1_EnSel	17
#define SP804_TIMER2_EnSel	19
#define SP804_TIMER3_EnSel	21
#define SP810_OS_SCSYSSTAT	(0x00000004)	/* System status register */
#define REALVIEW_EB_SYS_FLASH_OFFSET	(0x4C)
#define REALVIEW_EB_FLASHCTRL	(REALVIEW_EB_SCTL_BASE + REALVIEW_EB_SYS_FLASH_OFFSET)
#define REALVIEW_EB_FLASHPROG_FLVPPEN	(1 << 0)   /* Enable writing to flash */

/* 
 * Only one ethernet driver is incorporated (U-Boot policy is to reduce size)
 * Because different revisions of RealView EB have different ethernet chips
 * we use a driver containing merged smc91111,c && smc9118 code
 * which autodetects the chip
 *
 * Rev A - C	: LAN91C111
 * Rev D	: LAN9118
 */
#define CONFIG_DRIVER_SMC_RV
#undef  CONFIG_DRIVER_SMC91111
#undef  CONFIG_DRIVER_SMC9118


#define CONFIG_SMC_RV_BASE	0x4E000000




#define CONFIG_SMC91111_BASE	CONFIG_SMC_RV_BASE

/*
 * NS16550 Configuration
 */
#define CONFIG_PL011_SERIAL
#define CONFIG_PL011_CLOCK	24000000
#define CONFIG_PL01x_PORTS	{ (void *)CONFIG_SYS_SERIAL0, (void *)CONFIG_SYS_SERIAL1 }
#define CONFIG_CONS_INDEX	0

#define CONFIG_BAUDRATE		38400
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }


#define CONFIG_SYS_SERIAL0		0x10009000
#define CONFIG_SYS_SERIAL1		0x1000A000





/*
 * Command line configuration.
 */
#define CONFIG_CMD_BDI
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ENV
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_IMI
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_SAVEENV


/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_MASK	CONFIG_BOOTP_DEFAULT

#define CONFIG_BOOTDELAY	2
/* #define CONFIG_BOOTARGS "root=/dev/nfs mem=128M ip=dhcp netdev=25,0,0xf1010000,0xf1010010,eth0" */
/*
The kernel command line & boot command below are for a VersatilePB board flashed using Boot_Monitor.axf:-
0x34000000  u-boot
0x34040000  u-linux
0x34380000  mtdroot

Name: mtdroot
Flash Address: 0x34380000
Load Address : 0x00000000
Entry Point  : 0x00000000
Size         : 5012K
Blocks Used  : 20

and a VersatileAB board flashed using Boot_monitor.axf:-

0x34000000  u-boot
0x34020000  EMPTY_IMAGE
0x34040000  u-linux
0x34380000  mtdroot

Name: mtdroot
Flash Address: 0x34380000
Load Address : 0x00000000
Entry Point  : 0x00000000
Size         : 5012K
Blocks Used  : 40

i.e despite the difference in flash block size the same command may be used.

*/

#define CONFIG_BOOTARGS "root=/dev/mtdblock0 mtdparts=armflash.0:7268k@0x02800000(cramfs) ip=dhcp mem=128M console=ttyAMA0 video=vc:1-2clcdfb:"
#define CONFIG_BOOTCOMMAND "cp 0x42000000 0x7fc0 0x00200000 ; bootm"

/*
 * Static configuration when assigning fixed address
 */
/*#define CONFIG_NETMASK	255.255.255.0	/--* talk on MY local net */
/*#define CONFIG_IPADDR		xx.xx.xx.xx	/--* static IP I currently own */
/*#define CONFIG_SERVERIP	xx.xx.xx.xx	/--* current IP of my dev pc */
#define CONFIG_BOOTFILE			"/tftpboot/uImage" /* file to load */


/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP	/* undef to save memory		 */
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/
/* Monitor Command Prompt   */
#define CONFIG_SYS_PROMPT	"RealView_EB # "
#define CONFIG_IDENT_STRING "\n\n***  Auto-detects ethernet chip ***\n\n"
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS	16		/* max number of command args	 */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size		*/

#undef	CONFIG_SYS_CLKS_IN_HZ		/* everything, incl board info, in Hz */
#define CONFIG_SYS_LOAD_ADDR	0x7fc0	/* default load address */

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS		1	/* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		       	0x00000000	/* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE		0x08000000	/* 128 MB */
#define PHYS_FLASH_SIZE         (0x04000000)	/* 64MB */


/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
/*
 *  Use the CFI flash driver for ease of use
 */
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_ENV_IS_IN_FLASH	1		/* env in flash */

#define CONFIG_SYS_FLASH_BASE		0x40000000
#define CONFIG_SYS_MAX_FLASH_BANKS	(1)		/* max number of memory banks */

/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(2*CFG_HZ)	/* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(2*CFG_HZ)	/* Timeout for Flash Write */

#define CONFIG_SYS_MAX_FLASH_SECT	(259)		/* The maximum number of sectors we may need to hold data about */
						/* 255 0x40000 sectors plus first or last sector may have 4 erase regions == 259 */
#define FLASH_MAX_SECTOR_SIZE	(0x00040000)	/* 256 KB sectors */
#define FLASH_MIN_SECTOR_SIZE	(0x00010000)	/*  64 KB sectors */

/* Room required on the stack for the environment data */
#define CONFIG_ENV_SIZE         8192
/* 
 * Since we don't know which end has the small erase blocks 
 * or indeed whether they are present
 * we use the penultimate full sector location
 * for the environment - we save a full sector even tho 
 * the real env size CONFIG_ENV_SIZE is probably less
 * Maintain the distinction since we want to make stack size as small as possible 
 */
/* Amount of flash used for environment */
#define CONFIG_ENV_SECT_SIZE	FLASH_MAX_SECTOR_SIZE 
/* 
 * Top of flash must NOT be defined using sectors
 * Board variants may have differing flash components
 */ 
#define FLASH_TOP	(CONFIG_SYS_FLASH_BASE + PHYS_FLASH_SIZE)
#define CONFIG_ENV_ADDR	(FLASH_TOP - CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_OFFSET	(CONFIG_ENV_ADDR - CONFIG_SYS_FLASH_BASE)

#define CONFIG_SYS_FLASH_PROTECTION	/* The devices have real protection */
#define CONFIG_SYS_FLASH_EMPTY_INFO	/* flinfo indicates empty blocks */

#endif							/* __CONFIG_H */
