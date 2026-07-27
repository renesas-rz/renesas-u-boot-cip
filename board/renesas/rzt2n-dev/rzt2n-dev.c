/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022 Renesas Electronics Corporation
 */


#include <common.h>
#include <cpu_func.h>
#include <image.h>
#include <init.h>
#include <malloc.h>
#include <miiphy.h>
#include <netdev.h>
#include <dm.h>
#include <dm/platform_data/serial_sh.h>
#include <asm/processor.h>
#include <asm/mach-types.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/arch/renesas.h>
#include <asm/arch/rcar-mstp.h>
#include <asm/arch/sh_sdhi.h>
#include <i2c.h>
#include <mmc.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

#define SCKCR2			0x81280004
#define SCKCR2_CA55SCLK		BIT(12)
#define MSTPCRM			0x80280330
#define MSTPCRM_SDHI0		BIT(12)
#define MSTPCRM_SDHI1		BIT(13)
#define PRCRN			0x80294200
#define PRCRN_PRKEY		(0xa5 << 8)
#define PRCRN_WR_EN		0xF
#define PRCRS                   0x81296000
#define PRCRS_PRKEY             (0xa5 << 8)
#define PRCRS_WR_EN             0xF
#define PMC(x)			((x < 7) ? (0x812C0400 + (x)):(0x802C0400 + (x)))
#define PM(x)			((x < 7) ? (0x812C0200 + (x) * 2):(0x802C0200 + (x) * 2))
#define P(x)			((x < 7) ? (0x812C0000 + (x)):(0x802C0000 + (x)))
#define PFC(x)			((x < 7) ? (0x812C0600 + (x) * 8):(0x802C0600 + (x) * 8))

#define SCKCR			0x80280000
#define PHYSEL			BIT(21)

/* ETH Release Module Stop */

#define MSTPCRE 0x80280310

#define MSTPCRE_GMAC0 BIT(0)
#define MSTPCRE_ETHSW BIT(1)

#define MSTPCRE_ESC BIT(2)

#define MSTPCRE_ETHSS BIT(3)

#define MSTPCRE_USB BIT(8)

#define MSTPCRE_GMAC1 BIT(16)

#define MSTPCRE_GMAC2 BIT(17)

#define MSTPCRE_ESWM BIT(20)

#define MSTPCRE_HPSW BIT(21)

/* ETH Release Module reset */
#define MRCTLE 0x80280250

#define MRCTLE_GMAC0_PCLKH BIT(0)
#define MRCTLE_GMAC0_PCLKM BIT(1)

#define MRCTLE_ETHSW BIT(2)
#define MRCTLE_ESC_BUS BIT(3)

#define MRCTLE_ESC_IP BIT(4)
#define MRCTLE_ETHSS BIT(5)

#define MRCTLE_MII_CONVERTER BIT(6)
#define MRCTLE_GMAC1_PCLKH BIT(16)
#define MRCTLE_GMAC1_PCLKM BIT(17)


#define MRCTLE_GMAC2_PCLKH BIT(18)
#define MRCTLE_GMAC2_PCLKM BIT(19)

#define MRCTLE_ESWM BIT(20)
#define MRCTLE_HPSW BIT(21)

#define ADXCTL0_BASE		0x81290100
#define DDRMIR0_MASK		GENMASK(19, 16)
#define DDRMIR0(x)		(x << 16)
#define ADXC0_MASTERS		6

#define ADXCTL1_BASE		0x81291100
#define DDRMIR1_MASK		GENMASK(27, 24)
#define DDRMIR1(x)		(x << 24)
#define ADXC1_MASTERS		6

#define DDRMIR_MASK		GENMASK(27,  0)
#define addr_shift		0x2

/* USB2.0 Host registers */
#define USB2_BASE		(0x92040000)
#define USBF_BASE		(0x92041000)
#define USB2_INT_ENABLE		0x200
#define USB2_COMMCTRL		0x800
#define USB2_VBCTRL		0x80c
#define USB2_LINECTRL1		0x810
#define USB2_PHYCTRL		0x830
#define USB2_USBCTR		0x20c
#define USB2_UTMI_CTRL		0x318
#define USB2_OCSLPTIMSET	0x310
#define USB2_OBINTSTA		0x804
#define USB2_OBINTEN		0x808
#define USB2_HcRhDescriptorA	0x048
#define USBf_LPSTS		0x102
#define USB2_SPD_RSM_TIMSET	0x30c

/* INT_ENABLE */
#define USB2_INT_ENABLE_UCOM_INTEN	BIT(3)
#define USB2_INT_ENABLE_USBH_INTB_EN	BIT(2) /* For EHCI */
#define USB2_INT_ENABLE_USBH_INTA_EN	BIT(1) /* For OHCI */

/* USBCTR */
#define USB2_USBCTR_DIRPD	BIT(2)
#define USB2_USBCTR_PLL_RST	BIT(1)

/* SPD_RSM_TIMSET */
#define USB2_SPD_RSM_TIMSET_INIT	0x014e029b

/* OC_TIMSET */
#define USB2_OCSLPTIMSET_INIT	0x000209ab

/* COMMCTRL */
#define USB2_COMMCTRL_OTG_PERI	BIT(31) /* 1 = Peripheral mode */

/* VBCTRL */
#define USB2_VBCTRL_VBOUT	BIT(0)

