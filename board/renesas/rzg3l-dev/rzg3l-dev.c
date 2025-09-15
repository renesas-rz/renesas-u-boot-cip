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
#include <asm/arch/renesas.h>
#include <asm/arch/rcar-mstp.h>
#include <asm/arch/sh_sdhi.h>
#include <mmc.h>

DECLARE_GLOBAL_DATA_PTR;

#define PFC_BASE			0x11030000

#define ETH0_POC			(PFC_BASE + 0x3010) /* Ether ch0 IO Voltage Mode Control Register */
#define ETH1_POC			(PFC_BASE + 0x3014) /* Ether ch1 IO Voltage Mode Control Register */
#define ETH_PVDD_3300			0x00
#define ETH_PVDD_1800			0x01
#define ETH_PVDD_2500			0x02
#define ETH_MODE			(PFC_BASE + 0x3018) /* Ether MII/RGMII Mode Control Register */

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
#define CPG_SPI_DDIV                   (CPG_BASE + 0x220)
#define CPG_CLKDIV_STATUS		(CPG_BASE + 0x280)
#define CPG_CLKSEL_STATUS		(CPG_BASE + 0x284)
#define CPG_RST_USB			(CPG_BASE + 0x878)
#define CPG_RSTMON_USB			(CPG_BASE + 0x9F8)
#define CPG_CLKON_USB			(CPG_BASE + 0x578)
#define CPG_CLKMON_USB			(CPG_BASE + 0x6F8)

//g3l
#define CPG_ETH_SSEL			(CPG_BASE + 0x410)
#define CPG_ETH_SDIV			(CPG_BASE + 0x434)

/* PFC */
#define	PFC_P25				(PFC_BASE + 0x25)
#define	PFC_PM25			(PFC_BASE + 0x014A)
#define	PFC_PMC25			(PFC_BASE + 0x0225)

#define	PFC_P30				(PFC_BASE + 0x30)
#define	PFC_PM30			(PFC_BASE + 0x0160)
#define	PFC_PMC30			(PFC_BASE + 0x0230)
#define	PFC_PFC30			(PFC_BASE + 0x04c0)

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
#define	PFC_IEN_30_L		(PFC_BASE + 0x1980)
#define	PFC_IEN_30_H		(PFC_BASE + 0x1984)
#define	PFC_IEN_34			(PFC_BASE + 0x19A0)
#define	PFC_IEN_23			(PFC_BASE + 0x1918)
#define	PFC_IEN_24			(PFC_BASE + 0x1920)

//g3l Eth0
#define PFC_PMC2A			(PFC_BASE + 0x22A)
#define PFC_PMC2B			(PFC_BASE + 0x22B)
#define PFC_PMC2C			(PFC_BASE + 0x22C)
#define PFC_PFC2A			(PFC_BASE + 0x4A8)
#define PFC_PFC2B			(PFC_BASE + 0x4AC)
#define PFC_PFC2C			(PFC_BASE + 0x4B0)

//g3l Eth1
#define PFC_PMC2D			(PFC_BASE + 0x22D)
#define PFC_PMC2E			(PFC_BASE + 0x22E)
#define PFC_PMC2F			(PFC_BASE + 0x22F)
#define PFC_PFC2D			(PFC_BASE + 0x4B4)
#define PFC_PFC2E			(PFC_BASE + 0x4B8)
#define PFC_PFC2F			(PFC_BASE + 0x4BC)

//i2c
#define	PFC_PMC35			(PFC_BASE + 0x235)
#define	PFC_PFC35			(PFC_BASE + 0x4D4)

#define PFC_PWPR			(PFC_BASE + 0x3000)
#define PWPR_B0WI			BIT(7)	 /* Bit Write Disable */
#define PWPR_PFCWE			BIT(6)	/* PFC Register Write Enable */

#define USBPHY_BASE		(0x11e00000)
#define USB0_BASE		(0x11e10000)
#define USB1_BASE		(0x11e30000)
#define USBF_BASE		(0x11e20000)
#define USBPHY_RESET		(USBPHY_BASE + 0x000u)
#define COMMCTRL		0x800
#define HcRhDescriptorA		0x048
#define LPSTS			0x102
#define	AHB_BUS_CTR		0x208

