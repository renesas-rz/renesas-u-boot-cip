// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

/* CPU specific code */
#include <common.h>
#include <cpu_func.h>
#include <irq_func.h>
#include <asm/cache.h>
#include <asm/sbi.h>

#define SBI_EXT_VENDOR 0x09000000

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
	icache_disable();
	dcache_disable();

	/* Enable L1 D-cache before going to Linux */
	sbi_ecall(SBI_EXT_VENDOR, 18, 0, 0, 0, 0, 0, 0);

	return 0;
}
