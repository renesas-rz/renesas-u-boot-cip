/*
 * Copyright (c) 2016 iWave Systems Technologies Pvt. Ltd.
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
 * @file iwg22m_sm.c 
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
#include "iwg22m_sm.h"
#include <fdt_support.h>
#include <libfdt.h>
#include <micrel.h>
#include <miiphy.h>

#define BSP_VERSION				"iW-EMEWQ-SC-01-Linux4.4"
#define SOM_VERSION				"iW-PREZZ-AP-01"
u8 rst_cause = 0;

DECLARE_GLOBAL_DATA_PTR;

void s_init(void)
{

#if !defined(CONFIG_EXTRAM_BOOT)
	/* QoS */
	qos_init();
#endif
}

void reset_cause(void)
{
	struct iwg22m_rwdt *rwdt = (struct iwg22m_rwdt *)RWDT_BASE;
	struct iwg22m_swdt *swdt = (struct iwg22m_swdt *)SWDT_BASE;

	/* Reading the watchdog status register */
	rst_cause = readb(0xE6020004);
	if(rst_cause & 0x10)
		printf("Reset Cause : WDOG\n");
	else
		printf("Reset Cause : POR\n");
	/* Watchdog init */
	writel(0xA5A5A500, &rwdt->rwtcsra);
	writel(0xA5A5A500, &swdt->swtcsra);
}

#define SYSDMAC0_MSTP219	(1 << 19)

#define SYSDMAC_L		0xE6700000
#define SYSDMAC_OR		0x60
#define SYSDMAC_CHCLR		0x80

#define SYSDMAC_CH0		0xE6708000
#define SYSDMAC_SAR		0x00
#define SYSDMAC_DAR		0x04
#define SYSDMAC_TCR		0x08
#define SYSDMAC_CHCR		0x0C
#define SYSDMAC_CHCRB		0x1C
#define SYSDMAC_DPBASE		0x50

#define SYSDMAC_DESC_L		0xE670A000
#define SYSDMAC_DESC_SAR	0x0
#define SYSDMAC_DESC_DAR	0x4
#define SYSDMAC_DESC_TCR	0x8

/* ARM-INTC work around */
int start_dma_transfer(void)
{
	u32 val;

	/* Enable clock of SYS-DMAC0 */
	val = readl(MSTPSR2);
	val &= ~SYSDMAC0_MSTP219;
	writel(val, SMSTPCR2);
	do {
		val = readl(MSTPSR2) & SYSDMAC0_MSTP219;
	} while (val);

	/* Initialize ch0, Reset Descriptor */
	writel(0x00000001, SYSDMAC_L + SYSDMAC_CHCLR);
	writel(0x00008000, SYSDMAC_CH0 + SYSDMAC_CHCRB);

	/* Enable DMA */
	writel(0x00000001, SYSDMAC_L + SYSDMAC_OR);

	/* Set first transfer */
	writel(0xF1001FFC, SYSDMAC_CH0 + SYSDMAC_SAR);
	writel(0xE61C0000, SYSDMAC_CH0 + SYSDMAC_DAR);
	writel(0x00000001, SYSDMAC_CH0 + SYSDMAC_TCR);

	/* Set descriptor */
	writel(0x00000000, SYSDMAC_DESC_L + SYSDMAC_DESC_SAR);
	writel(0x00000000, SYSDMAC_DESC_L + SYSDMAC_DESC_DAR);
	writel(0x00200000, SYSDMAC_DESC_L + SYSDMAC_DESC_TCR);
	writel(0x00000080, SYSDMAC_CH0 + SYSDMAC_CHCRB);
	writel(SYSDMAC_DESC_L | 0x00000001, SYSDMAC_CH0 + SYSDMAC_DPBASE);

	/* Set transfer parameter, Start transfer */
	writel(0x32000411, SYSDMAC_CH0 + SYSDMAC_CHCR);
#ifndef CONFIG_DCACHE_OFF
	/*
	 * The caches are disabled when ACTLR.SMP is set to 0
	 * regardless of the value of the SCTLR.C (cache enable bit)
	 * on Cortex-A7 MPCore
	 */
	asm volatile(
		"mrc	p15, 0, r0, c1, c0, 1\n"	/* read ACTLR */
		"orr	r0, r0, #(1 << 6)\n"		/* set SMP */
		"mcr	p15, 0, r0, c1, c0, 1\n");	/* write ACTLR */
#endif
	return 0;
}

