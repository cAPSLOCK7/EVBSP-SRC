/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_	1

/**************************************************************************
 *
 * The "environment" is stored as a list of '\0' terminated
 * "name=value" strings. The end of the list is marked by a double
 * '\0'. New entries are always added at the end. Deleting an entry
 * shifts the remaining entries to the front. Replacing an entry is a
 * combination of deleting the old value and adding the new one.
 *
 * The environment is preceeded by a 32 bit CRC over the data part.
 *
 **************************************************************************
 */

#if defined(CONFIG_ENV_IS_IN_FLASH)
# ifndef  CONFIG_ENV_ADDR
#  define CONFIG_ENV_ADDR	(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
# endif
# ifndef  CONFIG_ENV_OFFSET
#  define CONFIG_ENV_OFFSET (CONFIG_ENV_ADDR - CONFIG_SYS_FLASH_BASE)
# endif
# if !defined(CONFIG_ENV_ADDR_REDUND) && defined(CONFIG_ENV_OFFSET_REDUND)
#  define CONFIG_ENV_ADDR_REDUND	(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET_REDUND)
# endif
# if defined(CONFIG_ENV_SECT_SIZE) || defined(CONFIG_ENV_SIZE)
#  ifndef  CONFIG_ENV_SECT_SIZE
#   define CONFIG_ENV_SECT_SIZE	CONFIG_ENV_SIZE
#  endif
#  ifndef  CONFIG_ENV_SIZE
#   define CONFIG_ENV_SIZE	CONFIG_ENV_SECT_SIZE
#  endif
# else
#  error "Both CONFIG_ENV_SECT_SIZE and CONFIG_ENV_SIZE undefined"
# endif
# if defined(CONFIG_ENV_ADDR_REDUND) && !defined(CONFIG_ENV_SIZE_REDUND)
#  define CONFIG_ENV_SIZE_REDUND	CONFIG_ENV_SIZE
# endif
# ifdef CONFIG_REALVIEW
	/* ENV_IS_EMBEDDED must be defined in realview.h if necessary */
# else
#  if (CONFIG_ENV_ADDR >= CONFIG_SYS_MONITOR_BASE) && \
      (CONFIG_ENV_ADDR+CONFIG_ENV_SIZE) <= (CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)
#   define ENV_IS_EMBEDDED	1
#  endif
# endif
# if defined(CONFIG_ENV_ADDR_REDUND) || defined(CONFIG_ENV_OFFSET_REDUND)
#  define CONFIG_SYS_REDUNDAND_ENVIRONMENT	1
# endif
#endif	/* CONFIG_ENV_IS_IN_FLASH */

#if defined(CONFIG_ENV_IS_IN_NAND)
# ifndef CONFIG_ENV_OFFSET
#  error "Need to define CONFIG_ENV_OFFSET when using CONFIG_ENV_IS_IN_NAND"
# endif
# ifndef CONFIG_ENV_SIZE
#  error "Need to define CONFIG_ENV_SIZE when using CONFIG_ENV_IS_IN_NAND"
# endif
# ifdef CONFIG_ENV_OFFSET_REDUND
#  define CONFIG_SYS_REDUNDAND_ENVIRONMENT
# endif
# ifdef CONFIG_ENV_IS_EMBEDDED
#  define ENV_IS_EMBEDDED	1
# endif
#endif /* CONFIG_ENV_IS_IN_NAND */

#ifdef USE_HOSTCC
# include <stdint.h>
#else
# include <linux/types.h>
#endif

#ifdef CONFIG_SYS_REDUNDAND_ENVIRONMENT
# define ENV_HEADER_SIZE	(sizeof(uint32_t) + 1)
#else
# define ENV_HEADER_SIZE	(sizeof(uint32_t))
#endif


#define ENV_SIZE (CONFIG_ENV_SIZE - ENV_HEADER_SIZE)

typedef	struct environment_s {
	uint32_t	crc;		/* CRC32 over data bytes	*/
#ifdef CONFIG_SYS_REDUNDAND_ENVIRONMENT
	unsigned char	flags;		/* active/obsolete flags	*/
#endif
	unsigned char	data[ENV_SIZE]; /* Environment data		*/
} env_t;

/* Function that returns a character from the environment */
unsigned char env_get_char (int);

/* Function that returns a pointer to a value from the environment */
unsigned char *env_get_addr(int);
unsigned char env_get_char_memory (int index);

/* Function that updates CRC of the enironment */
void env_crc_update (void);

/* [re]set to the default environment */
void set_default_env(void);

#endif	/* _ENVIRONMENT_H_ */
