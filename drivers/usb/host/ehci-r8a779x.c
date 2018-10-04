/*
 *  EHCI HCD (Host Controller Driver) for USB.
 *
 *  Copyright (C) 2013-2015 Renesas Electronics Corporation
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
#elif defined(GOSE_SDRAM_BASE)
#define PHYS_OFFSET	GOSE_SDRAM_BASE
#elif defined(ALT_SDRAM_BASE)
#define PHYS_OFFSET	ALT_SDRAM_BASE
#elif defined(ALEX_SDRAM_BASE)
#define PHYS_OFFSET	ALEX_SDRAM_BASE
#elif defined(SKRZG1C_SDRAM_BASE)
#define PHYS_OFFSET	SKRZG1C_SDRAM_BASE
#else
#error
#endif

#if defined(CONFIG_R8A7794X) || defined(CONFIG_R8A7747X)
/* USB High-Speed Module (HS-USB) */
#define REG_LPSTS       0x0102		/* 16bit */
#define SUSPM           0x4000
#define SUSPM_SUSPEND   0x0000
#define SUSPM_NORMAL    0x4000
#define REG_UGCTRL      0x0180		/* 32bit */
#define PLLRESET        0x00000001
#define REG_UGCTRL2     0x0184		/* 32bit */
#define USB0SEL         0x00000030
#define USB0SEL_EHCI    0x00000010
#define USB0SEL_HSUSB   0x00000020
#define USB0SEL_OTG     0x00000030
#endif

#define SMSTPCR703      (0x1 << 3)
#define SMSTPCR704      (0x1 << 4)
#define SMSTPCR705      (0x1 << 5)
#define SMSTPCR706      (0x1 << 6)

static u32 usb_base_address[CONFIG_USB_MAX_CONTROLLER_COUNT] = {
	0xee080000,	/* USB0 (EHCI) */
#ifdef CONFIG_MACH_LAGER
	0xee0a0000,	/* USB1 */
#endif
	0xee0c0000,	/* USB2 */
};

#if defined(CONFIG_R8A7794X) || defined(CONFIG_R8A7747X)
static u32 hsusb_base_address[CONFIG_USB_MAX_CONTROLLER_COUNT] = {
	0xE6590000,	/* HS-USB (Channel 0) */
	0xE6598000,	/* HS-USB (Channel 1) */
};
#endif

int ehci_hcd_stop(int index)
{
	u32 data;
	u32 base;
	int i;

	base = usb_base_address[index];

#if !(defined(CONFIG_R8A7794X) || defined(CONFIG_R8A7747X))
	writel(0, base + AHB_BUS_CTR);
#endif

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

#if defined(CONFIG_R8A7794X) || defined(CONFIG_R8A7747X)
	switch (index) {
	case 0:
		/* Stops supply of EHCI0 clock signal */
		setbits_le32(SMSTPCR7, SMSTPCR703);
		/* Stops supply of USBHS0 clock signal */
		setbits_le32(SMSTPCR7, SMSTPCR704);
		break;
	case 1:
		/* Stops supply of EHCI1 clock signal */
		setbits_le32(SMSTPCR7, SMSTPCR705);
		/* Stops supply of USBHS1 clock signal */
		setbits_le32(SMSTPCR7, SMSTPCR706);
		break;
	default:
		return -1;
	}
#else
	if (index == (CONFIG_USB_MAX_CONTROLLER_COUNT-1)) {
		data = readl(SMSTPCR7);
		data |= SMSTPCR703;
		writel(data, SMSTPCR7);
	}
#endif

	return 0;
}

