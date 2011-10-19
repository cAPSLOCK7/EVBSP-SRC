/*------------------------------------------------------------------------
 . smc9118.c
 . This is a driver for SMSC's LAN9118 single-chip Ethernet device.
 .
 . (C) Copyright 2006 ARM Ltd. <www.arm.com>
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
 . Device information contained in this file was obtained from the LAN9118
 . manual from SMC <www.smsc.com>.
 .
 . author:
 .	Peter Pearse				( peter.pearse@arm.com)
 .
 . Sources:
 .    o	  SMSC LAN9118 databook (www.smsc.com)
 .    o	  drivers/smc91111.c
 .
 . History:
 .	2006.10.24	Peter Pearse	Initial version based on drivers/smc91111.c	
 .                                      - Dropped board specific code
 .                                      - Dropped #if 0 code, 
 .                                        it's in smc91111.c if you want it 
 .                                      - Made all but interface functions static
 .                                      - Add in ARM smc9118 code
 ----------------------------------------------------------------------------*/
#include <common.h>
#include <command.h>
#include <config.h>
#include "smc9118.h"
#include <net.h>

#ifdef CONFIG_DRIVER_SMC9118

#define SMC9118_DEV_NAME "SMC9118"

#ifndef CONFIG_SMC9118_BASE
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

    for  (i = (volatile unsigned int *)CONFIG_SMC9118_BASE; (int)i < (int)SMSC9118_RESERVED3; i++){
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

/*-----------------------------------------------------------------
 .
 .  The driver can be entered at any of the following entry points.
 .
 .------------------------------------------------------------------  */
int  eth_init(bd_t *bd);
void eth_halt(void);
int  eth_rx(void);
int  eth_send(volatile void *packet, int length);

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

#define SMC9118_BASE_ADDRESS CONFIG_SMC9118_BASE

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
    int error;
    unsigned int id;
    error = 0;

    id = smsc9118_read_id();

    switch(((id >> 16) & 0xFFFF)) {
        case 0x118:
            // If bottom and top halves of the words are the same
            if(((id >> 16) & 0xFFFF) == (id & 0xFFFF)) {
                printf("Error: The SMSC9118 bus is in 16-bit mode. 32-bit mode was expected.\n");
                error = 1;
                return error;
            } else {
                printf("SMSC9118 is identified successfully.\n");
                break;
            }

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
	} else {
		/* read from smc9118 */
		if(*SMSC9118_E2P_CMD & 1) {
        		// Read current auto-loaded mac address.
    			smsc9118_mac_regread(SMSC9118_MAC_ADDRH, &mac_high);
    			smsc9118_mac_regread(SMSC9118_MAC_ADDRL, &mac_low);
			env_present = 1;
		}
	}
	if(env_present){
		/* Set the MAC address int the smc9118 registers */
		debug("MAC address is about to be set to high 0x%08x low 0x%08x\n", mac_high, mac_low);
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
		"Check that a SMSC9118 device is present on the system @ %p.\n", (void*)CONFIG_SMC9118_BASE);
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

/*
 * Ethernet API
 */
int eth_init(bd_t *bd) {
#ifdef SHARED_RESOURCES
	swap_to(ETHERNET);
#endif
       return smsc9118_initialise();
}

void eth_halt() {
	PRINTK2("%s: eth_halt\n", SMC9118_DEV_NAME);
	smsc9118_disable_xmit();
	smsc9118_disable_mac_xmit();
	smsc9118_disable_mac_recv();
}

int eth_rx() {
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
	return 4 * index;
}

int eth_send(volatile void *packet, int length) {
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
	return result;
}


#endif /* CONFIG_DRIVER_SMC9118 */

