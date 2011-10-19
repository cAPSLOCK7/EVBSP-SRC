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

#include <config.h>

#if defined(CONFIG_EMXX_EMMCBOOT) || defined(CONFIG_EMXX_ESDBOOT)

#include "../include/mini_config.h"
#include "../include/mmc.h"
#include "sdio.h"


static int sdio_send_cmd(int cmd, int arg, int type, int *resp);
static int sdio_send_extcsd(unsigned char *buf);
static int sdio_single_read(int addr, unsigned char *buf);
static int sdio_multi_read(int addr, unsigned char *buf, int num);
static int sdio_single_write(int addr, unsigned char *buf);
static int sdio_multi_write(int addr, unsigned char *buf, int num);
static void sdio_hw_init(void);
static void sdio_set_blklength(unsigned int length);
static void sdio_set_buswidth(unsigned int width);
static void sdio_set_clock(unsigned int clock);

const hw_if_st sdio_if = {
	.send_cmd	= sdio_send_cmd,
	.send_acmd	= sdio_send_cmd,
	.send_extcsd	= sdio_send_extcsd,
	.single_read	= sdio_single_read,
	.multi_read	= sdio_multi_read,
	.single_write	= sdio_single_write,
	.multi_write	= sdio_multi_write,
	.hw_init	= sdio_hw_init,
	.set_blklength	= sdio_set_blklength,
	.set_buswidth	= sdio_set_buswidth,
	.set_clock	= sdio_set_clock,
};


static volatile int *acc_mode=(volatile int *)ACC_MODE_ADDR;


static void
sdio_write(unsigned int val, unsigned int addr)
{
	outl(val, addr);
}

static unsigned int
sdio_read(unsigned int addr)
{
	return inl(addr);
}

static void
sdio_reset(unsigned int mask)
{
	unsigned int val;

	val = sdio_read(SDIO_CLK_TOUT_RST);
	sdio_write(val | mask, SDIO_CLK_TOUT_RST);

	while (sdio_read(SDIO_CLK_TOUT_RST) & mask) {
	}
}

static int
sdio_wait_response(void)
{
	int ret;
	unsigned int int_status;

	while (!(sdio_read(SDIO_INT_STS) & SDIO_INT_CMDCOMP)) {
		if (sdio_read(SDIO_INT_STS) & SDIO_INT_ALLERR) {
			break;
		}
	}
	int_status = sdio_read(SDIO_INT_STS);
	ret = int_status & SDIO_INT_ALLERR;
	if (ret != 0) {
		sdio_reset(SDIO_SOFTRST_CMD | SDIO_SOFTRST_DATA);
		if (ret & SDIO_INT_CMD_TOUT) {
			ret = MMC_RES_TIMEOUT;
		}
		return ret;
	}

	if (sdio_read(SDIO_MODE_CMD) & SDIO_CMD_DATA) {
		sdio_write(SDIO_INT_CMDCOMP, SDIO_INT_STS);
	} else {
		sdio_write(int_status, SDIO_INT_STS);
	}
	return 0;
}

static int 
sdio_send_cmd(int cmd, int arg, int type, int *resp)
{
	int ret;
	unsigned int val;

	sdio_write(0xffffffff, SDIO_INT_STS);	// Clear ALL

	sdio_write(arg, SDIO_ARG);

	val = SDIO_CMD_INDEX(cmd);

	switch (type) {
	case MMC_RSP_R1:
	case MMC_RSP_R5:
	case MMC_RSP_R6:
	case MMC_RSP_R7:
	case MMC_RSP_R1B:
		val |= SDIO_CMD_CRC_CHK | SDIO_CMD_INDEX_CHK | SDIO_CMD_RESP_48;
		break;
	case MMC_RSP_R2:
		val |= SDIO_CMD_CRC_CHK | SDIO_CMD_RESP_136;
		break;
	case MMC_RSP_R3:
	case MMC_RSP_R4:
		val |= SDIO_CMD_RESP_48;
		break;
	default:
		break;
	}

	switch (cmd) {
#if defined(CONFIG_EMXX_EMMCBOOT)
	case 8:
#endif
	case 17:
		val |= SDIO_CMD_DATA | SDIO_MODE_READ;
		break;
	case 18:
		val |= SDIO_CMD_DATA | SDIO_MODE_READ;
		val |= SDIO_MODE_ACMD12 | SDIO_MODE_MULTI | SDIO_MODE_BLK_COUNT_EN;
		break;
	case 24:
		val |= SDIO_CMD_DATA;
		break;
	case 25:
		val |= SDIO_CMD_DATA;
		val |= SDIO_MODE_ACMD12 | SDIO_MODE_MULTI | SDIO_MODE_BLK_COUNT_EN;
		break;
	default:
		break;
	}

	sdio_write(val, SDIO_MODE_CMD);

	ret = sdio_wait_response();
	if (ret == 0) {
		if ((type == MMC_RSP_R1B) || (cmd == 7)) {
			// wait DATA0 == 1
			while (!(sdio_read(SDIO_STATE) & SDIO_STATE_DAT0)) {
			}
		}
		if (resp != NULL) {
			*resp++ = sdio_read(SDIO_RSP01);
			*resp++ = sdio_read(SDIO_RSP23);
			*resp++ = sdio_read(SDIO_RSP45);
			*resp = sdio_read(SDIO_RSP67);
		}
	}
	return ret;
}

