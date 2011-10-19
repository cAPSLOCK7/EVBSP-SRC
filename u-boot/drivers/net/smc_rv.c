/*------------------------------------------------------------------------
 . smc_rv.c	This is a driver for the RealView boards ethernet chip(s)
 .
 . This program is free software; you can redistribute it and/or modify
 . it under the terms of the GNU General Public License as published by
 . the Free Software Foundation; either version 2 of the License, or
 . (at your option) any later version.
 .
 . This program is distributed in the hope that it will be useful,
 . but WITHOUT ANY WARRANTY; without even the implied warranty of
 . MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 . GNU General Public License for more details.
 .
 . You should have received a copy of the GNU General Public License
 . along with this program; if not, write to the Free Software
 . Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307	 USA
 .
 .	Before Rev D the chip was    SMSC's 91C111 single-chip Ethernet device.
 .	For    Rev D the chip became SMSC's 9118   single-chip Ethernet device.
 .
 .	This file merges the code from
 .
 .		net/drivers/smc91111.c
 . authors:
 .	Erik Stahlman				( erik@vt.edu )
 .	Daris A Nevil				( dnevil@snmc.com )
 .
 .		net/drivers/smc9118.c
 . author:
 .	Peter Pearse				( peter.pearse@arm.com)
 .
 .
 .	The chip type is detected during init and determines which code is run
 .
 . Sources:
 .    o	  Chip datasheets (www.smsc.com)
 .
 . author:
 .	Peter Pearse				( peter.pearse@arm.com)
 .
 . History
 .	2007.10.10	Initial file merge
 .
 ----------------------------------------------------------------------------*/

#include <common.h>
#include <command.h>
#include <config.h>
#include "smc_rv.h"
#include <net.h>

#ifdef CONFIG_DRIVER_SMC_RV

#ifdef CONFIG_REALVIEW
/*
 * Chip address varies by board
 */
DECLARE_GLOBAL_DATA_PTR;
#endif

// Chip type
int chip_type = LAN_INVALID;
/*
 * SMSC9118 has an id register, 91111 has no register at that address 
 */
int get_chip_type(void){
	unsigned int id_reg = *SMSC9118_ID_REV;
	if((id_reg & 0xFFFF0000) == 0x01180000){
		printf("LAN9118 ethernet chip detected\n");
		return LAN9118;
	} else {
		printf("LAN91C111 ethernet chip assumed\n");
		return LAN91C111;
	}
}

// 91111 code start

/* Use power-down feature of the chip */
#define POWER_DOWN	0

#define NO_AUTOPROBE

#define SMC_DEBUG 0

#if SMC_DEBUG > 1
static const char version[] =
	"smc91111.c:v1.0 04/25/01 by Daris A Nevil (dnevil@snmc.com)\n";
#endif

/* Autonegotiation timeout in seconds */
#ifndef CONFIG_SMC_AUTONEG_TIMEOUT
#define CONFIG_SMC_AUTONEG_TIMEOUT 10
#endif

/*------------------------------------------------------------------------
 .
 . Configuration options, for the experienced user to change.
 .
 -------------------------------------------------------------------------*/

/*
 . Wait time for memory to be free.  This probably shouldn't be
 . tuned that much, as waiting for this means nothing else happens
 . in the system
*/
#define MEMORY_WAIT_TIME 16


#if (SMC_DEBUG > 2 )
#define PRINTK3(args...) printf(args)
#else
#define PRINTK3(args...)
#endif

#if SMC_DEBUG > 1
#define PRINTK2(args...) printf(args)
#else
#define PRINTK2(args...)
#endif

#ifdef SMC_DEBUG
#define PRINTK(args...) printf(args)
#else
#define PRINTK(args...)
#endif


/*------------------------------------------------------------------------
 .
 . The internal workings of the driver.	 If you are changing anything
 . here with the SMC stuff, you should have the datasheet and know
 . what you are doing.
 .
 -------------------------------------------------------------------------*/
#define CARDNAME "LAN91C111"

/* Memory sizing constant */
#define LAN91C111_MEMORY_MULTIPLIER	(1024*2)

#ifndef CONFIG_SMC91111_BASE
#define CONFIG_SMC91111_BASE 0x20000300
#endif

#define SMC_BASE_ADDRESS CONFIG_SMC91111_BASE

#define SMC_DEV_NAME "SMC91111"
#define SMC_PHY_ADDR 0x0000
#define SMC_ALLOC_MAX_TRY 5
#define SMC_TX_TIMEOUT 30

#define SMC_PHY_CLOCK_DELAY 1000

#define ETH_ZLEN 60

#ifdef	CONFIG_SMC_USE_32_BIT
#define USE_32_BIT  1
#else
#undef USE_32_BIT
#endif
/*-----------------------------------------------------------------
 .
 .  The driver can be entered at any of the following entry points.
 .
 .------------------------------------------------------------------  */

int	eth_init	(bd_t *bd);
void	eth_halt	(void);
int	eth_rx		(void);
int	eth_send	(volatile void *packet, int length);

#ifdef SHARED_RESOURCES
	extern void swap_to(int device_id);
#endif

/*
 . This is called by  register_netdev().  It is responsible for
 . checking the portlist for the SMC9000 series chipset.  If it finds
 . one, then it will initialize the device, find the hardware information,
 . and sets up the appropriate device parameters.
 . NOTE: Interrupts are *OFF* when this procedure is called.
 .
 . NB:This shouldn't be static since it is referred to externally.
*/
int smc_init(void);

/*
 . This is called by  unregister_netdev().  It is responsible for
 . cleaning up before the driver is finally unregistered and discarded.
*/
void smc_destructor(void);

/*
 . The kernel calls this function when someone wants to use the device,
 . typically 'ifconfig ethX up'.
*/
static int smc_open(bd_t *bd);


/*
 . This is called by the kernel in response to 'ifconfig ethX down'.  It
 . is responsible for cleaning up everything that the open routine
 . does, and maybe putting the card into a powerdown state.
*/
static int smc_close(void);

/*
 . Configures the PHY through the MII Management interface
*/
#ifndef CONFIG_SMC91111_EXT_PHY
static void smc_phy_configure(void);
#endif /* !CONFIG_SMC91111_EXT_PHY */

/*
 . This is a separate procedure to handle the receipt of a packet, to
 . leave the interrupt code looking slightly cleaner
*/
static int smc_rcv(void);

/* See if a MAC address is defined in the current environment. If so use it. If not
 . print a warning and set the environment and other globals with the default.
 . If an EEPROM is present it really should be consulted.
*/
int smc_get_ethaddr(bd_t *bd);
int get_rom_mac(uchar *v_rom_mac);

/*
 ------------------------------------------------------------
 .
 . Internal routines
 .
 ------------------------------------------------------------
*/

#ifdef CONFIG_SMC_USE_IOFUNCS
/*
 * input and output functions
 *
 * Implemented due to inx,outx macros accessing the device improperly
 * and putting the device into an unkown state.
 *
 * For instance, on Sharp LPD7A400 SDK, affects were chip memory
 * could not be free'd (hence the alloc failures), duplicate packets,
 * packets being corrupt (shifted) on the wire, etc.  Switching to the
 * inx,outx functions fixed this problem.
 */
static inline word SMC_inw(dword offset);
static inline void SMC_outw(word value, dword offset);
static inline byte SMC_inb(dword offset);
static inline void SMC_outb(byte value, dword offset);
static inline void SMC_insw(dword offset, volatile uchar* buf, dword len);
static inline void SMC_outsw(dword offset, uchar* buf, dword len);

#define barrier() __asm__ __volatile__("": : :"memory")

static inline word SMC_inw(dword offset)
{
	word v;
	v = *((volatile word*)(SMC_BASE_ADDRESS+offset));
	barrier(); *(volatile u32*)(0xc0000000);
	return v;
}

static inline void SMC_outw(word value, dword offset)
{
	*((volatile word*)(SMC_BASE_ADDRESS+offset)) = value;
	barrier(); *(volatile u32*)(0xc0000000);
}

static inline byte SMC_inb(dword offset)
{
	word  _w;

	_w = SMC_inw(offset & ~((dword)1));
	return (offset & 1) ? (byte)(_w >> 8) : (byte)(_w);
}

static inline void SMC_outb(byte value, dword offset)
{
	word  _w;

	_w = SMC_inw(offset & ~((dword)1));
	if (offset & 1)
			*((volatile word*)(SMC_BASE_ADDRESS+(offset & ~((dword)1)))) = (value<<8) | (_w & 0x00ff);
	else
			*((volatile word*)(SMC_BASE_ADDRESS+offset)) = value | (_w & 0xff00);
}

static inline void SMC_insw(dword offset, volatile uchar* buf, dword len)
{
	volatile word *p = (volatile word *)buf;

	while (len-- > 0) {
		*p++ = SMC_inw(offset);
		barrier();
		*((volatile u32*)(0xc0000000));
	}
}

static inline void SMC_outsw(dword offset, uchar* buf, dword len)
{
	volatile word *p = (volatile word *)buf;

	while (len-- > 0) {
		SMC_outw(*p++, offset);
		barrier();
		*(volatile u32*)(0xc0000000);
	}
}
#endif  /* CONFIG_SMC_USE_IOFUNCS */

static char unsigned smc_mac_addr[6] = {0x02, 0x80, 0xad, 0x20, 0x31, 0xb8};

/*
 * This function must be called before smc_open() if you want to override
 * the default mac address.
 */

void smc_set_mac_addr(const unsigned char *addr) {
	int i;

	for (i=0; i < sizeof(smc_mac_addr); i++){
		smc_mac_addr[i] = addr[i];
	}
}

/*
 * smc_get_macaddr is no longer used. If you want to override the default
 * mac address, call smc_get_mac_addr as a part of the board initialization.
 */

#if 0
void smc_get_macaddr( byte *addr ) {
	/* MAC ADDRESS AT FLASHBLOCK 1 / OFFSET 0x10 */
	unsigned char *dnp1110_mac = (unsigned char *) (0xE8000000 + 0x20010);
	int i;


	for (i=0; i<6; i++) {
	    addr[0] = *(dnp1110_mac+0);
	    addr[1] = *(dnp1110_mac+1);
	    addr[2] = *(dnp1110_mac+2);
	    addr[3] = *(dnp1110_mac+3);
	    addr[4] = *(dnp1110_mac+4);
	    addr[5] = *(dnp1110_mac+5);
	}
}
#endif /* 0 */