#define TMU0_MSTP125	(1 << 25)

#define IIC1_MSTP323	(1 << 23)

#ifdef CONFIG_SH_MMCIF
#define MMC0_MSTP315	(1 << 15)
/* IWG22M: eMMC: Defining eMMC clock frequency to 97500Khz*/
#define MMC0CKCR 	0xE6150240
#define MMC0_97500KHZ   0x7
#endif

#define SDHI0_MSTP314   (1 << 14)
#define SDHI1_MSTP312	(1 << 12)

/* IWG22M: UART: 15th bit of MSTPR register to enable Debug UART */
#define SCIF4_MSTP715    (1 << 15)

#define ETHER_MSTP813	(1 << 13)

#ifdef CONFIG_SH_ETHER_RAVB
#define ETHER_MSTP812  (1 << 12)
#endif

#define SD1CKCR		0xE6150078
#define SD1_97500KHZ	0x7

#define SD0CKCR         0xE6150074
#define SD0_97500KHZ    0x60

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

	/* SCIF4 */
	val = readl(MSTPSR7);
	val &= ~SCIF4_MSTP715;
	writel(val, SMSTPCR7);

	/* ETHER */
	val = readl(MSTPSR8);
	val &= ~ETHER_MSTP813;
	writel(val, SMSTPCR8);

       	/*ETHER AVB */
	#ifdef CONFIG_SH_ETHER_RAVB
        val = readl(MSTPSR8);
        val &= ~ETHER_MSTP812;
        writel(val, SMSTPCR8);
	#endif

	/* MMC/SD */
	val = readl(MSTPSR3);
	val &= ~(MMC0_MSTP315 | SDHI0_MSTP314 | SDHI1_MSTP312);
	writel(val, SMSTPCR3);

	/*
	 * SD0 clock is set to 97.5MHz by default.
	 * Set SD1 to the 97.5MHz as well.
	 */
	writel(SD0_97500KHZ, SD0CKCR);
	writel(SD1_97500KHZ, SD1CKCR);

	/* ARM-INTC work around */
	start_dma_transfer();

#ifdef CONFIG_SH_MMCIF
	writel(MMC0_97500KHZ, MMC0CKCR);
#endif

	return 0;
}

/* LSI pin pull-up control */
#define PUPR3		0xe606010C
#define PUPR3_ETH	0x007FF800
#define PUPR3_ETH_MAGIC	(1 << 20)

#define PUPR1		0xe6060104
#define PUPR1_DREQ0_N	(1 << 20)

