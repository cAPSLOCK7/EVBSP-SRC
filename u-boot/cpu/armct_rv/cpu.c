/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
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

/*
 * CPU specific code for a core tile with an unknown cpu
 * - hence fairly empty......
 */

#include <common.h>
#include <command.h>

void icache_flush(void);

#if defined(CONFIG_USE_IRQ) || defined (CONFIG_REALVIEW)
DECLARE_GLOBAL_DATA_PTR;
#endif

int cpu_init (void)
{
	/*
	 * setup up stacks if necessary
	 */
#ifdef CONFIG_USE_IRQ
	IRQ_STACK_START = _armboot_start - CONFIG_SYS_MALLOC_LEN - CONFIG_SYS_GBL_DATA_SIZE - 4;
	FIQ_STACK_START = IRQ_STACK_START - CONFIG_STACKSIZE_IRQ;
#endif
	return 0;
}

/*
 * This function is called just before we call linux
 * it prepares the processor for linux
 *
 * Turn off caches etc ...
 */
int cleanup_before_linux (void)
{
	disable_interrupts ();
	icache_flush();

	return (0);
}

int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	extern void reset_cpu (ulong addr);

	disable_interrupts ();
	reset_cpu (0);
	/*NOTREACHED*/
	return (0);
}

/* May not be cached processor on the CT - do nothing */
void icache_enable (void)
{
}

void icache_disable (void)
{
}

/* return "disabled" */
int icache_status (void)
{
	return 0;
}

/*
 * The following functions are defined so that the code in do_bootelf
 * causes the correct cleanup before we run an ELF image.
 */
/* return "enabled" */
int dcache_status(void)
{
	return 1;

}
void dcache_enable(void)
{
}

void dcache_disable(void)
{
	cleanup_before_linux();
}

void icache_flush(void)
{
#ifdef CONFIG_REALVIEW
	gd->bd->icache_flush_arch();
#else
#endif
}

void icache_flush_v67(void)
{
#if defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_7A__)
	unsigned long i = 0;

	/* flush I-cache */
	icache_disable();
	asm ("mcr p15, 0, %0, c7, c5, 0": :"r" (i));  /* invalidate whole I cache */
	asm ("mcr p15, 0, %0, c7, c5, 6": :"r" (i));  /* flush btb */
	asm ("mcr p15, 0, %0, c7, c5, 4": :"r" (i));  /* ISB - deprecated but OK in V7 */
#endif
}

void icache_flush_v5t(void)
{
	printf("icache_flush_v5t() not implemented\n");
}


