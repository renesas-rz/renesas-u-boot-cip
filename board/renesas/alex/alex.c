/*
 * board/renesas/alex/alex.c
 *     This file is ale6/ale4 board support.
 *
 * Copyright (C) 2015 Renesas Electronics Corporation
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
#include "alex.h"

DECLARE_GLOBAL_DATA_PTR;

#define PLLECR		0xE61500D0
#define PLL0CR		0xE61500D8
#define PLL0ST		0x100

#define s_init_wait(cnt) \
		({	\
			volatile u32 i = 0x10000 * cnt;	\
			while (i > 0)	\
				i--;	\
		})

void s_init(void)
{
	struct r8a7794x_rwdt *rwdt = (struct r8a7794x_rwdt *)RWDT_BASE;
	struct r8a7794x_swdt *swdt = (struct r8a7794x_swdt *)SWDT_BASE;
	u32 val;
	u32 pll0_status;

	/* Watchdog init */
	writel(0xA5A5A500, &rwdt->rwtcsra);
	writel(0xA5A5A500, &swdt->swtcsra);

	/* cpu frequency setting */
	val = readl(PLL0CR);
	val &= ~0x7F000000;
	val |= 0x31000000;	/* EXTAL 20MHz * (49+1) = 1GHz */
	writel(val, PLL0CR);
	do {
		pll0_status = readl(PLLECR) & PLL0ST;
	} while (pll0_status == 0x0);
}

#define TMU1_MSTP111    (1 << 11)

#define I2C1_MSTP930	(1 << 30)
#define SDHI0_MSTP314	(1 << 14)
#define SDHI1_MSTP313	(1 << 13)
#define SDHI2_MSTP312	(1 << 12)

#define SCIF0_MSTP721	(1 << 21)

#define ETHER_MSTP813	(1 << 13)
#define AVB_MSTP812	(1 << 12)

#define SDCKCR		0xE6150074
#define SDH_780MHZ	(0x0 << 8)
#define SD0_97500KHZ	(0x6 << 4)
#define SD1_156000KHZ	0xC
#define SD2CKCR		0xE6150078
#define SD2_97500KHZ	0x7

int board_early_init_f(void)
{
	u32 val;

	/* TMU1 */
	val = readl(MSTPSR1);
	val &= ~TMU1_MSTP111;
	writel(val, SMSTPCR1);

	/* I2C1 */
	val = readl(MSTPSR9);
	val &= ~I2C1_MSTP930;
	writel(val, SMSTPCR9);

	/* SCIF0 */
	val = readl(MSTPSR7);
	val &= ~SCIF0_MSTP721;
	writel(val, SMSTPCR7);

	/* ETHER/AVB */
	val = readl(MSTPSR8);
	val &= ~ETHER_MSTP813;
#ifdef CONFIG_RAVB
	val &= ~AVB_MSTP812;
#endif
	writel(val, SMSTPCR8);

	/* MMC/SD */
	val = readl(MSTPSR3);
	val &= ~(SDHI0_MSTP314 | SDHI1_MSTP313 | SDHI2_MSTP312);
	writel(val, SMSTPCR3);

	/*
	 * SD0 clock is set to 97.5MHz by default.
	 * Set SD1 to the 156MHz as well.
	 * Set SD2 to the 97.5MHz as well.
	 */
	writel(SDH_780MHZ | SD0_97500KHZ | SD1_156000KHZ,
		SDCKCR);
	writel(SD2_97500KHZ, SD2CKCR);

	return 0;
}

