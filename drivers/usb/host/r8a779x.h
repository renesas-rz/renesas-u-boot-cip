
#ifndef __LINUX_USB_R8A779x_H
#define __LINUX_USB_R8A779x_H
#include <linux/types.h>

struct rmobile_usb_platdata {
	/* VBUS power on */
	void (*set_vbus)(int on);

	/* Overcurrent */
	int oci_irq;
	int (*check_oci)(void);

	/* common clock */
	void (*common_clock)(int on);
};

/*-------------------------------------------------------------------------*/
/* OverCurrent parameter*/
#define OVER_CURRENT_TIME	5

/*-------------------------------------------------------------------------*/
/* Register offset */
#define OHCI_OFFSET		0
#define OHCI_SIZE		0x1000

#define EHCI_OFFSET		0x1000
#define EHCI_SIZE		0x1000

#define EHCI_USBCMD	(EHCI_OFFSET + 0x0020)

/* PCI Configuration Registers */
#define PCI_CONF_OHCI_OFFSET	0x10000
#define OHCI_VID_DID		(PCI_CONF_OHCI_OFFSET + 0x0000)
#define OHCI_CMND_STS		(PCI_CONF_OHCI_OFFSET + 0x0004)
#define OHCI_BASEAD		(PCI_CONF_OHCI_OFFSET + 0x0010)
#define OHCI_PM_CONTROL		(PCI_CONF_OHCI_OFFSET + 0x0044)

#define PCI_CONF_EHCI_OFFSET	0x10100
#define EHCI_VID_DID		(PCI_CONF_EHCI_OFFSET + 0x0000)
#define EHCI_CMND_STS		(PCI_CONF_EHCI_OFFSET + 0x0004)
#define EHCI_BASEAD		(PCI_CONF_EHCI_OFFSET + 0x0010)
#define EHCI_PM_CONTROL		(PCI_CONF_EHCI_OFFSET + 0x0044)

/* PCI Configuration Registers for AHB-PCI Bridge Registers */
#define PCI_CONF_AHBPCI_OFFSET	0x10000
#define VID_DID			(PCI_CONF_AHBPCI_OFFSET + 0x0000)
#define CMND_STS		(PCI_CONF_AHBPCI_OFFSET + 0x0004)
#define REVID_CC		(PCI_CONF_AHBPCI_OFFSET + 0x0008)
#define CLS_LT_HT_BIST		(PCI_CONF_AHBPCI_OFFSET + 0x000C)
#define BASEAD			(PCI_CONF_AHBPCI_OFFSET + 0x0010)
#define WIN1_BASEAD		(PCI_CONF_AHBPCI_OFFSET + 0x0014)
#define WIN2_BASEAD		(PCI_CONF_AHBPCI_OFFSET + 0x0018)
#define SSVID_SSID		(PCI_CONF_AHBPCI_OFFSET + 0x002C)
#define INTR_LINE_PIN		(PCI_CONF_AHBPCI_OFFSET + 0x003C)

/* AHB-PCI Bridge PCI Communication Registers */
#define AHBPCI_OFFSET		0x10800
#define PCIAHB_WIN1_CTR		(AHBPCI_OFFSET + 0x0000)
#define PCIAHB_WIN2_CTR		(AHBPCI_OFFSET + 0x0004)
#define PCIAHB_DCT_CTR		(AHBPCI_OFFSET + 0x0008)
#define AHBPCI_WIN1_CTR		(AHBPCI_OFFSET + 0x0010)
#define AHBPCI_WIN2_CTR		(AHBPCI_OFFSET + 0x0014)
#define AHBPCI_DCT_CTR		(AHBPCI_OFFSET + 0x001C)
#define PCI_INT_ENABLE		(AHBPCI_OFFSET + 0x0020)
#define PCI_INT_STATUS		(AHBPCI_OFFSET + 0x0024)
#define AHB_BUS_CTR		(AHBPCI_OFFSET + 0x0030)
#define USBCTR			(AHBPCI_OFFSET + 0x0034)
#define PCI_ARBITER_CTR		(AHBPCI_OFFSET + 0x0040)
#define PCI_UNIT_REV		(AHBPCI_OFFSET + 0x004C)


/* CMND_STS (0x10004) */
#define DETPERR			(1 << 31)		/* RW */
#define SIGSERR			(1 << 30)		/* RW */
#define REMABORT		(1 << 29)		/* RW */
#define RETABORT		(1 << 28)		/* RW */
#define SIGTABORT		(1 << 27)		/* RW */
#define DEVTIM			((1 << 26)|(1 << 25))	/* R */
#define MDPERR			(1 << 24)		/* R */
#define FBTBCAP			(1 << 23)		/* R */
#define _66MCAP			(1 << 21)		/* R */
#define CAPLIST			(1 << 20)		/* R */
#define FBTBEN			(1 << 9)		/* R */
#define SERREN			(1 << 8)		/* RW */
#define STEPCTR			(1 << 7)		/* R */
#define PERREN			(1 << 6)		/* RW */
#define VGAPSNP			(1 << 5)		/* R */
#define MWINVEN			(1 << 4)		/* R */
#define SPECIALC		(1 << 3)		/* R */
#define MASTEREN		(1 << 2)		/* RW */
#define MEMEN			(1 << 1)		/* RW */
#define IOEN			(1 << 0)		/* R */

