/*------------------------------------------------------------------------
 . smc9118.h
 . Macros for the LAN9118 Ethernet Driver
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
 . MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 . GNU General Public License for more details.
 .
 . You should have received a copy of the GNU General Public License
 . along with this program; if not, write to the Free Software
 . Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 .
 . This file contains register information and access macros for
 . the LAN9118 single chip ethernet controller.  It is a modified
 . version of the smc91111.h file.
 .
 . author:
 .	Peter Pearse				( peter.pearse@arm.com)
 .
 . Sources:
 .    o	  SMSC LAN9118 databook (www.smsc.com)
 .    o	  drivers/smc1111.c
 .
 . History:
 .	2006.10.24	Peter Pearse	Initial version based on drivers/smc91111.c	
 .					- Dropped board specific code
 .					- Dropped #if 0 code 
 .                                        - it's in smc91111.h if you want it
 .					- Dropped 91111 implementation specific code
 .                                      - Dropped 16bit word & 32bit dword types
 .                                        as confusing for ARM 32bit word users.
 .					32 bit, macro access only.
 ---------------------------------------------------------------------------*/
#ifndef _SMC9118_H_
#define _SMC9118_H_

#include <asm/types.h>
#include <config.h>


/*
 *  Timeouts
 */
#define MS10     10
#define MS50     50
#define MS100   100
#define MS2000 2000

/*
 . DEBUGGING LEVELS
 .
 . 0 for normal operation
 . 1 for slightly more details
 . >2 for various levels of increasingly useless information
 .    2 for interrupt tracking, status flags
 .    3 for packet info
 .    4 for complete packet dumps
*/
/*#define SMC9118_DBG 0 */

/*
 * These macros may not work on some boards
 * Also the datasheet states:
 *
 * "32 bit access is the native environment for the LAN9118 Ethernet controller"
 *
 */
#ifndef CONFIG_SMC_USE_IOFUNCS 

	#if defined(CONFIG_SMC_USE_32_BIT)

		#define	SMC9118_inu32(reg) 	(*((volatile u32 *)(reg)))

		#define SMC9118_ins32(reg,base,len) 	({	int __i ;  \
					u32 *__b2;  \
			    		__b2 = (u32 *) base;  \
			    		for (__i = 0; __i < len; __i++) {  \
					  *(__b2 + __i) = SMC9118_inu32(reg);  \
					  SMC9118_inu32(0);  \
					};  \
				})

		#define	SMC9118_outu32(data,reg)	(*((volatile u32 *)(reg)) = data)
		#define SMC9118_outs32(reg,base,len)	({	int __i; \
					u32 *__b2; \
					__b2 = (u32 *) base; \
					for (__i = 0; __i < len; __i++) { \
					    SMC9118_outu32( *(__b2 + __i), r); \
					} \
				})
  	#else
		#ifdef CONFIG_DRIVER_SMC9118
		16 bit access macros have not been provided
		#endif
	#endif /* CONFIG_SMC_USE_32_BIT */
#else
	#ifdef CONFIG_DRIVER_SMC9118
	Access functions have not been provided
	#endif
#endif  /* CONFIG_USE_IOFUNCS */


/*---------------------------------------------------------------
 . SMSC registers
 -----------------------------------------------------------------------*/
/* Transmit Control Register */
#define	TCR_REG 	0x0000 	/* transmit control register */
#define TCR_ENABLE	0x0001	/* When 1 we can transmit */
#define TCR_LOOP	0x0002	/* Controls output pin LBK */
#define TCR_FORCOL	0x0004	/* When 1 will force a collision */
#define TCR_PAD_EN	0x0080	/* When 1 will pad tx frames < 64 bytes w/0 */
#define TCR_NOCRC	0x0100	/* When 1 will not append CRC to tx frames */
#define TCR_MON_CSN	0x0400	/* When 1 tx monitors carrier */
#define TCR_FDUPLX    	0x0800  /* When 1 enables full duplex operation */
#define TCR_STP_SQET	0x1000	/* When 1 stops tx if Signal Quality Error */
#define	TCR_EPH_LOOP	0x2000	/* When 1 enables EPH block loopback */
#define	TCR_SWFDUP	0x8000	/* When 1 enables Switched Full Duplex mode */

