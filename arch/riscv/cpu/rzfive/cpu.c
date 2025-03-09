// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Renesas Electronics Corporation
 * Based on arch/riscv/cpu/andesv5/cpu.c
 */

/* CPU specific code */
#include <common.h>
#include <cpu_func.h>
#include <irq_func.h>
#include <asm/cache.h>
#include <asm/csr.h>

#define CSR_MCACHE_CTL		0x7ca
#define CSR_MMISC_CTL		0x7d0
#define CSR_MARCHID		0xf12
#define CSR_MMISCCTL		0x7d0

#define V5_MCACHE_CTL_IC_EN_OFFSET		0
#define V5_MCACHE_CTL_DC_EN_OFFSET		1
#define V5_MMISC_CTL_MSA_OR_UNA_OFFSET		6
#define V5_MCACHE_CTL_CCTL_SUEN_OFFSET		8
#define V5_MCACHE_CTL_L1I_PREFETCH_OFFSET	9
#define V5_MCACHE_CTL_L1D_PREFETCH_OFFSET	10
#define V5_MCACHE_CTL_DC_WAROUND_OFFSET_1	13
#define V5_MCACHE_CTL_DC_WAROUND_OFFSET_2	14
#define V5_MCACHE_CTL_DC_COHEN_OFFSET		19
#define V5_MCACHE_CTL_DC_COHSTA_OFFSET		20

#define V5_MMISC_CTL_NON_BLOCKING_OFFSET        8

#define V5_MMISC_CTL_NON_BLOCKING_EN    BIT(V5_MMISC_CTL_NON_BLOCKING_OFFSET)
#define V5_MCACHE_CTL_IC_EN		BIT(V5_MCACHE_CTL_IC_EN_OFFSET)
#define V5_MCACHE_CTL_DC_EN		BIT(V5_MCACHE_CTL_DC_EN_OFFSET)
#define V5_MCACHE_CTL_CCTL_SUEN		BIT(V5_MCACHE_CTL_CCTL_SUEN_OFFSET)
#define V5_MCACHE_CTL_DC_COHEN_EN	BIT(V5_MCACHE_CTL_DC_COHEN_OFFSET)
#define V5_MCACHE_CTL_L1I_PREFETCH_EN	BIT(V5_MCACHE_CTL_L1I_PREFETCH_OFFSET)
#define V5_MCACHE_CTL_L1D_PREFETCH_EN	BIT(V5_MCACHE_CTL_L1D_PREFETCH_OFFSET)
#define V5_MCACHE_CTL_DC_WAROUND_1_EN	BIT(V5_MCACHE_CTL_DC_WAROUND_OFFSET_1)
#define V5_MCACHE_CTL_DC_WAROUND_2_EN	BIT(V5_MCACHE_CTL_DC_WAROUND_OFFSET_2)
#define V5_MCACHE_CTL_DC_COHSTA_EN	BIT(V5_MCACHE_CTL_DC_COHSTA_OFFSET)
#define V5_MMISC_CTL_MSA_OR_UNA_EN      BIT(V5_MMISC_CTL_MSA_OR_UNA_OFFSET)

/*
 * cleanup_before_linux() is called just before we call linux
 * it prepares the processor for linux
 *
 * we disable interrupt and caches.
 */
int cleanup_before_linux(void)
{
	disable_interrupts();

	/* turn off I/D-cache */
	cache_flush();

	return 0;
}

void harts_early_init(void)
{
	if (CONFIG_IS_ENABLED(RISCV_MMODE)) {
		unsigned long long mcache_ctl_val = csr_read(CSR_MCACHE_CTL);

		/* enable L1 cache */
		if (!(mcache_ctl_val & V5_MCACHE_CTL_IC_EN))
			mcache_ctl_val |= V5_MCACHE_CTL_IC_EN;
		if (!(mcache_ctl_val & V5_MCACHE_CTL_DC_EN))
			mcache_ctl_val |= V5_MCACHE_CTL_DC_EN;
		if (!(mcache_ctl_val & V5_MCACHE_CTL_CCTL_SUEN))
			mcache_ctl_val |= V5_MCACHE_CTL_CCTL_SUEN;
		if (!(mcache_ctl_val & V5_MCACHE_CTL_L1I_PREFETCH_EN))
			mcache_ctl_val |= V5_MCACHE_CTL_L1I_PREFETCH_EN;
		if (!(mcache_ctl_val & V5_MCACHE_CTL_L1D_PREFETCH_EN))
			mcache_ctl_val |= V5_MCACHE_CTL_L1D_PREFETCH_EN;
		mcache_ctl_val &= ~(V5_MCACHE_CTL_DC_WAROUND_1_EN | V5_MCACHE_CTL_DC_WAROUND_2_EN);
		mcache_ctl_val |= V5_MCACHE_CTL_DC_WAROUND_1_EN;
		csr_write(CSR_MCACHE_CTL, mcache_ctl_val);

		unsigned long long mmisc_ctl_val = csr_read(CSR_MMISCCTL);
		if (!(mmisc_ctl_val & V5_MMISC_CTL_NON_BLOCKING_EN))
			mmisc_ctl_val |= V5_MMISC_CTL_NON_BLOCKING_EN;
		mmisc_ctl_val &= ~(V5_MMISC_CTL_MSA_OR_UNA_EN);
		csr_write(CSR_MMISCCTL, mmisc_ctl_val);

	}
}
