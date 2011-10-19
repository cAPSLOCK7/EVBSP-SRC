/*
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

#ifndef _LOWLEBEL_INIT_VAL_H
#define _LOWLEBEL_INIT_VAL_H

#define EMXX_PWC_CHANGE_CORE
#ifndef CONFIG_EMXX_UNUSE_AB
#define EMXX_READ_VERSION
#endif

/*****************************************************
 * Chip Revison
 *****************************************************/
#define SMU_CHIP_REVISION_MASK		0x000000ff
#define SMU_CHIP_REVISION_ES1		0x00000010
#define SMU_CHIP_REVISION_ES2		0x00000020
#define SMU_CHIP_REVISION_ES3		0x00000030

/*****************************************************
 * Select Clock Mode
 *****************************************************/
#if defined(CONFIG_EMXX_PALLADIUM)
#define SMU_PLL1CTRL1_VAL		0x00000000
#define SMU_PLL2CTRL1_VAL		0x00000000
#define SMU_PLL4CTRL1_VAL		0x00000000
#define SMU_NORMALA_DIV_VAL		0x06426200
#define SMU_CKMODE_PLLSEL_VAL		0xd00c0004	/* use PLL2 */
#else
#define SMU_OSC1CTRL1_VAL		0x00000000
#define SMU_ROSCCTRL1_VAL		0x00000001
#define SMU_PLL1CTRL0_VAL		0x100104dd	/* 533MHz */
#define SMU_PLL1CTRL0_500M		0x100105f9	/* 500MHz */
#define SMU_PLL1CTRL1_VAL		0x00000000
#define SMU_PLL2CTRL0_VAL		0x00000058
#define SMU_PLL2CTRL1_VAL		0x00000000
#define SMU_PLL4CTRL0_VAL		0x00000077
#define SMU_PLL4CTRL1_VAL		0x00000000
#define SMU_CKMODE_PLLSEL_VAL		0xe00c0000	/* Normal use PLL1, Sleep use OSC1 */

#define SMU_NORMALA_DIV_ES2		0x01315100	/* CPU:DDR=533:266 */
#ifdef CONFIG_EMEV_CPU_500MHZ
#define SMU_NORMALA_DIV_ES1		0x02424200	/* CPU:DDR=500:166 */
#else
#define SMU_NORMALA_DIV_ES1		0x01315100	/* CPU:DDR=533:266 */
#endif
#define SMU_NORMALB_DIV_VAL		0x01315100
#define SMU_NORMALC_DIV_VAL		0x01315100
#define SMU_NORMALD_DIV_VAL		0x01315100
#define SMU_ECONOMY_DIV_VAL		0x03535300
#define SMU_SLEEP_DIV_VAL		0x00101000

#define SMU_CPUCLK_ASYNC_MODE_VAL	0x00000000	/* use CPU_DOMAINCLK */
#define SMU_DSPCLK_ASYNC_MODE_VAL	0x00000000	/* use DSP_DOMAINCLK */
#define SMU_CPUCLK_SYNCSET_VAL		0x00000000
#define SMU_CPUCLK_SYNCMODEACK		0x00030000
#endif

#define SMU_CLK_MODE_SEL_VAL		0x00000001	/* use normalA */
#define SMU_OSC_CX_VAL			0x0000000b
#define SMU_OSCLOCKTIME_VAL		0x00440044	/* OSC0/OSC1: 2ms */
#define SMU_PLLLOCKTIME0_VAL		0x00420007	/* PLL1:213.5[us], PLL2:2013[us] */
#define SMU_PLLLOCKTIME1_VAL		0x0042001B	/* PLL3:823.5[us], PLL4:2013[us] */

/*****************************************************
 * Check Fuse
 *****************************************************/
#define AFS_CPUENA_MASK			0xc0000000
#define AFS_CPUENA_MULTI		0x00000000
#define AFS_CPUDIS_CPU0			0x80000000
#define AFS_CPUDIS_CPU1			0x40000000

/*****************************************************
 * Initialize PINSEL
 *****************************************************/
