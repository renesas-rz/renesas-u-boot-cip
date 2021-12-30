// SPDX-License-Identifier: GPL-2.0+
/*
 * board/silinux/ek874/ek874.c
 *     This file is ek874 board support.
 *
 * Copyright (C) 2021 Renesas Electronics Corporation
 */

#include <common.h>
#include <env.h>
#include <asm/global_data.h>
#include <asm/arch/rmobile.h>
#include <asm/io.h>

#define RST_BASE	0xE6160000
#define RST_CA53RESCNT	(RST_BASE + 0x44)
#define RST_CA53_CODE	0x5A5A000F

DECLARE_GLOBAL_DATA_PTR;

#define PFC_PMMR	0xE6060000
#define PFC_PUEN4	0xE6060410
#define PFC_PUD4	0xE6060450
#define PFC_GPSR5	0xE6060114
#define GPIO_IOINTSEL5	0xE6055000
#define GPIO_INOUTSEL5	0xE6055004
#define GPIO_INDT5	0xE605500C
#define MSTP907_GPIO5	BIT(7)

static char board_rev;

char check_rev(void)
{
	u32 reg;
	int ret;

	if ((rmobile_get_cpu_rev_integer() == 1) &&
	    (rmobile_get_cpu_rev_fraction() < 1)) {
		ret = 'B';
	} else {
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

		ret = (ret ? 'C' : 'E');
	}

	return ret;
};
int board_init(void)
{
	/* Check board revision */
	board_rev = check_rev();

	/* address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_TEXT_BASE + 0x50000;

	return 0;
}

void reset_cpu(void)
{
	writel(RST_CA53_CODE, RST_CA53RESCNT);
}
int board_late_init(void)
{
	env_set("board_rev", &board_rev);
	return 0;
}
