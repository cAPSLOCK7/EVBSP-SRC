/*
 *
 * (C) Copyright 2007, 2009
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


#ifndef _LOWLEBEL_INIT_VAL_H
#define _LOWLEBEL_INIT_VAL_H

/*****************************************************
 * Select Clock Mode
 *****************************************************/
#define	ASMU_PLL1CTRL0_VAL			0x00000079	/* 499.712MHz */
#define	ASMU_PLL1CTRL1_VAL			0x00000000
#define	ASMU_NORMALA_DIV_VAL			0x00244200
#define	ASMU_NORMALB_DIV_VAL			0x00555533
#define	ASMU_CLK_MODE_SEL_NMA			0x00000001
#define	ASMU_CLK_MODE_SEL_NMB			0x00000002

#define ASMU_SP0CLK_12MHZ			0x00000091	/* PLL3 / 20 */

/*****************************************************
 * Setting LI HOLD
 *****************************************************/
#define	CHG_L1_HOLD_VAL				0x00000000

/*****************************************************
 * Initialize GIO
 *****************************************************/
/*     -- evaluation board -- -- module board --        -- design kit board --
   p00: ( in) 5T735_INTOUT    ( in) uPD9937_INTOUT      ( in) D9052_nIRQ
   p01: ( in) 5T735_USBWAK    ( in) uPD9937_USBWAK      (  -) -
   p02: (out) LCD_PD          (out) LCD_PD             *( in) Card Detect for SD1
   p03: (out) LCDRESET        (out) LCDRESET            ( in) Card Detect for SD0
   p04: ( in) LAN_INT         ( in) LAN_INT             (out) CAMSTANDBY
   p05: (  -) -               (out) CAM_SCLK            (out) CAM_SCLK
   p06: (out) USB_RESET       (out) USB_RESET(EXT_INT)  (out) NAND_WP
   p07: ( in) USB_INT         (  -) -                   (  -) -
   p08: (  -) -               (out) CAMERA_RESET        (out) NTSC ADC7179 /RESET
   p09: (out) Nor_VPEN        (  -) -                   (out) Audio AK4648 RESET
   p10: ( in) TouchPanel_INT  ( in)	TouchPanel_INT     *( in) WIFI5(SPI_INT to EM1)
   p11: (  -) -               (  -) -                  *(out) WIFI2(BT_Wakeup)
   p36: (  -) -               (  -) -                   ( in) PushSW1
   p37: (  -) -               (  -) -                  *(out) WIFI4(WL_Wakeup)
   p41: (  -) -               (  -) -                   ( in) LAN 9221 IRQ
   p44: (out) USB_CSN         (  -) -                   (out) LAN9221 RESET
   p46: (  -) -               (  -) -                   (out) LCD_RESET
   p47: (  -) -               (  -) -                   (out) USB ISP1507 CS_N/P
   p49: (  -) -               (  -) -                   (out) CAM_RESETZ
   p71: (  -) -               (  -) -                  *( in) PushSW2
   p94: (  -) -               (out) USB_PowSW Enable   *(out) WIFI3(WL_RESET)
   p95: (  -) -               (  -) -                  *(out) WIFI1(BT_RESET) */

/* set port output */
#if   defined(CONFIG_EM1_BOARD_DKIT)
#define	GIO_E1_L_VAL				0x00000b70
#define	GIO_E1_H_VAL				0x0002d020
#define	GIO_E1_HH_VAL				0x00000000
#elif defined(CONFIG_EM1_BOARD_MODULE)
#define	GIO_E1_L_VAL				0x0000016c
#define	GIO_E1_H_VAL				0x00000000
#define	GIO_E1_HH_VAL				0x40000000
#else
#define	GIO_E1_L_VAL				0x0000024c	/* p00-p31 */
#define	GIO_E1_H_VAL				0x00001000	/* p32-p63 */
#define	GIO_E1_HH_VAL				0x00000000	/* p64-p95 */
#endif
#define	GIO_E1_HHH_VAL				0x00000000	/* p96-p127 */

/* set port input */
#define	GIO_E0_L_VAL				0xffffffff	/* p00-p31 */
#define	GIO_E0_H_VAL				0xffffffff	/* p32-p63 */
#define	GIO_E0_HH_VAL				0xffffffff	/* p64-p95 */
#define	GIO_E0_HHH_VAL				0xffffffff	/* p96-p127 */