#define CHG_PINSEL_G000_VAL		0xff0bffeb
#define CHG_PINSEL_G032_VAL		0x00020000
#define CHG_PINSEL_G064_VAL		0x00000000
#define CHG_PINSEL_G096_VAL		0x00780000
#define CHG_PINSEL_G128_VAL		0x02000000

#define CHG_PINSEL_LCD3_BIT		0x00000ff3
#define CHG_PINSEL_LCD3_VAL		0x00000000
#define CHG_PINSEL_UART_VAL		0x00000000
#define CHG_PINSEL_IIC_VAL		0x00000000
#define CHG_PINSEL_SD_VAL		0x00000000
#define CHG_PINSEL_AB_BIT		0x00003fff
#define CHG_PINSEL_AB_VAL		0x00000000
#define CHG_PINSEL_USI_VAL		0x00000000
#define CHG_PINSEL_NTSC_VAL		0x00000000
#define CHG_PINSEL_CAM_VAL		0x00000000
#define CHG_PINSEL_HSI_VAL		0x00000000

/*****************************************************
 * Initialize GIO
 *****************************************************/

/*****************************************************
 * Set PullUp/PullDown
 *****************************************************/
#define CHG_PULL0_BIT			0xffffff0f
#define CHG_PULL0_VAL			0x00000400
#define CHG_PULL1_VAL			0x00000000
#define CHG_PULL2_VAL			0x00000000
#define CHG_PULL3_VAL			0xcccc0700
#define CHG_PULL4_VAL			0x0dc07ddc
#define CHG_PULL5_VAL			0xdddddddd
#define CHG_PULL6_VAL			0x0dddddc0
#define CHG_PULL7_VAL			0x70000000
#define CHG_PULL8_VAL			0x44444440
#define CHG_PULL9_VAL			0x44444444
#define CHG_PULL10_VAL			0x00000004
#define CHG_PULL11_VAL			0x00000000
#define CHG_PULL12_VAL			0x00004044
#define CHG_PULL13_VAL			0x00044044
#define CHG_PULL14_VAL			0x00000000
#define CHG_PULL15_VAL			0x00004044
#define CHG_PULL16_VAL			0x00044040
#define CHG_PULL17_VAL			0x00000004
#define CHG_PULL18_VAL			0x44400000
#define CHG_PULL19_VAL			0x44444444
#define CHG_PULL20_BIT			0x0f0fffff
#define CHG_PULL20_VAL			0x00004444
#define CHG_PULL21_VAL			0x40000444
#define CHG_PULL22_BIT			0xf0000000
#define CHG_PULL22_VAL			0x40000000
#define CHG_PULL25_VAL			0x00000404

#define CHG_LCD_ENABLE_V33		0x00000001	/* v3.3 enable */
#define CHG_P1_LAT_VAL			0x00002222	/* output through */
#define CHG_BUSHOLD_VAL			0x0000ffff

/*****************************************************
 * Set DEBUG MODE
 *****************************************************/

/*****************************************************
 * Reset Control
 *****************************************************/
#define SMU_GIO_RSTCTRL_VAL		0x00000001
#define SMU_CHG1_RSTCTRL_VAL		0x00000001
#define SMU_PMU_RSTCTRL_VAL		0x00000001
#define SMU_MEMC_RSTCTRL_VAL		0x00000001
#define SMU_USIAS0_RSTCTRL_VAL		0x00000003
#define SMU_USIAU0_RSTCTRL_VAL		0x00000003
#define SMU_IIC0_RSTCTRL_VAL		0x00000003
#define SMU_TI0_RSTCTRL_VAL		0x00000001

/*****************************************************
 * Clock Ctrl
 *****************************************************/
#define SMU_SDIO0GCLKCTRL_ENA		0x00000007
#define SMU_SDIO0SCLKDIV_VAL		0x00000004

#define SMU_SDCGCLKCTRL_ENA		0x00000005

#define SMU_USIAS0GCLKCTRL_ENA		0x00000007
#define SMU_USIAS0GCLKCTRL_DIS		0x00000000

