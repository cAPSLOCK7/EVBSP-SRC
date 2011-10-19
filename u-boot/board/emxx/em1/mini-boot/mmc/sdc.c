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

#include <config.h>

#if defined(CONFIG_EMXX_SDBOOT) || defined(CONFIG_EMXX_EMMCBOOT)

#include "../include/mini_config.h"
#include "../include/mmc.h"
#include "sdc.h"
#include "mmc.h"

static int sdc_send_cmd(int cmd, int arg, int type, int *resp);
static int sdc_send_acmd(int cmd, int arg, int type, int *resp);
static int sdc_send_extcsd(unsigned char *buf);
static int sdc_single_read(int addr, unsigned char *buf);
static int sdc_multi_read(int addr, unsigned char *buf, int num);
static int sdc_single_write(int addr, unsigned char *buf);
static int sdc_multi_write(int addr, unsigned char *buf, int num);
static void sdc_hw_init(void);
static void sdc_set_blklength(unsigned int length);
static void sdc_set_buswidth(unsigned int width);
static void sdc_set_clock(unsigned int clock);

#if !defined(EMXX_MINIBOOT) || defined(CONFIG_EMXX_SDBOOT)
const hw_if_st sdc_if = {
	.send_cmd	= sdc_send_cmd,
	.send_acmd	= sdc_send_acmd,
	.send_extcsd	= sdc_send_extcsd,
	.single_read	= sdc_single_read,
	.multi_read	= sdc_multi_read,
	.single_write	= sdc_single_write,
	.multi_write	= sdc_multi_write,
	.hw_init	= sdc_hw_init,
	.set_blklength	= sdc_set_blklength,
	.set_buswidth	= sdc_set_buswidth,
	.set_clock	= sdc_set_clock,
};
#else
const hw_if_st sdc_if = {
	.send_cmd	= sdc_send_cmd,
	.send_acmd	= NULL,
	.send_extcsd	= NULL,
	.single_read	= NULL,
	.multi_read	= sdc_multi_read,
	.single_write	= NULL,
	.multi_write	= NULL,
	.hw_init	= sdc_hw_init,
	.set_blklength	= sdc_set_blklength,
	.set_buswidth	= NULL,
	.set_clock	= sdc_set_clock,
};
#endif


static volatile int *acc_mode=(volatile int *)SRAM_ACC_MODE;


static int
sdc_wait_response(void)
{
	unsigned int info2_val;

	while (!(inl(SDM_INFO1) & SDM_INFO1_RES_END)) {
		info2_val = inl(SDM_INFO2) & SDM_INFO2_ERR_ALL;
		if (info2_val){
			outl(info2_val & ~SDM_INFO2_ERR_ALL, SDM_INFO2);
			return info2_val;
		}
	}
	info2_val = inl(SDM_INFO2) & SDM_INFO2_ERR_ALL;
	if (info2_val){
		outl(info2_val & ~SDM_INFO2_ERR_ALL, SDM_INFO2);
		return info2_val;
	}
	outl(inl(SDM_INFO1) & ~SDM_INFO1_RES_END, SDM_INFO1);
	return 0;
}

static int 
sdc_send_cmd(int cmd, int arg, int type, int *resp)
{
	int ret;

	outl(SDM_INFO_CLEAR, SDM_INFO1);	// Clear ALL
	outl(SDM_INFO_CLEAR, SDM_INFO2);	// Clear ALL

	outl(arg & 0xffff, SDM_ARG0);
	outl((arg >> 16) & 0xffff, SDM_ARG1);

	cmd = cmd & 0xff;
	switch (cmd) {
	case 17:
		outl((0x1800 | cmd), SDM_CMD);
		break;
	case 18:
		outl((0x3800 | cmd), SDM_CMD);
		break;
	case 24:
		outl((0x0800 | cmd), SDM_CMD);
		break;
	case 25:
		outl((0x2800 | cmd), SDM_CMD);
		break;
	default:
		outl(cmd, SDM_CMD);
	}

	ret = sdc_wait_response();
	if (ret == 0) {
		if (resp != NULL) {
			*resp = inl(SDM_RSP0) & 0xffff;
			*resp |= (inl(SDM_RSP1) & 0xffff) << 16;
			resp++;
			*resp = inl(SDM_RSP2) & 0xffff;
			*resp |= (inl(SDM_RSP3) & 0xffff) << 16;
			resp++;
			*resp = inl(SDM_RSP4) & 0xffff;
			*resp |= (inl(SDM_RSP5) & 0xffff) << 16;
			resp++;
			*resp = inl(SDM_RSP6) & 0xffff;
			*resp |= (inl(SDM_RSP7) & 0xffff) << 16;
		}
		if (type == MMC_RSP_R1B) {
			// wait DATA0 == 1
			while (!(inl( SDM_INFO2 ) & SDM_INFO2_DATA0_BIT)) {
			}
		}
	}
	return ret;
}