static int
sdio_send_extcsd(unsigned char *buf)
{
	int i;
	int ret;
	unsigned int int_status;
	unsigned int *tmp_buf = (unsigned int *)buf;

	/* CMD8 : SEND_EXT_CSD */
	ret = sdio_send_cmd(8, 0, MMC_RSP_R1, NULL);
	if (ret != 0) {
		return ret;
	}

	while (1) {
		/* wait read enable */
		int_status = sdio_read(SDIO_INT_STS);
		if (int_status & (SDIO_INT_RREADY | SDIO_INT_ALLERR)) {
			break;
		}
	}
	sdio_write(SDIO_INT_RREADY, SDIO_INT_STS);
	if (int_status & SDIO_INT_ALLERR) {
		sdio_reset(SDIO_SOFTRST_CMD | SDIO_SOFTRST_DATA);
		return -1;
	}

	/* 1block read */
	for (i = 0; i < MMC_BLOCKLEN_VAL / 4; i++) {
		*tmp_buf++ = sdio_read(SDIO_BUF);
	}

	/* wait for data read end */
	int_status = sdio_read(SDIO_INT_STS);
	while (!(int_status & SDIO_INT_TRANCOMP)) {
		int_status = sdio_read(SDIO_INT_STS);
		if (int_status & SDIO_INT_ALLERR){
			sdio_reset(SDIO_SOFTRST_CMD | SDIO_SOFTRST_DATA);
			return int_status;
		}
	}
	sdio_write(int_status, SDIO_INT_STS);
	return 0;
}

static int
sdio_single_read(int addr, unsigned char *buf)
{
	int i;
	int ret;
	unsigned int address, int_status;
	unsigned int *tmp_buf = (unsigned int *)buf;

	if (*acc_mode) {
		address = addr;
	} else {
		address = addr * MMC_BLOCKLEN_VAL;
	}

	/* CMD17 : READ_SINGLE_BLOCK */
	ret = sdio_send_cmd(17, address, MMC_RSP_R1, NULL);
	if (ret != 0){
		return ret;
	}
	while (1) {
		/* wait read enable */
		int_status = sdio_read(SDIO_INT_STS);
		if (int_status & (SDIO_INT_RREADY | SDIO_INT_ALLERR)) {
			break;
		}
	}
	sdio_write(SDIO_INT_RREADY, SDIO_INT_STS);
	if (int_status & SDIO_INT_ALLERR) {
		sdio_reset(SDIO_SOFTRST_CMD | SDIO_SOFTRST_DATA);
		return -1;
	}

	/* 1block read */
	for (i = 0; i < MMC_BLOCKLEN_VAL / 4; i++) {
		*tmp_buf++ = sdio_read(SDIO_BUF);
	}

	/* wait for data read end */
	int_status = sdio_read(SDIO_INT_STS);
	while (!(int_status & SDIO_INT_TRANCOMP)) {
		int_status = sdio_read(SDIO_INT_STS);
		if (int_status & SDIO_INT_ALLERR){
			sdio_reset(SDIO_SOFTRST_CMD | SDIO_SOFTRST_DATA);
			return int_status;
		}
	}
	sdio_write(int_status, SDIO_INT_STS);
	return 0;
}

