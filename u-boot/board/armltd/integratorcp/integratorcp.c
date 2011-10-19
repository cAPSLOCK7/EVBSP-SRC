/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * (C) Copyright 2003
 * Texas Instruments, <www.ti.com>
 * Kshitij Gupta <Kshitij@ti.com>
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <div64.h>



DECLARE_GLOBAL_DATA_PTR;

void flash__init (void);
void ether__init (void);
void peripheral_power_enable (void);

#if defined(CONFIG_SHOW_BOOT_PROGRESS)
void show_boot_progress(int progress)
{
	printf("Boot reached stage %d\n", progress);
}
#endif

#define COMP_MODE_ENABLE ((unsigned int)0x0000EAEF)

/*
 * Miscellaneous platform dependent initialisations
 */

int board_init (void)
{
	/* arch number of Integrator Board */
	gd->bd->bi_arch_number = MACH_TYPE_CINTEGRATOR;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x00000100;

	gd->flags = 0;

#ifdef CONFIG_CM_REMAP
extern void cm_remap(void);
	cm_remap();	/* remaps writeable memory to 0x00000000 */
#endif

	icache_enable ();

	flash__init ();
	ether__init ();
	return 0;
}


int misc_init_r (void)
{
	setenv("verify", "n");
	return (0);
}

/******************************
 Routine:
 Description: The Integrator/CP uses CFI flash.
 In order for the CFI commands to work we must ensure that
 the flash is accessible.
 Also sets up the number of banks for a later check against
 include/configs/integratorcp.h
******************************/
int nbanks	= 0xDEADBABE;
int sizeIndex	= 0xCAFEBABE;
void flash__init (void)
{
	vu_long *cpcr = (vu_long *)CPCR_BASE;

	/* Check if there is an extra bank of flash */
	if (cpcr[1] & CPMASK_EXTRABANK)
		nbanks = 2;
	else
		nbanks = 1;

	/* Check if there is an extra bank of flash */
	if (cpcr[1] & CPMASK_FLASHSIZE)
		sizeIndex = 1;
	else
		sizeIndex = 0;

	/* Enable flash write */
	cpcr[1] |= 3;
}
/******************************
 Routine:
 Description: Check Integrator/CP control
							register settings against
				configs header.
******************************/
void flash_check(void) {
	unsigned int sizes[] =	{0x01000000, 0x02000000};
	if(nbanks != CONFIG_SYS_MAX_FLASH_BANKS){
		printf(	"The board control register indicates %d flash banks, "
			"<U-Boot root>/include/configs/integratorcp.h::"
			"CONFIG_SYS_MAX_FLASH_BANKS is set to %d\n" \
			,nbanks, CONFIG_SYS_MAX_FLASH_BANKS);
	}

	if(PHYS_FLASH_SIZE != sizes[sizeIndex]){
		printf(	"The board control register indicates 0x%08x bytes of "
			"flash, <U-Boot root>/include/configs/integratorcp.h"
			"::PHYS_FLASH_SIZE is set to 0x%08x\n" \
			,sizes[sizeIndex], PHYS_FLASH_SIZE);
	}
}
/*************************************************************
 Routine:ether__init
 Description: take the Ethernet controller out of reset and wait
				for the EEPROM load to complete.
*************************************************************/
void ether__init (void)
{
}

/******************************
 Routine:
 Description:
******************************/
int dram_init (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size	 = PHYS_SDRAM_1_SIZE;

#ifdef CONFIG_CM_SPD_DETECT
		{
extern void dram_query(void);
	unsigned long cm_reg_sdram;
	unsigned long sdram_shift;

	dram_query();	/* Assembler accesses to CM registers */
			/* Queries the SPD values */

	/* Obtain the SDRAM size from the CM SDRAM register */

	cm_reg_sdram = *(volatile ulong *)(CM_BASE + OS_SDRAM);
	/*	 Register				SDRAM size
	 *
	 *	 0xXXXXXXbbb000bb	 16 MB
	 *	 0xXXXXXXbbb001bb	 32 MB
	 *	 0xXXXXXXbbb010bb	 64 MB
	 *	 0xXXXXXXbbb011bb	128 MB
	 *	 0xXXXXXXbbb100bb	256 MB
	 *
	 */
	sdram_shift		 = ((cm_reg_sdram & 0x0000001C)/4)%4;
	gd->bd->bi_dram[0].size	 = 0x01000000 << sdram_shift;

		}
#endif /* CM_SPD_DETECT */

	return 0;
}

/* The Integrator/CP timer1 is clocked at 1MHz
 * that clock can be divided by 16 or 256
 * Timer 1 can be set up as a 32-bit down counter
 */
