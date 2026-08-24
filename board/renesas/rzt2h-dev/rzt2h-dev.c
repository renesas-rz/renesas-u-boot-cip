#include <common.h>
#include <cpu_func.h>
#include <image.h>
#include <init.h>
#include <malloc.h>
#include <miiphy.h>
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
#include <i2c.h>
#include <mmc.h>
#include <linux/delay.h>
#include <efi_loader.h>
#include <linux/libfdt.h>
#include <linux/arm-smccc.h>
#include <stdio.h>
#include <stdint.h>

DECLARE_GLOBAL_DATA_PTR;

#define SCKCR2			0x81280004
#define SCKCR2_CA55SCLK		BIT(12)
#define MSTPCRM			0x80280330
#define MSTPCRM_SDHI0		BIT(12)
#define MSTPCRM_SDHI1		BIT(13)
#define PRCRN			0x80294200
#define PRCRN_PRKEY		(0xa5 << 8)
#define PRCRN_WR_EN		0xF
#define PRCRS                   0x81296000
#define PRCRS_PRKEY             (0xa5 << 8)
#define PRCRS_WR_EN             0xF
#define PMC(x)			((x < 13) ? (0x812C0400 + (x)):(0x802C0400 + (x)))
#define PM(x)			((x < 13) ? (0x812C0200 + (x) * 2):(0x802C0200 + (x) * 2))
#define P(x)			((x < 13) ? (0x812C0000 + (x)):(0x802C0000 + (x)))
#define PFC(x)			((x < 13) ? (0x812C0600 + (x) * 8):(0x802C0600 + (x) * 8))

#define MSTPCRE			0x80280310
#define MSTPCRE_USB			BIT(8)

#define ADXCTL0_BASE		0x81290100
#define DDRMIR0_MASK		GENMASK(20, 16)
#define DDRMIR0(x)		(x << 16)
#define ADXC0_MASTERS		6

#define ADXCTL1_BASE		0x81291100
#define DDRMIR1_MASK		GENMASK(28, 24)
#define DDRMIR1(x)		(x << 24)
#define ADXC1_MASTERS		7

#define DDRMIR_MASK		GENMASK(27,  0)
#define addr_shift		0x2

/* USB2.0 Host registers */
#define USB2_BASE		(0x92040000)
#define USBF_BASE		(0x92041000)
#define USB2_INT_ENABLE		0x200
#define USB2_COMMCTRL		0x800
#define USB2_VBCTRL		0x80c
#define USB2_LINECTRL1		0x810
#define USB2_PHYCTRL		0x830
#define USB2_USBCTR		0x20c
#define USB2_UTMI_CTRL		0x318
#define USB2_OCSLPTIMSET	0x310
#define USB2_OBINTSTA		0x804
#define USB2_OBINTEN		0x808
#define USB2_HcRhDescriptorA	0x048
#define USBf_LPSTS		0x102
#define USB2_SPD_RSM_TIMSET	0x30c

/* INT_ENABLE */
#define USB2_INT_ENABLE_UCOM_INTEN	BIT(3)
#define USB2_INT_ENABLE_USBH_INTB_EN	BIT(2) /* For EHCI */
#define USB2_INT_ENABLE_USBH_INTA_EN	BIT(1) /* For OHCI */

/* USBCTR */
#define USB2_USBCTR_DIRPD	BIT(2)
#define USB2_USBCTR_PLL_RST	BIT(1)

/* SPD_RSM_TIMSET */
#define USB2_SPD_RSM_TIMSET_INIT	0x014e029b

/* OC_TIMSET */
#define USB2_OCSLPTIMSET_INIT	0x000209ab

/* COMMCTRL */
#define USB2_COMMCTRL_OTG_PERI	BIT(31) /* 1 = Peripheral mode */

/* OBINTSTA and OBINTEN */
#define USB2_OBINT_SESSVLDCHG	BIT(12)
#define USB2_OBINT_IDDIGCHG	BIT(11)
#define USB2_OBINT_VBSTAINT	BIT(3)
#define USB2_OBINT_IDCHG	BIT(0)
#define USB2_OBINT_BITS		(USB2_OBINT_SESSVLDCHG | \
				USB2_OBINT_IDDIGCHG)
#define USB2_OBINT_BITS_T2H	(USB2_OBINT_IDCHG | \
				USB2_OBINT_VBSTAINT)

/* VBCTRL */
#define USB2_VBCTRL_VBSTA	BIT(29)
#define USB2_VBCTRL_VBLVL	BIT(21)
#define USB2_VBCTRL_OCCLREN	BIT(16)
#define USB2_VBCTRL_DRVVBUSSEL	BIT(8)
#define USB2_VBCTRL_SIDDQREL	BIT(2)
#define USB2_VBCTRL_VBOUT	BIT(0)

