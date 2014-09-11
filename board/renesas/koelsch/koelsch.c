/*
 * board/renesas/koelsch/koelsch.c
 *     This file is koelsch board support.
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
#include "koelsch.h"

DECLARE_GLOBAL_DATA_PTR;

#define s_init_wait(cnt) \
		({	\
			volatile u32 i = 0x10000 * cnt;	\
			while (i > 0)	\
				i--;	\
		})

#define PLL0CR		0xE61500D8

void s_init(void)
{
	struct r8a7791_rwdt *rwdt = (struct r8a7791_rwdt *)RWDT_BASE;
	struct r8a7791_swdt *swdt = (struct r8a7791_swdt *)SWDT_BASE;
	u32 val;

	/* Watchdog init */
	writel(0xA5A5A500, &rwdt->rwtcsra);
	writel(0xA5A5A500, &swdt->swtcsra);

	/* cpu frequency setting */
	val = readl(PLL0CR);
	val &= ~0x7F000000;
	val |= 0x4A000000;			/* 1.5GHz */
	writel(val, PLL0CR);

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
#define SDHI1_MSTP312	(1 << 12)
#define SDHI2_MSTP311	(1 << 11)

#define	MSTPSR7		0xE61501C4
#define	SMSTPCR7	0xE615014C
#define SCIF0_MSTP721	(1 << 21)

#define	MSTPSR8		0xE61509A0
#define	SMSTPCR8	0xE6150990
#define ETHER_MSTP813	(1 << 13)

#define SD1CKCR		0xE6150078
#define SD1_97500KHZ	0x7

#define SD2CKCR		0xE615026C
#define SD2_97500KHZ	0x7

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

	/* SD */
	val = readl(MSTPSR3);
	val &= ~(SDHI0_MSTP314 | SDHI1_MSTP312 | SDHI2_MSTP311);
	writel(val, SMSTPCR3);

	/*
	 * SD0 clock is set to 97.5MHz by default.
	 * Set SD1 and SD2 to the 97.5MHz as well.
	 */
	writel(SD1_97500KHZ, SD1CKCR);
	writel(SD2_97500KHZ, SD2CKCR);

	return 0;
}

DECLARE_GLOBAL_DATA_PTR;
int board_init(void)
{
	u32 val;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = KOELSCH_SDRAM_BASE + 0x100;

	/* Init PFC controller */
	r8a7791_pinmux_init();

	/* ETHER Enable */
	gpio_request(GPIO_FN_ETH_CRS_DV, NULL);
	gpio_request(GPIO_FN_ETH_RX_ER, NULL);
	gpio_request(GPIO_FN_ETH_RXD0, NULL);
	gpio_request(GPIO_FN_ETH_RXD1, NULL);
	gpio_request(GPIO_FN_ETH_LINK, NULL);
	gpio_request(GPIO_FN_ETH_REFCLK, NULL);
	gpio_request(GPIO_FN_ETH_MDIO, NULL);
	gpio_request(GPIO_FN_ETH_TXD1, NULL);
	gpio_request(GPIO_FN_ETH_TX_EN, NULL);
	gpio_request(GPIO_FN_ETH_TXD0, NULL);
	gpio_request(GPIO_FN_ETH_MDC, NULL);
	gpio_request(GPIO_FN_IRQ0, NULL);

#ifdef CONFIG_SH_SDHI
	gpio_request(GPIO_FN_SD0_DATA0, NULL);
	gpio_request(GPIO_FN_SD0_DATA1, NULL);
	gpio_request(GPIO_FN_SD0_DATA2, NULL);
	gpio_request(GPIO_FN_SD0_DATA3, NULL);
	gpio_request(GPIO_FN_SD0_CLK, NULL);
	gpio_request(GPIO_FN_SD0_CMD, NULL);
	gpio_request(GPIO_FN_SD0_CD, NULL);
	gpio_request(GPIO_FN_SD2_DATA0, NULL);
	gpio_request(GPIO_FN_SD2_DATA1, NULL);
	gpio_request(GPIO_FN_SD2_DATA2, NULL);
	gpio_request(GPIO_FN_SD2_DATA3, NULL);
	gpio_request(GPIO_FN_SD2_CLK, NULL);
	gpio_request(GPIO_FN_SD2_CMD, NULL);
	gpio_request(GPIO_FN_SD2_CD, NULL);
#endif

	val = readl(0xe6060114);
	val &= ~0x37FC0000;
	writel(val, 0xe6060114);

	sh_timer_init();

	gpio_request(GPIO_GP_5_22, NULL);	/* PHY_RST */
	val = readl(0xe6060114);
	val &= ~0x08000000;
	writel(val, 0xe6060114);

	gpio_direction_output(GPIO_GP_5_22, 0);
	mdelay(20);
	gpio_set_value(GPIO_GP_5_22, 1);
	udelay(1);

	/* sdhi0 */
	gpio_request(GPIO_GP_7_17, NULL);
	gpio_request(GPIO_GP_2_12, NULL);
	gpio_direction_output(GPIO_GP_7_17, 1);	/* power on */
	gpio_direction_output(GPIO_GP_2_12, 1);	/* 1: 3.3V, 0: 1.8V */
	/* sdhi1 */
	gpio_request(GPIO_GP_7_18, NULL);
	gpio_request(GPIO_GP_2_13, NULL);
	gpio_direction_output(GPIO_GP_7_18, 1);	/* power on */
	gpio_direction_output(GPIO_GP_2_13, 1);	/* 1: 3.3V, 0: 1.8V */
	/* sdhi2 */
	gpio_request(GPIO_GP_7_19, NULL);
	gpio_request(GPIO_GP_2_26, NULL);
	gpio_direction_output(GPIO_GP_7_19, 1);	/* power on */
	gpio_direction_output(GPIO_GP_2_26, 1);	/* 1: 3.3V, 0: 1.8V */

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
	gd->bd->bi_dram[0].start = KOELSCH_SDRAM_BASE;
	gd->bd->bi_dram[0].size = KOELSCH_SDRAM_SIZE;
}

int board_late_init(void)
{
	return 0;
}

#ifdef CONFIG_SH_SDHI
int board_mmc_init(bd_t *bis)
{
	int ret;

	/* use SDHI0-2 */
	ret = sdhi_mmc_init(SDHI0_BASE, 0);
	if (ret)
		return ret;

	ret = sdhi_mmc_init(SDHI1_BASE, 1);
	if (ret)
		return ret;

	return sdhi_mmc_init(SDHI2_BASE, 2);
}
#endif

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