/***********************************************
 * Show available memory		       *
 ***********************************************/
void dump_memory_info(void)
{
	word mem_info;
	word old_bank;

	old_bank = SMC_inw(BANK_SELECT)&0xF;

	SMC_SELECT_BANK(0);
	mem_info = SMC_inw( MIR_REG );
	PRINTK2("Memory: %4d available\n", (mem_info >> 8)*2048);

	SMC_SELECT_BANK(old_bank);
}
/*
 . A rather simple routine to print out a packet for debugging purposes.
*/
#if SMC_DEBUG > 2
static void print_packet( byte *, int );
#endif

#define tx_done(dev) 1


/* this does a soft reset on the device */
static void smc_reset( void );

/* Enable Interrupts, Receive, and Transmit */
static void smc_enable( void );

/* this puts the device in an inactive state */
static void smc_shutdown( void );

/* Routines to Read and Write the PHY Registers across the
   MII Management Interface
*/

#ifndef CONFIG_SMC91111_EXT_PHY
static word smc_read_phy_register(byte phyreg);
static void smc_write_phy_register(byte phyreg, word phydata);
#endif /* !CONFIG_SMC91111_EXT_PHY */


static int poll4int (byte mask, int timeout)
{
	int tmo = get_timer (0) + timeout * CFG_HZ;
	int is_timeout = 0;
	word old_bank = SMC_inw (BSR_REG);

	PRINTK2 ("Polling...\n");
	SMC_SELECT_BANK (2);
	while ((SMC_inw (SMC91111_INT_REG) & mask) == 0) {
		if (get_timer (0) >= tmo) {
			is_timeout = 1;
			break;
		}
	}

	/* restore old bank selection */
	SMC_SELECT_BANK (old_bank);

	if (is_timeout)
		return 1;
	else
		return 0;
}

/* Only one release command at a time, please */
static inline void smc_wait_mmu_release_complete (void)
{
	int count = 0;

	/* assume bank 2 selected */
	while (SMC_inw (MMU_CMD_REG) & MC_BUSY) {
		udelay (1);	/* Wait until not busy */
		if (++count > 200)
			break;
	}
}

/*
 . Function: smc_reset( void )
 . Purpose:
 .	This sets the SMC91111 chip to its normal state, hopefully from whatever
 .	mess that any other DOS driver has put it in.
 .
 . Maybe I should reset more registers to defaults in here?  SOFTRST  should
 . do that for me.
 .
 . Method:
 .	1.  send a SOFT RESET
 .	2.  wait for it to finish
 .	3.  enable autorelease mode
 .	4.  reset the memory management unit
 .	5.  clear all interrupts
 .
*/
static void smc_reset (void)
{
	PRINTK2 ("%s: smc_reset\n", SMC_DEV_NAME);

	/* This resets the registers mostly to defaults, but doesn't
	   affect EEPROM.  That seems unnecessary */
	SMC_SELECT_BANK (0);
	SMC_outw (RCR_SOFTRST, RCR_REG);

	/* Setup the Configuration Register */
	/* This is necessary because the CONFIG_REG is not affected */
	/* by a soft reset */

	SMC_SELECT_BANK (1);
#if defined(CONFIG_SMC91111_EXT_PHY)
	SMC_outw (CONFIG_DEFAULT | CONFIG_EXT_PHY, CONFIG_REG);
#else
	SMC_outw (CONFIG_DEFAULT, CONFIG_REG);
#endif


	/* Release from possible power-down state */
	/* Configuration register is not affected by Soft Reset */
	SMC_outw (SMC_inw (CONFIG_REG) | CONFIG_EPH_POWER_EN, CONFIG_REG);

	SMC_SELECT_BANK (0);

	/* this should pause enough for the chip to be happy */
	udelay (10);

	/* Disable transmit and receive functionality */
	SMC_outw (RCR_CLEAR, RCR_REG);
	SMC_outw (TCR_CLEAR, TCR_REG);

	/* set the control register */
	SMC_SELECT_BANK (1);
	SMC_outw (CTL_DEFAULT, CTL_REG);

	/* Reset the MMU */
	SMC_SELECT_BANK (2);
	smc_wait_mmu_release_complete ();
	SMC_outw (MC_RESET, MMU_CMD_REG);
	while (SMC_inw (MMU_CMD_REG) & MC_BUSY)
		udelay (1);	/* Wait until not busy */

	/* Note:  It doesn't seem that waiting for the MMU busy is needed here,
	   but this is a place where future chipsets _COULD_ break.  Be wary
	   of issuing another MMU command right after this */

	/* Disable all interrupts */
	SMC_outb (0, IM_REG);
}

/*
 . Function: smc_enable
 . Purpose: let the chip talk to the outside work
 . Method:
 .	1.  Enable the transmitter
 .	2.  Enable the receiver
 .	3.  Enable interrupts
*/
static void smc_enable()
{
	PRINTK2("%s: smc_enable\n", SMC_DEV_NAME);
	SMC_SELECT_BANK( 0 );
	/* see the header file for options in TCR/RCR DEFAULT*/
	SMC_outw( TCR_DEFAULT, TCR_REG );
	SMC_outw( RCR_DEFAULT, RCR_REG );

	/* clear MII_DIS */
/*	smc_write_phy_register(PHY_CNTL_REG, 0x0000); */
}

/*
 . Function: smc_shutdown
 . Purpose:  closes down the SMC91xxx chip.
 . Method:
 .	1. zero the interrupt mask
 .	2. clear the enable receive flag
 .	3. clear the enable xmit flags
 .
 . TODO:
 .   (1) maybe utilize power down mode.
 .	Why not yet?  Because while the chip will go into power down mode,
 .	the manual says that it will wake up in response to any I/O requests
 .	in the register space.	 Empirical results do not show this working.
*/
static void smc_shutdown()
{
	PRINTK2(CARDNAME ": smc_shutdown\n");

	/* no more interrupts for me */
	SMC_SELECT_BANK( 2 );
	SMC_outb( 0, IM_REG );

	/* and tell the card to stay away from that nasty outside world */
	SMC_SELECT_BANK( 0 );
	SMC_outb( RCR_CLEAR, RCR_REG );
	SMC_outb( TCR_CLEAR, TCR_REG );
#ifdef SHARED_RESOURCES
	swap_to(FLASH);
#endif
}


