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

/* xSPI clock enable*/
#define MRCTLA 0x80280240
#define MRCTLA_XSPI1 (5)

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

#define DEBUG_PRINT_ETHERNET_SETTINGS       (0)
#if DEBUG_PRINT_ETHERNET_SETTINGS == 1
void debug_ethernet_settings(void);
#endif

#define XSPI1_FLASH_RW_ENABLE               (0)

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

	/* ==========================================xSPI1 Flash =======================================*/

	/* xSPI1 : XSPI1_CKP/P15_6  & xSPI : XSPI1_CS0#/P15_7 */
	*(volatile u8 *)PMC(15) |= BIT(7) | BIT(6);
	*(volatile u64 *)PFC(15) = (*(volatile u64 *)PFC(15) & 0x0000FFFFFFFFFFFF) \
	| ((u64)0x20 << 56) | ((u64)0x20 << 48);

	/* xSPI1 : XSPI1_IO0/P16_2 xSPI : XSPI1_IO1/P16_3 xSPI : XSPI1_IO2/P16_4 xSPI : XSPI1_IO3/P16_5*/
	*(volatile u8 *)PMC(16) |= BIT(5)|BIT(4)|BIT(3)|BIT(2);
    *(volatile u64 *)PFC(16) = (*(volatile u64 *)PFC(16) & 0xFFFF00000000FFFF)
     | ((u64)0x20 << 40) | ((u64)0x20 << 32) | (0x20 << 24) | (0x20 << 16);

	/* Release module reset for GMAC1 */
	*(volatile u32 *)MRCTLA &= ~(MRCTLA_XSPI1);

	/* ==========================================SDHI0 & SDHI1=======================================*/

	*(volatile u8 *)PMC(4) |= BIT(7)|BIT(6)|BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0);
    *(volatile u64 *)PFC(4) = (*(volatile u64 *)PFC(4) & 0x0000000000000000)
     | ((u64)0x2C << 56) | ((u64)0x2C << 48) | ((u64)0x2C << 40) | ((u64)0x2C << 32) | (0x2C << 24) | (0x2C << 16) | (0x2C << 8) | (0x2C << 0);

	*(volatile u8 *)PMC(5) |= BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0);
    *(volatile u64 *)PFC(5) = (*(volatile u64 *)PFC(5) & 0xFFFF000000000000)
     | ((u64)0x2C << 40) | ((u64)0x2C << 32) | (0x2C << 24) | (0x2C << 16) | (0x2C << 8) | (0x2C << 0);

	*(volatile u8 *)PMC(3) |= BIT(7) | BIT(6);
	*(volatile u64 *)PFC(3) = (*(volatile u64 *)PFC(3) & 0x0000FFFFFFFFFFFF) \
	| ((u64)0x2C << 56) | ((u64)0x2C << 48);

	*(volatile u8 *)PMC(15) |= BIT(0);
	*(volatile u64 *)PFC(15) = (*(volatile u64 *)PFC(15) & 0xFFFFFFFFFFFFFF00) \
	|  (0x2C<< 0);

	/* ==================================================ETH0 =======================================*/
    /* PMC and PFC for ETH0_TXCLK,ETH0_TXD0,ETH0_TXD1,ETH0_TXD2,ETH0_TXD3,ETH0_TXEN,ETH0_RXCLK,ETH0_RXD0*/
	*(volatile u8 *)PMC(19) |= BIT(7)|BIT(6)|BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0);
    *(volatile u64 *)PFC(19) = (*(volatile u64 *)PFC(19) & 0x0000000000000000)
     | ((u64)0xf << 56) | ((u64)0xf << 48) | ((u64)0xf << 40) | ((u64)0xf << 32) | (0xf << 24) | (0xf << 16) | (0xf << 8) | (0xf << 0);

	/* PMC and PFC for ETH0_RXD1,ETH0_RXD2,ETH0_RXD3,ETH0_RXDV,ETH012_GMAC0_MDC,ETH012_GMAC0_MDIO,ETHSW_PHYLINK0,ETH0_REFCLK*/
	*(volatile u8 *)PMC(20) |= BIT(7)|BIT(6)|BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0);
    *(volatile u64 *)PFC(20) = (*(volatile u64 *)PFC(20) & 0x0000000000000000) \
     | ((u64)0x02 << 56) | ((u64)0x14<< 48) | ((u64)0x11<< 40) | ((u64)0x11 << 32) | (0xf << 24) | (0xf << 16) | (0xf << 8) | (0xf << 0);

	/* ==================================================ETH1 =======================================*/

    /* PMC and PFC for ETH1_TXCLK,ETH1_TXD0,ETH1_TXD1,ETH1_TXD2,ETH1_TXD3,ETH1_TXEN,ETH1_RXCLK*/
	*(volatile u8 *)PMC(21) |= BIT(7)|BIT(6)|BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1);
	*(volatile u64 *)PFC(21) = (*(volatile u64 *)PFC(21) & 0x00000000000000FF) \
	| ((u64)0xf << 56) | ((u64)0xf << 48) | ((u64)0xf << 40) | ((u64)0xf << 32) | (0xf << 24) | (0xf << 16) | (0xf << 8);

	/* PMC and PFC for ETH1_RXD0,ETH1_RXD1,ETH1_RXD2,ETH1_RXD3,ETH1_RXDV,ETHSW_PHYLINK1,ETH1_REFCLK*/
	*(volatile u8 *)PMC(22) |= BIT(7)| BIT(6)|BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0);
	*(volatile u64 *)PFC(22) = (*(volatile u64 *)PFC(22) & 0xFF00000000000000) \
	| ( ((u64)0x02<< 48) | ((u64)0x14<< 40) | ((u64)0xf << 32) | (0xf << 24) | (0xf << 16) | (0xf << 8) | (0xf << 0));

	/* ==================================================ETH2 =======================================*/

    /* PMC and PFC for ETH2_TXCLK,ETH2_TXD0,ETH2_TXD1,ETH2_TXD2,ETH2_TXD3,ETH2_TXEN,ETH2_RXCLK,ETH2_RXD0*/
	*(volatile u8 *)PMC(23) |= BIT(7)|BIT(6)|BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0);
	*(volatile u64 *)PFC(23) = (*(volatile u64 *)PFC(23) & 0x0000000000000000) \
	| ((u64)0xf << 56) | ((u64)0xf << 48) | ((u64)0xf << 40) | ((u64)0xf << 32) | (0xf << 24) | (0xf << 16) | (0xf << 8) | (0xf << 0);

	/* PMC and PFC for ETH2_RXD1,ETH2_RXD2,ETH2_RXD3,ETH2_RXDV,ETH2_REFCLK*/
	*(volatile u8 *)PMC(24) |= BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0);
	*(volatile u64 *)PFC(24) = (*(volatile u64 *)PFC(24) & 0xFFFFFF0000000000) \
	| ((u64)0x2<< 32) | (0xf << 24) | (0xf << 16) | (0xf << 8) | (0xf << 0);

	/* P25_0_ETHSW_PHYLINK2*/
	*(volatile u8 *)PMC(25) |= BIT(0);
	*(volatile u64 *)PFC(25) = (*(volatile u64 *)PFC(25) & 0xFFFFFFFFFFFFFF00) \
	|  (0x14<< 0);

	/* ==================================================ETH3 =======================================*/

    /* PMC and PFC for ETH3_TXCLK,ETH3_TXD0,ETH3_TXD1,ETH3_TXD2,ETH3_TXD3,ETH3_TXEN,ETH3_RXCLK,ETH3_RXD0*/
	*(volatile u8 *)PMC(0) |= BIT(7)|BIT(6)|BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0);

	*(volatile u64 *)PFC(0) = (*(volatile u64 *)PFC(0) & 0x0000000000000000) \
	| ((u64)0xf << 56) | ((u64)0xf << 48) | ((u64)0xf << 40) | ((u64)0xf << 32) | (0xf << 24) | (0xf << 16) | (0xf << 8) | (0xf << 0);

	/* PMC and PFC for ETH3_RXD1,ETH3_RXD2,ETH3_RXD3,ETH3_RXDV,ETH3_REFCLK ETH3_TXER IRQ*/
	*(volatile u8 *)PMC(1) |= BIT(5)| BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0);
	*(volatile u64 *)PFC(1) = (*(volatile u64 *)PFC(1) & 0xFFFF000000000000) \
	| ((u64)0x0 << 40) |((u64)0x2 << 32) | (0xf << 24) | (0xf << 16) | (0xf << 8) | (0xf << 0);

	/* P03_5_ETH3_RXER */
	*(volatile u8 *)PMC(3) |= BIT(5);
	*(volatile u64 *)PFC(3) = (*(volatile u64 *)PFC(3) & 0xFFFF00FFFFFFFFFF) \
	| ((u64)0xf<< 40);

	/* ==================================================ETH4 =======================================*/

    /* PMC and PFC for P01_6_ETH4_TXCLK_RMII_CLKIN & P01_7_ETH4_TXD0*/
	*(volatile u8 *)PMC(1) |=BIT(7)|BIT(6);
	*(volatile u64 *)PFC(1) = (*(volatile u64 *)PFC(1) & 0x0000FFFFFFFFFFFF) \
	|((u64)0xf << 56) | ((u64)0xf << 48);

	/*P02_0_ETH4_TXD1, P02_1_ETH4_TXD2, P02_2_ETH4_TXD3, P02_3_ETH4_TXEN
	P02_4_ETH4_RXCLK, P02_5_ETH4_RXD0, P02_6_ETH4_RXD1, P02_7_ETH4_RXD2 */
	*(volatile u8 *)PMC(2) |= BIT(7)|BIT(6)|BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0);
	*(volatile u64 *)PFC(2) = (*(volatile u64 *)PFC(2) & 0x0000000000000000) \
	| ((u64)0xf << 56) | ((u64)0xf << 48) | ((u64)0xf << 40) | ((u64)0xf << 32) | (0xf << 24) | (0xf << 16) | (0xf << 8) | (0xf << 0);

	/*P03_0_ETH4_RXD3, P03_1_ETH4_RXDV_CRS_DV */
	*(volatile u8 *)PMC(3) |=BIT(1)|BIT(0);
	*(volatile u64 *)PFC(3) = (*(volatile u64 *)PFC(3) & 0xFFFFFFFFFFFF0000) \
	|(0xf << 8) | (0xf << 0);

