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

#ifndef __ETC_H
#define __ETC_H

#ifndef __ASSEMBLY__

extern void *memcpy(void *, const void *, int);
extern int memcmp(const void *, const void *, int);
extern void udelay(unsigned int);
extern void uart_init(void);
extern void putc(unsigned char);
extern void dbg_print(const char*);
extern void dbg_print_val(const char*, unsigned long);
#endif

#endif /* __ETC_H */
