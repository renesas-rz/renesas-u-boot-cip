// SPDX-License-Identifier: GPL-2.0
/*
 * arch/arm/mach-renesas/cpu_info-rcar.c
 *
 * Copyright (C) 2013,2014 Renesas Electronics Corporation
 */

#include <asm/io.h>
#include <linux/libfdt.h>
#include <linux/arm-smccc.h>
#include <asm/arch/renesas.h>

#define PRR_MASK		0x7fff
#define R8A7796_REV_1_0		0x5200
#define R8A7796_REV_1_1		0x5210
#define R8A7796_REV_1_3		0x5211
#define R8A77995_REV_1_1	0x5810

#define T2H_SMC_CPUID		0x82000012
static u32 renesas_get_prr(void)
{
	if (IS_ENABLED(CONFIG_RCAR_64))
		return readl(0xFFF00044);

	return readl(0xFF000044);
}

u32 renesas_get_cpu_type(void)
{
	struct arm_smccc_res res;
    arm_smccc_smc(T2H_SMC_CPUID, 0, 0, 0, 0, 0, 0, 0, &res);
    u32 id = (res.a0 >> 12) & 0xF;
    if (id == 0x3)
        return RENESAS_CPU_TYPE_RZT2H;
    else if (id == 0x4) // N2H
        return RENESAS_CPU_TYPE_RZN2H;
	else
		return 0;
}

u32 renesas_get_cpu_rev_integer(void)
{
	const u32 prr = renesas_get_prr();
	const u32 rev = prr & PRR_MASK;

	if (rev == R8A7796_REV_1_1 || rev == R8A7796_REV_1_3 ||
	    rev == R8A77995_REV_1_1)
		return 1;
	else
		return ((prr & 0x000000F0) >> 4) + 1;
}

u32 renesas_get_cpu_rev_fraction(void)
{
	const u32 prr = renesas_get_prr();
	const u32 rev = prr & PRR_MASK;

	if (rev == R8A7796_REV_1_1 || rev == R8A77995_REV_1_1)
		return 1;
	else if (rev == R8A7796_REV_1_3)
		return 3;
	else
		return prr & 0x0000000F;
}