/* U-Boot expects a 32 bit timer, running at CFG_HZ (which is NOT configurable but constant 1000 Hz) */
/* Although all these functions return ulong == 64 bit */
static unsigned long long total_count = 0;
static unsigned long long last_count;	 /* Timer reading at last call	   */
static unsigned long long div_clock = 1; /* Divisor applied to timer clock */
static unsigned long long div_timer = 1; /* Divisor to convert timer reading
					  * change to U-Boot ticks
					  */
/* CFG_HZ = CONFIG_SYS_HZ_CLOCK/(div_clock * div_timer) */
static ulong timestamp;		/* U-Boot ticks since startup */

#define TIMER_LOAD_VAL ((u32)0xFFFFFFFF)
#define READ_TIMER     (*(volatile u32 *)(CONFIG_SYS_TIMERBASE+4))

/* 
 * All functions return values in U-Boot ticks i.e. (1/CFG_HZ) sec
 * - unless otherwise stated
 */

/* starts up a counter
 * - the Integrator/CP timer can be set up to issue an interrupt 
 * - but it isn't here
 */
int interrupt_init (void)
{
	/* Load timer with initial value */
	*(volatile u32 *)(CONFIG_SYS_TIMERBASE + 0) = TIMER_LOAD_VAL;
	/* Set timer to be
	 *	enabled			1
	 *	periodic		1
	 *	no interrupts		0
	 *	X			0
	 *	clock divisor 1	 	00
	 *      (if this is changed then 
	 *       div_clock must be changed 
	 *       accordingly) 
	 *	32 bit			1
	 *	wrapping		0
	 */
	*(volatile u32 *)(CONFIG_SYS_TIMERBASE + 8) = 0x000000C2;
	div_clock = 1;
	
	/* init the timestamp */
	total_count = 0UL;
	reset_timer_masked();

	div_timer  = (CONFIG_SYS_HZ_CLOCK / CFG_HZ);
	div_timer /= div_clock;

	return (0);
}

/*
 * timer without interrupts
 */
void reset_timer (void)
{
	reset_timer_masked ();
}

/*
 * This function must return milliseconds
 * For Integrator/CP 1 tick == 1 millisecond
 */
ulong get_timer (ulong base_ticks)
{
	return get_timer_masked () - base_ticks;
}

void set_timer (ulong ticks)
{
	timestamp   = ticks;
	total_count = ticks * div_timer;
}

/* Return raw timer count for timing intervals
 * less than 1 tick e.g. usecs
 * Caller must test for wrap.....
 */
u32 get_timer_raw(void)
{
	return READ_TIMER;
}

/* delay usec useconds */
/* IntegratorCP uses a down counter */
/* The counter may wrap */
void udelay (unsigned long usec)
{
	u64 target, elapsed;
	u32 last_count = get_timer_raw();	/* get current timestamp */
	u32 now;
	u32 remainder;

	/* Convert usecs to timer frequency */
	target   = (u64)usec * (u64)CONFIG_SYS_HZ_CLOCK;

	// Use the kernel macro to avoid errors
	// __udivdi3 is NOT available
	// target /= (1000000L);
	// target /= div_clock;

	remainder = do_div(target, (u32)1000000);
     	remainder = do_div(target, div_clock);

	if(target){

		elapsed = 0;
		now = last_count = get_timer_raw();

		while (elapsed < target){
			if(now > last_count) {
				/* Must have wrapped */
				elapsed += last_count + TIMER_LOAD_VAL + 1 - now;
			} else {
				elapsed += last_count - now;
			}
			last_count = now;
			now = get_timer_raw();
		}
	} /* else usec too small.... */
}

void reset_timer_masked (void)
{
	/* capture current decrementer value */
	last_count		= (unsigned long long)READ_TIMER;
	/* start "advancing" time stamp from 0 */
	timestamp = 0L;
}

/* converts the timer reading to U-Boot ticks */
/* the timestamp is the number of ticks since reset */
/* returns ticks */
ulong get_timer_masked (void)
{
	/* get current count */
	unsigned long long now = (unsigned long long)READ_TIMER;

	if(now > last_count) {
		/* Must have wrapped */
		total_count += last_count + TIMER_LOAD_VAL + 1 - now;
	} else {
		total_count += last_count - now;
	}
	last_count	  = now;

	/* Reuse "now" */
	now = total_count;
	do_div(now, div_timer);
	timestamp = now;

	return timestamp;
}

/* Waits specified delay value and resets timestamp */
void udelay_masked (unsigned long usec)
{
	udelay(usec);
	reset_timer_masked();
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value since the timer decrements at CFG_HZ.
 */
unsigned long long get_ticks(void)
{
	return (unsigned long long)get_timer(0);
}

/*
 * Return the timebase clock frequency
 * i.e. how often the timer count decrements
 */
ulong get_tbclk (void)
{
	return CONFIG_SYS_HZ_CLOCK/div_clock;
}

/*
 *  u32 get_board_rev() for ARM supplied development boards
 */
ARM_SUPPLIED_GET_BOARD_REV

