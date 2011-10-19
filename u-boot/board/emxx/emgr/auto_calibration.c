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

unsigned long memc_softcal_stripe(unsigned long mem_size,
		unsigned long base_addr, unsigned long data_pattern, unsigned char window_range_spec);
void auto_calibration(void);
unsigned long memc_softcal_search(unsigned char window_range_spec,
		  		  unsigned long (*pass_wd)[64]);
unsigned long memc_softcal_stripe(unsigned long mem_size,
			  	  unsigned long base_addr,
				  unsigned long data_pattern,
				  unsigned char window_range_spec);
void memc_cal_stripe(	unsigned long mem_size,
			unsigned long base_addr,
			unsigned long data_pattern,
			unsigned long *flag_cal);


/*****************************************************************************
	DDR-SDRAM Soft Calibration
*****************************************************************************/
void auto_calibration(void)
{
	unsigned long  mem_size, base_addr, data_pattern;
	unsigned char window_range_spec;

// For 1Gb Mobile DDR-SDRAM
	window_range_spec = 6;
	mem_size	= 0x00000080;
	base_addr	= 0x33FFFF70;
	data_pattern 	= 0xAAAAAAAA;

	*(volatile unsigned long *)(0xC00A2024) = memc_softcal_stripe( mem_size, base_addr, data_pattern, window_range_spec );
}

/*****************************************************************************
	DDR-SDRAM Soft Calibration DQS_DELAY setting value search func.
*****************************************************************************/
unsigned long memc_softcal_search(unsigned char window_range_spec,
		  		  unsigned long (*pass_wd)[64])
{
	unsigned int 	pass_inc1[4],pass_inc2[4],pass_dec1[4],pass_dec2[4],
			pass_min[4],pass_max[4],pass_cent[4];

	unsigned long	dqs_delay_setting;
	unsigned char i,j;	// i,:DQS_DELAY j:Byte_num 

	for(j=0;j<0x4;j++) {
		pass_inc1[j] = 0;
		pass_inc2[j] = 0;
		pass_dec1[j] = 0;
		pass_dec2[j] = 0;
		pass_min[j]  = 0;
		pass_max[j]  = 0;
		pass_cent[j] = 0;
	}

	for(j=0;j<0x4;j++)
		for(i=0;i<0x40;i++)			
			if (pass_wd[j][i] != 0) {
				pass_inc1[j] = i;
				break;
			}
	for(j=0;j<0x4;j++)
		for(i=pass_inc1[j]+1;i<0x40;i++)
			if (pass_wd[j][i] != 0) {
				pass_inc2[j] = i;
				break;
			}

	for(j=0;j<0x4;j++)
		for(i=0x3F;i>0;i--)
			if (pass_wd[j][i] != 0) {
				pass_dec1[j] = i;
				break;
			}

	for(j=0;j<0x4;j++)
		for(i=pass_dec1[j]-1;i>0;i--)
			if (pass_wd[j][i] != 0) {
				pass_dec2[j] = i;
				break;
			}

	for(j=0;j<0x4;j++){
		if ((pass_dec1[j] - pass_dec2[j]) == 1) pass_max[j] = pass_dec1[j];
		else					pass_max[j] = pass_dec2[j];

		if ((pass_inc2[j] - pass_inc1[j]) == 1) pass_min[j] = pass_inc1[j];
		else					pass_min[j] = pass_inc2[j];
	}
	for(j=0;j<0x4;j++) {
		if ( (pass_max[j] - pass_min[j]) < window_range_spec) {
			dqs_delay_setting = 0x3F3F3F3F ;	// Error Code
			return (dqs_delay_setting) ;
		}
		pass_cent[j] = ( pass_max[j] + pass_min[j] ) / 2 ;
	}

#ifdef EM1_D512_CHIP
	pass_cent[0] = pass_cent[0] - 2 ;
	pass_cent[1] = pass_cent[1] - 2 ;
	pass_cent[2] = pass_cent[2] - 2 ;
	pass_cent[3] = pass_cent[3] - 2 ;
#else
	pass_cent[0] = pass_cent[0] - 2 ;
	pass_cent[1] = pass_cent[1] - 1 ;
	pass_cent[2] = pass_cent[2] - 1 ;
	pass_cent[3] = pass_cent[3] - 1 ;
#endif

	dqs_delay_setting = 0x00000000;
	dqs_delay_setting |= (pass_cent[0]<<24);
	dqs_delay_setting |= (pass_cent[1]<<16);
	dqs_delay_setting |= (pass_cent[2]<<8);
	dqs_delay_setting |= (pass_cent[3]);

	return(dqs_delay_setting);
}