/* WIN1_BASEAD (0x10014) */
#define PCI_WIN1_BASEADR	0xF0000000		/* RW */
#define WIN1_PREFETCH		(1 << 3)		/* R */
#define WIN1_TYPE		((1 << 2)|(1 << 1))	/* R */
#define WIN1_MEM		(1 << 0)		/* R */

/* WIN2_BASEAD (0x10018) */
#define PCI_WIN2_BASEADR	0xF0000000		/* RW */
#define WIN2_PREFETCH		(1 << 3)		/* R */
#define WIN2_TYPE		((1 << 2)|(1 << 1))	/* R */
#define WIN2_MEM		(1 << 0)		/* R */

/* PCIAHB_WIN1_CTR (0x10800) */
#define AHB_BASEADR		((1 << 31)|(1 << 30))	/* RW */
#define ENDIAN_CTR		0x000001C0		/* RW */
#define PREFETCH		((1 << 1)|(1 << 0))	/* RW */

/* PCIAHB_WIN2_CTR (0x10804) */
#define AHB_BASE_ADR		(1 << 31)_(1 << 28)	/* RW */
#define ENDIAN_CTR		0x000001C0		/* RW */
#define PREFETCH		((1 << 1)|(1 << 0))	/* RW */

/* PCIAHB_DCT_CTR (0x10808) */
#define PCIAHB_DISCARD_TIMER	0x0000FFF0		/* RW */
#define DISCARD_EN		(1 << 0)		/* RW */

/* AHBPCI_WIN1_CTR (0x10810) */
#define PCIWIN1_BASEADR		0xFFFFF800		/* RW */
#define ENDIAN_CTR		0x000001C0		/* RW */
#define CFGTYPE			(1 << 4)		/* RW */
#define PCICMD			0x0000000E		/* RW */
#define PCIWIN1_PCICMD		((1 << 3)|(1 << 1))
#define AHB_CFG_AHBPCI		0x40000000
#define AHB_CFG_HOST		0x80000000

/* AHBPCI_WIN2_CTR (0x10814) */
#define PCIWIN2_BASEADR		0xFFFF0000		/* RW */
#define ENDIAN_CTR		0x000001C0		/* RW */
#define BURST_EN		(1 << 5)		/* RW */
#define PCICMD			0x0000000E		/* RW */
#define PCIWIN2_PCICMD		((1 << 2)|(1 << 1))
#define PCIWIN2_PREFETCH	(1 << 0)		/* RW */

/* AHBPCI_DCT_CTR (0x1081C) */
#define AHBPCI_DISCARD_TIMER	(1 << 15)_(1 << 4)	/* RW */
#define DISCARD_EN		(1 << 0)		/* RW */

/* PCI_INT_ENABLE (0x10820) */
#define USBH_PMEEN		(1 << 19)		/* RW */
#define USBH_INTBEN		(1 << 17)		/* RW */
#define USBH_INTAEN		(1 << 16)		/* RW */
#define AHBPCI_WIN_INTEN	(1 << 14)		/* RW */
#define PCIAHB_WIN2_INTEN	(1 << 13)		/* RW */
#define PCIAHB_WIN1_INTEN	(1 << 12)		/* RW */
#define DMA_AHBPCI_INTEN	(1 << 9)		/* RW */
#define DMA_PCIAHB_INTEN	(1 << 8)		/* RW */
#define RESERR_INTEN		(1 << 5)		/* RW */
#define SIGSERR_INTEN		(1 << 4)		/* RW */
#define PERR_INTEN		(1 << 3)		/* RW */
#define REMABORT_INTEN		(1 << 2)		/* RW */
#define RETABORT_INTEN		(1 << 1)		/* RW */
#define SIGTABORT_INTEN		(1 << 0)		/* RW */

/* PCI_INT_STATUS (0x10824) */
#define USBH_PME		(1 << 19)		/* R */
#define USBH_INTB		(1 << 17)		/* R */
#define USBH_INTA		(1 << 16)		/* R */
#define AHBPCI_WIN_INT		(1 << 14)		/* RW */
#define PCIAHB_WIN2_INT		(1 << 13)		/* RW */
#define PCIAHB_WIN1_INT		(1 << 12)		/* RW */
#define DMA_AHBPCI_INT		(1 << 9)		/* RW */
#define DMA_PCIAHB_INT		(1 << 8)		/* RW */
#define RESERR_INT		(1 << 5)		/* RW */
#define SIGSERR_INT		(1 << 4)		/* RW */
#define PERR_INT		(1 << 3)		/* RW */
#define REMABORT_INT		(1 << 2)		/* RW */
#define RETABORT_INT		(1 << 1)		/* RW */
#define SIGTABORT_INT		(1 << 0)		/* RW */