#define	TCR_CLEAR	0	/* do NOTHING */
/* the default settings for the TCR register : */
/* QUESTION: do I want to enable padding of short packets ? */
#define	TCR_DEFAULT  	TCR_ENABLE


/* EPH Status Register */
#define EPH_STATUS_REG	0x0002
#define ES_TX_SUC	0x0001	/* Last TX was successful */
#define ES_SNGL_COL	0x0002	/* Single collision detected for last tx */
#define ES_MUL_COL	0x0004	/* Multiple collisions detected for last tx */
#define ES_LTX_MULT	0x0008	/* Last tx was a multicast */
#define ES_16COL	0x0010	/* 16 Collisions Reached */
#define ES_SQET		0x0020	/* Signal Quality Error Test */
#define ES_LTXBRD	0x0040	/* Last tx was a broadcast */
#define ES_TXDEFR	0x0080	/* Transmit Deferred */
#define ES_LATCOL	0x0200	/* Late collision detected on last tx */
#define ES_LOSTCARR	0x0400	/* Lost Carrier Sense */
#define ES_EXC_DEF	0x0800	/* Excessive Deferral */
#define ES_CTR_ROL	0x1000	/* Counter Roll Over indication */
#define ES_LINK_OK	0x4000	/* Driven by inverted value of nLNK pin */
#define ES_TXUNRN	0x8000	/* Tx Underrun */


/* Receive Control Register */
#define	RCR_REG		0x0004
#define	RCR_RX_ABORT	0x0001	/* Set if a rx frame was aborted */
#define	RCR_PRMS	0x0002	/* Enable promiscuous mode */
#define	RCR_ALMUL	0x0004	/* When set accepts all multicast frames */
#define RCR_RXEN	0x0100	/* IFF this is set, we can receive packets */
#define	RCR_STRIP_CRC	0x0200	/* When set strips CRC from rx packets */
#define	RCR_ABORT_ENB	0x0200	/* When set will abort rx on collision */
#define	RCR_FILT_CAR	0x0400	/* When set filters leading 12 bit s of carrier */
#define RCR_SOFTRST	0x8000 	/* resets the chip */

/* the normal settings for the RCR register : */
#define	RCR_DEFAULT	(RCR_STRIP_CRC | RCR_RXEN)
#define RCR_CLEAR	0x0	/* set it to a base state */

/* Counter Register */
#define	COUNTER_REG	0x0006

/* Memory Information Register */
#define	MIR_REG		0x0008

/* Receive/Phy Control Register */
#define	RPC_REG		0x000A
#define	RPC_SPEED	0x2000	/* When 1 PHY is in 100Mbps mode. */
#define	RPC_DPLX	0x1000	/* When 1 PHY is in Full-Duplex Mode */
#define	RPC_ANEG	0x0800	/* When 1 PHY is in Auto-Negotiate Mode */
#define	RPC_LSXA_SHFT	5	/* Bits to shift LS2A,LS1A,LS0A to lsb */
#define	RPC_LSXB_SHFT	2	/* Bits to get LS2B,LS1B,LS0B to lsb */
#define RPC_LED_100_10	(0x00)	/* LED = 100Mbps OR's with 10Mbps link detect */
#define RPC_LED_RES	(0x01)	/* LED = Reserved */
#define RPC_LED_10	(0x02)	/* LED = 10Mbps link detect */
#define RPC_LED_FD	(0x03)	/* LED = Full Duplex Mode */
#define RPC_LED_TX_RX	(0x04)	/* LED = TX or RX packet occurred */
#define RPC_LED_100	(0x05)	/* LED = 100Mbps link dectect */
#define RPC_LED_TX	(0x06)	/* LED = TX packet occurred */
#define RPC_LED_RX	(0x07)	/* LED = RX packet occurred */
#define RPC_DEFAULT	( RPC_SPEED | RPC_DPLX | RPC_ANEG	\
			| (RPC_LED_100_10 << RPC_LSXA_SHFT)	\
			| (RPC_LED_TX_RX << RPC_LSXB_SHFT)	)

/* Bank 0 0x000C is reserved */

/* Bank Select Register */
/* All Banks */
#define BSR_REG	0x000E


