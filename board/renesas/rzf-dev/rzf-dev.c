// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021, Renesas Electronics Corporation. All rights reserved.
 */

#include <common.h>
#include <spl.h>
#include <init.h>
#include <asm/sections.h>

#include <renesas/rzf-dev/rzf-dev_def.h>
#include <renesas/rzf-dev/rzf-dev_sys.h>
#include "rzf-dev_spi_multi.h"

extern void cpg_setup(void);
extern void pfc_setup(void);
extern void ddr_setup(void);
extern int spi_multi_setup(uint32_t addr_width, uint32_t dq_width, uint32_t dummy_cycle);


DECLARE_GLOBAL_DATA_PTR;

void spl_early_board_init_f(void);
extern phys_addr_t prior_stage_fdt_address;
/*
 * Miscellaneous platform dependent initializations
 */
#define PFC_BASE	0x11030000

#define ETH_CH0		(PFC_BASE + 0x300c)
#define ETH_CH1		(PFC_BASE + 0x3010)
#define I2C_CH1 	(PFC_BASE + 0x1870)
#define ETH_PVDD_3300	0x00
#define ETH_PVDD_1800	0x01
#define ETH_PVDD_2500	0x02
#define ETH_MII_RGMII	(PFC_BASE + 0x3018)

/* CPG */
#define CPG_BASE					0x11010000
#define CPG_CLKON_BASE				(CPG_BASE + 0x500)
#define CPG_RST_BASE				(CPG_BASE + 0x800)
#define CPG_RST_ETH				(CPG_RST_BASE + 0x7C)
#define CPG_RST_I2C				(CPG_RST_BASE + 0x80)

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
#ifdef CONFIG_V5L2_CACHE
	v5l2_init();
#endif

	/* can go in board_eht_init() once enabled */
	*(volatile u32 *)(ETH_CH0) = (*(volatile u32 *)(ETH_CH0) & 0xFFFFFFFC) | ETH_PVDD_1800;
	*(volatile u32 *)(ETH_CH1) = (*(volatile u32 *)(ETH_CH1) & 0xFFFFFFFC) | ETH_PVDD_1800;
	/* Enable RGMII for both ETH{0,1} */
	*(volatile u32 *)(ETH_MII_RGMII) = (*(volatile u32 *)(ETH_MII_RGMII) & 0xFFFFFFFC);
	/* ETH CLK */
	*(volatile u32 *)(CPG_RST_ETH) = 0x30003;
	/* I2C CLK */
	*(volatile u32 *)(CPG_RST_I2C) = 0xF000F;
	/* I2C pin non GPIO enable */
	*(volatile u32 *)(I2C_CH1) = 0x01010101;

	return 0;
}
#endif

/* PFC */
#define PFC_P22						(PFC_BASE + 0x0022)
#define PFC_PM22					(PFC_BASE + 0x0144)
#define PFC_PMC22					(PFC_BASE + 0x0222)

/* SDHI */
#define CONFIG_SYS_SH_SDHI0_BASE	0x11C00000
#define CONFIG_SYS_SH_SDHI1_BASE	0x11C10000

#define BIT(nr)			(1UL << (nr))
#define SH_SDHI_QUIRK_64BIT_BUF		BIT(1)

int sh_sdhi_init(unsigned long addr, int ch, unsigned long quirks);

int board_mmc_init(struct bd_info *bis)
{
	int ret = 0;

	/* SD1 power control: P18_5 = 1; */
	*(volatile u32 *)(PFC_PMC22) &= 0xFFFFFFDF; /* Port func mode 0b0 */
	*(volatile u32 *)(PFC_PM22) = (*(volatile u32 *)(PFC_PM22) & 0xFFFFF3FF) | 0x800; /* Port output mode 0b10 */
	*(volatile u32 *)(PFC_P22) = (*(volatile u32 *)(PFC_P22) & 0xFFFFFFDF) | 0x20;	/* Port 13[2:1] output value 0b1*/


	ret |= sh_sdhi_init(CONFIG_SYS_SH_SDHI0_BASE,
						0,
						SH_SDHI_QUIRK_64BIT_BUF);
	ret |= sh_sdhi_init(CONFIG_SYS_SH_SDHI1_BASE,
						1,
						SH_SDHI_QUIRK_64BIT_BUF);

	return ret;
}

int board_init(void)
{
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

#ifndef CONFIG_DEBUG_RZF_FPGA
	/* initialize DDR */
	ddr_setup();
#endif

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