/* AHB_BUS_CTR (0x10830) */
#define SMODE_READY_CTR		(1 << 17)		/* RW */
#define SMODE_READ_BURST	(1 << 16)		/* RW */
#define MMODE_HBUSREQ		(1 << 7)		/* RW */
#define MMODE_BOUNDARY		((1 << 6)|(1 << 5))	/* RW */
#define MMODE_BURST_WIDTH	((1 << 4)|(1 << 3))	/* RW */
#define MMODE_SINGLE_MODE	((1 << 4)|(1 << 3))	/* RW */
#define MMODE_WR_INCR		(1 << 2)		/* RW */
#define MMODE_BYTE_BURST	(1 << 1)		/* RW */
#define MMODE_HTRANS		(1 << 0)		/* RW */
#define AHB_BUS_CTR_SET	\
	((1 << 17) | (1 << 7) | (1 << 4) | (1 << 3) | (1 << 2) | (1 << 1))

/* USBCTR (0x10834) */
#define DIRPD			(1 << 8)		/* RW */
#define PLL_RST			(1 << 2)		/* RW */
#define PCICLK_MASK		(1 << 1)		/* RW */
#define USBH_RST		(1 << 0)		/* RW */

/* PCI_ARBITER_CTR (0x10840) */
#define PCIBUS_PARK_TIMER	0x00FF0000		/* RW */
#define PCIBUS_PARK_TIMER_SET	0x00070000
#define PCIBP_MODE		(1 << 12)		/* RW */
#define PCIREQ7			(1 << 7)		/* RW */
#define PCIREQ6			(1 << 6)		/* RW */
#define PCIREQ5			(1 << 5)		/* RW */
#define PCIREQ4			(1 << 4)		/* RW */
#define PCIREQ3			(1 << 3)		/* RW */
#define PCIREQ2			(1 << 2)		/* RW */
#define PCIREQ1			(1 << 1)		/* RW */
#define PCIREQ0			(1 << 0)		/* RW */

/*-------------------------------------------------------------------------*/

struct rmobile_ohci_reg {
	u32	HcRevision;		/* HcRevision */
	u32	HcControl;		/* HcControl */
	u32	HcCommandStatus;	/* HcCommandStatus */
	u32	HcInterruptStatus;	/* HcInterruptStatus */
	u32	HcInterruptEnable;	/* HcInterruptEnable */
	u32	HcInterruptDisable;	/* HcInterruptDisable */
	u32	HcHCCA;			/* HcHCCA */
	u32	HcPeriodicCurrentED;	/* HcPeriodicCurrentED */
	u32	HcControlHeadED;	/* HcControlHeadED */
	u32	HcControlCurrentED;	/* HcControlCurrentED */
	u32	HcBulkHeadED;		/* HcBulkHeadED */
	u32	HcBulkCurrentED;	/* HcBulkCurrentED */
	u32	HcDoneHead;		/* HcDoneHead */
	u32	HcFmInterval;		/* HcFmInterval */
	u32	HcFmRemaining;		/* HcFmRemaining */
	u32	HcFmNumber;		/* HcFmNumber */
	u32	HcPeriodicStart;	/* HcPeriodicStart */
	u32	HcLSThreshold;		/* HcLSThreshold */
	u32	HcRhDescrptorA;		/* HcRhDescrptorA */
	u32	HcRhDescrptorB;		/* HcRhDescrptorB */
	u32	HcRhStatus;		/* HcRhStatus */
	u32	HcRhPortStatus;		/* HcRhPortStatus */

	u32	save_flag;		/* Save Flag */
};

struct rmobile_ehci_reg {
	u32	HCIVERSION;		/* HCIVERSION/CAPLENGTH */
	u32	HCSPARAMS;		/* HCSPARAMS */
	u32	HCCPARAMS;		/* HCCPARAMS */
	u32	HCSP_PORTROUTE;		/* HCSP_PORTROUTE */
	u32	USBCMD;			/* USBCMD */
	u32	USBSTS;			/* USBSTS */
	u32	USBINTR;		/* USBINTR */
	u32	FRINDEX;		/* FRINDEX */
	u32	CTRLDSSEGMENT;		/* CTRLDSSEGMENT */
	u32	PERIODICLISTBASE;	/* PERIODICLISTBASE */
	u32	ASYNCLISTADDR;		/* ASYNCLISTADDR */
	u32	Reserved_1[9];
	u32	CONFIGFLAG;		/* CONFIGFLAG */
	u32	PORTSC;			/* PORTSC */

	u32	save_flag;		/* Save Flag */
};

#endif /* __LINUX_USB_R8A779x_H */
