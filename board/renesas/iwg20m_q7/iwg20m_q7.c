/*
 * Copyright (c) 2015-2016 iWave Systems Technologies Pvt. Ltd.
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
 * @file iwg20m_q7.c 
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
#include <i2c.h>
#include <micrel.h>
#include <miiphy.h>
#include <fdt.h>
#include <libfdt.h>
#include <fdt_support.h>
#include "iwg20m_q7.h"

#define BSP_VERSION     "iW-EMEWQ-SC-01Linux4.4"
#define   MODEMR        0xE6160060
static int som_revision;

DECLARE_GLOBAL_DATA_PTR;

#define PLL0CR		0xE61500D8

void s_init(void)
{
	u32 val;

	/* cpu frequency setting */
	val = readl(PLL0CR);
	val &= ~0x7F000000;
	val |= 0x4A000000;	/* 1.5GHz */
	writel(val, PLL0CR);

#if !defined(CONFIG_EXTRAM_BOOT)
	/* QoS */
	qos_init();
#endif
}
void reset_cause(void)
{
	struct r8a7743_rwdt *rwdt = (struct r8a7743_rwdt *)RWDT_BASE;
	struct r8a7743_swdt *swdt = (struct r8a7743_swdt *)SWDT_BASE;
	u8 rst_cause = 0;

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

	return 0;
}

#define MMC0_MSTP315	(1 << 15)
#define TMU0_MSTP125	(1 << 25)
#define SDHI0_MSTP314	(1 << 14)
#define SDHI1_MSTP312	(1 << 12)
#define SCIF0_MSTP721	(1 << 21)
#define ETHER_MSTP813	(1 << 13)
#ifdef CONFIG_SH_ETHER_RAVB
#define ETHER_MSTP812  (1 << 12)
#endif

#define SD1CKCR		0xE6150078
#define SD1_97500KHZ	0x7

#define MMC0CKCR       0xE6150240
#define MMC0_97500KHZ  0x7
#define THERMAL_MSTP522  (1 << 22)

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
	
    /* THERMAL */
    val = readl(MSTPSR5);
    val &= ~THERMAL_MSTP522;
    writel(val, SMSTPCR5);

	/*ETHER AVB */
#ifdef CONFIG_SH_ETHER_RAVB
	val = readl(MSTPSR8);
	val &= ~ETHER_MSTP812;
	writel(val, SMSTPCR8);
#endif

	/* SD */
	val = readl(MSTPSR3);
	val &= ~(SDHI0_MSTP314 | MMC0_MSTP315 | SDHI1_MSTP312);
	writel(val, SMSTPCR3);

	/*
	 * SD0 clock is set to 97.5MHz by default.
	 * Set SD1 and MMC0 to the 97.5MHz as well.
	 */
	writel(SD1_97500KHZ, SD1CKCR);
	writel(MMC0_97500KHZ, MMC0CKCR);

	/* ARM-INTC work around */
	start_dma_transfer();

	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = IWG20M_SDRAM_BASE + 0x100;

	/* Init PFC controller */
	iwg20m_pinmux_init();

	sh_timer_init();