/*
 . Function:  smc_hardware_send_packet(struct net_device * )
 . Purpose:
 .	This sends the actual packet to the SMC9xxx chip.
 .
 . Algorithm:
 .	First, see if a saved_skb is available.
 .		( this should NOT be called if there is no 'saved_skb'
 .	Now, find the packet number that the chip allocated
 .	Point the data pointers at it in memory
 .	Set the length word in the chip's memory
 .	Dump the packet to chip memory
 .	Check if a last byte is needed ( odd length packet )
 .		if so, set the control flag right
 .	Tell the card to send it
 .	Enable the transmit interrupt, so I know if it failed
 .	Free the kernel data if I actually sent it.
*/
static int smc_send_packet (volatile void *packet, int packet_length)
{
	byte packet_no;
	unsigned long ioaddr;
	byte *buf;
	int length;
	int numPages;
	int try = 0;
	int time_out;
	byte status;
	byte saved_pnr;
	word saved_ptr;

	/* save PTR and PNR registers before manipulation */
	SMC_SELECT_BANK (2);
	saved_pnr = SMC_inb( PN_REG );
	saved_ptr = SMC_inw( PTR_REG );

	PRINTK3 ("%s: smc_hardware_send_packet\n", SMC_DEV_NAME);

	length = ETH_ZLEN < packet_length ? packet_length : ETH_ZLEN;

	/* allocate memory
	 ** The MMU wants the number of pages to be the number of 256 bytes
	 ** 'pages', minus 1 ( since a packet can't ever have 0 pages :) )
	 **
	 ** The 91C111 ignores the size bits, but the code is left intact
	 ** for backwards and future compatibility.
	 **
	 ** Pkt size for allocating is data length +6 (for additional status
	 ** words, length and ctl!)
	 **
	 ** If odd size then last byte is included in this header.
	 */
	numPages = ((length & 0xfffe) + 6);
	numPages >>= 8;		/* Divide by 256 */

	if (numPages > 7) {
		printf ("%s: Far too big packet error. \n", SMC_DEV_NAME);
		return 0;
	}

	/* now, try to allocate the memory */
	SMC_SELECT_BANK (2);
	SMC_outw (MC_ALLOC | numPages, MMU_CMD_REG);

	/* FIXME: the ALLOC_INT bit never gets set *
	 * so the following will always give a	   *
	 * memory allocation error.		   *
	 * same code works in armboot though	   *
	 * -ro
	 */

again:
	try++;
	time_out = MEMORY_WAIT_TIME;
	do {
		status = SMC_inb (SMC91111_INT_REG);
		if (status & IM_ALLOC_INT) {
			/* acknowledge the interrupt */
			SMC_outb (IM_ALLOC_INT, SMC91111_INT_REG);
			break;
		}
	} while (--time_out);

	if (!time_out) {
		PRINTK2 ("%s: memory allocation, try %d failed ...\n",
			 SMC_DEV_NAME, try);
		if (try < SMC_ALLOC_MAX_TRY)
			goto again;
		else
			return 0;
	}

	PRINTK2 ("%s: memory allocation, try %d succeeded ...\n",
		 SMC_DEV_NAME, try);

	/* I can send the packet now.. */

	ioaddr = SMC_BASE_ADDRESS;

	buf = (byte *) packet;

	/* If I get here, I _know_ there is a packet slot waiting for me */
	packet_no = SMC_inb (AR_REG);
	if (packet_no & AR_FAILED) {
		/* or isn't there?  BAD CHIP! */
		printf ("%s: Memory allocation failed. \n", SMC_DEV_NAME);
		return 0;
	}

	/* we have a packet address, so tell the card to use it */
#ifndef CONFIG_XAENIAX
	SMC_outb (packet_no, PN_REG);
#else
	/* On Xaeniax board, we can't use SMC_outb here because that way
	 * the Allocate MMU command will end up written to the command register
	 * as well, which will lead to a problem.
	 */
	SMC_outl (packet_no << 16, 0);
#endif
	/* do not write new ptr value if Write data fifo not empty */
	while ( saved_ptr & PTR_NOTEMPTY )
		printf ("Write data fifo not empty!\n");

	/* point to the beginning of the packet */
	SMC_outw (PTR_AUTOINC, PTR_REG);

	PRINTK3 ("%s: Trying to xmit packet of length %x\n",
		 SMC_DEV_NAME, length);

#if SMC_DEBUG > 2
	printf ("Transmitting Packet\n");
	print_packet (buf, length);
#endif

	/* send the packet length ( +6 for status, length and ctl byte )
	   and the status word ( set to zeros ) */
#ifdef USE_32_BIT
	SMC_outl ((length + 6) << 16, SMC91111_DATA_REG);
#else
	SMC_outw (0, SMC91111_DATA_REG);
	/* send the packet length ( +6 for status words, length, and ctl */
	SMC_outw ((length + 6), SMC91111_DATA_REG);
#endif

	/* send the actual data
	   . I _think_ it's faster to send the longs first, and then
	   . mop up by sending the last word.  It depends heavily
	   . on alignment, at least on the 486.	 Maybe it would be
	   . a good idea to check which is optimal?  But that could take
	   . almost as much time as is saved?
	 */
#ifdef USE_32_BIT
	SMC_outsl (SMC91111_DATA_REG, buf, length >> 2);
#ifndef CONFIG_XAENIAX
	if (length & 0x2)
		SMC_outw (*((word *) (buf + (length & 0xFFFFFFFC))),
			  SMC91111_DATA_REG);
#else
	/* On XANEIAX, we can only use 32-bit writes, so we need to handle
	 * unaligned tail part specially. The standard code doesn't work.
	 */
	if ((length & 3) == 3) {
		u16 * ptr = (u16*) &buf[length-3];
		SMC_outl((*ptr) | ((0x2000 | buf[length-1]) << 16),
				SMC91111_DATA_REG);
	} else if ((length & 2) == 2) {
		u16 * ptr = (u16*) &buf[length-2];
		SMC_outl(*ptr, SMC91111_DATA_REG);
	} else if (length & 1) {
		SMC_outl((0x2000 | buf[length-1]), SMC91111_DATA_REG);
	} else {
		SMC_outl(0, SMC91111_DATA_REG);
	}
#endif
#else
	SMC_outsw (SMC91111_DATA_REG, buf, (length) >> 1);
#endif /* USE_32_BIT */

#ifndef CONFIG_XAENIAX
	/* Send the last byte, if there is one.	  */
	if ((length & 1) == 0) {
		SMC_outw (0, SMC91111_DATA_REG);
	} else {
		SMC_outw (buf[length - 1] | 0x2000, SMC91111_DATA_REG);
	}
#endif

	/* and let the chipset deal with it */
	SMC_outw (MC_ENQUEUE, MMU_CMD_REG);

	/* poll for TX INT */
	/* if (poll4int (IM_TX_INT, SMC_TX_TIMEOUT)) { */
	/* poll for TX_EMPTY INT - autorelease enabled */
	if (poll4int(IM_TX_EMPTY_INT, SMC_TX_TIMEOUT)) {
		/* sending failed */
		PRINTK2 ("%s: TX timeout, sending failed...\n", SMC_DEV_NAME);

		/* release packet */
		/* no need to release, MMU does that now */
#ifdef CONFIG_XAENIAX
		 SMC_outw (MC_FREEPKT, MMU_CMD_REG);
#endif

		/* wait for MMU getting ready (low) */
		while (SMC_inw (MMU_CMD_REG) & MC_BUSY) {
			udelay (10);
		}

		PRINTK2 ("MMU ready\n");


		return 0;
	} else {
		/* ack. int */
		SMC_outb (IM_TX_EMPTY_INT, SMC91111_INT_REG);
		/* SMC_outb (IM_TX_INT, SMC91111_INT_REG); */
		PRINTK2 ("%s: Sent packet of length %d \n", SMC_DEV_NAME,
			 length);

		/* release packet */
		/* no need to release, MMU does that now */
#ifdef CONFIG_XAENIAX
		SMC_outw (MC_FREEPKT, MMU_CMD_REG);
#endif

		/* wait for MMU getting ready (low) */
		while (SMC_inw (MMU_CMD_REG) & MC_BUSY) {
			udelay (10);
		}

		PRINTK2 ("MMU ready\n");


	}

	/* restore previously saved registers */
#ifndef CONFIG_XAENIAX
	SMC_outb( saved_pnr, PN_REG );
#else
	/* On Xaeniax board, we can't use SMC_outb here because that way
	 * the Allocate MMU command will end up written to the command register
	 * as well, which will lead to a problem.
	 */
	SMC_outl(saved_pnr << 16, 0);
#endif
	SMC_outw( saved_ptr, PTR_REG );

	return length;
}

/*-------------------------------------------------------------------------
 |
 | smc_destructor( struct net_device * dev )
 |   Input parameters:
 |	dev, pointer to the device structure
 |
 |   Output:
 |	None.
 |
 ---------------------------------------------------------------------------
*/
void smc_destructor()
{
	PRINTK2(CARDNAME ": smc_destructor\n");
}


/*
 * Open and Initialize the board
 *
 * Set up everything, reset the card, etc ..
 *
 */
static int smc_open (bd_t * bd)
{
	int i, err;

	PRINTK2 ("%s: smc_open\n", SMC_DEV_NAME);

	/* reset the hardware */
	smc_reset ();
	smc_enable ();

	/* Configure the PHY */
#ifndef CONFIG_SMC91111_EXT_PHY
	smc_phy_configure ();
#endif

	/* conservative setting (10Mbps, HalfDuplex, no AutoNeg.) */
/*	SMC_SELECT_BANK(0); */
/*	SMC_outw(0, RPC_REG); */
	SMC_SELECT_BANK (1);

	err = smc_get_ethaddr (bd);	/* set smc_mac_addr, and sync it with u-boot globals */
	if (err < 0) {
		memset (bd->bi_enetaddr, 0, 6); /* hack to make error stick! upper code will abort if not set */
		return (-1);	/* upper code ignores this, but NOT bi_enetaddr */
	}
#ifdef USE_32_BIT
	for (i = 0; i < 6; i += 2) {
		word address;

		address = smc_mac_addr[i + 1] << 8;
		address |= smc_mac_addr[i];
		SMC_outw (address, (ADDR0_REG + i));
	}
#else
	for (i = 0; i < 6; i++)
		SMC_outb (smc_mac_addr[i], (ADDR0_REG + i));
#endif

	return 0;
}

/*-------------------------------------------------------------
 .
 . smc_rcv -  receive a packet from the card
 .
 . There is ( at least ) a packet waiting to be read from
 . chip-memory.
 .
 . o Read the status
 . o If an error, record it
 . o otherwise, read in the packet
 --------------------------------------------------------------
*/
static int smc_rcv()
{
	int	packet_number;
	word	status;
	word	packet_length;
	int	is_error = 0;
#ifdef USE_32_BIT
	dword stat_len;
#endif
	byte saved_pnr;
	word saved_ptr;

	SMC_SELECT_BANK(2);
	/* save PTR and PTR registers */
	saved_pnr = SMC_inb( PN_REG );
	saved_ptr = SMC_inw( PTR_REG );

	packet_number = SMC_inw( RXFIFO_REG );

	if ( packet_number & RXFIFO_REMPTY ) {

		return 0;
	}

	PRINTK3("%s: smc_rcv\n", SMC_DEV_NAME);
	/*  start reading from the start of the packet */
	SMC_outw( PTR_READ | PTR_RCV | PTR_AUTOINC, PTR_REG );

	/* First two words are status and packet_length */
#ifdef USE_32_BIT
	stat_len = SMC_inl(SMC91111_DATA_REG);
	status = stat_len & 0xffff;
	packet_length = stat_len >> 16;
#else
	status		= SMC_inw( SMC91111_DATA_REG );
	packet_length	= SMC_inw( SMC91111_DATA_REG );
#endif

	packet_length &= 0x07ff;  /* mask off top bits */

	PRINTK2("RCV: STATUS %4x LENGTH %4x\n", status, packet_length );

	if ( !(status & RS_ERRORS ) ){
		/* Adjust for having already read the first two words */
		packet_length -= 4; /*4; */


		/* set odd length for bug in LAN91C111, */
		/* which never sets RS_ODDFRAME */
		/* TODO ? */


#ifdef USE_32_BIT
		PRINTK3(" Reading %d dwords (and %d bytes) \n",
			packet_length >> 2, packet_length & 3 );
		/* QUESTION:  Like in the TX routine, do I want
		   to send the DWORDs or the bytes first, or some
		   mixture.  A mixture might improve already slow PIO
		   performance	*/
		SMC_insl( SMC91111_DATA_REG , NetRxPackets[0], packet_length >> 2 );
		/* read the left over bytes */
		if (packet_length & 3) {
			int i;

			byte *tail = (byte *)(NetRxPackets[0] + (packet_length & ~3));
			dword leftover = SMC_inl(SMC91111_DATA_REG);
			for (i=0; i<(packet_length & 3); i++)
				*tail++ = (byte) (leftover >> (8*i)) & 0xff;
		}
#else
		PRINTK3(" Reading %d words and %d byte(s) \n",
			(packet_length >> 1 ), packet_length & 1 );
		SMC_insw(SMC91111_DATA_REG , NetRxPackets[0], packet_length >> 1);

#endif /* USE_32_BIT */

#if	SMC_DEBUG > 2
		printf("Receiving Packet\n");
		print_packet( NetRxPackets[0], packet_length );
#endif
	} else {
		/* error ... */
		/* TODO ? */
		is_error = 1;
	}

	while ( SMC_inw( MMU_CMD_REG ) & MC_BUSY )
		udelay(1); /* Wait until not busy */

	/*  error or good, tell the card to get rid of this packet */
	SMC_outw( MC_RELEASE, MMU_CMD_REG );

	while ( SMC_inw( MMU_CMD_REG ) & MC_BUSY )
		udelay(1); /* Wait until not busy */

	/* restore saved registers */
#ifndef CONFIG_XAENIAX
	SMC_outb( saved_pnr, PN_REG );
#else
	/* On Xaeniax board, we can't use SMC_outb here because that way
	 * the Allocate MMU command will end up written to the command register
	 * as well, which will lead to a problem.
	 */
	SMC_outl( saved_pnr << 16, 0);
#endif
	SMC_outw( saved_ptr, PTR_REG );

	if (!is_error) {
		/* Pass the packet up to the protocol layers. */
		NetReceive(NetRxPackets[0], packet_length);
		return packet_length;
	} else {
		return 0;
	}

}


