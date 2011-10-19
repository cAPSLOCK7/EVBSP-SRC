/*
 * (C) Copyright 2001
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef __ASSEMBLY__
#define	__ASSEMBLY__			/* Dirty trick to get only #defines	*/
#endif
#define	__ASM_STUB_PROCESSOR_H__	/* don't include asm/processor.		*/
#include <config.h>
#undef	__ASSEMBLY__

#if defined(CONFIG_ENV_IS_IN_FLASH)
# ifndef CONFIG_REALVIEW
#  ifndef  CONFIG_ENV_ADDR
#   define CONFIG_ENV_ADDR	(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
#  endif
#  ifndef  CONFIG_ENV_OFFSET
#   define CONFIG_ENV_OFFSET (CONFIG_ENV_ADDR - CONFIG_SYS_FLASH_BASE)
#  endif
#  if !defined(CONFIG_ENV_ADDR_REDUND) && defined(CONFIG_ENV_OFFSET_REDUND)
#   define CONFIG_ENV_ADDR_REDUND	(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET_REDUND)
#  endif
# endif	 /* CONFIG_SYS_FLASH_BASE not known at compile time for realview boards */

# ifndef  CONFIG_ENV_SIZE
#  define CONFIG_ENV_SIZE	CONFIG_ENV_SECT_SIZE
# endif
# if defined(CONFIG_ENV_ADDR_REDUND) && !defined(CONFIG_ENV_SIZE_REDUND)
#  define CONFIG_ENV_SIZE_REDUND	CONFIG_ENV_SIZE
# endif
# ifndef CONFIG_REALVIEW
#  if (CONFIG_ENV_ADDR >= CONFIG_SYS_MONITOR_BASE) && \
      ((CONFIG_ENV_ADDR + CONFIG_ENV_SIZE) <= (CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN))
#   define ENV_IS_EMBEDDED	1
#  endif
# else
	/* ENV_IS_EMBEDDED must be defined in realview.h if necessary */
# endif
# if defined(CONFIG_ENV_ADDR_REDUND) || defined(CONFIG_ENV_OFFSET_REDUND)
#  define CONFIG_SYS_REDUNDAND_ENVIRONMENT	1
# endif
#endif	/* CONFIG_ENV_IS_IN_FLASH */

#ifdef CONFIG_SYS_REDUNDAND_ENVIRONMENT
# define ENV_HEADER_SIZE	(sizeof(uint32_t) + 1)
#else
# define ENV_HEADER_SIZE	(sizeof(uint32_t))
#endif

#define ENV_SIZE (CONFIG_ENV_SIZE - ENV_HEADER_SIZE)


extern uint32_t crc32 (uint32_t, const unsigned char *, unsigned int);

#ifdef	ENV_IS_EMBEDDED
extern unsigned int env_size;
extern unsigned char environment;
#endif	/* ENV_IS_EMBEDDED */

int main (int argc, char **argv)
{
#ifdef	ENV_IS_EMBEDDED
	uint32_t crc;
	unsigned char *envptr = &environment,
		*dataptr = envptr + ENV_HEADER_SIZE;
	unsigned int datasize = ENV_SIZE;

	crc = crc32 (0, dataptr, datasize);

	/* Check if verbose mode is activated passing a parameter to the program */
	if (argc > 1) {
		printf ("CRC32 from offset %08X to %08X of environment = %08X\n",
			(unsigned int) (dataptr - envptr),
			(unsigned int) (dataptr - envptr) + datasize,
			crc);
	} else {
		printf ("0x%08X\n", crc);
	}
#else
	printf ("0\n");
#endif
	return EXIT_SUCCESS;
}
