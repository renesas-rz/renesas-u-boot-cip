// SPDX-License-Identifier: GPL-2.0+
/*
 * board/renesas/hihope-rzg2n/hihope-rzg2n.c
 *     This file is HiHope RZ/G2N board support.
 *
 * Copyright (C) 2015-2019 Renesas Electronics Corporation
 * Copyright (C) 2015 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 */

#include <common.h>
#include <env.h>
#include <malloc.h>
#include <netdev.h>
#include <dm.h>
#include <dm/platform_data/serial_sh.h>
#include <asm/processor.h>
#include <asm/mach-types.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/sys_proto.h>
#include <asm/system.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/arch/rmobile.h>
#include <asm/arch/rcar-mstp.h>
#include <asm/arch/sh_sdhi.h>
#include <i2c.h>
#include <mmc.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <asm/arch/rcar-mstp.h>
#include <renesas_wdt.h>

#include "../rzg-common/common.h"

DECLARE_GLOBAL_DATA_PTR;

static int board_rev;

void s_init(void)
{
}

#define GPIO_WLAN_REG_ON 157
#define GPIO_BT_REG_ON 158
#define	GPIO_BT_POWER		73	/* GP3_13	*/
#define	GPIO_WIFI_POWER		82	/* GP4_06	*/

void clear_wlan_bt_reg_on(void)
{
	if (board_rev > 2)
	{
		gpio_request(GPIO_BT_POWER, "bt_power");
		gpio_request(GPIO_WIFI_POWER, "wifi_power");
		gpio_direction_output(GPIO_BT_POWER, 0);
		gpio_direction_output(GPIO_WIFI_POWER, 0);
	}
	else
	{
		gpio_request(GPIO_WLAN_REG_ON, "wlan_reg_on");
		gpio_request(GPIO_BT_REG_ON, "bt_reg_on");
		gpio_direction_output(GPIO_WLAN_REG_ON, 0);
		gpio_direction_output(GPIO_BT_REG_ON, 0);
	}
}

#define SCIF2_MSTP310		BIT(10)	/* SCIF2 */
#define DVFS_MSTP926		BIT(26)
#define GPIO2_MSTP910		BIT(10)
#define GPIO3_MSTP909		BIT(9)
#define GPIO5_MSTP907		BIT(7)
#define HSUSB_MSTP704		BIT(4)	/* HSUSB */

int board_early_init_f(void)
{
#if defined(CONFIG_SYS_I2C) && defined(CONFIG_SYS_I2C_SH)
	/* DVFS for reset */
	mstp_clrbits_le32(SMSTPCR9, SMSTPCR9, DVFS_MSTP926);
#endif
	return 0;
}

/* HSUSB block registers */
#define HSUSB_REG_LPSTS			0xE6590102
#define HSUSB_REG_LPSTS_SUSPM_NORMAL	BIT(14)
#define HSUSB_REG_UGCTRL2		0xE6590184
#define HSUSB_REG_UGCTRL2_USB0SEL	0x30
#define HSUSB_REG_UGCTRL2_USB0SEL_EHCI	0x10
#define HSUSB_REG_UGCTRL2_RESERVED_3	0x00000001 /* bit[3:0] must be B'0001 */

#define	GPIO_REV_BIT1		113	/* GP5_19	*/
#define	GPIO_REV_BIT0		115	/* GP5_21	*/
#define	GPIO_REV2_BOARD_CHECK	119	/* GP5_25	*/

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_TEXT_BASE + 0x50000;

	/* USB1 pull-up */
	setbits_le32(PFC_PUEN6, PUEN_USB1_OVC | PUEN_USB1_PWEN);

	/* Configure the HSUSB block */
	mstp_clrbits_le32(SMSTPCR7, SMSTPCR7, HSUSB_MSTP704);
	udelay(2);
	/* Choice USB0SEL */
	writel(HSUSB_REG_UGCTRL2_USB0SEL_EHCI | HSUSB_REG_UGCTRL2_RESERVED_3,
	       HSUSB_REG_UGCTRL2);
	/* low power status */
	setbits_le16(HSUSB_REG_LPSTS, HSUSB_REG_LPSTS_SUSPM_NORMAL);

	gpio_request(GPIO_REV2_BOARD_CHECK, "rev2_check");
	gpio_direction_input(GPIO_REV2_BOARD_CHECK);
	if (gpio_get_value(GPIO_REV2_BOARD_CHECK))
	{
		board_rev = 2;
	}
	else
	{
		gpio_request(GPIO_REV_BIT0, "rev_bit0");
		gpio_request(GPIO_REV_BIT1, "rev_bit1");
		gpio_direction_input(GPIO_REV_BIT1);
		gpio_direction_input(GPIO_REV_BIT0);
		board_rev = 0x03 + ((gpio_get_value(GPIO_REV_BIT1) << 1)  | gpio_get_value(GPIO_REV_BIT0));
	}
	clear_wlan_bt_reg_on();

	return 0;
}

