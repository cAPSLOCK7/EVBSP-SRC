/*
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

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_CMD_SAVEENV) && defined(CONFIG_CMD_FLASH)
#define CMD_SAVEENV
#elif defined(CONFIG_ENV_ADDR_REDUND)
#error Cannot use CONFIG_ENV_ADDR_REDUND without CONFIG_CMD_SAVEENV & CONFIG_CMD_FLASH
#endif

#if defined(CONFIG_ENV_SIZE_REDUND) && (CONFIG_ENV_SIZE_REDUND < CONFIG_ENV_SIZE)
#error CONFIG_ENV_SIZE_REDUND should not be less then CONFIG_ENV_SIZE
#endif

#ifdef CONFIG_INFERNO
# ifdef CONFIG_ENV_ADDR_REDUND
#error CONFIG_ENV_ADDR_REDUND is not implemented for CONFIG_INFERNO
# endif
#endif

char * env_name_spec = "Flash";

#ifdef ENV_IS_EMBEDDED

extern uchar environment[];
env_t *env_ptr = (env_t *)(&environment[0]);

#ifdef CMD_SAVEENV
/* static env_t *flash_addr = (env_t *)(&environment[0]);-broken on ARM-wd-*/
static env_t *flash_addr = (env_t *)CONFIG_ENV_ADDR;
#endif

#else /* ! ENV_IS_EMBEDDED */

# ifdef CONFIG_REALVIEW
env_t *env_ptr;
#  ifdef CMD_SAVEENV
static env_t *flash_addr;
#  endif
# else
env_t *env_ptr = (env_t *)CONFIG_ENV_ADDR;
#  ifdef CMD_SAVEENV
static env_t *flash_addr = (env_t *)CONFIG_ENV_ADDR;
#  endif
# endif

#endif /* ENV_IS_EMBEDDED */

#ifdef CONFIG_ENV_ADDR_REDUND
static env_t *flash_addr_new = (env_t *)CONFIG_ENV_ADDR_REDUND;

/* CONFIG_ENV_ADDR is supposed to be on sector boundary */
static ulong end_addr = CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE - 1;
static ulong end_addr_new = CONFIG_ENV_ADDR_REDUND + CONFIG_ENV_SECT_SIZE - 1;

#define ACTIVE_FLAG   1
#define OBSOLETE_FLAG 0
#endif /* CONFIG_ENV_ADDR_REDUND */

extern uchar default_environment[];
extern int default_environment_size;


uchar env_get_char_spec (int index)
{
	return ( *((uchar *)(gd->env_addr + index)) );
}

#ifdef CONFIG_ENV_ADDR_REDUND

int  env_init(void)
{
	int crc1_ok = 0, crc2_ok = 0;

	uchar flag1 = flash_addr->flags;
	uchar flag2 = flash_addr_new->flags;

	ulong addr_default = (ulong)&default_environment[0];
	ulong addr1 = (ulong)&(flash_addr->data);
	ulong addr2 = (ulong)&(flash_addr_new->data);

	crc1_ok = (crc32(0, flash_addr->data, ENV_SIZE) == flash_addr->crc);
	crc2_ok = (crc32(0, flash_addr_new->data, ENV_SIZE) == flash_addr_new->crc);

	if (crc1_ok && ! crc2_ok) {
		gd->env_addr  = addr1;
		gd->env_valid = 1;
	} else if (! crc1_ok && crc2_ok) {
		gd->env_addr  = addr2;
		gd->env_valid = 1;
	} else if (! crc1_ok && ! crc2_ok) {
		gd->env_addr  = addr_default;
		gd->env_valid = 0;
	} else if (flag1 == ACTIVE_FLAG && flag2 == OBSOLETE_FLAG) {
		gd->env_addr  = addr1;
		gd->env_valid = 1;
	} else if (flag1 == OBSOLETE_FLAG && flag2 == ACTIVE_FLAG) {
		gd->env_addr  = addr2;
		gd->env_valid = 1;
	} else if (flag1 == flag2) {
		gd->env_addr  = addr1;
		gd->env_valid = 2;
	} else if (flag1 == 0xFF) {
		gd->env_addr  = addr1;
		gd->env_valid = 2;
	} else if (flag2 == 0xFF) {
		gd->env_addr  = addr2;
		gd->env_valid = 2;
	}

	return (0);
}