/*****************************************************
 * Auto Clock Ctrl
 *****************************************************/
#define SMU_ACNT0_ES1			0x02222220	/* bus1 0[clk], other 3[clk] */
#define SMU_ACNT0_ES2			0x02222222	/* all 3[clk] */
#define SMU_ACNT1_VAL			0x00022222	/* all 3[clk] */
#define SMU_AHBCLKCTRL0_ES1		0x03370071	/* bus1,pbl0,pbl1,ahb,ahbh off */
#define SMU_AHBCLKCTRL0_ES2		0x03373271	/* ahb,ahbh off */
#define SMU_AHBCLKCTRL1_VAL		0x13320333	/* a3dmem off */
#define SMU_AHBCLKCTRL2_VAL		0x03333333	/* all on */
#define SMU_AHBCLKCTRL3_VAL		0x111001f7	/* all on */
#define SMU_APBCLKCTRL0_VAL		0x11731331	/* all on */
#define SMU_APBCLKCTRL1_VAL		0x01333137	/* all on */
#define SMU_APBCLKCTRL2_VAL		0x13316fff	/* all on */
#define SMU_CLKCTRL_VAL			0x0021f331	/* a3dcore off */

/*****************************************************
 * Drive Current
 *****************************************************/
#define CHG_DRIVE0_BIT			0x0000c000
#define CHG_DRIVE0_VAL			0x00008000
#define CHG_DRIVE1_BIT			0xf00ffc00
#define CHG_DRIVE1_VAL			0xa00aa800

/*****************************************************
 * Initialize MEMC
 *****************************************************/
#if defined(CONFIG_EMXX_PALLADIUM)
#define SMU_MEMCRCLKDIV_VAL1		0x00000015
#define SMU_MEMCRCLKDIV_VAL2		0x00000015
#define SMU_MEMCCLK270_SEL_VAL1		0x00000001
#define SMU_MEMCCLK270_SEL_VAL2		0x00000000

#define MEMC_CACHE_MODE_VAL		0x0000ffff

#define MEMC_DEGFUN_VAL			0x00000003
#define MEMC_REQSCH_VAL			0x0000001f
#define MEMC_DDR_CONFIGF_VAL		0x00000f0f
#define MEMC_DDR_CONFIGA1_VAL		0x54443204
#define MEMC_DDR_CONFIGA2_VAL		0x20da1042
#define MEMC_DDR_CONFIGC1_VAL		0x80400033
#define MEMC_DDR_CONFIGC2_VAL1		0x000000b0
#define MEMC_DDR_CONFIGC2_VAL2		0x0000003d
#define MEMC_DDR_CONFIGC2_VAL3		0x00000038
#define MEMC_DDR_CONFIGR1_VAL		0x00000049
#define MEMC_DDR_CONFIGR2_VAL		0x1f5f1f1f
#define MEMC_DDR_CONFIGR3_VAL		0x00003d3d
#define MEMC_DDR_CONFIGC2_VAL4		0x000000b0
#else
#define MEMC_DDR_CONFIGD_VAL		0x00009131
#define MEMC_DDR_CONFIGZD_VAL		0x01810181
#define MEMC_DDR_CONFIGZC_VAL		0x00e100e1
#define MEMC_DDR_CONFIGZA_VAL		0x01810181

#define MEMC_REQSCH_VAL      		0x0000001f

#define MEMC_DDR_CONFIGF_VAL 		0x8e000004
#define MEMC_DDR_CONFIGA1_VAL		0x5c4a4517
#define MEMC_DDR_CONFIGA2_ES1		0x8800a840
#define MEMC_DDR_CONFIGA2_ES2		0x8800aa60
#define MEMC_DDR_CONFIGA2_ES3		0x8800a840

#define MEMC_DDR_CONFIGR3_VAL1		0xc11a0000

#define MEMC_DDR_CONFIGC1_VAL1		0x40400043
#define MEMC_DDR_CONFIGC2_VAL1		0x0000001d
#define MEMC_DDR_CONFIGC2_VAL2		0x000000c0
#define MEMC_DDR_CONFIGC1_VAL2		0x00000400
#define MEMC_DDR_CONFIGC2_VAL3		0x00000040