/* LINECTRL1 */
#define USB2_LINECTRL1_DPRPD_EN	BIT(19)
#define USB2_LINECTRL1_DP_RPD	BIT(18)
#define USB2_LINECTRL1_DMRPD_EN	BIT(17)
#define USB2_LINECTRL1_DM_RPD	BIT(16)
#define USB2_LINECTRL1_OPMODE_NODRV	BIT(6)

/* PHYCTRL */
#define USB2_PHYCTRL_OTGSESSVLD	BIT(20)
#define USB2_PHYCTRL_IDDIG	BIT(19)
#define USB2_PHYCTRL_VBUSVALID	BIT(18)
#define USB2_PHYCTRL_IDPULLUP	BIT(5)  /* 1 = ID sampling is enabled */
#define USB2_PHYCTRL_DRVVBUS	BIT(4)

#define MODCTRL			0x8

#define CS0ENDAD_xSPI(x)	(0x80293004 + (0x100 * (x)))

#if IS_ENABLED(CONFIG_EFI_HAVE_CAPSULE_SUPPORT)

#define EFI_FIRMWARE_IMAGE_TYPE_RZT2H_GUID \
    EFI_GUID(0xe8a4b1d6, 0x2e3d, 0x4f4b, 0x9b, 0x0a, 0x9a, 0x6a, 0x68, 0xcd, 0x93, 0xac)

struct efi_fw_image fw_images[] = {
	{
		.image_type_id = EFI_FIRMWARE_IMAGE_TYPE_RZT2H_GUID,
		.fw_name = u"bl2_bp_xspi0.bin",
		.image_index = 1,
	},
	{
		.image_type_id = EFI_FIRMWARE_IMAGE_TYPE_RZT2H_GUID,
		.fw_name = u"fip.bin",
		.image_index = 2,
	},
};

struct efi_capsule_update_info update_info = {
	.dfu_string =
		/* BL2 in SPI NOR at offset 0x0, max size 0x60000 (384 KB) */
		"sf 0:0=bl2_bp_xspi0.bin raw 0x0 0x60000;"
		/* FIP in SPI NOR at offset 0x60000, max size 0x1F0000 (1984 KB) */
		"fip.bin raw 0x60000 0x1F0000",
	.num_images = ARRAY_SIZE(fw_images),
	.images = fw_images,
};

#endif /* EFI_HAVE_CAPSULE_SUPPORT */


void s_init(void)
{
	/* Disable Write protect to enable writing */
	*(volatile u32 *)PRCRN = PRCRN_PRKEY | PRCRN_WR_EN;
	*(volatile u32 *)PRCRS = PRCRS_PRKEY | PRCRS_WR_EN;

	/* Setting xSPI0 CS0 End Address
	Octal Flash MX25LW51245GXDI00-T 64 MiB range */
	*(volatile u32 *)CS0ENDAD_xSPI(0) = 0x43FFFFFF;

	/* Setting xSPI1 CS0 End Address */
	*(volatile u32 *)CS0ENDAD_xSPI(1) = 0x57FFFFFF;

	/* P8_6_SD1_IOVS_A Output LOW */
	*(volatile u8 *)PMC(8) &= ~BIT(6);
	*(volatile u8 *)P(8) &= ~BIT(6);
	*(volatile u16 *)PM(8) |= (0x3 << 12);

	/* Set Coretex-A55 DSU clock to 1GHz */
	*(volatile u32 *)SCKCR2 |= SCKCR2_CA55SCLK;

	/* Enable Write protect to disable writing */
	*(volatile u32 *)PRCRN = PRCRN_PRKEY;
	*(volatile u32 *)PRCRS = PRCRS_PRKEY;
}

