/*
 * board/renesas/lager/lager.c
 *     This file is lager board support.
 *
 * Copyright (C) 2013-2014 Renesas Electronics Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <common.h>
#include <malloc.h>
#include <asm/processor.h>
#include <asm/mach-types.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/arch/rmobile.h>
#include <netdev.h>
#include <i2c.h>
#include "lager.h"

DECLARE_GLOBAL_DATA_PTR;

#define PLL0CR		0xE61500D8

#define PLLECR		0xE61500D0
#define PLL0ST		0x100

#define s_init_wait(cnt) \
		({	\
			volatile u32 i = 0x10000 * cnt;	\
			while (i > 0)	\
				i--;	\
		})

void s_init(void)
{
	struct r8a7790_rwdt *rwdt = (struct r8a7790_rwdt *)RWDT_BASE;
	struct r8a7790_swdt *swdt = (struct r8a7790_swdt *)SWDT_BASE;
	u32 val;
	u32 pll0_status;

	/* Watchdog init */
	writel(0xA5A5A500, &rwdt->rwtcsra);
	writel(0xA5A5A500, &swdt->swtcsra);

	/* cpu frequency setting */
	if (rmobile_get_cpu_rev_integer() >= R8A7790_CUT_ES2X) {
		val = readl(PLL0CR);
		val &= ~0x7F000000;
		val |= 0x45000000;			/* 1.4GHz */
		writel(val, PLL0CR);

		do {
			pll0_status = readl(PLLECR) & PLL0ST;
		} while (pll0_status == 0x0);
	}

	/* QoS */
#if !(defined(CONFIG_EXTRAM_BOOT))
	qos_init();
#endif
}

#define	MSTPSR1		0xE6150038
#define	SMSTPCR1	0xE6150134
#define TMU0_MSTP125	(1 << 25)

#define MSTPSR3		0xE6150048
#define SMSTPCR3	0xE615013C
#define MMC0_MSTP315	(1 << 15)
#define SDHI0_MSTP314	(1 << 14)
#define SDHI1_MSTP313	(1 << 13)
#define SDHI2_MSTP312	(1 << 12)
#define SDHI3_MSTP311	(1 << 11)
#define MMC1_MSTP305	(1 << 5)

#define	MSTPSR7		0xE61501C4
#define	SMSTPCR7	0xE615014C
#define SCIF0_MSTP721	(1 << 21)

#define	MSTPSR8		0xE61509A0
#define	SMSTPCR8	0xE6150990
#define ETHER_MSTP813	(1 << 13)

int board_early_init_f(void)
{
	u32 val;

	/* TMU0 */
	val = readl(MSTPSR1);
	val &= ~TMU0_MSTP125;
	writel(val, SMSTPCR1);

	val = readl(MSTPSR7);
	val &= ~SCIF0_MSTP721;
	writel(val, SMSTPCR7);

	/* ETHER */
	val = readl(MSTPSR8);
	val &= ~ETHER_MSTP813;
	writel(val, SMSTPCR8);

	/* eMMC/SD */
	val = readl(MSTPSR3);
	val &= ~(MMC0_MSTP315 | MMC1_MSTP305 | SDHI0_MSTP314 | SDHI2_MSTP312);
	writel(val, SMSTPCR3);

	return 0;
}

