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

#ifndef _EM1_SYS_H_
#define _EM1_SYS_H_

#define SRAM_START		0xa0000000
#define SRAM_END		0xa0020000
#define SRAM_STACK_BASE		(SRAM_END - 0x104)

#undef TCLR
#undef TLDR
#undef TCRR
#define TCLR			0x00	/* Timer OP  offset */
#define TLDR			0x08	/* Timer SET offset */
#define TCRR			0x0c	/* Timer RCR offset */

/* DIVSP0SCLK */
#define SMU_BASE		(0xc0110000)
#define SMU_DIVSP0SCLK		(SMU_BASE + 0x118)
#define SMU_DIVSP0SCLK_VAL	0x00000091 /* PLL3/20:11.9MHz */

/* GCLKCTRL3 */
#define SMU_GLCKCTRL3		(SMU_BASE + 0x1cc)
#define SMU_GLCKCTRL3_ENA	(SMU_BASE + 0x1d0)
#define SMU_GLCKCTRL3_VAL	0x00600000 /* SPI0 (SCLK PCLK) */
#define SMU_GLCKCTRL3ENA_VAL	0x00600000 /* SPI0 (SCLK PCLK) */

/* RESETREQ1 */
#define SMU_RESETREQ1		(SMU_BASE + 0x00c)
#define SMU_RESETREQ1ENA	(SMU_BASE + 0x010)
#define SMU_RESETREQ1_VAL	0x00400000 /* SPI0 */
#define SMU_RESETREQ1ENA_VAL	0x00400000 /* SPI0 */

/* SPI Register Addr */
#define SP0_BASE		(0xc0120000)
#define SPX_MODE		(SP0_BASE + 0x000)
#define SPX_POL			(SP0_BASE + 0x004)
#define SPX_CONTROL		(SP0_BASE + 0x008)
#define SPX_TX_DATA		(SP0_BASE + 0x010)

/* SPI Val */
#define SPI_MODE_SEND		0x00000f00 /* CS BIT */
#define SPI_POL_INIT		0x00007004 /* CSW DLY POL */
#ifdef CONFIG_EM1_BOARD_DKIT
#define SPI_SEND_DATA		0x00001eac /* RESET (ADDR(R15):0x1e DATA:0xac) */
#else
#define SPI_SEND_DATA		0x00006601 /* RESET (ADDR:0x66 DATA:0x1) */
#endif
#define SPI_TX_START		0x00000009 /* Transfer Start */

#endif /* _EM1_SYS_H_ */