/* Configuration Reg */
#define CONFIG_REG	0x0000
#define CONFIG_EXT_PHY	0x0200	/* 1=external MII, 0=internal Phy */
#define CONFIG_GPCNTRL	0x0400	/* Inverse value drives pin nCNTRL */
#define CONFIG_NO_WAIT	0x1000	/* When 1 no extra wait states on ISA bus */
#define CONFIG_EPH_POWER_EN 0x8000 /* When 0 EPH is placed into low power mode. */

/* Default is powered-up, Internal Phy, Wait States, and pin nCNTRL=low */
#define CONFIG_DEFAULT	(CONFIG_EPH_POWER_EN)


/* Base Address Register */
#define	BASE_REG	0x0002


/* Individual Address Registers */
#define	ADDR0_REG	0x0004
#define	ADDR1_REG	0x0006
#define	ADDR2_REG	0x0008


/* General Purpose Register */
#define	GP_REG		0x000A


/* Control Register */
#define	CTL_REG		0x000C
#define CTL_RCV_BAD	0x4000 /* When 1 bad CRC packets are received */
#define CTL_AUTO_RELEASE 0x0800 /* When 1 tx pages are released automatically */
#define	CTL_LE_ENABLE	0x0080 /* When 1 enables Link Error interrupt */
#define	CTL_CR_ENABLE	0x0040 /* When 1 enables Counter Rollover interrupt */
#define	CTL_TE_ENABLE	0x0020 /* When 1 enables Transmit Error interrupt */
#define	CTL_EEPROM_SELECT 0x0004 /* Controls EEPROM reload & store */
#define	CTL_RELOAD	0x0002 /* When set reads EEPROM into registers */
#define	CTL_STORE	0x0001 /* When set stores registers into EEPROM */
#define CTL_DEFAULT     (0x1A10) /* Autorelease enabled*/

/* MMU Command Register */
#define MMU_CMD_REG	0x0000
#define MC_BUSY		1	/* When 1 the last release has not completed */
#define MC_NOP		(0<<5)	/* No Op */
#define	MC_ALLOC	(1<<5) 	/* OR with number of 256 byte packets */
#define	MC_RESET	(2<<5)	/* Reset MMU to initial state */
#define	MC_REMOVE	(3<<5) 	/* Remove the current rx packet */
#define MC_RELEASE  	(4<<5) 	/* Remove and release the current rx packet */
#define MC_FREEPKT  	(5<<5) 	/* Release packet in PNR register */
#define MC_ENQUEUE	(6<<5)	/* Enqueue the packet for transmit */
#define MC_RSTTXFIFO	(7<<5)	/* Reset the TX FIFOs */


/* Packet Number Register */
#define	PN_REG		0x0002


/* Allocation Result Register */
#define	AR_REG		0x0003
#define AR_FAILED	0x80	/* Alocation Failed */


/* RX FIFO Ports Register */
#define RXFIFO_REG	0x0004	/* Must be read as a u16 */
#define RXFIFO_REMPTY	0x8000	/* RX FIFO Empty */


/* TX FIFO Ports Register */
#define TXFIFO_REG	RXFIFO_REG	/* Must be read as a u16 */
#define TXFIFO_TEMPTY	0x80	/* TX FIFO Empty */


/* Pointer Register */
#define PTR_REG		0x0006
#define	PTR_RCV		0x8000 /* 1=Receive area, 0=Transmit area */
#define	PTR_AUTOINC 	0x4000 /* Auto increment the pointer on each access */
#define PTR_READ	0x2000 /* When 1 the operation is a read */
#define PTR_NOTEMPTY	0x0800 /* When 1 _do not_ write fifo DATA REG */


/* Data Register */
#define	SMC9118_DATA_REG	0x0008


/* Interrupt Status/Acknowledge Register */
#define	SMC9118_INT_REG	0x000C


/* Interrupt Mask Register */
#define IM_REG		0x000D
#define	IM_MDINT	0x80 /* PHY MI Register 18 Interrupt */
#define	IM_ERCV_INT	0x40 /* Early Receive Interrupt */
#define	IM_EPH_INT	0x20 /* Set by Etheret Protocol Handler section */
#define	IM_RX_OVRN_INT	0x10 /* Set by Receiver Overruns */
#define	IM_ALLOC_INT	0x08 /* Set when allocation request is completed */
#define	IM_TX_EMPTY_INT	0x04 /* Set if the TX FIFO goes empty */
#define	IM_TX_INT	0x02 /* Transmit Interrrupt */
#define IM_RCV_INT	0x01 /* Receive Interrupt */