/* AVB Ethernet Enable */
	gpio_request(GPIO_FN_AVB_RXD0, NULL);
	gpio_request(GPIO_FN_AVB_RXD1, NULL);
	gpio_request(GPIO_FN_AVB_RXD2, NULL);
	gpio_request(GPIO_FN_AVB_RXD3, NULL);
	gpio_request(GPIO_FN_AVB_RXD4, NULL);
	gpio_request(GPIO_FN_AVB_RXD5, NULL);
	gpio_request(GPIO_FN_AVB_RXD6, NULL);
	gpio_request(GPIO_FN_AVB_RXD7, NULL);
	gpio_request(GPIO_FN_AVB_TXD0, NULL);
	gpio_request(GPIO_FN_AVB_TXD1, NULL);
	gpio_request(GPIO_FN_AVB_TXD2, NULL);
	gpio_request(GPIO_FN_AVB_TXD3, NULL);
	gpio_request(GPIO_FN_AVB_TXD4, NULL);
	gpio_request(GPIO_FN_AVB_TXD5, NULL);
	gpio_request(GPIO_FN_AVB_TXD6, NULL);
	gpio_request(GPIO_FN_AVB_TXD7, NULL);
	gpio_request(GPIO_FN_AVB_RX_ER, NULL);
	gpio_request(GPIO_FN_AVB_MDIO, NULL);
	gpio_request(GPIO_FN_AVB_RX_DV, NULL);
	gpio_request(GPIO_FN_AVB_MAGIC, NULL);
	gpio_request(GPIO_FN_AVB_MDC, NULL);
	gpio_request(GPIO_FN_AVB_RX_CLK, NULL);
	gpio_request(GPIO_FN_AVB_LINK, NULL);
	gpio_request(GPIO_FN_AVB_CRS, NULL);
	gpio_request(GPIO_FN_AVB_PHY_INT, NULL);
	gpio_request(GPIO_FN_AVB_GTXREFCLK, NULL);
	gpio_request(GPIO_FN_AVB_TX_EN, NULL);
	gpio_request(GPIO_FN_AVB_TX_ER, NULL);
	gpio_request(GPIO_FN_AVB_TX_CLK, NULL);
	gpio_request(GPIO_FN_AVB_COL, NULL);
	gpio_request(GPIO_FN_AVB_GTX_CLK, NULL);

#ifdef CONFIG_SH_SDHI
	gpio_request(GPIO_FN_SD0_DATA0, NULL);
	gpio_request(GPIO_FN_SD0_DATA1, NULL);
	gpio_request(GPIO_FN_SD0_DATA2, NULL);
	gpio_request(GPIO_FN_SD0_DATA3, NULL);
	gpio_request(GPIO_FN_SD0_CLK, NULL);
	gpio_request(GPIO_FN_SD0_CMD, NULL);
	gpio_request(GPIO_FN_SD1_DATA0, NULL);
	gpio_request(GPIO_FN_SD1_DATA1, NULL);
	gpio_request(GPIO_FN_SD1_DATA2, NULL);
	gpio_request(GPIO_FN_SD1_DATA3, NULL);
	gpio_request(GPIO_FN_SD1_CLK, NULL);
	gpio_request(GPIO_FN_SD1_CMD, NULL);
	gpio_request(GPIO_FN_SD1_CD, NULL);
	gpio_request(GPIO_FN_SD1_WP, NULL);
	/* MicroSD Card Detect */
	gpio_request(GPIO_GP_7_11, NULL);
	gpio_direction_input(GPIO_GP_7_11);	/* SD0 card detect */
	/* SD power */
	gpio_request(GPIO_GP_1_16, NULL);
	gpio_direction_output(GPIO_GP_1_16, 0);
#endif

#ifdef CONFIG_MMC
	/* MMC */
	gpio_request(GPIO_FN_MMC_D6_B, NULL);
	gpio_request(GPIO_FN_MMC_D7_B, NULL);
	gpio_request(GPIO_FN_MMC_CLK, NULL);
	gpio_request(GPIO_FN_MMC_CMD, NULL);
	gpio_request(GPIO_FN_MMC_D0, NULL);
	gpio_request(GPIO_FN_MMC_D1, NULL);
	gpio_request(GPIO_FN_MMC_D2, NULL);
	gpio_request(GPIO_FN_MMC_D3, NULL);
	gpio_request(GPIO_FN_MMC_D4, NULL);
	gpio_request(GPIO_FN_MMC_D5, NULL);
#endif

	/* SATA LED */
	gpio_request(GPIO_GP_5_11, NULL);
	gpio_direction_output(GPIO_GP_5_11, 1);
	/* LCD Backlight, Power and PWM disable */ 
	gpio_request(GPIO_GP_6_30, NULL);
	gpio_direction_output(GPIO_GP_6_30, 0);
	gpio_request(GPIO_GP_7_25, NULL);
	gpio_direction_output(GPIO_GP_7_25, 0);
	gpio_request(GPIO_GP_1_24, NULL);
	gpio_direction_output(GPIO_GP_1_24, 0);

	return 0;
}
int board_phy_config(struct phy_device *phydev)
{
        if (phydev->drv->config)
                phydev->drv->config(phydev);

        return 0;
}

