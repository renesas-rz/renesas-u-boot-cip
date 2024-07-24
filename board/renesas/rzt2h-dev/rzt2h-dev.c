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

#define SCKCR			0x80280000
#define PHYSEL			BIT(21)

#define MSTPCRE			0x80280310
#define MSTPCRE_GMAC1		BIT(16)
#define MSTPCRE_ETHSS		BIT(3)

#define MRCTLE			0x80280250
#define MRCTLE_GMAC1_PCLKH	BIT(18)
#define MRCTLE_GMAC1_PCLKM	BIT(17)
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

#define MODCTRL			0x8

#define CS0ENDAD_xSPI(x)	(0x80293004 + (0x100 * (x)))


void s_init(void)
{
	/* Disable Write protect to enable writing */
	*(volatile u32 *)PRCRN = PRCRN_PRKEY | PRCRN_WR_EN;
	*(volatile u32 *)PRCRS = PRCRS_PRKEY | PRCRS_WR_EN;

	/* Setting xSPI1 CS0 End Address */
	*(volatile u32 *)CS0ENDAD_xSPI(1) = 0x57FFFFFF;

	/* P2_5_SD0_PWEN_A Output HIGH */
	*(volatile u8 *)PMC(2) &= ~BIT(5);
	*(volatile u8 *)P(2) |= BIT(5);
	*(volatile u16 *)PM(2) |= (0x3 << 10);

	/* P2_6_SD0_IOVS_A Output LOW */
	*(volatile u8 *)PMC(2) &= ~BIT(6);
	*(volatile u8 *)P(2) &= ~BIT(6);
	*(volatile u16 *)PM(2) |= (0x3 << 12);

	/* P8_5_SD1_PWEN_A Output HIGH */
	*(volatile u8 *)PMC(8) &= ~BIT(5);
	*(volatile u8 *)P(8) |= BIT(5);
	*(volatile u16 *)PM(8) |= (0x3 << 10);

	/* P8_6_SD1_IOVS_A Output LOW */
	*(volatile u8 *)PMC(8) &= ~BIT(6);
	*(volatile u8 *)P(8) &= ~BIT(6);
	*(volatile u16 *)PM(8) |= (0x3 << 12);

	/* Release module stop for SDHI0/1 */
	*(volatile u32 *)MSTPCRM &= ~(MSTPCRM_SDHI0 | MSTPCRM_SDHI1);

	/* Release PHY3 Reset state  P32_3 */
	*(volatile u8 *)PMC(32) &= ~BIT(3);
	*(volatile u8 *)P(32) |= BIT(3);
	*(volatile u16 *)PM(32) |= (0x3 << 6);

	/* ETH3 PIN */
	/* ETH3_TXCLK, ETH3_TXD[0], ETH3_TXD[1], ETH3_TXD[2], ETH3_TXD[3], ETH3_TXEN, ETH3_RXCLK, ETH3_RXD[0],
	ETH3_RXD[1], ETH3_RXD[2], ETH3_RXD[3], ETH3_RXDV, ETH3_TXER, ETH3_RXER, ETH3_CRS, ETH3_COL */
	*(volatile u8 *)PMC(33)		|= BIT(7)|BIT(6)|BIT(5)|BIT(4)|BIT(3)|BIT(2);
	*(volatile u64 *)PFC(33)	= (*(volatile u64 *)PFC(33) & 0x000000000000FFFF)
					| ((u64)0xf << 56) | ((u64)0xf << 48) | ((u64)0xf << 40) | ((u64)0xf << 32) | (0xf << 24) | (0xf << 16);

	*(volatile u8 *)PMC(34)		|= BIT(7)|BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0);
	*(volatile u64 *)PFC(34)	= (*(volatile u64 *)PFC(34) & 0x00ff000000000000)
					| ((u64)0xf << 56) | ((u64)0xf << 40) | ((u64)0xf << 32) | (0xf << 24) | (0xf << 16) | (0xf << 8) | (0xf << 0);

	*(volatile u8 *)PMC(35)		|= BIT(2)|BIT(1)|BIT(0);
	*(volatile u64 *)PFC(35)	= (*(volatile u64 *)PFC(35) & 0xffffffffff000000)
					| (0xf << 16) | (0xf << 8) | (0xf << 0);

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

	/* ETHSS: Mode Control 0x0, GMAC1 on port ETH3 */
	ethss_init_hw(0x0);
	/* ETHSS: GMAC1 RGMII_ID mode on port ETH3 */
	ret = ethss_config(3, PHY_INTERFACE_MODE_RGMII_ID);
	if (ret < 0)
		return ret;
	/* Set up speed for Converters for GMAC1 */
	ethss_link_up(3, PHY_INTERFACE_MODE_RGMII_ID, SPEED_1000, DUPLEX_FULL);

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