static int
sdio_multi_read(int addr, unsigned char *buf, int num)
{
	unsigned int address, int_status, val;
	int ret, i, j;
	unsigned int *tmp_buf = (unsigned int *)buf;

	val = sdio_read(SDIO_BLOCK) & 0xffff;
	sdio_write(val | (num << 16), SDIO_BLOCK);

	if (*acc_mode) {
		address = addr;
	} else {
		address = addr * MMC_BLOCKLEN_VAL;
	}

	/* CMD18 : READ_MULTI_BLOCK */
	ret = sdio_send_cmd(18, address, MMC_RSP_R1, NULL);
	if (ret != 0) {
		return ret;
	}

	while (1) {
		/* wait read enable */
		int_status = sdio_read(SDIO_INT_STS);
		if (int_status & (SDIO_INT_RREADY | SDIO_INT_ALLERR)) {
			break;
		}
	}
	sdio_write(SDIO_INT_RREADY, SDIO_INT_STS);
	if (int_status & SDIO_INT_ALLERR) {
		sdio_reset(SDIO_SOFTRST_CMD | SDIO_SOFTRST_DATA);
		return int_status;
	}

	for (i = 0 ; i < num; i++) {
		while ((sdio_read(SDIO_STATE) & SDIO_STATE_RDEN) == 0) {
			int_status = sdio_read(SDIO_INT_STS);
			if (int_status & (SDIO_INT_ALLERR)) {
				sdio_reset(SDIO_SOFTRST_CMD | SDIO_SOFTRST_DATA);
				return int_status;
			}
		}
		for (j = 0; j < MMC_BLOCKLEN_VAL / 4; j++) {
			*tmp_buf++ = sdio_read(SDIO_BUF);
		}
	}

	/* wait for data read end */
	int_status = sdio_read(SDIO_INT_STS);
	while (!(int_status & SDIO_INT_TRANCOMP)) {
		int_status = sdio_read(SDIO_INT_STS);
		if (int_status & SDIO_INT_ALLERR){
			sdio_reset(SDIO_SOFTRST_CMD | SDIO_SOFTRST_DATA);
			return int_status;
		}
	}
	sdio_write(int_status, SDIO_INT_STS);
	return 0;
}

static int
sdio_single_write(int addr, unsigned char *buf)
{
	int i;
	int ret;
	unsigned int address, int_status;
	unsigned int *tmp_buf = (unsigned int *)buf;

	if (*acc_mode) {
		address = addr;
	} else {
		address = addr * MMC_BLOCKLEN_VAL;
	}

	/* CMD24 : WRITE_SINGLE_BLOCK */
	ret = sdio_send_cmd(24, address, MMC_RSP_R1, NULL);
	if (ret != 0){
		return ret;
	}
	while (1) {
		/* wait write enable */
		int_status = sdio_read(SDIO_INT_STS);
		if (int_status & (SDIO_INT_WREADY | SDIO_INT_ALLERR)) {
			break;
		}
	}
	sdio_write(SDIO_INT_WREADY, SDIO_INT_STS);
	if (int_status & SDIO_INT_ALLERR) {
		sdio_reset(SDIO_SOFTRST_CMD | SDIO_SOFTRST_DATA);
		return -1;
	}

	if (buf != NULL) {
		/* 1block write */
		for (i = 0; i < MMC_BLOCKLEN_VAL / 4; i++) {
			sdio_write(*tmp_buf++, SDIO_BUF);
		}
	} else {
		/* 1block clear */
		for (i = 0; i < MMC_BLOCKLEN_VAL / 4; i++) {
			sdio_write(0, SDIO_BUF);
		}
	}

	/* wait for data write end */
	int_status = sdio_read(SDIO_INT_STS);
	while (!(int_status & SDIO_INT_TRANCOMP)) {
		int_status = sdio_read(SDIO_INT_STS);
		if (int_status & SDIO_INT_ALLERR){
			sdio_reset(SDIO_SOFTRST_CMD | SDIO_SOFTRST_DATA);
			return -1;
		}
	}
	sdio_write(int_status, SDIO_INT_STS);
	return 0;
}