/* set output data */
#define	GIO_OL_L_VAL				0xffff0000	/* p00-p15 */
#define	GIO_OH_L_VAL				0xffff0000	/* p16-p31 */
#define	GIO_OL_H_VAL				0xffff0000	/* p32-p47 */
#define	GIO_OH_H_VAL				0xffff0000	/* p48-p63 */
#define	GIO_OL_HH_VAL				0xffff0000	/* p64-p79 */
//#define	GIO_OH_HH_VAL			0x40004000	/* p80-p95 (module USB HOST)*/
#define	GIO_OH_HH_VAL				0xffff0000	/* p80-p95 */
#define	GIO_OL_HHH_VAL				0xffff0000	/* p96-p111 */
#define	GIO_OH_HHH_VAL				0xffff0000	/* p112-p127 */

/* disable interrupt */
#define	GIO_IDS_L_VAL				0xffffffff	/* p00-p31 */
#define	GIO_IDS_H_VAL				0xffffffff	/* p32-p63 */
#define	GIO_IDS_HH_VAL				0xffffffff	/* p64-p95 */
#define	GIO_IDS_HHH_VAL				0xffffffff	/* p96-p127 */

/* connect fiq */
#define	GIO_GSW_L_VAL				0x00000000	/* p00-p31 */
#define	GIO_GSW_H_VAL				0x00000000	/* p32-p63 */
#define	GIO_GSW_HH_VAL				0x00000000	/* p64-p95 */
#define	GIO_GSW_HHH_VAL				0x00000000	/* p96-p127 */

/* ditect interrupt type */
#if defined(CONFIG_EM1_BOARD_DKIT)
#define	GIO_IDT0_L_VAL				0x00001003
#define	GIO_IDT1_L_VAL				0x00000000
#else
#define	GIO_IDT0_L_VAL				0x00020000	/* p00-p07 */
#define	GIO_IDT1_L_VAL				0x00000100	/* p08-p15 */
#endif
#define	GIO_IDT2_L_VAL				0x00000000	/* p16-p23 */
#define	GIO_IDT3_L_VAL				0x00000000	/* p24-p31 */
#define	GIO_IDT0_H_VAL				0x00000000	/* p32-p39 */
#if defined(CONFIG_EM1_BOARD_DKIT)
#define	GIO_IDT1_H_VAL				0x00000020
#else
#define	GIO_IDT1_H_VAL				0x00000000	/* p40-p47 */
#endif
#define	GIO_IDT2_H_VAL				0x00000000	/* p48-p55 */
#define	GIO_IDT3_H_VAL				0x00000000	/* p56-p63 */
#define	GIO_IDT0_HH_VAL				0x00000000	/* p64-p71 */
#define	GIO_IDT1_HH_VAL				0x00000000	/* p72-p79 */
#define	GIO_IDT2_HH_VAL				0x00000000	/* p80-p87 */
#define	GIO_IDT3_HH_VAL				0x00000000	/* p88-p95 */
#define	GIO_IDT0_HHH_VAL			0x00000000	/* p96-p103 */
#define	GIO_IDT1_HHH_VAL			0x00000000	/* p104-p111 */
#define	GIO_IDT2_HHH_VAL			0x00000000	/* p112-p119 */
#define	GIO_IDT3_HHH_VAL			0x00000000	/* p120-p127 */

/* clear interrupt */
#define	GIO_IIR_L_VAL				0xffffffff	/* p00-p31 */
#define	GIO_IIR_H_VAL				0xffffffff	/* p32-p63 */
#define	GIO_IIR_HH_VAL				0xffffffff	/* p64-p95 */
#define	GIO_IIR_HHH_VAL				0xffffffff	/* p96-p127 */

/* active interrupt */
#if defined(CONFIG_EM1_BOARD_DKIT)
#define	GIO_IIA_L_VAL				0x0000000d	/* p00-p31 */
#define	GIO_IIA_H_VAL				0x00000210	/* p32-p63 */
#else
#if defined(CONFIG_EM1_BOARD_MODULE)
#define	GIO_IIA_L_VAL				0x00000413	/* p00-p31 */
#else
#define	GIO_IIA_L_VAL				0x00000493	/* p00-p31 */
#endif
#define	GIO_IIA_H_VAL				0x00000000	/* p32-p63 */
#endif
#define	GIO_IIA_HH_VAL				0x00000000	/* p64-p95 */
#define	GIO_IIA_HHH_VAL				0x00000000	/* p96-p127 */

