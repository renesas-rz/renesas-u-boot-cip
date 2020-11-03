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
 * @file iwg21m_q7.c 
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
#include "iwg21m_q7.h"
#include <fdt.h>
#include <libfdt.h>
#include <fdt_support.h>

#define BSP_VERSION                             "iW-PREXZ-SC-01-R2.0-REL1.0-Linux3.10.31-PATCH009"

DECLARE_GLOBAL_DATA_PTR;

#define PLL0CR		0xE61500D8

#define PLLECR		0xE61500D0
#define PLL0ST		0x100


#define FRQCRB			0xE6150004
#define FRQCRB_KICK		(0x1ul << 31)

#define FRQCRC			0xE61500E0
#define FRQCRC_ZFC_MASK		(0x1Ful << FRQCRC_ZFC_SHIFT)
#define FRQCRC_ZFC_SHIFT	(8)

#define PUPR3		0xE606010C
#define PUPR3_MMC	0xFEFEC000

#define   MODEMR        0xE6160060

#define s_init_wait(cnt) \
		({	\
			volatile u32 i = 0x10000 * cnt;	\
			while (i > 0)	\
				i--;	\
		})

static int som_rev;
int sw1_val;
u8 rst_cause = 0;

void s_init(void)
{
	struct iwg21m_rwdt *rwdt = (struct iwg21m_rwdt *)RWDT_BASE;
	struct iwg21m_swdt *swdt = (struct iwg21m_swdt *)SWDT_BASE;
	u32 val;
	u32 pll0_status;

	/* cpu frequency setting */
	if (rmobile_get_cpu_rev_integer() >= IWG21M_CUT_ES2X) {
		u32 mult;
		u32 kick;
		int count;

		val = readl(FRQCRC);
		mult = (val & FRQCRC_ZFC_MASK) >> FRQCRC_ZFC_SHIFT;

		/* z clk ratio is 1/32 */
		val &= ~FRQCRC_ZFC_MASK;
		val |= 0x1Ful << FRQCRC_ZFC_SHIFT;
		val |= 0x1Ful;
		writel(val, FRQCRC);

		kick = readl(FRQCRB);
		kick |= FRQCRB_KICK;
		writel(kick, FRQCRB);

		do {
			kick = readl(FRQCRB) & FRQCRB_KICK;
		} while (kick);

		val = readl(PLL0CR);
		val &= ~0x7F000000;
		val |= 0x45000000;			/* 1.4GHz */
		writel(val, PLL0CR);

		count = 0;
		do {
			pll0_status = readl(PLLECR) & PLL0ST;
			if (pll0_status)
				count++;
			else
				count = 0;

		} while (count < 10);

		/* z clk ratio is <mult>/32 */
		val = readl(FRQCRC);
		val &= ~FRQCRC_ZFC_MASK;
		val |= mult << FRQCRC_ZFC_SHIFT;
		val &= ~0x1Ful;
		writel(val, FRQCRC);

		kick = readl(FRQCRB);
		kick |= FRQCRB_KICK;
		writel(kick, FRQCRB);

		do {
			kick = readl(FRQCRB) & FRQCRB_KICK;
		} while (kick);
	}

	/* QoS */
#if !(defined(CONFIG_EXTRAM_BOOT))
	qos_init();
#endif
}

