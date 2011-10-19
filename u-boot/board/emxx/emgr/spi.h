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

#ifndef __SPI_H
#define __SPI_H

/* SPI Registers */
#define SIO0_BASE		0xe0120000
#define SPI0_BASE		(SIO0_BASE + 0x1000)

#define SP0_MODE		(SPI0_BASE + 0x0000)
#define SP0_POL			(SPI0_BASE + 0x0004)
#define SP0_CONTROL		(SPI0_BASE + 0x0008)
#define SP0_TX_TIM		(SPI0_BASE + 0x000c)
#define SP0_TX_DATA		(SPI0_BASE + 0x0010)
#define SP0_RX_DATA		(SPI0_BASE + 0x0014)
#define SP0_STATUS		(SPI0_BASE + 0x0018)
#define SP0_RAW_STATUS		(SPI0_BASE + 0x001c)
#define SP0_ENSET		(SPI0_BASE + 0x0020)
#define SP0_ENCLR		(SPI0_BASE + 0x0024)
#define SP0_FFCLR		(SPI0_BASE + 0x0028)

#define SP0_CONTROL2		(SPI0_BASE + 0x0034)
#define SP0_TIECS		(SPI0_BASE + 0x0038)

/* SP0_MODE */
#define SP0_MODE_NB		((16-1) << 8)		/* 16bit (add(H8bit) + data(L8bit)) */
#define SP0_MODE_CS		(0 << 4)		/* CS0 */
#define SP0_MODE_M_S		(0 << 1)
#define SP0_MODE_DMA		(0 << 0)
#define SP0_MODE_CS0		(SP0_MODE_NB | SP0_MODE_CS | SP0_MODE_M_S | SP0_MODE_DMA)

/* SP0_POL */
#define SP0_POL_CSW		((8-1) << 12)		/* 8clock */
#define SP0_POL_CS2_POL		(1 << 6)
#define SP0_POL_CS1_POL		(1 << 3)
#define SP0_POL_CK0_DLY		(1 << 2)		/* CK0 add Delay */
#define SP0_POL_CK0_POL		(0 << 1)		/* CK0 positive */
#define SP0_POL_CS0_POL		(0 << 0)		/* CS0 positive */
#define SP0_POL_CS0		(SP0_POL_CSW | SP0_POL_CK0_DLY | SP0_POL_CK0_POL | \
				 SP0_POL_CS0_POL | SP0_POL_CS1_POL |SP0_POL_CS2_POL)

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

/* SP0_TX_DATA */
#define TX_ADDR_SHIFT		8
#define TX_DATA(add, val)	(((unsigned short)(add) << TX_ADDR_SHIFT) | val)


/* PWC definition */
#define PWC_R10_ADD		10
#define PWC_R15_ADD		15
#define PWC_R20_ADD		20
#define PWC_R25_ADD		25
#define PWC_R29_ADD		29
#define PWC_R43_ADD		43
#define PWC_R45_ADD		45
#define PWC_R117_ADD		117
#define PWC_R121_ADD		121

#define PWC_R44_ADD		44
#define PWC_R44_VAL		0xaa	/* BuckCore: Sync mode */
#define PWC_R44_SET_W		(((PWC_R44_ADD << 1) << TX_ADDR_SHIFT) | PWC_R44_VAL)

#define PWC_R46_ADD		46
#define BUCKCORE_130v		0x60	/* BuckCore: 1.30v */
#define BUCKCORE_125v		0x5e	/* BuckCore: 1.25v */
#define BUCKCORE_120v		0x5c	/* BuckCore: 1.20v */
#define BUCKCORE_115v		0x5a	/* BuckCore: 1.15v */
#define BUCKCORE_110v		0x58	/* BuckCore: 1.10v */
#define PWC_R46_VAL		BUCKCORE_110v
#define PWC_R46_SET_W		(((PWC_R46_ADD << 1) << TX_ADDR_SHIFT) | PWC_R46_VAL)

#define PWC_R54_ADD		54
#define PWC_R54_VAL		0x66	/* LDO5: 3.10v */
#define PWC_R54_SET_W		(((PWC_R54_ADD << 1) << TX_ADDR_SHIFT) | PWC_R54_VAL)

#define PWC_R60_ADD		60
#define PWC_R60_VAL		0x01	/* Supply: VB_CORE_GO */
#define PWC_R60_SET_W		(((PWC_R60_ADD << 1) << TX_ADDR_SHIFT) | PWC_R60_VAL)

#define PWC_R62_ADD		62
#define PWC_R62_VAL		0xDF	/* ISET_Buck: 1.3mA */
#define PWC_R62_SET_W		(((PWC_R62_ADD << 1) << TX_ADDR_SHIFT) | PWC_R62_VAL)

#define PWC_R64_ADD		64
#define PWC_R64_VAL		0xF8	/* ISET_DCIN: 1.3mA */
#define PWC_R64_SET_W		(((PWC_R64_ADD << 1) << TX_ADDR_SHIFT) | PWC_R64_VAL)


#endif /* __SPI_H */