/* enable interrupt */
#define	GIO_IEN_L_VAL				0x00000000	/* p00-p31 */
#define	GIO_IEN_H_VAL				0x00000000	/* p32-p63 */
#define	GIO_IEN_HH_VAL				0x00000000	/* p64-p95 */
#define	GIO_IEN_HHH_VAL				0x00000000	/* p96-p127 */


/*****************************************************
 * Set CHG
 *****************************************************/
#define	CHG_CTRL_AB0_BOOT_VAL			0x00000001	/* Enable AB0_xx setting */

#define	CHG_PINSEL_G00_VAL			0x55400004
#define	CHG_PINSEL_G16_VAL			0x55555555
#if defined(CONFIG_EM1_BOARD_DKIT)
#define	CHG_PINSEL_G32_VAL			0x04515455
#define	CHG_PINSEL_G48_VAL			0x55555551
#else
#define	CHG_PINSEL_G32_VAL			0x54555555
#define	CHG_PINSEL_G48_VAL			0x55555555
#endif
#define	CHG_PINSEL_G64_VAL			0x00005555
#if defined(CONFIG_EM1_BOARD_MODULE)
#define	CHG_PINSEL_G80_VAL			0x4555557f
#else
#define	CHG_PINSEL_G80_VAL			0x5555557f
#endif
#define	CHG_PINSEL_G96_VAL			0x55555555
#define	CHG_PINSEL_G112_VAL			0x00000555

#define	CHG_PINSEL_SP0_VAL			0x00000000
#define	CHG_PINSEL_DTV_VAL			0x00000001
#define	CHG_PINSEL_SD0_VAL			0x00000000
#define	CHG_PINSEL_SD1_VAL			0x00000000
#define	CHG_PINSEL_IIC2_VAL			0x00000000
#define	CHG_PINSEL_REFCLKO_VAL			0x00000000


/*****************************************************
 * Set PullUp/PullDown
 *****************************************************/
#if   defined(CONFIG_EM1_BOARD_DKIT)
#define	CHG_PULL_G00_VAL			0x01116006
#define	CHG_PULL_G08_VAL			0x00000011
#elif defined(CONFIG_EM1_BOARD_MODULE)
#define	CHG_PULL_G00_VAL			0x01151155
#define	CHG_PULL_G08_VAL			0x00000501
#else
#define	CHG_PULL_G00_VAL			0x51051155
#define	CHG_PULL_G08_VAL			0x00000510
#endif
#define	CHG_PULL_G16_VAL			0x00000000
#define	CHG_PULL_G24_VAL			0x00000000
#if defined(CONFIG_EM1_BOARD_DKIT)
#define	CHG_PULL_G32_VAL			0x00050000
#define	CHG_PULL_G40_VAL			0x11010050
#define	CHG_PULL_G48_VAL			0x11111115
#else
#define	CHG_PULL_G32_VAL			0x00000000
#define	CHG_PULL_G40_VAL			0x00012200
#define	CHG_PULL_G48_VAL			0x11111155
#endif
#define	CHG_PULL_G56_VAL			0x11111111
#define	CHG_PULL_G64_VAL			0x11111111
#define	CHG_PULL_G72_VAL			0x00000000
#define	CHG_PULL_G80_VAL			0x40400044
#define	CHG_PULL_G88_VAL			0x11000666
#define	CHG_PULL_G96_VAL			0x44444444
#define	CHG_PULL_G104_VAL			0x04044044
#define	CHG_PULL_G112_VAL			0x00666661

#define	CHG_PULL0_VAL				0x50000004
#define	CHG_PULL1_VAL				0x15110600
//#define	CHG_PULL1_VAL			0x55550600
#define	CHG_PULL2_VAL				0x60661661
#define	CHG_PULL3_VAL				0x00000000


/*****************************************************
 * Set DEBUG MODE
 *****************************************************/
#define	ASMU_ACPU_DBGCTRL_VAL			0x00000001	/* DEBUG_EN ON */


/*****************************************************
 * Reset Control
 *****************************************************/
#define	ASMU_RESETREQ0ENA_VAL			0xffffffff
#define	ASMU_RESETREQ1ENA_VAL			0xffffffff
#define	ASMU_RESETREQ2ENA_VAL			0xffffffff
#define	ASMU_RESETREQ3ENA_VAL			0xffffffff