#if !defined(EMXX_MINIBOOT) || defined(CONFIG_EMXX_SDBOOT)
static int
sdc_send_acmd(int cmd, int arg, int type, int *resp)
{
	int ret;

	outl(SDM_INFO_CLEAR, SDM_INFO1);  // Clear ALL
	outl(SDM_INFO_CLEAR, SDM_INFO2);  // Clear ALL

	outl(arg & 0xffff, SDM_ARG0);
	outl((arg >> 16) & 0xffff, SDM_ARG1);

	cmd = cmd & 0xff;
	switch(cmd) {
	case 6:
	case 41:
		outl((0x0040 | cmd), SDM_CMD);	// acommand bit set
		ret = sdc_wait_response();
		break;
	default:
		outl(SDM_INFO_MASK_ALL, SDM_INFO1_MASK);
		outl(SDM_INFO_MASK_ALL, SDM_INFO2_MASK);
		ret = -1;
		break;
	}

	if (ret == 0 && resp != NULL) {
		*resp = inl( SDM_RSP0 ) & 0xffff;
		*resp |= (inl( SDM_RSP1 ) & 0xffff) << 16;
	}
	return ret;
}

static int
sdc_send_extcsd(unsigned char *buf)
{
	int i;
	int ret;
	unsigned int info2_tmp;
	unsigned short *tmp_buf = (unsigned short *)buf;

	/* CMD8 : SEND_EXT_CSD */
	ret = sdc_send_cmd(8, 0, MMC_RSP_R1, NULL);
	if (ret != 0) {
		return ret;
	}

	while (1) {
		info2_tmp = inl(SDM_INFO2);
		if (info2_tmp & (SDM_INFO2_BRE_BIT | SDM_INFO2_ERR_ALL)){
			break;
		}
	}
	if (info2_tmp & SDM_INFO2_ERR_ALL) {
		outl((info2_tmp & ~SDM_INFO2_ERR_ALL), SDM_INFO2);
		return -1;
	}

	/* 1block read */
	for (i = 0; i < MMC_HALF_BLOCKLEN_VAL; i++) {
		*tmp_buf++ = (unsigned short)inl(SDM_BUF0);
	}

	/* wait for data read end */
	while (!(inl( SDM_INFO1 ) & SDM_INFO1_RES_RW_END)) {
		if (inl( SDM_INFO2 ) & SDM_INFO2_ERR_ALL) {
			outl((inl(SDM_INFO2) & ~SDM_INFO2_ERR_ALL), SDM_INFO2);
			return -1;
		}
	}
	outl((inl(SDM_INFO1) & ~(SDM_INFO1_RES_END | SDM_INFO1_RES_RW_END)), SDM_INFO1);
	return 0;
}

static int
sdc_single_read(int addr, unsigned char *buf)
{
	int i;
	int ret;
	unsigned int address, info2_tmp;
	unsigned short *tmp_buf = (unsigned short *)buf;

	outl(0, SDM_STOP);

	if (*acc_mode) {
		address = addr;
	} else {
		address = addr * MMC_BLOCKLEN_VAL;
	}

	/* CMD17 : READ_SINGLE_BLOCK */
	ret = sdc_send_cmd(17, address, MMC_RSP_R1, NULL);
	if (ret != 0){
		return ret;
	}
	while (1) {
		info2_tmp = inl(SDM_INFO2);
		if (info2_tmp & (SDM_INFO2_BRE_BIT | SDM_INFO2_ERR_ALL)){
			break;
		}
	}
	if (info2_tmp & SDM_INFO2_ERR_ALL) {
		outl((info2_tmp & ~SDM_INFO2_ERR_ALL), SDM_INFO2);
		return -1;
	}

	/* 1block read */
	for (i = 0; i < MMC_HALF_BLOCKLEN_VAL; i++) {
		*tmp_buf++ = (unsigned short)inl(SDM_BUF0);
	}

	/* wait for data read end */
	while (!(inl( SDM_INFO1 ) & SDM_INFO1_RES_RW_END)) {
		if (inl( SDM_INFO2 ) & SDM_INFO2_ERR_ALL) {
			outl((inl(SDM_INFO2) & ~SDM_INFO2_ERR_ALL), SDM_INFO2);
			return -1;
		}
	}
	outl((inl(SDM_INFO1) & ~(SDM_INFO1_RES_END | SDM_INFO1_RES_RW_END)), SDM_INFO1);
	return 0;
}
#endif	/* defined(EMXX_MINIBOOT) */

