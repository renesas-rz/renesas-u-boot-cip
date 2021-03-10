// SPDX-License-Identifier: GPL-2.0
/*
 * board/renesas/rcar-common/common.c
 *
 * Copyright (C) 2013 Renesas Electronics Corporation
 * Copyright (C) 2013 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 * Copyright (C) 2015 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 */

#include <common.h>
#include <dm.h>
#include <init.h>
#include <dm/uclass-internal.h>
#include <asm/arch/rmobile.h>
#include <linux/libfdt.h>

#ifdef CONFIG_RCAR_GEN3
#if defined(CONFIG_R8A774E1)
#include <asm/system.h>
#include <asm/ptrace.h>

#include "../rzg-common/common.h"
#endif

DECLARE_GLOBAL_DATA_PTR;

/* If the firmware passed a device tree use it for U-Boot DRAM setup. */
extern u64 rcar_atf_boot_args[];

int fdtdec_board_setup(const void *fdt_blob)
{
	void *atf_fdt_blob = (void *)(rcar_atf_boot_args[1]);

	if (fdt_magic(atf_fdt_blob) == FDT_MAGIC)
		fdt_overlay_apply_node((void *)fdt_blob, 0, atf_fdt_blob, 0);

	return 0;
}

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
#if defined(CONFIG_R8A774E1)
	int use_ecc = 0;
	struct pt_regs regs;
#endif

	fdtdec_setup_memory_banksize();

#if defined(CONFIG_R8A774E1)
	/* Setting SiP Service GET_ECC_MODE command*/
	regs.regs[0] = RZG_SIP_SVC_GET_ECC_MODE;
	smc_call(&regs);
	/* First result is USE ECC or not*/
	use_ecc = regs.regs[0];

	if (use_ecc == 1) {
		int bank;

		for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
			if ((gd->bd->bi_dram[bank].start & (0x500000000U)) ==
			    (0x500000000U)) {
				gd->bd->bi_dram[bank].start =
				  (gd->bd->bi_dram[bank].start & 0x0FFFFFFFFU)
				  | 0x600000000U;
			}
		}
	}
#endif
	return 0;
}
#endif