DECLARE_GLOBAL_DATA_PTR;
int board_init(void)
{
	u32 val;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = ALEX_SDRAM_BASE + 0x100;

	/* Init PFC controller */
	r8a7794x_pinmux_init();

#ifdef CONFIG_RAVB
	/* EtherAVB Enable */
	gpio_request(GPIO_FN_AVB_TXD0, NULL);
	gpio_request(GPIO_FN_AVB_TXD1, NULL);
	gpio_request(GPIO_FN_AVB_TXD2, NULL);
	gpio_request(GPIO_FN_AVB_TXD3, NULL);
	gpio_request(GPIO_FN_AVB_TXD4, NULL);
	gpio_request(GPIO_FN_AVB_TXD5, NULL);
	gpio_request(GPIO_FN_AVB_TXD6, NULL);
	gpio_request(GPIO_FN_AVB_TXD7, NULL);
	gpio_request(GPIO_FN_AVB_RXD1, NULL);
	gpio_request(GPIO_FN_AVB_RXD2, NULL);
	gpio_request(GPIO_FN_AVB_RXD3, NULL);
	gpio_request(GPIO_FN_AVB_RXD4, NULL);
	gpio_request(GPIO_FN_AVB_RXD5, NULL);
	gpio_request(GPIO_FN_AVB_RXD6, NULL);
	gpio_request(GPIO_FN_AVB_RXD7, NULL);
	gpio_request(GPIO_FN_AVB_RX_ER, NULL);
	gpio_request(GPIO_FN_AVB_RX_CLK, NULL);
	gpio_request(GPIO_FN_AVB_RX_DV, NULL);
	gpio_request(GPIO_FN_AVB_CRS, NULL);
	gpio_request(GPIO_FN_AVB_MDC, NULL);
	gpio_request(GPIO_FN_AVB_MDIO, NULL);
	gpio_request(GPIO_FN_AVB_GTX_CLK, NULL);
	gpio_request(GPIO_FN_AVB_GTXREFCLK, NULL);
	gpio_request(GPIO_FN_AVB_TX_EN, NULL);
	gpio_request(GPIO_FN_AVB_TX_ER, NULL);
	gpio_request(GPIO_FN_AVB_TX_CLK, NULL);
	gpio_request(GPIO_FN_AVB_COL, NULL);
	gpio_request(GPIO_FN_AVB_RXD0, NULL);

	gpio_request(GPIO_GP_5_0, NULL);	/* PHY_RST */
	gpio_direction_output(GPIO_GP_5_0, 0);
	mdelay(20);
	gpio_set_value(GPIO_GP_5_0, 1);
	udelay(1);
#endif

	/* ETHER Enable */
#ifdef CONFIG_SH_ETHER
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

	gpio_request(GPIO_GP_5_16, NULL);
	gpio_direction_output(GPIO_GP_5_16, 0);

	gpio_request(GPIO_GP_5_0, NULL);	/* PHY_RST */
	gpio_direction_output(GPIO_GP_5_0, 0);
	mdelay(20);
	gpio_set_value(GPIO_GP_5_0, 1);
	udelay(1);
#endif

#ifdef CONFIG_SH_SDHI
	gpio_request(GPIO_FN_SD0_DATA0, NULL);
	gpio_request(GPIO_FN_SD0_DATA1, NULL);
	gpio_request(GPIO_FN_SD0_DATA2, NULL);
	gpio_request(GPIO_FN_SD0_DATA3, NULL);
	gpio_request(GPIO_FN_SD0_CLK, NULL);
	gpio_request(GPIO_FN_SD0_CMD, NULL);
	gpio_request(GPIO_FN_SD0_CD, NULL);
	gpio_request(GPIO_FN_SD0_WP, NULL);

	gpio_request(GPIO_FN_SD2_DATA0, NULL);
	gpio_request(GPIO_FN_SD2_DATA1, NULL);
	gpio_request(GPIO_FN_SD2_DATA2, NULL);
	gpio_request(GPIO_FN_SD2_DATA3, NULL);
	gpio_request(GPIO_FN_SD2_CLK, NULL);
	gpio_request(GPIO_FN_SD2_CMD, NULL);
	gpio_request(GPIO_FN_SD2_CD, NULL);
	gpio_request(GPIO_FN_SD2_WP, NULL);

	gpio_request(GPIO_FN_MMC0_CLK_SDHI1_CLK, NULL);
	gpio_request(GPIO_FN_MMC0_CMD_SDHI1_CMD, NULL);
	gpio_request(GPIO_FN_MMC0_D0_SDHI1_D0, NULL);
	gpio_request(GPIO_FN_MMC0_D1_SDHI1_D1, NULL);
	gpio_request(GPIO_FN_MMC0_D2_SDHI1_D2, NULL);
	gpio_request(GPIO_FN_MMC0_D3_SDHI1_D3, NULL);
	gpio_request(GPIO_FN_MMC0_D4, NULL);
	gpio_request(GPIO_FN_MMC0_D5, NULL);
	gpio_request(GPIO_FN_MMC0_D6, NULL);
	gpio_request(GPIO_FN_MMC0_D7, NULL);
#endif

	sh_timer_init();

	/* sdhi0 */
	gpio_request(GPIO_GP_4_22, NULL);
	gpio_request(GPIO_GP_4_23, NULL);
	gpio_direction_output(GPIO_GP_4_22, 1);
	gpio_direction_output(GPIO_GP_4_23, 1);
	/* sdhi2 */
	gpio_request(GPIO_GP_4_24, NULL);
	gpio_request(GPIO_GP_4_25, NULL);
	gpio_direction_output(GPIO_GP_4_24, 1);
	gpio_direction_output(GPIO_GP_4_25, 1);
	/* eMMC */
	gpio_request(GPIO_GP_5_25, NULL);
	gpio_direction_output(GPIO_GP_5_25, 1);

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

#ifdef CONFIG_RAVB
	ret = ravb_initialize(bis);
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
	gd->bd->bi_dram[0].start = ALEX_SDRAM_BASE;
	gd->bd->bi_dram[0].size = ALEX_SDRAM_SIZE;
}

