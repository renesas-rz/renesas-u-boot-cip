// SPDX-License-Identifier: GPL-2.0+
/*
 * Renesas RCar Gen3 memory map tables
 *
 * Copyright (C) 2017 Marek Vasut <marek.vasut@gmail.com>
 */

#include <common.h>
#include <asm/armv8/mmu.h>

#define R9A09G011_NR_REGIONS 16

static struct mm_region r9a09G011_mem_map[R9A09G011_NR_REGIONS] = {
	{
		/* DDR 1-S */
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* Slave */
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = 0x40000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* PCI */
		.virt = 0xc0000000UL,
		.phys = 0xc0000000UL,
		.size = 0x40000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* DDR 1-M */
		.virt = 0x100000000UL,
		.phys = 0x100000000UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* DDR 2 */
		.virt = 0x180000000UL,
		.phys = 0x180000000UL,
		.size = 0x180000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_NON_SHARE
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = r9a09G011_mem_map;
