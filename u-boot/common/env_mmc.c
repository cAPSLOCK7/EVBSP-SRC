/*
 * Copyright (C) 2010 Renesas Electronics Corporation
 *
 * This file is based on the common/env_flash.c
 *
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>

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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* #define DEBUG */

#include <common.h>

#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <malloc.h>

#if defined(CONFIG_CMD_SAVEENV)
#define CMD_SAVEENV
#elif defined(CONFIG_ENV_OFFSET_REDUND)
#error Cannot use CONFIG_ENV_OFFSET_REDUND without CONFIG_CMD_SAVEENV & CONFIG_CMD_NAND
#endif

#if defined(CONFIG_ENV_SIZE_REDUND) && (CONFIG_ENV_SIZE_REDUND != CONFIG_ENV_SIZE)
#error CONFIG_ENV_SIZE_REDUND should be the same as CONFIG_ENV_SIZE
#endif

/* references to names in env_common.c */
extern uchar default_environment[];
extern int default_environment_size;

#if defined(CONFIG_EMXX_EMMCBOOT)
char *env_name_spec = "eMMC NAND Flash";
#elif defined(CONFIG_EMXX_ESDBOOT)
char *env_name_spec = "eSD NAND Flash";
#else /* defined(CONFIG_EMXX_SDBOOT) */
char *env_name_spec = "SD Memory Card";
#endif

env_t *env_ptr = (env_t *)CONFIG_ENV_TMP_ADDR;
static unsigned long env_offset = 0;

struct mmc_rw_info {
	unsigned long addr;		/* copy destination address */
	unsigned long block;	/* copy source block number */
	unsigned long size;		/* copy size [byte] */
	unsigned long erase;	/* erase flag */
};

/* extern function */
extern unsigned long mmc_flash_init(void);
extern int mmc_read(struct mmc_rw_info *);
#ifdef CMD_SAVEENV
extern int mmc_write(struct mmc_rw_info *);
#endif
extern int mmc_erase(struct mmc_rw_info *);

/* extern variable */
extern uchar default_environment[];
extern int default_environment_size;


uchar env_get_char_spec(int index)
{
	DECLARE_GLOBAL_DATA_PTR;

	return (*((uchar *)(gd->env_addr + index)));
}

int env_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	int ret = -1;
	struct mmc_rw_info read_info;
	
	/* initialize protect table */
	env_offset = mmc_flash_init();
	
	if (env_offset) {
		/* set read information */
		read_info.addr		= CONFIG_ENV_TMP_ADDR;	/* copy destination address */
		read_info.block		= env_offset;			/* copy source block number */
		read_info.size		= CONFIG_ENV_SIZE;		/* copy size [block] */
		ret = mmc_read(&read_info);
	}
	if (!ret && (crc32(0, env_ptr->data, ENV_SIZE) == env_ptr->crc)) {
		gd->env_addr  = (ulong)&(env_ptr->data);
		gd->env_valid = 1;
	} else {
		gd->env_addr  = (ulong)&default_environment[0];
		gd->env_valid = 0;
	}
	
	return (0);
}

#ifdef CMD_SAVEENV
int saveenv(void)
{
	int ret;
	struct mmc_rw_info mmc_info;
	
	if (!env_offset) {
		printf("error: no area to save enviroment information in %s\n", env_name_spec);
		return 1;
	}
	/* copy current environment to temporary buffer */
	memcpy((uchar *)CONFIG_ENV_TMP_ADDR, env_ptr, CONFIG_ENV_SIZE);

#if 0
	printf("Erasing to %s... ", env_name_spec);
	/* set erase information */
	mmc_info.block		= env_offset;		/* erase block number */
	mmc_info.size		= CONFIG_ENV_SIZE;	/* erase size [byte] */
	mmc_info.erase		= 0;				/* erase flag */
	ret = mmc_erase(&mmc_info);
	if (!ret) {
		printf("done\n");
	} else {
		printf("error\n");
		return 1;
	}
#endif

	printf("Writing to %s... ", env_name_spec);
	/* set write information */
	mmc_info.addr		= CONFIG_ENV_TMP_ADDR;	/* copy destination address */
	mmc_info.block		= env_offset;			/* copy source block number */
	mmc_info.size		= CONFIG_ENV_SIZE;		/* copy size [byte] */
	ret = mmc_write(&mmc_info);
	if (!ret) {
		printf("done\n");
		return 0;
	} else {
		printf("error\n");
		return 1;
	}
}

int initenv(void)
{
	int ret;
	struct mmc_rw_info mmc_info;
	
	if (!env_offset) {
		printf("error: no area to save enviroment information in %s\n", env_name_spec);
		return 1;
	}

	printf("Erasing to %s... ", env_name_spec);
	/* set nand erase information */
	mmc_info.block		= env_offset;		/* erase block number */
	mmc_info.size		= CONFIG_ENV_SIZE;	/* erase size [byte] */
	mmc_info.erase		= 0;				/* erase flag */
	ret = mmc_erase(&mmc_info);
	if (!ret) {
		printf("done\n");
	} else {
		printf("error\n");
		return 1;
	}
}

#endif /* CMD_SAVEENV */

void env_relocate_spec(void)
{
	memcpy(env_ptr, (void*)CONFIG_ENV_TMP_ADDR, CONFIG_ENV_SIZE);
}

