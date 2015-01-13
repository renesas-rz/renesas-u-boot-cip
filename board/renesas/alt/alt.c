/*
 * board/renesas/alt/alt.c
 *     This file is alt board support.
 *
 * Copyright (C) 2014-2015 Renesas Electronics Corporation
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
#include "alt.h"

DECLARE_GLOBAL_DATA_PTR;

#define PLL0CR		0xE61500D8

#define s_init_wait(cnt) \
		({	\
			volatile u32 i = 0x10000 * cnt;	\
			while (i > 0)	\
				i--;	\
		})

void s_init(void)
{
	struct r8a7794_rwdt *rwdt = (struct r8a7794_rwdt *)RWDT_BASE;
	struct r8a7794_swdt *swdt = (struct r8a7794_swdt *)SWDT_BASE;

	/* Watchdog init */
	writel(0xA5A5A500, &rwdt->rwtcsra);
	writel(0xA5A5A500, &swdt->swtcsra);

	/* QoS */
#if !(defined(CONFIG_EXTRAM_BOOT))
	qos_init();
#endif
}

#define TMU0_MSTP125	(1 << 25)

#define IIC1_MSTP323	(1 << 23)
#define MMC0_MSTP315	(1 << 15)
#define SDHI0_MSTP314	(1 << 14)
#define SDHI1_MSTP312	(1 << 12)

#define SCIF2_MSTP719	(1 << 19)

#define ETHER_MSTP813	(1 << 13)

#define SD1CKCR		0xE6150078
#define SD1_97500KHZ	0x7

int board_early_init_f(void)
{
	u32 val;

	/* TMU0 */
	val = readl(MSTPSR1);
	val &= ~TMU0_MSTP125;
	writel(val, SMSTPCR1);

	/* IIC1 */
	val = readl(MSTPSR3);
	val &= ~IIC1_MSTP323;
	writel(val, SMSTPCR3);

	/* SCIF2 */
	val = readl(MSTPSR7);
	val &= ~SCIF2_MSTP719;
	writel(val, SMSTPCR7);

	/* ETHER */
	val = readl(MSTPSR8);
	val &= ~ETHER_MSTP813;
	writel(val, SMSTPCR8);

	/* MMC/SD */
	val = readl(MSTPSR3);
	val &= ~(MMC0_MSTP315 | SDHI0_MSTP314 | SDHI1_MSTP312);
	writel(val, SMSTPCR3);

	/*
	 * SD0 clock is set to 97.5MHz by default.
	 * Set SD1 to the 97.5MHz as well.
	 */
	writel(SD1_97500KHZ, SD1CKCR);

	return 0;
}

DECLARE_GLOBAL_DATA_PTR;
int board_init(void)
{
	u32 val;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = ALT_SDRAM_BASE + 0x100;

	/* Init PFC controller */
	r8a7794_pinmux_init();

	/* ETHER Enable */
#if !defined(CONFIG_ALT_ETHERB)
	gpio_request(GPIO_FN_ETH_CRS_DV, NULL);
	gpio_request(GPIO_FN_ETH_RX_ER, NULL);
	gpio_request(GPIO_FN_ETH_RXD0, NULL);
	gpio_request(GPIO_FN_ETH_RXD1, NULL);
	gpio_request(GPIO_FN_ETH_LINK, NULL);
	gpio_request(GPIO_FN_ETH_REFCLK, NULL);
	gpio_request(GPIO_FN_ETH_MDIO, NULL);
	gpio_request(GPIO_FN_ETH_TXD1, NULL);
	gpio_request(GPIO_FN_ETH_TX_EN, NULL);
	gpio_request(GPIO_FN_ETH_MAGIC, NULL);
	gpio_request(GPIO_FN_ETH_TXD0, NULL);
	gpio_request(GPIO_FN_ETH_MDC, NULL);
#else
	gpio_request(GPIO_FN_ETH_CRS_DV_B, NULL);
	gpio_request(GPIO_FN_ETH_RX_ER_B, NULL);
	gpio_request(GPIO_FN_ETH_RXD0_B, NULL);
	gpio_request(GPIO_FN_ETH_RXD1_B, NULL);
	gpio_request(GPIO_FN_ETH_LINK_B, NULL);
	gpio_request(GPIO_FN_ETH_REFCLK_B, NULL);
	gpio_request(GPIO_FN_ETH_MDIO_B, NULL);
	gpio_request(GPIO_FN_ETH_TXD1_B, NULL);
	gpio_request(GPIO_FN_ETH_TX_EN_B, NULL);
	gpio_request(GPIO_FN_ETH_MAGIC_B, NULL);
	gpio_request(GPIO_FN_ETH_TXD0_B, NULL);
	gpio_request(GPIO_FN_ETH_MDC_B, NULL);
#endif
	gpio_request(GPIO_FN_IRQ8, NULL);

#ifdef CONFIG_SH_SDHI
	gpio_request(GPIO_FN_SD0_DATA0, NULL);
	gpio_request(GPIO_FN_SD0_DATA1, NULL);
	gpio_request(GPIO_FN_SD0_DATA2, NULL);
	gpio_request(GPIO_FN_SD0_DATA3, NULL);
	gpio_request(GPIO_FN_SD0_CLK, NULL);
	gpio_request(GPIO_FN_SD0_CMD, NULL);
	gpio_request(GPIO_FN_SD0_CD, NULL);
	gpio_request(GPIO_FN_SD1_DATA0, NULL);
	gpio_request(GPIO_FN_SD1_DATA1, NULL);
	gpio_request(GPIO_FN_SD1_DATA2, NULL);
	gpio_request(GPIO_FN_SD1_DATA3, NULL);
	gpio_request(GPIO_FN_SD1_CLK, NULL);
	gpio_request(GPIO_FN_SD1_CMD, NULL);
	gpio_request(GPIO_FN_SD1_CD, NULL);
#endif

	sh_timer_init();

	gpio_request(GPIO_GP_1_24, NULL);	/* PHY_RST */
	gpio_direction_output(GPIO_GP_1_24, 0);
	mdelay(20);
	gpio_set_value(GPIO_GP_1_24, 1);
	udelay(1);

	/* mmc0 */
	gpio_request(GPIO_GP_4_31, NULL);
	gpio_set_value(GPIO_GP_4_31, 1);
	/* sdhi0 */
	gpio_request(GPIO_GP_2_26, NULL);
	gpio_request(GPIO_GP_2_29, NULL);
	gpio_direction_output(GPIO_GP_2_26, 1);
	gpio_direction_output(GPIO_GP_2_29, 1);
	/* sdhi1 */
	gpio_request(GPIO_GP_4_26, NULL);
	gpio_request(GPIO_GP_4_29, NULL);
	gpio_direction_output(GPIO_GP_4_26, 1);
	gpio_direction_output(GPIO_GP_4_29, 1);

	do {
		val = readl(0xE6600B0C) & 0xF;
	} while (val != 0x2);
	writel(0x2, 0xE6600B80);
	do {
		val = readl(0xE6600A14) & 0x1;
	} while (val != 0x0);
	writel(0x0, 0xE660012C);

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
	gd->bd->bi_dram[0].start = ALT_SDRAM_BASE;
	gd->bd->bi_dram[0].size = ALT_SDRAM_SIZE;
}