#define MEMC_DDR_CONFIGC1_VAL3		0x00000400
#define MEMC_DDR_CONFIGC2_VAL4		0x00000019
#define MEMC_DDR_CONFIGC1_VAL4		0x80000000
#define MEMC_DDR_CONFIGC2_VAL5		0x0000001f
#define MEMC_DDR_CONFIGC1_VAL5		0xc0000000
#define MEMC_DDR_CONFIGC2_VAL6		0x0000001f
#define MEMC_DDR_CONFIGC1_VAL6		0x40000000
#define MEMC_DDR_CONFIGC2_VAL7		0x0000001f
#define MEMC_DDR_CONFIGC1_VAL7		0x07430000
#define MEMC_DDR_CONFIGC2_VAL8		0x0000001f
#define MEMC_DDR_CONFIGC1_VAL8		0x00000400
#define MEMC_DDR_CONFIGC2_VAL9		0x00000019
#define MEMC_DDR_CONFIGC2_VAL10		0x0000001a
#define MEMC_DDR_CONFIGC2_VAL11		0x0000001a
#define MEMC_DDR_CONFIGC1_VAL9		0x06430000
#define MEMC_DDR_CONFIGC2_VAL12		0x0000001f
#define MEMC_DDR_CONFIGC1_VAL10		0x43820000
#define MEMC_DDR_CONFIGC2_VAL13		0x0000001f
#define MEMC_DDR_CONFIGC1_VAL11		0x40000000
#define MEMC_DDR_CONFIGC2_VAL14		0x0000001f

#define MEMC_DDR_CONFIGR1_VAL		0x00000069
#define MEMC_DDR_CONFIGR2_VAL		0x0037001f
#define MEMC_DDR_CONFIGR3_VAL2		0xc11a0000

#ifdef CONFIG_EMEV_CPU_500MHZ
#define MEMC_DDR_CONFIGT1_ES1		0x00000000
#define MEMC_DDR_CONFIGT2_ES1		0x110f1111
#else
#define MEMC_DDR_CONFIGT1_ES1		0x00000800
#define MEMC_DDR_CONFIGT2_ES1		0x03030303
#endif
#define MEMC_DDR_CONFIGT1_ES2		0x00000F00
#define MEMC_DDR_CONFIGT2_ES2		0x03030303

#define MEMC_DDR_CONFIGT1_ES3		0x00000000
#define MEMC_DDR_CONFIGT2_ES3		0x08080808

#define MEMC_DDR_CONFIGC2_VAL15		0x000000b0

#define MEMC_CACHE_MODE_VAL		0x00000000

#define SMU_MEMCRCLKDIV_VAL		0x0000000f
#define SMU_MEMCCLK270_SEL_VAL		0x00000000
#endif

/*****************************************************
 * Async Bridge
 *****************************************************/
#if defined(CONFIG_EMXX_PALLADIUM)
#define AB0_FLASHCOMSET_VAL		0x00000001	/* CS0 */
#else
#define AB0_FLASHCOMSET_VAL		0x00000007	/* CS0/CS1/CS2 */
#endif

/* NOR */
#define AB0_CS0BASEADD_VAL		0x00000000
#define AB0_CS0BITCOMP_VAL		0xf0000000

/* Ether */
#define AB0_CS1BASEADD_VAL		0x20000000
#define AB0_CS1BITCOMP_VAL		0xfff80000

/* NAND */
#define AB0_CS2BASEADD_VAL		0x10000000
#define AB0_CS2BITCOMP_VAL		0xf8000000

#if defined(CONFIG_EMXX_PALLADIUM)
#define	AB0_CS0WAITCTRL_VAL		0x00010101
#else
#define	AB0_CS0WAITCTRL_VAL		0x02020306
#endif
#define	AB0_CS0WAITCTRL_W_VAL		0x00020400
#define	AB0_CS0READCTRL_VAL		0x00000000