static int
sdc_multi_read(int addr, unsigned char *buf, int num)
{
	unsigned int address, info2_tmp;
	int ret, i, j;
	unsigned short *tmp_buf = (unsigned short *)buf;

	outl(inl(SDM_STOP) | SDM_STOP_SEC_BIT, SDM_STOP);
	outl(num, SDM_SECCNT);

	if (*acc_mode) {
		address = addr;
	} else {
		address = addr * MMC_BLOCKLEN_VAL;
	}

	/* CMD18 : READ_MULTI_BLOCK */
	ret = sdc_send_cmd(18, address, MMC_RSP_R1, NULL);
	if (ret != 0) {
		return ret;
	}
	for (i = 0 ; i < num; i++) {
		while (1) {
			info2_tmp = inl(SDM_INFO2);
			if (info2_tmp & (SDM_INFO2_BRE_BIT | SDM_INFO2_ERR_ALL)) {
				break;
			}
		}
		if (info2_tmp & SDM_INFO2_ERR_ALL) {
			return -1;
		}
		outl(inl(SDM_INFO2) & ~SDM_INFO2_BRE_BIT, SDM_INFO2);
		for (j = 0; j < MMC_HALF_BLOCKLEN_VAL; j++) {
			*tmp_buf++ = (unsigned short)inl(SDM_BUF0);
		}
	}

	/* wait for data read end */
	while (!(inl( SDM_INFO1 ) & SDM_INFO1_RES_RW_END)) {
		if (inl( SDM_INFO2 ) & SDM_INFO2_ERR_ALL) {
			outl((inl(SDM_INFO2) & ~SDM_INFO2_ERR_ALL), SDM_INFO2);
			return -1;
		}
	}
	outl((inl(SDM_INFO1) & ~(SDM_INFO1_RES_END | SDM_INFO1_RES_RW_END)), SDM_INFO1);
	return 0;
}

#if !defined(EMXX_MINIBOOT) || defined(CONFIG_EMXX_SDBOOT)
static int
sdc_single_write(int addr, unsigned char *buf)
{
	int i;
	int ret;
	unsigned int address, info2_tmp;
	unsigned short *tmp_buf = (unsigned short *)buf;

	outl(0, SDM_STOP);

	if (*acc_mode) {
		address = addr;
	} else {
		address = addr * MMC_BLOCKLEN_VAL;
	}

	/* CMD24 : WRITE_SINGLE_BLOCK */
	ret = sdc_send_cmd(24, address, MMC_RSP_R1, NULL);
	if (ret != 0){
		return ret;
	}
	while (1) {
		info2_tmp = inl(SDM_INFO2);
		if (info2_tmp & (SDM_INFO2_BWE_BIT | SDM_INFO2_ERR_ALL)){
			break;
		}
	}
	if (info2_tmp & SDM_INFO2_ERR_ALL) {
		outl((info2_tmp & ~SDM_INFO2_ERR_ALL), SDM_INFO2);
		return -1;
	}

	if (buf != NULL) {
		/* 1block write */
		for (i = 0; i < MMC_HALF_BLOCKLEN_VAL; i++) {
			outl(*tmp_buf++, SDM_BUF0);
		}
	} else {
		/* 1block clear */
		for (i = 0; i < MMC_HALF_BLOCKLEN_VAL; i++) {
			outl(0, SDM_BUF0);
		}
	}

	/* wait for data write end */
	while (!(inl( SDM_INFO1 ) & SDM_INFO1_RES_RW_END)) {
		if (inl( SDM_INFO2 ) & SDM_INFO2_ERR_ALL) {
			outl((inl(SDM_INFO2) & ~SDM_INFO2_ERR_ALL), SDM_INFO2);
			return -1;
		}
	}
	outl((inl(SDM_INFO1) & ~(SDM_INFO1_RES_END | SDM_INFO1_RES_RW_END)), SDM_INFO1);
	return 0;
}

