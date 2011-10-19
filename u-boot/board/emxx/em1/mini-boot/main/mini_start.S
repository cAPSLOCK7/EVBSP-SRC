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

#include "mini_config.h"

.text
	.align 4
	.type init,function
	.global	stext
stext:
	.set	ICACHE_LINE_SIZE_ORD, 5

#ifndef DISABLE_I_CACHE
	/* Enable I-Cache */
	mrc	p15, 0, r1, c1, c0, 0
	orr	r1, r1, #0x1000
	mcr	p15, 0, r1, c1, c0, 0
#endif

	/* Clear BSS */
	ldr	r1, data_end
	ldr	r2, bss_end
	mov	r3,	#0
1:
	cmp	r1, r2
	strcc	r3, [r1], #4
	bcc	1b

	/* Set Stack base */
	ldr	r1, stack_base

	msr	CPSR_c, #0xd2		/* set IRQ | F | I */
	sub	sp, r1, #SUB_IRQ_STACK	/* set IRQ mode stack base */

	msr	CPSR_c, #0xd3		/* set SVC | F | I */
	sub	sp, r1, #SUB_SVC_STACK	/* set SVC mode stack base */

	/* Set fatal_err_loop */
	ldr	r1, fatal_err_sram_addr
	ldr	r2, fatal_err_loop
	str	r2, [r1]

	bl lowlevel_init

	/* Jump u-boot & kernel copy function */
	bl	mini_boot
	/* never return */

loop:
	b	loop

stack_base:		.word	STACK_START	/* SRAM */
data_end:		.word	__bss_start
bss_end:		.word	_end
fatal_err_sram_addr:	.word	FATAL_ERR_VECTOR
fatal_err_loop:
	b	fatal_err_loop