static int
sdio_multi_write(int addr, unsigned char *buf, int num)
{
	unsigned int address, int_status, val;
	int ret, i, j;
	unsigned int *tmp_buf = (unsigned int *)buf;

	val = sdio_read(SDIO_BLOCK) & 0xffff;
	sdio_write(val | (num << 16), SDIO_BLOCK);

	if (*acc_mode) {
		address = addr;
	} else {
		address = addr * MMC_BLOCKLEN_VAL;
	}

	/* CMD25 : WRITE_MULTI_BLOCK */
	ret = sdio_send_cmd(25, address, MMC_RSP_R1, NULL);
	if (ret != 0) {
		return ret;
	}

	while (1) {
		/* wait write enable */
		int_status = sdio_read(SDIO_INT_STS);
		if (int_status & (SDIO_INT_WREADY | SDIO_INT_ALLERR)) {
			break;
		}
	}
	sdio_write(SDIO_INT_WREADY, SDIO_INT_STS);
	if (int_status & SDIO_INT_ALLERR) {
		sdio_reset(SDIO_SOFTRST_CMD | SDIO_SOFTRST_DATA);
		return -1;
	}

	for (i = 0 ; i < num; i++) {
		while ((sdio_read(SDIO_STATE) & SDIO_STATE_WREN) == 0) {
			int_status = sdio_read(SDIO_INT_STS);
			if (int_status & (SDIO_INT_ALLERR)) {
				sdio_reset(SDIO_SOFTRST_CMD | SDIO_SOFTRST_DATA);
				return -1;
			}
		}
		if (buf != NULL) {
			/* 1block write */
			for (j = 0; j < MMC_BLOCKLEN_VAL / 4; j++) {
				sdio_write(*tmp_buf++, SDIO_BUF);
			}
		} else {
			/* 1block clear */
			for (j = 0; j < MMC_BLOCKLEN_VAL / 4; j++) {
				sdio_write(*tmp_buf++, SDIO_BUF);
			}
		}
	}

	/* wait for data write end */
	int_status = sdio_read(SDIO_INT_STS);
	while (!(int_status & SDIO_INT_TRANCOMP)) {
		int_status = sdio_read(SDIO_INT_STS);
		if (int_status & SDIO_INT_ALLERR){
			sdio_reset(SDIO_SOFTRST_CMD | SDIO_SOFTRST_DATA);
			return -1;
		}
	}
	sdio_write(int_status, SDIO_INT_STS);
	return 0;
}

static void
sdio_hw_init(void)
{
#if defined(EMXX_MINIBOOT)
	sdio_write(SDIO_INT_MASK, SDIO_INT_STSEN);
	sdio_write(SDIO_AMBA0_TMODE_SINGLE, SDIO_AMBA0);
#endif
#if 0
	unsigned int val;

	/* initialize SDIO */
	sdio_write(SDIO_MODEN_ENABLE, SDIO_MODEN);
	sdio_write(SDIO_DELAY_REVERSE, SDIO_DELAY);

	val = sdio_read(SDIO_GIO0);
	sdio_write((val & ~SDIO_GIO0_DETECT), SDIO_GIO0);

	udelay(1500);

	sdio_reset(SDIO_SOFTRST_ALL);

	val = (SDIO_POWER_VOLT_30 | SDIO_POWER_POWER);
	sdio_write(val, SDIO_HP_BW);

	sdio_write(SDIO_INT_MASK, SDIO_INT_STSEN);

	val = SDIO_CLK_CLKDIV64 | SDIO_CLK_CLKEN | SDIO_TIMEOUT_COUNT_MAX;
	sdio_write(val, SDIO_CLK_TOUT_RST);
	while (!((val = sdio_read(SDIO_CLK_TOUT_RST)) & SDIO_CLK_CLKSTA)) {
	}
	val |= SDIO_CLK_SDCLKEN;
	sdio_write(val, SDIO_CLK_TOUT_RST);

	udelay(1000);
#endif
}

static void
sdio_set_blklength(unsigned int length)
{
	sdio_write(length | (7 << 12), SDIO_BLOCK);
}

static void
sdio_set_buswidth(unsigned int width)
{
	unsigned int val;

	val = sdio_read(SDIO_HP_BW);
	if (width == 1) {
		sdio_write(val & ~SDIO_HOST_WIDTH, SDIO_HP_BW);
	} else {
		sdio_write(val | SDIO_HOST_WIDTH, SDIO_HP_BW);
	}
}

static void
sdio_set_clock(unsigned int clock)
{
	unsigned int val;
	unsigned int val2;

	val = sdio_read(SDIO_CLK_TOUT_RST);
	val &= ~SDIO_CLK_MASK;
	sdio_write(val, SDIO_CLK_TOUT_RST);	// Stop clock

	if (clock) {
		if (clock == MMC_CLK_HIGH) {
			val |= SDIO_CLK_CLKDIV1 | SDIO_CLK_CLKEN;
			val2 = sdio_read(SDIO_GIO0) | 0x80000000;
			sdio_write(val2, SDIO_GIO0);
			val2 = sdio_read(SDIO_HP_BW) | SDIO_HOST_HS;
			sdio_write(val2, SDIO_HP_BW);
		} else {
			val |= SDIO_CLK_CLKDIV2 | SDIO_CLK_CLKEN;
		}
		sdio_write(val, SDIO_CLK_TOUT_RST);
		while (!((val = sdio_read(SDIO_CLK_TOUT_RST)) & SDIO_CLK_CLKSTA)) {
		}
		val |= SDIO_CLK_SDCLKEN;
		sdio_write(val, SDIO_CLK_TOUT_RST);
	}
}

#endif	/* defined(CONFIG_EMXX_EMMCBOOT) */