#if XSPI1_FLASH_RW_ENABLE == 0
	/*P16_3_ETH4_RXER*/
	*(volatile u8 *)PMC(16) |=BIT(3);
	*(volatile u64 *)PFC(16) = (*(volatile u64 *)PFC(16) & 0xFFFFFFFF00FFFFFF) \
	|(0x10 <<24);
#endif
	/*P03_4_ETH4_REFCLK */
	*(volatile u8 *)PMC(3) |=BIT(4);
	*(volatile u64 *)PFC(3) = (*(volatile u64 *)PFC(3) & 0xFFFFFF00FFFFFFFF) \
	| ((u64)0x2 << 32);

	/* ===================================== ETH1 GMAC1 =======================================*/

    /* Release PHY Reset state P4_7 */
    *(volatile u8 *)PMC(4) &= ~BIT(7);
    *(volatile u8 *)P(4) |= BIT(7);

    /* P4_7 set as output */
    *(volatile u16 *)PM(4) |= (0x3 << 14);


	/* ETH1 connect to GMAC0*/
	*(volatile u8 *)PMC(20) |= (BIT(5)|BIT(4));
	*(volatile u64 *)PFC(20) = (*(volatile u64 *)PFC(20) & 0xFFFF0000FFFFFFFF) \
	|(((u64)0x11<< 40) | ((u64)0x11<< 32));

	/*P24_6_ETH3_GMAC1_MDC & P24_7_ETH3_GMAC1_MDIO*/
	*(volatile u8 *)PMC(24) |= (BIT(7)|BIT(6));
	*(volatile u64 *)PFC(24) = (*(volatile u64 *)PFC(24) & 0x0000FFFFFFFFFFFF) \
	| ( ((u64)0x12<<56) | ((u64)0x12<< 48));


	/* Release module stop for GMAC1 */
	*(volatile u32 *)MSTPCRE &= ~(MSTPCRE_GMAC1);

	/* Release module reset for GMAC1 */
	*(volatile u32 *)MRCTLE &= ~(MRCTLE_GMAC1_PCLKH | MRCTLE_GMAC1_PCLKM);

	/* ==========================================GMAC2 =======================================*/

	/* Release PHY Reset state P16_1 */
	*(volatile u8 *)PMC(16) &= ~BIT(1);
	*(volatile u8 *)P(16) |= BIT(1);

	/* P16_1 set as output */
	*(volatile u16 *)PM(16) |= (0x3 << 2);

	/* Release module stop for GMAC2 */
	*(volatile u32 *)MSTPCRE &= ~(MSTPCRE_GMAC2);

	/* Release module reset for GMAC2 */
	*(volatile u32 *)MRCTLE &= ~(MRCTLE_GMAC2_PCLKH | MRCTLE_GMAC2_PCLKM);

	/* ==========================================END=======================================*/

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

