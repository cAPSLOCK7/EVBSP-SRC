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
#define CONFIG_REALVIEW   1

#define CONFIG_SYS_MEMTEST_START	0x100000
#define CONFIG_SYS_MEMTEST_END		0x10000000
#define CFG_HZ	       			(1000)
#define CONFIG_SYS_HZ			CFG_HZ
#define CONFIG_SYS_HZ_CLOCK		1000000		/* Timers clocked at 1Mhz */
#define CONFIG_SYS_TIMERBASE		0x10011000	/* Timer 0 and 1 base	*/
#define CONFIG_SYS_TIMER_RELOAD		0xFFFFFFFF
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
#define REALVIEW_SCTL_BASE	0x10001000	/* SP810 System controller */
/*
 * System controller (SP804) register offsets & bit assignment
 */
#define SP804_REFCLK	0
#define SP804_TIMCLK	1
#define SP804_TIMER0_EnSel	15
#define SP804_TIMER1_EnSel	17
#define SP804_TIMER2_EnSel	19
#define SP804_TIMER3_EnSel	21
#define SP810_OS_SCSYSSTAT	(0x00000004)	/* System status register */
#define REALVIEW_SYS_FLASH_OFFSET	(0x4C)
#define REALVIEW_FLASHCTRL	(REALVIEW_SCTL_BASE + REALVIEW_SYS_FLASH_OFFSET)
#define REALVIEW_FLASHPROG_FLVPPEN	(1 << 0)   /* Enable writing to flash */

/* 
 * Only one ethernet driver is incorporated (U-Boot policy is to reduce size)
 * Because different revisions of RealView PB have different ethernet chips
 * we use a driver containing merged smc91111,c && smc9118 code
 * which autodetects the chip
 *
 * Rev A - C	: LAN91C111
 * Rev D	: LAN9118
 */
#define CONFIG_DRIVER_SMC_RV
#undef  CONFIG_DRIVER_SMC91111
#undef  CONFIG_DRIVER_SMC9118

#define CONFIG_SMC_RV_BASE	gd->bd->smcrv_base
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

/*
 * Command line configuration.
 */
#define CONFIG_CMD_BDI
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ELF
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
/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP	/* undef to save memory		 */
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/
/* Monitor Command Prompt   */
#define CONFIG_IDENT_STRING "\n\n***  Auto-detects ethernet chip ***\n\n"
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

/* 
 * Note that the following are example environment settings.
 * They will need to be changed  to settings for the local network, 
 * using the u-boot command line setenv functionality
 */
#define CONFIG_BOOTARGS "root=/dev/nfs ip=dhcp mem=128M console=ttyAMA0 video=vc:1-2clcdfb: nfsroot=10.1.77.36:/work/exports/exported_link"
#define CONFIG_BOOTCOMMAND "bootp ; bootm"
/*
 * Static configuration when assigning fixed address
 */
/*#define CONFIG_NETMASK	255.255.255.0	/--* talk on MY local net */
/*#define CONFIG_IPADDR		xx.xx.xx.xx	/--* static IP I currently own */
/*#define CONFIG_SERVERIP	xx.xx.xx.xx	/--* current IP of my dev pc */
#define	CONFIG_EXTRA_ENV_SETTINGS	\
	"ethaddr=00:02:F7:00:19:17\0"	\
	"ipaddr=10.1.77.77\0"	\
	"gatewayip=10.1.77.1\0"	\
	"serverip=10.1.77.36\0"	
#define CONFIG_BOOTFILE	"/work/tftpboot/realview" /* file to load */
/*
 * End of example environment settings
 */

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
/*
 *  Use the CFI flash driver for ease of use
 */
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER

#define CONFIG_SYS_MAX_FLASH_BANKS	(1)		/* max number of memory banks */


// Cant do this - gets used in other defines
// #define CONFIG_SYS_FLASH_BASE	(gd->bd->flash_base) 
// But do to find uses.....
#define CONFIG_SYS_FLASH_BASE	gd->       

/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(2*CFG_HZ)	/* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(2*CFG_HZ)	/* Timeout for Flash Write */

#define CONFIG_SYS_MAX_FLASH_SECT	(259)		/* The maximum number of sectors we may need to hold data about */
						/* 255 0x40000 sectors plus first or last sector may have 4 erase regions == 259 */
						/* Has room for pre-production boards which had 256 * 0x40000 sectors */
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
// #define CONFIG_SYS_MONITOR_LEN	       (4 * FLASH_SECTOR_SIZE)
// #define ARM_BM_START		(CONFIG_SYS_FLASH_BASE)
//
/* Store environment at top of flash - don't use actual sector size it may vary */
#define CONFIG_ENV_IS_IN_FLASH	1		/* env in flash */
#define CONFIG_ENV_OFFSET	(PHYS_FLASH_SIZE - (2 * CONFIG_ENV_SECT_SIZE))

#define CONFIG_SYS_FLASH_PROTECTION	/* The devices have real protection */
#define CONFIG_SYS_FLASH_EMPTY_INFO	/* flinfo indicates empty blocks */

/*
	These definitions must be made here, rather than by expanding the (non-existent) definitions
#  if (CONFIG_ENV_ADDR >= CONFIG_SYS_MONITOR_BASE) && \
      ((CONFIG_ENV_ADDR + CONFIG_ENV_SIZE) <= (CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN))
#   define ENV_IS_EMBEDDED	1
#  endif
*/

/*
IF (CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE)

- CONFIG_SYS_MONITOR_BASE:
		Physical start address of boot monitor code (set by
		make config files to be same as the text base address
		(TEXT_BASE) used when linking) - same as
		CONFIG_SYS_FLASH_BASE when booting from flash.

#define RUNNING_FROM_FLASH
 */  

// These must be lists of offsets from the flash base which is unknown at compile time
// - in the public U-Boot code they are lists of absolute addresses
# define CONFIG_SYS_FLASH_BANKS_LIST	{ 0 }
/*
 * However we're also uncertain of the flash sector size so we don't define this one
 *
# define CONFIG_SYS_FLASH_AUTOPROTECT_LIST
 */
#define CONFIG_SYS_SERIAL0		(serial_base + 0)
#define CONFIG_SYS_SERIAL1		(serial_base + 0x00001000) 
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+strlen(gd->bd->prompt)+16)

#endif

