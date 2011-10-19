/*
 * Copyright (C) 2010 Renesas Electronics Corporation
 *
 * See file CREDITS for list of people who contributed to this project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 */

#ifndef __MINI_CONFIG_H
#define __MINI_CONFIG_H

#ifndef NULL
#define NULL	0
#endif

#define inl(a)				(*(volatile unsigned long *)(a))
#define outl(v,a)			(*(volatile unsigned long *)(a) = (v))
#define inh(a)				(*(volatile unsigned short *)(a))
#define outh(v,a)			(*(volatile unsigned short *)(a) = (v))
#define inb(a)				(*(volatile unsigned char *)(a))
#define outb(v,a)			(*(volatile unsigned char *)(a) = (v))


/* SRAM(128K 0xf0000000-0xf001ffff)
 *	0xf0000000 - 0xf000ffff		mini-boot TEXT & DATA area
 *	
 *	0xf0017000 - 0xf0018fff		rom-boot WORK area
 *	0xf0019000 - ?			rom-boot DATA area
 *	?          - 0xf001feff		STACK area
 * 
 *	0xf001ffe4			RCA_VAL ADDR
 *	0xf001ffe8			ACC_MODE ADDR
 *	0xf001ffec			BOOT JUMP ADDR
 *	0xf001fff0			FATAL ERR VECTOR
 *	0xf001fff4			FIQ INT VECTOR
 *	0xf001fff8			IRQ INT VECTOR
 *	0xf001fffc			SWI INT VECTOR
 */
#define SRAM_START			0xf0000000
#define SRAM_END			0xf0020000

#define MINIBOOT_OFFSET			0x00000000
#define ROMBOOT_WORK_OFFSET		0x00017000
#define ROMBOOT_DATA_OFFSET		0x00019000

#define MINIBOOT_START			(SRAM_START + MINIBOOT_OFFSET)
#define MINIBOOT_SIZE			0x2000		/* 8KB */
#define ROMBOOT_WORK_START		(SRAM_START + ROMBOOT_WORK_OFFSET)
#define ROMBOOT_DATA_START		(SRAM_START + ROMBOOT_DATA_OFFSET)

#define STACK_START			(SRAM_END - 0x104)
#define SUB_IRQ_STACK			0x0
#define SUB_SVC_STACK			0x200

#define SRAM_SWI_VECTOR			(SRAM_END - 0x04)
#define SRAM_IRQ_VECTOR			(SRAM_END - 0x08)
#define SRAM_FIQ_VECTOR			(SRAM_END - 0x0c)
#define FATAL_ERR_VECTOR		(SRAM_END - 0x10)
#define BOOT_JUMP_ADDR			(SRAM_END - 0x14)
#define ACC_MODE_ADDR			(SRAM_END - 0x18)
#define RCA_VAL_ADDR			(SRAM_END - 0x1c)

#define SRAMTEST_ERR_ADDR		(SRAM_END - 0x40)
#define REGTEST_ERR_ADDR		(SRAM_END - 0x44)

#define MINIBOOT_NAND_SIZE		MINIBOOT_SIZE
#define MINIBOOT_EMMC_SIZE		MINIBOOT_SIZE
#define MINIBOOT_SD_SIZE		(64 * 1024)	/* max load size: 64KB */

#define DELAY_1US			(230/16 + 1)	/* System clock(230MHz/16)/1000000 [clk/usec] */
#define DELAY_LOOP_CYCLE		8		/* 1loop = 8cycle */

#endif /* __MINI_CONFIG_H */