#ifdef CMD_SAVEENV
int saveenv(void)
{
	char *saved_data = NULL;
	int rc = 1;
	char flag = OBSOLETE_FLAG, new_flag = ACTIVE_FLAG;
#if CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE
	ulong up_data = 0;
#endif

	debug ("Protect off %08lX ... %08lX\n",
		(ulong)flash_addr, end_addr);

	if (flash_sect_protect (0, (ulong)flash_addr, end_addr)) {
		goto Done;
	}

	debug ("Protect off %08lX ... %08lX\n",
		(ulong)flash_addr_new, end_addr_new);

	if (flash_sect_protect (0, (ulong)flash_addr_new, end_addr_new)) {
		goto Done;
	}

#if CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE
	up_data = (end_addr_new + 1 - ((long)flash_addr_new + CONFIG_ENV_SIZE));
	debug ("Data to save 0x%x\n", up_data);
	if (up_data) {
		if ((saved_data = malloc(up_data)) == NULL) {
			printf("Unable to save the rest of sector (%ld)\n",
				up_data);
			goto Done;
		}
		memcpy(saved_data,
			(void *)((long)flash_addr_new + CONFIG_ENV_SIZE), up_data);
		debug ("Data (start 0x%x, len 0x%x) saved at 0x%x\n",
			   (long)flash_addr_new + CONFIG_ENV_SIZE,
				up_data, saved_data);
	}
#endif
	puts ("Erasing Flash...");
	debug (" %08lX ... %08lX ...",
		(ulong)flash_addr_new, end_addr_new);

	if (flash_sect_erase ((ulong)flash_addr_new, end_addr_new)) {
		goto Done;
	}

	puts ("Writing to Flash... ");
	debug (" %08lX ... %08lX ...",
		(ulong)&(flash_addr_new->data),
		sizeof(env_ptr->data)+(ulong)&(flash_addr_new->data));
	if ((rc = flash_write((char *)env_ptr->data,
			(ulong)&(flash_addr_new->data),
			sizeof(env_ptr->data))) ||
	    (rc = flash_write((char *)&(env_ptr->crc),
			(ulong)&(flash_addr_new->crc),
			sizeof(env_ptr->crc))) ||
	    (rc = flash_write(&flag,
			(ulong)&(flash_addr->flags),
			sizeof(flash_addr->flags))) ||
	    (rc = flash_write(&new_flag,
			(ulong)&(flash_addr_new->flags),
			sizeof(flash_addr_new->flags))))
	{
		flash_perror (rc);
		goto Done;
	}
	puts ("done\n");

#if CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE
	if (up_data) { /* restore the rest of sector */
		debug ("Restoring the rest of data to 0x%x len 0x%x\n",
			   (long)flash_addr_new + CONFIG_ENV_SIZE, up_data);
		if (flash_write(saved_data,
				(long)flash_addr_new + CONFIG_ENV_SIZE,
				up_data)) {
			flash_perror(rc);
			goto Done;
		}
	}
#endif
	{
		env_t * etmp = flash_addr;
		ulong ltmp = end_addr;

		flash_addr = flash_addr_new;
		flash_addr_new = etmp;

		end_addr = end_addr_new;
		end_addr_new = ltmp;
	}

	rc = 0;
Done:

	if (saved_data)
		free (saved_data);
	/* try to re-protect */
	(void) flash_sect_protect (1, (ulong)flash_addr, end_addr);
	(void) flash_sect_protect (1, (ulong)flash_addr_new, end_addr_new);

	return rc;
}
#endif /* CMD_SAVEENV */

#else /* ! CONFIG_ENV_ADDR_REDUND */

int  env_init(void)
{
#ifdef CONFIG_REALVIEW
	env_ptr = (env_t *)(gd->bd->flash_base + CONFIG_ENV_OFFSET);

# ifdef CMD_SAVEENV
	flash_addr = (env_t *)(gd->bd->flash_base + CONFIG_ENV_OFFSET);
# endif
#endif
	if (crc32(0, env_ptr->data, ENV_SIZE) == env_ptr->crc) {
		gd->env_addr  = (ulong)&(env_ptr->data);
		gd->env_valid = 1;
		return(0);
	}

	gd->env_addr  = (ulong)&default_environment[0];
	gd->env_valid = 0;
	return (0);
}

#ifdef CMD_SAVEENV

#ifdef __ARMSUPPLIED_

