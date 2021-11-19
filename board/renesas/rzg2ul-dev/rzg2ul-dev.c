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
#include <asm/arch/rmobile.h>
#include <asm/arch/rcar-mstp.h>
#include <asm/arch/sh_sdhi.h>
#include <i2c.h>
#include <mmc.h>

DECLARE_GLOBAL_DATA_PTR;

#define PFC_BASE	0x11030000

#define ETH_CH0		(PFC_BASE + 0x300c)
#define ETH_CH1		(PFC_BASE + 0x3010)
#define ETH_PVDD_3300	0x00
#define ETH_PVDD_1800	0x01
#define ETH_PVDD_2500	0x02
#define ETH_MII_RGMII	(PFC_BASE + 0x3018)

/* CPG */
#define CPG_BASE					0x11010000
#define CPG_CLKON_BASE				(CPG_BASE + 0x500)
#define CPG_RESET_BASE				(CPG_BASE + 0x800)
#define CPG_RESET_ETH				(CPG_RESET_BASE + 0x7C)
#define CPG_PL2_SDHI_DSEL			(CPG_BASE + 0x218)
#define CPG_CLK_STATUS				(CPG_BASE + 0x280)

void s_init(void)
{
#if CONFIG_TARGET_RZG2UL_TYPE2_DEV
	/* can go in board_eht_init() once enabled */
	*(volatile u32 *)(ETH_CH0) = (*(volatile u32 *)(ETH_CH0) & 0xFFFFFFFC) | ETH_PVDD_2500;
	/* Enable RGMII for ETH0 */
	*(volatile u32 *)(ETH_MII_RGMII) = (*(volatile u32 *)(ETH_MII_RGMII) & 0xFFFFFFFC);
	/* ETH CLK */
	*(volatile u32 *)(CPG_RESET_ETH) = 0x30001;
#else
	/* can go in board_eht_init() once enabled */
	*(volatile u32 *)(ETH_CH0) = (*(volatile u32 *)(ETH_CH0) & 0xFFFFFFFC) | ETH_PVDD_3300;
	*(volatile u32 *)(ETH_CH1) = (*(volatile u32 *)(ETH_CH1) & 0xFFFFFFFC) | ETH_PVDD_3300;
	/* Enable RGMII for both ETH{0,1} */
	*(volatile u32 *)(ETH_MII_RGMII) = (*(volatile u32 *)(ETH_MII_RGMII) & 0xFFFFFFFC);
	/* ETH CLK */
	*(volatile u32 *)(CPG_RESET_ETH) = 0x30003;
#endif
	/* SD CLK */
	*(volatile u32 *)(CPG_PL2_SDHI_DSEL) = 0x00110011;
	while (*(volatile u32 *)(CPG_CLK_STATUS) != 0)
		;
}

int board_early_init_f(void)
{

	return 0;
}

/* PFC */

#define	PFC_P16				(PFC_BASE + 0x0016)
#define	PFC_P22				(PFC_BASE + 0x0022)
#define	PFC_PM16			(PFC_BASE + 0x012C)
#define	PFC_PM22			(PFC_BASE + 0x0144)
#define	PFC_PMC16			(PFC_BASE + 0x0216)
#define	PFC_PMC22			(PFC_BASE + 0x0222)

#define	PFC_P1D				(PFC_BASE + 0x001D)
#define	PFC_PM1D			(PFC_BASE + 0x013A)
#define	PFC_PMC1D			(PFC_BASE + 0x021D)


#define CONFIG_SYS_SH_SDHI0_BASE	0x11C00000
#define CONFIG_SYS_SH_SDHI1_BASE	0x11C10000

int board_mmc_init(struct bd_info *bis)
{
	int ret = 0;

#if CONFIG_TARGET_RZG2UL_TYPE2_DEV
	/* SD1 power control : P13_4 = 1 P13_3 = 0 */
	*(volatile u8 *)(PFC_PMC1D) &= 0xE7;	/* Port func mode 0b0000	*/
	*(volatile u16 *)(PFC_PM1D) = (*(volatile u16 *)(PFC_PM1D) & 0xFC3F) | 0x0280; /* Port output mode 0b1010 */
	*(volatile u8 *)(PFC_P1D) = (*(volatile u8 *)(PFC_P1D) & 0xE7) | 0x10; /* P13_4 output 1, P13_3 output 0 */
#else
	/* SD1 power control : P18_5 = 1 P6_2 = 1 */
	*(volatile u8 *)(PFC_PMC16) &= 0xFB;	/* Port func mode 0b00	*/
	*(volatile u8 *)(PFC_PMC22) &= 0xDF;	/* Port func mode 0b00	*/
	*(volatile u16 *)(PFC_PM16) = (*(volatile u16 *)(PFC_PM16) & 0xFFCF) | 0x0020; /* Port output mode 0b10 */
	*(volatile u16 *)(PFC_PM22) = (*(volatile u16 *)(PFC_PM22) & 0xF3FF) | 0x0800; /* Port output mode 0b10 */
	*(volatile u8 *)(PFC_P16) = (*(volatile u8 *)(PFC_P16) & 0xFB) | 0x04; /* P6_2  output 0	*/
	*(volatile u8 *)(PFC_P22) = (*(volatile u8 *)(PFC_P22) & 0xDF) | 0x20; /* P18_5 output 1	*/
#endif

	ret |= sh_sdhi_init(CONFIG_SYS_SH_SDHI0_BASE,
						0,
						SH_SDHI_QUIRK_64BIT_BUF);
	ret |= sh_sdhi_init(CONFIG_SYS_SH_SDHI1_BASE,
						1,
						SH_SDHI_QUIRK_64BIT_BUF);

	return ret;
}


int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_TEXT_BASE + 0x50000;

	return 0;
}

void reset_cpu(void)
{

}
