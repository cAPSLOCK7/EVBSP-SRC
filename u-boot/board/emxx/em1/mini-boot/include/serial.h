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

#ifndef __SERIAL_H
#define __SERIAL_H

/* UART register */
#define EM1_UART0_BASE			0x50000000

#define UART0_THR			(EM1_UART0_BASE + 0x00)
#define UART0_IER			(EM1_UART0_BASE + 0x04)
#define UART0_FCR			(EM1_UART0_BASE + 0x0c)
#define UART0_LCR			(EM1_UART0_BASE + 0x10)
#define UART0_MCR			(EM1_UART0_BASE + 0x14)
#define UART0_LSR			(EM1_UART0_BASE + 0x18)
#define UART0_DLL			(EM1_UART0_BASE + 0x24)
#define UART0_DLM			(EM1_UART0_BASE + 0x28)

/* baudrate 115200bps for U70SCLK=229.376MHz */
#define UART_DLL_115200			0x1F
#define UART_DLM_115200			0x00

#define UART_LCR_DLAB			0x80
#define UART_LCR_WLEN8			0x03

#define UART_MCR_RTS			0x02
#define UART_MCR_DTR			0x01

#define UART_FCR_FIFO_ENABLE		0x01
#define UART_FCR_FIFO_RESET		0x07

#define UART_IER_CLR			0x00

/* Tx Buffer empty */
#define UART_LSR_THRE			0x20

#endif /* __SERIAL_H */