int board_late_init(void)
{
#ifdef CONFIG_WDT_RENESAS
	reinitr_wdt();
#endif
	env_set_hex("board_rev", board_rev);

	return 0;
}

#define RST_BASE	0xE6160000
#define RST_CA57RESCNT	(RST_BASE + 0x40)
#define RST_RSTOUTCR	(RST_BASE + 0x58)
#define RST_CODE	0xA5A5000F

void reset_cpu(ulong addr)
{
#if defined(CONFIG_SYS_I2C) && defined(CONFIG_SYS_I2C_SH)
	i2c_reg_write(CONFIG_SYS_I2C_POWERIC_ADDR, 0x20, 0x80);
#else
	writel(RST_CODE, RST_CA57RESCNT);
#endif
}

void board_add_ram_info(int use_default)
{
	int i;

	printf("\n");
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		if (!gd->bd->bi_dram[i].size)
			break;
		printf("Bank #%d: 0x%09llx - 0x%09llx, ", i,
		       (unsigned long long)(gd->bd->bi_dram[i].start),
		       (unsigned long long)(gd->bd->bi_dram[i].start
		       + gd->bd->bi_dram[i].size - 1));
		print_size(gd->bd->bi_dram[i].size, "\n");
	};
}

void board_cleanup_before_linux(void)
{
	/*
	 * Turn off the clock that was turned on outside
	 * the control of the driver
	 */
	/* Configure the HSUSB block */
	mstp_setbits_le32(SMSTPCR7, SMSTPCR7, HSUSB_MSTP704);

	/*
	 * Because of the control order dependency,
	 * turn off a specific clock at this timing
	 */
	mstp_setbits_le32(SMSTPCR9, SMSTPCR9,
			  GPIO2_MSTP910 | GPIO3_MSTP909 | GPIO5_MSTP907);
}

static const char * const dt_non_ecc[] = {
	"/memory@480000000", "reg", "<0x4 0x80000000 0x0 0x80000000>",
	"/reserved-memory/linux,lossy_decompress", "reg",
					"<0x0 0x54000000 0x0 0x3000000>",
	"/reserved-memory/linux,lossy_decompress", "no-map", NULL,
	"/reserved-memory/linux,cma", "reg", "<0x0 0x58000000 0x0 0x20000000>",
	"/reserved-memory/linux,multimedia", "reg",
					     "<0x0 0x78000000 0x0 0x10000000>",
	"/mmngr", "memory-region", "<&/reserved-memory/linux,multimedia \
				     &/reserved-memory/linux,lossy_decompress>",
};

static const char * const dt_ecc_full_single[] = {
	"/memory@480000000", "reg", "<0x4 0x80000000 0x0 0x14000000>",
	"/reserved-memory/linux,lossy_decompress", NULL, NULL,
	"/reserved-memory/linux,cma", "reg", "<0x0 0x50000000 0x0 0x20000000>",
	"/reserved-memory/linux,multimedia", "reg",
					     "<0x0 0x70000000 0x0 0x10000000>",
	"/mmngr", "memory-region", "<&/reserved-memory/linux,multimedia>",
};

int ft_verify_fdt(void *fdt)
{
	const char **fdt_dt = NULL;
	int use_ecc, ecc_mode, size;
	struct pt_regs regs;

	size = 0;
	/* Setting SiP Service GET_ECC_MODE command*/
	regs.regs[0] = RZG_SIP_SVC_GET_ECC_MODE;
	smc_call(&regs);
	/* First result is USE ECC or not, Second result is ECC MODE*/
	use_ecc = regs.regs[0];
	ecc_mode = regs.regs[1];

	if (!use_ecc) {
		fdt_dt = (const char **)dt_non_ecc;
		size = ARRAY_SIZE(dt_non_ecc);
	} else if (use_ecc == 1) {
		switch (ecc_mode) {
		case 0:
			/* The memory map of partial ECC same as non-ECC mode*/
			fdt_dt = (const char **)dt_non_ecc;
			size = ARRAY_SIZE(dt_non_ecc);
			break;
		case 2:
			fdt_dt = (const char **)dt_ecc_full_single;
			size = ARRAY_SIZE(dt_ecc_full_single);
			break;
		default:
			printf("Not support changing device-tree to ");
			printf("compatible with ECC_MODE = %d\n", ecc_mode);
			return 1;
		};
	} else {
		return 1;
	}

	return update_fdt(fdt, fdt_dt, size);
}
