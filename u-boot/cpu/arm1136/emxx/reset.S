/*
 *  armboot - Startup Code for OMP2420/ARM1136 CPU-core
 *
 *  Copyright (c) 2007, 2008  NEC Electronics Corporation.
 *
 *  Copyright (c) 2004	Texas Instruments <r-woodruff2@ti.com>
 *
 *  Copyright (c) 2001	Marius Gröger <mag@sysgo.de>
 *  Copyright (c) 2002	Alex Züpke <azu@sysgo.de>
 *  Copyright (c) 2002	Gary Jennejohn <gj@denx.de>
 *  Copyright (c) 2003	Richard Woodruff <r-woodruff2@ti.com>
 *  Copyright (c) 2003	Kshitij <kshitij@ti.com>
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

#include <asm/arch/em1.h>

.globl reset_cpu
reset_cpu:
	ldr	r0, =SMU_BASE
	/* SPI0 CLK Disable */
	ldr	r1, =SMU_GLCKCTRL3ENA_VAL
	str	r1, [r0, #(SMU_GLCKCTRL3_ENA & 0xfff)]
	ldr	r1, =0x00000000
	str	r1, [r0, #(SMU_GLCKCTRL3     & 0xfff)]

	/* SPI0 SCLK Set (PLL3:24MHz) */
	ldr	r1, =SMU_DIVSP0SCLK_VAL
	str	r1, [r0, #(SMU_DIVSP0SCLK & 0xfff)]

	/* SPI0 CLK Enable */
	ldr	r1, =SMU_GLCKCTRL3ENA_VAL
	str	r1, [r0, #(SMU_GLCKCTRL3_ENA & 0xfff)]
	ldr	r1, =SMU_GLCKCTRL3_VAL
	str	r1, [r0, #(SMU_GLCKCTRL3     & 0xfff)]

	/* SPI0 RESET Disable */
	ldr	r1, =SMU_RESETREQ1ENA_VAL
	str	r1, [r0, #(SMU_RESETREQ1ENA  & 0xfff)]
	ldr	r1, =SMU_RESETREQ1_VAL
	str	r1, [r0, #(SMU_RESETREQ1     & 0xfff)]

        /* Wait 100us */
        mov     r2, #(0x61a8 & 0xff00)
        add     r2, r2, #(0x61a8 & 0x00ff) /* 500MHz (25000d) */
WAIT_100US:
        sub     r2, r2, #1
        cmp     r2, #0
        bne     WAIT_100US

	/* Send SoftReset */
	ldr	r0, =SP0_BASE
	ldr	r1, =SPI_MODE_SEND /* CS BIT */
	str	r1, [r0, #(SPX_MODE       & 0xfff)]
	ldr	r1, =SPI_POL_INIT  /* CSW DLY POL */	
	str	r1, [r0, #(SPX_POL        & 0xfff)]
	ldr	r1, =SPI_SEND_DATA /* RESET (ADDR:0x66 DATA:0x1) */
	str	r1, [r0, #(SPX_TX_DATA    & 0xfff)]	
	ldr	r1, =SPI_TX_START  /* Transfer Start */
	str	r1, [r0, #(SPX_CONTROL    & 0xfff)]

_loop_forever:
	b	_loop_forever