/* LINECTRL1 */
#define USB2_LINECTRL1_DPRPD_EN	BIT(19)
#define USB2_LINECTRL1_DP_RPD	BIT(18)
#define USB2_LINECTRL1_DMRPD_EN	BIT(17)
#define USB2_LINECTRL1_DM_RPD	BIT(16)
#define USB2_LINECTRL1_OPMODE_NODRV	BIT(6)

#define CS0ENDAD_xSPI(x)	(0x80293004 + (0x100 * (x)))

#define XSPI1_FLASH_RW_ENABLE               (0)

/* Needed by lowlevel_init.S*/
void s_init(void)
{
	/* Disable Write protect to enable writing */
	*(volatile u32 *)PRCRN = PRCRN_PRKEY | PRCRN_WR_EN;
	*(volatile u32 *)PRCRS = PRCRS_PRKEY | PRCRS_WR_EN;

	/* Setting xSPI0 CS0 End Address 
	Octal Flash MX25LW51245GXDI00-T 64 MiB range */
	*(volatile u32 *)CS0ENDAD_xSPI(0) = 0x43FFFFFF;

	/* Setting xSPI0 CS0 End Address */
	*(volatile u32 *)CS0ENDAD_xSPI(1) = 0x57FFFFFF;

	/* Enable Write protect to disable writing */
	*(volatile u32 *)PRCRN = PRCRN_PRKEY;
	*(volatile u32 *)PRCRS = PRCRS_PRKEY;
}

static void board_usb_init(void)
{
	/* Disable Write protect to enable writing */
	*(volatile u32 *)PRCRN = PRCRN_PRKEY | PRCRN_WR_EN;
	*(volatile u32 *)PRCRS = PRCRS_PRKEY | PRCRS_WR_EN;

	/* set P11_0 operation as USB_VBUSEN*/
	*(volatile u64 *)PFC(11) = (*(volatile u64 *)PFC(11) & 0xFFFFFFFFFFFFFF00) | (0x17<< 0);
	*(volatile u8 *)PMC(11) |= BIT(0);

	/* USB Module Stop Release */
	*(volatile u32 *)MSTPCRE &= ~(MSTPCRE_USB);
	udelay(1);
	for(u32 i=0;i<7;i++)
	{
		(void)(*(volatile u32 *)(USB2_BASE + USB2_PHYCTRL));
	}

	/* Enable Write protect to disable writing*/
	(*(volatile u32 *)PRCRN) = PRCRN_PRKEY;
	(*(volatile u32 *)PRCRS) = PRCRS_PRKEY;
	
	/* Disable interrupt */
	*(volatile u32 *)(USB2_BASE + USB2_INT_ENABLE) = 0;

	/* enable pull down resisor */
	*(volatile u32 *)(USB2_BASE + USB2_LINECTRL1) |= USB2_LINECTRL1_DP_RPD | USB2_LINECTRL1_DPRPD_EN | USB2_LINECTRL1_DMRPD_EN | USB2_LINECTRL1_DM_RPD;

	/* usb is HOST */
	*(volatile u32 *)(USB2_BASE + USB2_COMMCTRL) &= ~USB2_COMMCTRL_OTG_PERI;

	/*  LPSTS.SUSPM =1 UTMI normal mode */
	(*(volatile u16 *)(USBF_BASE + USBf_LPSTS)) |= 0x4000;

	/* Enable VBUS output */
	*(volatile u32 *)(USB2_BASE + USB2_VBCTRL) |= USB2_VBCTRL_VBOUT;

	/* Overcurrent is not supported */
	(*(volatile u32 *)(USB2_BASE + USB2_HcRhDescriptorA)) |= (0x1u << 12);

	/* Release PLL reset */
	(*(volatile u32 *)(USB2_BASE + USB2_USBCTR)) |= USB2_USBCTR_PLL_RST;
	(*(volatile u32 *)(USB2_BASE + USB2_USBCTR)) &= ~USB2_USBCTR_PLL_RST;

	udelay(100);
}

/* Needed by common/board_f */

int board_early_init_f(void)
{

	return 0;
}

/* Setting for ADXCTLn registers */
int adxctl_init(void)
{
	int i;
	/*DDR Mirror0 addr_shift=0 Mirror1 addr_shift=2*/
	for (i = 2; i < ADXC0_MASTERS; i++)
		writel(((readl((uintptr_t)(ADXCTL0_BASE + i * 0x4)) & ~DDRMIR_MASK) |
				DDRMIR1(addr_shift)), (uintptr_t)(ADXCTL0_BASE + i * 0x4));

	for (i = 0; i < ADXC1_MASTERS; i++)
		writel(((readl((uintptr_t)(ADXCTL1_BASE + i * 0x4)) & ~DDRMIR_MASK) |
			DDRMIR1(addr_shift)), (uintptr_t)(ADXCTL1_BASE + i * 0x4));
	return 0;
}

/* Needed by common/board_f */
int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_TEXT_BASE + 0x50000;

	adxctl_init();

	board_usb_init();

	return 0;
}

void reset_cpu(void)
{

}

/*
 * Reallocate gd to inside DDR area to avoid using it from
 * inaccessible outside DDR mem.
 */
int mach_cpu_init(void)
{
	volatile gd_t *pgd;

	pgd = malloc(sizeof(*pgd));
	if (!pgd) {
		printf("Out of memory for pgd size 0x%lx\n", sizeof(*pgd));
		return -ENOMEM;
	}

	memset((void *)pgd, 0, sizeof(*pgd));
	pgd = gd;
	set_gd(pgd);

	return 0;
}