static int
sdc_multi_write(int addr, unsigned char *buf, int num)
{
	unsigned int address, info2_tmp;
	int ret, i, j;
	unsigned short *tmp_buf = (unsigned short *)buf;

	outl(inl(SDM_STOP) | SDM_STOP_SEC_BIT, SDM_STOP);
	outl(num, SDM_SECCNT);

	if (*acc_mode) {
		address = addr;
	} else {
		address = addr * MMC_BLOCKLEN_VAL;
	}

	/* CMD25 : WRITE_MULTI_BLOCK */
	ret = sdc_send_cmd(25, address, MMC_RSP_R1, NULL);
	if (ret != 0) {
		return ret;
	}
	for (i = 0 ; i < num; i++) {
		while (1) {
			info2_tmp = inl(SDM_INFO2);
			if (info2_tmp & (SDM_INFO2_BWE_BIT | SDM_INFO2_ERR_ALL)) {
				break;
			}
		}
		if (info2_tmp & SDM_INFO2_ERR_ALL) {
			return -1;
		}
		outl(inl(SDM_INFO2) & ~SDM_INFO2_BWE_BIT, SDM_INFO2);
		if (buf != NULL) {
			/* 1block write */
			for (j = 0; j < MMC_HALF_BLOCKLEN_VAL; j++) {
				outl(*tmp_buf++, SDM_BUF0);
			}
		} else {
			/* 1block clear */
			for (j = 0; j < MMC_HALF_BLOCKLEN_VAL; j++) {
				outl(0, SDM_BUF0);
			}
		}
	}

	/* wait for data write end */
	while (!(inl( SDM_INFO1 ) & SDM_INFO1_RES_RW_END)) {
		if (inl( SDM_INFO2 ) & SDM_INFO2_ERR_ALL) {
			outl((inl(SDM_INFO2) & ~SDM_INFO2_ERR_ALL), SDM_INFO2);
			return -1;
		}
	}
	outl((inl(SDM_INFO1) & ~(SDM_INFO1_RES_END | SDM_INFO1_RES_RW_END)), SDM_INFO1);
	return 0;
}
#endif	/* defined(EMXX_MINIBOOT) */

static void
sdc_hw_init(void)
{
#if defined(CONFIG_EMXX_EMMCBOOT)
	MBRecord		*mbr_data = (MBRecord *)ROMBOOT_DATA_START;
	unsigned int	first_sector, last_sector;

	last_sector = (inl(SDM_ARG0) & 0xffff) | ((inl(SDM_ARG1) & 0xffff)<< 16);
	/* check access mode */
	first_sector  = mbr_data->partitionTable[0].firstSectorNumbers;
	if( (last_sector - first_sector) < MMC_BLOCKLEN_VAL ) {
		*acc_mode = 1;
	} else {
		*acc_mode = 0;
	}
#else
	*acc_mode = 0;
#endif
#if 0
	/* initialize SDC */
	outl(SDM_SOFT_RST_MDL, SDM_SOFT_RST);
	outl(SDM_SOFT_RST_SFT, SDM_SOFT_RST);

	outl(SDM_OPTION_VAL, SDM_OPTION);

	outl(SDM_INFO_CLEAR, SDM_INFO1);  // Clear ALL
	outl(SDM_INFO_CLEAR, SDM_INFO2);  // Clear ALL
	outl(SDM_INFO_MASK_ALL, SDM_INFO1_MASK);	// mask Clear ALL
	outl(SDM_INFO_MASK_ALL, SDM_INFO2_MASK);

	outl(SDM_CLK_CTRL_DIV64, SDM_CLK_CTRL);	// Clken and SystemCLK Div64(set 224KHz)
	/* wait (Long time in 1ms and 74clock (74/224,000 --> 350usec)) */
	udelay(1000);
#endif
}

static void
sdc_set_blklength(unsigned int length)
{
	outl(length, SDM_SIZE);
}

#if !defined(EMXX_MINIBOOT) || defined(CONFIG_EMXX_SDBOOT)
static void
sdc_set_buswidth(unsigned int width)
{
	if (width == 1) {
		outl(inl(SDM_OPTION) | SDM_OPT_WIDTH_BIT, SDM_OPTION);
	} else {
		outl((inl( SDM_OPTION) & ~SDM_OPT_WIDTH_BIT), SDM_OPTION);
	}
}
#endif

static void
sdc_set_clock(unsigned int clock)
{
	if (clock) {
		outl(0, SDM_CLK_CTRL);
#if defined(CONFIG_EMXX_EMMCBOOT)
		outl(SDM_CLK_CTRL_DIV2, SDM_CLK_CTRL);
#else
		outl(SDM_CLK_CTRL_DIV4, SDM_CLK_CTRL);
#endif
	} else {
		outl(0x0020, SDM_CLK_CTRL);	//Clk Disable
	}
}

#endif	/* defined(CONFIG_EMXX_SDBOOT) || defined(CONFIG_EMXX_EMMCBOOT) */