/*----------------------------------------------------
 . smc_close
 .
 . this makes the board clean up everything that it can
 . and not talk to the outside world.	Caused by
 . an 'ifconfig ethX down'
 .
 -----------------------------------------------------*/
static int smc_close()
{
	PRINTK2("%s: smc_close\n", SMC_DEV_NAME);

	/* clear everything */
	smc_shutdown();

	return 0;
}


#if 0
/*------------------------------------------------------------
 . Modify a bit in the LAN91C111 register set
 .-------------------------------------------------------------*/
static word smc_modify_regbit(int bank, int ioaddr, int reg,
	unsigned int bit, int val)
{
	word regval;

	SMC_SELECT_BANK( bank );

	regval = SMC_inw( reg );
	if (val)
		regval |= bit;
	else
		regval &= ~bit;

	SMC_outw( regval, 0 );
	return(regval);
}


/*------------------------------------------------------------
 . Retrieve a bit in the LAN91C111 register set
 .-------------------------------------------------------------*/
static int smc_get_regbit(int bank, int ioaddr, int reg, unsigned int bit)
{
	SMC_SELECT_BANK( bank );
	if ( SMC_inw( reg ) & bit)
		return(1);
	else
		return(0);
}


/*------------------------------------------------------------
 . Modify a LAN91C111 register (word access only)
 .-------------------------------------------------------------*/
static void smc_modify_reg(int bank, int ioaddr, int reg, word val)
{
	SMC_SELECT_BANK( bank );
	SMC_outw( val, reg );
}


/*------------------------------------------------------------
 . Retrieve a LAN91C111 register (word access only)
 .-------------------------------------------------------------*/
static int smc_get_reg(int bank, int ioaddr, int reg)
{
	SMC_SELECT_BANK( bank );
	return(SMC_inw( reg ));
}

#endif /* 0 */

/*---PHY CONTROL AND CONFIGURATION----------------------------------------- */

#if (SMC_DEBUG > 2 )

/*------------------------------------------------------------
 . Debugging function for viewing MII Management serial bitstream
 .-------------------------------------------------------------*/
static void smc_dump_mii_stream (byte * bits, int size)
{
	int i;

	printf ("BIT#:");
	for (i = 0; i < size; ++i) {
		printf ("%d", i % 10);
	}

	printf ("\nMDOE:");
	for (i = 0; i < size; ++i) {
		if (bits[i] & MII_MDOE)
			printf ("1");
		else
			printf ("0");
	}

	printf ("\nMDO :");
	for (i = 0; i < size; ++i) {
		if (bits[i] & MII_MDO)
			printf ("1");
		else
			printf ("0");
	}

	printf ("\nMDI :");
	for (i = 0; i < size; ++i) {
		if (bits[i] & MII_MDI)
			printf ("1");
		else
			printf ("0");
	}

	printf ("\n");
}
#endif

/*------------------------------------------------------------
 . Reads a register from the MII Management serial interface
 .-------------------------------------------------------------*/
#ifndef CONFIG_SMC91111_EXT_PHY
static word smc_read_phy_register (byte phyreg)
{
	int oldBank;
	int i;
	byte mask;
	word mii_reg;
	byte bits[64];
	int clk_idx = 0;
	int input_idx;
	word phydata;
	byte phyaddr = SMC_PHY_ADDR;

	/* 32 consecutive ones on MDO to establish sync */
	for (i = 0; i < 32; ++i)
		bits[clk_idx++] = MII_MDOE | MII_MDO;

	/* Start code <01> */
	bits[clk_idx++] = MII_MDOE;
	bits[clk_idx++] = MII_MDOE | MII_MDO;

	/* Read command <10> */
	bits[clk_idx++] = MII_MDOE | MII_MDO;
	bits[clk_idx++] = MII_MDOE;

	/* Output the PHY address, msb first */
	mask = (byte) 0x10;
	for (i = 0; i < 5; ++i) {
		if (phyaddr & mask)
			bits[clk_idx++] = MII_MDOE | MII_MDO;
		else
			bits[clk_idx++] = MII_MDOE;

		/* Shift to next lowest bit */
		mask >>= 1;
	}

	/* Output the phy register number, msb first */
	mask = (byte) 0x10;
	for (i = 0; i < 5; ++i) {
		if (phyreg & mask)
			bits[clk_idx++] = MII_MDOE | MII_MDO;
		else
			bits[clk_idx++] = MII_MDOE;

		/* Shift to next lowest bit */
		mask >>= 1;
	}

	/* Tristate and turnaround (2 bit times) */
	bits[clk_idx++] = 0;
	/*bits[clk_idx++] = 0; */

	/* Input starts at this bit time */
	input_idx = clk_idx;

	/* Will input 16 bits */
	for (i = 0; i < 16; ++i)
		bits[clk_idx++] = 0;

	/* Final clock bit */
	bits[clk_idx++] = 0;

	/* Save the current bank */
	oldBank = SMC_inw (BANK_SELECT);

	/* Select bank 3 */
	SMC_SELECT_BANK (3);

	/* Get the current MII register value */
	mii_reg = SMC_inw (MII_REG);

	/* Turn off all MII Interface bits */
	mii_reg &= ~(MII_MDOE | MII_MCLK | MII_MDI | MII_MDO);

	/* Clock all 64 cycles */
	for (i = 0; i < sizeof bits; ++i) {
		/* Clock Low - output data */
		SMC_outw (mii_reg | bits[i], MII_REG);
		udelay (SMC_PHY_CLOCK_DELAY);


		/* Clock Hi - input data */
		SMC_outw (mii_reg | bits[i] | MII_MCLK, MII_REG);
		udelay (SMC_PHY_CLOCK_DELAY);
		bits[i] |= SMC_inw (MII_REG) & MII_MDI;
	}

	/* Return to idle state */
	/* Set clock to low, data to low, and output tristated */
	SMC_outw (mii_reg, MII_REG);
	udelay (SMC_PHY_CLOCK_DELAY);

	/* Restore original bank select */
	SMC_SELECT_BANK (oldBank);

	/* Recover input data */
	phydata = 0;
	for (i = 0; i < 16; ++i) {
		phydata <<= 1;

		if (bits[input_idx++] & MII_MDI)
			phydata |= 0x0001;
	}

#if (SMC_DEBUG > 2 )
	printf ("smc_read_phy_register(): phyaddr=%x,phyreg=%x,phydata=%x\n",
		phyaddr, phyreg, phydata);
	smc_dump_mii_stream (bits, sizeof bits);
#endif

	return (phydata);
}


/*------------------------------------------------------------
 . Writes a register to the MII Management serial interface
 .-------------------------------------------------------------*/
static void smc_write_phy_register (byte phyreg, word phydata)
{
	int oldBank;
	int i;
	word mask;
	word mii_reg;
	byte bits[65];
	int clk_idx = 0;
	byte phyaddr = SMC_PHY_ADDR;

	/* 32 consecutive ones on MDO to establish sync */
	for (i = 0; i < 32; ++i)
		bits[clk_idx++] = MII_MDOE | MII_MDO;

	/* Start code <01> */
	bits[clk_idx++] = MII_MDOE;
	bits[clk_idx++] = MII_MDOE | MII_MDO;

	/* Write command <01> */
	bits[clk_idx++] = MII_MDOE;
	bits[clk_idx++] = MII_MDOE | MII_MDO;

	/* Output the PHY address, msb first */
	mask = (byte) 0x10;
	for (i = 0; i < 5; ++i) {
		if (phyaddr & mask)
			bits[clk_idx++] = MII_MDOE | MII_MDO;
		else
			bits[clk_idx++] = MII_MDOE;

		/* Shift to next lowest bit */
		mask >>= 1;
	}

	/* Output the phy register number, msb first */
	mask = (byte) 0x10;
	for (i = 0; i < 5; ++i) {
		if (phyreg & mask)
			bits[clk_idx++] = MII_MDOE | MII_MDO;
		else
			bits[clk_idx++] = MII_MDOE;

		/* Shift to next lowest bit */
		mask >>= 1;
	}

	/* Tristate and turnaround (2 bit times) */
	bits[clk_idx++] = 0;
	bits[clk_idx++] = 0;

	/* Write out 16 bits of data, msb first */
	mask = 0x8000;
	for (i = 0; i < 16; ++i) {
		if (phydata & mask)
			bits[clk_idx++] = MII_MDOE | MII_MDO;
		else
			bits[clk_idx++] = MII_MDOE;

		/* Shift to next lowest bit */
		mask >>= 1;
	}

	/* Final clock bit (tristate) */
	bits[clk_idx++] = 0;

	/* Save the current bank */
	oldBank = SMC_inw (BANK_SELECT);

	/* Select bank 3 */
	SMC_SELECT_BANK (3);

	/* Get the current MII register value */
	mii_reg = SMC_inw (MII_REG);

	/* Turn off all MII Interface bits */
	mii_reg &= ~(MII_MDOE | MII_MCLK | MII_MDI | MII_MDO);

	/* Clock all cycles */
	for (i = 0; i < sizeof bits; ++i) {
		/* Clock Low - output data */
		SMC_outw (mii_reg | bits[i], MII_REG);
		udelay (SMC_PHY_CLOCK_DELAY);


		/* Clock Hi - input data */
		SMC_outw (mii_reg | bits[i] | MII_MCLK, MII_REG);
		udelay (SMC_PHY_CLOCK_DELAY);
		bits[i] |= SMC_inw (MII_REG) & MII_MDI;
	}

	/* Return to idle state */
	/* Set clock to low, data to low, and output tristated */
	SMC_outw (mii_reg, MII_REG);
	udelay (SMC_PHY_CLOCK_DELAY);

	/* Restore original bank select */
	SMC_SELECT_BANK (oldBank);

#if (SMC_DEBUG > 2 )
	printf ("smc_write_phy_register(): phyaddr=%x,phyreg=%x,phydata=%x\n",
		phyaddr, phyreg, phydata);
	smc_dump_mii_stream (bits, sizeof bits);
#endif
}
#endif /* !CONFIG_SMC91111_EXT_PHY */


/*------------------------------------------------------------
 . Waits the specified number of milliseconds - kernel friendly
 .-------------------------------------------------------------*/
#ifndef CONFIG_SMC91111_EXT_PHY
static void smc_wait_ms(unsigned int ms)
{
	udelay(ms*1000);
}
#endif /* !CONFIG_SMC91111_EXT_PHY */