int board_init(void)
{
	u32 val;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = IWG22M_SDRAM_BASE + 0x100;

	/* Init PFC controller */
	iwg22m_pinmux_init();

#ifdef CONFIG_SPI
        /* IWG22M:SPI: SPI pins defined */
        gpio_request(GPIO_FN_SPCLK, NULL);
        gpio_request(GPIO_FN_MOSI_IO0, NULL);
        gpio_request(GPIO_FN_MISO_IO1, NULL);
        gpio_request(GPIO_FN_SSL, NULL);
#endif

#ifdef CONFIG_SCIF_CONSOLE
	/* UART */ 
	/* IWG22M: UART: UART MUX pin defined */ 
	gpio_request(GPIO_FN_SCIF4_TXD_B, NULL); 
	gpio_request(GPIO_FN_SCIF4_RXD_B, NULL);
#endif

	/* ETHER Enable */
#ifdef CONFIG_SH_ETHER
	//ether mac
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
#ifdef CONFIG_SH_ETHER_RAVB
	/* IWG22M: Ethernet: MUX pin are added*/
	 /* ETHER AVB */
	gpio_request(GPIO_FN_AVB_RX_CLK, NULL);
	gpio_request(GPIO_FN_AVB_RX_DV, NULL);
	gpio_request(GPIO_FN_AVB_RXD0, NULL);
	gpio_request( GPIO_FN_AVB_RXD1, NULL);
	gpio_request(GPIO_FN_AVB_RXD2, NULL);
	gpio_request(GPIO_FN_AVB_RXD3, NULL);
	gpio_request(GPIO_FN_AVB_RXD4, NULL);	
	gpio_request( GPIO_FN_AVB_RXD5, NULL);
	gpio_request(GPIO_FN_AVB_RXD6, NULL);
	gpio_request(GPIO_FN_AVB_RXD7, NULL);
	gpio_request(GPIO_FN_AVB_RX_ER, NULL);
	gpio_request(GPIO_FN_AVB_COL, NULL);
	gpio_request(GPIO_FN_AVB_TX_EN, NULL);
	gpio_request(GPIO_FN_AVB_TX_CLK, NULL);
	gpio_request(GPIO_FN_AVB_TXD0, NULL);
	gpio_request(GPIO_FN_AVB_TXD1, NULL);
	gpio_request(GPIO_FN_AVB_TXD2, NULL);
	gpio_request(GPIO_FN_AVB_TXD3, NULL);
	gpio_request(GPIO_FN_AVB_TXD4, NULL);
	gpio_request(GPIO_FN_AVB_TXD5, NULL);
	gpio_request(GPIO_FN_AVB_TXD6, NULL);
	gpio_request(GPIO_FN_AVB_TXD7, NULL);
	gpio_request(GPIO_FN_AVB_TX_ER, NULL);
	gpio_request(GPIO_FN_AVB_GTX_CLK, NULL);
	gpio_request(GPIO_FN_AVB_MDC, NULL);
	gpio_request(GPIO_FN_AVB_MDIO, NULL);
	gpio_request(GPIO_FN_AVB_LINK, NULL);
	gpio_request(GPIO_FN_AVB_MAGIC, NULL);
	gpio_request(GPIO_FN_AVB_PHY_INT, NULL);
	gpio_request(GPIO_FN_AVB_CRS, NULL);
	gpio_request(GPIO_FN_AVB_GTXREFCLK, NULL);
#endif
   

	val = readl(PUPR3);
	val &= ~(PUPR3_ETH & ~PUPR3_ETH_MAGIC);
	writel(val, PUPR3);

#ifdef CONFIG_SH_MMCIF
	/* IWG22M: eMMC: eMMC IOMUX customisation added*/
  	/* MMC 0*/
	gpio_request(GPIO_FN_MMC_D0, NULL);
	gpio_request(GPIO_FN_MMC_D1, NULL);
	gpio_request(GPIO_FN_MMC_D2, NULL);
	gpio_request(GPIO_FN_MMC_D3, NULL);
	gpio_request(GPIO_FN_MMC_D4, NULL);
	gpio_request(GPIO_FN_MMC_D5, NULL);
	gpio_request(GPIO_FN_MMC_D6, NULL);
	gpio_request(GPIO_FN_MMC_D7, NULL);
	gpio_request(GPIO_FN_MMC_CLK, NULL);
	gpio_request(GPIO_FN_MMC_CMD, NULL);
	/* Assert this pin High after 10mSec to reset the eMMC*/
	//MMC0 RST
	gpio_direction_output(GPIO_GP_3_26, 1);
	udelay(1000);
	gpio_direction_output(GPIO_GP_3_26, 0);
	udelay(1000);
	gpio_direction_output(GPIO_GP_3_26, 1);
#endif

#ifdef CONFIG_SH_SDHI
	gpio_request(GPIO_FN_SD0_DATA0, NULL);
	gpio_request(GPIO_FN_SD0_DATA1, NULL);
	gpio_request(GPIO_FN_SD0_DATA2, NULL);
	gpio_request(GPIO_FN_SD0_DATA3, NULL);
	gpio_request(GPIO_FN_SD0_CLK, NULL);
	gpio_request(GPIO_FN_SD0_CMD, NULL);
	gpio_request(GPIO_GP_0_20, NULL);
	gpio_direction_output(GPIO_GP_0_20, 1);
 	gpio_request(GPIO_GP_6_6, NULL);
	gpio_direction_input(GPIO_GP_6_6);
	gpio_request(GPIO_FN_SD1_DATA0, NULL);
	gpio_request(GPIO_FN_SD1_DATA1, NULL);
	gpio_request(GPIO_FN_SD1_DATA2, NULL);
	gpio_request(GPIO_FN_SD1_DATA3, NULL);
	gpio_request(GPIO_FN_SD1_CLK, NULL);
	gpio_request(GPIO_FN_SD1_CMD, NULL);
	gpio_request(GPIO_GP_3_31, NULL); 
	gpio_direction_input(GPIO_GP_3_31);
#endif
	sh_timer_init();

        /* LCD Backlight disable */
        gpio_request(GPIO_GP_1_13, NULL);
        gpio_direction_output(GPIO_GP_1_13, 0);

	return 0;
}

