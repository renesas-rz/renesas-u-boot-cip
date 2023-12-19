// SPDX-License-Identifier: (GPL-2.0-only OR BSD-2-Clause)
/*
 *  * Copyright (C) 2023 Renesas Electronics Corp.
 */

#include <common.h>
#include <cpu_func.h>
#include <hang.h>
#include <image.h>
#include <init.h>
#include <malloc.h>
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
#include <asm/arch/rmobile.h>
#include <asm/arch/rcar-mstp.h>
#include <asm/arch/sh_sdhi.h>
#include <mmc.h>

DECLARE_GLOBAL_DATA_PTR;

#define PFC_BASE			0x11030000

/* Ether ch0 IO Voltage Mode Control Register */
#define ETH0_POC			(PFC_BASE + 0x3010)
/* Ether ch1 IO Voltage Mode Control Register */
#define ETH1_POC			(PFC_BASE + 0x3014)
#define ETH_PVDD_3300			0x00
#define ETH_PVDD_1800			0x01
#define ETH_PVDD_2500			0x02
/* Ether MII/RGMII Mode Control Register */
#define ETH_MODE			(PFC_BASE + 0x3018)

/* CPG */
#define CPG_BASE			0x11010000
#define CPG_CLKON_BASE			(CPG_BASE + 0x500)
#define CPG_CLKMON_BASE			(CPG_BASE + 0x680)
#define CPG_RESET_BASE			(CPG_BASE + 0x800)
#define CPG_CLKON_ETH			(CPG_CLKON_BASE + 0x7C)
#define CPG_CLKMON_ETH			(CPG_CLKMON_BASE + 0x7C)
#define CPG_RESET_ETH			(CPG_RESET_BASE + 0x7C)
#define CPG_RESET_I2C			(CPG_RESET_BASE + 0x80)
#define CPG_SDHI_DDIV			(CPG_BASE + 0x218)
#define CPG_SDHI_DSEL			(CPG_BASE + 0x244)
#define CPG_SPI_DDIV			(CPG_BASE + 0x220)
#define CPG_CLKDIV_STATUS		(CPG_BASE + 0x280)
#define CPG_CLKSEL_STATUS		(CPG_BASE + 0x284)
#define CPG_RST_USB			(CPG_BASE + 0x878)
#define CPG_RSTMON_USB			(CPG_BASE + 0x9F8)
#define CPG_CLKON_USB			(CPG_BASE + 0x578)
#define CPG_CLKMON_USB			(CPG_BASE + 0x6F8)

/* PFC */
#define	PFC_P25				(PFC_BASE + 0x25)
#define	PFC_PM25			(PFC_BASE + 0x014A)
#define	PFC_PMC25			(PFC_BASE + 0x0225)

#define	PFC_P31				(PFC_BASE + 0x31)
#define	PFC_PM31			(PFC_BASE + 0x0162)
#define	PFC_PMC31			(PFC_BASE + 0x0231)

#define	PFC_P33				(PFC_BASE + 0x33)
#define	PFC_PM33			(PFC_BASE + 0x0166)
#define	PFC_PMC33			(PFC_BASE + 0x0233)

#define	PFC_P35				(PFC_BASE + 0x35)
#define	PFC_PM35			(PFC_BASE + 0x016A)
#define	PFC_PMC35			(PFC_BASE + 0x0235)

#define	PFC_IOLH_30_L			(PFC_BASE + 0x1180)
#define	PFC_IOLH_34_L			(PFC_BASE + 0x11A0)
#define	PFC_IEN_30			(PFC_BASE + 0x1980)
#define	PFC_IEN_34			(PFC_BASE + 0x19A0)
#define	PFC_IEN_23			(PFC_BASE + 0x1918)
#define	PFC_IEN_24			(PFC_BASE + 0x1920)

#define PFC_PWPR			(PFC_BASE + 0x3000)
#define PWPR_B0WI			BIT(7)	 /* Bit Write Disable */
#define PWPR_PFCWE			BIT(6)	/* PFC Register Write Enable */