static void board_usb_init(void)
{
	u32 dummy;
	int i;

	/* Disable Write protect to enable writing */
	*(volatile u32 *)PRCRN = PRCRN_PRKEY | PRCRN_WR_EN;
	*(volatile u32 *)PRCRS = PRCRS_PRKEY | PRCRS_WR_EN;

	const u32 cpu_type = renesas_get_cpu_type();
	if (cpu_type == RENESAS_CPU_TYPE_RZT2H) {
		/* set P00_0 operation as USB_VBUSEN*/
		*(volatile u64 *)PFC(0) = (*(volatile u64 *)PFC(0) & 0x3f) | 0x13;
		*(volatile u8 *)PMC(0) |= BIT(0);
	}
	else if (cpu_type == RENESAS_CPU_TYPE_RZN2H) {
		/* set P02_2 operation as USB_VBUSEN*/
		*(volatile u64 *)PFC(2)	= (*(volatile u64 *)PFC(2) & 0xFFFFFFFFFF00FFFF) | (0x13 << 16);
		*(volatile u8 *)PMC(2)	|= BIT(2);
	}
	else
	{
		printf("Error Unknown CPU type %d \n",cpu_type);
	}

	/* USB module stop release */
	*(volatile u32 *)MSTPCRE &= ~(MSTPCRE_USB);
	udelay(1);
	for(i=0;i<7;i++)
	{
		dummy = *(volatile u32 *)(USB2_BASE + USB2_PHYCTRL);
	}
	/* Enable Write protect to disable writing*/
	(*(volatile u32 *)PRCRN) = PRCRN_PRKEY;
	(*(volatile u32 *)PRCRS) = PRCRS_PRKEY;

	/* Disable interrupt */
	*(volatile u32 *)(USB2_BASE + USB2_INT_ENABLE) = 0;

	/* enable pull down resisor */
	*(volatile u32 *)(USB2_BASE + USB2_LINECTRL1) |= USB2_LINECTRL1_DP_RPD | USB2_LINECTRL1_DPRPD_EN | USB2_LINECTRL1_DMRPD_EN | USB2_LINECTRL1_DM_RPD;

	/* enable VBUS valid comparator */
	*(volatile u32 *)(USB2_BASE + USB2_PHYCTRL) |= USB2_PHYCTRL_DRVVBUS;

	/* usb is HOST */
	*(volatile u32 *)(USB2_BASE + USB2_COMMCTRL) &= ~USB2_COMMCTRL_OTG_PERI;

	/* UTMI normal mode */
	(*(volatile u16 *)(USBF_BASE + USBf_LPSTS)) |= 0x4000;

	/* SIDDQ mode release and enable VBUS output */
	*(volatile u32 *)(USB2_BASE + USB2_VBCTRL) |= USB2_VBCTRL_SIDDQREL | USB2_VBCTRL_VBOUT;

	/* Overcurrent is not supported */
	(*(volatile u32 *)(USB2_BASE + USB2_HcRhDescriptorA)) |= (0x1u << 12);

	/* Release PLL reset */
	(*(volatile u32 *)(USB2_BASE + USB2_USBCTR)) |= USB2_USBCTR_PLL_RST;
	(*(volatile u32 *)(USB2_BASE + USB2_USBCTR)) &= ~USB2_USBCTR_PLL_RST;

	udelay(10);
}

int board_early_init_f(void)
{
	return 0;
}

/* Setting for ADXCTLn registers */
int adxctl_init(void)
{
	int i;

	for (i = 2; i < ADXC0_MASTERS; i++)
		writel(((readl((uintptr_t)(ADXCTL0_BASE + i * 0x4)) & ~DDRMIR_MASK) |
				DDRMIR1(addr_shift)), (uintptr_t)(ADXCTL0_BASE + i * 0x4));

	for (i = 0; i < ADXC1_MASTERS; i++)
		writel(((readl((uintptr_t)(ADXCTL1_BASE + i * 0x4)) & ~DDRMIR_MASK) |
			DDRMIR1(addr_shift)), (uintptr_t)(ADXCTL1_BASE + i * 0x4));
	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_TEXT_BASE + 0x50000;
	adxctl_init();

	board_usb_init();

	return 0;
}

int board_fit_config_name_match(const char *name)
{
	const u32 cpu_type = renesas_get_cpu_type();
#if (defined CONFIG_PLAT_ECC_ENABLE)
	if (cpu_type == RENESAS_CPU_TYPE_RZT2H && (strcmp(name, "rzt2h-dev-ecc") == 0))
		return 0;
	else if (cpu_type == RENESAS_CPU_TYPE_RZN2H && (strcmp(name, "rzn2h-dev-ecc") == 0))
		return 0;   // N2H
#else
	if (cpu_type == RENESAS_CPU_TYPE_RZT2H && (strcmp(name, "rzt2h-dev") == 0))
		return 0;
	else if (cpu_type == RENESAS_CPU_TYPE_RZN2H && (strcmp(name, "rzn2h-dev") == 0))
		return 0;   // N2H
#endif
    return -1;
}


void reset_cpu(void)
{

}

int board_late_init(void)
{
	return 0;
}

/*
 * Reallocate gd to inside DDR area to avoid using it from
 * inaccessible outside DDR mem.
 */
int mach_cpu_init(void)
{
	volatile gd_t *pgd;

	pgd = malloc(sizeof(*pgd));
	if (!pgd) {
		printf("Out of memory for pgd size 0x%lx\n", sizeof(*pgd));
		return -ENOMEM;
	}

	memset((void *)pgd, 0, sizeof(*pgd));
	pgd = gd;
	set_gd(pgd);

	return 0;
}