/*------------------------------------------------------------
 . Configures the specified PHY using Autonegotiation. Calls
 . smc_phy_fixed() if the user has requested a certain config.
 .-------------------------------------------------------------*/
#ifndef CONFIG_SMC91111_EXT_PHY
static void smc_phy_configure ()
{
	int timeout;
	byte phyaddr;
	word my_phy_caps;	/* My PHY capabilities */
	word my_ad_caps;	/* My Advertised capabilities */
	word status = 0;	/*;my status = 0 */
	int failed = 0;

	PRINTK3 ("%s: smc_program_phy()\n", SMC_DEV_NAME);


	/* Get the detected phy address */
	phyaddr = SMC_PHY_ADDR;

	/* Reset the PHY, setting all other bits to zero */
	smc_write_phy_register (PHY_CNTL_REG, PHY_CNTL_RST);

	/* Wait for the reset to complete, or time out */
	timeout = 6;		/* Wait up to 3 seconds */
	while (timeout--) {
		if (!(smc_read_phy_register (PHY_CNTL_REG)
		      & PHY_CNTL_RST)) {
			/* reset complete */
			break;
		}

		smc_wait_ms (500);	/* wait 500 millisecs */
	}

	if (timeout < 1) {
		printf ("%s:PHY reset timed out\n", SMC_DEV_NAME);
		goto smc_phy_configure_exit;
	}

	/* Read PHY Register 18, Status Output */
	/* lp->lastPhy18 = smc_read_phy_register(PHY_INT_REG); */

	/* Enable PHY Interrupts (for register 18) */
	/* Interrupts listed here are disabled */
	smc_write_phy_register (PHY_MASK_REG, 0xffff);

	/* Configure the Receive/Phy Control register */
	SMC_SELECT_BANK (0);
	SMC_outw (RPC_DEFAULT, RPC_REG);

	/* Copy our capabilities from PHY_STAT_REG to PHY_AD_REG */
	my_phy_caps = smc_read_phy_register (PHY_STAT_REG);
	my_ad_caps = PHY_AD_CSMA;	/* I am CSMA capable */

	if (my_phy_caps & PHY_STAT_CAP_T4)
		my_ad_caps |= PHY_AD_T4;

	if (my_phy_caps & PHY_STAT_CAP_TXF)
		my_ad_caps |= PHY_AD_TX_FDX;

	if (my_phy_caps & PHY_STAT_CAP_TXH)
		my_ad_caps |= PHY_AD_TX_HDX;

	if (my_phy_caps & PHY_STAT_CAP_TF)
		my_ad_caps |= PHY_AD_10_FDX;

	if (my_phy_caps & PHY_STAT_CAP_TH)
		my_ad_caps |= PHY_AD_10_HDX;

	/* Update our Auto-Neg Advertisement Register */
	smc_write_phy_register (PHY_AD_REG, my_ad_caps);

	/* Read the register back.  Without this, it appears that when */
	/* auto-negotiation is restarted, sometimes it isn't ready and */
	/* the link does not come up. */
	smc_read_phy_register(PHY_AD_REG);

	PRINTK2 ("%s: phy caps=%x\n", SMC_DEV_NAME, my_phy_caps);
	PRINTK2 ("%s: phy advertised caps=%x\n", SMC_DEV_NAME, my_ad_caps);

	/* Restart auto-negotiation process in order to advertise my caps */
	smc_write_phy_register (PHY_CNTL_REG,
				PHY_CNTL_ANEG_EN | PHY_CNTL_ANEG_RST);

	/* Wait for the auto-negotiation to complete.  This may take from */
	/* 2 to 3 seconds. */
	/* Wait for the reset to complete, or time out */
	timeout = CONFIG_SMC_AUTONEG_TIMEOUT * 2;
	while (timeout--) {

		status = smc_read_phy_register (PHY_STAT_REG);
		if (status & PHY_STAT_ANEG_ACK) {
			/* auto-negotiate complete */
			break;
		}

		smc_wait_ms (500);	/* wait 500 millisecs */

		/* Restart auto-negotiation if remote fault */
		if (status & PHY_STAT_REM_FLT) {
			printf ("%s: PHY remote fault detected\n",
				SMC_DEV_NAME);

			/* Restart auto-negotiation */
			printf ("%s: PHY restarting auto-negotiation\n",
				SMC_DEV_NAME);
			smc_write_phy_register (PHY_CNTL_REG,
						PHY_CNTL_ANEG_EN |
						PHY_CNTL_ANEG_RST |
						PHY_CNTL_SPEED |
						PHY_CNTL_DPLX);
		}
	}

	if (timeout < 1) {
		printf ("%s: PHY auto-negotiate timed out\n", SMC_DEV_NAME);
		failed = 1;
	}

	/* Fail if we detected an auto-negotiate remote fault */
	if (status & PHY_STAT_REM_FLT) {
		printf ("%s: PHY remote fault detected\n", SMC_DEV_NAME);
		failed = 1;
	}

	/* Re-Configure the Receive/Phy Control register */
	SMC_outw (RPC_DEFAULT, RPC_REG);

smc_phy_configure_exit:	;

}
#endif /* !CONFIG_SMC91111_EXT_PHY */


#if SMC_DEBUG > 2
static void print_packet( byte * buf, int length )
{
	int i;
	int remainder;
	int lines;

	printf("Packet of length %d \n", length );

#if SMC_DEBUG > 3
	lines = length / 16;
	remainder = length % 16;

	for ( i = 0; i < lines ; i ++ ) {
		int cur;

		for ( cur = 0; cur < 8; cur ++ ) {
			byte a, b;

			a = *(buf ++ );
			b = *(buf ++ );
			printf("%02x%02x ", a, b );
		}
		printf("\n");
	}
	for ( i = 0; i < remainder/2 ; i++ ) {
		byte a, b;

		a = *(buf ++ );
		b = *(buf ++ );
		printf("%02x%02x ", a, b );
	}
	printf("\n");
#endif
}
#endif

int smc_get_ethaddr (bd_t * bd)
{
	int env_size, rom_valid, env_present = 0, reg;
	char *s = NULL, *e, *v_mac, es[] = "11:22:33:44:55:66";
	char s_env_mac[64];
	uchar v_env_mac[6], v_rom_mac[6];

	env_size = getenv_r ("ethaddr", s_env_mac, sizeof (s_env_mac));
	if ((env_size > 0) && (env_size < sizeof (es))) {	/* exit if env is bad */
		printf ("\n*** ERROR: ethaddr is not set properly!!\n");
		return (-1);
	}

	if (env_size > 0) {
		env_present = 1;
		s = s_env_mac;
	}

	for (reg = 0; reg < 6; ++reg) { /* turn string into mac value */
		v_env_mac[reg] = s ? simple_strtoul (s, &e, 16) : 0;
		if (s)
			s = (*e) ? e + 1 : e;
	}

	rom_valid = get_rom_mac (v_rom_mac);	/* get ROM mac value if any */

	if (!env_present) {	/* if NO env */
		if (rom_valid) {	/* but ROM is valid */
			v_mac = (char *)v_rom_mac;
			sprintf (s_env_mac, "%02X:%02X:%02X:%02X:%02X:%02X",
				 v_mac[0], v_mac[1], v_mac[2], v_mac[3],
				 v_mac[4], v_mac[5]);
			setenv ("ethaddr", s_env_mac);
		} else {	/* no env, bad ROM */
			printf ("\n*** ERROR: ethaddr is NOT set !!\n");
			return (-1);
		}
	} else {		/* good env, don't care ROM */
		v_mac = (char *)v_env_mac;	/* always use a good env over a ROM */
	}

	if (env_present && rom_valid) { /* if both env and ROM are good */
		if (memcmp (v_env_mac, v_rom_mac, 6) != 0) {
			printf ("\nWarning: MAC addresses don't match:\n");
			printf ("\tHW MAC address:  "
				"%02X:%02X:%02X:%02X:%02X:%02X\n",
				v_rom_mac[0], v_rom_mac[1],
				v_rom_mac[2], v_rom_mac[3],
				v_rom_mac[4], v_rom_mac[5] );
			printf ("\t\"ethaddr\" value: "
				"%02X:%02X:%02X:%02X:%02X:%02X\n",
				v_env_mac[0], v_env_mac[1],
				v_env_mac[2], v_env_mac[3],
				v_env_mac[4], v_env_mac[5]) ;
			debug ("### Set MAC addr from environment\n");
		}
	}
	memcpy (bd->bi_enetaddr, v_mac, 6);	/* update global address to match env (allows env changing) */
	smc_set_mac_addr ((uchar *)v_mac);	/* use old function to update smc default */
	PRINTK("Using MAC Address %02X:%02X:%02X:%02X:%02X:%02X\n", v_mac[0], v_mac[1],
		v_mac[2], v_mac[3], v_mac[4], v_mac[5]);
	return (0);
}

int get_rom_mac (uchar *v_rom_mac)
{
#ifdef HARDCODE_MAC	/* used for testing or to supress run time warnings */
	char hw_mac_addr[] = { 0x02, 0x80, 0xad, 0x20, 0x31, 0xb8 };

	memcpy (v_rom_mac, hw_mac_addr, 6);
	return (1);
#else
	int i;
	int valid_mac = 0;

	SMC_SELECT_BANK (1);
	for (i=0; i<6; i++)
	{
		v_rom_mac[i] = SMC_inb ((ADDR0_REG + i));
		valid_mac |= v_rom_mac[i];
	}

	return (valid_mac ? 1 : 0);
#endif
}

// 91111 code end

// 9118 code start
#define SMC9118_DEV_NAME "SMC9118"

#ifndef CONFIG_SMC_RV_BASE
	include/configs/<board>.h must define the base address of the device registers
#endif

#define SMC9118_DBG 0

#if SMC9118_DBG > 1
static const char version[] =
	"smc9118.c:v1.0 06/10/24 by Peter Pearse <peter.pearse@arm.com>\n";
#endif

#if (SMC9118_DBG > 2 )
# define PRINTK3(args...) printf(args)
#else
# define PRINTK3(args...)
#endif

#if SMC9118_DBG > 1
# define PRINTK2(args...) printf(args)
#else
# define PRINTK2(args...)
#endif

#ifdef SMC9118_DBG
# define PRINTK(args...) printf(args)
#else
# define PRINTK(args...)
#endif

#if SMC9118_DBG > 0

