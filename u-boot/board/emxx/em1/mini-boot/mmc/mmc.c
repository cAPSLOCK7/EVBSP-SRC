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

#if defined(CONFIG_EMXX_MMCBOOT)

#include "../include/mini_config.h"
#include "../include/mmc.h"
#include "mmc.h"

/* static function (mini-boot & u-boot) */
static int mmc_multi_block_read(unsigned int address, unsigned char *data, unsigned int size);

/* static variable */
static const hw_if_st *hw_if  = &sdc_if;
static volatile int *acc_mode = (volatile int *)SRAM_ACC_MODE;
static volatile int *RCA_VAL  = (volatile int *)SRAM_RCA_VAL;

#if !defined(EMXX_MINIBOOT)

#include <common.h>

#define EMXX_MMC_OK			0
#define EMXX_MMC_ERR_START		-1
#define EMXX_MMC_ERR_END		-2
#define EMXX_MMC_ERR_PARTITION		-3
#define EMXX_MMC_ERR_PROTECT		-4
#define EMXX_MMC_ERR_WRITE		-5
#define EMXX_MMC_ERR_READ		-6
#define EMXX_MMC_ERR_OTHER		-9

#define EMXX_MMC_MAXBLK			0x80000000	/* 2G Block */
#define EMXX_ERASE_MAXBYTE		0x80000000	/* 2G Byte */
#define EMXX_ERASE_MAXBLK		(EMXX_ERASE_MAXBYTE / MMC_BLOCKLEN_VAL)	/* 4M Block */

/* boot information */
struct boot_info {
	unsigned long addr;	/* ddr address */
	unsigned long block;	/* block number */
	unsigned long size;	/* size [byte] */
	unsigned long erase;	/* erase flag */
};

/* static function definition */
static int mmc_check_protect( unsigned long, unsigned long, int );
static int mmc_multi_block_write( unsigned int address, unsigned char *data, unsigned int size);
static int mmc_multi_block_erase( unsigned int address, unsigned int size );

/* global variable */
int mmc_erase_blocks = CONFIG_MMC_SECT_OF_BLOCKS;
unsigned int mmc_protect_blk = 0;

/* static variable */
static unsigned int dev_total_blk = 0;

#if 0
void ......uboot_if_func_for_env_mmc......() {}
#endif

