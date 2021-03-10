// SPDX-License-Identifier: GPL-2.0+
/*
 * board/renesas/ebisu/ebisu.c
 *     This file is EK874 board support.
 *
 * Copyright (C) 2018 Marek Vasut <marek.vasut+renesas@gmail.com>
 */

#include <common.h>
#include <env.h>
#include <cpu_func.h>
#include <hang.h>
#include <init.h>
#include <malloc.h>
#include <netdev.h>
#include <dm.h>
#include <dm/platform_data/serial_sh.h>
#include <asm/processor.h>
#include <asm/mach-types.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/arch/rmobile.h>
#include <asm/arch/rcar-mstp.h>
#include <asm/arch/sh_sdhi.h>
#include <i2c.h>
#include <mmc.h>

DECLARE_GLOBAL_DATA_PTR;

#define	PFC_PMMR		0xE6060000
#define	PFC_PUEN4		0xE6060410
#define	PFC_PUD4		0xE6060450
#define	PFC_GPSR5		0xE6060114
#define	GPIO_IOINTSEL5		0xE6055000
#define	GPIO_INOUTSEL5		0xE6055004
#define	GPIO_INDT5		0xE605500C
#define	MSTP907_GPIO5		BIT(7)

static char board_rev;

char check_rev(void)
{
	u32 reg;
	int ret;

	/* Check GPIO/Peripheral Function mode */
	reg = readl(PFC_GPSR5) & BIT(19);
	if (reg) {
		reg = readl(PFC_GPSR5) & ~BIT(19);
		writel(~reg, PFC_PMMR);
		writel(reg, PFC_GPSR5);
	}

	/* Check Clock support to Module GPIO5 */
	reg = readl(MSTPSR9) & MSTP907_GPIO5;
	if (reg)
		writel(readl(SMSTPCR9) & ~MSTP907_GPIO5, SMSTPCR9);

	/* Enable Pull up/down function */
	writel(readl(PFC_PUEN4) | BIT(17), PFC_PUEN4);
	/* Enable Pull up */
	writel(readl(PFC_PUD4) | BIT(17), PFC_PUD4);

	/* Set General input/output mode for GP5_19 */
	writel(readl(GPIO_IOINTSEL5) & ~BIT(19), GPIO_IOINTSEL5);
	/* Set General input mode for GP5_19 */
	writel(readl(GPIO_INOUTSEL5) & ~BIT(19), GPIO_INOUTSEL5);
	/* Read the value received through GP5_19 */
	ret = readl(GPIO_INDT5) & BIT(19);

	/* Enable Pull down */
	writel(readl(PFC_PUD4) & BIT(17), PFC_PUD4);
	/* Disable Pull up/down function */
	writel(readl(PFC_PUEN4) & BIT(17), PFC_PUEN4);

	return ret ? 'C' : 'E';
}

void s_init(void)
{
}

int board_early_init_f(void)
{
	return 0;
}

int board_init(void)
{
	board_rev = check_rev();
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_TEXT_BASE + 0x50000;

	return 0;
}

int board_late_init(void)
{
	env_set("board_rev", &board_rev);

	return 0;
}

#define RST_BASE	0xE6160000
#define RST_CA53RESCNT	(RST_BASE + 0x44)
#define RST_CA53_CODE	0x5A5A000F

void reset_cpu(ulong addr)
{
	writel(RST_CA53_CODE, RST_CA53RESCNT);
}

void board_add_ram_info(int use_default)
{
	int i;

	printf("\n");
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		if (!gd->bd->bi_dram[i].size)
			break;
		printf("Bank #%d: 0x%09llx - 0x%09llx, ", i,
		      (unsigned long long)(gd->bd->bi_dram[i].start),
		      (unsigned long long)(gd->bd->bi_dram[i].start
		      + gd->bd->bi_dram[i].size - 1));
		print_size(gd->bd->bi_dram[i].size, "\n");
	};
}
