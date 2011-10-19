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
