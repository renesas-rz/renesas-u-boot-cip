// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021, Renesas Electronics Corporation. All rights reserved.
 */

#include <common.h>
#include <spl.h>
#include <init.h>
#include <dm.h>
#include <asm/sections.h>
#include <asm/arch/sh_sdhi.h>
#include <mmc.h>
#include <i2c.h>
#include <hang.h>
#include <cache.h>
#include <renesas/rzf-dev/rzf-dev_def.h>
#include <renesas/rzf-dev/rzf-dev_sys.h>
#include <renesas/rzf-dev/rzf-dev_pfc_regs.h>
#include <renesas/rzf-dev/rzf-dev_cpg_regs.h>

extern void cpg_setup(void);
extern void pfc_setup(void);
extern void ddr_setup(void);
extern int spi_multi_setup(void);


DECLARE_GLOBAL_DATA_PTR;

void spl_early_board_init_f(void);
extern phys_addr_t prior_stage_fdt_address;
/*
 * Miscellaneous platform dependent initializations
 */
static void v5l2_init(void)
{
	struct udevice *dev;

	uclass_get_device(UCLASS_CACHE, 0, &dev);

	if (dev)
		cache_enable(dev);
}

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{

#if CONFIG_TARGET_SMARC_RZF
	/* can go in board_eht_init() once enabled */
	*(volatile u32 *)(PFC_ETH_ch0) = (*(volatile u32 *)(PFC_ETH_ch0) & 0xFFFFFFFC) | ETH_ch0_3_3;
	*(volatile u32 *)(PFC_ETH_ch1) = (*(volatile u32 *)(PFC_ETH_ch1) & 0xFFFFFFFC) | ETH_ch1_1_8;
	/* Enable RGMII for both ETH{0,1} */
	*(volatile u32 *)(PFC_ETH_MII) = (*(volatile u32 *)(PFC_ETH_MII) & 0xFFFFFFFC);
	/* ETH CLK */
	*(volatile u32 *)(CPG_RST_ETH) = 0x30002;
#else
	/* can go in board_eht_init() once enabled */
	*(volatile u32 *)(PFC_ETH_ch0) = (*(volatile u32 *)(PFC_ETH_ch0) & 0xFFFFFFFC) | ETH_ch0_3_3;
	*(volatile u32 *)(PFC_ETH_ch1) = (*(volatile u32 *)(PFC_ETH_ch1) & 0xFFFFFFFC) | ETH_ch1_3_3;
	/* Enable RGMII for both ETH{0,1} */
	*(volatile u32 *)(PFC_ETH_MII) = (*(volatile u32 *)(PFC_ETH_MII) & 0xFFFFFFFC);
	/* ETH CLK */
	*(volatile u32 *)(CPG_RST_ETH) = 0x30003;
#endif

	return 0;
}
#endif