int ehci_hcd_init(int index, struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	u32 base;
	struct rmobile_ehci_reg *rehci;
#if defined(CONFIG_R8A7794X) || defined(CONFIG_R8A7747X)
	struct ahb_bridge *ahb;
	struct usb_core_reg *ucore;
#else
	u32 data;
	u32 phys_base;
#endif
	uint32_t cap_base;

	base = usb_base_address[index];
#if defined(CONFIG_R8A7794X) || defined(CONFIG_R8A7747X)
	switch (index) {
	case 0:
		/* Enables supply of EHCI0 clock signal */
		clrbits_le32(SMSTPCR7, SMSTPCR703);
		/* Enables supply of USBHS0 clock signal */
		clrbits_le32(SMSTPCR7, SMSTPCR704);
		break;
	case 1:
		/* Enables supply of EHCI1 clock signal */
		clrbits_le32(SMSTPCR7, SMSTPCR705);
		/* Enables supply of USBHS1 clock signal */
		clrbits_le32(SMSTPCR7, SMSTPCR706);
		break;
	default:
		return -1;
	}
#else
	phys_base = base;

	if (index == 0) {
		data = readl(SMSTPCR7);
		data &= ~(SMSTPCR703);
		writel(data, SMSTPCR7);
	}
#endif

	/* init */
	rehci = (struct rmobile_ehci_reg *)(base + EHCI_OFFSET);
#if defined(CONFIG_R8A7794X) || defined(CONFIG_R8A7747X)
	ahb = (struct ahb_bridge *)(uintptr_t)(base + AHB_OFFSET);
	ucore = (struct usb_core_reg *)(uintptr_t)(base + USB_CORE_OFFSET);
#endif

#if !(defined(CONFIG_R8A7794X) || defined(CONFIG_R8A7747X))
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
#endif

#if defined(CONFIG_R8A7794X) || defined(CONFIG_R8A7747X)
	switch (index) {
	case 0:		/* HS-USB Channel 0(CN6) */
		/* USBHS PHY PLL reset release */
		clrbits_le32(hsusb_base_address[0]+REG_UGCTRL, PLLRESET);
		mdelay(100);  /* 100ms wait */

		/* HS-USB Channel 0(CN6)                    */
		/*    -> 10:Select HS USB module for USB2.0 */
		clrsetbits_le32(hsusb_base_address[index]+REG_UGCTRL2
				, USB0SEL, USB0SEL_HSUSB);

		/* low power status */
		clrsetbits_le16(hsusb_base_address[index]+REG_LPSTS
				, SUSPM, SUSPM_NORMAL);

		mdelay(100);  /* 100ms wait */

		/* Clock & Reset */
		clrbits_le32(&ahb->usbctr, PLL_RST);

		/* INT_ENALBLE ALL DISABLE */
		clrbits_le32(&ahb->int_enable, (WAKEON_INTEN | UCOM_INTEN
				| USBH_INTBEN | USBH_INTAEN | USBH_INTEN));

		/* Suspend/Resume Timer Setting */
		/* (TIMER_CONNECT:334cycle(us) / TIMER_RESUME:667cycle(us)) */
		writel(0x014e029b, &ucore->spd_rsm_timset);

		/* Overcurrent Detection Timer Setting */
		/* 133547cycle(us) */
		writel(0x000209ab, &ucore->oc_timset);

		break;
	case 1:		/* HS-USB Channel 1(CN5) */
		/* HS-USB Channel 1(CN5)                            */
		/*    -> 01:Select EHCI/OHCI host module for USB2.0 */
		clrsetbits_le32(hsusb_base_address[index]+REG_UGCTRL2
				, USB0SEL, USB0SEL_EHCI);

		/* low power status */
		clrsetbits_le16(hsusb_base_address[index]+REG_LPSTS
				, SUSPM, SUSPM_NORMAL);

		mdelay(100);  /* 100ms wait */

		/* Clock & Reset */
		clrbits_le32(&ahb->usbctr, PLL_RST);

		/* INT_ENALBLE */
		setbits_le32(&ahb->int_enable, USBH_INTBEN | USBH_INTAEN);

		/* Suspend/Resume Timer Setting */
		/* (TIMER_CONNECT:334cycle(us) / TIMER_RESUME:667cycle(us)) */
		writel(0x014e029b, &ucore->spd_rsm_timset);

		/* Overcurrent Detection Timer Setting 133547cycle(us) */
		writel(0x000209ab, &ucore->oc_timset);

		break;
	default:
		return -1;
	}
#else
	/* PCI_INT_ENABLE */
	data = __raw_readl(base + PCI_INT_ENABLE);
	data |= USBH_PMEEN | USBH_INTBEN | USBH_INTAEN;
	writel(data | USBH_PMEEN | USBH_INTBEN | USBH_INTAEN,
			base + PCI_INT_ENABLE);
#endif

	*hccr = (struct ehci_hccr *)((uint32_t)&rehci->HCIVERSION);
	cap_base = ehci_readl(&(*hccr)->cr_capbase);
	*hcor = (struct ehci_hcor *)((uint32_t)*hccr + HC_LENGTH(cap_base));

	return 0;
}

