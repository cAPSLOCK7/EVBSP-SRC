/*
 *
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


#ifndef __SPI_H
#define __SPI_H

/* SPI Registers */
#define EM1_SPI0_BASE		0xc0120000

#define SP0_MODE		(EM1_SPI0_BASE + 0x0000)
#define SP0_POL			(EM1_SPI0_BASE + 0x0004)
#define SP0_CONTROL		(EM1_SPI0_BASE + 0x0008)
#define SP0_TX_TIM		(EM1_SPI0_BASE + 0x000c)
#define SP0_TX_DATA		(EM1_SPI0_BASE + 0x0010)
#define SP0_RX_DATA		(EM1_SPI0_BASE + 0x0014)
#define SP0_STATUS		(EM1_SPI0_BASE + 0x0018)
#define SP0_RAW_STATUS		(EM1_SPI0_BASE + 0x001c)
#define SP0_ENSET		(EM1_SPI0_BASE + 0x0020)
#define SP0_ENCLR		(EM1_SPI0_BASE + 0x0024)
#define SP0_FFCLR		(EM1_SPI0_BASE + 0x0028)

#define SP0_CONTROL2		(EM1_SPI0_BASE + 0x0034)
#define SP0_TIECS		(EM1_SPI0_BASE + 0x0038)

/* SP0_MODE */
#define SP0_MODE_NB		((16-1) << 8)	/* 16bit (add(H8bit) + data(L8bit)) */
#define SP0_MODE_CS		(0 << 4)		/* CS0 */
#define SP0_MODE_M_S		(0 << 1)
#define SP0_MODE_DMA		(0 << 0)
#define SP0_MODE_CS0		(SP0_MODE_NB | SP0_MODE_CS | SP0_MODE_M_S | SP0_MODE_DMA)

/* SP0_POL */
#define SP0_POL_CS5_POL		(1 << 19)
#define SP0_POL_CS4_POL		(1 << 16)
#define SP0_POL_CSW		((8-1) << 12)	/* 8clock */
#define SP0_POL_CS3_POL		(1 << 9)
#define SP0_POL_CS2_POL		(1 << 6)
#define SP0_POL_CS1_POL		(1 << 3)
#define SP0_POL_CK0_DLY		(1 << 2)		/* CK0 add Delay */
#define SP0_POL_CK0_POL		(0 << 1)		/* CK0 positive */
#define SP0_POL_CS0_POL		(0 << 0)		/* CS0 positive */
#define SP0_POL_CS0		(SP0_POL_CSW | SP0_POL_CK0_DLY | SP0_POL_CK0_POL | SP0_POL_CS0_POL | \
	SP0_POL_CS1_POL | SP0_POL_CS2_POL | SP0_POL_CS3_POL | SP0_POL_CS4_POL | SP0_POL_CS5_POL)

/* SP0_CONTROL */
#define SP0_CTL_TX_EMP		(1 << 15)		/* TX FIFO Empty */
#define SP0_CTL_RX_FULL		(1 << 15)		/* RX FIFO Full */
#define SP0_CTL_RST		(1 << 8)		/* set Reset */
#define SP0_CTL_RST_FREE	(0 << 8)		/* free Reset */
#define SP0_CTL_RX_EMP		(1 << 6)		/* RX Empty */
#define SP0_CTL_WRT_ENA		(1 << 3)		/* TX enable */
#define SP0_CTL_WRT_DIS		(0 << 3)		/* TX disable */
#define SP0_CTL_RD_ENA		(1 << 2)		/* RD enable */
#define SP0_CTL_RD_DIS		(0 << 2)		/* RD disable */
#define SP0_CTL_START		(1 << 0)		/* TX/RX start */

#define SP0_CTL_CHK_EMP		(SP0_CTL_TX_EMP | SP0_CTL_RX_EMP)

/* EM1_SP0_TX_DATA */
#define TX_ADDR_SHIFT		8
#define TX_DATA(add, val)	(((unsigned short)(add) << TX_ADDR_SHIFT) | val)

/* PowerIC register */
#define PSCNT2			0x06

#define PSCNT2_GP3_ON		0x02
#define PSCNT2_GP4_ON		0x04

#define PWC_R1_ADD		1
/*#define PWC_R1_CHECK_BIT	0x98 */
#define PWC_R1_CHECK_BIT	0x18
#define PWC_R1_SUPPLY_VBUS	0x10
#define PWC_R1_SET_R		(((PWC_R1_ADD << 1) + 1) << TX_ADDR_SHIFT)

#define PWC_R54_ADD		54
#define PWC_R54_VAL		0x66	/* LDO5: 3.10v */
#define PWC_R54_SET_W		(((PWC_R54_ADD << 1) << TX_ADDR_SHIFT) | PWC_R54_VAL)

#define PWC_R28_ADD		28
#define PWC_R28_LED0		0xa2	/* LED0:ON */
#define PWC_R28_LED1		0x22	/* LED0-1:ON */
#define PWC_R28_SET_0		(((PWC_R28_ADD << 1) << TX_ADDR_SHIFT) | PWC_R28_LED0)
#define PWC_R28_SET_1		(((PWC_R28_ADD << 1) << TX_ADDR_SHIFT) | PWC_R28_LED1)

#define PWC_R62_ADD		62

#define PWC_R81_ADD		81
#define PWC_R81_VAL		0x13	/* VBAT pin (channel 3) selected */
#define PWC_R81_SET_W		(((PWC_R81_ADD << 1) << TX_ADDR_SHIFT) | PWC_R81_VAL)
#define PWC_R81_SET_R		(((PWC_R81_ADD << 1) + 1) << TX_ADDR_SHIFT)

#define PWC_R83_ADD		83
#define PWC_R83_SET_R		(((PWC_R83_ADD << 1) + 1) << TX_ADDR_SHIFT)

#define PWC_R84_ADD		84
#define PWC_R84_SET_R		(((PWC_R84_ADD << 1) + 1) << TX_ADDR_SHIFT)

#endif /* __SPI_H */