/* Multicast Table Registers */
#define	MCAST_REG1	0x0000
#define	MCAST_REG2	0x0002
#define	MCAST_REG3	0x0004
#define	MCAST_REG4	0x0006


/* Management Interface Register (MII) */
#define	MII_REG		0x0008
#define MII_MSK_CRS100	0x4000 /* Disables CRS100 detection during tx half dup */
#define MII_MDOE	0x0008 /* MII Output Enable */
#define MII_MCLK	0x0004 /* MII Clock, pin MDCLK */
#define MII_MDI		0x0002 /* MII Input, pin MDI */
#define MII_MDO		0x0001 /* MII Output, pin MDO */


/* Revision Register */
#define	REV_REG		0x000A /* ( hi: chip id   low: rev # ) */


/* Early RCV Register */
/* this is NOT on SMC9192 */
#define	ERCV_REG	0x000C
#define ERCV_RCV_DISCRD	0x0080 /* When 1 discards a packet being received */
#define ERCV_THRESHOLD	0x001F /* ERCV Threshold Mask */

/* External Register */
#define	EXT_REG		0x0000


#define CHIP_9192	3
#define CHIP_9194	4
#define CHIP_9195	5
#define CHIP_9196	6
#define CHIP_91100	7
#define CHIP_91100FD	8
#define CHIP_91111FD	9

/*
 . Transmit status bits
*/
#define TS_SUCCESS 0x0001
#define TS_LOSTCAR 0x0400
#define TS_LATCOL  0x0200
#define TS_16COL   0x0010

/*
 . Receive status bits
*/
#define RS_ALGNERR	0x8000
#define RS_BRODCAST	0x4000
#define RS_BADCRC	0x2000
#define RS_ODDFRAME	0x1000	/* bug: the LAN9118 never sets this on receive */
#define RS_TOOLONG	0x0800
#define RS_TOOSHORT	0x0400
#define RS_MULTICAST	0x0001
#define RS_ERRORS	(RS_ALGNERR | RS_BADCRC | RS_TOOLONG | RS_TOOSHORT)


/* PHY Types */
enum {
	PHY_LAN83C183 = 1,	/* LAN9118 Internal PHY */
	PHY_LAN83C180
};


/* PHY Register Addresses (LAN9118 Internal PHY) */

/* PHY Control Register */
#define PHY_CNTL_REG		0x00
#define PHY_CNTL_RST		0x8000	/* 1=PHY Reset */
#define PHY_CNTL_LPBK		0x4000	/* 1=PHY Loopback */
#define PHY_CNTL_SPEED		0x2000	/* 1=100Mbps, 0=10Mpbs */
#define PHY_CNTL_ANEG_EN	0x1000 /* 1=Enable Auto negotiation */
#define PHY_CNTL_PDN		0x0800	/* 1=PHY Power Down mode */
#define PHY_CNTL_MII_DIS	0x0400	/* 1=MII 4 bit interface disabled */
#define PHY_CNTL_ANEG_RST	0x0200 /* 1=Reset Auto negotiate */
#define PHY_CNTL_DPLX		0x0100	/* 1=Full Duplex, 0=Half Duplex */
#define PHY_CNTL_COLTST		0x0080	/* 1= MII Colision Test */

/* PHY Status Register */
#define PHY_STAT_REG		0x01
#define PHY_STAT_CAP_T4		0x8000	/* 1=100Base-T4 capable */
#define PHY_STAT_CAP_TXF	0x4000	/* 1=100Base-X full duplex capable */
#define PHY_STAT_CAP_TXH	0x2000	/* 1=100Base-X half duplex capable */
#define PHY_STAT_CAP_TF		0x1000	/* 1=10Mbps full duplex capable */
#define PHY_STAT_CAP_TH		0x0800	/* 1=10Mbps half duplex capable */
#define PHY_STAT_CAP_SUPR	0x0040	/* 1=recv mgmt frames with not preamble */
#define PHY_STAT_ANEG_ACK	0x0020	/* 1=ANEG has completed */
#define PHY_STAT_REM_FLT	0x0010	/* 1=Remote Fault detected */
#define PHY_STAT_CAP_ANEG	0x0008	/* 1=Auto negotiate capable */
#define PHY_STAT_LINK		0x0004	/* 1=valid link */
#define PHY_STAT_JAB		0x0002	/* 1=10Mbps jabber condition */
#define PHY_STAT_EXREG		0x0001	/* 1=extended registers implemented */

