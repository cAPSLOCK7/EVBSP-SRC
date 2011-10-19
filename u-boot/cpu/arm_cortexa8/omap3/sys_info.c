/*
 * (C) Copyright 2008
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *      Manikandan Pillai <mani.pillai@ti.com>
 *
 * Derived from Beagle Board and 3430 SDP code by
 *      Richard Woodruff <r-woodruff2@ti.com>
 *      Syed Mohammed Khasim <khasim@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/mem.h>	/* get mem tables */
#include <asm/arch/sys_proto.h>
#include <i2c.h>

extern omap3_sysinfo sysinfo;
static gpmc_csx_t *gpmc_cs_base = (gpmc_csx_t *)GPMC_CONFIG_CS0_BASE;
static sdrc_t *sdrc_base = (sdrc_t *)OMAP34XX_SDRC_BASE;
static ctrl_t *ctrl_base = (ctrl_t *)OMAP34XX_CTRL_BASE;

/******************************************
 * get_cpu_type(void) - extract cpu info
 ******************************************/
u32 get_cpu_type(void)
{
	return readl(&ctrl_base->ctrl_omap_stat);
}

/******************************************
 * get_cpu_rev(void) - extract version info
 ******************************************/
u32 get_cpu_rev(void)
{
	u32 cpuid = 0;

	/*
	 * On ES1.0 the IDCODE register is not exposed on L4
	 * so using CPU ID to differentiate
	 * between ES2.0 and ES1.0.
	 */
	__asm__ __volatile__("mrc p15, 0, %0, c0, c0, 0":"=r"(cpuid));
	if ((cpuid & 0xf) == 0x0)
		return CPU_3430_ES1;
	else
		return CPU_3430_ES2;

}

/****************************************************
 * is_mem_sdr() - return 1 if mem type in use is SDR
 ****************************************************/
u32 is_mem_sdr(void)
{
	if (readl(&sdrc_base->cs[CS0].mr) == SDP_SDRC_MR_0_SDR)
		return 1;
	return 0;
}

/***********************************************************************
 * get_cs0_size() - get size of chip select 0/1
 ************************************************************************/
u32 get_sdr_cs_size(u32 cs)
{
	u32 size;

	/* get ram size field */
	size = readl(&sdrc_base->cs[cs].mcfg) >> 8;
	size &= 0x3FF;		/* remove unwanted bits */
	size *= SZ_2M;		/* find size in MB */
	return size;
}

/***********************************************************************
 * get_sdr_cs_offset() - get offset of cs from cs0 start
 ************************************************************************/
u32 get_sdr_cs_offset(u32 cs)
{
	u32 offset;

	if (!cs)
		return 0;

	offset = readl(&sdrc_base->cs_cfg);
	offset = (offset & 15) << 27 | (offset & 0x30) >> 17;

	return offset;
}

/***********************************************************************
 * get_board_type() - get board type based on current production stats.
 *  - NOTE-1-: 2 I2C EEPROMs will someday be populated with proper info.
 *    when they are available we can get info from there.  This should
 *    be correct of all known boards up until today.
 *  - NOTE-2- EEPROMs are populated but they are updated very slowly.  To
 *    avoid waiting on them we will use ES version of the chip to get info.
 *    A later version of the FPGA migth solve their speed issue.
 ************************************************************************/
u32 get_board_type(void)
{
	if (get_cpu_rev() == CPU_3430_ES2)
		return sysinfo.board_type_v2;
	else
		return sysinfo.board_type_v1;
}

/***************************************************************************
 *  get_gpmc0_base() - Return current address hardware will be
 *     fetching from. The below effectively gives what is correct, its a bit
 *   mis-leading compared to the TRM.  For the most general case the mask
 *   needs to be also taken into account this does work in practice.
 *   - for u-boot we currently map:
 *       -- 0 to nothing,
 *       -- 4 to flash
 *       -- 8 to enent
 *       -- c to wifi
 ****************************************************************************/
u32 get_gpmc0_base(void)
{
	u32 b;

	b = readl(&gpmc_cs_base->config7);
	b &= 0x1F;		/* keep base [5:0] */
	b = b << 24;		/* ret 0x0b000000 */
	return b;
}

/*******************************************************************
 * get_gpmc0_width() - See if bus is in x8 or x16 (mainly for nand)
 *******************************************************************/
u32 get_gpmc0_width(void)
{
	return WIDTH_16BIT;
}

/*************************************************************************
 * get_board_rev() - setup to pass kernel board revision information
 * returns:(bit[0-3] sub version, higher bit[7-4] is higher version)
 *************************************************************************/
u32 get_board_rev(void)
{
	return 0x20;
}

/*********************************************************************
 *  display_board_info() - print banner with board info.
 *********************************************************************/
void display_board_info(u32 btype)
{
	char *cpu_s, *mem_s, *sec_s;

	switch (get_cpu_type()) {
	case OMAP3503:
		cpu_s = "3503";
		break;
	case OMAP3515:
		cpu_s = "3515";
		break;
	case OMAP3525:
		cpu_s = "3525";
		break;
	case OMAP3530:
		cpu_s = "3530";
		break;
	default:
		cpu_s = "35XX";
		break;
	}

	if (is_mem_sdr())
		mem_s = "mSDR";
	else
		mem_s = "LPDDR";

	switch (get_device_type()) {
	case TST_DEVICE:
		sec_s = "TST";
		break;
	case EMU_DEVICE:
		sec_s = "EMU";
		break;
	case HS_DEVICE:
		sec_s = "HS";
		break;
	case GP_DEVICE:
		sec_s = "GP";
		break;
	default:
		sec_s = "?";
	}


	printf("OMAP%s-%s rev %d, CPU-OPP2 L3-165MHz\n", cpu_s,
	       sec_s, get_cpu_rev());
	printf("%s + %s/%s\n", sysinfo.board_string,
	       mem_s, sysinfo.nand_string);

}

/********************************************************
 *  get_base(); get upper addr of current execution
 *******************************************************/
u32 get_base(void)
{
	u32 val;

	__asm__ __volatile__("mov %0, pc \n":"=r"(val)::"memory");
	val &= 0xF0000000;
	val >>= 28;
	return val;
}

/********************************************************
 *  is_running_in_flash() - tell if currently running in
 *  FLASH.
 *******************************************************/
u32 is_running_in_flash(void)
{
	if (get_base() < 4)
		return 1;	/* in FLASH */

	return 0;		/* running in SRAM or SDRAM */
}

/********************************************************
 *  is_running_in_sram() - tell if currently running in
 *  SRAM.
 *******************************************************/
u32 is_running_in_sram(void)
{
	if (get_base() == 4)
		return 1;	/* in SRAM */

	return 0;		/* running in FLASH or SDRAM */
}

/********************************************************
 *  is_running_in_sdram() - tell if currently running in
 *  SDRAM.
 *******************************************************/
u32 is_running_in_sdram(void)
{
	if (get_base() > 4)
		return 1;	/* in SDRAM */

	return 0;		/* running in SRAM or FLASH */
}

/***************************************************************
 *  get_boot_type() - Is this an XIP type device or a stream one
 *  bits 4-0 specify type. Bit 5 says mem/perif
 ***************************************************************/
u32 get_boot_type(void)
{
	return (readl(&ctrl_base->status) & SYSBOOT_MASK);
}

/*************************************************************
 *  get_device_type(): tell if GP/HS/EMU/TST
 *************************************************************/
u32 get_device_type(void)
{
	return ((readl(&ctrl_base->status) & (DEVICE_MASK)) >> 8);
}
