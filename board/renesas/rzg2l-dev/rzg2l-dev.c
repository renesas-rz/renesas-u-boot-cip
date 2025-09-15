#include <common.h>
#include <cpu_func.h>
#include <image.h>
#include <init.h>
#include <malloc.h>
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
#include <asm/system.h>
#include <asm/ptrace.h>
#include <i2c.h>
#include <mmc.h>
#include <rzg2l_wdt.h>
#include <wdt.h>
#include <spi.h>
#include <spi-mem.h>
#include <linux/mtd/spi-nor.h>
#include "../rzg-common/common.h"
#include <efi_loader.h>

DECLARE_GLOBAL_DATA_PTR;

#define PFC_BASE	0x11030000

#define ETH_CH0		(PFC_BASE + 0x300c)
#define ETH_CH1		(PFC_BASE + 0x3010)
#define I2C_CH1 	(PFC_BASE + 0x1870)
#define ETH_PVDD_3300	0x00
#define ETH_PVDD_1800	0x01
#define ETH_PVDD_2500	0x02
#define ETH_MII_RGMII	(PFC_BASE + 0x3018)

/* CPG */
#define CPG_BASE					0x11010000
#define CPG_CLKON_BASE				(CPG_BASE + 0x500)
#define CPG_RESET_BASE				(CPG_BASE + 0x800)
#define CPG_RESET_ETH				(CPG_RESET_BASE + 0x7C)
#define CPG_RESET_I2C                           (CPG_RESET_BASE + 0x80)
#define CPG_RST_USB				(CPG_BASE + 0x878)
#define CPG_CLKON_USB				(CPG_BASE + 0x578)

/* PFC */
#define PFC_P37						(PFC_BASE + 0x037)
#define PFC_PM37					(PFC_BASE + 0x16E)
#define PFC_PMC37					(PFC_BASE + 0x237)
#define PFC_PWPR					(PFC_BASE + 0x3014)
#define PFC_PMC14					(PFC_BASE + 0x214)
#define PFC_PFC14					(PFC_BASE + 0x450)
#define PFC_PMC15					(PFC_BASE + 0x215)
#define PFC_PFC15					(PFC_BASE + 0x454)
#define PFC_PMC3A					(PFC_BASE + 0x23a)
#define PFC_PFC3A					(PFC_BASE + 0x4e8)

#define USBPHY_BASE		(0x11c40000)
#define USB0_BASE		(0x11c50000)
#define USB1_BASE		(0x11c70000)
#define USBF_BASE		(0x11c60000)
#define USBPHY_RESET		(USBPHY_BASE + 0x000u)
#define COMMCTRL		0x800
#define HcRhDescriptorA		0x048
#define LPSTS			0x102

#define RPC_CMNCR		0x10060000

#define NVCR_READ_CMD		0xB5
#define NVCR_WRITE_CMD		0xB1
#define WRITE_ENABLE_CMD	0x06
#define NVCR_LENGTH		2
#define NVCR_BIT4_MASK		~(1 << 4)

/* WDT */
#define WDT_INDEX		0

/* SYSC */
#define SYS_BASE			(0x11020000)
#define SYS_LSI_MODE			(SYS_BASE + 0xA00)
#define SYS_LSI_MODE_STAT_MD_BOOT_MASK	(0x7)
#define ESD_MODE			(0)
#if IS_ENABLED(CONFIG_EFI_HAVE_CAPSULE_SUPPORT)

#define EFI_FIRMWARE_IMAGE_TYPE_RZG2L_GUID\
    EFI_GUID(0x8b62f3c0, 0x707f, 0x4a65, 0x9b, 0x84, \
             0x1f, 0x0a, 0x4c, 0x12, 0xd7, 0xa6)

struct efi_fw_image fw_images[] = {
	{
		.image_type_id = EFI_FIRMWARE_IMAGE_TYPE_RZG2L_GUID,
		.fw_name = u"bl2_bp-smarc-rzg2l_pmic.bin",
		.image_index = 1,
	},
	{
		.image_type_id = EFI_FIRMWARE_IMAGE_TYPE_RZG2L_GUID,
		.fw_name = u"fip-smarc-rzg2l_pmic.bin",
		.image_index = 2,
	},
};

struct efi_capsule_update_info update_info = {
	.dfu_string =
		/* BL2 in SPI NOR at offset 0x0, max size 0x20000  */
		"sf 0:0=bl2_bp-smarc-rzg2l_pmic.bin raw 0x0 0x20000;"
		/* FIP in SPI NOR at offset 0x60000, max size 0x1F0000 (1984 KB) */
		"fip-smarc-rzg2l_pmic.bin raw 0x20000 0x1F0000",
	.num_images = ARRAY_SIZE(fw_images),
	.images = fw_images,
};