/* PHY Identifier Registers */
#define PHY_ID1_REG		0x02	/* PHY Identifier 1 */
#define PHY_ID2_REG		0x03	/* PHY Identifier 2 */

/* PHY Auto-Negotiation Advertisement Register */
#define PHY_AD_REG		0x04
#define PHY_AD_NP		0x8000	/* 1=PHY requests exchange of Next Page */
#define PHY_AD_ACK		0x4000	/* 1=got link code u16 from remote */
#define PHY_AD_RF		0x2000	/* 1=advertise remote fault */
#define PHY_AD_T4		0x0200	/* 1=PHY is capable of 100Base-T4 */
#define PHY_AD_TX_FDX		0x0100	/* 1=PHY is capable of 100Base-TX FDPLX */
#define PHY_AD_TX_HDX		0x0080	/* 1=PHY is capable of 100Base-TX HDPLX */
#define PHY_AD_10_FDX		0x0040	/* 1=PHY is capable of 10Base-T FDPLX */
#define PHY_AD_10_HDX		0x0020	/* 1=PHY is capable of 10Base-T HDPLX */
#define PHY_AD_CSMA		0x0001	/* 1=PHY is capable of 802.3 CMSA */

/* PHY Auto-negotiation Remote End Capability Register */
#define PHY_RMT_REG		0x05
/* Uses same bit definitions as PHY_AD_REG */

/* PHY Configuration Register 1 */
#define PHY_CFG1_REG		0x10
#define PHY_CFG1_LNKDIS		0x8000	/* 1=Rx Link Detect Function disabled */
#define PHY_CFG1_XMTDIS		0x4000	/* 1=TP Transmitter Disabled */
#define PHY_CFG1_XMTPDN		0x2000	/* 1=TP Transmitter Powered Down */
#define PHY_CFG1_BYPSCR		0x0400	/* 1=Bypass scrambler/descrambler */
#define PHY_CFG1_UNSCDS		0x0200	/* 1=Unscramble Idle Reception Disable */
#define PHY_CFG1_EQLZR		0x0100	/* 1=Rx Equalizer Disabled */
#define PHY_CFG1_CABLE		0x0080	/* 1=STP(150ohm), 0=UTP(100ohm) */
#define PHY_CFG1_RLVL0		0x0040	/* 1=Rx Squelch level reduced by 4.5db */
#define PHY_CFG1_TLVL_SHIFT	2	/* Transmit Output Level Adjust */
#define PHY_CFG1_TLVL_MASK	0x003C
#define PHY_CFG1_TRF_MASK	0x0003	/* Transmitter Rise/Fall time */


/* PHY Configuration Register 2 */
#define PHY_CFG2_REG		0x11
#define PHY_CFG2_APOLDIS	0x0020	/* 1=Auto Polarity Correction disabled */
#define PHY_CFG2_JABDIS		0x0010	/* 1=Jabber disabled */
#define PHY_CFG2_MREG		0x0008	/* 1=Multiple register access (MII mgt) */
#define PHY_CFG2_INTMDIO	0x0004	/* 1=Interrupt signaled with MDIO pulseo */

/* PHY Status Output (and Interrupt status) Register */
#define PHY_INT_REG		0x12	/* Status Output (Interrupt Status) */
#define PHY_INT_INT		0x8000	/* 1=bits have changed since last read */
#define	PHY_INT_LNKFAIL		0x4000	/* 1=Link Not detected */
#define PHY_INT_LOSSSYNC	0x2000	/* 1=Descrambler has lost sync */
#define PHY_INT_CWRD		0x1000	/* 1=Invalid 4B5B code detected on rx */
#define PHY_INT_SSD		0x0800	/* 1=No Start Of Stream detected on rx */
#define PHY_INT_ESD		0x0400	/* 1=No End Of Stream detected on rx */
#define PHY_INT_RPOL		0x0200	/* 1=Reverse Polarity detected */
#define PHY_INT_JAB		0x0100	/* 1=Jabber detected */
#define PHY_INT_SPDDET		0x0080	/* 1=100Base-TX mode, 0=10Base-T mode */
#define PHY_INT_DPLXDET		0x0040	/* 1=Device in Full Duplex */