#define USBPHY_BASE			(0x11e00000)
#define USB0_BASE			(0x11e10000)
#define USB1_BASE			(0x11e30000)
#define USBF_BASE			(0x11e20000)
#define USBPHY_RESET			(USBPHY_BASE + 0x000u)
#define COMMCTRL			0x800
#define HcRhDescriptorA			0x048
#define LPSTS				0x102
#define	AHB_BUS_CTR			0x208

void s_init(void)
{
	*(volatile u32 *)(PFC_PWPR) = 0;
	*(volatile u32 *)(PFC_PWPR) = PWPR_PFCWE;

#if CONFIG_TARGET_RZG3S_DEV
	/* SD1 power control : P13_4 = 1 P13_0 = 1 */
	/* Set P13_4(bit4) and P13_0(bit0) to Port Mode */
	*(volatile u8 *)(PFC_PMC25) &= 0xEE;
	/* Set P13_4(bit[9:8]) and P13_0(bit[1:0]) to Output mode */
	*(volatile u16 *)(PFC_PM25) = (*(volatile u16 *)(PFC_PM25) & 0xFCFC) | 0x0202;
	/* Set P13_4(bit4) and P13_0(bit0) to High output */
	*(volatile u8 *)(PFC_P25) = (*(volatile u8 *)(PFC_P25) & 0xEE) | 0x11;

	/* SD2 power control : P13_2 = 1 */
	*(volatile u8 *)(PFC_PMC25) &= 0xFB;	/* Set P13_2(bit2) to Port Mode */
	/* Set P13_2(bit[5:4]) to Output mode */
	*(volatile u16 *)(PFC_PM25) = (*(volatile u16 *)(PFC_PM25) & 0xFFCF) | 0x20;
	/* Set P13_2(bit2) to High output */
	*(volatile u8 *)(PFC_P25) = (*(volatile u8 *)(PFC_P25) & 0xFB) | 0x4;

#elif CONFIG_TARGET_SMARC_RZG3S
	/* SD power enable: SD0_PWR_EN - P2_1 = 1, SDIO_PWR_EN - P2_3 = 1, SDIO_PWR_SEL - P4_2 = 1, SD2_PWR_EN - P8_1 = 1 */
	/* Set P2_1(bit1), P2_2(bit2) and P2_3(bit3) to GPIO Mode */
	*(volatile u8 *)(PFC_PMC31) &= 0xF1;
	*(volatile u8 *)(PFC_PMC33) &= 0xFB; /* Set P4_2(bit2) to GPIO Mode */
	*(volatile u8 *)(PFC_PMC35) &= 0xFD; /* Set P8_1(bit1) to GPIO Mode */
	/* Set P2_1(bit[3:2]), P2_2(bit[5:4]) and P2_3(bit[7:6]) to Output mode */
	*(volatile u16 *)(PFC_PM31) = (*(volatile u16 *)(PFC_PM31) & 0xFF03) | 0x00A8;
	/* Set P4_2(bit[5:4]) to Output mode */
	*(volatile u16 *)(PFC_PM33) = (*(volatile u16 *)(PFC_PM33) & 0xFFCF) | 0x0020;
	/* Set P8_1(bit[3:2]) to Output mode */
	*(volatile u16 *)(PFC_PM35) = (*(volatile u16 *)(PFC_PM35) & 0xFFF3) | 0x0008;
	/* Set P2_1(bit1), P2_2(bit2) and P2_3(bit3) to High output */
	*(volatile u8 *)(PFC_P31) |= 0x0E;
	*(volatile u8 *)(PFC_P33) |= 0x04; /* Set P4_2(bit2) to High output */
	*(volatile u8 *)(PFC_P35) |= 0x02; /* Set P8_1(bit1) to High output */
#endif

	/* Input Enable Control */
	*(volatile u32 *)(PFC_IEN_23) = 0x01010100;	/* SD2_DATA1, SD2_DATA0, SD2_CMD	*/
	*(volatile u32 *)(PFC_IEN_24) = 0x0101;		/* SD2_DATA3, SD2_DATA2			*/

	/* Can go in board_eth_init() once enabled */
	*(volatile u32 *)(ETH0_POC) = (*(volatile u32 *)(ETH0_POC) & 0xFFFFFFFC) | ETH_PVDD_1800;
	*(volatile u32 *)(ETH1_POC) = (*(volatile u32 *)(ETH1_POC) & 0xFFFFFFFC) | ETH_PVDD_1800;
	/* Enable RGMII for both ETH{0,1} */
	*(volatile u32 *)(ETH_MODE) = (*(volatile u32 *)(ETH_MODE) & 0xFFFFFFF0);
	/* Driving Ability Control */
	*(volatile u32 *)(PFC_IOLH_30_L) &= ~(0x03);	/* P1_0=b'00	*/
	*(volatile u32 *)(PFC_IOLH_34_L) &= ~(0x03);	/* P7_0=b'00	*/
	/* Input Enable Control */
	*(volatile u32 *)(PFC_IEN_30) = 0x01;
	*(volatile u32 *)(PFC_IEN_34) = 0x01;

	*(volatile u32 *)(PFC_PWPR) = 0;
	*(volatile u32 *)(PFC_PWPR) = PWPR_B0WI;
	/* ETH CLK */
	*(volatile u32 *)(CPG_CLKON_ETH) = 0x01010101;
	while (*(volatile u32 *)(CPG_CLKMON_ETH) != 0x00000101)
		;
	*(volatile u32 *)(CPG_RESET_ETH) = 0x00010001;

	/*
	 * Setting SD CLKs.
	 * Currently, we use IMCLKs with output CLK rate 133 MHz
	 * HSCLK will be considered to support later.
	 */
	*(volatile u32 *)(CPG_SDHI_DDIV) = 0x01110000;
	*(volatile u32 *)(CPG_SDHI_DSEL) = 0x01110222;
	while ((*(volatile u32 *)(CPG_CLKDIV_STATUS) != 0) ||
	       (*(volatile u32 *)(CPG_CLKSEL_STATUS) != 0))
		;
	/* I2C CLK */
	*(volatile u32 *)(CPG_RESET_I2C) = 0xF000F;

	/* Setting xSPI CLK 133 MHz */
	*(volatile u32 *)(CPG_SPI_DDIV) = 0x00010001;
	while (*(volatile u32 *)(CPG_CLKDIV_STATUS) != 0)
		;
}

