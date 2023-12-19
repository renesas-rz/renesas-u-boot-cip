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
#define CPG_RESET_BASE			(CPG_BASE + 0x800)
#define CPG_RESET_ETH			(CPG_RESET_BASE + 0x7C)
#define CPG_RESET_I2C			(CPG_RESET_BASE + 0x80)
#define CPG_SDHI_DDIV			(CPG_BASE + 0x218)
#define CPG_SDHI_DSEL			(CPG_BASE + 0x244)
#define CPG_CLKDIV_STATUS		(CPG_BASE + 0x280)
#define CPG_CLKSEL_STATUS		(CPG_BASE + 0x284)

/* PFC */
#define	PFC_P25				(PFC_BASE + 0x25)
#define	PFC_PM25			(PFC_BASE + 0x014A)
#define	PFC_PMC25			(PFC_BASE + 0x0225)

void s_init(void)
{
#if CONFIG_TARGET_RZG3S_DEV
	/* SD1 power control : P13_4 = 1 P13_0 = 1 */
	/* Set P13_4(bit4) and P13_0(bit0) to Port Mode */
	*(volatile u8 *)(PFC_PMC25) &= 0xEE;
	/* Set P13_4(bit[9:8]) and P13_0(bit[1:0]) to Output mode */
	*(volatile u16 *)(PFC_PM25) = (*(volatile u16 *)(PFC_PM25) & 0xFCFC) | 0x0202;
	/* Set P13_4(bit4) and P13_0(bit0) to High output */
	*(volatile u8 *)(PFC_P25) = (*(volatile u8 *)(PFC_P25) & 0xEE) | 0x11;
	/* can go in board_eht_init() once enabled */
	*(volatile u32 *)(ETH0_POC) = (*(volatile u32 *)(ETH0_POC) & 0xFFFFFFFC) | ETH_PVDD_3300;
	*(volatile u32 *)(ETH1_POC) = (*(volatile u32 *)(ETH1_POC) & 0xFFFFFFFC) | ETH_PVDD_3300;
	/* Enable RGMII for both ETH{0,1} */
	*(volatile u32 *)(ETH_MODE) = (*(volatile u32 *)(ETH_MODE) & 0xFFFFFFF0);
	/* ETH CLK */
	*(volatile u32 *)(CPG_RESET_ETH) = 0x00030003;
#endif
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
}

int board_early_init_f(void)
{
	return 0;
}

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_TEXT_BASE + 0x50000;

	return 0;
}

void reset_cpu(void)
{
}
