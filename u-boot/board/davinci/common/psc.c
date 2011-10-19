/*
 * Power and Sleep Controller (PSC) functions.
 *
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 * Copyright (C) 2008 Lyrtech <www.lyrtech.com>
 * Copyright (C) 2004 Texas Instruments.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <common.h>
#include <asm/arch/hardware.h>

#define PINMUX0_EMACEN (1 << 31)
#define PINMUX0_AECS5  (1 << 11)
#define PINMUX0_AECS4  (1 << 10)

#define PINMUX1_I2C    (1 <<  7)
#define PINMUX1_UART1  (1 <<  1)
#define PINMUX1_UART0  (1 <<  0)

/*
 * The DM6446 includes two separate power domains: "Always On" and "DSP". The
 * "Always On" power domain is always on when the chip is on. The "Always On"
 * domain is powered by the VDD pins of the DM6446. The majority of the
 * DM6446's modules lie within the "Always On" power domain. A separate
 * domain called the "DSP" domain houses the C64x+ and VICP. The "DSP" domain
 * is not always on. The "DSP" power domain is powered by the CVDDDSP pins of
 * the DM6446.
 */

/* Works on Always On power domain only (no PD argument) */
void lpsc_on(unsigned int id)
{
	dv_reg_p mdstat, mdctl;

	if (id >= DAVINCI_LPSC_GEM)
		return;			/* Don't work on DSP Power Domain */

	mdstat = REG_P(PSC_MDSTAT_BASE + (id * 4));
	mdctl = REG_P(PSC_MDCTL_BASE + (id * 4));

	while (REG(PSC_PTSTAT) & 0x01);

	if ((*mdstat & 0x1f) == 0x03)
		return;			/* Already on and enabled */

	*mdctl |= 0x03;

	/* Special treatment for some modules as for sprue14 p.7.4.2 */
	switch (id) {
	case DAVINCI_LPSC_VPSSSLV:
	case DAVINCI_LPSC_EMAC:
	case DAVINCI_LPSC_EMAC_WRAPPER:
	case DAVINCI_LPSC_MDIO:
	case DAVINCI_LPSC_USB:
	case DAVINCI_LPSC_ATA:
	case DAVINCI_LPSC_VLYNQ:
	case DAVINCI_LPSC_UHPI:
	case DAVINCI_LPSC_DDR_EMIF:
	case DAVINCI_LPSC_AEMIF:
	case DAVINCI_LPSC_MMC_SD:
	case DAVINCI_LPSC_MEMSTICK:
	case DAVINCI_LPSC_McBSP:
	case DAVINCI_LPSC_GPIO:
		*mdctl |= 0x200;
		break;
	}

	REG(PSC_PTCMD) = 0x01;

	while (REG(PSC_PTSTAT) & 0x03);
	while ((*mdstat & 0x1f) != 0x03);	/* Probably an overkill... */
}

/* If DSPLINK is used, we don't want U-Boot to power on the DSP. */
#if !defined(CONFIG_SYS_USE_DSPLINK)
void dsp_on(void)
{
	int i;

	if (REG(PSC_PDSTAT1) & 0x1f)
		return;			/* Already on */

	REG(PSC_GBLCTL) |= 0x01;
	REG(PSC_PDCTL1) |= 0x01;
	REG(PSC_PDCTL1) &= ~0x100;
	REG(PSC_MDCTL_BASE + (DAVINCI_LPSC_GEM * 4)) |= 0x03;
	REG(PSC_MDCTL_BASE + (DAVINCI_LPSC_GEM * 4)) &= 0xfffffeff;
	REG(PSC_MDCTL_BASE + (DAVINCI_LPSC_IMCOP * 4)) |= 0x03;
	REG(PSC_MDCTL_BASE + (DAVINCI_LPSC_IMCOP * 4)) &= 0xfffffeff;
	REG(PSC_PTCMD) = 0x02;

	for (i = 0; i < 100; i++) {
		if (REG(PSC_EPCPR) & 0x02)
			break;
	}

	REG(PSC_CHP_SHRTSW) = 0x01;
	REG(PSC_PDCTL1) |= 0x100;
	REG(PSC_EPCCR) = 0x02;

	for (i = 0; i < 100; i++) {
		if (!(REG(PSC_PTSTAT) & 0x02))
			break;
	}

	REG(PSC_GBLCTL) &= ~0x1f;
}
#endif /* CONFIG_SYS_USE_DSPLINK */

void davinci_enable_uart0(void)
{
	lpsc_on(DAVINCI_LPSC_UART0);

	/* Bringup UART0 out of reset */
	REG(UART0_PWREMU_MGMT) = 0x0000e003;

	/* Enable UART0 MUX lines */
	REG(PINMUX1) |= PINMUX1_UART0;
}

#ifdef CONFIG_DRIVER_TI_EMAC
void davinci_enable_emac(void)
{
	lpsc_on(DAVINCI_LPSC_EMAC);
	lpsc_on(DAVINCI_LPSC_EMAC_WRAPPER);
	lpsc_on(DAVINCI_LPSC_MDIO);

	/* Enable GIO3.3V cells used for EMAC */
	REG(VDD3P3V_PWDN) = 0;

	/* Enable EMAC. */
	REG(PINMUX0) |= PINMUX0_EMACEN;
}
#endif

void davinci_enable_i2c(void)
{
	lpsc_on(DAVINCI_LPSC_I2C);

	/* Enable I2C pin Mux */
	REG(PINMUX1) |= PINMUX1_I2C;
}

void davinci_errata_workarounds(void)
{
	/*
	 * Workaround for TMS320DM6446 errata 1.3.22:
	 *   PSC: PTSTAT Register Does Not Clear After Warm/Maximum Reset
	 *   Revision(s) Affected: 1.3 and earlier
	 */
	REG(PSC_SILVER_BULLET) = 0;

	/*
	 * Set the PR_OLD_COUNT bits in the Bus Burst Priority Register (PBBPR)
	 * as suggested in TMS320DM6446 errata 2.1.2:
	 *
	 * On DM6446 Silicon Revision 2.1 and earlier, under certain conditions
	 * low priority modules can occupy the bus and prevent high priority
	 * modules like the VPSS from getting the required DDR2 throughput.
	 * A hex value of 0x20 should provide a good ARM (cache enabled)
	 * performance and still allow good utilization by the VPSS or other
	 * modules.
	 */
	REG(VBPR) = 0x20;
}