static void smsc9118_print_mac_registers(void)
{
    unsigned int read;
    int i;

    i = 0;
    read = 0;

    for(i = 1; i <= 0xC; i++) {
        smsc9118_mac_regread(i, &read);
        debug("MAC Register %02d: 0x%08x\n",i,read);
    }

    debug("\n");
    return;
}
static void smsc9118_print_registers(void){
    volatile unsigned int *i;

    for  (i = (volatile unsigned int *)CONFIG_SMC_RV_BASE; (int)i < (int)SMSC9118_RESERVED3; i++){
        debug("Register @%p 0x%08x\n",i, *i);
    }

} 
static void smsc9118_print_phy_registers(void)
{
    unsigned short read;
    unsigned int i;

    i = 0;
    read = 0;
    for(i = 0; i <= 6; i++) {
        smsc9118_phy_regread(i, &read);
        debug("PHY Register %02d: 0x%08x\n",i,read);
    }
    smsc9118_phy_regread(i = 17, &read);
    debug("Phy Register %02d: 0x%08x\n", i, read);

    smsc9118_phy_regread(i = 18, &read);
    debug("Phy Register %02d: 0x%08x\n", i, read);

    smsc9118_phy_regread(i = 27, &read);
    debug("Phy Register %02d: 0x%08x\n", i, read);

    smsc9118_phy_regread(i = 29, &read);
    debug("Phy Register %02d: 0x%08x\n", i, read);

    smsc9118_phy_regread(i = 30, &read);
    debug("Phy Register %02d: 0x%08x\n", i, read);

    smsc9118_phy_regread(i = 31, &read);
    debug("Phy Register %02d: 0x%08x\n", i, read);

    debug("\n");
    return;
}
#endif /* SMC9118_DBG > 0 */

#ifdef SHARED_RESOURCES
/*
 * Resource swapper
 */
void swap_to(int device_id);
#endif

/*
 * ----------------------------------------------------------
 *
 * Chip register access routines
 *
 * ----------------------------------------------------------
 */
static int smsc9118_mac_regread(unsigned char regoffset, unsigned int *data)
{
    unsigned int val, maccmd;
    int timedout;
    int error;
    ulong start;

    error = 0;
    val = *SMSC9118_MAC_CSR_CMD;
    if(!(val & ((unsigned int)1 << 31))) {    // Make sure there's no pending operation
        maccmd = 0;
        maccmd |= regoffset;
        maccmd |= ((unsigned int)1 << 30);     // Indicates read
        maccmd |= ((unsigned int)1 << 31);     // Start bit
        *SMSC9118_MAC_CSR_CMD = maccmd; // Start operation

	start = get_timer (0);
        do {
	    timedout = get_timer(start) > MS50 ? 1 : 0;
            val = *SMSC9118_BYTE_TEST;  // A no-op read.
        } while(!timedout && (*SMSC9118_MAC_CSR_CMD & ((unsigned int)1 << 31)));

        if(timedout) {
            printf("Error: SMSC9118 MAC CSR read operation timed out.\n");
            error = 1;
            return error;
        }
        *data = *SMSC9118_MAC_CSR_DATA;
    } else {
        printf("Warning: SMSC9118 MAC CSR is busy. No data read.\n");
        *data = 0;
    }
    return 0;
}

static int smsc9118_mac_regwrite(unsigned char regoffset, unsigned int val)
{
    unsigned int read, maccmd;
    int timedout;
    int error;
    ulong start;

    debug("MAC[%02d] write 0x%08x \n", regoffset, val);

    error = 0;
    read = *SMSC9118_MAC_CSR_CMD;
    if(!(read & ((unsigned int)1 << 31))) { // Make sure there's no pending operation
        *SMSC9118_MAC_CSR_DATA = val;       // Load data.
        maccmd = 0;
        maccmd |= regoffset;
        maccmd &= ~((unsigned int)1 << 30); // Clear indicates write
        maccmd |= ((unsigned int)1 << 31);  // Indicate start of operation
        *SMSC9118_MAC_CSR_CMD = maccmd;

	start = get_timer (0);
        do {
	    timedout = get_timer(start) > MS50 ? 1 : 0;
            read = *SMSC9118_BYTE_TEST;     // A no-op read.
        } while(!timedout && (*SMSC9118_MAC_CSR_CMD & ((unsigned int)1 << 31)));

        if(timedout) {
            printf("Error: SMSC9118 MAC CSR write operation timed out.\n");
            error = 1;
            return error;
        }
    } else {
        printf("Warning: SMSC9118 MAC CSR is busy. No data written.\n");
    }
    return 0;
}

static int smsc9118_phy_regread(unsigned char regoffset, unsigned short *data)
{
    unsigned int val, phycmd; int error;
    int timedout;
    ulong start;

    error = 0;

    smsc9118_mac_regread(SMSC9118_MAC_MII_ACC, &val);

    if(!(val & 1)) {				// Not busy
        phycmd = 0;
        phycmd |= (1 << 11);			// 1 to [15:11]
        phycmd |= ((regoffset & 0x1F) << 6);	// Put regoffset to [10:6]
        phycmd &= ~(1 << 1);			// Clear [1] indicates read.
        phycmd |= (1 << 0);			// Set [0] indicates operation start

        smsc9118_mac_regwrite(SMSC9118_MAC_MII_ACC, phycmd);

	        val = 0;
        timedout = 0;
	start = get_timer (0);
        do {
	    timedout = get_timer(start) > MS50 ? 1 : 0;
            smsc9118_mac_regread(SMSC9118_MAC_MII_ACC,&val);
        } while(!timedout && (val & ((unsigned int)1 << 0)));

        if(timedout) {
            printf("Error: SMSC9118 MAC MII read operation timed out.\n");
            error = 1;
            return error;
        }
        smsc9118_mac_regread(SMSC9118_MAC_MII_DATA, &val);

    } else {
        printf("Warning: SMSC9118 MAC MII is busy. No data read.\n");
        val = 0;
    }
    *data = (unsigned short)(val & 0xFFFF);
    return 0;
}

static int smsc9118_phy_regwrite(unsigned char regoffset, unsigned short data)
{
    unsigned int val, phycmd, u32data; int error;
    int timedout;
    ulong start;

    u32data = (unsigned int)data;

    debug("PHY[%02d] write 0x%08x \n", regoffset, u32data);
    smsc9118_mac_regread(SMSC9118_MAC_MII_ACC, &val);

    if(!(val & 1)) {    // Not busy
        smsc9118_mac_regwrite(SMSC9118_MAC_MII_DATA, u32data); // Load the data
        phycmd = 0;
        phycmd |= (1 << 11);                    // 1 to [15:11]
        phycmd |= ((regoffset & 0x1F) << 6);     // Put regoffset to [10:6]
        phycmd |= (1 << 1);                     // Set [1] indicates write.
        phycmd |= (1 << 0);                     // Set [0] indicates operation start
        smsc9118_mac_regwrite(SMSC9118_MAC_MII_ACC, phycmd);   // Start operation

        phycmd = 0;
        timedout = 0;

	start = get_timer (0);
        do {
	    timedout = get_timer(start) > MS50 ? 1 : 0;
            smsc9118_mac_regread(SMSC9118_MAC_MII_ACC, &phycmd);
        } while(!timedout && (phycmd & (1 << 0)));

        if(timedout) {
            printf("Error: SMSC9118 MAC MII write operation timed out.\n");
            error = 1;
            return error;
        }

    } else {
        printf("Warning: SMSC9118 MAC MII is busy. No data written.\n");
    }
    return 0;
}

/*
 ------------------------------------------------------------
 .
 . Internal routines
 .
 ------------------------------------------------------------
*/
/*
 * Functions left as in/outu16, even for ARM where ARM WORD == 32 bits == (2* Other Architecture Word)
 * until we decide we can't use the common functions.....
 */
#ifdef CONFIG_SMC_USE_IOFUNCS
/*
 * input and output functions
 * - the access macros defined in smc9118.h may not 
 *   work for other boards - they have only been tested on 
 *   ARM RealViewEB Revision D boards 
 *
 *   This code is copied from smc91111.c (functions & types renamed),
 *   however 16 bit access may be different for SMC9118 
 *   - see the datasheet & test well before use
 *
 */
# if defined(CONFIG_SMC_USE_32_BIT)
	32 bit access functions not yet provided
# else
static inline u16 SMC9118_inu16(u32 offset);
static inline void SMC9118_outu16(u16 value, u32 offset);
static inline u8 SMC9118_inu8(u32 offset);
static inline void SMC9118_outu8(u8 value, u32 offset);
static inline void SMC9118_ins16(u32 offset, volatile uchar* buf, u32 len);
static inline void SMC9118_outs16(u32 offset, uchar* buf, u32 len);

#define barrier() __asm__ __volatile__("": : :"memory")

#define SMC9118_BASE_ADDRESS CONFIG_SMC_RV_BASE

static inline u16 SMC9118_inu16(u32 offset)
{
	u16 v;
	v = *((volatile u16*)(SMC9118_BASE_ADDRESS+offset));
	barrier(); *(volatile u32*)(0xc0000000);
	return v;
}

static inline void SMC9118_outu16(u16 value, u32 offset)
{
	*((volatile u16*)(SMC9118_BASE_ADDRESS+offset)) = value;
	barrier(); *(volatile u32*)(0xc0000000);
}

static inline u8 SMC9118_inu8(u32 offset)
{
	u16  _w;

	_w = SMC9118_inu16(offset & ~((u32)1));
	return (offset & 1) ? (u8)(_w >> 8) : (u8)(_w);
}

static inline void SMC9118_outu8(u8 value, u32 offset)
{
	u16  _w;

	_w = SMC9118_inu16(offset & ~((u32)1));
	if (offset & 1)
			*((volatile u16*)(SMC9118_BASE_ADDRESS+(offset & ~((u32)1)))) = (value<<8) | (_w & 0x00ff);
	else
			*((volatile u16*)(SMC9118_BASE_ADDRESS+offset)) = value | (_w & 0xff00);
}

static inline void SMC9118_ins16(u32 offset, volatile uchar* buf, u32 len)
{
	volatile u16 *p = (volatile u16 *)buf;

	while (len-- > 0) {
		*p++ = SMC9118_inu16(offset);
		barrier();
		*((volatile u32*)(0xc0000000));
	}
}