int board_late_init(void)
{
	return 0;
}

int board_mmc_init(bd_t *bis)
{
	int ret = 0;

#ifdef CONFIG_SH_SDHI
	/* use SDHI0 */
	ret = sdhi_mmc_init(SDHI0_BASE, 0);
	if (ret)
		return ret;

	/* use SDHI1 for eMMC */
	ret = sdhi_mmc_init(SDHI1_BASE, 1);
	if (ret)
		return ret;

	/* use SDHI2 */
	ret = sdhi_mmc_init(SDHI2_BASE, 2);
#endif

	return ret;
}

void reset_cpu(ulong addr)
{
	u8 val;
#ifdef CONFIG_I2C_DA9063_USE
	i2c_init(CONFIG_SYS_I2C_SPEED, 0);
	i2c_read(DA9063_I2C_ADDR, REG_LDO5_CONT, 1, &val, 1);
	val |= L_LDO5_PD_DIS;
	i2c_write(DA9063_I2C_ADDR, REG_LDO5_CONT, 1, &val, 1);

	i2c_read(DA9063_I2C_ADDR, REG_CONTROL_F, 1, &val, 1);
	val |= L_SHUTDOWN;
	i2c_write(DA9063_I2C_ADDR, REG_CONTROL_F, 1, &val, 1);
#else
	/* Enable RWDT clock */
	val = readl(MSTPSR4);
	val &= ~RWDT_MSTP402;
	writel(val, SMSTPCR4);

	/* Enable RWDT reset request */
	writel(RST_WDTRSTCR_RWDT, RST_BASE + RST_WDTRSTCR);
	/* Set boot address */
	writel(RST_CABAR_BOOTADR, RST_BASE + RST_CA7BAR);

	/* Set the wdt counter to overflow just before */
	writel(RWDT_RWTCNT_FULL, RWDT_BASE + RWDT_RWTCNT);
	/* Start count up of RWDT */
	writel(RWDT_RWTCSRA_START, RWDT_BASE + RWDT_RWTCSRA);
#endif
}

enum {
	MSTP00, MSTP01, MSTP02, MSTP03, MSTP04, MSTP05,
	MSTP07, MSTP08, MSTP09, MSTP10,
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
	[MSTP00] = { SMSTPCR0,  0x00440001, 0x00400000,
		     RMSTPCR0,  0x00440001, 0x00000000 },
	[MSTP01] = { SMSTPCR1,  0x9168998A, 0x00000000,
		     RMSTPCR1,  0x9168998A, 0x00000000 },
	[MSTP02] = { SMSTPCR2,  0x100C2120, 0x00002000,
		     RMSTPCR2,  0x100C2120, 0x00000000 },
	[MSTP03] = { SMSTPCR3,  0xEC087800, 0x00000000,
		     RMSTPCR3,  0xEC087800, 0x00000000 },
	[MSTP04] = { SMSTPCR4,  0x000003C4, 0x00000180,
		     RMSTPCR4,  0x000003C4, 0x00000000 },
	[MSTP05] = { SMSTPCR5,  0x40800004, 0x00000000,
		     RMSTPCR5,  0x40800004, 0x00000000 },
	[MSTP07] = { SMSTPCR7,  0x0DBFE078, 0x00200000,
		     RMSTPCR7,  0x0DBFE078, 0x00000000 },
	[MSTP08] = { SMSTPCR8,  0x00003C03, 0x00000000,
		     RMSTPCR8,  0x00003C03, 0x00000000 },
	[MSTP09] = { SMSTPCR9,  0xF8479F88, 0x00000000,
		     RMSTPCR9,  0xF8479F88, 0x00000000 },
	[MSTP10] = { SMSTPCR10, 0x7E3EFFE0, 0x00000000,
		     RMSTPCR10, 0x7E3EFFE0, 0x00000000 },
};

#define TSTR1   4
#define TSTR1_STR3      0x1

void arch_preboot_os()
{
	u32 val;
	int i;

	/* stop TMU1 */
	val = readb(TMU_BASE + TSTR1);
	val &= ~TSTR1_STR3;
	writeb(val, TMU_BASE + TSTR1);

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