static void get_som_revision(void)
{
	int val[3];
	gpio_request(GPIO_GP_1_12, NULL);
	gpio_request(GPIO_GP_3_29, NULL);
	gpio_request(GPIO_GP_1_23, NULL);

        gpio_direction_input(GPIO_GP_1_12);
        gpio_direction_input(GPIO_GP_3_29);
        gpio_direction_input(GPIO_GP_1_23);

	val[0] = gpio_get_value(GPIO_GP_1_12);
	val[1] = gpio_get_value(GPIO_GP_3_29);
	val[2] = gpio_get_value(GPIO_GP_1_23);
	som_revision = (val[2] << 2) | (val[1] << 1) | val[0];
	
}

int dynamic_fdt_file_selection(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (rmobile_get_cpu_type() == 0x47)
		setenv("fdt_file","r8a7743-iwg20m_q7.dtb");
	else
		setenv("fdt_file","r8a7744-iwg20m_q7.dtb");
	return 0;
}

U_BOOT_CMD(
        dynamicfdt, 1, 0,    dynamic_fdt_file_selection,
        "Loading Device tree by checking Processor",
        ""
);

int print_board_info (void)
{
	get_som_revision();
	printf ("\n");
	printf ("Board Info:\n");
	printf ("\tBSP Version     : %s\n", BSP_VERSION);
	printf ("\tSOM Version     : iW-PREWZ-AP-01-R3.%x\n", som_revision);
	printf ("\n");
	return 0;
}

int board_eth_init(bd_t *bis)
{
	int ret = -ENODEV;
	u32 val, addr1, addr2;
	unsigned char enetaddr[6];

	if (!eth_getenv_enetaddr("ethaddr", enetaddr))
		return ret;

#ifdef CONFIG_SH_ETHER
	ret = sh_eth_initialize(bis);
	addr1 = 0xEE7003C0;
	addr2 = 0xEE7003C8;
#elif defined(CONFIG_SH_ETHER_RAVB)
	ret = ravb_initialize(bis);
	addr1 = 0xE68005C0;
	addr2 = 0xE68005C8;
#endif
	/* Set Mac address */
	val = enetaddr[0] << 24 | enetaddr[1] << 16 | enetaddr[2] << 8 | enetaddr[3];
	writel(val, addr1);
	val = enetaddr[4] << 8 | enetaddr[5];
	writel(val, addr2); 

	return ret;
}

int dram_init(void)
{
	if (rmobile_get_cpu_type() == 0x47)
		gd->ram_size = CONFIG_SYS_SDRAM_SIZE;
	else
		gd->ram_size = IWG20M_SDRAM_SIZE;

	return 0;
}

const struct rmobile_sysinfo sysinfo = {
	CONFIG_RMOBILE_BOARD_STRING,
	CONFIG_RMOBILE_STRING
};

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = IWG20M_SDRAM_BASE;
	gd->bd->bi_dram[0].size = IWG20M_SDRAM_SIZE;
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
	/* use SDHI0-2 */
	ret = sdhi_mmc_init(SDHI1_BASE, 1);
	if (ret)
		return ret;

	ret = sdhi_mmc_init(SDHI0_BASE, 0);
	if (ret)
		return ret;
#endif
#ifdef CONFIG_SH_MMCIF
	ret = mmcif_mmc_init();
		return ret;
#endif
}