int board_late_init(void)
{
	return 0;
}

int board_mmc_init(bd_t *bis)
{
	int ret = 0;

#ifdef CONFIG_SH_MMCIF
	ret = mmcif_mmc_init();
#endif

#ifdef CONFIG_SH_SDHI
	/* use SDHI0,1 */
	ret = sdhi_mmc_init(SDHI0_BASE, 0);
	if (ret)
		return ret;

	ret = sdhi_mmc_init(SDHI1_BASE, 1);
#endif

	return ret;
}

void reset_cpu(ulong addr)
{
	u8 val;

	i2c_init(CONFIG_SYS_I2C_SPEED, 0);
	i2c_read(0x58, 0x13, 1, &val, 1);
	val |= 0x02;
	i2c_write(0x58, 0x13, 1, &val, 1);
}

#define TSTR0	4
#define TSTR0_STR0	0x1

enum {
	MSTP00, MSTP01, MSTP02, MSTP03, MSTP04, MSTP05,
	MSTP07, MSTP08, MSTP09, MSTP10, MSTP11,
	MSTP_NR,
};

struct mstp_ctl {
	u32 s_addr;
	u32 s_dis;
	u32 s_ena;
	u32 r_addr;
	u32 r_dis;
	u32 r_ena;
} mstptbl[MSTP_NR] = {
	[MSTP00] = { SMSTPCR0,  0x00440801, 0x00400000,
		     RMSTPCR0,  0x00440801, 0x00000000 },
	[MSTP01] = { SMSTPCR1,  0x936899DA, 0x00000000,
		     RMSTPCR1,  0x936899DA, 0x00000000 },
	[MSTP02] = { SMSTPCR2,  0x100D21FC, 0x00002000,
		     RMSTPCR2,  0x100D21FC, 0x00000000 },
	[MSTP03] = { SMSTPCR3,  0xE084D810, 0x00000000,
		     RMSTPCR3,  0xE084D810, 0x00000000 },
	[MSTP04] = { SMSTPCR4,  0x800001C4, 0x00000180,
		     RMSTPCR4,  0x800001C4, 0x00000000 },
	[MSTP05] = { SMSTPCR5,  0x40C00044, 0x00000000,
		     RMSTPCR5,  0x40C00044, 0x00000000 },
	[MSTP07] = { SMSTPCR7,  0x013FE618, 0x00080000,
		     RMSTPCR7,  0x013FE618, 0x00000000 },
	[MSTP08] = { SMSTPCR8,  0x40803C05, 0x00000000,
		     RMSTPCR8,  0x40803C05, 0x00000000 },
	[MSTP09] = { SMSTPCR9,  0xFB879FEE, 0x00000000,
		     RMSTPCR9,  0xFB879FEE, 0x00000000 },
	[MSTP10] = { SMSTPCR10, 0xFFFEFFE0, 0x00000000,
		     RMSTPCR10, 0xFFFEFFE0, 0x00000000 },
	[MSTP11] = { SMSTPCR11, 0x000001C0, 0x00000000,
		     RMSTPCR11, 0x000001C0, 0x00000000 },
};

void arch_preboot_os()
{
	u32 val;
	int i;

	/* stop TMU0 */
	val = readb(TMU_BASE + TSTR0);
	val &= ~TSTR0_STR0;
	writeb(val, TMU_BASE + TSTR0);

	/* stop all module clock*/
	for (i = MSTP00; i < MSTP_NR; i++) {
		val = readl(mstptbl[i].s_addr);
		writel((val | mstptbl[i].s_dis) & ~(mstptbl[i].s_ena),
		       mstptbl[i].s_addr);
		val = readl(mstptbl[i].r_addr);
		writel((val | mstptbl[i].r_dis) & ~(mstptbl[i].r_ena),
		       mstptbl[i].r_addr);
	}
}
