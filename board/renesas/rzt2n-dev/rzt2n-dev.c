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
#define PMC(x)			((x < 13) ? (0x812C0400 + (x)):(0x802C0400 + (x)))
#define PM(x)			((x < 13) ? (0x812C0200 + (x) * 2):(0x802C0200 + (x) * 2))
#define P(x)			((x < 13) ? (0x812C0000 + (x)):(0x802C0000 + (x)))
#define PFC(x)			((x < 13) ? (0x812C0600 + (x) * 8):(0x802C0600 + (x) * 8))

#define SCKCR			0x80280000
#define PHYSEL			BIT(21)

/* ETH Release Module Stop */

#define MSTPCRE 0x80280310

#define MSTPCRE_GMAC0 BIT(0)
#define MSTPCRE_ETHSW BIT(1)

#define MSTPCRE_ESC BIT(2)

#define MSTPCRE_ETHSS BIT(3)

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
#define DDRMIR0_MASK		GENMASK(20, 16)
#define DDRMIR0(x)		(x << 16)
#define ADXC0_MASTERS		6

#define ADXCTL1_BASE		0x81291100
#define DDRMIR1_MASK		GENMASK(28, 24)
#define DDRMIR1(x)		(x << 24)
#define ADXC1_MASTERS		7

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

/*SW_MODE=0 ETH0 = ETHSW Port 0 */
#define ETHSW_PORT0_IN_USE   	0
/*SW_MODE=0 ETH1 = ETHSW Port 1 */
#define ETHSW_PORT1_IN_USE   	0
/*SW_MODE=0 ETH2 = ETHSW Port 2 */
#define ETHSW_PORT2_IN_USE   	0

/*SW_MODE=0 ETH3 = GMAC1 */
#define GMAC1_ETH_BLOCK_IN_USE	1
/*SW_MODE=0 ETH4 = GMAC2 */
#define GMAC2_ETH_BLOCK_IN_USE	1


#if ETHSW_PORT1_IN_USE == 1
static void eth1_io_multiplex_config(void);
#endif


/* Needed by lowlevel_init.S*/
void s_init(void)
{
	/* Disable Write protect to enable writing */
	*(volatile u32 *)PRCRN = PRCRN_PRKEY | PRCRN_WR_EN;
	*(volatile u32 *)PRCRS = PRCRS_PRKEY | PRCRS_WR_EN;

	/* Setting xSPI1 CS0 End Address */
	*(volatile u32 *)CS0ENDAD_xSPI(1) = 0x57FFFFFF;

	/* P03_3 enable both SD0_PWEN_A & SD1_PWEN_A Output HIGH */

	*(volatile u8 *)PMC(3) &= ~BIT(3);
	*(volatile u8 *)P(3) |= BIT(3);
	*(volatile u16 *)PM(3) |= (0x3 << 6);

	/* P3_2_SD1_IOVS Output LOW */
	*(volatile u8 *)PMC(3) &= ~BIT(2);
	*(volatile u8 *)P(3) &= ~BIT(2);
	*(volatile u16 *)PM(3) |= (0x3 << 4);

	/* Release module stop for SDHI0/1 */
	*(volatile u32 *)MSTPCRM &= ~(MSTPCRM_SDHI0 | MSTPCRM_SDHI1);

	/*ETH1 settings*/
	/* IO Multiplexing Configuration */
   /* PMC and PFC for ETH1_TXCLK,ETH1_TXD0,ETH1_TXD1,ETH1_TXD2,ETH1_TXD3,ETH1_TXEN,ETH1_RXCLK*/
	*(volatile u8 *)PMC(21) |= BIT(7)|BIT(6)|BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1);
	*(volatile u64 *)PFC(21) = (*(volatile u64 *)PFC(21) & 0x00000000000000FF) \
	| ((u64)0xf << 56) | ((u64)0xf << 48) | ((u64)0xf << 40) | ((u64)0xf << 32) | (0xf << 24) | (0xf << 16) | (0xf << 8);

	/* PMC and PFC for ETH1_RXD0,ETH1_RXD1,ETH1_RXD2,ETH1_RXD3,ETH1_RXDV,ETH012_GMAC0_MDC,ETH012_GMAC0_MDIO,ETH1_REFCLK,ETHSW_PHYLINK1*/
	*(volatile u8 *)PMC(22) |= BIT(6)|BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0);
	*(volatile u64 *)PFC(22) = (*(volatile u64 *)PFC(22) & 0xFF00000000000000) \
	| ( ((u64)0x02<< 48) | ((u64)0xf << 32) | (0xf << 24) | (0xf << 16) | (0xf << 8) | (0xf << 0));

	/* GMAC0 Shared between ETH012*/
	*(volatile u8 *)PMC(20) |= (BIT(5)|BIT(4));
	*(volatile u64 *)PFC(20) = (*(volatile u64 *)PFC(20) & 0xFFFF0000FFFFFFFF) \
	|(((u64)0x11<< 40) | ((u64)0x11<< 32));