int board_eth_init(bd_t *bis)
{
	int ret = -ENODEV;

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
	setenv("eth_vin", "ethernet");
#elif defined(CONFIG_SH_ETHER_RAVB)
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
	gd->bd->bi_dram[0].start = IWG22M_SDRAM_BASE;
	gd->bd->bi_dram[0].size = IWG22M_SDRAM_SIZE;
}

/* IWG22M: Print: iWave BSP and SOM version print added */
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
	print_board_info ();
	return 0;
}

int board_mmc_init(bd_t *bis)
{
	int ret = 0;

#ifdef CONFIG_SH_SDHI
	/* use SDHI0,1 */
        ret = sdhi_mmc_init(SDHI0_BASE, 0);
        if (ret)
                return ret;

	ret = sdhi_mmc_init(SDHI1_BASE, 1);
#endif
#ifdef CONFIG_SH_MMCIF
        ret = mmcif_mmc_init();
#endif


	return ret;
}

void reset_cpu(ulong addr)
{
        struct iwg22m_rwdt *rwdt = (struct iwg22m_rwdt *)RWDT_BASE;

	/* Disable presetout */
	writel(0x0, RST_RSTOUTCR);
	/* 10ms delay after disabling presetout */
	mdelay(10);


        writel(0x5A5AFF00, &rwdt->rwtcnt);
        writel(0xA5A5A500, &rwdt->rwtcsra);
        writel(0xA55A0002, WDTRSTCR);
        writel(0XE63410, CA7BAR);
        writel(0xA5A5A580, &rwdt->rwtcsra);

}

void iwg22m_fdt_update(void *fdt)
{
	int nodeoffset, offs, ret;
	uint32_t addr_phandle;
	char *status_disabled = "disabled";

	/* iWave: FDT: camera selection */
	if (!strcmp("ov7725", getenv ("vin2_camera"))) {
		offs = fdt_path_offset(fdt, "/soc/i2c@e6528000/ov7725@21");
		fdt_status_okay(fdt, offs);
		do_fixup_by_path(fdt, "/soc/i2c@e6528000/ov5640@3c", "status", status_disabled, sizeof(status_disabled), 1);

		nodeoffset = fdt_path_offset (fdt, "/soc/i2c@e6528000/ov7725@21/port/endpoint");

		ret = fdt_create_phandle (fdt, nodeoffset);
		if (ret < 0) {
			printf ("Error creating camera node in FDT\n");
			return;
		}
		addr_phandle = fdt_get_phandle (fdt, nodeoffset);
		do_fixup_by_path_u32(fdt, "/soc/video@e6ef1000/port/endpoint", "remote-endpoint", addr_phandle, 0);
	}
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
	[MSTP02] = { SMSTPCR2,  0x100D21FC, 0x00082000,
		     RMSTPCR2,  0x100D21FC, 0x00000000 },
	[MSTP03] = { SMSTPCR3,  0xE084D810, 0x00000000,
		     RMSTPCR3,  0xE084D810, 0x00000000 },
	[MSTP04] = { SMSTPCR4,  0x800001C4, 0x00000180,
		     RMSTPCR4,  0x800001C4, 0x00000000 },
	[MSTP05] = { SMSTPCR5,  0x40C00044, 0x00000000,
		     RMSTPCR5,  0x40C00044, 0x00000000 },
	[MSTP07] = { SMSTPCR7,  0x013F6618, 0x00080000,
		     RMSTPCR7,  0x013F6618, 0x00000000 },
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
