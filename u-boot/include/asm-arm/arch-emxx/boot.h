/*
 * Copyright (C) 2007,2010 Renesas Electronics Corporation
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

#ifndef _BOOT_H
#define _BOOT_H

#ifdef __ASSEMBLER__


#define	SET_BASEREG(base_reg, VALUE)	\
        ldr     base_reg, =VALUE

#define	STORE_WORD(base_reg, OFFSET, VALUE, work_reg)	\
        ldr     work_reg, =VALUE;		\
        str     work_reg, [base_reg, #((OFFSET) & 0x00fff)];


#define	STORE_HALF_WORD(base_reg, OFFSET, VALUE, work_reg)	\
        MOV     work_reg, #((VALUE) & 0xff00);		\
	.ifne ((VALUE) & 0x00ff); \
        ADD     work_reg, work_reg, #((VALUE) & 0x00ff);	\
	.endif; \
        STRH     work_reg, [base_reg, #((OFFSET) & 0x00fff)];

#define	LOAD_WORD(base_reg, OFFSET, work_reg)	\
        LDR     work_reg, [base_reg, #((OFFSET) & 0x0fff)];

#define	LOAD_HALF_WORD(base_reg, OFFSET, work_reg)	\
        LDRH     work_reg, [base_reg, #((OFFSET) & 0x0fff)];

#else /* __ASSEMBLER__ */
#endif /* __ASSEMBLER__ */


#endif /* _BOOT_H */
