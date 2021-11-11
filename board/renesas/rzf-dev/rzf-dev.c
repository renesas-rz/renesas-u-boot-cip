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
#include <dm.h>
#include <spl.h>

DECLARE_GLOBAL_DATA_PTR;

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
#define CPG_RESET_BASE				(CPG_BASE + 0x800)
#define CPG_RESET_ETH				(CPG_RESET_BASE + 0x7C)
#define CPG_RESET_I2C                           (CPG_RESET_BASE + 0x80)

void s_init(void)
{
	/* can go in board_eht_init() once enabled */
	*(volatile u32 *)(ETH_CH0) = (*(volatile u32 *)(ETH_CH0) & 0xFFFFFFFC) | ETH_PVDD_1800;
	*(volatile u32 *)(ETH_CH1) = (*(volatile u32 *)(ETH_CH1) & 0xFFFFFFFC) | ETH_PVDD_1800;
	/* Enable RGMII for both ETH{0,1} */
	*(volatile u32 *)(ETH_MII_RGMII) = (*(volatile u32 *)(ETH_MII_RGMII) & 0xFFFFFFFC);
	/* ETH CLK */
	*(volatile u32 *)(CPG_RESET_ETH) = 0x30003;
	/* I2C CLK */
	*(volatile u32 *)(CPG_RESET_I2C) = 0xF000F;
	/* I2C pin non GPIO enable */
	*(volatile u32 *)(I2C_CH1) = 0x01010101;
}


/* PFC */
#define PFC_P37						(PFC_BASE + 0x037)
#define PFC_PM37					(PFC_BASE + 0x16E)
#define PFC_PMC37					(PFC_BASE + 0x237)

#define CONFIG_SYS_SH_SDHI0_BASE	0x11C00000
#define CONFIG_SYS_SH_SDHI1_BASE	0x11C10000

#define BIT(nr)			(1UL << (nr))
#define SH_SDHI_QUIRK_16BIT_BUF		BIT(0)
#define SH_SDHI_QUIRK_64BIT_BUF		BIT(1)

int sh_sdhi_init(unsigned long addr, int ch, unsigned long quirks);

int board_mmc_init(struct bd_info *bis)
{
	int ret = 0;
	
	/* SD1 power control: P39_1 = 0; P39_2 = 1; */
	*(volatile u32 *)(PFC_PMC37) &= 0xFFFFFFF9; /* Port func mode 0b00 */
	*(volatile u32 *)(PFC_PM37) = (*(volatile u32 *)(PFC_PM37) & 0xFFFFFFC3) | 0x28; /* Port output mode 0b1010 */
#if CONFIG_TARGET_SMARC_RZG2L
	*(volatile u32 *)(PFC_P37) = (*(volatile u32 *)(PFC_P37) & 0xFFFFFFF9) | 0x6;	/* Port 39[2:1] output value 0b11*/
#else

	*(volatile u32 *)(PFC_P37) = (*(volatile u32 *)(PFC_P37) & 0xFFFFFFF9) | 0x4;	/* Port 39[2:1] output value 0b10*/
#endif
		
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
	gd->bd->bi_boot_params = PHYS_SDRAM_0 + 0x400;

	return 0;
}

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

#if defined(CONFIG_FTMAC100) && !defined(CONFIG_DM_ETH)
int board_eth_init(struct bd_info *bd)
{
	return ftmac100_initialize(bd);
}
#endif

ulong board_flash_get_legacy(ulong base, int banknum, flash_info_t *info)
{
	return 0;
}

void *board_fdt_blob_setup(void)
{
	return (void *)CONFIG_SYS_FDT_BASE;
}

int smc_init(void)
{
	int node = -1;
	const char *compat = "andestech,atfsmc020";
	void *blob = (void *)gd->fdt_blob;
	fdt_addr_t addr;
	struct ftsmc020_bank *regs;

	node = fdt_node_offset_by_compatible(blob, -1, compat);
	if (node < 0)
		return -FDT_ERR_NOTFOUND;

	addr = fdtdec_get_addr_size_auto_noparent(blob, node,
		"reg", 0, NULL, false);

	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	regs = (struct ftsmc020_bank *)addr;
	regs->cr &= ~FTSMC020_BANK_WPROT;

	return 0;
}

static void v5l2_init(void)
{
	struct udevice *dev;

	uclass_get_device(UCLASS_CACHE, 0, &dev);
}

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	smc_init();
	v5l2_init();

	return 0;
}
#endif

#ifdef CONFIG_SPL
void board_boot_order(u32 *spl_boot_list)
{
	u8 i;
	u32 boot_devices[] = {
#ifdef CONFIG_SPL_RAM_SUPPORT
		BOOT_DEVICE_RAM,
#endif
#ifdef CONFIG_SPL_MMC_SUPPORT
		BOOT_DEVICE_MMC1,
#endif
	};

	for (i = 0; i < ARRAY_SIZE(boot_devices); i++)
		spl_boot_list[i] = boot_devices[i];
}
#endif

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* boot using first FIT config */
	return 0;
}
#endif

void rzf_early_platform_setup(void);
void rzf_platform_setup(void);

int spl_board_init_f(void)
{
    // initialize pinconfig,clock,reset control
    rzf_early_platform_setup();
    
    // initialize DDR, setup IO
    rzf_platform_setup();
    
	return 0;
}
