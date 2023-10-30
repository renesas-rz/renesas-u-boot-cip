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
#include <asm/sbi.h>
#include <mmc.h>
#include <i2c.h>
#include <hang.h>
#include <renesas/rzf-dev/mmio.h>
#include <renesas/rzf-dev/rzf-dev_def.h>
#include <renesas/rzf-dev/rzf-dev_sys.h>
#include <renesas/rzf-dev/rzf-dev_sys_regs.h>
#include <renesas/rzf-dev/rzf-dev_pfc_regs.h>
#include <renesas/rzf-dev/rzf-dev_cpg_regs.h>
#include "rzf-dev_spi_multi.h"
#include "tzc400.h"
#include "tzc_common.h"
#include "plat_tzc_def.h"

extern void cpg_setup(void);
extern void pfc_setup(void);
extern void ddr_setup(void);
extern int spi_multi_setup(uint32_t addr_width, uint32_t dq_width, uint32_t dummy_cycle);

void cpu_cpg_setup(void);

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
}

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	return 0;
}
#endif

static int sd_power_setup(void)
{
	int ret = 0;

	mmio_write_32(PFC_PWPR, 0x0);
	mmio_write_32(PFC_PWPR, PWPR_PFCWE);
#if CONFIG_TARGET_SMARC_RZF
	/* SD1 power control : P0_3 = 1 P6_1 = 1        */
	*(volatile u8 *)(PFC_PMC10) &= 0xF7;    /* Port func mode 0b00  */
	*(volatile u8 *)(PFC_PMC16) &= 0xFD;    /* Port func mode 0b00  */
	*(volatile u16 *)(PFC_PM10) = (*(volatile u16 *)(PFC_PM10) & 0xFF3F) | 0x0080; /* Port output mode 0b10 */
	*(volatile u16 *)(PFC_PM16) = (*(volatile u16 *)(PFC_PM16) & 0xFFF3) | 0x0008; /* Port output mode 0b10 */
	*(volatile u8 *)(PFC_P10) = (*(volatile u8 *)(PFC_P10) & 0xF7) | 0x08; /* P0_3  output 1        */
	*(volatile u8 *)(PFC_P16) = (*(volatile u8 *)(PFC_P16) & 0xFD) | 0x02; /* P6_1  output 1        */
#else
	/* SD0 power control: P5_4=1,P18_4 = 1; */
	*(volatile u8 *)(PFC_PMC15) &= 0xEF;
	*(volatile u8 *)(PFC_PMC22) &= 0xEF; /* Port func mode 0b0 */
	*(volatile u16 *)(PFC_PM15) = (*(volatile u16 *)(PFC_PM15) & 0xFCFF) | 0x200;
	*(volatile u16 *)(PFC_PM22) = (*(volatile u16 *)(PFC_PM22) & 0xFCFF) | 0x200; /* Port output mode 0b10 */
	*(volatile u8 *)(PFC_P15) = (*(volatile u8 *)(PFC_P15) & 0xEF) | 0x10;
	*(volatile u8 *)(PFC_P22) = (*(volatile u8 *)(PFC_P22) & 0xEF) | 0x10;  /* Port 18[2:1] output value 0b1*/

	/* SD1 power control: P6_2=1,P18_5 = 1; */
	*(volatile u8 *)(PFC_PMC16) &= 0xFB; /* Port func mode 0b0 */
	*(volatile u8 *)(PFC_PMC22) &= 0xDF; /* Port func mode 0b0 */
	*(volatile u16 *)(PFC_PM16) = (*(volatile u16 *)(PFC_PM16) & 0xFFCF) | 0x20; /* Port output mode 0b10 */
	*(volatile u16 *)(PFC_PM22) = (*(volatile u16 *)(PFC_PM22) & 0xF3FF) | 0x800; /* Port output mode 0b10 */
	*(volatile u8 *)(PFC_P16) = (*(volatile u8 *)(PFC_P16) & 0xFB) | 0x04;  /* Port 6[2:1] output value 0b1*/
	*(volatile u8 *)(PFC_P22) = (*(volatile u8 *)(PFC_P22) & 0xDF) | 0x20;  /* Port 18[2:1] output value 0b1*/
#endif
	mmio_write_32(PFC_PWPR, 0x0);
	mmio_write_32(PFC_PWPR, PWPR_B0Wl);

	return 0;
}

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
#ifdef CONFIG_V5L2_CACHE
	v5l2_init();
#endif
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
	/* SD CLK */
	*(volatile u32 *)(CPG_PL2SDHI_DSEL) = 0x00110011;
	while (*(volatile u32 *)(CPG_CLKSTATUS) != 0)
		;

	sd_power_setup();

	return 0;
}
#endif

int board_init(void)
{
#if CONFIG_TARGET_SMARC_RZF
	struct sbiret sret;
	struct udevice *dev;
	struct udevice *bus;
	const u8 pmic_bus = 0;
	const u8 pmic_addr = 0x58;
	u8 data;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_I2C, pmic_bus, &bus);
	if (ret)
		hang();

	ret = i2c_get_chip(bus, pmic_addr, 1, &dev);
	if (ret)
		hang();

	ret = dm_i2c_read(dev, 0x2, &data, 1);
	if (ret)
		hang();

	if ((data & 0x08) == 0) {
		printf("SW_ET0_EN: ON\n");
		*(volatile u32 *)(PFC_ETH_ch0) = (*(volatile u32 *)(PFC_ETH_ch0) & 0xFFFFFFFC) | ETH_ch0_1_8;
	} else {
		printf("SW_ET0_EN: OFF\n");
		*(volatile u32 *)(PFC_ETH_ch0) = (*(volatile u32 *)(PFC_ETH_ch0) & 0xFFFFFFFC) | ETH_ch0_3_3;
	}
	udelay(10);
	sret = sbi_ecall(0x900031e, 2, 0, 0, 0, 0, 0, 0);