#define	ASMU_RESETREQ0_VAL			0x8ff04027	/* MEMC/U70/GIO/AINT/SRC/PB0-1/AB0-1/DMA/DCV/ACPU */
#define	ASMU_RESETREQ1_VAL			0x064b3fff	/* IIC/SP0/PDMA/PM0-1/TG0-5/TW0-3/TI0-3 */
#define	ASMU_RESETREQ2_VAL			0x0000167f	/* PMU/CHG/PWM/SHXB/DXHB/MHXB/SWL0-1/AXL0-1 */

#if defined(CONFIG_EMXX_EMMCBOOT)
#define	ASMU_RESETREQ3_VAL			0x00000012	/* SDIC/DMA */
#elif defined(CONFIG_EMXX_SDBOOT)
#define	ASMU_RESETREQ3_VAL			0x00000006	/* SDIA/DMA */
#else	/* NORBOOT */
#define	ASMU_RESETREQ3_VAL			0x00000002	/* DMA */
#endif	/* SELECT_EMMCBOOT */

#define	CHG_RST_CTRL_VAL			0x00000001	/* CHGREG Reset Enable */

/*****************************************************
 * Clock Ctrl
 *****************************************************/
#define	ASMU_GCLKCTRL0ENA_VAL			0xffffffff
#define	ASMU_GCLKCTRL1ENA_VAL			0xffffffff
#define	ASMU_GCLKCTRL2ENA_VAL			0xffffffff
#define	ASMU_GCLKCTRL3ENA_VAL			0xffffffff
#define	ASMU_GCLKCTRL4ENA_VAL			0xffffffff
#define	ASMU_GCLKCTRL0_VAL			0x760000e0	/* DMA/PDMA */
#define	ASMU_GCLKCTRL1_VAL			0x03ffc1f7	/* DCV/AB1/PB0/AXL0-1/MHXB/DHXB/SWL0-1/CHG/AINT/SRC/MEMC */
#define	ASMU_GCLKCTRL2_VAL			0x6010707e	/* SHXB/REFCLK/PM0-1/U70/IIC */
#define	ASMU_GCLKCTRL3_VAL			0xb8604409	/* FLASH/PWM/SP0/ATIM/TW0/TI0/TI3 */

#if defined(CONFIG_EMXX_EMMCBOOT)
#define	ASMU_GCLKCTRL4_VAL			0x00000390	/* PM0-1/SDIC/DMA */
#elif defined(CONFIG_EMXX_SDBOOT)
#define	ASMU_GCLKCTRL4_VAL			0x00000330	/* PM0-1/SDIA/DMA */
#else	/* NORBOOT */
#define	ASMU_GCLKCTRL4_VAL			0x00000310	/* PM0-1/DMA */
#endif	/* SELECT_EMMCBOOT */


/*****************************************************
 * Auto Clock Ctrl
 *****************************************************/
#define	ASMU_AHBCLKCTRL0_VAL			0xfdcefff3	/* all */
#define	ASMU_AHBCLKCTRL1_VAL			0x0000bfff	/* all */
#define	ASMU_APBCLKCTRL0_VAL			0x0001d1ff	/* all */
#define	ASMU_APBCLKCTRL1_VAL			0x0000affe	/* all */
#define	ASMU_APBCLKCTRL2_VAL			0x0000000c	/* all */
#define	ASMU_CLKCTRL_VAL			0x0000003f	/* all */
#define	ASMU_CLKCTRL1_VAL			0x00000003	/* all */

#define	ASMU_AHBCLKCTRL1_rmMEM_VAL		0x0000bfef	/* remove MEM */
#define	ASMU_CLKCTRL_rmMEM_VAL			0x0000003e	/* remove MEM */

/*****************************************************
 * Drive Current
 *****************************************************/
#if	  defined(CONFIG_EM1_CHIP_EM1D) || defined(CONFIG_EM1_BOARD_DKIT)
#define	CHG_DRIVE0_H_VAL			0xaaa002aa
//#define	CHG_DRIVE0_H_VAL		0xaaaaaaaa
#elif defined(CONFIG_EM1_BOARD_MODULE)
#define	CHG_DRIVE0_H_VAL			0xaaaaaaaa
#elif defined(CONFIG_EM1_BOARD_SOCKET)
#define	CHG_DRIVE0_H_VAL			0xaaaffaaa
#else
#define	CHG_DRIVE0_H_VAL			0xaaaaa6aa
#endif

