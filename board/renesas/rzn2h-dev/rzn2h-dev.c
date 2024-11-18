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
#include <asm/arch/rmobile.h>
#include <asm/arch/rcar-mstp.h>
#include <asm/arch/sh_sdhi.h>
#include <i2c.h>
#include <mmc.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

#define SCKCR2			0x81280004
#define SCKCR2_CA55SCLK		BIT(12)
#define MSTPCRM                 0x80280330
#define MSTPCRM_SDHI0           BIT(12)
#define MSTPCRM_SDHI1           BIT(13)
#define PRCRN                   0x80294200
#define PRCRN_PRKEY             (0xa5 << 8)
#define PRCRN_WR_EN             0xF
#define PRCRS                   0x81296000
#define PRCRS_PRKEY             (0xa5 << 8)
#define PRCRS_WR_EN             0xF
#define PMC(x)			((x < 13) ? (0x812C0400 + (x)):(0x802C0400 + (x)))
#define PM(x)			((x < 13) ? (0x812C0200 + (x) * 2):(0x802C0200 + (x) * 2))
#define P(x)			((x < 13) ? (0x812C0000 + (x)):(0x802C0000 + (x)))
#define PFC(x)			((x < 13) ? (0x812C0600 + (x) * 8):(0x802C0600 + (x) * 8))

#define SCKCR			0x80280000
#define PHYSEL			BIT(21)

#define MSTPCRE			0x80280310
#define MSTPCRE_GMAC1		BIT(16)
#define MSTPCRE_GMAC2		BIT(17)
#define MSTPCRE_ETHSS		BIT(3)

#define MRCTLE			0x80280250
#define MRCTLE_GMAC1_PCLKH	BIT(16)
#define MRCTLE_GMAC1_PCLKM	BIT(17)
#define MRCTLE_GMAC2_PCLKH	BIT(18)
#define MRCTLE_GMAC2_PCLKM	BIT(19)
#define MRCTLE_ETHSS		BIT(5)
#define MRCTLE_MII_CONV		BIT(6)

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

/* USB 2.0 Host registers */
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

#define CS0ENDAD_xSPI(x)       (0x80293004 + (0x100 * (x)))