static void board_usb_init(void)
{
	/*Enable USB*/
	(*(volatile u32 *)CPG_RST_USB) = 0x000f000f;
	while ((*(volatile u32 *)CPG_RSTMON_USB & 0x0000000F) != 0x0000000F)
		;
	(*(volatile u32 *)CPG_CLKON_USB) = 0x000f000f;
	while ((*(volatile u32 *)CPG_CLKMON_USB & 0x0000000F) != 0x0000000F)
		;

	(*(volatile u32 *)(USB0_BASE + AHB_BUS_CTR)) = 0x02;
	(*(volatile u32 *)(USB1_BASE + AHB_BUS_CTR)) = 0x02;
	/*Enable 2 USB ports*/
	(*(volatile u32 *)USBPHY_RESET) = 0x00001000u;
	/*USB0 is HOST*/
	(*(volatile u32 *)(USB0_BASE + COMMCTRL)) = 0;
	/*USB1 is HOST*/
	(*(volatile u32 *)(USB1_BASE + COMMCTRL)) = 0;
	/* Set USBPHY normal operation (Function only) */
	(*(volatile u16 *)(USBF_BASE + LPSTS)) |= (0x1u << 14);	/* USBPHY.SUSPM = 1 (func only) */
	/* Overcurrent is not supported */
	(*(volatile u32 *)(USB0_BASE + HcRhDescriptorA)) |= (0x1u << 12); /* NOCP = 1 */
	(*(volatile u32 *)(USB1_BASE + HcRhDescriptorA)) |= (0x1u << 12); /* NOCP = 1 */
}

int board_early_init_f(void)
{
	return 0;
}

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_TEXT_BASE + 0x50000;
	board_usb_init();

	return 0;
}

void reset_cpu(void)
{
}