//#define	CHG_DRIVE1_H_VAL		0xaaaaaaaa
#define	CHG_DRIVE1_H_VAL			0xa5aaaaaa
#define	CHG_DRIVE2_H_VAL			0x00022aaa

/*****************************************************
 * Initialize MEMC
 *****************************************************/
#define ASMU_MEMCCLK270_SEL_VAL1		0x00000001
#define MEMC_DDR_CONFIGT1_VAL1			0x00000006
#define MEMC_DDR_CONFIGT2_VAL			0x14141414
#define ASMU_MEMCCLK270_SEL_VAL2		0x00000000

#if   defined(CONFIG_EM1_CHIP_EM1D)
#define MEMC_DDR_CONFIGT1_VAL2			0x000d0803
#elif defined(CONFIG_EM1_CHIP_EM1S)
#define MEMC_DDR_CONFIGT1_VAL2			0x000f0a03
#else
#define MEMC_DDR_CONFIGT1_VAL2			0x00040803
#endif

#ifdef CONFIG_EM1_DDR_1GBit
#define MEMC_DDR_CONFIGF_VAL			0x0000000f
#else
#define MEMC_DDR_CONFIGF_VAL			0x00000015
#endif

#if defined(CONFIG_EM1_CHIP_EM1D) || defined(CONFIG_EM1_BOARD_MODULE)
#define MEMC_DDR_CONFIGA1_VAL			0x53443203
#else
#define MEMC_DDR_CONFIGA1_VAL			0x54443203
//#define MEMC_DDR_CONFIGA1_VAL			0x53343203
#endif

#ifdef CONFIG_EM1_CHIP_EM1D
#define MEMC_DDR_CONFIGA2_VAL			0x28da1042
#else
#define MEMC_DDR_CONFIGA2_VAL			0x20da1042
#endif

#define MEMC_DDR_CONFIGC2_VAL1			0x0000001d

#ifdef CONFIG_EM1_BOARD_DKIT
#define MEMC_DDR_CONFIGC1_SETA			0x40200033
#define MEMC_DDR_CONFIGC1_SETB			0x80200033
#else
#define MEMC_DDR_CONFIGC1_VAL			0x80200033
#endif
#define MEMC_DDR_CONFIGC2_VAL2			0x00000018

#define MEMC_REQSCH_VAL				0x0000001f
#define MEMC_DDR_CONFIGC2_VAL3			0x00000090

#define MEMC_DDR_CONFIGR1_VAL			0x00690069
#ifdef CONFIG_EM1_BOARD_DKIT
#define MEMC_DDR_CONFIGR2_SETA			0x37770101	/* Auto Self Refresh OFF */
#define MEMC_DDR_CONFIGR2_SETB			0x3777011f	/* Auto Self Refresh ON */
#else
#define MEMC_DDR_CONFIGR2_VAL			0x3777011f	/* Auto Self Refresh ON */
//#define MEMC_DDR_CONFIGR2_VAL			0x37770101	/* Auto Self Refresh OFF */
#endif
#define MEMC_DDR_CONFIGR3_VAL			0x00001415	/* Auto Power Down ON */
//#define MEMC_DDR_CONFIGR3_VAL			0x00001414	/* Auto Power Down OFF */

#define MEMC_CACHE_MODE_VAL			0x00000102
#define MEMC_DEGFUN_VAL				0x00000000


/*****************************************************
 * Async Bridge
 *****************************************************/
#define AB0_FLASHCLKCTRL_VAL			0x00000001
#define AB0_FLASHCOMSET_VAL			0x00000009	/* CS0,CS3 */

/* access to address range 0x00000000-0x01FFFFFF asserts CS0 */
#define	AB0_CS0BASEADD_VAL			0x00000000
#define	AB0_CS0BITCOMP_VAL			0xfe000000
/* access to address range 0x08000000-0x08FFFFFF asserts CS1 */
#define	AB0_CS1BASEADD_VAL			0x08000000
#define	AB0_CS1BITCOMP_VAL			0xff000000
/* access to address range 0x10000000-0x17FFFFFF asserts CS2 */
#define	AB0_CS2BASEADD_VAL			0x10000000
#define	AB0_CS2BITCOMP_VAL			0xf8000000
/* access to address range 0x20000000-0x2007FFFF asserts CS3 */
#define	AB0_CS3BASEADD_VAL			0x20000000
#define	AB0_CS3BITCOMP_VAL			0xfff80000