DECLARE_GLOBAL_DATA_PTR;
int board_init(void)
{
	u32 val;

	/* board id for linux */
	gd->bd->bi_arch_number = MACH_TYPE_LAGER;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = LAGER_SDRAM_BASE + 0x100;

	/* Init PFC controller */
	r8a7790_pinmux_init();

	/* ETHER Enable */
	gpio_request(GPIO_FN_ETH_CRS_DV, NULL);
	gpio_request(GPIO_FN_ETH_RX_ER, NULL);
	gpio_request(GPIO_FN_ETH_RXD0, NULL);
	gpio_request(GPIO_FN_ETH_RXD1, NULL);
	gpio_request(GPIO_FN_ETH_LINK, NULL);
	gpio_request(GPIO_FN_ETH_REF_CLK, NULL);
	gpio_request(GPIO_FN_ETH_MDIO, NULL);
	gpio_request(GPIO_FN_ETH_TXD1, NULL);
	gpio_request(GPIO_FN_ETH_TX_EN, NULL);
	gpio_request(GPIO_FN_ETH_MAGIC, NULL);
	gpio_request(GPIO_FN_ETH_TXD0, NULL);
	gpio_request(GPIO_FN_ETH_MDC, NULL);
	gpio_request(GPIO_FN_IRQ0, NULL);

	sh_timer_init();

	gpio_request(GPIO_GP_5_31, NULL);	/* PHY_RST */
	gpio_direction_output(GPIO_GP_5_31, 0);
	mdelay(20);
	gpio_set_value(GPIO_GP_5_31, 1);
	udelay(1);

#ifdef CONFIG_SH_MMCIF
	gpio_request(GPIO_FN_MMC1_D0, NULL);
	gpio_request(GPIO_FN_MMC1_D1, NULL);
	gpio_request(GPIO_FN_MMC1_D2, NULL);
	gpio_request(GPIO_FN_MMC1_D3, NULL);
	gpio_request(GPIO_FN_MMC1_D4, NULL);
	gpio_request(GPIO_FN_MMC1_D5, NULL);
	gpio_request(GPIO_FN_MMC1_D6, NULL);
	gpio_request(GPIO_FN_MMC1_D7, NULL);
	gpio_request(GPIO_FN_MMC1_CLK, NULL);
	gpio_request(GPIO_FN_MMC1_CMD, NULL);
#endif

#ifdef CONFIG_SH_SDHI
	gpio_request(GPIO_FN_SD0_DAT0, NULL);
	gpio_request(GPIO_FN_SD0_DAT1, NULL);
	gpio_request(GPIO_FN_SD0_DAT2, NULL);
	gpio_request(GPIO_FN_SD0_DAT3, NULL);
	gpio_request(GPIO_FN_SD0_CLK, NULL);
	gpio_request(GPIO_FN_SD0_CMD, NULL);
	gpio_request(GPIO_FN_SD0_CD, NULL);
	gpio_request(GPIO_FN_SD2_DAT0, NULL);
	gpio_request(GPIO_FN_SD2_DAT1, NULL);
	gpio_request(GPIO_FN_SD2_DAT2, NULL);
	gpio_request(GPIO_FN_SD2_DAT3, NULL);
	gpio_request(GPIO_FN_SD2_CLK, NULL);
	gpio_request(GPIO_FN_SD2_CMD, NULL);
	gpio_request(GPIO_FN_SD2_CD, NULL);
#endif

	/* need JP3 set to pin-1 side on board. */
	/* sdhi0 */
	gpio_request(GPIO_GP_5_24, NULL);
	gpio_request(GPIO_GP_5_29, NULL);
	gpio_direction_output(GPIO_GP_5_24, 1);	/* power on */
	gpio_direction_output(GPIO_GP_5_29, 1);	/* 1: 3.3V, 0: 1.8V */
	/* sdhi2 */
	gpio_request(GPIO_GP_5_25, NULL);
	gpio_request(GPIO_GP_5_30, NULL);
	gpio_direction_output(GPIO_GP_5_25, 1);	/* power on */
	gpio_direction_output(GPIO_GP_5_30, 1);	/* 1: 3.3V, 0: 1.8V */

	do {
		val = readl(0xE6600B0C) & 0xF;
	} while (val != 0x2);
	writel(0x2, 0xE6600B80);
	do {
		val = readl(0xE6600A14) & 0x1;
	} while (val != 0x0);
	writel(0x0, 0xE660012C);

	do {
		val = readl(0xE6620B0C) & 0xF;
	} while (val != 0x2);
	writel(0x2, 0xE6620B80);
	do {
		val = readl(0xE6620A14) & 0x1;
	} while (val != 0x0);
	writel(0x0, 0xE662012C);

	/* wait 5ms */
	udelay(5000);

	return 0;
}

int board_eth_init(bd_t *bis)
{
	int ret = -ENODEV;
	u32 val;
	unsigned char enetaddr[6];

#ifdef CONFIG_SH_ETHER
	ret = sh_eth_initialize(bis);
	if (!eth_getenv_enetaddr("ethaddr", enetaddr))
		return ret;

	/* Set Mac address */
	val = enetaddr[0] << 24 | enetaddr[1] << 16 |
	    enetaddr[2] << 8 | enetaddr[3];
	writel(val, 0xEE7003C0);

	val = enetaddr[4] << 8 | enetaddr[5];
	writel(val, 0xEE7003C8);
#endif

	return ret;
}

int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	return 0;
}

const struct rmobile_sysinfo sysinfo = {
	CONFIG_RMOBILE_BOARD_STRING
};

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = LAGER_SDRAM_BASE;
	gd->bd->bi_dram[0].size = LAGER_SDRAM_SIZE;
}

int board_late_init(void)
{
	return 0;
}

int board_mmc_init(bd_t *bis)
{
#ifdef CONFIG_SH_SDHI
	int ret;

	ret = mmcif_mmc_init();
	if (ret)
		return ret;

	/* use SDHI0/SDHI2 */
	ret = sdhi_mmc_init(SDHI0_BASE, 0);
	if (ret)
		return ret;

	return sdhi_mmc_init(SDHI2_BASE, 2);
#else
	return mmcif_mmc_init();
#endif
}

void reset_cpu(ulong addr)
{
	u8 val;

	i2c_init(0, 0);
	i2c_read(0x58, 0x13, 1, &val, 1);
	val |= 0x02;
	i2c_write(0x58, 0x13, 1, &val, 1);
}

#define TSTR0	4
#define TSTR0_STR0	0x1

void arch_preboot_os()
{
	u32 val;

	/* stop TMU0 */
	val = readb(TMU_BASE + TSTR0);
	val &= ~TSTR0_STR0;
	writeb(val, TMU_BASE + TSTR0);

	val = readl(MSTPSR1);
	val |= TMU0_MSTP125;
	writel(val, SMSTPCR1);
}
