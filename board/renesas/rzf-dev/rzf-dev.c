// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

#include <common.h>
#include <flash.h>
#include <image.h>
#include <init.h>
#include <net.h>
#if defined(CONFIG_FTMAC100) && !defined(CONFIG_DM_ETH)
#include <netdev.h>
#endif
#include <linux/io.h>
#include <faraday/ftsmc020.h>
#include <fdtdec.h>
#include <asm/gpio.h>
#include <dm.h>
#include <spl.h>
#include <asm/sections.h>

#include "include/rzf_def.h"
#include "include/mmio.h"
#include "include/pfc.h"
#include "include/cpg.h"
#include "include/ddr.h"
#include "include/sys.h"
#include "include/sys_regs.h"
#include "include/spi_multi.h"

DECLARE_GLOBAL_DATA_PTR;

void spl_early_board_init_f(void);


int board_init(void)
{
	gd->bd->bi_boot_params = CONFIG_SYS_TEXT_BASE + 0x50000;

	return 0;
}


int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}


void *board_fdt_blob_setup(void)
{
	void *fdt_blob = NULL;
#ifdef CONFIG_SPL_BUILD
	/* FDT is at end of BSS unless it is in a different memory region */
	if (IS_ENABLED(CONFIG_SPL_SEPARATE_BSS))
		fdt_blob = (ulong *)&_image_binary_end;
	else
		fdt_blob = (ulong *)&__bss_end;
#else
#ifdef CONFIG_OF_PRIOR_STAGE
extern phys_addr_t prior_stage_fdt_address;

        fdt_blob = (void *)prior_stage_fdt_address;
#else
        /* FDT is at end of image */
    	fdt_blob = (ulong *)&_end;
#endif
#endif
	return fdt_blob;
}


#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* boot using first FIT config */
	return 0;
}
#endif


#ifdef CONFIG_SPL
u32 spl_boot_device(void)
{
    uint16_t boot_dev;

#ifdef CONFIG_DEBUG_RZF_FPGA
	boot_dev = *((uint16_t *)RZF_BOOTINFO_BASE) & MASK_BOOTM_DEVICE;
#else
    boot_dev = BOOT_MODE_SPI_1_8;
#endif
    switch (boot_dev)
    {
    case BOOT_MODE_SPI_1_8:
    case BOOT_MODE_SPI_3_3:
        return BOOT_DEVICE_NOR;
        
    case BOOT_MODE_ESD:
    case BOOT_MODE_EMMC_1_8:
    case BOOT_MODE_EMMC_3_3:
        return BOOT_DEVICE_MMC1;
        
    case BOOT_MODE_SCIF:
    default:
        return BOOT_DEVICE_NONE;
    }
}
#endif


#ifdef CONFIG_SPL_BUILD
void spl_early_board_init_f(void)
{
	/* setup PFC */
	pfc_setup();

	/* setup Clock and Reset */
	cpg_setup();
}

int spl_board_init_f(void)
{
	uint16_t boot_dev;

	/* initialize DDR */
	ddr_setup();

    /* initisalize SPI Multi when SPI BOOT */
	boot_dev = *((uint16_t *)RZF_BOOTINFO_BASE) & MASK_BOOTM_DEVICE;
	if (boot_dev == BOOT_MODE_SPI_1_8 ||
		boot_dev == BOOT_MODE_SPI_3_3) {
		spi_multi_setup(SPI_MULTI_ADDR_WIDES_24, SPI_MULTI_DQ_WIDES_1_4_4, SPI_MULTI_DUMMY_10CYCLE);
	}
    
	return 0;
}

void board_init_f(ulong dummy)
{
	int ret;

    /* Initialize SPL*/
	ret = spl_early_init();
	if (ret)
		panic("spl_early_init() failed: %d\n", ret);

    /* Initialize CPU Architecure */
	arch_cpu_init_dm();

    /* Initialixe Bord part */
    spl_early_board_init_f();
    
    /* Initialize console */
	preloader_console_init();

    /* Initialize Board part */
	ret = spl_board_init_f();
	if (ret)
		panic("spl_board_init_f() failed: %d\n", ret);
}
#endif