#endif /* EFI_HAVE_CAPSULE_SUPPORT */

/* ECC */
#define DDR_MEMC_BASE		(0x11410000)
#define ECC_ENABLE_ADDR		(0x0174)
#define ECC_ENABLE_MASK		GENMASK(25, 24)

void s_init(void)
{
	/* SD1 power control: P39_1 = 0; P39_2 = 1; */
	*(volatile u32 *)(PFC_PMC37) &= 0xFFFFFFF9; /* Port func mode 0b00 */
	*(volatile u32 *)(PFC_PM37) = (*(volatile u32 *)(PFC_PM37) & 0xFFFFFFC3) | 0x28; /* Port output mode 0b1010 */
#if CONFIG_TARGET_SMARC_RZG2L
	*(volatile u32 *)(PFC_P37) = (*(volatile u32 *)(PFC_P37) & 0xFFFFFFF9) | 0x6;	/* Port 39[2:1] output value 0b11*/
#else

	*(volatile u32 *)(PFC_P37) = (*(volatile u32 *)(PFC_P37) & 0xFFFFFFF9) | 0x4;	/* Port 39[2:1] output value 0b10*/
#endif

	/* can go in board_eht_init() once enabled */
	*(volatile u32 *)(ETH_CH0) = (*(volatile u32 *)(ETH_CH0) & 0xFFFFFFFC) | ETH_PVDD_1800;
	*(volatile u32 *)(ETH_CH1) = (*(volatile u32 *)(ETH_CH1) & 0xFFFFFFFC) | ETH_PVDD_1800;
	/* Enable RGMII for both ETH{0,1} */
	*(volatile u32 *)(ETH_MII_RGMII) = (*(volatile u32 *)(ETH_MII_RGMII) & 0xFFFFFFFC);
	/* ETH CLK */
	*(volatile u32 *)(CPG_RESET_ETH) = 0x30003;
	/* I2C CLK */
	*(volatile u32 *)(CPG_RESET_I2C) = 0xF000F;
	/* I2C pin non GPIO enable */
	*(volatile u32 *)(I2C_CH1) = 0x01010101;

	*(volatile u32 *)(RPC_CMNCR) = 0x01FFF300;
}

static void board_usb_init(void)
{
	/*Enable USB*/
	(*(volatile u32 *)CPG_RST_USB) = 0x000f000f;
	(*(volatile u32 *)CPG_CLKON_USB) = 0x000f000f;

	/* Setup  */
	/* Disable GPIO Write Protect */
	(*(volatile u32 *)PFC_PWPR) &= ~(0x1u << 7);	/* PWPR.BOWI = 0 */
	(*(volatile u32 *)PFC_PWPR) |= (0x1u << 6);	/* PWPR.PFCWE = 1 */

	/* set P4_0 as Func.1 for VBUSEN */
	(*(volatile u8 *)PFC_PMC14) |= (0x1u << 0);	/* PMC14.b0 = 1 */
	(*(volatile u8 *)PFC_PFC14) &= ~(0x7u << 0);	/* PFC14.PFC0 = 0 */
	(*(volatile u8 *)PFC_PFC14) |= (0x1u << 0);

	/* set P5_0 as Func.1 for OVERCUR */
	(*(volatile u8 *)PFC_PMC15) |= (0x1u << 0);	/* PMC15.b0 = 1 */
	(*(volatile u8 *)PFC_PFC15) &= ~(0x7u << 0);	/* PFC15.PFC0 = 0 */
	(*(volatile u8 *)PFC_PFC15) |= (0x1u << 0);

	/* set P42_0 as Func.1 for VBUSEN */
	(*(volatile u8 *)PFC_PMC3A) |= (0x1u << 0);	/* PMC14.b0 = 1 */
	(*(volatile u8 *)PFC_PFC3A) &= ~(0xfu << 0);	/* PFC15.PFC0 = 0 */
	(*(volatile u8 *)PFC_PFC3A) |= (0x1u << 0);

	/* set P42_1 as Func.1 for OVERCUR */
	(*(volatile u8 *)PFC_PMC3A) |= (0x1u << 0);	/* PMC14.b1 = 1 */
	(*(volatile u8 *)PFC_PFC3A) &= ~(0xfu << 4);	/* PFC15.PFC1 = 0 */
	(*(volatile u8 *)PFC_PFC3A) |= (0x1u << 4);

	/* Enable write protect */
	(*(volatile u32 *)PFC_PWPR) &= ~(0x1u << 6);	/* PWPR.PFCWE = 0 */
	(*(volatile u32 *)PFC_PWPR) |= (0x1u << 7);	/* PWPR.BOWI = 1 */

	/*Enable 2 USB ports*/
	(*(volatile u32 *)USBPHY_RESET) = 0x00001000u;
	/*USB0 is HOST*/
	(*(volatile u32 *)(USB0_BASE + COMMCTRL)) = 0;
	/*USB1 is HOST*/
	(*(volatile u32 *)(USB1_BASE + COMMCTRL)) = 0;
	/* Set USBPHY normal operation (Function only) */
	(*(volatile u16 *)(USBF_BASE + LPSTS)) |= (0x1u << 14);		/* USBPHY.SUSPM = 1 (func only) */
	/* Overcurrent is not supported */
	(*(volatile u32 *)(USB0_BASE + HcRhDescriptorA)) |= (0x1u << 12);	/* NOCP = 1 */
	(*(volatile u32 *)(USB1_BASE + HcRhDescriptorA)) |= (0x1u << 12);	/* NOCP = 1 */
}

