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


/* base address */
#if defined(CONFIG_EMXX_SDBOOT)
#define	SDC_BASE			0x50050000
#else
#define	SDC_BASE			0x50090000
#endif

/* register address */
#define SDM_CMD				(SDC_BASE + 0x0000)
#define SDM_PORTSEL			(SDC_BASE + 0x0004)
#define SDM_ARG0			(SDC_BASE + 0x0008)
#define SDM_ARG1			(SDC_BASE + 0x000c)
#define SDM_STOP			(SDC_BASE + 0x0010)
#define SDM_SECCNT			(SDC_BASE + 0x0014)
#define SDM_RSP0			(SDC_BASE + 0x0018)
#define SDM_RSP1			(SDC_BASE + 0x001c)
#define SDM_RSP2			(SDC_BASE + 0x0020)
#define SDM_RSP3			(SDC_BASE + 0x0024)
#define SDM_RSP4			(SDC_BASE + 0x0028)
#define SDM_RSP5			(SDC_BASE + 0x002c)
#define SDM_RSP6			(SDC_BASE + 0x0030)
#define SDM_RSP7			(SDC_BASE + 0x0034)

#define SDM_INFO1			(SDC_BASE + 0x0038)
#define SDM_INFO2			(SDC_BASE + 0x003c)
#define SDM_INFO1_MASK			(SDC_BASE + 0x0040)
#define SDM_INFO2_MASK			(SDC_BASE + 0x0044)
#define SDM_CLK_CTRL			(SDC_BASE + 0x0048)
#define SDM_SIZE			(SDC_BASE + 0x004C)

#define SDM_OPTION			(SDC_BASE + 0x0050)

#define SDM_BUF0			(SDC_BASE + 0x0060)

#define SDM_SOFT_RST			(SDC_BASE + 0x01c0)

/* register set value define */
#define SDM_SOFT_RST_MDL		0x00000000	/* module reset */
#define SDM_SOFT_RST_SFT		0x00000007	/* release reset */
#define SDM_OPTION_VAL			0x000080ee	/* response time out counter */
#define SDM_INFO_CLEAR			0x00000000	/* interrupt status clear */
#define SDM_INFO_MASK_ALL		0x0000ffff	/* interrupt mask */
#define SDM_INFO_MASK_FREE		0x00000000	/* interrupt enable */
#define SDM_CLK_CTRL_DIV256		0x00000140	/* enable SD clock(LBUS(=57.344) * 1/256) ※no control */
#define SDM_CLK_CTRL_DIV8		0x00000302	/* enable SD clock(LBUS(=57.344) * 1/  8) ※auto control */
#define SDM_CLK_CTRL_DIV4		0x00000301	/* enable SD clock(LBUS(=57.344) * 1/  4) ※auto control */
#define SDM_CLK_CTRL_DIV2		0x00000700	/* enable SD clock(LBUS(=83.3) * 1/  2) ※auto control */
#define SDM_INFO1_MASK_VAL		0x0000fffa	/* Response end,R/W access all end Enable */
#define SDM_INFO2_MASK_VAL		0x0000807F	/*  */

#define SDM_INFO1_RES_END		0x0001		/* RES END BIT */
#define SDM_INFO1_RES_RW_END		0x0004		/* RES Read/Write END BIT */

#define SDM_INFO2_ERR_ALL		0x807F		/* INFO2 ERR */
#define SDM_INFO2_BWE_BIT		0x0200		/* INFO2 BWE BIT */
#define SDM_INFO2_BRE_BIT		0x0100		/* INFO2 BRE BIT */
#define SDM_INFO2_DATA0_BIT		0x0080		/* INFO2 DATA0 BIT */
#define SDM_INFO2_RES_TOUT		0x0040		/* INFO2 ERR6 BIT */

#define SDM_OPT_WIDTH_BIT		0x8000

#define SDM_STOP_SEC_BIT		0x0100

