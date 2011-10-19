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

#include "mini_config.h"
#include "serial.h"

/* SMU */
#define	SMU_S0_BASE			(0xe0110000)
#define SMU_USIAU0GCLKCTRL		(SMU_S0_BASE + 0x04A0)
#define SMU_USIAU0SCLKDIV		(SMU_S0_BASE + 0x061C)

#define SMU_USIAU0GCLKCTRL_SCLKDIS	~0x00000002
#define SMU_USIAU0GCLKCTRL_SCLKENA	0xffffffff
#define	SMU_USIAU0SCLKDIV_VAL		0x00000003	/* 229.376MHz(=PLL3(229.376MHz) * 1/ 4) */

#ifdef DEBUG
static void hex2char(unsigned long val);
#endif

void *
memcpy(void *dest, const void *src, int count)
{
	char *dest_tmp = (char *)dest;
	char *src_temp = (char *)src;

	while (count) {
		*dest_tmp++ = *src_temp++;
		count--;
	}

	return dest_tmp;
}

int
memcmp(const void *src1, const void *src2, int count)
{
	const unsigned char *src1_tmp = (const unsigned char *)src1;
	const unsigned char *src2_tmp = (const unsigned char *)src2;
	int ret = 0;

	while (count) {
		if ((ret = *src1_tmp++ - *src2_tmp++) != 0)
			break;
		count--;
	}

	return ret;
}

/* uart0 initial Function */
void
uart_init(void)
{
#ifdef DEBUG
#if !defined(CONFIG_EMXX_PALLADIUM)
	/* UART0 Setting */
	/* stop sclk */
	outl(SMU_USIAU0GCLKCTRL_SCLKDIS, SMU_USIAU0GCLKCTRL);
	/* SCLK=229.376M / 4, 115200bps, 8bit, nonparity */
	outl(SMU_USIAU0SCLKDIV_VAL, SMU_USIAU0SCLKDIV);
	/* start sclk */
	outl(SMU_USIAU0GCLKCTRL_SCLKENA, SMU_USIAU0GCLKCTRL);
#endif

	outl(UART_IER_CLR, UART0_IER );
	outl((UART_LCR_DLAB | UART_LCR_WLEN8), UART0_LCR );
	outl(UART_DLL_115200, UART0_DLL );
	outl(UART_DLM_115200, UART0_DLM );
	outl(UART_LCR_WLEN8, UART0_LCR );
	outl((UART_MCR_RTS | UART_MCR_DTR), UART0_MCR );
	outl(UART_FCR_FIFO_ENABLE, UART0_FCR );
	outl(UART_FCR_FIFO_RESET, UART0_FCR );
#endif
}

/* send character Function */
void
putc(unsigned char c)
{
#ifdef DEBUG
	if (c == '\n') {
		/* Wait until there is space in the FIFO */
		while ((inl(UART0_LSR) & UART_LSR_THRE) == 0) {
			;
		}
		outb('\r', UART0_THR);
	}

	/* Wait until there is space in the FIFO */
	while ((inl(UART0_LSR) & UART_LSR_THRE) == 0) {
		;
	}
	/* Send the character */
	outb(c, UART0_THR);
#endif
}

/* send stream Function */
void
dbg_print(const char* s)
{
#ifdef DEBUG
	for ( ; *s != '\0'; s++) {
		if (*s == '\n') {
			putc('\r');
			putc('\n');
		} else {
			putc(*s);
		}
	}
#endif
}

void
dbg_print_val(const char* s, unsigned long num)
{
#ifdef DEBUG
	for ( ; *s != '\0'; s++) {
		if (*s == '\n') {
			putc('\r');
			putc('\n');
		} else if (*s == '%') {
			s++;
			if (*s == 'c') {
				putc(*(char *)num);
			} else if (*s == 's') {
				dbg_print((char *)num);
			} else if (*s == 'x') {
				hex2char(num);
			} else {
				putc('%');
				putc(*s);
			}
		} else {
			putc(*s);
		}
    }
#endif
}

#ifdef DEBUG
static void
hex2char( unsigned long val )
{
	char			buf[10];
	char			*tmp_ptr = buf;
	int				i=1;
	unsigned int	tmp, uint16=val;
	
	while( 1 ) {
		uint16 >>= 4;
		if( uint16 == 0 ) {
			break;
		}
		i++;
	}
	*(tmp_ptr + i) = '\0';
	
	tmp_ptr += (i - 1);
	uint16 = val;
	while( i ) {
		tmp = uint16 & 0x0000000F;
		if( tmp < 10 ) {
			*tmp_ptr-- = (char)('0' + tmp);
		} else {
			*tmp_ptr-- = (char)('a' + tmp - 10);
		}
		uint16 >>= 4;
		i--;
	}
	dbg_print(buf);
}
#endif