unsigned long 
mmc_flash_init( void )
{
	int		i, ret, sd_ver = 0;
	unsigned int	block_num, dev_size = 0, read_blk_len = 0;
	unsigned int	tmp_u32;
	unsigned int	*wp_tbl = (unsigned int *)CONFIG_MMC_PROTECT_TBL;
	MBRecord	*mbr_data = (MBRecord *)ROMBOOT_DATA_START;
	int		resp[4];
	int		c_size_n;
	int		c_size_mult[8] = { 4, 8, 16, 32, 64, 128, 256, 512};
	int		blk_len[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 512, 1024, 2048, 0, 0, 0, 0};
#if defined(CONFIG_EMXX_EMMCBOOT)
	unsigned char	ExtCsd[512];

	*RCA_VAL = 1;
#else

	ret = hw_if->single_read(0, (unsigned char *)mbr_data);
#endif

	/* CMD7 : DESELECT_CARD */
	ret = hw_if->send_cmd(7, 0, MMC_RSP_NONE, NULL);

	/* CMD9 : SEND_CSD */
	ret = hw_if->send_cmd(9, *RCA_VAL << 16, MMC_RSP_R2, resp);
	if (ret != 0 ) {
		return ret;
	}

	/* CMD7 : SELECT */
	ret = hw_if->send_cmd(7, *RCA_VAL << 16, MMC_RSP_R1, NULL);
	if (ret != 0) {
		return ret;
	}

#if defined(CONFIG_EMXX_EMMCBOOT)
	/* read EXTCSD */
	ret = hw_if->send_extcsd(ExtCsd);
	if(ret != 0) {
		ExtCsd[511] = 0xff;
	}

	dev_total_blk = ExtCsd[215];
	dev_total_blk <<= 8;
	dev_total_blk |= ExtCsd[214];
	dev_total_blk <<= 8;
	dev_total_blk |= ExtCsd[213];
	dev_total_blk <<= 8;
	dev_total_blk |= ExtCsd[212];

	/* get erase size [block] */
	block_num  = (resp[0] & 0xe0000000) >> 29;		/* get RSP1 */
	block_num |= (resp[1] & 0x00000003) << 3;		/* get RSP2 */
	tmp_u32 = ((resp[1] & 0x0000007c) >> 2) + 1;
	mmc_erase_blocks = (block_num + 1) * tmp_u32;
#else
	/* get erase size [block] */
	mmc_erase_blocks = (resp[1] & 0x00000040) >> 6;		/* get RSP2 */
	if (mmc_erase_blocks == 0) {
		block_num  = (resp[0] & 0x80000000) >> 31;	/* get RSP1 */
		block_num |= (resp[1] & 0x0000003f) << 1;	/* get RSP2 */
		mmc_erase_blocks = block_num;
	} else {
		mmc_erase_blocks = 1;
	}
	/* get SD Version  [Ver:1.0 or 2.0] */
	sd_ver = (resp[3] & 0x00c00000) >> 22;			/* get RSP7 */
#endif
	/* get sd device size [block] */
	if ((sd_ver == 0) && (dev_total_blk == 0)) {
		c_size_n  = (resp[1] & 0x00000380) >> 7;	/* get RSP2: C_SIZE_MULTI */
		dev_size  = (resp[1] & 0xffc00000) >> 22;	/* get RSP3: C_SIZE */
		dev_size |= (resp[2] & 0x00000003) << 10;	/* get RSP4: C_SIZE */
		dev_size = (dev_size + 1) * c_size_mult[c_size_n];

		read_blk_len = (resp[2] & 0x00000f00) >> 8;	/* get RSP4: READ_BL_LEN */
		read_blk_len = blk_len[read_blk_len];

		dev_total_blk = (dev_size * read_blk_len) / MMC_BLOCKLEN_VAL;
	} else if (sd_ver == 1) {
		dev_size = (resp[1] & 0xffffff00) >> 8;
		dev_total_blk = (dev_size + 1) * 1024;
	}

	i=0;
	for (block_num=CONFIG_MMC_BLOCKS; block_num<EMXX_MMC_MAXBLK; block_num<<=1) {
		if (block_num >= dev_total_blk) {
			break;
		}
		i++;
	}
	if (block_num == EMXX_MMC_MAXBLK) {
		dev_total_blk = EMXX_MMC_MAXBLK;
	}
	mmc_protect_blk = CONFIG_MMC_SECT_OF_BLOCKS << i;

	for (i=0; i<(CONFIG_MMC_PROTECT_TBL_SIZE >> 2); i++) {
		*(wp_tbl++) = 0x01010101;
	}

#if defined(CONFIG_EMXX_SDBOOT)
	if ((CONFIG_ENV_OFFSET + (CONFIG_ENV_SIZE/MMC_BLOCKLEN_VAL))
				 <= mbr_data->partitionTable[1].numberOfSectors) {
		return mbr_data->partitionTable[1].firstSectorNumbers + CONFIG_ENV_OFFSET;
#else
	if ((CONFIG_ENV_OFFSET + (CONFIG_ENV_SIZE/MMC_BLOCKLEN_VAL))
				 <= mbr_data->partitionTable[0].numberOfSectors) {
		return mbr_data->partitionTable[0].firstSectorNumbers + CONFIG_ENV_OFFSET;
#endif
	} else {
		return 0;
	}
}

int 
mmc_erase( struct boot_info *boot_info )
{
	int		ret;
	unsigned int	src;

	src  = boot_info->block;

	if (boot_info->erase) {
		/* erase */
		ret = mmc_multi_block_erase(src, boot_info->size);
	} else {
		/* clear */
		ret = mmc_multi_block_write(src, (unsigned char *)NULL, boot_info->size);
	}
	return ret;
}

int 
mmc_write( struct boot_info *boot_info )
{
	int		ret;
	unsigned int	dest, src;

	dest = (unsigned int)boot_info->addr;
	src  = boot_info->block;

	/* write */
	ret = mmc_multi_block_write(src, (unsigned char *)dest, boot_info->size);
	return ret;
}

int 
mmc_read( struct boot_info *boot_info )
{
	int		ret;
	unsigned int	dest, src;

	dest = (unsigned int)boot_info->addr;
	src  = boot_info->block;

	/* read */
	ret = mmc_multi_block_read(src, (unsigned char *)dest, boot_info->size);
	return ret;
}


#if 0
void ......uboot_if_func_for_cmd_flash......() {}
#endif

/* print */
int 
mmc_print_part( unsigned long part )
{
	int		ret;
	unsigned long	s_blk, e_blk;
	unsigned long	chk_sblk, chk_eblk;
	unsigned long	sect_size, sect_num, p_size;
	MBRecord	*mbr_data = (MBRecord *)ROMBOOT_DATA_START;
	int		i;
	unsigned char	*tmp_addr;

	if (part > CONFIG_MMC_PARTS) {
		return EMXX_MMC_ERR_PARTITION;
	}

	if (part) {
		part--;
		s_blk  = mbr_data->partitionTable[part].firstSectorNumbers;
		p_size = mbr_data->partitionTable[part].numberOfSectors;
		if ((!s_blk) || (!p_size)) {
			return EMXX_MMC_ERR_PARTITION;
		}
		printf("  -------------------------------------------------\n");
		printf("  Total       : %x Blocks\n", dev_total_blk);
		e_blk = s_blk + p_size;
		printf("  Partition %ld : %08lx - %08lx [block]\n", part+1, s_blk, e_blk-1);

		chk_sblk = (s_blk / mmc_protect_blk) * mmc_protect_blk;
		chk_eblk = ((e_blk + mmc_protect_blk - 1) / mmc_protect_blk) * mmc_protect_blk;
		if (chk_eblk > dev_total_blk) {
			chk_eblk = dev_total_blk;
		}
	} else {
		printf("  -------------------------------------------------\n");
		printf("  Total       : %x Blocks\n", dev_total_blk);
		for(part=0; part<CONFIG_MMC_PARTS; part++) {
			s_blk  = mbr_data->partitionTable[part].firstSectorNumbers;
			p_size = mbr_data->partitionTable[part].numberOfSectors;
			if(p_size) {
				printf("  Partition %ld : %08lx - %08lx [block]\n",
							part+1, s_blk, s_blk+p_size-1);
			}
		}
		chk_sblk = 0;
		chk_eblk = dev_total_blk;
	}

	sect_size = mmc_erase_blocks;
	sect_num = dev_total_blk / mmc_erase_blocks;
	printf("  Erase Size  : ");
	if (sect_size < 0x400) {
		printf("%lx Blocks in %ld Erase Unit\n", sect_size, sect_num);
	} else if (sect_size < 0x100000) {
		printf("%lxK Blocks in %ld Erase Unit\n", (sect_size >> 10), sect_num);
	} else {
		printf("%lxM Blocks in %ld Erase Unit\n", (sect_size >> 20), sect_num);
	}

	sect_size = mmc_protect_blk;
	sect_num = dev_total_blk / mmc_protect_blk;
	printf("  Protect Size: ");
	if (sect_size < 0x400) {
		printf("%lx Blocks in %ld Sectors\n", sect_size, sect_num);
	} else if (sect_size < 0x100000) {
		printf("%lxK Blocks in %ld Sectors\n", (sect_size >> 10), sect_num);
	} else {
		printf("%lxM Blocks in %ld Sectors\n", (sect_size >> 20), sect_num);
	}

	printf("  -------------------------------------------------\n");
	printf("  Sector Start Block Number:\n    ");
	ret = mmc_check_protect(chk_sblk, chk_eblk, 1);
	return ret;
}

/* erase */
int 
mmc_erase_sect( unsigned long s_blk, unsigned long e_blk )
{
	int			ret;
	unsigned long		chk_sblk, chk_eblk, size;
	struct boot_info	info;

	printf("erase check:   start = %lx, end = %lx\n", s_blk, e_blk);
	if ((s_blk & (mmc_erase_blocks - 1)) || (s_blk >= dev_total_blk)) {
		return EMXX_MMC_ERR_START;
	}
	if ((e_blk & (mmc_erase_blocks - 1)) || (s_blk >= e_blk) || (e_blk > dev_total_blk)) {
		return EMXX_MMC_ERR_END;
	}

	chk_sblk = (s_blk / mmc_protect_blk) * mmc_protect_blk;
	chk_eblk = ((e_blk + mmc_protect_blk - 1) / mmc_protect_blk) * mmc_protect_blk;
	if (chk_eblk > dev_total_blk) {
		chk_eblk = dev_total_blk;
	}

	printf("protect check: start = %lx, end = %lx\n", chk_sblk, chk_eblk);
	ret = mmc_check_protect(chk_sblk, chk_eblk, 0);
	if (ret) {
		return EMXX_MMC_ERR_PROTECT;
	}

	size = e_blk - s_blk;
	info.erase = 1;							/* erase flag */
	while (size > 0) {
		/* set emmc erase information */
		info.block = s_blk;					/* erase block */
		if (size > EMXX_ERASE_MAXBLK) {
			info.size = EMXX_ERASE_MAXBYTE;		/* erase size [byte] */
			size -= EMXX_ERASE_MAXBLK;
			s_blk += EMXX_ERASE_MAXBLK;
		} else {
			info.size = size * MMC_BLOCKLEN_VAL;	/* erase size [byte] */
			size = 0;
		}
		ret = mmc_erase(&info);
		if (ret != 0) {
			break;
		}
	}
	return ret;
}

int 
mmc_erase_part( unsigned long part )
{
	int		ret;
	unsigned long	s_blk, e_blk, p_size;
	MBRecord	*mbr_data = (MBRecord *)ROMBOOT_DATA_START;

	if (part > CONFIG_MMC_PARTS) {
		return EMXX_MMC_ERR_PARTITION;
	}

	if (part) {
		part--;
		s_blk  = mbr_data->partitionTable[part].firstSectorNumbers;
		p_size = mbr_data->partitionTable[part].numberOfSectors;
		if ((!s_blk) || (!p_size)) {
			return EMXX_MMC_ERR_PARTITION;
		}
		e_blk = s_blk + p_size;
	} else {
		s_blk = 0;
		e_blk = dev_total_blk;
	}
	ret = mmc_erase_sect(s_blk, e_blk);
	return ret;
}

/* write */
int 
mmc_write_sect( unsigned long addr, unsigned long s_blk, unsigned long e_blk )
{
	int			ret;
	unsigned long		chk_sblk, chk_eblk, size;
	struct boot_info	info;

	size = (e_blk - s_blk) * MMC_BLOCKLEN_VAL;
	if (addr < PHYS_SDRAM_1) {
		return EMXX_MMC_ERR_OTHER;
	}
	if ((addr + size) > (PHYS_SDRAM_2 + PHYS_SDRAM_2_SIZE)) {
		return EMXX_MMC_ERR_OTHER;
	}
	if (s_blk > dev_total_blk) {
		return EMXX_MMC_ERR_START;
	}
	if (e_blk > dev_total_blk) {
		return EMXX_MMC_ERR_END;
	}

	chk_sblk = (s_blk / mmc_protect_blk) * mmc_protect_blk;
	chk_eblk = ((e_blk + mmc_protect_blk - 1) / mmc_protect_blk) * mmc_protect_blk;
	if (chk_eblk > dev_total_blk) {
		chk_eblk = dev_total_blk;
	}

	printf("protect check: start = %lx, end = %lx\n", chk_sblk, chk_eblk);
	ret = mmc_check_protect(chk_sblk, chk_eblk, 0);
	if (ret) {
		return EMXX_MMC_ERR_PROTECT;
	}

	/* set emmc erase information */
	info.addr	= addr;		/* src address */
	info.block	= s_blk;	/* dst block */
	info.size	= size;		/* write size [byte] */
	ret = mmc_write(&info);
	if (ret) {
		return EMXX_MMC_ERR_WRITE;
	}
	return e_blk - s_blk;
}

int 
mmc_write_part( unsigned long addr, unsigned long part, unsigned int wb_size)
{
	int		ret=0;
	unsigned long	blk, p_size;
	MBRecord	*mbr_data = (MBRecord *)ROMBOOT_DATA_START;

	if ((part == 0) || (part > CONFIG_MMC_PARTS)) {
		return EMXX_MMC_ERR_PARTITION;
	}
	
	part--;
	blk = mbr_data->partitionTable[part].firstSectorNumbers;
	if (!blk) {
		return EMXX_MMC_ERR_PARTITION;
	}

	p_size = mbr_data->partitionTable[part].numberOfSectors;
	if (wb_size > p_size) {
		return EMXX_MMC_ERR_END;
	}
	ret = mmc_write_sect(addr, blk, blk + wb_size);
	return ret;
}

/* protect */
int 
mmc_protect_sect( unsigned long s_blk, unsigned long e_blk, int mode )
{
	unsigned char	*wp_tbl = (unsigned char *)CONFIG_MMC_PROTECT_TBL;
	unsigned long	sect, e_sect;

	if ((s_blk & (mmc_protect_blk - 1)) || (s_blk >= dev_total_blk)) {
		return EMXX_MMC_ERR_START;
	}
	if ((e_blk & (mmc_protect_blk - 1) && (e_blk != dev_total_blk))
			|| (s_blk >= e_blk) || (e_blk > dev_total_blk)) {
		return EMXX_MMC_ERR_END;
	}
	if (mode & 0xFFFFFFFE) {
		return EMXX_MMC_ERR_OTHER;
	}

	sect = s_blk / mmc_protect_blk;
	wp_tbl += sect;
	e_sect = (e_blk / mmc_protect_blk) - sect;
	if (!e_sect) {
		e_sect = 1;
	}
	for (sect=0; sect<e_sect; sect++) {
		*(wp_tbl++) = mode;
	}
	return sect;
}

int 
mmc_protect_part( unsigned long part, int mode )
{
	int		ret;
	unsigned long	s_blk, e_blk, p_size;
	MBRecord	*mbr_data = (MBRecord *)ROMBOOT_DATA_START;

	if (part > CONFIG_MMC_PARTS) {
		return EMXX_MMC_ERR_PARTITION;
	}
	
	if (part) {
		part--;
		s_blk  = mbr_data->partitionTable[part].firstSectorNumbers;
		p_size = mbr_data->partitionTable[part].numberOfSectors;
		if ((!s_blk) || (!p_size)) {
			return EMXX_MMC_ERR_PARTITION;
		}
		e_blk = s_blk + p_size;

		s_blk = (s_blk / mmc_protect_blk) * mmc_protect_blk;
		e_blk = ((e_blk + mmc_protect_blk - 1) / mmc_protect_blk) * mmc_protect_blk;
		if (e_blk > dev_total_blk) {
			e_blk = dev_total_blk;
		}
	} else {
		s_blk = 0;
		e_blk = dev_total_blk;
	}
	ret = mmc_protect_sect(s_blk, e_blk, mode);
	return ret;
}

#if 0
void ......uboot_static_func......() {}
#endif

static int 
mmc_check_protect( unsigned long s_blk, unsigned long e_blk, int print )
{
	unsigned char	*wp_tbl = (unsigned char *)CONFIG_MMC_PROTECT_TBL;
	unsigned long	sect, e_sect, p_blk;

	if ((s_blk & (mmc_protect_blk - 1)) || (s_blk >= dev_total_blk)) {
		return EMXX_MMC_ERR_START;
	}
	if ((e_blk & (mmc_protect_blk - 1) && (e_blk != dev_total_blk))
			|| (s_blk >= e_blk) || (e_blk > dev_total_blk)) {
		return EMXX_MMC_ERR_END;
	}

	sect = s_blk / mmc_protect_blk;
	wp_tbl += sect;
	e_sect = (e_blk / mmc_protect_blk) - sect;
	if (print) {
		p_blk = s_blk;
		for (sect = 0; sect < e_sect; sect++) {
			printf("%08lX ", p_blk);
			p_blk += mmc_protect_blk;
			if (*(wp_tbl++)) {
				printf("(RO) ");
			} else {
				printf("     ");
			}
			if (((sect+1)%5) == 0) {
				printf("\n    ");
			}
		}
		printf("\n");
	} else {
		for (sect = 0; sect < e_sect; sect++) {
			if (*(wp_tbl++)) {
				return EMXX_MMC_ERR_PROTECT;
			}
		}
	}
	return EMXX_MMC_OK;
}

static int
mmc_multi_block_write( unsigned int address, unsigned char *data, unsigned int size )
{
	int		ret;
	unsigned int	block_num;

	if (size == 0) {
		return -1;
	}

	block_num = (size / MMC_BLOCKLEN_VAL);
	if( size & (MMC_BLOCKLEN_VAL-1) ) {
		block_num ++;
	}

	ret = hw_if->multi_write(address, data, block_num);
	return ret;
}


static int
mmc_multi_block_erase( unsigned int address, unsigned int size )
{
	int		ret;
	unsigned int	end_block;

	printf("eMMC Erase: start %08x[block] length %08x[block]\n", address, (size / MMC_BLOCKLEN_VAL));
	if( size == 0 ) {
		return -1;
	}

	if (*acc_mode) {
		end_block = address + (size / MMC_BLOCKLEN_VAL) - 1;
		if( size & (MMC_BLOCKLEN_VAL-1) ) {
			end_block ++;
		}
	} else {
		address *= MMC_BLOCKLEN_VAL;
		end_block = address + size -1 ;
	}

#if defined(CONFIG_EMXX_EMMCBOOT)
	/* CMD35 : ERASE_GROUP_START */
	ret = hw_if->send_cmd(35, address, MMC_RSP_R1, NULL);
	if (ret != 0) {
		return ret;
	}

	/* CMD36 : ERASE_GROUP_END */
	ret = hw_if->send_cmd(36, end_block, MMC_RSP_R1, NULL);
	if (ret != 0) {
		return ret;
	}
#else
	/* CMD32 : ERASE_WR_BLK_START */
	ret = hw_if->send_cmd(32, address, MMC_RSP_R1, NULL);
	if (ret != 0) {
		return ret;
	}

	/* CMD33 : ERASE_WR_BLK_END */
	ret = hw_if->send_cmd(33, end_block, MMC_RSP_R1, NULL);
	if (ret != 0) {
		return ret;
	}
#endif

	/* CMD38 : ERASE */
	ret = hw_if->send_cmd(38, 0, MMC_RSP_R1B, NULL);
	if (ret != 0) {
		return ret;
	}
	return 0;
}

#else	/* defined(EMXX_MINIBOOT) */

#include "../include/etc.h"

#define IMAGE_DATA_SIZE		0x0c

#if defined(CONFIG_EMXX_SDBOOT)
#if 0
void ......miniboot_static_func......() {}
#endif

/* static function */
static int fat_get_cluster(unsigned char *, fat_info_t *, int);
static int fat_mbr_pram_chk(MBRecord *);
static int fat_bpb_pram_chk(BPBlock *);
static int fat_cpy_data(unsigned char *, fat_info_t *, unsigned int, unsigned int);

/* static variable */
static int fat32_flag = 0;

static const unsigned char match_name[3][8] = {
	{0x75, 0x62, 0x6F, 0x6F, 0x74, 0x2D, 0x73, 0x64},	/* uboot-sd */
	{0x75, 0x49, 0x6D, 0x61, 0x67, 0x65, 0x20, 0x20},	/* uImage */
	{0x63, 0x72, 0x61, 0x6D, 0x66, 0x73, 0x20, 0x20},	/* cramfs */
};
static const unsigned char MATCH_NAME[3][8] = {
	{0x55, 0x42, 0x4F, 0x4F, 0x54, 0x2D, 0x53, 0x44},	/* UBOOT-SD */
	{0x55, 0x49, 0x4D, 0x41, 0x47, 0x45, 0x20, 0x20},	/* UIMAGE */
	{0x43, 0x52, 0x41, 0x4D, 0x46, 0x53, 0x20, 0x20},	/* CRAMFS */
};

static int
fat_get_cluster(unsigned char *read_data, fat_info_t *fat_info, int type)
{
	int	i,j,ret;
	RDEntry *rde;

	const unsigned char match_exet[3] = {0x62, 0x69, 0x6E}; /* bin */
	const unsigned char MACTH_EXET[3] = {0x42, 0x49, 0x4E}; /* BIN */

	int read_clus_sec, fat_sec_num, get_clus, rde_clus;
	unsigned int *read_fat32_buf;
	unsigned char *rde_clus_data = (unsigned char *)MINIBOOT_WORK;

	if (fat32_flag){
		read_fat32_buf = (unsigned int *)read_data;
		rde_clus = fat_info->rde_sec_pos;

		while (1) {
			read_clus_sec = fat_info->user_sec_pos + 
							((rde_clus - 2) * fat_info->sec_per_clus);
			ret = hw_if->multi_read(read_clus_sec, rde_clus_data, fat_info->sec_per_clus);
			if (ret != 0) {
				dbg_print("read ERR\n");
				return ret;
			}
			for (j = 0; j < 16 * fat_info->sec_per_clus; j++) {
				rde = (RDEntry *)(read_data + (32 * j));
				if (!memcmp(rde->name, match_name[type], 8) || !memcmp(rde->name, MATCH_NAME[type], 8)) {
					if (type != LOAD_TYPE_UBOOT) {
						fat_info->data_num_clus = rde->cluster | (rde->clusterHighWord << 16);
						return 0;
					} else {
						if (!memcmp(rde->extension, match_exet, 3) || !memcmp(rde->extension, MACTH_EXET, 3)) {
							fat_info->data_num_clus = rde->cluster | (rde->clusterHighWord << 16);
							return 0;
						}
					}
				}
			}

			fat_sec_num = rde_clus >> 7;
			if (fat_info->now_fat_sec != fat_sec_num) {
				ret = hw_if->single_read(fat_info->fat_sec_pos + fat_sec_num, 
						(unsigned char *)read_fat32_buf);
				if (ret != 0) {
					return ret;
				}
				fat_info->now_fat_sec = fat_sec_num;
			}
			get_clus = read_fat32_buf[rde_clus & 0x7F];
			if (get_clus < 0x00000002 || get_clus > 0x0FFFFFF6) {
				break;
			}
			rde_clus = get_clus;
		}
	} else {
		for (i = 0; i < fat_info->sec_rde_size; i++) {
			ret = hw_if->single_read(fat_info->rde_sec_pos + i, read_data);
			if (ret != 0) {
				dbg_print("read ERR\n");
				return ret;
			}
			for (j = 0; j < 16; j++) {
				rde = (RDEntry *)(read_data + (32 * j));
				if (!memcmp(rde->name, match_name[type], 8) || !memcmp(rde->name, MATCH_NAME[type], 8)) {
					if (type != LOAD_TYPE_UBOOT) {
						fat_info->data_num_clus = rde->cluster;
						return 0;
					} else {
						if (!memcmp(rde->extension, match_exet, 3) || !memcmp(rde->extension, MACTH_EXET, 3)) {
							fat_info->data_num_clus = rde->cluster;
							return 0;
						}
					}
				}
			}
		}
	}
	return -1;
}

static int
fat_mbr_pram_chk(MBRecord *mbr)
{
	BPBlock *bpb;
	unsigned char match_fat16[5] = {0x46, 0x41, 0x54, 0x31, 0x36}; /* FAT16 */
	unsigned char match_fat32[5] = {0x46, 0x41, 0x54, 0x33, 0x32}; /* FAT32 */

	/* CHK pattrn */
	if (!((mbr->sig[0] == 0x55) && (mbr->sig[1] == 0xAA))) {
		return -1;
	}
	/* CHK FAT16 (more 32M) or FAT32 */
	if ((mbr->partitionTable[0].fileSystemDescriptor == 0x6) ||
			(mbr->partitionTable[0].fileSystemDescriptor == 0xb)) {
		return 0;
	} else {
		bpb = (BPBlock *)mbr;
		if (!memcmp(bpb->fat16.fileSystemType, match_fat16, 5)) {
			return 1;
		}
		if (!memcmp(bpb->fat32.fileSystemType, match_fat32, 5)) {
			return 1;
		}
		return -1;
	}
}

static int
fat_bpb_pram_chk(BPBlock *bpb)
{
	/* CHK pattrn */
	if (!((bpb->sig[0] == 0x55) && (bpb->sig[1] == 0xAA))) {
		return -1;
	}
	/* if bytesPerSector != 512, not support!! */
	if ((bpb->bytesPerSector != 0x200)) {
		return -1;
	}
	/* if sectorsPerCluster > 128, not support!! */
	if ((bpb->sectorsPerCluster > 128)) {
		return -1;
	}
	return 0;
}

static int
fat_cpy_data(unsigned char *read_data, fat_info_t *fat_info, unsigned int start_addr, unsigned int max_size)
{
	int	ret, last_flag = 0, write_size = 0, clus_size;
	int	read_clus_sec, fat_sec_num, get_clus;
	unsigned short *read_fat_buf = (unsigned short *)read_data;
	unsigned int *read_fat32_buf = (unsigned int *)read_data;
	unsigned char *boot_addr = (unsigned char *)start_addr;

	clus_size = MMC_BLOCKLEN_VAL * fat_info->sec_per_clus;
	while ((write_size + clus_size) <= max_size) {
		read_clus_sec = fat_info->user_sec_pos + 
							((fat_info->data_num_clus - 2) * fat_info->sec_per_clus);
		ret = hw_if->multi_read(read_clus_sec, 
				(unsigned char *)boot_addr + write_size, fat_info->sec_per_clus);
		if (ret != 0) {
			return ret;
		}

		if (fat32_flag == 1) {
			fat_sec_num = fat_info->data_num_clus >> 7;
			if (fat_info->now_fat_sec != fat_sec_num) {
				ret = hw_if->single_read(fat_info->fat_sec_pos + fat_sec_num, read_data);
				if (ret != 0) {
					return ret;
				}
				fat_info->now_fat_sec = fat_sec_num;
			}
			get_clus = read_fat32_buf[fat_info->data_num_clus & 0x7F];
			if (get_clus < 0x00000002 || get_clus > 0x0FFFFFF6) {
				if (get_clus >= 0x0FFFFFF8 && get_clus <= 0x0FFFFFFF) {
					last_flag = 1;
				}
				break;
			}
		} else {
			fat_sec_num = fat_info->data_num_clus >> 8;
			if (fat_info->now_fat_sec != fat_sec_num) {
				ret = hw_if->single_read(fat_info->fat_sec_pos + fat_sec_num, read_data);
				if (ret != 0) {
					return ret;
				}
				fat_info->now_fat_sec = fat_sec_num;
			}
			get_clus = read_fat_buf[fat_info->data_num_clus & 0xFF];
			if (get_clus < 0x0002 || get_clus > 0xFFF6) {
				if (get_clus >= 0xFFF8 && get_clus <= 0xFFFF) {
					last_flag = 1;
				}
				break;
			}
		}
		fat_info->data_num_clus = get_clus;
		write_size += clus_size;
	}
	if (last_flag != 1) {
		if (SD_BOOT_MAX_LOADSIZE == max_size ) {
			dbg_print(" uboot-sd.bin is over 512KB.\n");
		} else {
			dbg_print(" uImage is over 3.5MB.\n");
		}
		return -1;
	}
	return 0;
}
#endif

#if 0
void ......miniboot_if_func......() {}
#endif

void 
mini_boot( void )
{
	int		ret;
	MBRecord	*mbr_data = (MBRecord *)ROMBOOT_DATA_START;
#if defined(CONFIG_EMXX_EMMCBOOT)
	unsigned int	dest, src, k_size;
	unsigned char	*head_ptr;
#else
	BPBlock		*bpb;
	fat_info_t	fat_info;
	unsigned char	*read_data = (unsigned char *)(ROMBOOT_DATA_START + MMC_BLOCKLEN_VAL);
	unsigned char	*fat_info_data = (unsigned char *)ROMBOOT_DATA_START;
	int		resp[4];

	uart_init();
#endif

	hw_if->hw_init();
	/* change high speed mode */
	hw_if->set_clock(MMC_CLK_HIGH);

#if defined(CONFIG_EMXX_EMMCBOOT)
	/* u-boot read */
	src  = mbr_data->partitionTable[0].firstSectorNumbers + (MINIBOOT_EMMC_SIZE / MMC_BLOCKLEN_VAL);
	dest = (unsigned int)U_BOOT_TEXTADDR;
	ret = mmc_multi_block_read(src, (unsigned char *)dest, U_BOOT_SIZE);
	if (ret != 0) {
		goto ERR_END;
	}

	/* kernel read */
	src  = mbr_data->partitionTable[1].firstSectorNumbers;
	dest = (unsigned int)KERNEL_START;
	ret = mmc_multi_block_read(src, (unsigned char *)dest, MMC_BLOCKLEN_VAL);
	if (ret != 0) {
		goto ERR_END;
	}
	head_ptr = (unsigned char *)dest + IMAGE_DATA_SIZE;
	k_size = (unsigned int)(*head_ptr++);
	k_size <<= 8;
	k_size |= (unsigned int)(*head_ptr++);
	k_size <<= 8;
	k_size |= (unsigned int)(*head_ptr++);
	k_size <<= 8;
	k_size |= (unsigned int)*head_ptr;
	ret = mmc_multi_block_read(src, (unsigned char *)dest, (k_size + 64));
	if (ret != 0) {
		goto ERR_END;
	}
#else
	/* CMD7 : DESELECT_CARD */
	ret = hw_if->send_cmd(7, 0, MMC_RSP_R1, NULL);

	/* CMD3 SET */
	ret = hw_if->send_cmd(3, 0, MMC_RSP_R6, resp);
	if (ret != 0) {
		dbg_print("CMD3 error\n");
		goto ERR_END;
	}
	*RCA_VAL = (resp[0] >> 16) & 0xffff;

	/* CMD7 : SELECT */
	ret = hw_if->send_cmd(7, *RCA_VAL << 16, MMC_RSP_R1, NULL);
	if (ret != 0) {
		dbg_print("CMD7 error\n");
		goto ERR_END;
	}
	
	/* CMD55 SET */
	ret = hw_if->send_cmd(55, *RCA_VAL << 16, MMC_RSP_R1, NULL);
	if(ret != 0) {
		dbg_print("CMD55 error\n");
		goto ERR_END;
	}

	/* ACMD6 SET Width 4bit */
	ret = hw_if->send_acmd(6, 0x0002, MMC_RSP_R1, NULL);
	if(ret != 0) {
		dbg_print("ACMD6 error\n");
		goto ERR_END;
	}
	/* set Width 4bit */
	hw_if->set_buswidth(4);

	/* read mbr */
	ret = hw_if->single_read(0, (unsigned char *)read_data);
	if (ret != 0) {
		dbg_print("mbr read error\n");
		goto ERR_END;
	}

	mbr_data = (MBRecord *)read_data;
	ret = fat_mbr_pram_chk(mbr_data);
	if (ret == -1) {
		dbg_print("check MasterBootRecord error\n");
		goto ERR_END;
	} else if (ret == 0) {
		fat_info.bpb_sec_pos = mbr_data->partitionTable[0].firstSectorNumbers;
		ret = hw_if->single_read(fat_info.bpb_sec_pos, (unsigned char *)read_data);
		if (ret != 0) {
			dbg_print("bpb read error\n");
			goto ERR_END;
		}
	} else if (ret == 1) {
		fat_info.bpb_sec_pos = 0;
	}

	bpb = (BPBlock *)read_data;
	ret = fat_bpb_pram_chk(bpb);
	if (ret != 0) {
		dbg_print("check BIOS Parameter Block error\n");
		goto ERR_END;
	}

	fat_info.fat_sec_pos   = fat_info.bpb_sec_pos + bpb->reservedSectors;
	fat_info.byte_per_sec  = bpb->bytesPerSector;
	fat_info.sec_per_clus  = bpb->sectorsPerCluster;
	fat_info.root_entry    = bpb->rootEntries;
	fat_info.data_num_clus = 0;

	if (bpb->rootEntries == 0) {
		fat32_flag = 1;
		fat_info.sec_per_fat  = bpb->fat32.bigSectorsPerFAT;
		fat_info.rde_sec_pos  = bpb->fat32.rootDirStrtClus;
		fat_info.sec_rde_size = 0;
		fat_info.user_sec_pos  = fat_info.fat_sec_pos + (bpb->numberOfFATs * fat_info.sec_per_fat);
	} else {
		fat32_flag = 0;
		fat_info.sec_per_fat  = bpb->sectorsPerFAT;
		fat_info.rde_sec_pos  = fat_info.fat_sec_pos + (bpb->numberOfFATs * fat_info.sec_per_fat);
		fat_info.sec_rde_size = (fat_info.root_entry * 32) >> 9;
		fat_info.user_sec_pos = fat_info.rde_sec_pos + fat_info.sec_rde_size;
	}

	/* uboot-sd.bin */
#ifdef CONFIG_EMXX_SDBOOT_LINE
	dbg_print("\n\nLoad LineSystem U-BOOT ... ");
#endif
	fat_info.now_fat_sec = -1;
	ret = fat_get_cluster(read_data, &fat_info, LOAD_TYPE_UBOOT);
	if (ret != 0) {
		dbg_print("file uboot-sd.bin not exist error\n");
		goto ERR_END;
	}

	fat_info.now_fat_sec = -1;
	ret = fat_cpy_data(read_data, &fat_info, (unsigned int)U_BOOT_TEXTADDR, SD_BOOT_MAX_LOADSIZE);
	if (ret != 0) {
		dbg_print("copy uboot-sd.bin error\n");
		goto ERR_END;
	}

#ifdef CONFIG_EMXX_SDBOOT_LINE
	dbg_print("Done\n");
	dbg_print("Load LineSystem Kernel ... ");
#endif
	/* uImage */
	fat_info.now_fat_sec = -1;
	ret = fat_get_cluster(read_data, &fat_info, LOAD_TYPE_KERNEL);
	if (ret != 0) {
		dbg_print("file uImage not exist error\n");
		goto ERR_END;
	}
	fat_info.now_fat_sec = -1;

	ret = fat_cpy_data(read_data, &fat_info, (unsigned int)KERNEL_START, KERNEL_SIZE);
	if (ret != 0) {
		dbg_print("copy uImage error\n");
		goto ERR_END;
	}

#ifdef CONFIG_EMXX_SDBOOT_LINE
	dbg_print("Done\n");
	dbg_print("Load LineSystem Filesystem ... ");

	/* cramfs */
	ret = fat_get_cluster(read_data, &sd_info, LOAD_TYPE_FS);
	if (ret != 0) {
		dbg_print("\nfile cramfs not exist error\n");
		goto READ_SKIP;
	}
	sd_info.now_fat_sec = -1;

	ret = sd_cpy_data(read_data, &sd_info, (unsigned int)FS_START, FS_SIZE);
	if (ret != 0) {
		dbg_print("\ncopy cramfs error\n");
		goto READ_SKIP;
	}

	dbg_print("Done\n");
 READ_SKIP:
#endif
	dbg_print("jump u-boot\n");
#endif

	/* jump u-boot */
	((void (*)(void))U_BOOT_TEXTADDR)();

ERR_END:
	return;
}
#endif	/* defined(EMXX_MINIBOOT) */

#if 0
void ......miniboot_and_uboot_static_func......() {}
#endif

static int
mmc_multi_block_read( unsigned int address, unsigned char *data, unsigned int size )
{
	int		ret;
	unsigned int	block_num;

	if ((data == NULL) || (size == 0)) {
		return -1;
	}

	block_num = (size / MMC_BLOCKLEN_VAL);
	if( size & (MMC_BLOCKLEN_VAL-1) ) {
		block_num ++;
	}

	ret = hw_if->multi_read(address, data, block_num);
	return ret;
}
#endif	/* defined(CONFIG_EMXX_MMCBOOT) */