/* PHY Interrupt/Status Mask Register */
#define PHY_MASK_REG		0x13	/* Interrupt Mask */
/* Uses the same bit definitions as PHY_INT_REG */


// SMSC9118 FIFO Ports
#define SMSC9118_RX_DATA_PORT   (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0x0)
#define SMSC9118_TX_DATA_PORT   (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0x20)

// SMSC9118 FIFO status ports and peeks
// (Reads on ports destructive, reads on peeks have no side effect)
#define SMSC9118_RX_STAT_PORT   (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0x40)
#define SMSC9118_RX_STAT_PEEK   (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0x44)

#define SMSC9118_TX_STAT_PORT   (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0x48)
#define SMSC9118_TX_STAT_PEEK   (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0x4C)


// SMSC9118 Registers
#define SMSC9118_ID_REV         (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0x50)
#define SMSC9118_IRQ_CFG        (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0x54)
#define SMSC9118_INT_STS        (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0x58)
#define SMSC9118_INT_EN         (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0x5C)
#define SMSC9118_RESERVED1      (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0x60)
#define SMSC9118_BYTE_TEST      (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0x64)
#define SMSC9118_FIFO_INT       (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0x68)
#define SMSC9118_RX_CFG	        (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0x6C)
#define SMSC9118_TX_CFG	        (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0x70)
#define SMSC9118_HW_CFG	        (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0x74)
#define SMSC9118_RX_DP_CTL	(volatile unsigned int *)(CONFIG_SMC9118_BASE + 0x78)
#define SMSC9118_RX_FIFO_INF    (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0x7C)
#define SMSC9118_TX_FIFO_INF    (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0x80)
#define SMSC9118_PMT_CTRL       (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0x84)
#define SMSC9118_GPIO_CFG       (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0x88)
#define SMSC9118_GPT_CFG        (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0x8C)
#define SMSC9118_GPT_CNT        (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0x90)
#define SMSC9118_RESERVED2      (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0x94)
#define SMSC9118_ENDIAN	        (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0x98)
#define SMSC9118_FREE_RUN       (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0x9C)
#define SMSC9118_RX_DROP        (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0xA0)
#define SMSC9118_MAC_CSR_CMD    (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0xA4)
#define SMSC9118_MAC_CSR_DATA   (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0xA8)
#define SMSC9118_AFC_CFG        (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0xAC)
#define SMSC9118_E2P_CMD        (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0xB0)
#define SMSC9118_E2P_DATA       (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0xB4)
#define SMSC9118_RESERVED3      (volatile unsigned int *)(CONFIG_SMC9118_BASE + 0xB8)


// SMSC9118 MAC Registers       Indices
#define SMSC9118_MAC_CR         0x1
#define SMSC9118_MAC_ADDRH      0x2
#define SMSC9118_MAC_ADDRL      0x3
#define SMSC9118_MAC_HASHH      0x4
#define SMSC9118_MAC_HASHL      0x5
#define SMSC9118_MAC_MII_ACC    0x6
#define SMSC9118_MAC_MII_DATA   0x7
#define SMSC9118_MAC_FLOW       0x8
#define SMSC9118_MAC_VLAN1      0x9
#define SMSC9118_MAC_VLAN2      0xA
#define SMSC9118_MAC_WUFF       0xB
#define SMSC9118_MAC_WUCSR      0xC

// SMSC9118 PHY Registers       Indices
#define SMSC9118_PHY_BCONTROL   0x0
#define SMSC9118_PHY_BSTATUS    0x1
#define SMSC9118_PHY_ID1        0x2
#define SMSC9118_PHY_ID2        0x3
#define SMSC9118_PHY_ANEG_ADV   0x4
#define SMSC9118_PHY_ANEG_LPA   0x5
#define SMSC9118_PHY_ANEG_EXP   0x6
#define SMSC9118_PHY_MCONTROL   0x17
#define SMSC9118_PHY_MSTATUS    0x18
#define SMSC9118_PHY_CSINDICATE 0x27
#define SMSC9118_PHY_INTSRC     0x29
#define SMSC9118_PHY_INTMASK    0x30
#define SMSC9118_PHY_CS         0x31



#endif  /* _SMC9118_9118_H_ */
