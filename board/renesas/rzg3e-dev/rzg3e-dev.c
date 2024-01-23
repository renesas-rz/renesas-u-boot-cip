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
#include <dt-bindings/pinctrl/rzv2h-pinctrl.h>

DECLARE_GLOBAL_DATA_PTR;

#define PFC_BASE		0x10410000

#define P(x)			(PFC_BASE + 0x20 + PORT_##x)
#define PM(x)			(PFC_BASE + 0x140 + 2 * PORT_##x)
#define PMC(x)			(PFC_BASE + 0x220 + PORT_##x)
#define PFC(x)			(PFC_BASE + 0x0400 + 0x80 + 4 * PORT_##x)

#define PFC_PWPR		(PFC_BASE + 0x3C04)
#define PFC_PWPR_REGWE_A	BIT(6)

void s_init(void)
{
	/* Enable writing to PFC and PMC registers */
       *(volatile u32 *)PFC_PWPR = *(volatile u32 *)PFC_PWPR | PFC_PWPR_REGWE_A;

#if CONFIG_TARGET_RZG3E_DEV
	/* QSD1 */
	*(volatile u8 *)PMC(1) &= ~(BIT(5) | BIT(6)); /* P1_5, P1_6 */
	*(volatile u8 *)P(1) = (*(volatile u8 *)P(1) & ~BIT(5)) | BIT(6); /* P1_5 = 0, P1_6 = 1 */
	*(volatile u16 *)PM(1) = (*(volatile u16 *)PM(1) & ~GENMASK(13, 10)) | BIT(13) | BIT(11); /* P1_5, P1_6 output */

	*(volatile u32 *)PFC(G) = 0x00111111;
	*(volatile u8 *)PMC(G)  = 0x3f;

	/* QSD2 */
	*(volatile u8 *)PMC(K) &= ~(BIT(1) | BIT(2)); /* PK_1, PK_2 */
	*(volatile u8 *)P(K) = (*(volatile u8 *)P(K) & ~BIT(1)) | BIT(2); /* PK_1 = 0, PK_2 = 1 */
	*(volatile u16 *)PM(K) = (*(volatile u16 *)PM(K) & ~GENMASK(5, 2)) | BIT(5) | BIT(3); /* PK_1, PK_2 output */

	*(volatile u32 *)PFC(H) = 0x00111111;
	*(volatile u8 *)PMC(H)  = 0x3f;
#endif

#if CONFIG_TARGET_SMARC_RZG3E
	/* QSD2 */
	*(volatile u8 *)PMC(K) &= ~(BIT(1) | BIT(2)); /* PK_1, PK_2 */
	*(volatile u8 *)P(K) = *(volatile u8 *)P(K) | BIT(1) | BIT(2); /* PK_1 = 1, PK_2 = 1 */
	*(volatile u16 *)PM(K) = (*(volatile u16 *)PM(K) & ~GENMASK(5, 2)) | BIT(5) | BIT(3); /* PK_1, PK_2 output */
#endif

	/* Disable writing to PFC and PMC registers */
       *(volatile u32 *)PFC_PWPR = *(volatile u32 *)PFC_PWPR & ~PFC_PWPR_REGWE_A;
}

int board_early_init_f(void)
{

	return 0;
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
