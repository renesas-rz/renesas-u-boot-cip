/*
 *  EHCI HCD (Host Controller Driver) for USB.
 *
 *  Copyright (C) 2013-2014 Renesas Electronics Corporation
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 */

#include <common.h>
#include <asm/io.h>
#include "ehci.h"
#include "r8a779x.h"

#ifdef LAGER_SDRAM_BASE
#define PHYS_OFFSET     LAGER_SDRAM_BASE
#elif defined(KOELSCH_SDRAM_BASE)
#define PHYS_OFFSET	KOELSCH_SDRAM_BASE
#elif defined(ALT_SDRAM_BASE)
#define PHYS_OFFSET	ALT_SDRAM_BASE
#else
#error
#endif

#define SMSTPCR7        0xe615014c
#define SMSTPCR703      0x08

static u32 usb_base_address[CONFIG_USB_MAX_CONTROLLER_COUNT] = {
	0xee080000,	/* USB0 (EHCI) */
#ifdef CONFIG_MACH_LAGER
	0xee0a0000,	/* USB1 */
#endif
	0xee0c0000,	/* USB2 */
};

int ehci_hcd_stop(int index)
{
	u32 data;
	u32 base;
	int i;

	base = usb_base_address[index];

	writel(0, base + AHB_BUS_CTR);

	/* reset ehci */
	data = readl(base + EHCI_USBCMD);
	data |= CMD_RESET;
	writel(data, base + EHCI_USBCMD);

	for (i = 0; i < 100; i++) {
		data = readl(base + EHCI_USBCMD);
		if (!(data & CMD_RESET))
			break;
		udelay(100);
	}
	if (i == 100)
		printf("error : ehci(%d) reset failed.\n", index);

	if (index == (CONFIG_USB_MAX_CONTROLLER_COUNT-1)) {
		data = readl(SMSTPCR7);
		data |= SMSTPCR703;
		writel(data, SMSTPCR7);
	}

	return 0;
}

int ehci_hcd_init(int index, struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	u32 data;
	u32 base;
	u32 phys_base;
	struct rmobile_ehci_reg *rehci;
	uint32_t cap_base;

	base = usb_base_address[index];
	phys_base = base;

	if (index == 0) {
		data = readl(SMSTPCR7);
		data &= ~(SMSTPCR703);
		writel(data, SMSTPCR7);
	}

	/* init */
	rehci = (struct rmobile_ehci_reg *)(base + EHCI_OFFSET);

	/* Clock & Reset & Direct Power Down */
	data = readl(base + USBCTR);
	data &= ~(DIRPD | PCICLK_MASK | USBH_RST);
	data |= 0x800;	/* PCI-AHB Window 1GB */
	writel(data, base + USBCTR);
	writel(data & ~PLL_RST, base + USBCTR);

	data = SMODE_READY_CTR | MMODE_HBUSREQ |
			MMODE_WR_INCR | MMODE_BYTE_BURST | MMODE_HTRANS;

	/****** AHB-PCI Bridge Communication Registers ******/
	writel(data, base + AHB_BUS_CTR);
	writel((PHYS_OFFSET & 0xf0000000) | PREFETCH,
			base + PCIAHB_WIN1_CTR);
	writel(0xf0000000 | PREFETCH, base + PCIAHB_WIN2_CTR);
	writel(phys_base | PCIWIN2_PCICMD,
			base + AHBPCI_WIN2_CTR);
	data = __raw_readl(base + PCI_ARBITER_CTR);
	writel(data | PCIBP_MODE | PCIREQ1 | PCIREQ0,
			base + PCI_ARBITER_CTR);

	/****** PCI Configuration Registers for AHBPCI ******/
	writel(PCIWIN1_PCICMD | AHB_CFG_AHBPCI,
			base + AHBPCI_WIN1_CTR);
	writel(phys_base + AHBPCI_OFFSET, base + BASEAD);
	writel(PHYS_OFFSET & 0xf0000000, base + WIN1_BASEAD);
	writel(0xf0000000, base + WIN2_BASEAD);
	writel(SERREN | PERREN | MASTEREN | MEMEN,
			base + CMND_STS);

	/****** PCI Configuration Registers for ehci/EHCI ******/
	data = PCIWIN1_PCICMD | AHB_CFG_HOST;
	writel(data, base + AHBPCI_WIN1_CTR);
	writel(phys_base + OHCI_OFFSET, base + OHCI_BASEAD);
	writel(phys_base + EHCI_OFFSET, base + EHCI_BASEAD);
	data = SERREN | PERREN | MASTEREN | MEMEN;
	writel(data, base + OHCI_CMND_STS);
	writel(data, base + EHCI_CMND_STS);

	/* PCI_INT_ENABLE */
	data = __raw_readl(base + PCI_INT_ENABLE);
	data |= USBH_PMEEN | USBH_INTBEN | USBH_INTAEN;
	writel(data | USBH_PMEEN | USBH_INTBEN | USBH_INTAEN,
			base + PCI_INT_ENABLE);

	*hccr = (struct ehci_hccr *)((uint32_t)&rehci->HCIVERSION);
	cap_base = ehci_readl(&(*hccr)->cr_capbase);
	*hcor = (struct ehci_hcor *)((uint32_t)*hccr + HC_LENGTH(cap_base));

	return 0;
}

