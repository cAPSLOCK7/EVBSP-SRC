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


#define LOAD_TYPE_UBOOT		0
#define LOAD_TYPE_KERNEL	1
#define LOAD_TYPE_FS		2

#define SD_BOOT_MAX_LOADSIZE	0x00040000	/* 256K */

#define KERNEL_START		CONFIG_SYS_LOAD_ADDR	/* SDRAM */
#define KERNEL_SIZE		0x00480000	/* 4.5M */

#define FS_START		0x46000000	/* SDRAM */
#define FS_SIZE			0x01c00000	/* 28M */

#define MINIBOOT_WORK		0x42000000


typedef struct MasterBootRecord {
	unsigned char nondata[446];
	struct {
		unsigned char	bootDescriptor;
		unsigned char	firstPartitionSector[3];
		unsigned char	fileSystemDescriptor;
		unsigned char	lastPartitionSector[3];
		unsigned int	firstSectorNumbers;
		unsigned int	numberOfSectors;
	} partitionTable[4];
	unsigned char    sig[2]; 
} __attribute__ ((packed)) MBRecord;

typedef struct BIOS_Parameter_Block {
		unsigned char	jmpOpeCode[3];
		unsigned char	OEMName[8];
		unsigned short	bytesPerSector;
		unsigned char	sectorsPerCluster;
		unsigned short	reservedSectors;
		unsigned char	numberOfFATs;
		unsigned short	rootEntries;
		unsigned short	totalSectors;
		unsigned char	mediaDescriptor;
		unsigned short	sectorsPerFAT;
		unsigned short	sectorsPerTrack;
		unsigned short	heads;
		unsigned int	hiddenSectors;
		unsigned int	bigTotalSectors;
	union {
		struct FAT16BPB_t {
			/* info */
			unsigned char	driveNumber;
			unsigned char	unused;
			unsigned char	extBootSignature;
			unsigned int	serialNumber;
			unsigned char	volumeLabel[11];
			unsigned char	fileSystemType[8];
			unsigned char	loadProgramCode[448];
		} __attribute__ ((packed)) fat16;

		struct FAT32BPB_t {
			unsigned int	bigSectorsPerFAT;
			unsigned short	extFlags;
			unsigned short	FS_Version;
			unsigned int	rootDirStrtClus;
			unsigned short	FSInfoSec;
			unsigned short	bkUpBootSec;
			unsigned char	reserved[12];
			/* info */
			unsigned char	driveNumber;
			unsigned char	unused;
			unsigned char	extBootSignature;
			unsigned int	serialNumber;
			unsigned char	volumeLabel[11];
			unsigned char	fileSystemType[8];
			unsigned char	loadProgramCode[420];
		} __attribute__ ((packed)) fat32;
	}__attribute__ ((packed));
	unsigned char	sig[2];                 /* 0x55, 0xaa */
} __attribute__ ((packed)) BPBlock;

typedef struct Root_Directory_Entry {
	unsigned char	name[8];
	unsigned char	extension[3];
	unsigned char	attribute;
	unsigned char	reserved;
	unsigned char	createTimeMs;
	unsigned short	createTime;
	unsigned short	createDate;
	unsigned short	accessDate;
	unsigned short	clusterHighWord;
	unsigned short	updateTime;
	unsigned short	updateDate;
	unsigned short	cluster;
	unsigned int	fileSize;
} __attribute__ ((packed)) RDEntry;

typedef struct FAT_INFO {
	int	bpb_sec_pos;
	int	fat_sec_pos;
	int	rde_sec_pos;
	int	user_sec_pos;
	int	byte_per_sec;
	int	sec_per_clus;
	int	root_entry;
	int	sec_per_fat;
	int	sec_rde_size;
	int	data_num_clus;
	int	now_fat_sec;
} fat_info_t;