#endif
	return 0;
}


int dram_init(void)
{
#ifdef CONFIG_DEBUG_RZF_FPGA
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;
	return 0;
#endif
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

typedef struct arm_tzc_regions_info {
	unsigned long long base;
	unsigned long long end;
	unsigned int sec_attr;
	unsigned int nsaid_permissions;
} arm_tzc_regions_info_t;

static const struct {
	uint32_t reg;
	uint32_t msk;
	uint32_t val;
} sys_acctl[] = {
	/* Master Access Control Register */
	{SYS_MSTACCCTL0,  0x000000BBU, 0x0000AA88U},
	{SYS_MSTACCCTL1,  0xBBBBBBBBU, 0xAAAAAAAAU},
	{SYS_MSTACCCTL2,  0x00BBBBBBU, 0x00AAAAAAU},
	/* Slave Access Control Register */
	{SYS_SLVACCCTL0,  0x0000000FU, 0x00000008U},
	{SYS_SLVACCCTL1,  0x3CFFFCFFU, 0x0U},
	{SYS_SLVACCCTL2,  0xC3C0FF, 0x0U},
	{SYS_SLVACCCTL3,  0x3FFF0000U, 0x0U},
	{SYS_SLVACCCTL4,  0xFFFF0FFFU, 0x00000000U},
	{SYS_SLVACCCTL5,  0x00000033U, 0x00000000U},
	{SYS_SLVACCCTL6,  0x0000000FU, 0x00000000U},
	{SYS_SLVACCCTL7,  0x0000000CU, 0x0U},
	{SYS_SLVACCCTL8,  0x00000030U, 0x00000000U},
	{SYS_SLVACCCTL10, 0x00000003U, 0x00000000U},
	{SYS_SLVACCCTL12, 0x00000003U, 0x00000000U},
	{SYS_SLVACCCTL13, 0x00000003U, 0x00000000U},
	{SYS_SLVACCCTL14, 0x00000003U, 0x00000000U},
};

static void plat_access_control_setup(void)
{
	uint32_t i;

	for (i = 0; i < ARRAY_SIZE(sys_acctl); i++)
	{
		uint32_t val = mmio_read_32(sys_acctl[i].reg) & (~sys_acctl[i].msk);
		val |= (sys_acctl[i].val & sys_acctl[i].msk);
		mmio_write_32(sys_acctl[i].reg, val);
	}
}

static uint8_t tzc400_get_num_filters(uintptr_t tzc_base)
{
	uint32_t tzc400_build;

	tzc400_build = mmio_read_32(tzc_base + BUILD_CONFIG_OFF);

	printf("%s CFG: %x\n", __func__, tzc400_build);
	return (uint8_t)((tzc400_build >> BUILD_CONFIG_NF_SHIFT) & BUILD_CONFIG_NF_MASK) + 1U;
}

static void plat_tzc400_setup(uintptr_t tzc_base, const arm_tzc_regions_info_t *tzc_regions)
{
	uint8_t num_filters;
	unsigned int region_index = 1U;
	const arm_tzc_regions_info_t *p;
	const arm_tzc_regions_info_t init_tzc_regions[] = {
		{0}
	};

	tzc400_init(tzc_base);

	tzc400_disable_filters();

	tzc400_configure_region0(TZC_REGION_S_RDWR, PLAT_TZC_REGION_ACCESS_NS_UNPRIV);

	if (tzc_regions == NULL)
		p = init_tzc_regions;
	else
		p = tzc_regions;

	num_filters = tzc400_get_num_filters(tzc_base);

	for (; p->base != 0UL; p++) {
		tzc400_configure_region((1 << num_filters) - 1, region_index,
			p->base, p->end, p->sec_attr, p->nsaid_permissions);
		region_index++;
	}

	tzc400_set_action(TZC_ACTION_ERR);

	tzc400_enable_filters();
}

static void bl2_security_setup(void)
{
// 	const arm_tzc_regions_info_t ddr_tzc_regions[] = {
// 		{}
// 	};
// 
// 	/* initialize TZC-400 */
// 	plat_tzc400_setup(RZF_TZC_DDR_BASE, &ddr_tzc_regions[0]);

	/* setup Master/Slave Access Control */
	plat_access_control_setup();
}

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
#if CONFIG_TARGET_SMARC_RZF
		spi_multi_setup(SPI_MULTI_ADDR_WIDES_24, SPI_MULTI_DQ_WIDES_1_1_1, SPI_MULTI_DUMMY_8CYCLE);
#else
		spi_multi_setup(SPI_MULTI_ADDR_WIDES_24, SPI_MULTI_DQ_WIDES_1_4_4, SPI_MULTI_DUMMY_10CYCLE);
#endif
	}

	return 0;
}

void board_init_f(ulong dummy)
{
	int ret;

	cpu_cpg_setup();

	bl2_security_setup();

	/* Initialize SPL*/
	ret = spl_early_init();
	if (ret)
		panic("spl_early_init() failed: %d\n", ret);

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
