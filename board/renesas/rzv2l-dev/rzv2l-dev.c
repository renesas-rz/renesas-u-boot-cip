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

void s_init(void)
{
	/* can go in board_eht_init() once enabled */
	*(volatile u32 *)(ETH_CH0) = (*(volatile u32 *)(ETH_CH0) & 0xFFFFFFFC) | ETH_PVDD_1800;
	*(volatile u32 *)(ETH_CH1) = (*(volatile u32 *)(ETH_CH1) & 0xFFFFFFFC) | ETH_PVDD_1800;
	/* Enable RGMII for both ETH{0,1} */
	*(volatile u32 *)(ETH_MII_RGMII) = (*(volatile u32 *)(ETH_MII_RGMII) & 0xFFFFFFFC);
}

int board_early_init_f(void)
{

	return 0;
}

/* CPG */
#define CPG_BASE					0x11010000
#define CPG_CLKON_BASE				(CPG_BASE + 0x500)
#define CPG_CLKON_SCIF				(CPG_CLKON_BASE + 0x84)
#define CPG_CLKON_SDHI				(CPG_CLKON_BASE + 0x54)
#define CPG_CLKON_GPIO				(CPG_CLKON_BASE + 0x98)
#define CPG_CLKON_ETH				(CPG_CLKON_BASE + 0x7C)

/* PFC */
#define PFC_P37						(PFC_BASE + 0x037)
#define PFC_PM37					(PFC_BASE + 0x16E)
#define PFC_PMC37					(PFC_BASE + 0x237)

#define CONFIG_SYS_SH_SDHI0_BASE	0x11C00000
#define CONFIG_SYS_SH_SDHI1_BASE	0x11C10000
int board_mmc_init(struct bd_info *bis)
{
	int ret = 0;
	
	/* SD1 power control: P39_1 = 0; P39_2 = 1; */
	*(volatile u32 *)(PFC_PMC37) &= 0xFFFFFFF9; /* Port func mode 0b00 */
	*(volatile u32 *)(PFC_PM37) = (*(volatile u32 *)(PFC_PM37) & 0xFFFFFFC3) | 0x28; /* Port output mode 0b1010 */
#if CONFIG_TARGET_SMARC_RZV2L
	*(volatile u32 *)(PFC_P37) = (*(volatile u32 *)(PFC_P37) & 0xFFFFFFF9) | 0x6;	/* Port 39[2:1] output value 0b11*/
#else

	*(volatile u32 *)(PFC_P37) = (*(volatile u32 *)(PFC_P37) & 0xFFFFFFF9) | 0x4;	/* Port 39[2:1] output value 0b10*/
#endif
		
	ret |= sh_sdhi_init(CONFIG_SYS_SH_SDHI0_BASE,
						0,
						SH_SDHI_QUIRK_64BIT_BUF);
	ret |= sh_sdhi_init(CONFIG_SYS_SH_SDHI1_BASE,
						1,
						SH_SDHI_QUIRK_64BIT_BUF);

	return ret;
}

static int board_clk_init(void)
{
	/* Enable SCIF0 */
	*(volatile u32 *)(CPG_CLKON_SCIF) |= (0x1F <<1);
	*(volatile u32 *)(CPG_CLKON_SCIF) |= 0x1;

	/* Enable SDHI0,SHDI1 */
	*(volatile u32 *)(CPG_CLKON_SDHI) |= (0xFF << 16) ;
	*(volatile u32 *)(CPG_CLKON_SDHI) |= 0xFF;
	
	/* Enable SDHI0,SHDI1 */
	*(volatile u32 *)(CPG_CLKON_SCIF) |= (0xFF << 16) ;
	*(volatile u32 *)(CPG_CLKON_SCIF) |= 0xFF;

	/* Enable GPIO HCLK */
	*(volatile u32 *)(CPG_CLKON_GPIO) |= (0x1 << 16) ;
	*(volatile u32 *)(CPG_CLKON_GPIO) |= 1;

	/* Enable ETH0,ETH1 */
	*(volatile u32 *)(CPG_CLKON_ETH) |= (0x3 << 16) ;
	*(volatile u32 *)(CPG_CLKON_ETH) |= 3;
	
	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_TEXT_BASE + 0x50000;

	/*CLK init 
	board_clk_init();*/ 

	return 0;
}

void reset_cpu(ulong addr)
{

}