void s_init(void)
{
	/* Disable Write protect to enable writing */
        *(volatile u32 *)PRCRN = PRCRN_PRKEY | PRCRN_WR_EN;
	*(volatile u32 *)PRCRS = PRCRS_PRKEY | PRCRS_WR_EN;

        /* Setting xSPI1 CS0 End Address */
        *(volatile u32 *)CS0ENDAD_xSPI(1) = 0x57FFFFFF;

	/* P2_5_SD0_PWEN_A Output HIGH */
	*(volatile u8 *)PMC(2) |= BIT(5);
	*(volatile u64 *)PFC(2) = (*(volatile u64 *)PFC(2) & 0xffff00ffffffffff) | ((u64)0x29 << 40);

	/* P2_6_SD0_IOVS_A Output LOW */
	*(volatile u8 *)PMC(2) |= BIT(6);
	*(volatile u64 *)PFC(8) = (*(volatile u64 *)PFC(8) & 0xffff00ffffffffff) | ((u64)0x29 << 40);

	/* P08_5_SD1_PWEN_A Output HIGH */
	*(volatile u8 *)PMC(8) &= ~BIT(5);
	*(volatile u8 *)P(8) |= BIT(5);
	*(volatile u16 *)PM(8) |= (0x3 << 10);

	/* P08_6_SD1_IOVS_A Output LOW */
	*(volatile u8 *)PMC(8) &= ~BIT(6);
	*(volatile u8 *)P(8) &= ~BIT(6);
	*(volatile u16 *)PM(8) |= (0x3 << 12);

	/* Release module stop for SDHI0/1 */
	*(volatile u32 *)MSTPCRM &= ~(MSTPCRM_SDHI0 | MSTPCRM_SDHI1);

	/* Release PHY3 Reset state  P03_2 */
	*(volatile u8 *)PMC(3) &= ~BIT(2);
	*(volatile u8 *)P(3) |= BIT(2);
	*(volatile u16 *)PM(3) |= (0x3 << 4);

	/* ETH3 PIN */
	/* ETH3_TXCLK, ETH3_TXD[0], ETH3_TXD[1], ETH3_TXD[2], ETH3_TXD[3], ETH3_TXEN, ETH3_RXCLK, ETH3_RXD[0],
	ETH3_RXD[1], ETH3_RXD[2], ETH3_RXD[3], ETH3_RXDV, ETH3_TXER, ETH3_RXER, ETH3_CRS, ETH3_COL */
	*(volatile u8 *)PMC(33)		|= BIT(7)|BIT(6)|BIT(5)|BIT(4)|BIT(3)|BIT(2);
	*(volatile u64 *)PFC(33)	= (*(volatile u64 *)PFC(33) & 0x000000000000FFFF)
					| ((u64)0xf << 56) | ((u64)0xf << 48) | ((u64)0xf << 40) | ((u64)0xf << 32) | (0xf << 24) | (0xf << 16);

	*(volatile u8 *)PMC(34)		|= BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0);
	*(volatile u64 *)PFC(34)	= (*(volatile u64 *)PFC(34) & 0x00ff000000000000)
					| ((u64)0xf << 40) | ((u64)0xf << 32) | (0xf << 24) | (0xf << 16) | (0xf << 8) | (0xf << 0);

	*(volatile u8 *)PMC(0)		|= BIT(3)|BIT(2)|BIT(1)|BIT(0);
	*(volatile u64 *)PFC(0)		= (*(volatile u64 *)PFC(0) & 0xffffffffff000000)
					|(0xf << 24) | (0xf << 16) | (0xf << 8) | (0xf << 0);

	/* GMAC1_MDC, GMAC1_MDIO */
	*(volatile u8 *)PMC(26)		|= BIT(2)|BIT(1);
	*(volatile u64 *)PFC(26)	= (*(volatile u64 *)PFC(26) & ~(0x3f << 16) & ~(0x3f << 8))
					| (0x10 << 16) | (0x10 << 8);

	/* ETH3_REFCLK */
	*(volatile u8 *)PMC(34)		|= BIT(6);
	*(volatile u64 *)PFC(34)	= (*(volatile u64 *)PFC(34) & ~((u64)0x3f << 48)) | ((u64)0x02 << 48);

	/* GMAC CLK */
	/* Release module stop for GMAC1 */
	*(volatile u32 *)MSTPCRE &= ~(MSTPCRE_GMAC1);

	/* Choose PLL1 for ETH_REFCLK */
	*(volatile u32 *)SCKCR &= ~(PHYSEL);

	/* Release module reset for GMAC1 */
	*(volatile u32 *)MRCTLE &= ~(MRCTLE_GMAC1_PCLKH | MRCTLE_GMAC1_PCLKM);

	/* Release PHY2 Reset state  P03_1 */
	*(volatile u8 *)PMC(3) &= ~BIT(1);
	*(volatile u8 *)P(3) |= BIT(1);
	*(volatile u16 *)PM(3) |= (0x3 << 2);

	/* ETH2 PIN */
	/* ETH2_TXCLK, ETH2_TXD[0], ETH2_TXD[1], ETH2_TXD[2], ETH2_TXD[3], ETH2_TXEN, ETH2_RXCLK, ETH2_RXD[0],
	ETH2_RXD[1], ETH2_RXD[2], ETH2_RXD[3], ETH2_RXDV, ETH2_RXER, ETH2_TXER, ETH2_CRS, ETH2_COL */
	*(volatile u8 *)PMC(29)		|= BIT(7)|BIT(6)|BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1);
	*(volatile u64 *)PFC(29)	= (*(volatile u64 *)PFC(29) & 0x00000000000000ff)
					| ((u64)0xf << 56) | ((u64)0xf << 48) | ((u64)0xf << 40) | ((u64)0xf << 32) | (0xf << 24) | (0xf << 16) | (0xf << 8);

	*(volatile u8 *)PMC(30)		|= BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0);
	*(volatile u64 *)PFC(30)	= (*(volatile u64 *)PFC(30) & 0xffffff0000000000)
					| ((u64)0xf << 32) | (0xf << 24) | (0xf << 16) | (0xf << 8) | (0xf << 0);

	*(volatile u8 *)PMC(31)		|= BIT(5)|BIT(4)|BIT(2)|BIT(1);
	*(volatile u64 *)PFC(31)	= (*(volatile u64 *)PFC(31) & 0xffff0000ff0000ff)
					| ((u64)0xf << 40) | ((u64)0xf << 32) | (0xf << 16) | (0xf << 8);

	/* GMAC2_MDC, GMAC2_MDIO */
	*(volatile u8 *)PMC(30)		|= BIT(6)|BIT(5);
	*(volatile u64 *)PFC(30)	= (*(volatile u64 *)PFC(30) & ~((u64)0x3f << 48) & ~((u64)0x3f << 40))
					| ((u64)0x10 << 48) | ((u64)0x10 << 40);

	/* ETH2_REFCLK */
	*(volatile u8 *)PMC(31)		|= BIT(0);
	*(volatile u64 *)PFC(31)	= (*(volatile u64 *)PFC(31) & ~(0x3f << 0)) | (0x02 << 0);

	/* GMAC2 CLK */
	/* Release module stop for GMAC2 */
	*(volatile u32 *)MSTPCRE &= ~(MSTPCRE_GMAC2);

	/* Release module reset for GMAC2 */
	*(volatile u32 *)MRCTLE &= ~(MRCTLE_GMAC2_PCLKH | MRCTLE_GMAC2_PCLKM);

	/* ETH SS CLK */
	/* Release module stop for ETH_SS */
	*(volatile u32 *)MSTPCRE &= ~(MSTPCRE_ETHSS);

	/* Release module reset for ETH_SS, MII_CONV  */
	*(volatile u32 *)MRCTLE &= ~(MRCTLE_ETHSS | MRCTLE_MII_CONV);

	/* Set Coretex-A55 DSU clock to 1GHz */
	*(volatile u32 *)SCKCR2 |= SCKCR2_CA55SCLK;

	/* Enable Write protect to disable writing */
	*(volatile u32 *)PRCRN = PRCRN_PRKEY;
	*(volatile u32 *)PRCRS = PRCRS_PRKEY;
}