#if defined(CONFIG_EM1_BOARD_DKIT)
#define	AB0_CS0WAITCTRL_VAL			0x02000502
#define	AB0_CS0WAITCTRL_W_VAL			0x00010302
#else
#define	AB0_CS0WAITCTRL_VAL			0x02000503
#define	AB0_CS0WAITCTRL_W_VAL			0x00020303
#endif
#define	AB0_CS0READCTRL_VAL			0x00000000
#define	AB0_CS0CONTROL_VAL			0x01000104

#define	AB0_CS1WAITCTRL_VAL			0x000f1f0f
#define	AB0_CS1WAITCTRL_W_VAL			0x000f1f0f
#define	AB0_CS1READCTRL_VAL			0x00000000
#define	AB0_CS1CONTROL_VAL			0x00010100

#define	AB0_CS2WAITCTRL_VAL			0x000f1f0f
#define	AB0_CS2WAITCTRL_W_VAL			0x000f1f0f
#define	AB0_CS2READCTRL_VAL			0x00000000
#define	AB0_CS2CONTROL_VAL			0x00010100

#if defined(CONFIG_EM1_BOARD_DKIT)
#define	AB0_CS3WAITCTRL_VAL			0x01000300
#define	AB0_CS3WAITCTRL_W_VAL			0x00000300
#else
#define	AB0_CS3WAITCTRL_VAL			0x00010401
#define	AB0_CS3WAITCTRL_W_VAL			0x00010400
#endif
#define	AB0_CS3READCTRL_VAL			0x00000000
#define	AB0_CS3CONTROL_VAL			0x00010100


/*****************************************************
 * PLL Auto Frequency Change
 *****************************************************/
#define	ASMU_AUTO_FRQ_MASK0_VAL			0x00002060	/* DTV/NTS/CAM/IMC/AVC/DMA2/IPU/DMA_PCH0/PDMA/DCV/DSP/CPU */
#define	ASMU_AUTO_FRQ_MASK1_VAL			0x0000dcfd	/* AB0/MMM/USB/NAND/MSP */
#define	ASMU_AUTO_FRQ_MASK3_VAL			0x00000002	/* SDIC/SDIA */
#define	ASMU_DFS_HALFMODE_VAL			0x00000000
#define	ASMU_AUTO_FRQ_CHANGE_VAL		0xf0000001	/* divide 16 */

#define	ASMU_AUTO_FRQ_MASK0_ALL			0x000fffff	/* all request ignore */
#define	ASMU_AUTO_FRQ_MASK1_ALL			0x000fffff	/* all request ignore */
#define	ASMU_AUTO_FRQ_MASK3_ALL			0x00000007	/* all request ignore */
#define	ASMU_AUTO_FRQ_CHANGE_DIV4		0x30000001	/* divide by 4 */

#define	ASMU_PLL2CTRL0_VAL			0x00000074	/* 479.232MHz */
#define	ASMU_PLL2CTRL1_VAL			0x00000000
#ifdef CONFIG_EM1_PLL3_238MHZ
#define	ASMU_PLL3CTRL0_238MHZ			0x00000039	/* 237.567MHz */
#define	ASMU_PLL3CTRL1_STANDBY			0x000000ff
#define	ASMU_PLL3CTRL1_ACTIVE			0x00000000	/* have to wait 800usec */
#endif
#define	ASMU_PLLVDDWAIT_VAL			0x00000001
#define	ASMU_PLLLOCKTIME_VAL			0x00000002

#define	ASMU_DFS_FIFOMODE_VAL			0x00000001
#define	ASMU_LCD_FIFOTHRESHOLD_VAL		0x0000f080

/*****************************************************
 * Initialize SRAM
 *****************************************************/
#define	SRC_MODE_VAL				0x00000000


/*****************************************************
 * Set Wait Timing of Async Bus 
 *****************************************************/
