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

#include <common.h>
#include <asm/io.h>
#include "reg.h"
#include "lowlevel_init_val.h"
#include "spi.h"

#ifndef outl
#define outl	writel
#endif
#ifndef inl
#define inl	readl
#endif

#define RTC_ALM_MIN		PWC_R117_ADD
#define RTC_ALM_YEAR		PWC_R121_ADD
#define RTC_ALM_MASK		PWC_R10_ADD

#define RTC_ALARMY_TICK_ON	0x80
#define RTC_TICK_TYPE_MIN	0x80
#define RTC_M_ALARM		0x20

static void uart_pll_init (void);
static void timer0_init(void);
static void pwcreg_init(void);
static void ether_init (void);
static void sp0_init(void);
static void sp0_deinit(void);
static int sp0_write(unsigned char addr, unsigned char data);
static int sp0_read(register unsigned char addr, register unsigned char *data);


int board_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_arch_number = MACH_TYPE_EMXX;
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	icache_enable();

	uart_pll_init();
	timer0_init();

	sp0_init();
	pwcreg_init();
	sp0_deinit();

	ether_init();

	return 0;
}

static void pwcreg_init(void)
{
#if 0
	sp0_write(PWC_R54_ADD, PWC_R54_LDO5ENA);	/* PWC R54 -> LDO5: 3.10v */

	udelay(1200);					/* wait LDO5 stand-up */
#endif
}

static void uart_pll_init(void)
{
	outl(0x00000000, SMU_USIAU0GCLKCTRL);	/* UART0_CLK stop */
	outl(0x00000001, SMU_USIAU0SCLKDIV);	/* UART0_CLK select PLL3 div 2 */
	outl(0x00000003, SMU_USIAU0GCLKCTRL);	/* UART0_CLK supply */

	outl(UART_FCR_FIFO_ENABLE, UART0_FCR);
	outl(UART_FCR_FIFO_RESET, UART0_FCR);
}

static void ether_init(void)
{
}


static void timer0_init(void)
{
	outl(SMU_TWI0TIN_SEL_32K, SMU_TWI0TIN_SEL);	/* TI0/TW0 div */
	outl(0x00000001, SMU_TIMGCLKCTRL);		/* TIM_PCLK supply */
	outl(0x00000001, SMU_TI0GCLKCTRL);		/* TI0_CLK supply */
	outl(0x00000001, SMU_TI0_RSTCTRL);		/* TI0 unreset */

	interrupt_init();
}


void timer0_exit(void)
{
	outl(0x00000000, SMU_TI0GCLKCTRL);		/* TI0_CLK stop */
}

int dram_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size  = PHYS_SDRAM_1_SIZE;
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size  = PHYS_SDRAM_2_SIZE;

	return 0;
}

static int sp0_read(register unsigned char addr, register unsigned char *data)
{
	unsigned int tx_val;

	if( !(inl(SP0_CONTROL) & SP0_CTL_TX_EMP) ) {
		sp0_init();
	}
	if( !(inl(SP0_CONTROL) & SP0_CTL_RX_EMP) ) {
		sp0_init();
	}

	/* set tx data */
	tx_val = TX_DATA((addr << 1) | 0x01, 0);
	outl(tx_val, SP0_TX_DATA);

	/* tx/rx start */
	outl((SP0_CTL_WRT_ENA | SP0_CTL_RD_ENA | SP0_CTL_START), SP0_CONTROL);

	/* wait rx end */
	while( inl(SP0_CONTROL) & (SP0_CTL_START | SP0_CTL_RX_EMP) ) {
	}

	/* read rx data */
	*data = (unsigned char)inl(SP0_RX_DATA);

	/* tx/rx stop */
	outl(0, SP0_CONTROL);

	return 0;
}

static int sp0_write(register unsigned char addr, register unsigned char data)
{
	unsigned int tx_val;

	if( !(inl(SP0_CONTROL) & SP0_CTL_TX_EMP) ) {
		sp0_init();
	}

	/* set tx data */
	tx_val = TX_DATA(addr << 1, data);
	outl(tx_val, SP0_TX_DATA);

	/* tx start */
	outl((SP0_CTL_WRT_ENA | SP0_CTL_START), SP0_CONTROL);

	/* wait tx end */
	while( inl(SP0_CONTROL) & SP0_CTL_START ) {
	}

	/* tx stop */
	outl(0, SP0_CONTROL);

	return 0;
}