static void board_usb_init(void)
{
	/* Disable Write protect to enable writing */
	*(volatile u32 *)PRCRN = PRCRN_PRKEY | PRCRN_WR_EN;
	*(volatile u32 *)PRCRS = PRCRS_PRKEY | PRCRS_WR_EN;

	/* set P02_2 operation as USB_VBUSEN*/
	*(volatile u8 *)PMC(2)		|= BIT(2);
	*(volatile u64 *)PFC(2)		= (*(volatile u64 *)PFC(2) & 0xFFFFFFFFFF00FFFF) | (0x13 << 16);

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
		writel(((readl(ADXCTL0_BASE + i * 0x4) & ~DDRMIR_MASK) |
				DDRMIR1(addr_shift)), ADXCTL0_BASE + i * 0x4);

	for (i = 0; i < ADXC1_MASTERS; i++)
		writel(((readl(ADXCTL1_BASE + i * 0x4) & ~DDRMIR_MASK) |
			DDRMIR1(addr_shift)), ADXCTL1_BASE + i * 0x4);
	return 0;
}

int board_init(void)
{
	int ret;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_TEXT_BASE + 0x50000;
	adxctl_init();

	/* ETHSS: Mode Control 0x6, GMAC1 on port ETH3, GMAC2 on port ETH2 */
	ethss_init_hw(0x6);

	/* ETHSS: GMAC1 RGMII_ID mode on port ETH3 */
	ret = ethss_config(3, PHY_INTERFACE_MODE_RGMII_ID);
	if (ret < 0)
		return ret;
	/* Set up speed for Converters for GMAC1 */
	ethss_link_up(3, PHY_INTERFACE_MODE_RGMII_ID, SPEED_1000, DUPLEX_FULL);

	/* ETHSS: GMAC2 RGMII_ID mode on port ETH2 */
	ret = ethss_config(2, PHY_INTERFACE_MODE_RGMII_ID);
	if (ret < 0)
		return ret;
	/* Set up speed for Converters for GMAC2 */
	ethss_link_up(2, PHY_INTERFACE_MODE_RGMII_ID, SPEED_1000, DUPLEX_FULL);

	board_usb_init();

	return 0;
}

void reset_cpu(void)
{

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
