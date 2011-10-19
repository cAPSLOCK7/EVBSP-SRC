/*
 * Board specific setup info
 *
 * Copyright (C) 2010, 2011 Renesas Electronics Corporation
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

#include <config.h>
#include <version.h>

#include <asm/arch/emev.h>
#include <asm/arch/boot.h>
#include "reg.h"
#include "lowlevel_init_val.h"
#include "spi.h"

#if !defined(EMXX_MINIBOOT)
_TEXT_BASE:
	.word	TEXT_BASE	/* sdram load addr from config.mk */
#endif

_delay:
	sub	r7, r7, #1
	cmp	r7, #0
	bne	_delay
	mov	pc, lr

#if defined(EMXX_PWC_CHANGE_CORE)
	/* r4: ctrl_bit(in), r5: wait_bit(in), r6: tx_data(in) */
_spi_rw:
	ldr	r1, [r3, #(SP0_CONTROL & 0xff)]
	ldr	r7, =SP0_CTL_CHK_EMP
	and	r1, r1, r7
	cmp	r1, r7
	bne	_stop_spi0clk

	/* set tx data */
	str	r6, [r3, #(SP0_TX_DATA & 0xfff)]
	/* tx/rx start */
	str	r4, [r3, #(SP0_CONTROL & 0xfff)]
	/* wait rx/tx end */
_wait_rw_end:
	ldr	r1, [r3, #(SP0_CONTROL & 0xff)]
	and	r1, r1, r5
	cmp	r1, #0
	bne	_wait_rw_end
	mov	pc, lr
#endif

.globl lowlevel_init
lowlevel_init:
#if defined(CONFIG_EMXX_NORBOOT)
	/* Set up the stack */
	ldr	r3, =SRAM_STACK_BASE
	sub	sp, r3, #12		/* leave 3 words for abort-stack */

	/* set r0 SMU_BASE */
	SET_BASEREG(r0, SMU_S0_BASE)

	/* set r2 AFS_BASE */
	SET_BASEREG(r2, AFS_BASE)

	/* get CPU num */
	ldr	r1, [r2, #AFS_DATA2 & 0xfff]
	and	r1, r1, #AFS_CPUENA_MASK
	ldr	r2, =AFS_CPUENA_MULTI
	cmp	r1, r2
	bne	_boot_init

	/* get CPU ID */
	mrc	p15, 0, r1, c0, c0, 5
	mov	r2, #0			/* CPU0 */
	and	r1, r1, #3		/* CPU ID */
	cmp	r1, r2
	beq	_boot_init
	str	r2, [r0, #SMU_GENERAL_REG0 & 0xfff]

	/* set interrupt */
	msr	CPSR_c, #0xd3		/* set SVC | F | I */

	/* set r2 INTA_CPU_BASE */
	SET_BASEREG(r2, INTA_CPU_BASE)
	STORE_WORD(r2, GIC_CPU_PRIMASK, INTA_CPU_PRIMASK_VAL, r1)
	STORE_WORD(r2, GIC_CPU_CTRL, INTA_CPU_CTRL_VAL, r1)

_loop_wfi:
	wfi
	/* Interrupt Clear */
	LOAD_WORD(r2, GIC_CPU_INTACK, r1)
	str	r1, [r2, #GIC_CPU_EOI & 0xfff]

	LOAD_WORD(r0, SMU_GENERAL_REG0, r1)
	cmp	r1, #0
	beq	_loop_wfi
	mov	pc, r1

	.pool

	.align 4

_boot_init:
	SET_BASEREG(r2, CHG_BASE)
	STORE_WORD(r2, CHG_P1_LAT, CHG_P1_LAT_VAL, r1)

	/* change active logic "H->L" of INT_COMMTX */
	SET_BASEREG(r2, INTD_BASE)
	STORE_WORD(r2, INTD_IT_PINV_SET2, INTD_IT_PINV_SET2_VAL, r1)

	/* All interrupt disable */
	SET_BASEREG(r2, INTA_DST_BASE)
	STORE_WORD(r2, GIC_DIST_CTRL, INTA_DIST_CTRL_VAL, r1)
#endif
	mov	r9, lr

	/* Set form POWER ON MODE to NORMAL MODE */
	SET_BASEREG(r0, SMU_S0_BASE)

	mvn	r1, #0
	/* Enable used clock lines */
	str	r1, [r0, #SMU_GIOGCLKCTRL    & 0xfff]
	str	r1, [r0, #SMU_INTAGCLKCTRL   & 0xfff]
	str	r1, [r0, #SMU_MEMCGCLKCTRL   & 0xfff]

	/* UnReset Macro */
	str	r1, [r0, #SMU_GIO_RSTCTRL    & 0xfff]
	str	r1, [r0, #SMU_CHG1_RSTCTRL   & 0xfff]
	str	r1, [r0, #SMU_PMU_RSTCTRL    & 0xfff]
	str	r1, [r0, #SMU_MEMC_RSTCTRL   & 0xfff]
	str	r1, [r0, #SMU_USIAS0_RSTCTRL & 0xfff]
	str	r1, [r0, #SMU_USIAU0_RSTCTRL & 0xfff]

	mov	r1, #0
	/* Reset Macro */
	str	r1, [r0, #SMU_TI0_RSTCTRL    & 0xfff]
	str	r1, [r0, #SMU_TW0_RSTCTRL    & 0xfff]
	str	r1, [r0, #SMU_USB0_RSTCTRL   & 0xfff]
	str	r1, [r0, #SMU_USB1_RSTCTRL   & 0xfff]

	/* Disable unused clock lines */
	ldr	r7, =SMU_LCDGCLKCTRL
	ldr	r2, =SMU_TW4GCLKCTRL
	str	r1, [r7, #0]
_stop_clk:
	add	r7, r7, #4
	str	r1, [r7, #0]
	cmp	r7, r2
	bne	_stop_clk

	str	r1, [r0, #SMU_DSPGCLKCTRL  & 0xfff]
	str	r1, [r0, #SMU_PDMAGCLKCTRL & 0xfff]

	/* Enable used clock lines */
#if defined(CONFIG_EMXX_EMMCBOOT) || defined(CONFIG_EMXX_ESDBOOT)
	STORE_WORD(r0, SMU_SDIO0SCLKDIV, SMU_SDIO0SCLKDIV_VAL, r1)
	STORE_WORD(r0, SMU_SDIO0GCLKCTRL, SMU_SDIO0GCLKCTRL_ENA, r1)
#elif defined(CONFIG_EMXX_SDBOOT)
	STORE_WORD(r0, SMU_SDCGCLKCTRL, SMU_SDCGCLKCTRL_ENA, r1)
#endif

	/*-----------------------*/
	/* Initialize PIN select */
	/*-----------------------*/
	/* Set CHG */
	SET_BASEREG(r3, CHG_BASE)

	/* pin select */
	STORE_WORD(r3, CHG_LCD_ENABLE, CHG_LCD_ENABLE_V33, r1)

	LOAD_WORD(r3, CHG_PINSEL_LCD3, r1)
	bic	r1, r1, #(CHG_PINSEL_LCD3_BIT & 0x0000ff00)
	bic	r1, r1, #(CHG_PINSEL_LCD3_BIT & 0x000000ff)
	str	r1, [r3, #CHG_PINSEL_LCD3 & 0xfff]

	LOAD_WORD(r3, CHG_PINSEL_AB, r1)
	bic	r1, r1, #(CHG_PINSEL_AB_BIT & 0x0000ff00)
	bic	r1, r1, #(CHG_PINSEL_AB_BIT & 0x000000ff)
	str	r1, [r3, #CHG_PINSEL_AB & 0xfff]

	STORE_WORD(r3, CHG_PINSEL_UART, CHG_PINSEL_UART_VAL, r1)
	STORE_WORD(r3, CHG_PINSEL_IIC,  CHG_PINSEL_IIC_VAL, r1)
	STORE_WORD(r3, CHG_PINSEL_SD,   CHG_PINSEL_SD_VAL, r1)
	STORE_WORD(r3, CHG_PINSEL_USI,  CHG_PINSEL_USI_VAL, r1)
	STORE_WORD(r3, CHG_PINSEL_NTSC, CHG_PINSEL_NTSC_VAL, r1)
	STORE_WORD(r3, CHG_PINSEL_CAM,  CHG_PINSEL_CAM_VAL, r1)
	STORE_WORD(r3, CHG_PINSEL_HSI,  CHG_PINSEL_HSI_VAL, r1)

	STORE_WORD(r3, CHG_PINSEL_G000, CHG_PINSEL_G000_VAL, r1)
	STORE_WORD(r3, CHG_PINSEL_G032, CHG_PINSEL_G032_VAL, r1)
	STORE_WORD(r3, CHG_PINSEL_G064, CHG_PINSEL_G064_VAL, r1)
	STORE_WORD(r3, CHG_PINSEL_G096, CHG_PINSEL_G096_VAL, r1)
	STORE_WORD(r3, CHG_PINSEL_G128, CHG_PINSEL_G128_VAL, r1)

	/* set bus-hold */
	STORE_WORD(r3, CHG_BUSHOLD, CHG_BUSHOLD_VAL, r1)

	/* wait 1msec */
	ldr	r7, =0x753		/* 15MHz (1875d(=15000/8[loop])) */
	bl	_delay

	/* Set PullUp/PullDown */
	LOAD_WORD(r3, CHG_PULL0, r1)
	and	r1, r1, #(~CHG_PULL0_BIT)
	ldr	r7, =CHG_PULL0_VAL
	orr	r7, r7, r1
	str	r7, [r3, #CHG_PULL0 & 0xfff]

	STORE_WORD(r3, CHG_PULL1,  CHG_PULL1_VAL, r1)
	STORE_WORD(r3, CHG_PULL2,  CHG_PULL2_VAL, r1)
	STORE_WORD(r3, CHG_PULL3,  CHG_PULL3_VAL, r1)
	STORE_WORD(r3, CHG_PULL4,  CHG_PULL4_VAL, r1)
	STORE_WORD(r3, CHG_PULL5,  CHG_PULL5_VAL, r1)
	STORE_WORD(r3, CHG_PULL6,  CHG_PULL6_VAL, r1)
	STORE_WORD(r3, CHG_PULL7,  CHG_PULL7_VAL, r1)
	STORE_WORD(r3, CHG_PULL8,  CHG_PULL8_VAL, r1)
	STORE_WORD(r3, CHG_PULL9,  CHG_PULL9_VAL, r1)
	STORE_WORD(r3, CHG_PULL10, CHG_PULL10_VAL, r1)
	STORE_WORD(r3, CHG_PULL11, CHG_PULL11_VAL, r1)
	STORE_WORD(r3, CHG_PULL12, CHG_PULL12_VAL, r1)
	STORE_WORD(r3, CHG_PULL13, CHG_PULL13_VAL, r1)
	STORE_WORD(r3, CHG_PULL14, CHG_PULL14_VAL, r1)
	STORE_WORD(r3, CHG_PULL15, CHG_PULL15_VAL, r1)
	STORE_WORD(r3, CHG_PULL16, CHG_PULL16_VAL, r1)
	STORE_WORD(r3, CHG_PULL17, CHG_PULL17_VAL, r1)
	STORE_WORD(r3, CHG_PULL18, CHG_PULL18_VAL, r1)
	STORE_WORD(r3, CHG_PULL19, CHG_PULL19_VAL, r1)
	STORE_WORD(r3, CHG_PULL21, CHG_PULL21_VAL, r1)
	STORE_WORD(r3, CHG_PULL25, CHG_PULL25_VAL, r1)

	LOAD_WORD(r3, CHG_PULL20, r1)
	and	r1, r1, #(~CHG_PULL20_BIT & 0x00ff0000)
	ldr	r7, =CHG_PULL20_VAL
	orr	r7, r7, r1
	str	r7, [r3, #CHG_PULL20 & 0xfff]

	LOAD_WORD(r3, CHG_PULL22, r1)
	bic	r1, r1, #(CHG_PULL22_BIT & 0xff000000)
	orr	r1, r1, #CHG_PULL22_VAL
	str	r1, [r3, #CHG_PULL22 & 0xfff]

	LOAD_WORD(r3, CHG_DRIVE0, r1)
	bic	r1, r1, #(CHG_DRIVE0_BIT & 0x0000ff00)
	orr	r1, r1, #(CHG_DRIVE0_VAL & 0x0000ff00)
	str	r1, [r3, #CHG_DRIVE0 & 0xfff]

	LOAD_WORD(r3, CHG_DRIVE1, r1)
	bic	r1, r1, #(CHG_DRIVE1_BIT & 0xff000000)
	bic	r1, r1, #(CHG_DRIVE1_BIT & 0x00ff0000)
	bic	r1, r1, #(CHG_DRIVE1_BIT & 0x0000ff00)
	orr	r1, r1, #(CHG_DRIVE1_VAL & 0xff000000)
	orr	r1, r1, #(CHG_DRIVE1_VAL & 0x00ff0000)
	orr	r1, r1, #(CHG_DRIVE1_VAL & 0x0000ff00)
	str	r1, [r3, #CHG_DRIVE1 & 0xfff]

#ifndef CONFIG_EMXX_UNUSE_AB
	/* Initialize AB0 */
	/* Set AB0 */
	SET_BASEREG(r2, AB0_BASE)

	/* AB_CS0 NOR Setting */
	STORE_WORD(r2, AB0_CS0BASEADD,  AB0_CS0BASEADD_VAL, r1)
	STORE_WORD(r2, AB0_CS0BITCOMP,  AB0_CS0BITCOMP_VAL, r1)
	STORE_WORD(r2, AB0_CS0WAITCTRL, AB0_CS0WAITCTRL_VAL, r1)

#if !defined(CONFIG_EMXX_PALLADIUM)
	STORE_WORD(r2, AB0_CS0WAITCTRL_W, AB0_CS0WAITCTRL_W_VAL, r1)
	STORE_WORD(r2, AB0_CS0READCTRL, AB0_CS0READCTRL_VAL, r1)

	/* AB_CS1 Ether Setting */
	STORE_WORD(r2, AB0_CS1BASEADD,  AB0_CS1BASEADD_VAL, r1)
	STORE_WORD(r2, AB0_CS1BITCOMP,  AB0_CS1BITCOMP_VAL, r1)

	STORE_WORD(r2, AB0_CS1WAITCTRL, AB0_CS1WAITCTRL_VAL, r1)
	STORE_WORD(r2, AB0_CS1WAITCTRL_W, AB0_CS1WAITCTRL_W_VAL, r1)
	STORE_WORD(r2, AB0_CS1READCTRL, AB0_CS1READCTRL_VAL, r1)

	/* AB_CS2 NAND Setting */
	STORE_WORD(r2, AB0_CS2BASEADD,  AB0_CS2BASEADD_VAL, r1)
	STORE_WORD(r2, AB0_CS2BITCOMP,  AB0_CS2BITCOMP_VAL, r1)

	STORE_WORD(r2, AB0_CS2WAITCTRL, AB0_CS2WAITCTRL_VAL, r1)
	STORE_WORD(r2, AB0_CS2WAITCTRL_W, AB0_CS2WAITCTRL_W_VAL, r1)
	STORE_WORD(r2, AB0_CS2READCTRL, AB0_CS2READCTRL_VAL, r1)
#endif

	STORE_WORD(r2, AB0_FLASHCOMSET, AB0_FLASHCOMSET_VAL, r1)
#endif

	/* check chip revision */
	SET_BASEREG(r3, SMU_S0_BASE_2000)
	LOAD_WORD(r3, SMU_CHIP_REVISION, r2)
	and	r2, r2, #0x000000ff

#if defined(EMXX_PWC_CHANGE_CORE)
	/*---------*/
	/* set pwc */
	/*---------*/
_spi_init:
	/* set SPI0_CLK Divide */
	LOAD_WORD(r0, SMU_USIASCLKDIV, r4)
	bic	r4, r4, #(SP0_CLK_MASK & 0x0000ff00)
	bic	r4, r4, #(SP0_CLK_MASK & 0x000000ff)
	orr	r4, r4, #(SP0_CLK_12MHZ & 0x000000ff)
	str	r4, [r0, #SMU_USIASCLKDIV & 0xfff]

	/* SPI0_CLK supply */
	STORE_WORD(r0, SMU_USIAS0GCLKCTRL, SMU_USIAS0GCLKCTRL_ENA, r1)

	SET_BASEREG(r3, SPI0_BASE)
	STORE_WORD(r3, SP0_CONTROL, SP0_CTL_RST, r1)

	/* Wait 100us */
	ldr	r7, =0xbb	/* 15MHz (188d(=1500/8[loop])) */
	bl	_delay

	STORE_WORD(r3, SP0_CONTROL, SP0_CTL_RST_FREE, r1)
	STORE_WORD(r3, SP0_MODE, SP0_MODE_CS0, r1)
	STORE_WORD(r3, SP0_POL, SP0_POL_CS0, r1)

	ldr	r1, [r3, #(SP0_CONTROL & 0xff)]
	ldr	r7, =SP0_CTL_CHK_EMP
	and	r1, r1, r7
	cmp	r1, r7
	bne	_stop_spi0clk

	/* write pwc R54 = 0x66 */
	mov	r4, #(SP0_CTL_WRT_ENA | SP0_CTL_START)
	mov	r5, #SP0_CTL_START
	ldr	r6, =PWC_R54_SET_W
	bl	_spi_rw

	/* tx/rx stop */
	STORE_WORD(r3, SP0_CONTROL, 0, r1)

	/* Wait 1200us */
	ldr	r7, =0x4700 	/* 15MHz (18000d(=15000/8[loop])) */
	bl	_delay

	ldr	r1, [r3, #(SP0_CONTROL & 0xff)]
	ldr	r7, =SP0_CTL_CHK_EMP
	and	r1, r1, r7
	cmp	r1, r7
	bne	_stop_spi0clk

	/* write pwc R64 = 0xF8 */
	ldr	r6, =PWC_R64_SET_W
	bl	_spi_rw

	/* tx/rx stop */
	STORE_WORD(r3, SP0_CONTROL, 0, r1)

	ldr	r1, [r3, #(SP0_CONTROL & 0xff)]
	ldr	r7, =SP0_CTL_CHK_EMP
	and	r1, r1, r7
	cmp	r1, r7
	bne	_stop_spi0clk

	/* write pwc R62 = 0xDF */
	ldr	r6, =PWC_R62_SET_W
	bl	_spi_rw

	/* tx/rx stop */
	STORE_WORD(r3, SP0_CONTROL, 0, r1)

	ldr	r1, [r3, #(SP0_CONTROL & 0xff)]
	ldr	r7, =SP0_CTL_CHK_EMP
	and	r1, r1, r7
	cmp	r1, r7
	bne	_stop_spi0clk

	/* write pwc R44 = 0xAA */
	ldr	r6, =PWC_R44_SET_W
	bl	_spi_rw

	/* tx/rx stop */
	STORE_WORD(r3, SP0_CONTROL, 0, r1)

	ldr	r1, [r3, #(SP0_CONTROL & 0xff)]
	ldr	r7, =SP0_CTL_CHK_EMP
	and	r1, r1, r7
	cmp	r1, r7
	bne	_stop_spi0clk

	/* Wait 20us */
	ldr	r7, =0x26	/* 15MHz (38d(=300/8[loop])) */
	bl	_delay

	/* write pwc R46 = 0xxx */
	ldr	r6, =SMU_CHIP_REVISION_ES1
	cmp	r2, r6
	beq	pwc_r46_set_es1
	ldr	r6, =SMU_CHIP_REVISION_ES2
	cmp	r2, r6
	beq	pwc_r46_set_es2
	ldr	r6, =PWC_R46_SET_ES3
	b	pwc_r46_set_end
pwc_r46_set_es2:
	ldr	r6, =PWC_R46_SET_ES2
	b	pwc_r46_set_end
pwc_r46_set_es1:
	ldr	r6, =PWC_R46_SET_ES1
pwc_r46_set_end:
	bl	_spi_rw

	/* tx/rx stop */
	STORE_WORD(r3, SP0_CONTROL, 0, r1)

	ldr	r1, [r3, #(SP0_CONTROL & 0xff)]
	ldr	r7, =SP0_CTL_CHK_EMP
	and	r1, r1, r7
	cmp	r1, r7
	bne	_stop_spi0clk

	/* write pwc R60 = 0x01 */
	ldr	r6, =PWC_R60_SET_W
	bl	_spi_rw

	/* tx/rx stop */
	STORE_WORD(r3, SP0_CONTROL, 0, r1)

_stop_spi0clk:
	/* SPI0_CLK stop */
	STORE_WORD(r0, SMU_USIAS0GCLKCTRL, SMU_USIAS0GCLKCTRL_DIS, r1)
#endif

	/*---------------------*/
	/* change system clock */
	/*---------------------*/

	/* set OSC/PLLx lockup time */
	STORE_WORD(r0, SMU_OSCLOCKTIME, SMU_OSCLOCKTIME_VAL, r1)
	STORE_WORD(r0, SMU_PLLLOCKTIME0, SMU_PLLLOCKTIME0_VAL, r1)
	STORE_WORD(r0, SMU_PLLLOCKTIME1, SMU_PLLLOCKTIME1_VAL, r1)

#if defined(CONFIG_EMXX_PALLADIUM)
	STORE_WORD(r0, SMU_PLL1CTRL1, SMU_PLL1CTRL1_VAL, r1)
	STORE_WORD(r0, SMU_PLL2CTRL1, SMU_PLL2CTRL1_VAL, r1)
	STORE_WORD(r0, SMU_PLL4CTRL1, SMU_PLL4CTRL1_VAL, r1)
	STORE_WORD(r0, SMU_CKMODE_PLLSEL, SMU_CKMODE_PLLSEL_VAL, r1)

	STORE_WORD(r0, SMU_NORMALA_DIV, SMU_NORMALA_DIV_VAL, r1)
#else
	STORE_WORD(r0, SMU_OSC_CX, SMU_OSC_CX_VAL, r1)
	STORE_WORD(r0, SMU_OSC1CTRL1, SMU_OSC1CTRL1_VAL, r1)

	/* wait 1msec */
	ldr	r7, =0x753		/* 15MHz (1875d(=15000/8[loop])) */
	bl	_delay

#if defined(CONFIG_EMEV_CPU_500MHZ)
	STORE_WORD(r0, SMU_PLL1CTRL0, SMU_PLL1CTRL0_500M, r1)
#else
	STORE_WORD(r0, SMU_PLL1CTRL0, SMU_PLL1CTRL0_VAL, r1)
#endif
	STORE_WORD(r0, SMU_PLL1CTRL1, SMU_PLL1CTRL1_VAL, r1)
	STORE_WORD(r0, SMU_ROSCCTRL1, SMU_ROSCCTRL1_VAL, r1)

	/* wait 1msec */
	ldr	r7, =0x753		/* 15MHz (1875d(=15000/8[loop])) */
	bl	_delay

	STORE_WORD(r0, SMU_CKMODE_PLLSEL, SMU_CKMODE_PLLSEL_VAL, r1)

	ldr	r6, =SMU_CHIP_REVISION_ES1

	cmp	r2, r6
	beq	normala_div_es1
	STORE_WORD(r0, SMU_NORMALA_DIV, SMU_NORMALA_DIV_ES2, r1)
	b	normala_div_end
normala_div_es1:
	STORE_WORD(r0, SMU_NORMALA_DIV, SMU_NORMALA_DIV_ES1, r1)
normala_div_end:
	STORE_WORD(r0, SMU_NORMALB_DIV, SMU_NORMALB_DIV_VAL, r1)
	STORE_WORD(r0, SMU_NORMALC_DIV, SMU_NORMALC_DIV_VAL, r1)
	STORE_WORD(r0, SMU_NORMALD_DIV, SMU_NORMALD_DIV_VAL, r1)
	STORE_WORD(r0, SMU_ECONOMY_DIV, SMU_ECONOMY_DIV_VAL, r1)
	STORE_WORD(r0, SMU_SLEEP_DIV,   SMU_SLEEP_DIV_VAL,   r1)

	STORE_WORD(r0, SMU_CPUCLK_ASYNC_MODE, SMU_CPUCLK_ASYNC_MODE_VAL, r1)
	STORE_WORD(r0, SMU_DSPCLK_ASYNC_MODE, SMU_DSPCLK_ASYNC_MODE_VAL, r1)
	STORE_WORD(r0, SMU_CPUCLK_SYNCSET,    SMU_CPUCLK_SYNCSET_VAL, r1)
wait_syncmode:
	LOAD_WORD(r0, SMU_CPUCLK_SYNCSET, r7)
	and	r7, r7, #SMU_CPUCLK_SYNCMODEACK
	cmp	r7, #0
	bne	wait_syncmode
#endif
	STORE_WORD(r0, SMU_CLK_MODE_SEL, SMU_CLK_MODE_SEL_VAL, r1)

	/* Wait CLK_MODE_SEL until bit[11:8]==0x1 */
WAIT_MODE_STATUS:
	LOAD_WORD(r0, SMU_CLK_MODE_SEL, r7)
	and	r7, r7, #0x00000f00
	cmp	r7, #0x00000100
	bne	WAIT_MODE_STATUS


#if defined(EMXX_READ_VERSION)
	/*-------------------*/
	/* read Board Config */
	/*-------------------*/
	SET_BASEREG(r3, SMSC9118_BASE)
	mov	r1, #HW_CFG_SRST
	str	r1, [r3, #(HW_CFG & 0xff)]
wait_srst_free:
	ldr	r1, [r3, #(HW_CFG & 0xff)]
	and	r1, r1, #HW_CFG_SRST
	cmp	r1, #0
	bne	wait_srst_free

	mov	r5, #E2P_READ_CFGIO
	mov	r6, #0
wait_e2p_busy:
	ldr	r1, [r3, #(E2P_CMD & 0xff)]
	and	r7, r1, #E2P_CMD_EPC_BUSY
	cmp	r7, #0x00000000
	bne	wait_e2p_busy
	and	r7, r1, #E2P_CMD_EPC_TIMEOUT
	cmp	r7, #0x00000000
	beq	write_e2p_addr
	str	r1, [r3, #(E2P_CMD & 0xff)]
	b	wait_e2p_busy

write_e2p_addr:
	and	r1, r1, #E2P_CMD_EPC_ADDR
	and	r7, r5, #E2P_CMD_EPC_ADDR
	cmp	r1, r7
	beq	read_e2p_data
	str	r5, [r3, #(E2P_CMD & 0xff)]
	b	wait_e2p_busy

read_e2p_data:
	ldr	r1, [r3, #(E2P_DATA & 0xff)]
	orr	r6, r6, r1
	lsl	r6, r6, #8
	cmp	r5, #E2P_READ_CFGCPU
	beq	read_e2p_end
	sub	r5, r5, #1
	b	wait_e2p_busy

read_e2p_end:
#else
	mov	r6, #0x00000400
#endif
	/*----------------*/
	/* Initialize DDR */
	/*----------------*/
#if defined(CONFIG_EMXX_PALLADIUM)
	/* Initialize DDR (palladium setting) */
	SET_BASEREG(r3, MEMC_BASE)
	STORE_WORD(r3, MEMC_DEGFUN,        MEMC_DEGFUN_VAL, r1)

	SET_BASEREG(r2, MEMC_REQSCH)
	STORE_WORD(r2, MEMC_REQSCH,        MEMC_REQSCH_VAL, r1)

	SET_BASEREG(r2, MEMC_DDR_BASE)
	STORE_WORD(r2, MEMC_DDR_CONFIGF,   MEMC_DDR_CONFIGF_VAL, r1)
	STORE_WORD(r2, MEMC_DDR_CONFIGA1,  MEMC_DDR_CONFIGA1_VAL, r1)
	STORE_WORD(r2, MEMC_DDR_CONFIGA2,  MEMC_DDR_CONFIGA2_VAL, r1)
	STORE_WORD(r2, MEMC_DDR_CONFIGC1,  MEMC_DDR_CONFIGC1_VAL, r1)
	STORE_WORD(r2, MEMC_DDR_CONFIGC2,  MEMC_DDR_CONFIGC2_VAL1, r1)
	STORE_WORD(r2, MEMC_DDR_CONFIGC2,  MEMC_DDR_CONFIGC2_VAL2, r1)
	STORE_WORD(r2, MEMC_DDR_CONFIGC2,  MEMC_DDR_CONFIGC2_VAL3, r1)
	STORE_WORD(r2, MEMC_DDR_CONFIGR1,  MEMC_DDR_CONFIGR1_VAL, r1)
	STORE_WORD(r2, MEMC_DDR_CONFIGR2,  MEMC_DDR_CONFIGR2_VAL, r1)
	STORE_WORD(r2, MEMC_DDR_CONFIGR3,  MEMC_DDR_CONFIGR3_VAL, r1)
	STORE_WORD(r2, MEMC_DDR_CONFIGC2,  MEMC_DDR_CONFIGC2_VAL4, r1)

	STORE_WORD(r0, SMU_MEMCRCLKDIV,    SMU_MEMCRCLKDIV_VAL1, r1)
	STORE_WORD(r0, SMU_MEMCCLK270_SEL, SMU_MEMCCLK270_SEL_VAL1, r1)

WAIT_AUTO_CALIBRATE:
	LOAD_WORD(r2, MEMC_DDR_CONFIGT1, r7)
	and	r7, r7, #0x00000002
	cmp	r7, #0x00000002
	bne	WAIT_AUTO_CALIBRATE

	LOAD_WORD(r2, MEMC_DDR_CONFIGT3, r4)
	str r4, [r2, #MEMC_DDR_CONFIGT2 & 0xfff]
	STORE_WORD(r0, SMU_MEMCRCLKDIV,    SMU_MEMCRCLKDIV_VAL2, r1)
	STORE_WORD(r0, SMU_MEMCCLK270_SEL, SMU_MEMCCLK270_SEL_VAL2, r1)

	/* use DDR cache */
	STORE_WORD(r3, MEMC_CACHE_MODE, MEMC_CACHE_MODE_VAL, r1)
#else
	/* check chip revision */
	orr	r6, r6, r2

	SET_BASEREG(r3, SRAM_HWINFO_BOOT)
	str	r6, [r3, #0]
	ldr	r6, =SMU_CHIP_REVISION_ES1
	ldr	r5, =SMU_CHIP_REVISION_ES3

	SET_BASEREG(r3, MEMC_DDR_BASE)
	STORE_WORD(r3, MEMC_DDR_CONFIGD,   MEMC_DDR_CONFIGD_VAL,    r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGZD,  MEMC_DDR_CONFIGZD_VAL,   r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGZC,  MEMC_DDR_CONFIGZC_VAL,   r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGZA,  MEMC_DDR_CONFIGZA_VAL,   r1)

	SET_BASEREG(r3, MEMC_REQSCH)
	STORE_WORD(r3, MEMC_REQSCH,        MEMC_REQSCH_VAL, r1)

	SET_BASEREG(r3, MEMC_DDR_BASE)
	STORE_WORD(r3, MEMC_DDR_CONFIGF,   MEMC_DDR_CONFIGF_VAL,    r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGA1,  MEMC_DDR_CONFIGA1_VAL,   r1)

	cmp	r2, r6
	beq	memc_configa2_es1
	cmp	r2, r5
	beq	memc_configa2_es3
	STORE_WORD(r3, MEMC_DDR_CONFIGA2,  MEMC_DDR_CONFIGA2_ES2,   r1)
	b	memc_configa2_end
memc_configa2_es3:
	STORE_WORD(r3, MEMC_DDR_CONFIGA2,  MEMC_DDR_CONFIGA2_ES3,   r1)
	b	memc_configa2_end
memc_configa2_es1:
	STORE_WORD(r3, MEMC_DDR_CONFIGA2,  MEMC_DDR_CONFIGA2_ES1,   r1)
memc_configa2_end:

	STORE_WORD(r3, MEMC_DDR_CONFIGR3,  MEMC_DDR_CONFIGR3_VAL1,  r1)

	STORE_WORD(r3, MEMC_DDR_CONFIGC1,  MEMC_DDR_CONFIGC1_VAL1,  r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGC2,  MEMC_DDR_CONFIGC2_VAL1,  r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGC2,  MEMC_DDR_CONFIGC2_VAL2,  r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGC1,  MEMC_DDR_CONFIGC1_VAL2,  r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGC2,  MEMC_DDR_CONFIGC2_VAL3,  r1)

	/* wait 400usec */
	ldr	r7, =0x681A		/* 533MHz (26650d(=533000/8[loop])) */
	bl	_delay

	STORE_WORD(r3, MEMC_DDR_CONFIGC1,  MEMC_DDR_CONFIGC1_VAL3,  r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGC2,  MEMC_DDR_CONFIGC2_VAL4,  r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGC1,  MEMC_DDR_CONFIGC1_VAL4,  r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGC2,  MEMC_DDR_CONFIGC2_VAL5,  r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGC1,  MEMC_DDR_CONFIGC1_VAL5,  r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGC2,  MEMC_DDR_CONFIGC2_VAL6,  r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGC1,  MEMC_DDR_CONFIGC1_VAL6,  r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGC2,  MEMC_DDR_CONFIGC2_VAL7,  r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGC1,  MEMC_DDR_CONFIGC1_VAL7,  r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGC2,  MEMC_DDR_CONFIGC2_VAL8,  r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGC1,  MEMC_DDR_CONFIGC1_VAL8,  r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGC2,  MEMC_DDR_CONFIGC2_VAL9,  r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGC2,  MEMC_DDR_CONFIGC2_VAL10, r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGC2,  MEMC_DDR_CONFIGC2_VAL11, r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGC1,  MEMC_DDR_CONFIGC1_VAL9,  r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGC2,  MEMC_DDR_CONFIGC2_VAL12, r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGC1,  MEMC_DDR_CONFIGC1_VAL10, r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGC2,  MEMC_DDR_CONFIGC2_VAL13, r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGC1,  MEMC_DDR_CONFIGC1_VAL11, r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGC2,  MEMC_DDR_CONFIGC2_VAL14, r1)

	STORE_WORD(r3, MEMC_DDR_CONFIGR1,  MEMC_DDR_CONFIGR1_VAL,   r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGR2,  MEMC_DDR_CONFIGR2_VAL,   r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGR3,  MEMC_DDR_CONFIGR3_VAL2,  r1)

	cmp	r2, r6
	beq	memc_configt_es1
	cmp	r2, r5
	beq	memc_configt_es3
	STORE_WORD(r3, MEMC_DDR_CONFIGT1,  MEMC_DDR_CONFIGT1_ES2,   r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGT2,  MEMC_DDR_CONFIGT2_ES2,   r1)
	b	memc_configt_end
memc_configt_es3:
	STORE_WORD(r3, MEMC_DDR_CONFIGT1,  MEMC_DDR_CONFIGT1_ES3,   r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGT2,  MEMC_DDR_CONFIGT2_ES3,   r1)
	b	memc_configt_end
memc_configt_es1:
	STORE_WORD(r3, MEMC_DDR_CONFIGT1,  MEMC_DDR_CONFIGT1_ES1,   r1)
	STORE_WORD(r3, MEMC_DDR_CONFIGT2,  MEMC_DDR_CONFIGT2_ES1,   r1)
memc_configt_end:
	STORE_WORD(r3, MEMC_DDR_CONFIGC2,  MEMC_DDR_CONFIGC2_VAL15, r1)

	SET_BASEREG(r3, MEMC_BASE)

	/* use DDR cache */
	STORE_WORD(r3, MEMC_CACHE_MODE, MEMC_CACHE_MODE_VAL, r1)

	STORE_WORD(r0, SMU_MEMCRCLKDIV,    SMU_MEMCRCLKDIV_VAL, r1)
	STORE_WORD(r0, SMU_MEMCCLK270_SEL, SMU_MEMCCLK270_SEL_VAL, r1)

	/* auto caribration */


	/* Enable Automatic Clock Control */
	STORE_WORD(r0, SMU_ACNT1,       SMU_ACNT1_VAL, r1)
	STORE_WORD(r0, SMU_AHBCLKCTRL1, SMU_AHBCLKCTRL1_VAL, r1)
	STORE_WORD(r0, SMU_AHBCLKCTRL2, SMU_AHBCLKCTRL2_VAL, r1)
	STORE_WORD(r0, SMU_AHBCLKCTRL3, SMU_AHBCLKCTRL3_VAL, r1)
	STORE_WORD(r0, SMU_APBCLKCTRL0, SMU_APBCLKCTRL0_VAL, r1)
	STORE_WORD(r0, SMU_APBCLKCTRL1, SMU_APBCLKCTRL1_VAL, r1)
	STORE_WORD(r0, SMU_APBCLKCTRL2, SMU_APBCLKCTRL2_VAL, r1)
	STORE_WORD(r0, SMU_CLKCTRL,     SMU_CLKCTRL_VAL, r1)

	cmp	r2, r6
	beq	auto_clkctrl_es1
	STORE_WORD(r0, SMU_ACNT0,       SMU_ACNT0_ES2, r1)
	STORE_WORD(r0, SMU_AHBCLKCTRL0, SMU_AHBCLKCTRL0_ES2, r1)
	b	auto_clkctrl_end
auto_clkctrl_es1:
	STORE_WORD(r0, SMU_ACNT0,       SMU_ACNT0_ES1, r1)
	STORE_WORD(r0, SMU_AHBCLKCTRL0, SMU_AHBCLKCTRL0_ES1, r1)
auto_clkctrl_end:

	cmp	r2, r5
	beq	pc_pwsw_para_es3
	/* Set Auto Control POWER SW */
	STORE_WORD(r0, SMU_PC_PWSW_PARA, SMU_PC_PWSW_PARA_ES1, r1)
	b	pc_pwsw_para_end
pc_pwsw_para_es3:
	STORE_WORD(r0, SMU_PC_PWSW_PARA, SMU_PC_PWSW_PARA_ES3, r1)
pc_pwsw_para_end:

	/* Enable Automatic Frequency Control */
	cmp	r2, r6
	beq	auto_dmdivchg_es1
	STORE_WORD(r0, SMU_AUTO_DMDIVCNG_PARAM, SMU_AUTO_DMDIVCNG_PARAM_ES2, r1)
	cmp	r2, r5
	beq	auto_ckreqmode_mask0_es3
	STORE_WORD(r0, SMU_CKRQMODE_MASK0,      SMU_CKRQMODE_MASK0_ES2, r1)
	b	auto_dmdivchg_end
auto_ckreqmode_mask0_es3:
	STORE_WORD(r0, SMU_CKRQMODE_MASK0,      SMU_CKRQMODE_MASK0_ES3, r1)
	b	auto_dmdivchg_end
auto_dmdivchg_es1:
	STORE_WORD(r0, SMU_AUTO_DMDIVCNG_PARAM, SMU_AUTO_DMDIVCNG_PARAM_ES1, r1)
	STORE_WORD(r0, SMU_CKRQMODE_MASK0,      SMU_CKRQMODE_MASK0_ES1, r1)
auto_dmdivchg_end:
	STORE_WORD(r0, SMU_AUTO_DMDIVCNG_MODE,  SMU_AUTO_DMDIVCNG_MODE_VAL, r1)
	STORE_WORD(r0, SMU_LCD_FIFOTHRESHOLD,   SMU_LCD_FIFOTHRESHOLD_VAL, r1)
	STORE_WORD(r0, SMU_DFS_FIFO_REQMASK,    SMU_DFS_FIFO_REQMASK_VAL, r1)
	STORE_WORD(r0, SMU_DFS_FIFOMODE,        SMU_DFS_FIFOMODE_VAL, r1)
	STORE_WORD(r0, SMU_CKRQMODE_MASK1,      SMU_CKRQMODE_MASK1_VAL, r1)

	SET_BASEREG(r3, SMU_S0_BASE_1000)
	LOAD_WORD(r3, SMU_CKRQMODE_MASK2, r1)
	orr	r1, r1, #SMU_CKRQMODE_MASK2_VAL
	str	r1, [r3, #SMU_CKRQMODE_MASK2 & 0xfff]

	STORE_WORD(r0, SMU_CKRQ_MODE,           SMU_CKRQ_MODE_VAL, r1)

	/* Set POWER Saving */
wait_p2pg_busy:
	LOAD_WORD(r0, SMU_SEQ_BUSY, r7)
	and	r7, r7, #SMU_SEQ_BUSY_P2PGPD
	cmp	r7, #0
	bne	wait_p2pg_busy

	STORE_WORD(r0, SMU_PD_SWON, SMU_PD_SWON_OFF, r1)
	STORE_WORD(r0, SMU_P2_SWON, SMU_P2_SWON_OFF, r1)
	STORE_WORD(r0, SMU_PG_SWON, SMU_PG_SWON_OFF, r1)
#endif

	/* Avoid Underrun */
	SET_BASEREG(r3, MEMC_BASE)
	cmp	r2, r5
	beq	set_avoid_underrun_es3

	SET_BASEREG(r4, MEMC_REQSCH)
	STORE_WORD(r4, MEMC_REQSCH, MEMC_REQSCH_URUN, r1)

	STORE_WORD(r3, MEMC_DEGFUN, MEMC_DEGFUN_URUN_ES2, r1)
	STORE_WORD(r3, MEMC_CACHE_MODE, MEMC_CACHE_MODE_URUN, r1)
	b	set_avoid_underrun_next
set_avoid_underrun_es3:
	STORE_WORD(r3, MEMC_DEGFUN, MEMC_DEGFUN_ES3, r1)
set_avoid_underrun_next:

	SET_BASEREG(r3, L2CC_BASE)
	STORE_WORD(r3, L2CC_ADD_FILTER_END, L2CC_ADD_FILTER_END_URUN, r1)
	STORE_WORD(r3, L2CC_ADD_FILTER_START, L2CC_ADD_FILTER_START_URUN, r1)

	mov	pc, r9

