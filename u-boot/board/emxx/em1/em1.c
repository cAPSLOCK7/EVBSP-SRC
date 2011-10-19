/*
 * (C) Copyright 2007, 2008
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

#define SP0_CLK_12MHZ		0x00000091	/* PLL3 / 20 */
#ifdef CONFIG_EM1_BOARD_DKIT
#define PWC_R54_LDO5ENA		0x66	/* LDO5: 3.10v */
#define PWC_R62_CHG_USB_ILIM	0x40	/* Automatic USB supply current limit enabled */

#define GIO_LANRST_SET		0x10000000	/* GIO P44 low */
#define GIO_LANRST_CLR		0x10001000	/* GIO P44 high */

#define GIO_USB_CS_N		0x80000000	/* GIO P47 low */
#define GIO_USB_RESETB		0x80008000	/* GIO P47 high */
#else
#define PWC_PSCNT_ADD		0x06
#define PWC_IOSEL1_ADD		0x10
#define PWC_IOOUT1_ADD		0x14

//#define PWC_PSCNT_DATA	0x0F	/* GP2-GP5 all ON */
#define PWC_PSCNT_DATA		0x02	/* GP3 ON */
#define PWC_P7ON_DATA		0x80
#define PWC_LANRST_SET		0x80
#define PWC_LANRST_CLR		0x00
#endif

static void uart_pll_init (void);
static void timer0_init(void);
static void pwcreg_init(void);
static void ether_init (void);
static int sp0_init(void);
static int sp0_write(unsigned char addr, unsigned char data);
static int sp0_read(register unsigned char addr, register unsigned char *data);


int board_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_arch_number = MACH_TYPE_EM1;
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	icache_enable();

	uart_pll_init();
	timer0_init();

    sp0_init();
	pwcreg_init();
	ether_init();

	return 0;
}

static void pwcreg_init(void)
{
#ifdef CONFIG_EM1_BOARD_DKIT
	unsigned int config;
	unsigned char data=0;

#ifndef CONFIG_EM1_CPU_BOARD_ONLY
	sp0_write(PWC_R54_ADD, PWC_R54_LDO5ENA);	/* PWC R54 -> LDO5: 3.10v */

	udelay(1200);					/* wait LDO5 stand-up */
#endif

	config = inl(SRAM_HWINFO_BOOT);
	if (config & E2P_CFGCPU_USB_SMSC) {
		/* Automatic USB supply current limit disabled */
		sp0_read(PWC_R62_ADD, &data);
		sp0_write(PWC_R62_ADD, (data & ~PWC_R62_CHG_USB_ILIM));
		outl(GIO_USB_RESETB, GIO_OL_H);		/* USB RESETB High */
	} else {
		outl(GIO_USB_CS_N, GIO_OL_H);		/* USB CS_N Low */
	}
#else
    sp0_write(PWC_PSCNT_ADD,  PWC_PSCNT_DATA);		/* PWC REGGP3 -> on */
#endif
}

static void uart_pll_init(void)
{
	outl(0x00000040, ASMU_GCLKCTRL2ENA);
	outl(0x00000000, ASMU_GCLKCTRL2);		/* U70_SCLK stop */
	outl(0x00000010, ASMU_DIVU70SCLK);		/* U70_SCLK select PLL3 div 2 */
	outl(0x00000040, ASMU_GCLKCTRL2);		/* U70_SCLK supply */
	outl(0x00000000, ASMU_GCLKCTRL2ENA);

	outl(UART_FCR_FIFO_ENABLE, UART0_FCR );
	outl(UART_FCR_FIFO_RESET, UART0_FCR );
}

static void ether_init(void)
{
#ifdef CONFIG_EM1_BOARD_DKIT
	outl(GIO_LANRST_SET, GIO_OL_H);			/* LAN RESET */
	udelay(2);
	outl(GIO_LANRST_CLR, GIO_OL_H);			/* Release LAN RESET */
#else
    sp0_write(PWC_IOSEL1_ADD, PWC_P7ON_DATA);		/* PWC GPIO P7 -> outport */
    sp0_write(PWC_IOOUT1_ADD, PWC_LANRST_SET);		/* LAN RESET */
	udelay(1);
    sp0_write(PWC_IOOUT1_ADD, PWC_LANRST_CLR);		/* Release LAN RESET */
#endif
}


static void timer0_init(void)
{
	outl(0x00004001, ASMU_GCLKCTRL3ENA);
	outl(0x00004001, ASMU_GCLKCTRL3);		/* TI0_TIN_GCK supply */
	outl(0x00000000, ASMU_GCLKCTRL3ENA);
	outl(0x00000001, ASMU_RESETREQ1ENA);
	outl(0x00000001, ASMU_RESETREQ1);		/* TI0 unreset */
	outl(0x00000000, ASMU_RESETREQ1ENA);

	interrupt_init();
}


void timer0_exit(void)
{
	outl(0x00004001, ASMU_GCLKCTRL3ENA);
	outl(0x00000000, ASMU_GCLKCTRL3);		/* TI0_TIN_GCK, TIM_TIN_GCK stop */
	outl(0x00004001, ASMU_GCLKCTRL3ENA);
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
#ifdef CONFIG_EM1_BOARD_DKIT
	tx_val = TX_DATA((addr << 1) | 0x01, 0);
#else
	tx_val = TX_DATA(addr, 0);
#endif
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
#ifdef CONFIG_EM1_BOARD_DKIT
	tx_val = TX_DATA(addr << 1, data);
#else
	tx_val = TX_DATA(addr, data);
#endif
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

static int sp0_init(void)
{
	/* set sp0 clock 12MHz */
	outl(SP0_CLK_12MHZ,    ASMU_DIVSP0SCLK);

	/* soft reset sp0 */
	outl(SP0_CTL_RST,      SP0_CONTROL);
	udelay(1);
	outl(SP0_CTL_RST_FREE, SP0_CONTROL);

	/* initialize hw sp0-cs0 */
	outl(SP0_MODE_CS0, SP0_MODE);
	outl(SP0_POL_CS0, SP0_POL);

	return 0;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
#ifdef CONFIG_EM1_BOARD_DKIT
	unsigned int config;
#endif
	
	printf("Booted Device: ");
#if defined(CONFIG_EMXX_EMMCBOOT)
	printf("eMMC\n");
#elif defined(CONFIG_EMXX_SDBOOT)
	printf("SD\n");
#else
	printf("NOR Flash\n");
#endif

#ifdef CONFIG_EM1_BOARD_DKIT
	config = inl(SRAM_HWINFO_BOOT);
	switch(config & E2P_CFGIO_VERSION) {
	case E2P_CFGIO_VERSION_2ND:
		printf ("board: HDK 2nd (Rev.%08x)\n", config);
		break;
	default:
		printf ("board: HDK 1st (Rev.%08x)\n", config);
		break;
	}
#endif
	return 0;
}
#endif

