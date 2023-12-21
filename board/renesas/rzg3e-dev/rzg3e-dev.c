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

#define PFC_BASE                       0x10410000
#define        P_34                            (PFC_BASE + 0x0034)
#define        PM_34                           (PFC_BASE + 0x0168)
#define        PMC_34                          (PFC_BASE + 0x0234)


void s_init(void)
{
#if CONFIG_TARGET_RZG3E_DEV
       *(volatile u8 *)PMC_34   &= ~(0x06<<4); /* PK1,PK2 port */
       *(volatile u8 *)P_34      = (*(volatile u32 *)P_34  & ~(0x03<<4)) | (0x01 <<5); /* PK1=1,PK2=0          */
       *(volatile u16 *)PM_34    = (*(volatile u32 *)PM_34 & ~(0x0f<<8)) | (0x0c <<8); /* PK1,PK2 output       */
#endif


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