static void board_usb_init(void)
{
	/* Disable Write protect to enable writing */
	*(volatile u32 *)PRCRN = PRCRN_PRKEY | PRCRN_WR_EN;
	*(volatile u32 *)PRCRS = PRCRS_PRKEY | PRCRS_WR_EN;

	/* set P11_0 operation as USB_VBUSEN*/
	*(volatile u64 *)PFC(11) = (*(volatile u64 *)PFC(11) & 0xFFFFFFFFFFFFFF00) | (0x17<< 0);
	*(volatile u8 *)PMC(11) |= BIT(0);

	/* Enable Write protect to disable writing*/
	(*(volatile u32 *)PRCRN) = PRCRN_PRKEY;
	(*(volatile u32 *)PRCRS) = PRCRS_PRKEY;

	/* Release USB module from the stopped state MSTPCRE[8]=0 is performed in TF-A */
	
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
	int ret;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_TEXT_BASE + 0x50000;

	adxctl_init();

	ethss_init_hw(0x0);

	ret = ethss_config(3, PHY_INTERFACE_MODE_RGMII_ID);
	if (ret < 0)
	{
		printf("ETH3 port Init FAILED \n");
		return ret;
	}
	ethss_link_up(3, PHY_INTERFACE_MODE_RGMII_ID, SPEED_1000, DUPLEX_FULL);

	ret = ethss_config(4, PHY_INTERFACE_MODE_RGMII_ID);
	if (ret < 0)
	{
		printf("ETH4 port Init FAILED \n");
		return ret;
	}
	ethss_link_up(4, PHY_INTERFACE_MODE_RGMII_ID, SPEED_1000, DUPLEX_FULL);

	board_usb_init();
#if DEBUG_PRINT_ETHERNET_SETTINGS == 1
	debug_ethernet_settings();
#endif
	return 0;
}
#if DEBUG_PRINT_ETHERNET_SETTINGS == 1
void debug_ethernet_settings(void)
{

	printf("======================================ETH0 ==============================\n");
	printf("ETH0 PMC19 = 0x%02x\n", *(volatile u8 *)PMC(19));
	printf("ETH0 PFC19 = 0x%016llx\n",  *(volatile u64 *)PFC(19));

	printf("ETH0 PMC20 = 0x%02x\n", *(volatile u8 *)PMC(20));
	printf("ETH0 PFC20 = 0x%016llx\n",  *(volatile u64 *)PFC(20));

	printf("======================================ETH1 ==============================\n");
	printf("ETH1 PMC21 = 0x%02x\n", *(volatile u8 *)PMC(21));
	printf("ETH1 PFC21 = 0x%016llx\n",  *(volatile u64 *)PFC(21));

	printf("ETH1 PMC22 = 0x%02x\n", *(volatile u8 *)PMC(22));
	printf("ETH1 PFC22 = 0x%016llx\n",  *(volatile u64 *)PFC(22));

	printf("======================================ETH2 ==============================\n");
	printf("ETH2 PMC23 = 0x%02x\n", *(volatile u8 *)PMC(23));
	printf("ETH2 PFC23 = 0x%016llx\n",  *(volatile u64 *)PFC(23));

	printf("ETH2 PMC24 = 0x%02x\n", *(volatile u8 *)PMC(24));
	printf("ETH2 PFC24 = 0x%016llx\n",  *(volatile u64 *)PFC(24));

	printf("ETH2 PMC25 = 0x%02x\n", *(volatile u8 *)PMC(25));
	printf("ETH2 PFC25 = 0x%016llx\n",  *(volatile u64 *)PFC(25));

	printf("======================================ETH3 ==============================\n");
	printf("ETH3 PMC0 = 0x%02x\n", *(volatile u8 *)PMC(0));
	printf("ETH3 PFC0 = 0x%016llx\n",  *(volatile u64 *)PFC(0));
	printf("ETH3 PMC1 = 0x%02x\n", *(volatile u8 *)PMC(1));
	printf("ETH3 PFC1 = 0x%016llx\n",  *(volatile u64 *)PFC(1));
	printf("ETH3 PMC3 = 0x%02x\n", *(volatile u8 *)PMC(3));
	printf("ETH3 PFC3 = 0x%016llx\n",  *(volatile u64 *)PFC(3));

	printf("======================================ETH4 ==============================\n");
	printf("ETH4 PMC1 = 0x%02x\n", *(volatile u8 *)PMC(1));
	printf("ETH4 PFC1 = 0x%016llx\n",  *(volatile u64 *)PFC(1));
	printf("ETH4 PMC2 = 0x%02x\n", *(volatile u8 *)PMC(2));
	printf("ETH4 PFC3 = 0x%016llx\n",  *(volatile u64 *)PFC(2));
	printf("ETH4 PMC3 = 0x%02x\n", *(volatile u8 *)PMC(3));
	printf("ETH4 PFC3 = 0x%016llx\n",  *(volatile u64 *)PFC(3));


	printf(" ============================== ETH1 GMAC1 =============================\n");
	printf("GMAC1 PMC4 = 0x%02x\n", *(volatile u8 *)PMC(4));
	printf("GMAC1 P4   = 0x%02x\n", *(volatile u8 *)P(4));
	printf("GMAC1 PM4  = 0x%04x\n", *(volatile u16 *)PM(4));

	printf("ETH1 PMC20 = 0x%02x\n", *(volatile u8 *)PMC(20));
	printf("ETH1 PFC20 = 0x%016llx\n",  *(volatile u64 *)PFC(20));

	printf("ETH1 PMC24 = 0x%02x\n", *(volatile u8 *)PMC(24));
	printf("ETH1 PFC24 = 0x%016llx\n",  *(volatile u64 *)PFC(24));

	printf("MSTPCRE = 0x%08x\n", *(volatile u32 *)MSTPCRE);
	printf("MRCTLE  = 0x%08x\n", *(volatile u32 *)MRCTLE);
	printf("SCKCR2  = 0x%08x\n", *(volatile u32 *)SCKCR2);

	printf("MODCTRL  = 0x%08x\n", *(volatile u32 *)0x80110008);
	printf("CONVCTRL0  = 0x%08x\n", *(volatile u32 *)0x80110100);
	printf("CONVCTRL1  = 0x%08x\n", *(volatile u32 *)0x80110104);
	printf("CONVCTRL2  = 0x%08x\n", *(volatile u32 *)0x80110108);
	printf("CONVCTRL3  = 0x%08x\n", *(volatile u32 *)0x8011010C);
	printf("CONVCTRL4  = 0x%08x\n", *(volatile u32 *)0x80110110);
	printf("CONVRST    = 0x%08x\n", *(volatile u32 *)0x80110114);

	printf(" ============================== CLOCK=============================\n");
	printf("SCKCR     		= 0x%08x\n", *(volatile u32 *)0x81280000);
	printf("SCKCR2    		= 0x%08x\n", *(volatile u32 *)0x81280004);
	printf("SCKCR3    		= 0x%08x\n", *(volatile u32 *)0x81280008);
	printf("SCKCR4    		= 0x%08x\n", *(volatile u32 *)0x8128000C);
	printf("PMSEL     		= 0x%08x\n", *(volatile u32 *)0x81280010);
	printf("PLL0MON   		= 0x%08x\n", *(volatile u32 *)0x81280020);
	printf("PLL0EN    		= 0x%08x\n", *(volatile u32 *)0x81280030);
	printf("PLL1MON  		= 0x%08x\n", *(volatile u32 *)0x81280040);
	printf("PLL2MON  		= 0x%08x\n", *(volatile u32 *)0x81280090);
	printf("PLL2EN  		= 0x%08x\n", *(volatile u32 *)0x812800A0);				
}
#endif

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