static inline void SMC9118_outs16(u32 offset, uchar* buf, u32 len)
{
	volatile u16 *p = (volatile u16 *)buf;

	while (len-- > 0) {
		SMC9118_outu16(*p++, offset);
		barrier();
		*(volatile u32*)(0xc0000000);
	}
}
# endif
#endif  /* CONFIG_SMC_USE_IOFUNCS */

// Returns smsc9118 id.
static unsigned int smsc9118_read_id(void)
{
    return *SMSC9118_ID_REV;
}

static int smsc9118_check_id(void)
{
    int error = 0;
    unsigned int id;

    id = smsc9118_read_id();

    switch(((id >> 16) & 0xFFFF)) {
        case 0x118:
            // If bottom and top halves of the words are the same
	    // then the bus mode is incorrect
            if(((id >> 16) & 0xFFFF) == (id & 0xFFFF)) {
                printf("Error: The SMSC9118 bus is in 16-bit mode. 32-bit mode was expected.\n");
                error = 1;
            } 
            break;

        default:
            printf("Error: SMSC9118 id reads: 0x%08x, either an unknown chip, or error.\n",id);
            error = 1;
            break;
    }

    if((id & 0xFFFF) == 0) {
        printf("Error: This test is not intended for this chip revision.\n");
        error = 1;
    }

    return error;
}

// Initiates a soft reset, returns failure or success.
static __inline int smsc9118_soft_reset(void)
{
    int timedout = 0;
    ulong start;

    // Soft reset
    *SMSC9118_HW_CFG |= 1;

    // Wait
    start = get_timer(0);
    while (!timedout && (*SMSC9118_HW_CFG & 1)){
    	timedout = get_timer(start) < MS10 ? 0 : 1;
    }

    return timedout;
}


static 
__inline 
void smsc9118_set_txfifo(unsigned int val)
{
    // 2kb minimum, 14kb maximum
    if(val < 2 || val > 14)
        return;

    *SMSC9118_HW_CFG = val << 16;
}

static int smsc9118_wait_eeprom(void)
{
    int timedout = 0;
    // Wait
    ulong start = get_timer(0);
    while (!timedout && (*SMSC9118_E2P_CMD & ((unsigned int) 1 << 31))){
    	timedout = get_timer(start) < MS50 ? 0 : 1;
    }

    return timedout;
}

static 
__inline 
void smsc9118_init_irqs(void)
{
	*SMSC9118_INT_EN    = 0;
	*SMSC9118_INT_STS   = 0xFFFFFFFF;
	*SMSC9118_IRQ_CFG   = 0x22000100;   //irq deassertion at 220 usecs.
}

/* 
 * If ethaddr environment variable has a valid values, set the MAC address to it,
 * otherwise check that the MAC address loaded in the smc9118 is valid
 * Note that we do not change the value of the MAC address stored in the smc9118 EEPROM (the auto-loaded MAC)
 *
 * Datasheet has 12:34:56:78:9A:BC stored as ADDRH 0xBC9A
 *		 ADDRH 0x....BC9A
 *		 ADDRL 0x12345678
 *		 0x12 transmitted first
 * This code gets them in the same order in the TX buffer as the smc91111
 *
 * Returns 1 on success
 */
int smc9118_set_valid_ethaddr(void){
	unsigned int mac_low;
	unsigned int mac_high;
	uchar mac[6];
	int env_size = 0, env_present = 0, reg;
	char *s = NULL, *endp, dummy_mac[] = "11:22:33:44:55:66";
	char s_env_mac[0x18];

	/* Try for ethadd from environment */
	env_size = getenv_r ("ethaddr", s_env_mac, sizeof (s_env_mac));
	if ((env_size > 0) && (env_size != sizeof (dummy_mac))) {	/* exit if env is bad */
		printf ("\n*** ERROR: ethaddr is not set properly!!\n");
	} else {
		env_present = 1;
	}

	if (env_present) {
		s = s_env_mac;

		/* Environment string to mac numbers */
		for (reg = 0; reg < 6; ++reg) { 
			mac[reg] = s ? simple_strtoul (s, &endp, 16) : 0;
			if (s)
				s = (*endp) ? endp + 1 : endp;
		}
		mac_high =                                             (mac[5] * 0x100) + mac[4];
		mac_low =  (mac[3] * 0x1000000) + (mac[2] * 0x10000) + (mac[1] * 0x100) + mac[0];
		debug("MAC Address environment                   value hi:0x%08x lo:0x%08x\n", mac_high, mac_low);
	} else {
		/* read from smc9118 */
		if(*SMSC9118_E2P_CMD & 1) {
        		// Read current auto-loaded mac address.
    			smsc9118_mac_regread(SMSC9118_MAC_ADDRH, &mac_high);
    			smsc9118_mac_regread(SMSC9118_MAC_ADDRL, &mac_low);
			env_present = 1;
			debug("MAC Address Auto Load                     value hi:0x%08x lo:0x%08x\n", mac_high, mac_low);
		} else {
			printf("This board has no MAC Address Auto Load\n");
		}
	}
	if(env_present){
		/* Set the MAC address int the smc9118 registers */
		debug("MAC address in chip is about to be set to value hi:0x%08x lo:0x%08x\n", mac_high, mac_low);
		smsc9118_mac_regwrite(SMSC9118_MAC_ADDRH, mac_high);
		smsc9118_mac_regwrite(SMSC9118_MAC_ADDRL, mac_low);
	}
	return env_present; 
}

static __inline int smsc9118_check_phy(void)
{
    unsigned short phyid1, phyid2;

    smsc9118_phy_regread(SMSC9118_PHY_ID1,&phyid1);
    smsc9118_phy_regread(SMSC9118_PHY_ID2,&phyid2);
    debug("PHY ID1: 0x%08x, PHY ID2: 0x%08x\n\n",phyid1, phyid2);
    return ((phyid1 == 0xFFFF && phyid2 == 0xFFFF) ||
            (phyid1 == 0x0 && phyid2 == 0x0));
}
static __inline int smsc9118_reset_phy(void)
{
	unsigned short read;
	int error;

	debug("smsc9118_reset_phy()\n");

	error = 0;

	if(smsc9118_phy_regread(SMSC9118_PHY_BCONTROL, &read)) {
		printf("Error: PHY BCONTROL read failed.\n");
		error = 1;
	} else {

		read |= (1 << 15);
		if(smsc9118_phy_regwrite(SMSC9118_PHY_BCONTROL, read)) {
			printf("Error: PHY BCONTROL write failed.\n");
			error = 1;
		}
	}
	return error;
}

/* Advertise all speeds and pause capabilities */
static __inline void smsc9118_advertise_cap(void)
{
    unsigned short aneg_adv;
    aneg_adv = 0;

    smsc9118_phy_regread(SMSC9118_PHY_ANEG_ADV, &aneg_adv);
    debug("advertise_cap: PHY_ANEG_ADV before write: 0x%08x\n",aneg_adv);
    aneg_adv |= 0xDE0;

    smsc9118_phy_regwrite(SMSC9118_PHY_ANEG_ADV, aneg_adv);
    smsc9118_phy_regread(SMSC9118_PHY_ANEG_ADV, &aneg_adv);
    debug("advertise_cap: PHY_ANEG_ADV: after write: 0x%08x\n",aneg_adv);
    return;
}
static 
__inline 
void smsc9118_establish_link(void)
{
    unsigned short bcr;

    smsc9118_phy_regread(SMSC9118_PHY_BCONTROL, &bcr);
    debug("establish link: PHY_BCONTROL before write: 0x%08x\n",bcr);
    bcr |= (1 << 12) | (1 << 9);
    smsc9118_phy_regwrite(SMSC9118_PHY_BCONTROL, bcr);
    smsc9118_phy_regread(SMSC9118_PHY_BCONTROL, &bcr);
    debug("establish link: PHY_BCONTROL after write: 0x%08x\n", bcr);

    {
        unsigned int hw_cfg;

        hw_cfg = 0;
        hw_cfg = *SMSC9118_HW_CFG;

        hw_cfg &= 0xF0000;
        hw_cfg |= (1 << 20);
        *SMSC9118_HW_CFG = hw_cfg;
    }

    return;
}
static 
__inline 
void smsc9118_enable_xmit(void)
{
    *SMSC9118_TX_CFG = 0x2; // Enable transmission
    return;
}

static __inline void smsc9118_enable_mac_xmit(void)
{
    unsigned int mac_cr;

    mac_cr = 0;
    smsc9118_mac_regread(SMSC9118_MAC_CR, &mac_cr);

    mac_cr |= (1 << 3);     // xmit enable
    mac_cr |= (1 << 28);    // Heartbeat disable

    smsc9118_mac_regwrite(SMSC9118_MAC_CR, mac_cr);
    return;
}

static __inline void smsc9118_enable_mac_recv(void)
{
    unsigned int mac_cr;

    mac_cr = 0;
    smsc9118_mac_regread(SMSC9118_MAC_CR, &mac_cr);
    mac_cr |= (1 << 2);     // Recv enable
    smsc9118_mac_regwrite(SMSC9118_MAC_CR, mac_cr);

    return;
}


static __inline void smsc9118_disable_mac_xmit(void)
{
    unsigned int mac_cr;

    mac_cr = 0;
    smsc9118_mac_regread(SMSC9118_MAC_CR, &mac_cr);

    mac_cr &= ~(1 << 3);     // xmit enable
    mac_cr &= ~(1 << 28);    // Heartbeat disable

    smsc9118_mac_regwrite(SMSC9118_MAC_CR, mac_cr);
    return;
}
static __inline void smsc9118_disable_xmit(void)
{
    *SMSC9118_TX_CFG = 0; // Disable trasmission
    return;
}

static __inline void smsc9118_disable_mac_recv(void)
{
    unsigned int mac_cr;

    mac_cr = 0;
    smsc9118_mac_regread(SMSC9118_MAC_CR, &mac_cr);
    mac_cr &= ~(1 << 2);     // Recv enable
    smsc9118_mac_regwrite(SMSC9118_MAC_CR, mac_cr);

    return;
}



#define SMC9118_DEFAULT_TXFIFO_KB	5
/*
 * Note that the datasheet states that an interrupt status bits is set regardless
 * of the setting of the interrupt enable bit for that interrupt.
 * i.e. we don't have to enable interrupts for a polled driver.
 */
