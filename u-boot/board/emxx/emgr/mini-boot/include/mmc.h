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

#ifndef __MMC_H
#define __MMC_H

#define MMC_BLOCKLEN_VAL	512
#define MMC_HALF_BLOCKLEN_VAL	256

#define MMC_RES_TIMEOUT	0x40

#define MMC_RSP_NONE		0
#define MMC_RSP_R1		1
#define MMC_RSP_R1B		2
#define MMC_RSP_R2		3
#define MMC_RSP_R3		4
#define MMC_RSP_R4		5
#define MMC_RSP_R5		6
#define MMC_RSP_R6		7
#define MMC_RSP_R7		8

#define MMC_CLK_STOP		0
#define MMC_CLK_NORMAL		1
#define MMC_CLK_HIGH		2

typedef struct {
	int (*send_cmd)(int cmd, int arg, int type, int *resp);
	int (*send_acmd)(int cmd, int arg, int type, int *resp);
	int (*send_extcsd)(unsigned char *buf);
	int (*single_read)(int addr, unsigned char *buf);
	int (*multi_read)(int addr, unsigned char *buf, int num);
	int (*single_write)(int addr, unsigned char *buf);
	int (*multi_write)(int addr, unsigned char *buf, int num);
	void (*hw_init)(void);
	void (*set_blklength)(unsigned int length);
	void (*set_buswidth)(unsigned int width);
	void (*set_clock)(unsigned int clock);
} hw_if_st;


extern const hw_if_st sdc_if;
extern const hw_if_st sdio_if;

#endif	/* __MMC_H */

