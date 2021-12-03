// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021, Renesas Electronics Corporation. All rights reserved.
 */

#include <common.h>
#include <spl.h>
#include <init.h>
#include <asm/sections.h>

#include "include/rzf-dev_def.h"
#include "include/rzf-dev_pfc.h"
#include "include/rzf-dev_cpg.h"
#include "include/rzf-dev_ddr.h"
#include "include/rzf-dev_sys.h"
#include "include/rzf-dev_spi_multi.h"

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

#ifdef CONFIG_DEBUG_RZF_FPGA
    return BOOT_DEVICE_NOR;
#else
    uint16_t boot_dev;
    
	boot_dev = *((uint16_t *)RZF_BOOTINFO_BASE) & MASK_BOOTM_DEVICE;
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
#endif
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