static int smsc9118_initialise(void)
{
	int error = 1;
	ulong start;

	if(smsc9118_check_id()) {
		printf("Reading the Ethernet ID register failed.\n"
		"Check that a SMSC9118 device is present on the system @ %p.\n", (void*)CONFIG_SMC_RV_BASE);
	} else if(smsc9118_soft_reset()) {
		printf("Error: SMSC9118 soft reset failed to complete.\n");
	} else {
		smsc9118_set_txfifo(SMC9118_DEFAULT_TXFIFO_KB);

		// Sets automatic flow control thresholds, and backpressure
		// threshold to defaults specified.
		*SMSC9118_AFC_CFG = 0x006E3740;

		if(smsc9118_wait_eeprom()) {
			printf("Error: EEPROM failed to finish initialisation.\n");
		} else {

			// Configure GPIOs as LED outputs.
			*SMSC9118_GPIO_CFG = 0x70070000;

			smsc9118_init_irqs();
		
			/* Configure MAC addresses */
			// Set from environment if present
			// else check valid
			if(smc9118_set_valid_ethaddr()){

				if(smsc9118_check_phy()) {
					printf("Error: SMSC9118 PHY not present.\n");
				} else {

					if(smsc9118_reset_phy()) {
						printf("Error: SMSC9118 PHY reset failed.\n");
					} else {
						unsigned short phyreset = 0;

						// Wait
						start = get_timer(0);
						while (get_timer(start) < MS100){
							;
						}

						// Checking whether phy reset completed successfully.
						smsc9118_phy_regread(SMSC9118_PHY_BCONTROL, &phyreset);
						if(phyreset & (1 << 15)) {
							printf("Error: SMSC9118 PHY reset stage failed to complete.\n");
						} else {

							/* Advertise capabilities */
							smsc9118_advertise_cap();

							/* Begin to establish link */
							smsc9118_establish_link();      // bit [12] of BCONTROL seems self-clearing.
							// Although it's not so in the manual.

							/* Interrupt threshold */
							*SMSC9118_FIFO_INT = 0xFF000000;

							smsc9118_enable_mac_xmit();

							smsc9118_enable_xmit();

							*SMSC9118_RX_CFG = 0;

							smsc9118_enable_mac_recv();

							// Rx status FIFO level irq threshold
							*SMSC9118_FIFO_INT &= ~(0xFF);  // Clear 2 bottom nibbles

							// This spin is compulsory otherwise txmit/receive will fail.
							start = get_timer(0);
							while (get_timer(start) < MS2000){
								;
							}
							error = 0;
						}
					}
				}
			}
		}
	}

	return error;
}

int smsc9118_recv_packet(unsigned int *recvbuf, int *index)
{
    unsigned int rxfifo_inf;    // Tells us the status of rx payload and status fifos.
    unsigned int rxfifo_stat;

    unsigned int pktsize;
    unsigned int dwords_to_read;

    debug("recv_packet start: recvbuf: 0x%08x index: %d\n",
            (unsigned int)recvbuf,*index);

    rxfifo_inf = *SMSC9118_RX_FIFO_INF;

    if(rxfifo_inf & 0xFFFF) { // If there's data
        rxfifo_stat = *SMSC9118_RX_STAT_PORT;
        if(rxfifo_stat != 0) {   // Fetch status of this packet
            pktsize = ((rxfifo_stat >> 16) & 0x3FFF);
            // debug("recv_packet: rxfifo_stat: 0x%08x, pktsize (bytes): %u\n",rxfifo_stat, pktsize);
	    if(*SMSC9118_INT_STS & 0x4000){
	    	printf("Status read RXE is %d\n", *SMSC9118_INT_STS & 0x4000 ? 1 : 0);
	    	*SMSC9118_INT_STS = 0xFFFFFFFF;
	    }
            if(rxfifo_stat & (1 << 15)) {
                printf("Error occured during receiving of packets on the bus.\n");
                return 1;
            } else {
                /* Below formula (recommended by SMSC9118 code)
                 * gives 1 more than required. This is because
                 * a last word is needed for not word aligned packets.
                 */
                dwords_to_read = (pktsize + sizeof(unsigned int) - 1) >> 2;
                debug("recv_packet: dwords_to_read: %u\n",dwords_to_read);
                // PIO copy of data received:
                while((*SMSC9118_RX_FIFO_INF & 0x0000FFFF) && (dwords_to_read > 0)) {
                    recvbuf[*index] = *SMSC9118_RX_DATA_PORT;
                    // debug("recv_packet: Received word[%d]: 0x%08x\n",*index,recvbuf[*index]);
                    (*index)++;
                    dwords_to_read--;
                }
	        if(*SMSC9118_INT_STS & 0x4000){
			printf("Data read RXE is %d\n", *SMSC9118_INT_STS & 0x4000 ? 1 : 0);
	    		*SMSC9118_INT_STS = 0xFFFFFFFF;
		}
            }
        } else {
            printf("Error: rx fifo status reads zero where data is available.\n");
            return 1;
        }
    } else {
        // printf("Error: No data available in rx FIFO\n");
        return 1;
    }

    return 0;
}
// Does the actual transfer of data to FIFO, note it does no
// fifo availability checking. This should be done by caller.
// ASSUMES the whole frame is transferred at once as a single segment
// ASSUMES chip auto pads to minimum ethernet packet length
void smsc9118_xfer_to_txFIFO(unsigned char * pkt, unsigned int length)
{
    unsigned int txcmd_a, txcmd_b;
    unsigned int dwords_to_write;
    volatile unsigned int dwritten;
    unsigned int *pktptr;

    pktptr = (unsigned int *) pkt;
    
    txcmd_a = 0;
    txcmd_b = 0;

    txcmd_a |= (1 << 12) | (1 << 13);   // First and last segments
    txcmd_a |= length & 0x7FF;          // [10:0] contains length

    txcmd_b |= ((length & 0xFFFF) << 16); // [31:16] contains length, rather than tag
    txcmd_b |= length & 0x7FF;          // [10:0] also contains length

    debug("txcmd_a: 0x%08x\n", txcmd_a);
    debug("txcmd_b: 0x%08x\n", txcmd_b);

    *SMSC9118_TX_DATA_PORT = txcmd_a;
    *SMSC9118_TX_DATA_PORT = txcmd_b;
    dwritten = dwords_to_write = (length + sizeof(unsigned int) - 1) >> 2;


    // PIO Copy to FIFO. Could replace this with DMA.
    while(dwords_to_write > 0) {
         *SMSC9118_TX_DATA_PORT = *pktptr;
         debug("Transmitting word[%d]: 0x%08x\n",dwritten-dwords_to_write,*pktptr);
         pktptr++;
         dwords_to_write--;
    }

    {
    	/* 
	 * Note that the read after read timings from table 6.2 are not used
	 */
        volatile unsigned int xmit_stat, xmit_stat2, xmit_inf;
        int i;
        xmit_stat = *SMSC9118_TX_STAT_PORT;
        debug("Finished transfer. TX_STATUS_WORD: 0x%08x\n",xmit_stat);
        xmit_stat2 = *SMSC9118_TX_STAT_PORT;
        xmit_inf = *SMSC9118_TX_FIFO_INF;
        debug("After popping TX_STAT: %08x, TX_INF: 0x%08x\n\n",xmit_stat2, xmit_inf);

	// ASSUMES that the register is emptied
	// - doesn't seem to do so in real life.......
        if(xmit_stat2 != 0 ) {
            debug("The second read of TX_STAT is non-zero. Retry reading a few more times.\n");
            for(i = 0; i < 6; i++) {
                xmit_stat2 = *SMSC9118_TX_STAT_PORT;
                debug("Retry %d: TX_STAT: 0x%08x\n",i+1,xmit_stat2);
            }
        }
    }
}

int smsc9118_xmit_packet(unsigned char * pktbuf, int pktsize)
{
    unsigned int txfifo_inf;

    txfifo_inf = *SMSC9118_TX_FIFO_INF;
    debug("TX_FIFO_INF: 0x%08x\n", txfifo_inf);

    if((txfifo_inf & 0xFFFF) >= pktsize) {
        smsc9118_xfer_to_txFIFO(pktbuf, pktsize);
    } else {
        printf("Insufficient tx fifo space for packet size %d\n",pktsize);
        return 1;
    }
    return 0;
}
// 9118 code end

// Merged functions

/*
 * Ethernet API
 */
int eth_init(bd_t *bd) {
	int retval = 0;

#ifdef SHARED_RESOURCES
	swap_to(ETHERNET);
#endif
	if(LAN_INVALID == chip_type){
		chip_type = get_chip_type();
	}
	switch (chip_type){
	case LAN91C111:	
		retval = smc_open(bd);
		break;

	case LAN9118:	
		retval = smsc9118_initialise();
		break;
	}

	return retval;
}


void eth_halt() {
	if(LAN_INVALID == chip_type){
		chip_type = get_chip_type();
	}
	switch(chip_type){
	case LAN91C111:
		smc_close();
		break;

	case LAN9118:
		smsc9118_disable_xmit();
		smsc9118_disable_mac_xmit();
		smsc9118_disable_mac_recv();
		break;
	}
}

int eth_rx() {
	int retval = 0;
	if(LAN_INVALID == chip_type){
		chip_type = get_chip_type();
	}
	switch(chip_type){
	case LAN91C111:
		retval = smc_rcv();
		break;

	case LAN9118:
	{
		int index = 0;
    
    		int rxfifo_inf = *SMSC9118_RX_FIFO_INF;
		/*
		 *  Any status info???
		 */
		if(rxfifo_inf & 0x00FF0000){

			smsc9118_recv_packet((unsigned int *)NetRxPackets[0], &index);
			if(index) {
				NetReceive(NetRxPackets[0], index * 4);
			}
		}
		retval = 4 * index;

		break;
	}
	}
	return retval;

}

int eth_send(volatile void *packet, int length) {
	int retval = 0;
	if(LAN_INVALID == chip_type){
		chip_type = get_chip_type();
	}
	switch(chip_type){
	case LAN91C111:
		retval = smc_send_packet(packet, length);
		break;
	
	case LAN9118:
	{
		int result = 0;
		int i;
		debug("Raw packet as %d decimal bytes:\n", length);
		for(i= 0; i < length; i++){
			debug(" %02x ", ((char*)packet)[i]);
			if(!((i+1)%8)){										    
				debug("\n");									    
			}											    
		}												    
		debug("\n");											    
		if(!smsc9118_xmit_packet((unsigned char *)packet, length)){
			result = length;
		} 
		retval = result;
		break;
	}
	}
	return retval;
}


#endif /* CONFIG_DRIVER_SMC_RV */
