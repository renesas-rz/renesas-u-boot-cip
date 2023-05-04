// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * Misc boot support
 */
#include <common.h>
#include <command.h>
#include <net.h>

#ifdef CONFIG_CMD_GO

/* Allow ports to override the default behavior */
__attribute__((weak))
unsigned long do_go_exec(ulong (*entry)(int, char * const []), int argc,
				 char *const argv[])
{
	return entry (argc, argv);
}

static int do_go(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	ulong	addr, rc;
	int     rcode = 0;

	if (argc < 2)
		return CMD_RET_USAGE;

	addr = hextoul(argv[1], NULL);

	printf ("## Starting application at 0x%08lX ...\n", addr);

	/*
	 * pass address parameter as argv[0] (aka command name),
	 * and all remaining args
	 */
	rc = do_go_exec ((void *)addr, argc - 1, argv + 1);
	if (rc != 0) rcode = 1;

	printf ("## Application terminated, rc = 0x%lX\n", rc);
	return rcode;
}

/* -------------------------------------------------------------------- */

U_BOOT_CMD(
	go, CONFIG_SYS_MAXARGS, 1,	do_go,
	"start application at address 'addr'",
	"addr [arg ...]\n    - start application at address 'addr'\n"
	"      passing 'arg' as arguments"
);

#endif

U_BOOT_CMD(
	reset, 2, 0,	do_reset,
	"Perform RESET of the CPU",
	"- cold boot without level specifier\n"
	"reset -w - warm reset if implemented"
);

#ifdef CONFIG_CMD_POWEROFF
U_BOOT_CMD(
	poweroff, 1, 0,	do_poweroff,
	"Perform POWEROFF of the device",
	""
);
#endif

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


//static int do_wakeup_secondary_cpu(cmd_tl_t *cmdtp, int flag, int argc, char * const argv[])
static int do_wakeup_secondary_cpu(caddr_t *cmdtp, int flag, int argc, char * const argv[])

//caddr_t
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
        printf("[ERROR] ResetVector address[%08x] is greater than Linux area[%08x].\n", \
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
