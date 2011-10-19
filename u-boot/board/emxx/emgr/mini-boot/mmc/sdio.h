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

/* register address define */
/* base address */
#define SDIO0_BASE			0xe2900000

#define SDIO_SYSADD			(SDIO0_BASE + 0x0000)
#define SDIO_BLOCK			(SDIO0_BASE + 0x0004)
#define SDIO_ARG			(SDIO0_BASE + 0x0008)
#define SDIO_MODE_CMD			(SDIO0_BASE + 0x000C)
#define SDIO_RSP01			(SDIO0_BASE + 0x0010)
#define SDIO_RSP23			(SDIO0_BASE + 0x0014)
#define SDIO_RSP45			(SDIO0_BASE + 0x0018)
#define SDIO_RSP67			(SDIO0_BASE + 0x001C)
#define SDIO_BUF			(SDIO0_BASE + 0x0020)
#define SDIO_STATE			(SDIO0_BASE + 0x0024)
#define SDIO_HP_BW			(SDIO0_BASE + 0x0028)
#define SDIO_CLK_TOUT_RST		(SDIO0_BASE + 0x002C)
#define SDIO_INT_STS			(SDIO0_BASE + 0x0030)
#define SDIO_INT_STSEN			(SDIO0_BASE + 0x0034)
#define SDIO_INT_SIGEN			(SDIO0_BASE + 0x0038)
#define SDIO_CMD12_ERR			(SDIO0_BASE + 0x003C)
#define SDIO_CONF			(SDIO0_BASE + 0x0040)
#define SDIO_MAXREG			(SDIO0_BASE + 0x0048)
#define SDIO_ERRINT_FORCE		(SDIO0_BASE + 0x0050)
#define SDIO_ADMA_ERR			(SDIO0_BASE + 0x0054)
#define SDIO_ADMA_SYSADD		(SDIO0_BASE + 0x0058)
#define SDIO_CEATA			(SDIO0_BASE + 0x0080)
#define SDIO_SLOTINT_STS		(SDIO0_BASE + 0x00FC)
#define SDIO_AMBA0			(SDIO0_BASE + 0x0100)
#define SDIO_AMBA1			(SDIO0_BASE + 0x0104)

#define SDIO_DELAY			(SDIO0_BASE + 0xE000)
#define SDIO_GIO0			(SDIO0_BASE + 0xE004)
#define SDIO_MODEN			(SDIO0_BASE + 0xF000)

/* register set value define */
/* MODE(16bit) + CMD(16bit) */
#define SDIO_MODE_MULTI			(1 << 5)
#define SDIO_MODE_READ			(1 << 4)
#define SDIO_MODE_ACMD12		(1 << 2)
#define SDIO_MODE_BLK_COUNT_EN		(1 << 1)
#define SDIO_MODE_DMA_EN		(1 << 0)

#define SDIO_CMD_INDEX(x)		((x & 0x3f) << 24)
#define SDIO_CMD_DATA			(1 << 21)
#define SDIO_CMD_INDEX_CHK		(1 << 20)
#define SDIO_CMD_CRC_CHK		(1 << 19)
#define SDIO_CMD_RESP_NONE		(0 << 16)
#define SDIO_CMD_RESP_136		(1 << 16)
#define SDIO_CMD_RESP_48		(2 << 16)
#define SDIO_CMD_RESP_48B		(3 << 16)

/* STATE(32bit) */
#define SDIO_STATE_DAT0			(1 << 20)
#define SDIO_STATE_WP			(1 << 19)
#define SDIO_STATE_CD			(1 << 18)
#define SDIO_STATE_STABLE		(1 << 17)
#define SDIO_STATE_INSERT		(1 << 16)
#define SDIO_STATE_RDEN			(1 << 11)
#define SDIO_STATE_WREN			(1 << 10)
#define SDIO_STATE_RD_ACTIVE		(1 << 9)
#define SDIO_STATE_WR_ACTIVE		(1 << 8)
#define SDIO_STATE_DAT_ACTIVE		(1 << 2)
#define SDIO_STATE_DAT_INHIBIT		(1 << 1)
#define SDIO_STATE_CMD_INHIBIT		(1 << 0)

/* HOST(8bit) + POWER(8bit) + WAKEUP(16bit) */
#define SDIO_HOST_CDSEL			(1 << 7)
#define SDIO_HOST_CDTL			(1 << 6)
#define SDIO_HOST_MMC8B			(1 << 5)
#define SDIO_HOST_HS			(1 << 2)
#define SDIO_HOST_WIDTH			(1 << 1)
#define SDIO_HOST_LED			(1 << 0)