/*****************************************************************************
	DDR-SDRAM Soft Calibration execute func. (Stripe Pattern)
*****************************************************************************/
unsigned long memc_softcal_stripe(unsigned long mem_size,
			  	  unsigned long base_addr,
				  unsigned long data_pattern,
				  unsigned char window_range_spec)
{
	unsigned long flag_cal[4],pass_wd[4][64],dqs_delay_setting;
	unsigned int i,j,h;	// i,h:DQS_DELAY j:Byte_num 

	for(j=0;j<4;j++)
		for(h=0;h<0x40;h++) pass_wd[j][h] = 0;	//Pass_window 配列初期化

	( *(volatile unsigned long *)0xC01101A4 ) &= ~(0x1<<4); //AHB-Macro-CLK(MEMC)自動CLK制御OFF(bit4=0x0)
	( *(volatile unsigned long *)0xC01101B0 ) &= ~(0x1); 	//MEMC_CLK270CLK-自動CLK制御OFF(bit1=0x0)

	for(i=0;i<0x40;i++){
		for(j=0;j<4;j++) flag_cal[j] = 0;	//PassFail Flag 配列初期化
		*(volatile unsigned long *)(0xC00A2024) = 0x00000000;
		*(volatile unsigned long *)(0xC00A2024) |= (i<<24);
		*(volatile unsigned long *)(0xC00A2024) |= (i<<16);
		*(volatile unsigned long *)(0xC00A2024) |= (i<<8);
		*(volatile unsigned long *)(0xC00A2024) |= i;
		memc_cal_stripe(mem_size,base_addr,data_pattern,&flag_cal[0]);

		for(j=0;j<0x4;j++){
			if (flag_cal[j] == 0) 	pass_wd[j][i] = i+1;
			else 			pass_wd[j][i] = 0;
		}	
	}

	dqs_delay_setting = memc_softcal_search( window_range_spec ,pass_wd ); 

	( *(volatile unsigned long *)0xC01101A4 ) |=  (0x1<<4); 	//AHB-Macro-CLK(MEMC)自動CLK制御ON(bit4=0x1)
	( *(volatile unsigned long *)0xC01101B0 ) |=  (0x1); 	//MEMC_CLK270CLK-自動CLK制御ON(bit0=0x1)

	return(dqs_delay_setting);
}

/**************************************************************************
	DDR-SDRAM Soft Calibration Pattern write/verify func. (Stripe)
**************************************************************************/
void memc_cal_stripe(	unsigned long mem_size,
			unsigned long base_addr,
			unsigned long data_pattern,
			unsigned long *flag_cal)
{
	unsigned long *addr;
	unsigned long i,read_data_true,read_data_bar;

	mem_size /= 4;

/** Stripe Pattern Write Seq. **/
	addr = (unsigned long *)(base_addr);
	for(i=0;i<mem_size;i=i+2,addr=addr+2) {
		*addr = data_pattern;
		*(addr+1) = ~data_pattern;
	}

/** Stripe Pattern Verify Seq. **/
	addr = (unsigned long *)(base_addr);
	for (i=0;i<mem_size;i=i+2,addr=addr+2) {
		read_data_true = *addr;		// True Data Read
		read_data_bar  = *(addr+1);	// Add inc & Bar Data Read


		if( (read_data_true & 0x000000FF) != (data_pattern & 0x000000FF) ) //data_true Byte0 verify
			flag_cal[0] = flag_cal[0] +1 ;
		if( (read_data_true & 0x0000FF00) != (data_pattern & 0x0000FF00) ) //data_true Byte1 verify
			flag_cal[1] = flag_cal[1] +1 ;
		if( (read_data_true & 0x00FF0000) != (data_pattern & 0x00FF0000) ) //data_true Byte2 verify
			flag_cal[2] = flag_cal[2] +1 ;
		if( (read_data_true & 0xFF000000) != (data_pattern & 0xFF000000) ) //data_true Byte3 verify
			flag_cal[3] = flag_cal[3] +1 ;

		if( (read_data_bar & 0x000000FF) != (~data_pattern & 0x000000FF) ) //data_bar Byte0 verify
			flag_cal[0] = flag_cal[0] +1 ;
		if( (read_data_bar & 0x0000FF00) != (~data_pattern & 0x0000FF00) ) //data_bar Byte1 verify
			flag_cal[1] = flag_cal[1] +1 ;
		if( (read_data_bar & 0x00FF0000) != (~data_pattern & 0x00FF0000) ) //data_bar Byte2 verify
			flag_cal[2] = flag_cal[2] +1 ;
		if( (read_data_bar & 0xFF000000) != (~data_pattern & 0xFF000000) ) //data_bar Byte3 verify
			flag_cal[3] = flag_cal[3] +1 ;
	}
}