static void sp0_init(void)
{
	unsigned int tmp;

	/* set sp0 clock 12MHz */
	outl(0x00000000, SMU_USIAS0GCLKCTRL);		/* SPI0_CLK stop */
	tmp = inl(SMU_USIASCLKDIV) & ~SP0_CLK_MASK;
	outl((tmp | SP0_CLK_12MHZ), SMU_USIASCLKDIV);	/* SPI0_CLK select PLL3 div 20 */
	outl(0x00000007, SMU_USIAS0GCLKCTRL);		/* SPI0_CLK supply */

	/* soft reset sp0 */
	outl(SP0_CTL_RST,      SP0_CONTROL);
	udelay(1);
	outl(SP0_CTL_RST_FREE, SP0_CONTROL);

	/* initialize hw sp0-cs0 */
	outl(SP0_MODE_CS0, SP0_MODE);
	outl(SP0_POL_CS0, SP0_POL);
}

static void sp0_deinit(void)
{
	/* SPI0_CLK stop */
	outl(0x00000000, SMU_USIAS0GCLKCTRL);
}


#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	unsigned int  config;
	unsigned char val;

	printf("Booted Device: ");
#if defined(CONFIG_EMXX_ESDBOOT)
	printf("eSD\n");
#elif defined(CONFIG_EMXX_EMMCBOOT)
	printf("eMMC\n");
#elif defined(CONFIG_EMXX_SDBOOT)
	printf("SD\n");
#elif defined(CONFIG_EMXX_NANDBOOT)
	printf("NAND\n");
#else
	printf("NOR Flash\n");
#endif
#if 1
	config = inl(SRAM_HWINFO_BOOT);
	switch(config & SMU_CHIP_REVISION_MASK) {
	case SMU_CHIP_REVISION_ES3:
		printf ("Board: EM/EV ES3 (Rev.%08x)\n", config);
		break;
	case SMU_CHIP_REVISION_ES2:
		printf ("Board: EM/EV ES2 (Rev.%08x)\n", config);
		break;
	default:
		printf ("Board: EM/EV ES1 (Rev.%08x)\n", config);
		break;
	}

	sp0_init();
	sp0_read(PWC_R46_ADD,  (unsigned char *)&val);
	sp0_deinit();
	switch(val) {
	case BUCKCORE_110v:
		printf ("Core:  1.10v\n");
		break;
	case BUCKCORE_115v:
		printf ("Core:  1.15v\n");
		break;
	case BUCKCORE_120v:
		printf ("Core:  1.20v\n");
		break;
	case BUCKCORE_125v:
		printf ("Core:  1.25v\n");
		break;
	default:
		printf ("Core:  1.30v\n");
		break;
	}
#endif

	return 0;
}
#endif

void reset_cpu(ulong addr)
{
	unsigned char min_val, year_val, mask_val;

	sp0_init();

	/* setup for RTC interrupt */
	sp0_read(RTC_ALM_MIN,  (unsigned char *)&min_val);
	sp0_read(RTC_ALM_YEAR, (unsigned char *)&year_val);
	sp0_read(RTC_ALM_MASK, (unsigned char *)&mask_val);

	min_val  &= ~RTC_TICK_TYPE_MIN;		/* one second */
	year_val |=  RTC_ALARMY_TICK_ON;
	mask_val &= ~RTC_M_ALARM;

	sp0_write(RTC_ALM_MIN,  min_val);
	sp0_write(RTC_ALM_YEAR, year_val);

	udelay(61);
	sp0_write(RTC_ALM_MASK, mask_val);

	/* setup for power down */
	sp0_read(PWC_R46_ADD,  (unsigned char *)&mask_val);
	mask_val &= 0xc0;
	mask_val |= 0x18;
	sp0_write(PWC_R46_ADD, mask_val);
	sp0_write(PWC_R20_ADD, 0x41);
	sp0_write(PWC_R29_ADD, 0x95);
	sp0_write(PWC_R45_ADD, 0x88);
	sp0_write(PWC_R43_ADD, 0xC1);
	/* for DA9052 auto wake. */
	sp0_write(PWC_R25_ADD, 0x11);
	/* power down */
	sp0_write(PWC_R15_ADD, 0xAD);

	sp0_deinit();
}