#define	AB0_CS1WAITCTRL_VAL		0x02000400
#define	AB0_CS1WAITCTRL_W_VAL		0x00020400
#define	AB0_CS1READCTRL_VAL		0x00000000

#define AB0_CS2WAITCTRL_VAL		0x03000200
#define AB0_CS2WAITCTRL_W_VAL		0x00010100
#define AB0_CS2READCTRL_VAL		0x00000000

/*****************************************************
 * PLL Auto Frequency Change
 *****************************************************/
#ifdef CONFIG_EMEV_CPU_500MHZ
#define SMU_AUTO_DMDIVCNG_PARAM_ES1	0x06666266
#else
#define SMU_AUTO_DMDIVCNG_PARAM_ES1	0x07777177
#endif
#define SMU_AUTO_DMDIVCNG_PARAM_ES2	0x07777177
#define SMU_AUTO_DMDIVCNG_MODE_VAL	0x00000001

#define SMU_LCD_FIFOTHRESHOLD_VAL	0x03980200
#define SMU_DFS_FIFO_REQMASK_VAL	0x00000007
#define SMU_DFS_FIFOMODE_VAL		0x00000001

#define SMU_CKRQMODE_MASK0_ES1		0x000000e0
#define SMU_CKRQMODE_MASK0_ES2		0x000000a0
#define SMU_CKRQMODE_MASK0_ES3		0x00000020
#define SMU_CKRQMODE_MASK1_VAL		0x77fc4fff
#define SMU_CKRQMODE_MASK2_VAL		0x00000001
#define SMU_CKRQ_MODE_VAL		0x00000001

/*****************************************************
 * Set Auto Control POWER SW
 *****************************************************/
#define SMU_SEQ_BUSY_P2PGPD		0x000001a0
#define SMU_PD_SWON_OFF			0x00000100	/* sw-off: power down mode */
#define SMU_P2_SWON_OFF			0x00000100	/* sw-off: power down mode */
#define SMU_PG_SWON_OFF			0x00000000	/* sw-off: retention mode */

/*****************************************************
 * Set Initial value to Power SW Control
*****************************************************/
#define SMU_PC_PWSW_PARA_ES1		0x00000047
#define SMU_PC_PWSW_PARA_ES3		0x00000006

/*****************************************************
 * Set LCD LCLOCK DIV
 *****************************************************/

/*****************************************************
 * Initialize Interrupt
 *****************************************************/
#define INTA_CPU_CTRL_VAL		0x00000003	/* Enable */
#define INTA_CPU_PRIMASK_VAL		0x000000f0	/* All Enable */

#define INTA_DIST_CTRL_VAL		0x00000000	/* Disable */

#define INTD_IT_PINV_SET2_VAL		0x80000000	/* Change INT_COMMTX Low Active */

/*****************************************************
 * Set Initial value to SP0
 *****************************************************/
#define SP0_CLK_MASK			0x000003ff	/* USIA-SIO0 */
#define SP0_CLK_12MHZ			0x0000002d	/* PLL3 / 20 */

/*****************************************************
 * Set Initial value to UART
 *****************************************************/
#define UART_FCR_FIFO_ENABLE		0x01
#define UART_FCR_FIFO_RESET		0x07

/*****************************************************
 * Set Initial value to TI0
 *****************************************************/
#define SMU_TWI0TIN_SEL_32K		0x11

/*****************************************************
 * Avoid Underrun
 *****************************************************/
#define MEMC_CACHE_MODE_URUN		0x00200000
#define MEMC_DEGFUN_URUN_ES2		0x00000200
#define MEMC_DEGFUN_URUN_ES3		0x00000400
#define MEMC_DEGFUN_LCDDIRECT		0x00000020
#define MEMC_DEGFUN_ES3			(MEMC_DEGFUN_URUN_ES3 | MEMC_DEGFUN_LCDDIRECT)
#define MEMC_REQSCH_URUN		0x2020001f

#define L2CC_ADD_FILTER_START_URUN	0xffe00001
#define L2CC_ADD_FILTER_END_URUN	0xfff00000

