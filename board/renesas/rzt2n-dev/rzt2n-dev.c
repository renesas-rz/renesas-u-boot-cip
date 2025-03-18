/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022 Renesas Electronics Corporation
 */


#include <common.h>
#include <cpu_func.h>
#include <image.h>
#include <init.h>
#include <malloc.h>
#include <miiphy.h>
#include <netdev.h>
#include <dm.h>
#include <dm/platform_data/serial_sh.h>
#include <asm/processor.h>
#include <asm/mach-types.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/arch/renesas.h>
#include <asm/arch/rcar-mstp.h>
#include <asm/arch/sh_sdhi.h>
#include <i2c.h>
#include <mmc.h>

DECLARE_GLOBAL_DATA_PTR;

#define SCKCR2			0x81280004
#define SCKCR2_CA55SCLK		BIT(12)
#define MSTPCRM			0x80280330
#define MSTPCRM_SDHI0		BIT(12)
#define MSTPCRM_SDHI1		BIT(13)
#define PRCRN			0x80294200
#define PRCRN_PRKEY		(0xa5 << 8)
#define PRCRN_WR_EN		0xF
#define PRCRS                   0x81296000
#define PRCRS_PRKEY             (0xa5 << 8)
#define PRCRS_WR_EN             0xF
#define PMC(x)			((x < 13) ? (0x812C0400 + (x)):(0x802C0400 + (x)))
#define PM(x)			((x < 13) ? (0x812C0200 + (x) * 2):(0x802C0200 + (x) * 2))
#define P(x)			((x < 13) ? (0x812C0000 + (x)):(0x802C0000 + (x)))
#define PFC(x)			((x < 13) ? (0x812C0600 + (x) * 8):(0x802C0600 + (x) * 8))

#define SCKCR			0x80280000
#define PHYSEL			BIT(21)

#define MSTPCRE			0x80280310
#define MSTPCRE_GMAC1		BIT(16)
#define MSTPCRE_ETHSS		BIT(3)

#define ADXCTL0_BASE		0x81290100
#define DDRMIR0_MASK		GENMASK(19, 16)
#define DDRMIR0(x)		(x << 16)
#define ADXC0_MASTERS		6

#define ADXCTL1_BASE		0x81291100
#define DDRMIR1_MASK		GENMASK(27, 24)
#define DDRMIR1(x)		(x << 24)
#define ADXC1_MASTERS		6

#define DDRMIR_MASK		GENMASK(27,  0)
#define addr_shift		0x2

#define MODCTRL			0x8

#define CS0ENDAD_xSPI(x)	(0x80293004 + (0x100 * (x)))

/* Needed by lowlevel_init.S*/
void s_init(void)
{
	/* Disable Write protect to enable writing */
	*(volatile u32 *)PRCRN = PRCRN_PRKEY | PRCRN_WR_EN;
	*(volatile u32 *)PRCRS = PRCRS_PRKEY | PRCRS_WR_EN;

	/* Setting xSPI1 CS0 End Address */
	*(volatile u32 *)CS0ENDAD_xSPI(1) = 0x57FFFFFF;

/* P03_3 enable both SD0_PWEN_A & SD1_PWEN_A Output HIGH */

	*(volatile u8 *)PMC(3) &= ~BIT(3);
	*(volatile u8 *)P(3) |= BIT(3);
	*(volatile u16 *)PM(3) |= (0x3 << 6);

/* P3_2_SD1_IOVS Output LOW */
	*(volatile u8 *)PMC(3) &= ~BIT(2);
	*(volatile u8 *)P(3) &= ~BIT(2);
	*(volatile u16 *)PM(3) |= (0x3 << 4);

	/* Release module stop for SDHI0/1 */
	*(volatile u32 *)MSTPCRM &= ~(MSTPCRM_SDHI0 | MSTPCRM_SDHI1);

	/* Set Coretex-A55 DSU clock to 1GHz */
	*(volatile u32 *)SCKCR2 |= SCKCR2_CA55SCLK;

	/* Enable Write protect to disable writing */
	*(volatile u32 *)PRCRN = PRCRN_PRKEY;
	*(volatile u32 *)PRCRS = PRCRS_PRKEY;
}

/* Needed by common/board_f */

int board_early_init_f(void)
{

	return 0;
}

/* Setting for ADXCTLn registers */
int adxctl_init(void)
{
	int i;
	/*DDR Mirror0 addr_shift=0 Mirror1 addr_shift=2*/
	for (i = 2; i < ADXC0_MASTERS; i++)
		writel(((readl((uintptr_t)(ADXCTL0_BASE + i * 0x4)) & ~DDRMIR_MASK) |
				DDRMIR1(addr_shift)), (uintptr_t)(ADXCTL0_BASE + i * 0x4));

	for (i = 0; i < ADXC1_MASTERS; i++)
		writel(((readl((uintptr_t)(ADXCTL1_BASE + i * 0x4)) & ~DDRMIR_MASK) |
			DDRMIR1(addr_shift)), (uintptr_t)(ADXCTL1_BASE + i * 0x4));
	return 0;
}

/* Needed by common/board_f */
int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_TEXT_BASE + 0x50000;
	
	adxctl_init();

	return 0;
}

void reset_cpu(void)
{

}

/*
 * Reallocate gd to inside DDR area to avoid using it from
 * inaccessible outside DDR mem.
 */
int mach_cpu_init(void)
{
	volatile gd_t *pgd;

	pgd = malloc(sizeof(*pgd));
	if (!pgd) {
		printf("Out of memory for pgd size 0x%lx\n", sizeof(*pgd));
		return -ENOMEM;
	}

	memset((void *)pgd, 0, sizeof(*pgd));
	pgd = gd;
	set_gd(pgd);

	return 0;
}
