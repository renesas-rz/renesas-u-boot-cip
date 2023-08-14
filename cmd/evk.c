/*******************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only
* intended for use with Renesas products. No other uses are authorized. This
* software is owned by Renesas Electronics Corporation and is protected under
* all applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT
* LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
* AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR
* ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE
* BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software
* and to discontinue the availability of this software. By using this software,
* you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
* Copyright (C) 2022 Renesas Electronics Corporation. All rights reserved.
*******************************************************************************/
#include <common.h>
#include <command.h>

#define BASE_ADDR_PSC       0xA3700000

#define BitCA53Core0    0x0u
#define BitCA53Core1    0x1u
#define TARGET_CORE     BitCA53Core1

#define BASE_ADDR_CA53      0xA3F02000
#define OFFSET_REG_EDCR     0x00    /* Endian */
#define OFFSET_REG_VACR     0x04    /* Vector address */
#define OFFSET_REG_TECR     0x08    /* Thumb */
#define OFFSET_REG_CP15SACR 0x0C    /* Coprocessor15 */
#define OFFSET_REG_AARCHCR  0x10
#define OFFSET_REG_RVA1CRL  0x28    /* Reset vector Low */
#define OFFSET_REG_RVA1CRH  0x2C    /* Reset vector High */
#define OFFSET_REG_PSCR     0x3C    /* Processor power Status */

#define EDCR_LITTLE_ENDIAN      0x0
#define VACR_ABTVECT_ADR_ZERO   0x0
#define TECR_ENABLE_ARM         0x0
#define CP15SACR_WRITE_ENABLE   0x0
#define AARCHCR_AARCH64         0x1
#define PSCR_PS_POWERUP         0x1

#define BASE_ADDR_CPG       0xA3500000
#define OFFSET_REG_CPG_RST8 0x61C

#define NCPUPORESET_RESET_ASSERT        (0x0<<1)
#define NCPUPORESET_RESET_DEASSERT      (0x1<<1)
#define NCPUPORESET_EN (0x1<<17)

#define NCORERESET1_RESET_ASSERT        (0x0<<3)
#define NCORERESET1_RESET_DEASSERT      (0x1<<3)
#define NCORERESET1_EN (0x1<<19)

//#define ONLY_NCPUPORESET_MODE

#ifdef ONLY_NCPUPORESET_MODE
#define CORE1_COLD_RESET_ASSERT         (NCPUPORESET_RESET_ASSERT) + \
                                        (NCPUPORESET_EN)
#define CORE1_COLD_RESET_DEASSERT       (NCPUPORESET_RESET_DEASSERT) + \
                                        (NCPUPORESET_EN)
#else
#define CORE1_COLD_RESET_ASSERT         (NCPUPORESET_RESET_ASSERT + NCORERESET1_RESET_ASSERT) + \
                                        (NCPUPORESET_EN + NCORERESET1_EN)
#define CORE1_COLD_RESET_DEASSERT       (NCPUPORESET_RESET_DEASSERT + NCORERESET1_RESET_DEASSERT) + \
                                        (NCPUPORESET_EN + NCORERESET1_EN)
#endif /*ONLY_NCPUPORESET_MODE*/

extern void CMN_DelayInUSec(uint64_t us);

static inline unsigned int ca53_read32( uintptr_t addr )
{
        return *(volatile unsigned int*)(BASE_ADDR_CA53+addr);
}

static inline void ca53_write32( uintptr_t addr, uint32_t val )
{
        *(volatile unsigned int*)(BASE_ADDR_CA53+addr) = val;
}

static inline void cpg_write32( uintptr_t addr, uint32_t val )
{
        *(volatile unsigned int*)(BASE_ADDR_CPG+addr) = val;
}

static inline void psc_write32( uintptr_t addr, uint32_t val )
{
       *(volatile unsigned int*)(BASE_ADDR_PSC+addr) = val;
}

static int do_evk_shutdown(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{

    printf("Shutdown the evk board...\n");
    psc_write32(0x50,0x1);
    CMN_DelayInUSec(1000*1000); //wait 1sec
    return 0;
}

U_BOOT_CMD(
       evk_shutdown, 1, 0,     do_evk_shutdown,
       "Shutdown the EVK board.",
       "\n"
       "\t- Shutdown the EVK board on u-boot"
);

static int do_wakeup_secondary_cpu(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
    unsigned int reg;
    unsigned int reset_vector_addr;
    unsigned long kernel_start_addr;

    if(argc < 2)
    {
        return CMD_RET_USAGE;
    }

    reset_vector_addr = simple_strtoul(argv[1], NULL, 16);
    kernel_start_addr = env_get_hex("fdt_addr",0);
    if( reset_vector_addr > kernel_start_addr )
    {
        printf("[ERROR] ResetVector address[%08x] is greater than Linux area[%08lx].\n", \
                reset_vector_addr,kernel_start_addr
        );
        return CMD_RET_USAGE;
    }
    printf("[DEBUG]Reset vector address 0x%08x\n",reset_vector_addr);

    /* Setting secondary cpu for cold reset */
    reg = ca53_read32( OFFSET_REG_EDCR );
    reg &= ( ~TARGET_CORE );    //Little endian
    ca53_write32( OFFSET_REG_EDCR, reg );

    reg = ca53_read32( OFFSET_REG_VACR );
    reg &= ( ~TARGET_CORE );    //Abort vector address is zero
    ca53_write32( OFFSET_REG_VACR, reg );

    reg = ca53_read32( OFFSET_REG_TECR );
    reg &= ( ~TARGET_CORE );    //Reset on ARM
    ca53_write32( OFFSET_REG_TECR, reg );

    reg = ca53_read32( OFFSET_REG_CP15SACR );
    reg &= ( ~TARGET_CORE );    //Enable write access for CP15 secure register
    ca53_write32( OFFSET_REG_CP15SACR, reg );

    reg = ca53_read32( OFFSET_REG_AARCHCR );
    reg |= ( AARCHCR_AARCH64 << TARGET_CORE );
    ca53_write32( OFFSET_REG_AARCHCR, reg );

    ca53_write32( OFFSET_REG_RVA1CRL, reset_vector_addr );  //Set reset vector
    ca53_write32( OFFSET_REG_RVA1CRH, 0x0 );                //Set reset vector

    printf("Wakeup for Secondary CPU\n");

    /*Assert ca53 core1 processor */
    cpg_write32( OFFSET_REG_CPG_RST8, CORE1_COLD_RESET_ASSERT);
    CMN_DelayInUSec(1); //wait 1us

    /*Deassert ca53 core1 processor */
    cpg_write32( OFFSET_REG_CPG_RST8, CORE1_COLD_RESET_DEASSERT);
    CMN_DelayInUSec(1); //wait 1us

    return 0;
}

U_BOOT_CMD(
        wakeup_a53core1, 2, 0,  do_wakeup_secondary_cpu,
        "Wakeup the Cortex-A53 core1 for Real Time OS.",
    "[reset vector address]\n"
    "\t- Cold boot CA53 #1 at [reset vector address]"
);