void reset_cpu(ulong addr)
{
	struct r8a7743_rwdt *rwdt = (struct r8a7743_rwdt *)RWDT_BASE;
	/* Disable presetout */
	writel(0x0, RST_RSTOUTCR);

	/* 10ms delay after disabling presetout */
	mdelay(10);

	writel(0x5A5AFF00, &rwdt->rwtcnt);
	writel(0xA5A5A500, &rwdt->rwtcsra);
	writel(0xA55A0002, WDTRSTCR);
	writel(0XE63410, CA15BAR);
	writel(0xA5A5A580, &rwdt->rwtcsra);
}

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
	[MSTP00] = { SMSTPCR0,  0x00640801, 0x00400000,
		     RMSTPCR0,  0x00640801, 0x00000000 },
	[MSTP01] = { SMSTPCR1,  0x9B6C9B5A, 0x00000000,
		     RMSTPCR1,  0x9B6C9B5A, 0x00000000 },
	[MSTP02] = { SMSTPCR2,  0x100D21FC, 0x00082000,
		     RMSTPCR2,  0x100D21FC, 0x00000000 },
	[MSTP03] = { SMSTPCR3,  0xF08CD810, 0x00000000,
		     RMSTPCR3,  0xF08CD810, 0x00000000 },
	[MSTP04] = { SMSTPCR4,  0x800001C4, 0x00000180,
		     RMSTPCR4,  0x800001C4, 0x00000000 },
	[MSTP05] = { SMSTPCR5,  0x44C00046, 0x00000000,
		     RMSTPCR5,  0x44C00046, 0x00000000 },
	[MSTP07] = { SMSTPCR7,  0x05BFE618, 0x00200000,
		     RMSTPCR7,  0x05BFE618, 0x00000000 },
	[MSTP08] = { SMSTPCR8,  0x40C0FE85, 0x00000000,
		     RMSTPCR8,  0x40C0FE85, 0x00000000 },
	[MSTP09] = { SMSTPCR9,  0xFF979FFF, 0x00000000,
		     RMSTPCR9,  0xFF979FFF, 0x00000000 },
	[MSTP10] = { SMSTPCR10, 0xFFFEFFE0, 0x00000000,
		     RMSTPCR10, 0xFFFEFFE0, 0x00000000 },
	[MSTP11] = { SMSTPCR11, 0x000001C0, 0x00000000,
		     RMSTPCR11, 0x000001C0, 0x00000000 },
};

#define TSTR0		4
#define TSTR0_STR0	0x1

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

void iwg20m_fdt_update(void *fdt)
{
	int phy_mode, ret, offs;
	uint32_t addr_phandle, nodeoffset;
	char *status_disabled = "disabled";

	/* iWave: FDT: camera selection */
	if (!strcmp("ov7725", getenv ("vin2_camera")))
	{
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
		do_fixup_by_path_u32(fdt, "/soc/video@e6ef2000/port/endpoint", "remote-endpoint", addr_phandle, 0);
	}


	if ((rmobile_get_cpu_type() == 0x47))
	{
		/*
		* MD24:MD23
		*   0 0        SATA1 and SATA0
		*   0 1        SATA1 and USB3.0
		*   1 0        PCIe and SATA0
		*   1 1        PCIe and USB3.0
		*/
		/* iWave: USB: XHCI & USB2(PCI1) uses the USB PHY channel 2, either one can work at a time */
		phy_mode = (readl(MODEMR) & 0x1800000) >> 23;
		switch (phy_mode)
		{
			case 0:
				fdt_status_okay_by_alias(fdt, "sata0");
				fdt_status_okay_by_alias(fdt, "sata1");
				fdt_status_okay_by_alias(fdt, "usb2");
				break;
			case 1:
				fdt_status_okay_by_alias(fdt, "sata1");
				fdt_status_okay_by_alias(fdt, "xhci");
				break;
			case 2:
				fdt_status_okay_by_alias(fdt, "sata0");
				fdt_status_okay_by_alias(fdt, "pciec");
				fdt_status_okay_by_alias(fdt, "pcie_clk");
				fdt_status_okay_by_alias(fdt, "usb2");
				break;

			case 3:
				fdt_status_okay_by_alias(fdt, "xhci");
				fdt_status_okay_by_alias(fdt, "pciec");
				fdt_status_okay_by_alias(fdt, "pcie_clk");
				break;
			default:
				break;
		}
	} else {
		/*
		* MD24:MD23
		*   0 0        SATA0
		*   0 1        USB3.0
		*   1 0        PCIe
		*/
		/* iWave: USB: XHCI & USB2(PCI1) uses the USB PHY channel 2, either one can work at a time */
		phy_mode = (readl(MODEMR) & 0x1800000) >> 23;
		switch (phy_mode)
		{
			case 0:
				fdt_status_okay_by_alias(fdt, "sata0");
				fdt_status_okay_by_alias(fdt, "usb2");
				break;
			case 1:
				fdt_status_okay_by_alias(fdt, "xhci");
				break;
			case 2:
				fdt_status_okay_by_alias(fdt, "pciec");
				fdt_status_okay_by_alias(fdt, "pcie_clk");
				fdt_status_okay_by_alias(fdt, "usb2");
			break;
				default:
			break;

		}
	}
}

