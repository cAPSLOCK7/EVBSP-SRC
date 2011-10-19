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

#ifndef _EMEV_SYS_H_
#define _EMEV_SYS_H_

#define SRAM_START		0xf0000000
#define SRAM_END		0xf0020000
#define SRAM_STACK_BASE		(SRAM_END - 0x104)

#undef TCLR
#undef TLDR
#undef TCRR
#define TCLR			0x00	/* Timer OP  offset */
#define TLDR			0x08	/* Timer SET offset */
#define TCRR			0x0c	/* Timer RCR offset */

#endif /* _EMEV_SYS_H_ */