static int board_spinor_op_nvcr_setup(void)
{
	struct spi_slave *spi;
	struct spi_nor *nor;
	int ret;
	u8 nvcr[2];

	/* Initialize SPI */
	spi = spi_setup_slave(0, 0, 1000000, SPI_MODE_0);
	if (!spi) {
		printf("Failed to set up SPI slave\n");
		return -1;
	}

	ret = spi_claim_bus(spi);
	if (ret) {
		printf("Failed to claim SPI bus\n");
		spi_free_slave(spi);
		return ret;
	}

	nor = dev_get_uclass_priv(spi->dev);
	if (!nor) {
		printf("Failed to get SPI NOR\n");
		ret = -ENODEV;
		goto release_bus;
	}

	ret = nor->read_reg(nor, NVCR_READ_CMD, nvcr, 2);
	if (ret) {
		printf("Failed to send NVCR Read command: %d\n", ret);
		goto release_bus;
	}

	/* Clear bit 4 of the NVCR - RESET# on DQ3 */
	nvcr[0] &= NVCR_BIT4_MASK;

	/* Write enable */
	ret = nor->write_reg(nor, SPINOR_OP_WREN, NULL, 0);
	if (ret) {
		printf("Failed to send Write Enable command: %d\n", ret);
		goto release_bus;
	}

	/* Write NVCR */
	nor->cmd_buf[0] = nvcr[0];
	nor->cmd_buf[1] = nvcr[1];
	ret = nor->write_reg(nor, NVCR_WRITE_CMD, nor->cmd_buf, 2);
	if (ret) {
		printf("Failed to send NVCR Write command: %d\n", ret);
		goto release_bus;
	}

release_bus:
	spi_release_bus(spi);
	spi_free_slave(spi);
	return ret;
}

int board_early_init_f(void)
{

	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_TEXT_BASE + 0x50000;
	board_spinor_op_nvcr_setup();
	board_usb_init();

	return 0;
}

void reset_cpu(void)
{
#ifdef CONFIG_RENESAS_RZG2LWDT
	struct udevice *wdt_dev;
	if (uclass_get_device(UCLASS_WDT, WDT_INDEX, &wdt_dev) < 0) {
		printf("failed to get wdt device. cannot reset\n");
		return;
	}
	if (wdt_expire_now(wdt_dev, 0) < 0) {
		printf("failed to expire_now wdt\n");
	}
#endif // CONFIG_RENESAS_RZG2LWDT
}

int board_late_init(void)
{
#ifdef CONFIG_RENESAS_RZG2LWDT
	rzg2l_reinitr_wdt();
#endif // CONFIG_RENESAS_RZG2LWDT

	return 0;
}

static const char * const rzg2l_dt_esd_mode[] = {
	"/soc/mmc@11c00000", "vmmc-supply", "<&/regulator-vcc-sdhi0>",
	"/soc/mmc@11c00000", "vqmmc-supply", "<&/regulator-vccq-sdhi0>",
};

int ft_verify_fdt(void *fdt)
{
	const char **fdt_dt = NULL;
	int size = 0;
	u32 boot_mode = readl(SYS_LSI_MODE);

	switch (boot_mode & SYS_LSI_MODE_STAT_MD_BOOT_MASK) {
	case ESD_MODE:
	{
		fdt_dt = (const char **)rzg2l_dt_esd_mode;
		size = ARRAY_SIZE(rzg2l_dt_esd_mode);
		break;
	}
	default:
		return 1;
	}

	return update_fdt(fdt, fdt_dt, size);
};

#if defined(CONFIG_MULTI_DTB_FIT)
int board_fit_config_name_match(const char *name)
{
	u32 ecc;

	ecc = readl(DDR_MEMC_BASE + ECC_ENABLE_ADDR);
	ecc &= ECC_ENABLE_MASK;

	if (!strcmp(name, "smarc-rzg2l-ecc") && ecc)
		return 0;

	if (!strcmp(name, "smarc-rzg2l") && !(ecc))
		return 0;

	return -1;
}
#endif