/*****************************************************
 * Read Evaluation Board Configuration
 *****************************************************/
#define HW_CFG_SRST			0x00000001

#define E2P_CMD_EPC_BUSY		0x80000000
#define E2P_CMD_EPC_CMD			0x70000000
#define E2P_CMD_EPC_TIMEOUT 		0x00000200
#define E2P_CMD_MAC_ADDR_LOADED		0x00000100
#define E2P_CMD_EPC_ADDR		0x000000ff

#define E2P_DATA_EEPROM_DATA		0x000000ff

#define E2P_CMD_EPC_CMD_READ		0x00000000
#define E2P_CMD_EPC_CMD_EWDS		0x10000000
#define E2P_CMD_EPC_CMD_EWEN		0x20000000
#define E2P_CMD_EPC_CMD_WRITE		0x30000000
#define E2P_CMD_EPC_CMD_WRAL		0x40000000
#define E2P_CMD_EPC_CMD_ERASE		0x50000000
#define E2P_CMD_EPC_CMD_ERAL		0x60000000
#define E2P_CMD_EPC_CMD_RELOAD  	0x70000000

/* ------------------------------------------------------ */
#define E2P_ADDR_CFGCPU			0x10
#define E2P_ADDR_CFGPWC			0x11
#define E2P_ADDR_CFGIO			0x12

#define E2P_READ_CFGCPU			(E2P_CMD_EPC_BUSY | E2P_CMD_EPC_CMD_READ | E2P_ADDR_CFGCPU)
#define E2P_READ_CFGPWC			(E2P_CMD_EPC_BUSY | E2P_CMD_EPC_CMD_READ | E2P_ADDR_CFGPWC)
#define E2P_READ_CFGIO			(E2P_CMD_EPC_BUSY | E2P_CMD_EPC_CMD_READ | E2P_ADDR_CFGIO)
/* ------------------------------------------------------ */
#define	E2P_CFGCPU_MASK			0x000000ff
#define	E2P_CFGPWC_MASK			0x0000ff00
#define	E2P_CFGIO_MASK			0x00ff0000

#define E2P_CFGCPU_CHIP			0x000000e0	/* bit7-5 */
#define E2P_CFGCPU_CHIP_ES1		0x00000000
#define E2P_CFGCPU_CHIP_ES2		0x00000020

#define E2P_CFGCPU_eMMC			0x00000010	/* bit4   */
#define E2P_CFGCPU_eMMC_SAMSUNG		0x00000000

#define E2P_CFGCPU_DDR			0x00000008	/* bit3   */
#define E2P_CFGCPU_DDR_DDR2		0x00000000
#define E2P_CFGCPU_DDR_LPDDR		0x00000008

#define E2P_CFGCPU_NAND			0x00000004	/* bit2   */
#define E2P_CFGCPU_NAND_EXIST		0x00000000
#define E2P_CFGCPU_NAND_NONE		0x00000004

#define E2P_CFGCPU_RESERVE		0x00000003	/* bit1-0 */
/* ------------------------------------------------------ */
#define E2P_CFGPWC_PWIC			0x0000e000	/* bit7-5 */
#define E2P_CFGPWC_DA9052		0x00000000

#define E2P_CFGPWC_OTP			0x00001c00	/* bit4-2 */
#define E2P_CFGPWC_OTP_DRAFT		0x00000000
#define E2P_CFGPWC_OTP_V1		0x00000400

#define E2P_CFGPWC_RESERVE		0x00000300	/* bit1-0 */
/* ------------------------------------------------------ */
#define E2P_CFGIO_SPDIF			0x00c00000	/* bit7-6 */
#define E2P_CFGIO_SPDIF_CS8427		0x00000000

#define E2P_CFGIO_HDMI			0x00300000	/* bit5-4 */
#define E2P_CFGIO_HDMI_ADV7523		0x00000000

#define E2P_CFGIO_RESERVE		0x000f0000	/* bit3-0 */
/* ------------------------------------------------------ */

#endif /* _LOWLEBEL_INIT_VAL_H */