#define BM_FOOTER_SIZE 140
/* On boards with the small flash sectors at the base of flash we
 * need to decrement the block number by one in the Boot Monitor footer
 */
#ifdef CONFIG_REALVIEW_PB1176
#define BM_SECTOR_NUM (CONFIG_ENV_OFFSET / CONFIG_ENV_SECT_SIZE) -1
#else
#define BM_SECTOR_NUM CONFIG_ENV_OFFSET / CONFIG_ENV_SECT_SIZE
#endif

static ulong flash_checksum(ulong data, int size)
{
	ulong checksum = 0;
	ulong *ptr;
	ulong word;

	if(data%sizeof(ulong)) {
		printf("flash_checksum() passed misaligned pointer value\n");
	} else {
		ptr = (ulong*)data;
	
		while (size > 0) {
			word = *ptr++;
			if (word > ~checksum)
				checksum++;
			checksum += word;
			size -= sizeof(ulong);
		}
	}
	return ~checksum;
}
#endif	/* __ARMSUPPLIED_ */

int saveenv(void)
{
	int	len, rc;
	ulong	end_addr;
	ulong	flash_sect_addr;
#if defined(CONFIG_ENV_SECT_SIZE) && (CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE)
	ulong	flash_offset;
	uchar	env_buffer[CONFIG_ENV_SECT_SIZE];
#else
	uchar *env_buffer = (uchar *)env_ptr;
#endif	/* CONFIG_ENV_SECT_SIZE */
	int rcode = 0;

#if defined(CONFIG_ENV_SECT_SIZE) && (CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE)

	flash_offset    = ((ulong)flash_addr) & (CONFIG_ENV_SECT_SIZE-1);
	flash_sect_addr = ((ulong)flash_addr) & ~(CONFIG_ENV_SECT_SIZE-1);

	debug ( "copy old content: "
		"sect_addr: %08lX  env_addr: %08lX  offset: %08lX\n",
		flash_sect_addr, (ulong)flash_addr, flash_offset);

	/* copy old contents to temporary buffer */
	memcpy (env_buffer, (void *)flash_sect_addr, CONFIG_ENV_SECT_SIZE);

	/* copy current environment to temporary buffer */
	memcpy ((uchar *)((unsigned long)env_buffer + flash_offset),
		env_ptr,
		CONFIG_ENV_SIZE);

#ifdef __ARMSUPPLIED_
	{
	/* 
	 * Write an ARM Boot Monitor tag to the end of the sector. 
	 * This will make the environment sector visible to other
	 * ARM tools and prevent it being overwritten
	 */
	ulong  *env_long = (ulong *)env_buffer;
	memset ((uchar *)((unsigned long)env_buffer + CONFIG_ENV_SECT_SIZE - BM_FOOTER_SIZE), 0, BM_FOOTER_SIZE);

	env_long[CONFIG_ENV_SECT_SIZE/sizeof(ulong) - 33] = 1;		/* Attrib    */
	env_long[CONFIG_ENV_SECT_SIZE/sizeof(ulong) - 32] = 1;		/* # Regions */
	env_long[CONFIG_ENV_SECT_SIZE/sizeof(ulong) - 30] = CONFIG_ENV_SIZE; /* R 1 Size  */
	env_long[CONFIG_ENV_SECT_SIZE/sizeof(ulong) - 15] = BM_SECTOR_NUM;	/* Blk Start */
	env_long[CONFIG_ENV_SECT_SIZE/sizeof(ulong) - 14] = BM_SECTOR_NUM;	/* Blk End   */
	env_long[CONFIG_ENV_SECT_SIZE/sizeof(ulong) - 12] = 0x6F422D55;	/* "U-Bo"    */
	env_long[CONFIG_ENV_SECT_SIZE/sizeof(ulong) - 11] = 0x452D746F;	/* "ot-E"    */
	env_long[CONFIG_ENV_SECT_SIZE/sizeof(ulong) - 10] = 0x7269766E;	/* "nvir"    */
	env_long[CONFIG_ENV_SECT_SIZE/sizeof(ulong) - 9]  = 0x656D6E6F;	/* "onme"    */
	env_long[CONFIG_ENV_SECT_SIZE/sizeof(ulong) - 8]  = 0x0000746E;	/* "nt  "    */
	env_long[CONFIG_ENV_SECT_SIZE/sizeof(ulong) - 4]  = 0x5C;		/* Offset    */
	env_long[CONFIG_ENV_SECT_SIZE/sizeof(ulong) - 3]  = 1;		/* Version   */
	env_long[CONFIG_ENV_SECT_SIZE/sizeof(ulong) - 2]  = 0x464C5348;	/* "FLSH"    */
	env_long[CONFIG_ENV_SECT_SIZE/sizeof(ulong) - 1]  = 0x464F4F54;	/* "FOOT"    */


	env_long[CONFIG_ENV_SECT_SIZE/sizeof(ulong) - 28] = flash_checksum((ulong)env_long, CONFIG_ENV_SIZE);

	env_long[CONFIG_ENV_SECT_SIZE/sizeof(ulong) - 13] = flash_checksum((ulong)env_buffer +
					CONFIG_ENV_SECT_SIZE - BM_FOOTER_SIZE,
				 	BM_FOOTER_SIZE);
	}
#endif

	len	 = CONFIG_ENV_SECT_SIZE;
#else
	flash_sect_addr = (ulong)flash_addr;
	len	 = CONFIG_ENV_SIZE;
#endif	/* CONFIG_ENV_SECT_SIZE */

#ifndef CONFIG_INFERNO
	end_addr = flash_sect_addr + len - 1;
#else
	/* this is the last sector, and the size is hardcoded here */
	/* otherwise we will get stack problems on loading 128 KB environment */
	end_addr = flash_sect_addr + 0x20000 - 1;
#endif

	debug ("Protect off %08lX ... %08lX\n",
		(ulong)flash_sect_addr, end_addr);

	if (flash_sect_protect (0, flash_sect_addr, end_addr))
		return 1;

	puts ("Erasing Flash...");
	if (flash_sect_erase (flash_sect_addr, end_addr))
		return 1;

	puts ("Writing to Flash... ");
	rc = flash_write((char *)env_buffer, flash_sect_addr, len);
	if (rc != 0) {
		flash_perror (rc);
		rcode = 1;
	} else {
		puts ("done\n");
	}

	/* try to re-protect */
	(void) flash_sect_protect (1, flash_sect_addr, end_addr);
	return rcode;
}