#define	ASMU_AB1_U70WAITCTRL_VAL		0x00010200
#define	ASMU_AB1_U71WAITCTRL_VAL		0x00010200
#define	ASMU_AB1_U72WAITCTRL_VAL		0x00010200
#define	ASMU_AB1_IICWAITCTRL_VAL		0x00000300
#define	ASMU_AB1_SDIAWAITCTRL_VAL		0x00000300
#define	ASMU_AB1_SDIBWAITCTRL_VAL		0x00000300
#define	ASMU_AB1_SDICWAITCTRL_VAL		0x00000300
#define	ASMU_AB1_U70READCTRL_VAL		0x00000000
#define	ASMU_AB1_U71READCTRL_VAL		0x00000000
#define	ASMU_AB1_U72READCTRL_VAL		0x00000000
#define	ASMU_AB1_IICREADCTRL_VAL		0x00000000
#define	ASMU_AB1_SDIAREADCTRL_VAL		0x00000000
#define	ASMU_AB1_SDIBREADCTRL_VAL		0x00000000
#define	ASMU_AB1_SDICREADCTRL_VAL		0x00000000


/*****************************************************
 * Set Auto Control POWER SW
 *****************************************************/
#define	ASMU_POWERSW_ENA_VAL			0x000fff05
//#define	ASMU_POWERSW_ACTRL_EN_VAL	0x00000003
#define	ASMU_POWERSW_ACTRL_EN_VAL		0x00000001

/*****************************************************
 * Set Auto Control POWER SW
 *****************************************************/
#define	ASMU_AUTO_MODE_EN_VAL			0x00020000
#define	ASMU_NORMALC_DIV_VAL			0x00244222
#define	ASMU_NORMALD_DIV_VAL			0x00355333
#define	ASMU_ECONOMY_DIV_VAL			0x00244200
#define	ASMU_STANDBY_DIV_VAL			0x00055500
#define	ASMU_POWERON_DIV_VAL			0x00133100
#define	ASMU_QR_CLKDIV_VAL			0x00020402
#define	ASMU_PWRCNT_VAL				0x00000000


/*****************************************************
 * Set Initial value to Secure interrupt mask (MSE)
 *****************************************************/
#define	AINT_IT0_IENS0_VAL			0xffffffff
#define	AINT_IT0_IENS1_VAL			0xffffffff
#define	AINT_IT0_IENS2_VAL			0xffffffff


/*****************************************************
 * Set Initial value to Power SW Control
 *****************************************************/
#define	ASMU_LOG2SW_ACTRLEN_VAL			0x00530101
//#define	ASMU_LOG3SW_ACTRLEN_VAL		0x00530101
#define	ASMU_LOG3SW_ACTRLEN_VAL			0x00000100

#define	ASMU_L3_POWERSW_BUZ_VAL_1		0x00000000
#define	ASMU_L3_POWERSW_BUZ_VAL_2		0x00000001
#define	ASMU_L3_POWERSW_BUZ_VAL_3		0x00000003
#define	ASMU_L3_POWERSW_BUZ_VAL_4		0x00000007
#define	ASMU_L3_POWERSW_BUZ_VAL_5		0x0000000f
#define	ASMU_L3_POWERSW_BUZ_VAL_6		0x0000001f
#define	ASMU_L3_POWERSW_BUZ_VAL_7		0x0000003f
#define	ASMU_L3_POWERSW_BUZ_VAL_8		0x0000007f
#define	ASMU_L3_POWERSW_BUZ_VAL_9		0x000000ff


/*****************************************************
 * Set LCD LCLOCK DIV
 *****************************************************/
#define	ASMU_DIVLCDLCLK_VAL			0x00000042	/* use PLL2, 1/20 */


/*****************************************************
 * Set Select USB Core
 *****************************************************/
#define	ASMU_SEL_BIGWEST_VAL			0x00000001


/*****************************************************
 * Set Initial value to AAXI
 *****************************************************/
#define	AXI_LAZY_LOCK_VAL			0x00000002


/*****************************************************
 * Set Initial value to UART
 *****************************************************/
#define UART_FCR_FIFO_ENABLE			0x01
#define UART_FCR_FIFO_RESET			0x07


#if defined(CONFIG_EM1_BOARD_DKIT)
/*****************************************************
 * Get HW Development Kit Configuration
 *****************************************************/
#define GIO_LANRST_SET				0x10000000	/* GIO P44 low */
#define GIO_LANRST_CLR				0x10001000	/* GIO P44 high */

#define E2P_CMD_EPC_BUSY			0x80000000
#define E2P_CMD_EPC_CMD				0x70000000
#define E2P_CMD_EPC_TIMEOUT 			0x00000200
#define E2P_CMD_MAC_ADDR_LOADED			0x00000100
#define E2P_CMD_EPC_ADDR			0x000000ff