void s_init(void)
{
	/* Disable GPIO Write Protect */
	(*(volatile u32 *)PFC_PWPR) &= ~(0x1u << 7);	/* PWPR.BOWI = 0 */
	(*(volatile u32 *)PFC_PWPR) |= (0x1u << 6);	/* PWPR.PFCWE = 1 */

	/* PG-PG5 function 1	*/
	*(volatile u32 *)(PFC_PFC30) = (*(volatile u32 *)(PFC_PFC30) & 0xFF000000) | 0x111111;
	*(volatile u8 *)(PFC_PMC30) |= 0x3F;	/* PG0-PG5 function	*/

#if CONFIG_TARGET_SMARC_RZG3L
	/* PJ1 = 0 QSD1_IOVS=0	*/
	
	*(volatile u8 *)(PFC_PMC33) &= ~(0x02);
	*(volatile u16 *)(PFC_PM33) = (*(volatile u16 *)(PFC_PM33) & 0xFF) | 0x08; /*bit[5:4]='B10	*/
	*(volatile u8 *)(PFC_P33) &= ~(0x02);
	/* PJ2 = 1 QSD1_PWEN=1	*/
	*(volatile u8 *)(PFC_PMC33) &= ~(0x04);
	*(volatile u16 *)(PFC_PM33) = (*(volatile u16 *)(PFC_PM33) & 0xFF) | 0x20; /*bit[5:4]='B10	*/
	*(volatile u8 *)(PFC_P33) |= 0x04;

#else
	/* P56 = 1 QSD1_PWEN=1	*/
	*(volatile u8 *)(PFC_PMC25) &= ~(0x40);
	*(volatile u16 *)(PFC_PM25) = (*(volatile u16 *)(PFC_PM25) & 0xFFF) | 0x2000; /*bit[13:12]='B10	*/
	*(volatile u8 *)(PFC_P25) |= 0x40;
#endif

	/* Input Enable Control PG0-PG5 Enable	*/
	*(volatile u32 *)(PFC_IEN_30_L) = 0x01010101;
	*(volatile u32 *)(PFC_IEN_30_H) = 0x00000101;

	//g3l
	/* Pinmux for ETH0 */
	*(volatile u32 *)(PFC_PFC2A) = 0x00001111;
	*(volatile u32 *)(PFC_PFC2B) = 0x11111111;
	*(volatile u32 *)(PFC_PFC2C) = 0x00000111;
	*(volatile u8 *)(PFC_PMC2A) = 0xF;
	*(volatile u8 *)(PFC_PMC2B) = 0xFF;
	*(volatile u8 *)(PFC_PMC2C) = 0x7;


	/* Pinmux for ETH1 */
	*(volatile u32 *)(PFC_PFC2D) = 0x00001111;
	*(volatile u32 *)(PFC_PFC2E) = 0x11111111;
	*(volatile u32 *)(PFC_PFC2F) = 0x00000111;
	*(volatile u8 *)(PFC_PMC2D) = 0xF;
	*(volatile u8 *)(PFC_PMC2E) = 0xFF;
	*(volatile u8 *)(PFC_PMC2F) = 0x7;


	/* Pinmux for I2C0	*/
	*(volatile u32 *)(PFC_PFC35) |= 0x4400;
	*(volatile u8 *)(PFC_PMC35)  |= 0x0C;

	/* can go in board_eht_init() once enabled */
	*(volatile u32 *)(ETH0_POC) = (*(volatile u32 *)(ETH0_POC) & 0xFFFFFFFC) | ETH_PVDD_1800;
	*(volatile u32 *)(ETH1_POC) = (*(volatile u32 *)(ETH1_POC) & 0xFFFFFFFC) | ETH_PVDD_1800;
	/* Enable RGMII for both ETH{0,1} */
	*(volatile u32 *)(ETH_MODE) = (*(volatile u32 *)(ETH_MODE) & 0xFFFFFFF0);

	*(volatile u32 *)(PFC_PWPR) = 0;
	*(volatile u32 *)(PFC_PWPR) = PWPR_B0WI;


	//g3l
	/* ETH CLK Setup for Eth0 and Eth1 */
	*(volatile u32 *)(CPG_CLKON_ETH) = 0x3FFF3FFF;
	while(*(volatile u32 *)(CPG_CLKMON_ETH) != 0x00003FFF);
	*(volatile u32 *)(CPG_ETH_SSEL) = 0xFFFF0202;  //source clock selection for ETH0 and ETH1
	*(volatile u32 *)(CPG_RESET_ETH) = 0x00030003;

	/*
	 * Setting SD CLKs.
	 * Currently, we use IMCLKs with output CLK rate 133 MHz, HSCLK will be considered to support later.
	 */
	*(volatile u32 *)(CPG_SDHI_DDIV) = 0x01110000;
	*(volatile u32 *)(CPG_SDHI_DSEL) = 0x01110222;
	while ((*(volatile u32 *)(CPG_CLKDIV_STATUS) != 0) || (*(volatile u32 *)(CPG_CLKSEL_STATUS) != 0))
		;
        /* I2C CLK */
        *(volatile u32 *)(CPG_RESET_I2C) = 0xF000F;

       /* Setting xSPI CLK 133 MHz */
       *(volatile u32 *)(CPG_SPI_DDIV) = 0x00010001;
       while (*(volatile u32 *)(CPG_CLKDIV_STATUS) != 0);
}

static void board_usb_init(void)
{
}

int board_early_init_f(void)
{
	return 0;
}

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_TEXT_BASE + 0x50000;
	board_usb_init();

	return 0;
}

void reset_cpu(void)
{
}
