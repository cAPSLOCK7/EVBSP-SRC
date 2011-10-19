/*
 * (C) Copyright 2007, 2010
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
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

/* SRAM(128K 0xa0000000-0xa001ffff)
 *	0xa0000000 - 0xa0010000		mini-boot TEXT & DATA area
 * 
 *	0xa0019000 - ?			rom-boot DATA area
 *	?          - 0xa001feff		STACK area
 * 
 *	0xa001fff0			FATAL ERR VECTOR
 *	0xa001fff4			FIQ INT VECTOR
 *	0xa001fff8			IRQ INT VECTOR
 *	0xa001fffC			SWI INT VECTOR
 */
#define EM1_SRAM_START			0xa0000000
#define EM1_SRAM_END			0xa0020000

#define MINIBOOT_OFFSET			0x00000000
#define ROMBOOT_DATA_OFFSET		0x00019000

#define ROMBOOT_DATA_START		(EM1_SRAM_START + ROMBOOT_DATA_OFFSET)

#define STACK_START			(EM1_SRAM_END - 0x104)
#define SUB_IRQ_STACK			0x0
#define SUB_SVC_STACK			0x200

#define SRAM_SWI_VECTOR			(EM1_SRAM_END - 0x04)
#define SRAM_IRQ_VECTOR			(EM1_SRAM_END - 0x08)
#define SRAM_FIQ_VECTOR			(EM1_SRAM_END - 0x0c)
#define FATAL_ERR_VECTOR		(EM1_SRAM_END - 0x10)
#define SRAM_ACC_MODE			(EM1_SRAM_END - 0x14)
#define SRAM_RCA_VAL			(EM1_SRAM_END - 0x18)

#define CORRECT_DATA_SAVE_ADDR		(MINIBOOT_START + MINIBOOT_SIZE)

/* NAND
 *	0x00000000 - 0x000007ff	miniboot(master)
 *	0x00000800 - 0x00000fff	miniboot(mirror)
 */
#define MINIBOOT_NAND_START		0
#define MINIBOOT_NAND_SIZE		MINIBOOT_SIZE

#define MINIBOOT_NAND_MIRROR_START	(MINIBOOT_NAND_START + MINIBOOT_NAND_SIZE)
#define MINIBOOT_NAND_MIRROR_SIZE	MINIBOOT_NAND_SIZE

#define MINIBOOT_EMMC_SIZE		4096		/* 4k */

#endif /* __MINI_CONFIG_H */