void reset_cause(void)
{
        struct iwg21m_rwdt *rwdt = (struct iwg21m_rwdt *)RWDT_BASE;
        struct iwg21m_swdt *swdt = (struct iwg21m_swdt *)SWDT_BASE;

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
#ifdef CONFIG_SH_ETHER_RAVB
#define ETHER_MSTP812		(1 << 12)
#endif

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

#define TMU0_MSTP125	(1 << 25)

#define SDHI0_MSTP314	(1 << 14)
#define SDHI1_MSTP313	(1 << 13)
#define SDHI2_MSTP312	(1 << 12)
#define SDHI3_MSTP311	(1 << 11)
#define MMC1_MSTP305	(1 << 5)

#define SCIFA2_MSTP202	(1 << 2)

#define ETHER_MSTP813	(1 << 13)

#define SD2CKCR		0xE6150078
#define SD2_97500KHZ	0x7

#define MMC1CKCR        0xE6150244
#define MMC1_97500KHZ   0x7

int board_early_init_f(void)
{
	u32 val;

	/* TMU0 */
	val = readl(MSTPSR1);
	val &= ~TMU0_MSTP125;
	writel(val, SMSTPCR1);

	/* UART */
	val = readl(MSTPSR2);
	val &= ~SCIFA2_MSTP202;
	writel(val, SMSTPCR2);

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

	/* eMMC/SD */
	val = readl(MSTPSR3);
	/* IWG21M: eMMC/SD: setting 5th bit of MSTPCR register to enable eMMC and 
	12th bit to enable SDHI2*/
	val &= ~(MMC1_MSTP305 | SDHI2_MSTP312);
	writel(val, SMSTPCR3);

	/*
	 * Set SD2 and MMC1 to the 97.5MHz as well.
	 */
	writel(SD2_97500KHZ, SD2CKCR);
	writel(MMC1_97500KHZ, MMC1CKCR);

	/* ARM-INTC work around */
	start_dma_transfer();

	return 0;
}

DECLARE_GLOBAL_DATA_PTR;
int board_init(void)
{
	u32 val;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = IWG21M_SDRAM_BASE + 0x100;

	/* Init PFC controller */
	iwg21m_pinmux_init();

	/* IWG21M:SPI: SPI pins defined */
	gpio_request(GPIO_FN_SPCLK, NULL);
    	gpio_request(GPIO_FN_MOSI_IO0, NULL);
	gpio_request(GPIO_FN_MISO_IO1, NULL);
	gpio_request(GPIO_FN_SSL, NULL);

	/* IWG21M:UART: UART pins defined */
	gpio_request(GPIO_FN_SCIFA2_TXD_C, NULL);
	gpio_request(GPIO_FN_SCIFA2_RXD_C, NULL);

#ifdef CONFIG_SH_ETHER_RAVB
         /* ETHER AVB */
        gpio_request(GPIO_FN_AVB_RX_CLK, NULL);
        gpio_request(GPIO_FN_AVB_RX_DV, NULL);
        gpio_request(GPIO_FN_AVB_RXD0, NULL);
        gpio_request(GPIO_FN_AVB_RXD1, NULL);
        gpio_request(GPIO_FN_AVB_RXD2, NULL);
        gpio_request(GPIO_FN_AVB_RXD3, NULL);
        gpio_request(GPIO_FN_AVB_RXD4, NULL);
        gpio_request(GPIO_FN_AVB_RXD5, NULL);
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

#ifdef CONFIG_SH_ETHER
	/* ETHER Enable */
	gpio_request(GPIO_FN_ETH_CRS_DV, NULL);
	gpio_request(GPIO_FN_ETH_RX_ER, NULL);
	gpio_request(GPIO_FN_ETH_RXD0, NULL);
	gpio_request(GPIO_FN_ETH_RXD1, NULL);
	gpio_request(GPIO_FN_ETH_REF_CLK, NULL);
	gpio_request(GPIO_FN_ETH_MDIO, NULL);
	gpio_request(GPIO_FN_ETH_TXD1, NULL);
	gpio_request(GPIO_FN_ETH_TX_EN, NULL);
	gpio_request(GPIO_FN_ETH_TXD0, NULL);
	gpio_request(GPIO_FN_ETH_MDC, NULL);
#endif
	sh_timer_init();

#ifdef CONFIG_SH_MMCIF
	gpio_request(GPIO_FN_MMC1_D0, NULL);
	gpio_request(GPIO_FN_MMC1_D1, NULL);
	gpio_request(GPIO_FN_MMC1_D2, NULL);
	gpio_request(GPIO_FN_MMC1_D3, NULL);
	gpio_request(GPIO_GP_4_12, NULL);
	//MMC 8/4bit mode selection
	gpio_direction_input(GPIO_GP_4_12);
	sw1_val = gpio_get_value(GPIO_GP_4_12);
	if( !sw1_val )	
	{
		gpio_request(GPIO_FN_MMC1_D4, NULL);
		gpio_request(GPIO_FN_MMC1_D5, NULL);
		gpio_request(GPIO_FN_MMC1_D6, NULL);
		gpio_request(GPIO_FN_MMC1_D7, NULL);
	}
	gpio_request(GPIO_FN_MMC1_CLK, NULL);
	gpio_request(GPIO_FN_MMC1_CMD, NULL);
#endif

#ifdef CONFIG_SH_SDHI
	gpio_request(GPIO_FN_SD2_DAT0, NULL);
	gpio_request(GPIO_FN_SD2_DAT1, NULL);
	gpio_request(GPIO_FN_SD2_DAT2, NULL);
	gpio_request(GPIO_FN_SD2_DAT3, NULL);
	gpio_request(GPIO_FN_SD2_CLK, NULL);
	gpio_request(GPIO_FN_SD2_CMD, NULL);
	gpio_request(GPIO_FN_SD2_CD, NULL);
	gpio_request(GPIO_FN_SD2_WP, NULL);
#endif

	//MMC1 RST
	gpio_request(GPIO_GP_2_22, NULL);
	gpio_direction_output(GPIO_GP_2_22, 0); 
	/* wait 10ms */
	udelay(10000);
	gpio_direction_output(GPIO_GP_2_22, 1); 


	//SDHI2
	//SD2_PWR 
	gpio_request(GPIO_GP_1_27, NULL);
	//SD2 LED
	gpio_request(GPIO_GP_5_22, NULL);
	//SD2_PWR_SW
	gpio_request(GPIO_GP_1_8, NULL);

	gpio_direction_output(GPIO_GP_1_27, 0); /* power on */
	gpio_set_value(GPIO_GP_1_27, 0);
	gpio_direction_output(GPIO_GP_5_22, 0); /* LED  */
	gpio_set_value(GPIO_GP_5_22, 1);
	gpio_direction_output(GPIO_GP_1_8, 0); /* 1: 3.3V, 0: 1.8V*/
	gpio_set_value(GPIO_GP_1_8, 1);

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

#ifdef CONFIG_SH_ETHER
	u32 val;
	unsigned char enetaddr[6];
	ret = sh_eth_initialize(bis);
	if (!eth_getenv_enetaddr("ethaddr", enetaddr))
		return ret;

	/* Set Mac address */
	val = enetaddr[0] << 24 | enetaddr[1] << 16 |
	    enetaddr[2] << 8 | enetaddr[3];
	writel(val, 0xEE7003C0);

	val = enetaddr[4] << 8 | enetaddr[5];
	writel(val, 0xEE7003C8);
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
	gd->bd->bi_dram[0].start = IWG21M_SDRAM_BASE;
	gd->bd->bi_dram[0].size = IWG21M_SDRAM_SIZE;
}

/* IWG21M: Read: iWave SOM version */
static void get_som_revision(void)
{
	int val[3];

	gpio_request(GPIO_GP_1_21, NULL);
	gpio_request(GPIO_GP_4_16, NULL);
	gpio_request(GPIO_GP_4_21, NULL);
	gpio_request(GPIO_GP_4_17, NULL);

	gpio_direction_input(GPIO_GP_1_21);
        gpio_direction_input(GPIO_GP_4_16);
        gpio_direction_input(GPIO_GP_4_21);
        gpio_direction_input(GPIO_GP_4_17);

	val[0] = gpio_get_value(GPIO_GP_1_21);
	val[1] = gpio_get_value(GPIO_GP_4_16);
	val[2] = gpio_get_value(GPIO_GP_4_21);
	val[3] = gpio_get_value(GPIO_GP_4_17);

	som_rev= (val[3] << 3) | (val[2] << 2) | (val[1] << 1) | val[0];

	gpio_free(GPIO_GP_1_21);
	gpio_free(GPIO_GP_4_16);
	gpio_free(GPIO_GP_4_21);
	gpio_free(GPIO_GP_4_17);
}

/* IWG21M: Print: iWave BSP and SOM version print added */
void print_board_info (void)
{
        get_som_revision();
        printf ("\n");
        printf ("Board Info:\n");
        printf ("\tSOM Version     : iW-PREXZ-AP-01-R2.%x\n", som_rev);
        printf ("\n");
        return ;
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
	/* use SDHI2 */
	ret = sdhi_mmc_init(SDHI2_BASE, 2);
#endif

#ifdef CONFIG_SH_MMCIF
	ret = mmcif_mmc_init();
#endif
	return ret;
}

void reset_cpu(ulong addr)
{
	struct iwg21m_rwdt *rwdt = (struct iwg21m_rwdt *)RWDT_BASE;
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
	[MSTP01] = { SMSTPCR1,  0xDB6E9BDF, 0x00000000,
		     RMSTPCR1,  0xDB6E9BDF, 0x00000000 },
	[MSTP02] = { SMSTPCR2,  0x300DA1FC, 0x00082004,
		     RMSTPCR2,  0x300DA1FC, 0x00000004 },
	[MSTP03] = { SMSTPCR3,  0xF08CF831, 0x00000000,
		     RMSTPCR3,  0xF08CF831, 0x00000000 },
	[MSTP04] = { SMSTPCR4,  0x80000184, 0x00000180,
		     RMSTPCR4,  0x80000184, 0x00000000 },
	[MSTP05] = { SMSTPCR5,  0x44C00046, 0x00000000,
		     RMSTPCR5,  0x44C00046, 0x00000000 },
	[MSTP07] = { SMSTPCR7,  0x07F30718, 0x00200000,
		     RMSTPCR7,  0x07F30718, 0x00000000 },
	[MSTP08] = { SMSTPCR8,  0x01F0FF84, 0x00000000,
		     RMSTPCR8,  0x01F0FF84, 0x00000000 },
	[MSTP09] = { SMSTPCR9,  0xF597804F, 0x00000000,
		     RMSTPCR9,  0xF597804F, 0x00000000 },
	[MSTP10] = { SMSTPCR10, 0xFFFEFFE0, 0x00000000,
		     RMSTPCR10, 0xFFFEFFE0, 0x00000000 },
	[MSTP11] = { SMSTPCR11, 0x00000000, 0x00000000,
		     RMSTPCR11, 0x00000000, 0x00000000 },
};

#define TSTR0	4
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

void iwg21m_fdt_update(void *fdt)
{
	u32 vin0_st, vin1_st, vin2_st, width, off;
	char *status_ok = "okay";
	char *vin2_check = NULL;
	int vin_all, val, phy_mode;

	/* Update the SOM revision */
	do_fixup_by_path_u32(fdt, "/soc/iwg21m_q7_common", "som-rev", som_rev, 1);
	/* iWave: FDT: camera selection */
	/* VIN0 camera selection */
	if (!strcmp("ov5640", getenv ("vin0_camera")))
		do_fixup_by_path_u32(fdt, "/soc/iwg21m_q7_common", "vin0-ov5640", 1, 0);
	else
		do_fixup_by_path_u32(fdt, "/soc/iwg21m_q7_common", "vin0-ov5640", 0, 0);

	/* VIN1 camera selection */
	if (!strcmp("ov5640", getenv ("vin1_camera")))
		do_fixup_by_path_u32(fdt, "/soc/iwg21m_q7_common", "vin1-ov5640", 1, 0);
	else
		do_fixup_by_path_u32(fdt, "/soc/iwg21m_q7_common", "vin1-ov5640", 0, 0);

	/* VIN2 camera selection */
	if (!strcmp("ov5640", getenv ("vin2_camera")))
		do_fixup_by_path_u32(fdt, "/soc/iwg21m_q7_common", "vin2-ov5640", 1, 0);
	else
		do_fixup_by_path_u32(fdt, "/soc/iwg21m_q7_common", "vin2-ov5640", 0, 0);

	/* VIN3 camera selection */
	if (!strcmp("ov5640", getenv ("vin3_camera")))
		do_fixup_by_path_u32(fdt, "/soc/iwg21m_q7_common", "vin3-ov5640", 1, 0);
	else
		do_fixup_by_path_u32(fdt, "/soc/iwg21m_q7_common", "vin3-ov5640", 0, 0);

	/*
	 * MD24:MD23
	 *   0 0        SATA1 and SATA0
	 *   0 1        SATA1 and USB3.0
	 *   1 0        PCIe and SATA0  
	 *   1 1        PCIe and USB3.0 
	 */
	phy_mode = (readl(MODEMR) & 0x1800000) >> 23;

	if( (phy_mode & 0x1) == 0x1 )	
	{
		fdt_status_okay_by_alias(fdt, "xhci");
		fdt_status_disabled_by_alias(fdt, "sata0");
	}
	else
	{
		fdt_status_okay_by_alias(fdt, "sata0");
		fdt_status_disabled_by_alias(fdt, "xhci");
	}

	if( (phy_mode & 0x2) == 0x2 )
	{
		fdt_status_okay_by_alias(fdt, "pciec");
		fdt_status_okay_by_alias(fdt, "pcie_clk");
		fdt_status_disabled_by_alias(fdt, "sata1");
	}
	else
	{
		fdt_status_okay_by_alias(fdt, "sata1");
		fdt_status_disabled_by_alias(fdt, "pciec");
		fdt_status_disabled_by_alias(fdt, "pcie_clk");
	}
	/* Update the VI0 or AVB selection
	 * switch(0)- AVB select
	 * switch(1)- VIN0 select 
	 */
	gpio_request(GPIO_GP_5_21, NULL);
	gpio_direction_input(GPIO_GP_5_21);
	if (gpio_get_value(GPIO_GP_5_21)) {
		/* Camera selected */
		vin0_st = 1;
	}else{
		/* AVB selected */
		vin0_st = 0;
		do_fixup_by_path_u32(fdt, "/soc/iwg21m_q7_common", "vin0-status", vin0_st, 1);
		do_fixup_by_path(fdt, "/soc/ethernet@e6800000", "status", status_ok, 4,  0);
	} 

	/* Update the VI1 or MMC-8bit selection
	 * switch(0)- mmc-8bit select 
	 * switch(1)- VIN1 select 
	 */
	if( sw1_val ){
		/* Camera selected */
		vin1_st = 1;
	}else{
		/* MMC-8bit selectd */
		vin1_st = 0;
		width=8;
		off = fdt_path_offset(fdt, "/soc/mmc@ee220000");
		fdt_delprop(fdt, off, "pinctrl-1");
		do_fixup_by_path_u32(fdt, "/soc/mmc@ee220000", "pinctrl-0", 0x12, 0);
		do_fixup_by_path_u32(fdt, "/soc/iwg21m_q7_common", "vin1-status", vin1_st, 0);
		do_fixup_by_path_u32(fdt, "/soc/mmc@ee220000", "bus-width", width, 1);
		/*Enabling Pull-ups for MMC1_D4-PUPR3_30, MMC1_D5-PUPR3_31, MMC1_D6-PUPR3_14, MMC1_D7-PUPR3_15*/ 
		val = readl(PUPR3);
		val |= PUPR3_MMC;
		writel(val, PUPR3);
	} 

	/* Update the VI2 or QSPI SPI flash selection
	 * vin2_sel(1) - set the the gpio for VIN2 select  
	 * vin2_sel(0) - reset the gpio for QSPI(flash) select 
	 */
	vin2_check = getenv("vin2_sel");

	gpio_request(GPIO_GP_0_18, NULL);
	gpio_direction_output(GPIO_GP_0_18, 0);

	if (simple_strtoul(vin2_check, NULL, 16)){
		gpio_set_value(GPIO_GP_0_18, 1);
		vin2_st = 1;
	}else{
		vin2_st = 0;
		do_fixup_by_path_u32(fdt, "/soc/iwg21m_q7_common", "vin2-status", vin2_st, 0);
		gpio_set_value(GPIO_GP_0_18, 0);
		do_fixup_by_path(fdt, "/soc/spi@e6b10000", "status", status_ok, 4,  0);
	}

	/* VIN2 Camera 8-bit or 16-bit selection GPIO */
	gpio_request(GPIO_GP_5_7, NULL);
	gpio_direction_input(GPIO_GP_5_7);

	vin_all = (vin0_st << 0) | (vin1_st << 1) | (vin2_st << 2) ;
	off = fdt_path_offset(fdt, "/soc/pfc@e6060000");
	switch(vin_all){
		case 0:	
			fdt_delprop(fdt, off, "pinctrl-1");
			fdt_delprop(fdt, off, "pinctrl-2");
			fdt_delprop(fdt, off, "pinctrl-3");
			break;
		case 1:
			fdt_delprop(fdt, off, "pinctrl-2");
			fdt_delprop(fdt, off, "pinctrl-3");
			break;
		case 2:
			do_fixup_by_path_u32(fdt, "/soc/pfc@e6060000", "pinctrl-1", 0x15, 0);
			fdt_delprop(fdt, off, "pinctrl-2");
			fdt_delprop(fdt, off, "pinctrl-3");
			break;
		case 3:
			fdt_delprop(fdt, off, "pinctrl-3");
			break;
		case 4:
			fdt_delprop(fdt, off, "pinctrl-2");
			fdt_delprop(fdt, off, "pinctrl-3");
			do_fixup_by_path_u32(fdt, "/soc/pfc@e6060000", "pinctrl-1", 0x16, 0);
			break;
		case 5:
			fdt_delprop(fdt, off, "pinctrl-3");
			do_fixup_by_path_u32(fdt, "/soc/pfc@e6060000", "pinctrl-2", 0x16, 0);
			break;
		case 6:
			fdt_delprop(fdt, off, "pinctrl-3");
			do_fixup_by_path_u32(fdt, "/soc/pfc@e6060000", "pinctrl-1", 0x15, 0);
			do_fixup_by_path_u32(fdt, "/soc/pfc@e6060000", "pinctrl-2", 0x16, 0);
			break;
		default:
			break;
	}
}