#endif /* CMD_SAVEENV */

#endif /* CONFIG_ENV_ADDR_REDUND */

void env_relocate_spec (void)
{
#if !defined(ENV_IS_EMBEDDED) || defined(CONFIG_ENV_ADDR_REDUND)
#ifdef CONFIG_ENV_ADDR_REDUND
	if (gd->env_addr != (ulong)&(flash_addr->data)) {
		env_t * etmp = flash_addr;
		ulong ltmp = end_addr;

		flash_addr = flash_addr_new;
		flash_addr_new = etmp;

		end_addr = end_addr_new;
		end_addr_new = ltmp;
	}

	if (flash_addr_new->flags != OBSOLETE_FLAG &&
	    crc32(0, flash_addr_new->data, ENV_SIZE) ==
	    flash_addr_new->crc) {
		char flag = OBSOLETE_FLAG;

		gd->env_valid = 2;
		flash_sect_protect (0, (ulong)flash_addr_new, end_addr_new);
		flash_write(&flag,
			    (ulong)&(flash_addr_new->flags),
			    sizeof(flash_addr_new->flags));
		flash_sect_protect (1, (ulong)flash_addr_new, end_addr_new);
	}

	if (flash_addr->flags != ACTIVE_FLAG &&
	    (flash_addr->flags & ACTIVE_FLAG) == ACTIVE_FLAG) {
		char flag = ACTIVE_FLAG;

		gd->env_valid = 2;
		flash_sect_protect (0, (ulong)flash_addr, end_addr);
		flash_write(&flag,
			    (ulong)&(flash_addr->flags),
			    sizeof(flash_addr->flags));
		flash_sect_protect (1, (ulong)flash_addr, end_addr);
	}

	if (gd->env_valid == 2)
		puts ("*** Warning - some problems detected "
		      "reading environment; recovered successfully\n\n");
#endif /* CONFIG_ENV_ADDR_REDUND */
#ifdef CMD_SAVEENV
	memcpy (env_ptr, (void*)flash_addr, CONFIG_ENV_SIZE);
#endif
#endif /* ! ENV_IS_EMBEDDED || CONFIG_ENV_ADDR_REDUND */
}