#define SDIO_POWER_VOLT_18		(5 << 9)
#define SDIO_POWER_VOLT_30		(6 << 9)
#define SDIO_POWER_VOLT_33		(7 << 9)
#define SDIO_POWER_POWER		(1 << 8)

/* CLKCTRL(16bit) + TIMEOUT(8bit) + SOFTRST(8bit) */
#define SDIO_CLK_CLKDIV1		(0x00 << 8)
#define SDIO_CLK_CLKDIV2		(0x01 << 8)
#define SDIO_CLK_CLKDIV4		(0x02 << 8)
#define SDIO_CLK_CLKDIV8		(0x04 << 8)
#define SDIO_CLK_CLKDIV16		(0x08 << 8)
#define SDIO_CLK_CLKDIV32		(0x10 << 8)
#define SDIO_CLK_CLKDIV64		(0x20 << 8)
#define SDIO_CLK_CLKDIV128		(0x40 << 8)
#define SDIO_CLK_CLKDIV256		(0x80 << 8)
#define SDIO_CLK_SDCLKEN		(1 << 2)
#define SDIO_CLK_CLKSTA			(1 << 1)
#define SDIO_CLK_CLKEN			(1 << 0)
#define SDIO_CLK_MASK			(0xffff << 0)

#define SDIO_TIMEOUT_COUNT_MIN		(0 << 16)
#define SDIO_TIMEOUT_COUNT_MAX		(0xE << 16)

#define SDIO_SOFTRST_DATA		(1 << 26)
#define SDIO_SOFTRST_CMD		(1 << 25)
#define SDIO_SOFTRST_ALL		(1 << 24)

/* INTERRUPTS(32bit) */
#define SDIO_INT_CMD12_ERR		(1 << 24)
#define SDIO_INT_DATA_END		(1 << 22)
#define SDIO_INT_DATA_CRC		(1 << 21)
#define SDIO_INT_DATA_TOUT		(1 << 20)
#define SDIO_INT_CMD_INDEX		(1 << 19)
#define SDIO_INT_CMD_END		(1 << 18)
#define SDIO_INT_CMD_CRC		(1 << 17)
#define SDIO_INT_CMD_TOUT		(1 << 16)
#define SDIO_INT_ERR			(1 << 15)
#define SDIO_INT_SDIO_INT		(1 << 8)
#define SDIO_INT_CARD_REM		(1 << 7)
#define SDIO_INT_CARD_INS		(1 << 6)
#define SDIO_INT_RREADY			(1 << 5)
#define SDIO_INT_WREADY			(1 << 4)
#define SDIO_INT_DMA			(1 << 3)
#define SDIO_INT_BGE			(1 << 2)
#define SDIO_INT_TRANCOMP		(1 << 1)
#define SDIO_INT_CMDCOMP		(1 << 0)

#define SDIO_INT_ALLERR			(0x017f8000)
#define SDIO_INT_MASK	\
	(SDIO_INT_ALLERR  | SDIO_INT_CMDCOMP | SDIO_INT_TRANCOMP \
	| SDIO_INT_RREADY | SDIO_INT_WREADY)

/* AMBA0(32bit) */
#define SDIO_AMBA0_TMODE_INCR4		(0x00 << 0)
#define SDIO_AMBA0_TMODE_INCR8		(0x01 << 0)
#define SDIO_AMBA0_TMODE_INCR16		(0x02 << 0)
#define SDIO_AMBA0_TMODE_SINGLE		(0x04 << 0)

/* DELAY(32bit) */
#define SDIO_DELAY_REVERSE		(0x01 << 4)
#define SDIO_DELAY_0_0ns		(0x00 << 0)
#define SDIO_DELAY_0_5ns		(0x01 << 0)
#define SDIO_DELAY_1_0ns		(0x02 << 0)
#define SDIO_DELAY_1_5ns		(0x03 << 0)
#define SDIO_DELAY_2_0ns		(0x04 << 0)
#define SDIO_DELAY_2_5ns		(0x05 << 0)
#define SDIO_DELAY_3_0ns		(0x06 << 0)
#define SDIO_DELAY_3_5ns		(0x07 << 0)
#define SDIO_DELAY_MASK			(0x07 << 0)

/* MODEN(32bit) */
#define SDIO_MODEN_ENABLE		(0x01 << 0)

/* GIO0(32bit) */
#define SDIO_GIO0_DETECT		(0x01 << 15)