int board_mmc_init(struct bd_info *bis)
{
	int ret = 0;

#if CONFIG_TARGET_SMARC_RZF
	/* SD1 power control : P0_3 = 1 P6_1 = 1	*/
	*(volatile u8 *)(PFC_PMC10) &= PMC_PMC3_MASK;	/* Port func mode 0b00	*/
	*(volatile u8 *)(PFC_PMC16) &= PMC_PMC1_MASK;	/* Port func mode 0b00	*/
	*(volatile u16 *)(PFC_PM10) = (*(volatile u16 *)(PFC_PM10) & PM3_MASK) | PM3_OUT_DIS; /* Port output mode 0b10 */
	*(volatile u16 *)(PFC_PM16) = (*(volatile u16 *)(PFC_PM16) & PM1_MASK) | PM1_OUT_DIS; /* Port output mode 0b10 */
	*(volatile u8 *)(PFC_P10) = (*(volatile u8 *)(PFC_P10) & P_P3_MASK) | P_P3; /* P0_3  output 1	*/
	*(volatile u8 *)(PFC_P16) = (*(volatile u8 *)(PFC_P16) & P_P1_MASK) | P_P1; /* P6_1  output 1	*/
#else
	/* SD0 power control: P18_4 = 1; */
	*(volatile u8 *)(PFC_PMC15) &= PMC_PMC4_MASK;
	*(volatile u8 *)(PFC_PMC22) &= PMC_PMC4_MASK; /* Port func mode 0b0 */
	*(volatile u16 *)(PFC_PM15) = (*(volatile u16 *)(PFC_PM15) & PM4_MASK) | PM4_OUT_DIS; /* Port output mode 0b10 */
	*(volatile u16 *)(PFC_PM22) = (*(volatile u16 *)(PFC_PM22) & PM4_MASK) | PM4_OUT_DIS; /* Port output mode 0b10 */
	*(volatile u8 *)(PFC_P22) = (*(volatile u8 *)(PFC_P22) & P_P4_MASK) | P_P4;	/* P18_4 output 1 */
#if (CONFIG_RZF_SDHI0_VOLTAGE == 3300)
	/* SD0 1.8V power control: P5_4=1; */
	*(volatile u8 *)(PFC_P15) = (*(volatile u8 *)(PFC_P15) & P_P4_MASK) | P_P4;	/* P5_4 output 1 */
#else
	/* SD0 3.3V power control: P5_4=0; */
	*(volatile u8 *)(PFC_P15) = (*(volatile u8 *)(PFC_P15) & P_P4_MASK);	/* P5_4 output 0 */
#endif
	/* SD1 power control: P6_2=1,P18_5 = 1; */
	*(volatile u8 *)(PFC_PMC16) &= PMC_PMC2_MASK; /* Port func mode 0b0 */
	*(volatile u8 *)(PFC_PMC22) &= PMC_PMC5_MASK; /* Port func mode 0b0 */
	*(volatile u16 *)(PFC_PM16) = (*(volatile u16 *)(PFC_PM16) & PM2_MASK) | PM2_OUT_DIS; /* Port output mode 0b10 */
	*(volatile u16 *)(PFC_PM22) = (*(volatile u16 *)(PFC_PM22) & PM5_MASK) | PM5_OUT_DIS; /* Port output mode 0b10 */
	*(volatile u8 *)(PFC_P16) = (*(volatile u8 *)(PFC_P16) & P_P2_MASK) | P_P2;	/* P6_2 output 1 */
	*(volatile u8 *)(PFC_P22) = (*(volatile u8 *)(PFC_P22) & P_P5_MASK) | P_P5;	/* P18_5 output 1 */
#endif

	ret |= sh_sdhi_init(RZF_SD0_BASE,
						0,
						SH_SDHI_QUIRK_64BIT_BUF);
	ret |= sh_sdhi_init(RZF_SD1_BASE,
						1,
						SH_SDHI_QUIRK_64BIT_BUF);

	return ret;
}

int board_init(void)
{
#ifdef CONFIG_V5L2_CACHE
	v5l2_init();
#endif

#if CONFIG_TARGET_SMARC_RZF
#if 0
	struct udevice *dev;
	const u8 pmic_bus = 0;
	const u8 pmic_addr = 0x58;
	u8 data;
	int ret;

	ret = i2c_get_chip_for_busnum(pmic_bus, pmic_addr, 1, &dev);
	if (ret)
		hang();

	ret = dm_i2c_read(dev, 0x2, &data, 1);
	if (ret)
		hang();

	if ((data & 0x08) == 0) {
		printf("SW_ET0_EN: ON\n");
		*(volatile u32 *)(PFC_ETH_ch0) = (*(volatile u32 *)(PFC_ETH_ch0) & 0xFFFFFFFC) | ETH_ch1_1_8;
	} else {
		printf("SW_ET0_EN: OFF\n");
		*(volatile u32 *)(PFC_ETH_ch0) = (*(volatile u32 *)(PFC_ETH_ch0) & 0xFFFFFFFC) | ETH_ch0_3_3;
	}
#endif
#endif
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
		spi_multi_setup();
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