#define E2P_DATA_EEPROM_DATA			0x000000ff

#define E2P_CMD_EPC_CMD_READ			0x00000000
#define E2P_CMD_EPC_CMD_EWDS			0x10000000
#define E2P_CMD_EPC_CMD_EWEN			0x20000000
#define E2P_CMD_EPC_CMD_WRITE			0x30000000
#define E2P_CMD_EPC_CMD_WRAL			0x40000000
#define E2P_CMD_EPC_CMD_ERASE			0x50000000
#define E2P_CMD_EPC_CMD_ERAL			0x60000000
#define E2P_CMD_EPC_CMD_RELOAD  		0x70000000

/* ------------------------------------------------------ */
#define E2P_ADDR_CFGCPU				0x10
#define E2P_ADDR_CFGPWC				0x11
#define E2P_ADDR_CFGIO				0x12

#define E2P_READ_CFGCPU				(E2P_CMD_EPC_BUSY | E2P_CMD_EPC_CMD_READ | E2P_ADDR_CFGCPU)
#define E2P_READ_CFGPWC				(E2P_CMD_EPC_BUSY | E2P_CMD_EPC_CMD_READ | E2P_ADDR_CFGPWC)
#define E2P_READ_CFGIO				(E2P_CMD_EPC_BUSY | E2P_CMD_EPC_CMD_READ | E2P_ADDR_CFGIO)
/* ------------------------------------------------------ */
#define	E2P_CFGCPU_MASK				0x000000ff
#define	E2P_CFGPWC_MASK				0x0000ff00
#define	E2P_CFGIO_MASK				0x00ff0000

#define E2P_CFGCPU_CHIP				0x00000080	/* bit7   */
#define E2P_CFGCPU_CHIP_EM1S			0x00000000
#define E2P_CFGCPU_CHIP_EM1D512			0x00000080

#define E2P_CFGCPU_USB				0x00000040	/* bit6   */
#define E2P_CFGCPU_USB_NXP			0x00000000
#define E2P_CFGCPU_USB_SMSC			0x00000040

#define E2P_CFGCPU_DDRC1			0x00000020	/* bit5   */
#define E2P_CFGCPU_DDRC1_SETA			0x00000000
#define E2P_CFGCPU_DDRC1_SETB			0x00000020

#define E2P_CFGCPU_eMMC				0x00000010	/* bit4   */
#define E2P_CFGCPU_eMMC_Micron			0x00000000
#define E2P_CFGCPU_eMMC_SAMSUNG			0x00000010

#define E2P_CFGCPU_DDRR2			0x00000008	/* bit3   */
#define E2P_CFGCPU_DDRR2_SETA			0x00000000
#define E2P_CFGCPU_DDRR2_SETB			0x00000008

#define E2P_CFGCPU_RESERVE			0x00000007	/* bit2-0 */
/* ------------------------------------------------------ */
#define E2P_CFGPWC_DA9052			0x0000e000	/* bit7-5 */
#define E2P_CFGPWC_DA9052_ES			0x00000000
#define E2P_CFGPWC_DA9052_ASi			0x00002000

#define E2P_CFGPWC_OTP				0x00001c00	/* bit4-2 */
#define E2P_CFGPWC_OTP_r04			0x00000000
#define E2P_CFGPWC_OTP_r04m			0x00000400
#define E2P_CFGPWC_OTP_r09			0x00008000

#define E2P_CFGPWC_RESERVE			0x00000300	/* bit1-0 */
/* ------------------------------------------------------ */
#define E2P_CFGIO_NAND				0x00800000	/* bit7   */
#define E2P_CFGIO_NAND_NONE			0x00000000
#define E2P_CFGIO_NAND_EXIST			0x00800000

#define E2P_CFGIO_VERSION			0x00700000	/* bit6-4   */
#define E2P_CFGIO_VERSION_1ST			0x00000000
#define E2P_CFGIO_VERSION_2ND			0x00100000

#define E2P_CFGIO_RESERVE			0x000f0000	/* bit3-0 */
/* ------------------------------------------------------ */
#define CFG_HDK1_DUMMY				0xd0000000	/* for cpu board only */
#define CFG_HDK2_DUMMY				0xd0102860	/* for cpu board only */
#define USE_HDK_VERSION				CFG_HDK2_DUMMY
#endif


#endif /* _LOWLEBEL_INIT_VAL_H */
