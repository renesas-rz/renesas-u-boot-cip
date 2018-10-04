/*
 * Copyright (c) 2015 iWave Systems Technologies Pvt. Ltd.
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

/*
 * @file iwg23s_Pi.c 
 *
 * @brief Board file Description 
 *
 * @ingroup Main
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
#include <miiphy.h>
#include <i2c.h>
#include "iwg23s_Pi.h"
#include <libfdt.h>
#include <spi_flash.h>
#include <micrel.h>

#define BSP_VERSION                             "iW-PRFCC-SC-01-R3.0-REL1.0-Linux3.10.31"
#define SOM_VERSION                             "iW-PRFCC-AP-01-R3.x"

u8 rst_cause = 0;

DECLARE_GLOBAL_DATA_PTR;

extern char *name_spi;

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
	struct r8a7747x_rwdt *rwdt = (struct iwg23s_rwdt *)RWDT_BASE;
	struct r8a7747x_swdt *swdt = (struct iwg23s_swdt *)SWDT_BASE;
	u32 val;
	u32 pll0_status;

	/* Reading the watchdog status register */ 
        rst_cause = readb(0xE6020004);

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

void reset_cause(void)
{
        if(rst_cause & 0x10)
                printf("Reset Cause : WDOG\n");
        else
                printf("Reset Cause : POR\n");

}

#define TMU1_MSTP111    (1 << 11)

#define I2C1_MSTP930	(1 << 30)
#define SDHI1_MSTP313	(1 << 13)
#define SDHI2_MSTP312	(1 << 12)

#define SCIF1_MSTP720   (1 << 20)

#define AVB_MSTP812	(1 << 12)

#define SDCKCR		0xE6150074
#define SDH_780MHZ	(0x0 << 8)
#define SD0_97500KHZ	(0x6 << 4)
#define SD1_156000KHZ	0xC
#define SD1_97500KHZ    0x6
#define SD2CKCR		0xE6150078
#define SD2_97500KHZ	0x7

#define SPI_MSTP918   (1 << 18)

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
	/* SCIF1 */
	val = readl(MSTPSR7);
	val &= ~SCIF1_MSTP720;
	writel(val, SMSTPCR7);
#ifdef CONFIG_SPI
	val = readl(MSTPSR9);
	val &= ~SPI_MSTP918;
	writel(val, SMSTPCR9);
#endif

	/* AVB */
	val = readl(MSTPSR8);
#ifdef CONFIG_RAVB
	val &= ~AVB_MSTP812;
#endif
	writel(val, SMSTPCR8);

	/* MMC/SD */
	val = readl(MSTPSR3);
	val &= ~(SDHI1_MSTP313 | SDHI2_MSTP312);
	writel(val, SMSTPCR3);

	/*
	 * Set SD1 to the 97.5MHz as well.
	 * Set SD2 to the 97.5MHz as well.
	 */
	writel(SDH_780MHZ | SD0_97500KHZ | SD1_97500KHZ,
		SDCKCR);
	writel(SD2_97500KHZ, SD2CKCR);

	return 0;
}

DECLARE_GLOBAL_DATA_PTR;
int board_init(void)
{
	u32 val;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = IWG23S_SDRAM_BASE + 0x100;

	/* Init PFC controller */
	r8a7747x_pinmux_init();

#ifdef CONFIG_SCIF_CONSOLE
	/* UART */
	/* IWG23S: UART: UART MUX pin defined */
	gpio_request(GPIO_FN_RX1_B, NULL);
	gpio_request(GPIO_FN_TX1_B, NULL);
#endif

#ifdef CONFIG_SPI
	gpio_request(GPIO_FN_QSPI0_SPCLK, NULL);
	gpio_request(GPIO_FN_QSPI0_MOSI_IO0, NULL);
	gpio_request(GPIO_FN_QSPI0_MISO_IO1, NULL);
	gpio_request(GPIO_FN_QSPI0_SSL, NULL);
#endif

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

	udelay(1);
#endif

#ifdef CONFIG_SH_SDHI
	gpio_request(GPIO_FN_SD2_DATA0, NULL);
	gpio_request(GPIO_FN_SD2_DATA1, NULL);
	gpio_request(GPIO_FN_SD2_DATA2, NULL);
	gpio_request(GPIO_FN_SD2_DATA3, NULL);
	gpio_request(GPIO_FN_SD2_CLK, NULL);
	gpio_request(GPIO_FN_SD2_CMD, NULL);
	gpio_request(GPIO_FN_SD2_CD, NULL);
	gpio_request(GPIO_GP_2_24, NULL);
	gpio_direction_output(GPIO_GP_2_24, 1);

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
	gpio_request(GPIO_GP_5_14, NULL);
	gpio_direction_output(GPIO_GP_5_14, 1);
	udelay(1000);
	gpio_direction_output(GPIO_GP_5_14, 0);
	udelay(1000);
	gpio_direction_output(GPIO_GP_5_14, 1);
#endif
	gpio_request(GPIO_FN_USB1_PWEN, NULL);

	sh_timer_init();

	return 0;
}