#if 0
    /* Release PHY Reset state P4_7 */
    *(volatile u8 *)PMC(4) &= ~BIT(7);
    *(volatile u8 *)P(4) |= BIT(7);

    /* P4_7 set as output */
    *(volatile u16 *)PM(4) |= (0x3 << 14);
#endif
	/* Release PHY Reset state P16_1 */
	*(volatile u8 *)PMC(16) &= ~BIT(1);
	*(volatile u8 *)P(16) |= BIT(1);

	/* P16_1 set as output */
	*(volatile u16 *)PM(16) |= (0x3 << 2);
#if 0
	/* Release module stop for GMAC0 */
	*(volatile u32 *)MSTPCRE &= ~(MSTPCRE_GMAC0);

	/* Release module reset for GMAC0 */
	*(volatile u32 *)MRCTLE &= ~(MRCTLE_GMAC0_PCLKH | MRCTLE_GMAC0_PCLKM);
#endif
	/* Release module stop for ETH_SS */
	*(volatile u32 *)MSTPCRE &= ~(MSTPCRE_ETHSS);

	/* Release module reset for ETH_SS, MII_CONV */
	*(volatile u32 *)MRCTLE &= ~(MRCTLE_ETHSS | MRCTLE_MII_CONVERTER);

    /* Release module stop and reset for GMAC ETHSW, ESC, NSWM, or HPSW */

	/* Set Coretex-A55 DSU clock to 1GHz */
	*(volatile u32 *)SCKCR2 |= SCKCR2_CA55SCLK;

	/* Enable Write protect to disable writing */
	*(volatile u32 *)PRCRN = PRCRN_PRKEY;
	*(volatile u32 *)PRCRS = PRCRS_PRKEY;
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
	int ret;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_TEXT_BASE + 0x50000;

	adxctl_init();
	/* ETHSS: Mode Control 0x0, GMAC1 on port ETH3, GMAC2 on ETH4 */
	ethss_init_hw(0x0);
#if 0
#if GMAC1_ETH_BLOCK_IN_USE == 1
	/* ETHSS: GMAC1 RGMII_ID mode on port ETH3 */
	ret = ethss_config(3, PHY_INTERFACE_MODE_RGMII_ID);
	if (ret < 0)
	{
		printf("ETH3 port Init FAILED \n");
		return ret;
	}

	/* Set up speed for Converters for GMAC1 */
	ethss_link_up(3, PHY_INTERFACE_MODE_RGMII_ID, SPEED_1000, DUPLEX_FULL);

#endif

#if GMAC2_ETH_BLOCK_IN_USE == 1

	/* ETHSS: GMAC2 RGMII_ID mode on port ETH4 */
	ret = ethss_config(4, PHY_INTERFACE_MODE_RGMII_ID);
	if (ret < 0)
	{
		printf("ETH4 port Init FAILED \n");
		return ret;
	}
	/* Set up speed for Converters for GMAC2 */
	ethss_link_up(4, PHY_INTERFACE_MODE_RGMII_ID, SPEED_1000, DUPLEX_FULL);
#endif
	board_usb_init();
#endif
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