static void rzg1x_gmii_rework(struct phy_device *phydev)
{
	/* skew settings */
	/* min rx/tx control */
	phydev->drv->writeext(phydev, MII_KSZ9031_EXT_MMD_ADD2,
			MII_KSZ9031_EXT_RGMII_CTRL_SKEW, 0x8000, 0x0080);
	/* min rx data delay */ 
	phydev->drv->writeext(phydev, MII_KSZ9031_EXT_MMD_ADD2,
			MII_KSZ9031_EXT_RGMII_RX_DATA_SKEW, 0x8000, 0x7787);
	/* min tx data delay */
	phydev->drv->writeext(phydev, MII_KSZ9031_EXT_MMD_ADD2,
			MII_KSZ9031_EXT_RGMII_RX_DATA_SKEW, 0x8000, 0x0000);
	/* max rx/tx clock delay */
	phydev->drv->writeext(phydev, MII_KSZ9031_EXT_MMD_ADD2,
			MII_KSZ9031_EXT_RGMII_CLOCK_SKEW, 0x8000, 0x03ff);
}

int board_phy_config(struct phy_device *phydev)
{
        if (phydev->drv->config)
                phydev->drv->config(phydev);

        return 0;
}

int board_eth_init(bd_t *bis)
{
	int ret = -ENODEV;
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
	gd->bd->bi_dram[0].start = IWG23S_SDRAM_BASE;
	gd->bd->bi_dram[0].size = IWG23S_SDRAM_SIZE;
}

/* IWG23S: Print: iWave BSP and SOM version print added */
void print_board_info(void)
{
        printf("\n");
        printf("Board Info:\n");
        printf("\tBSP Version     : %s\n", BSP_VERSION);
        printf("\tSOM Version     : %s\n", SOM_VERSION);
        printf("\n");
}

int board_late_init(void)
{
	print_board_info();
	return 0;
}

int board_mmc_init(bd_t *bis)
{
	int ret = 0;

#ifdef CONFIG_SH_SDHI
	/* use SDHI2 */
	ret = sdhi_mmc_init(SDHI2_BASE, 2);
	if (ret)
		return ret;
#endif

	/* use SDHI1 for eMMC */
	ret = sdhi_mmc_init(SDHI1_BASE, 1);
	if (ret)
		return ret;

	return ret;
}

/* IWG23: SPI: Adding fdt support for SST25VF016B,SST26VF016B,IS25LP016D */
void iwg23s_fdt_update(void *fdt)
{
	int nodeoffset;
	nodeoffset = fdt_path_offset (fdt, "/spi@e6b10000/flash@0");
        if ( name_spi == "SST25VF016B" )
                fdt_setprop_string(fdt, nodeoffset, "compatible", "sst,sst25vf016b");
        else if (name_spi == "SST26VF016B")
                fdt_setprop_string(fdt, nodeoffset, "compatible", "sst,sst26vf016b");
        else if (name_spi == "IS25LP016D")
                fdt_setprop_string(fdt, nodeoffset, "compatible", "sst,is25lp016d");
        else
                fdt_setprop_string(fdt, nodeoffset, "compatible", "sst,m25pe16");
}

void reset_cpu(ulong addr)
{
	u8 val;
	/* Enable RWDT clock */
	val = readl(MSTPSR4);
	val &= ~RWDT_MSTP402;
	writel(val, SMSTPCR4);

	/* Disable presetout */
	writel(0x0, RST_BASE + RST_RSTOUTCR);

	/* 10ms delay after disabling presetout */
	mdelay(10);

	/* Enable RWDT reset request */
	writel(RST_WDTRSTCR_RWDT, RST_BASE + RST_WDTRSTCR);
	/* Set boot address */
	writel(RST_CABAR_BOOTADR, RST_BASE + RST_CA7BAR);

	/* Set the wdt counter to overflow just before */
	writel(RWDT_RWTCNT_FULL, RWDT_BASE + RWDT_RWTCNT);
	/* Start count up of RWDT */
	writel(RWDT_RWTCSRA_START, RWDT_BASE + RWDT_RWTCSRA);
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
	[MSTP07] = { SMSTPCR7,  0x0DBFE078, 0x00100000,
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
