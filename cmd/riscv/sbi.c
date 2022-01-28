// SPDX-License-Identifier: GPL-2.0+
/*
 * The 'sbi' command displays information about the SBI implementation.
 *
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#include <common.h>
#include <command.h>
#include <asm/sbi.h>
#include <irq_func.h>
#include <asm/csr.h>
#include <asm/encoding.h>

struct sbi_ext {
	const u32 id;
	const char *name;
};

static struct sbi_ext extensions[] = {
	{ 0x00000000, "sbi_set_timer" },
	{ 0x00000001, "sbi_console_putchar" },
	{ 0x00000002, "sbi_console_getchar" },
	{ 0x00000003, "sbi_clear_ipi" },
	{ 0x00000004, "sbi_send_ipi" },
	{ 0x00000005, "sbi_remote_fence_i" },
	{ 0x00000006, "sbi_remote_sfence_vma" },
	{ 0x00000007, "sbi_remote_sfence_vma_asid" },
	{ 0x00000008, "sbi_shutdown" },
	{ 0x00000010, "SBI Base Functionality" },
	{ 0x54494D45, "Timer Extension" },
	{ 0x00735049, "IPI Extension" },
	{ 0x52464E43, "RFENCE Extension" },
	{ 0x0048534D, "Hart State Management Extension" },
};

static int do_sbi(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[])
{
	int i;
	long ret;

	ret = sbi_get_spec_version();
	if (ret >= 0)
		printf("SBI %ld.%ld\n", ret >> 24, ret & 0xffffff);
	ret = sbi_get_impl_id();
	if (ret >= 0) {
		switch (ret) {
		case 0:
			printf("Berkeley Boot Loader (BBL)\n");
			break;
		case 1:
			printf("OpenSBI\n");
			break;
		case 2:
			printf("Xvisor\n");
			break;
		case 3:
			printf("KVM\n");
			break;
		default:
			printf("Unknown implementation\n");
			break;
		}
	}
	printf("Extensions:\n");
	for (i = 0; i < ARRAY_SIZE(extensions); ++i) {
		ret = sbi_probe_extension(extensions[i].id);
		if (ret > 0)
			printf("  %s\n", extensions[i].name);
	}
	return 0;
}

#ifdef CONFIG_SYS_LONGHELP
static char sbi_help_text[] =
	"- display SBI spec version, implementation, and available extensions";

#endif

U_BOOT_CMD_COMPLETE(
	sbi, 1, 0, do_sbi,
	"display SBI information",
	sbi_help_text, NULL
);

#ifdef CONFIG_DEBUG_OPENSBI

#define SBI_EXT_TIMER 0x54494D45

#define SBI_EXT_VENDOR 0x09000000
#define SBI_EXT_ANDES_GET_MCACHE_CTL_STATUS   0
#define SBI_EXT_ANDES_GET_MMISC_CTL_STATUS    1
#define SBI_EXT_ANDES_SET_MCACHE_CTL          2
#define SBI_EXT_ANDES_SET_MMISC_CTL           3
#define SBI_EXT_ANDES_ICACHE_OP               4
#define SBI_EXT_ANDES_DCACHE_OP               5
#define SBI_EXT_ANDES_L1CACHE_I_PREFETCH      6
#define SBI_EXT_ANDES_L1CACHE_D_PREFETCH      7
#define SBI_EXT_ANDES_NON_BLOCKING_LOAD_STORE 8
#define SBI_EXT_ANDES_WRITE_AROUND            9

enum sbi_ext_renesas_fid {
	SBI_EXT_RENESAS_GET_MCACHE_CTL_STATUS = 0,
	SBI_EXT_RENESAS_GET_MMISC_CTL_STATUS,
	SBI_EXT_RENESAS_SET_MCACHE_CTL,
	SBI_EXT_RENESAS_SET_MMISC_CTL,
	SBI_EXT_RENESAS_ICACHE_OP,
	SBI_EXT_RENESAS_DCACHE_OP,
	SBI_EXT_RENESAS_L1CACHE_I_PREFETCH,
	SBI_EXT_RENESAS_L1CACHE_D_PREFETCH,
	SBI_EXT_RENESAS_NON_BLOCKING_LOAD_STORE,
	SBI_EXT_RENESAS_WRITE_AROUND,
};

enum sbi_ext_id_v01 {
	SBI_EXT_0_1_SET_TIMER = 0x0,
	SBI_EXT_0_1_CONSOLE_PUTCHAR = 0x1,
	SBI_EXT_0_1_CONSOLE_GETCHAR = 0x2,
	SBI_EXT_0_1_CLEAR_IPI = 0x3,
	SBI_EXT_0_1_SEND_IPI = 0x4,
	SBI_EXT_0_1_REMOTE_FENCE_I = 0x5,
	SBI_EXT_0_1_REMOTE_SFENCE_VMA = 0x6,
	SBI_EXT_0_1_REMOTE_SFENCE_VMA_ASID = 0x7,
	SBI_EXT_0_1_SHUTDOWN = 0x8,
};

#define IC_EN 0x1 << 0
#define DC_EN 0x1 << 1
#define IC_ECCEN 0x1 << 2
#define DC_ECCEN 0x1 << 4
#define IC_RWECC 0x1 << 6
#define DC_RWECC 0x1 << 7
#define CCTL_SUEN 0x1 << 8
#define IPREF_EN 0x1 << 9
#define DPREF_EN 0x1 << 10
#define IC_FIRST_WORD 0x1 << 11
#define DC_FIRST_WORD 0x1 << 12
#define DC_WAROUND 0x1 << 13
#define DC_COHEN 0x1 << 19
#define DC_COHSTA 0x1 << 20

#define VEC_PLIC 0x1 << 1
#define RVCOMPM  0x1 << 2
#define BRPE     0x1 << 3
#define MSA_UNA  0x1 << 6
#define NBLD_EN  0x1 << 8

#define MTIME 0x110C0000

int do_sbi_get_mcache_ctl_status(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct sbiret ret;

	ret = sbi_ecall(SBI_EXT_VENDOR, SBI_EXT_ANDES_GET_MCACHE_CTL_STATUS,
			0, 0, 0, 0, 0, 0);
	printf("%s\n",__func__);
	printf("value :  0x%lx\n", ret.value);
	if(ret.error)
	{
		printf("error :  %ld\n", ret.error);
	}
	return 0;
}

int do_sbi_get_mmisc_ctl_status(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct sbiret ret;

	ret = sbi_ecall(SBI_EXT_VENDOR, SBI_EXT_ANDES_GET_MMISC_CTL_STATUS,
			0, 0, 0, 0, 0, 0);
	printf("%s\n",__func__);
	printf("value :  0x%lx\n", ret.value);
	if(ret.error)
	{
		printf("error :  %ld\n", ret.error);
	}
	return 0;
}

int do_sbi_set_mcache_ctl(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	long value=0;
	struct sbiret ret;
	const char *cmd;
	
	
	if (argc < 2)
		return CMD_RET_USAGE;
	
	cmd = argv[1];
	switch (*cmd) {
	case '0':
		value=IC_EN | DC_EN;
		break;
	case '1':
		value=DC_EN;
		break;
	case '2':
		value=IC_ECCEN | DC_EN;
		break;
	case '3':
		value=DC_ECCEN | DC_EN;
		break;
	case '4':
		value=IC_RWECC | DC_EN;
		break;
	case '5':
		value=DC_RWECC | DC_EN;
		break;
	case '6':
		value=CCTL_SUEN | DC_EN;
		break;
	case '7':
		value=IPREF_EN | DC_EN;
		break;
	case '8':
		value=DPREF_EN | DC_EN;
		break;
	case '9':
		value=IC_FIRST_WORD | DC_EN;
		break;
	case 'a':
		value=DC_FIRST_WORD | DC_EN;
		break;
	case 'b':
		value=DC_WAROUND | DC_EN;
		break;
	case 'c':
		value=DC_COHEN | DC_EN;
		break;
	case 'd':
		value=DC_COHSTA | DC_EN;
		break;
	case 'e':
		value=0x0;
		break;
	default:
		goto mcache_ctl_end;
	}

	ret = sbi_ecall(SBI_EXT_VENDOR, SBI_EXT_ANDES_SET_MCACHE_CTL,
			value, 0, 0, 0, 0, 0);
	printf("%s\n",__func__);
	if(ret.error)
	{
		printf("error :  %ld\n", ret.error);
	}
mcache_ctl_end:
	return 0;
}

int do_sbi_set_mmisc_ctl(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	unsigned int value;
	struct sbiret ret;
	const char *cmd;
	
	
	if (argc < 2)
		return CMD_RET_USAGE;
	
	cmd = argv[1];
	switch (*cmd) {
	case '0':
		value=VEC_PLIC;
		break;
	case '1':
		value=RVCOMPM;
		break;
	case '2':
		value=BRPE;
		break;
	case '3':
		value=MSA_UNA;
		break;
	case '4':
		value=NBLD_EN;
		break;
	case '5':
		value=0x48;
		break;
	case '6':
		value=0x00;
		break;
	default:
		goto mmisc_ctl_end;
	}

	ret = sbi_ecall(SBI_EXT_VENDOR, SBI_EXT_ANDES_SET_MMISC_CTL,
		value, 0, 0, 0, 0, 0);
	printf("%s\n",__func__);
	if(ret.error)
	{
		printf("error :  %ld\n", ret.error);
	}
mmisc_ctl_end:
	return 0;
}

int do_sbi_icache(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	unsigned int value;
	struct sbiret ret;
	const char *cmd;
	

	if (argc < 2)
		return CMD_RET_USAGE;

	cmd = argv[1];
	switch (*cmd) {
	case '0':	
		value=0x0;/*off*/
		break;
	case '1':
		value=0x1;/*on*/
		break;
	default:
		goto icache_end;
	}
	ret = sbi_ecall(SBI_EXT_VENDOR, SBI_EXT_ANDES_ICACHE_OP,
			value, 0, 0, 0, 0, 0);
	printf("%s\n",__func__);
	if(ret.error)
	{
		printf("error :  %ld\n", ret.error);
	}
icache_end:
	return 0;
}


int do_sbi_dcache(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	unsigned int value;
	struct sbiret ret;
	const char *cmd;
	
	if (argc < 2)
		return CMD_RET_USAGE;
	
	cmd = argv[1];
	switch (*cmd) {
	case '0':
		value=0x0;/*off*/
		break;
	case '1':
		value=0x1;/*on*/
		break;
	default:
		goto dcache_end;
	}
	ret = sbi_ecall(SBI_EXT_VENDOR, SBI_EXT_ANDES_DCACHE_OP,
			value, 0, 0, 0, 0, 0);
	printf("%s\n",__func__);
	if(ret.error)
	{
		printf("error :  %ld\n", ret.error);
	}
dcache_end:
	return 0;
}

int do_sbi_l1cache_i_prefetch(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	unsigned int value;
	struct sbiret ret;
	const char *cmd;
	
	if (argc < 2)
		return CMD_RET_USAGE;
	
	cmd = argv[1];
	switch (*cmd) {
	case '0':
		value=0x0;/*off*/
		break;
	case '1':
		value=0x1;/*on*/
		break;
	default:
		goto i_prefetch_end;
	}
	ret = sbi_ecall(SBI_EXT_VENDOR, SBI_EXT_ANDES_L1CACHE_I_PREFETCH,
			value, 0, 0, 0, 0, 0);
	printf("%s\n",__func__);
	if(ret.error)
	{
		printf("error :  %ld\n", ret.error);
	}
i_prefetch_end:
	return 0;
}

int do_sbi_l1cache_d_prefetch(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	unsigned int value;
	struct sbiret ret;
	const char *cmd;
	
	if (argc < 2)
		return CMD_RET_USAGE;
	
	cmd = argv[1];
	switch (*cmd) {
	case '0':
		value=0x0;/*off*/
		break;
	case '1':
		value=0x1;/*on*/
		break;
	default:
		goto d_prefetch_end;
	}

	ret = sbi_ecall(SBI_EXT_VENDOR, SBI_EXT_ANDES_L1CACHE_D_PREFETCH,
		value, 0, 0, 0, 0, 0);
	printf("%s\n",__func__);
	if(ret.error)
	{
		printf("error :  %ld\n", ret.error);
	}
d_prefetch_end:
	return 0;
}

int do_sbi_non_blocking_load_store(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	unsigned int value;
	struct sbiret ret;
	const char *cmd;
	
	if (argc < 2)
		return CMD_RET_USAGE;
	
	cmd = argv[1];
	switch (*cmd) {
	case '0':
		value=0x0;/*off*/
		break;
	case '1':
		value=0x1;/*on*/
		break;
	default:
		goto non_blocking_end;
	}
	ret = sbi_ecall(SBI_EXT_VENDOR, SBI_EXT_ANDES_NON_BLOCKING_LOAD_STORE,
		value, 0, 0, 0, 0, 0);
	printf("%s\n",__func__);
	if(ret.error)
	{
		printf("error :  %ld\n", ret.error);
	}
non_blocking_end:
	return 0;
}


int do_sbi_write_around(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	unsigned int value;
	struct sbiret ret;
	const char *cmd;
	
	if (argc < 2)
		return CMD_RET_USAGE;
	
	cmd = argv[1];
	switch (*cmd) {
	case '0':
		value=0x0;/*off*/
		break;
	case '1':
		value=0x1;/*on*/
		break;
	default:
		goto write_around_end;
	}
	ret = sbi_ecall(SBI_EXT_VENDOR, SBI_EXT_ANDES_WRITE_AROUND,
		value, 0, 0, 0, 0, 0);
	printf("%s\n",__func__);
	if(ret.error)
	{
		printf("error :  %ld\n", ret.error);
	}
write_around_end:
	return 0;
}

int do_sbi_ecall_timer(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct sbiret ret;
	unsigned long value;
	const char *cmd;
	unsigned long *mtime;

	if (argc < 2)
		return CMD_RET_USAGE;
	
	printf("%s\n",__func__);
	mtime = (unsigned long *)(MTIME);
	printf("mtime:0x%lx\n",*mtime);
	printf("mtime:%lds\n",*mtime/12000000);

	cmd = argv[1];
	switch (*cmd) {
	case '0':
		value=0x00;
		break;
	case '1':
		value=0x00000000050000000;
		break;
	case '2':
		value=*mtime+(10*12*1000000);
		break;
	default:
		goto ext_set_timer_end;
	}
	printf("value:0x%lx\n",value);
	printf("value:%lds\n",value/12000000);


	ret = sbi_ecall(SBI_EXT_TIMER, 0, value,
		  0, 0, 0, 0, 0);
	csr_clear(CSR_SIP,SIP_STIP);
	csr_set(CSR_SSTATUS,SR_SIE);
	csr_set(CSR_SIE,SIE_STIE);
ext_set_timer_end:
	return 0;
}

int do_sbi_ext_set_timer(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct sbiret ret;
	unsigned long value;
	const char *cmd;
	unsigned long *mtime;

	if (argc < 2)
		return CMD_RET_USAGE;

	printf("%s\n",__func__);
	mtime = (unsigned long *)(MTIME);
	printf("mtime:0x%lx\n",*mtime);
	printf("mtime:%lds\n",*mtime/12000000);

	cmd = argv[1];
	switch (*cmd) {
	case '0':
		value=0x00;
		break;
	case '1':
		value=0x00000000050000000;
		break;
	case '2':
		value=*mtime+(10*12*1000000);
		break;
	default:
		goto ext_set_timer_end;
	}
	printf("value:0x%lx\n",value);
	printf("value:%lds\n",value/12000000);


	ret = sbi_ecall(SBI_EXT_SET_TIMER, SBI_FID_SET_TIMER, value,
		  0, 0, 0, 0, 0);
	csr_clear(CSR_SIP,SIP_STIP);
	csr_set(CSR_SSTATUS,SR_SIE);
	csr_set(CSR_SIE,SIE_STIE);
	
ext_set_timer_end:
	return 0;
}

void timer_interrupt(struct pt_regs *regs)
{
	struct sbiret ret;
	unsigned long value;

	value = -1;
	ret = sbi_ecall(SBI_EXT_SET_TIMER, SBI_FID_SET_TIMER, value,
	  0, 0, 0, 0, 0);
	csr_clear(CSR_SSTATUS,SR_SIE);
	csr_clear(CSR_SIE,SIE_STIE);
	printf("%s\n",__func__);
}

int do_sbi_ext_get_spec_version(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct sbiret ret;
	
	ret = sbi_ecall(SBI_EXT_BASE, SBI_EXT_BASE_GET_SPEC_VERSION,
			0, 0, 0, 0, 0, 0);
	printf("%s\n",__func__);
	printf("value :  %ld\n", ret.value);
	if(ret.error)
	{
		printf("error :  %ld\n", ret.error);
	}

	return 0;
}

int do_sbi_ext_get_impl_id(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct sbiret ret;

	ret = sbi_ecall(SBI_EXT_BASE, SBI_EXT_BASE_GET_IMP_ID,
			0, 0, 0, 0, 0, 0);
	printf("%s\n",__func__);
	printf("value :  %ld\n", ret.value);
	if(ret.error)
	{
		printf("error :  %ld\n", ret.error);
	}

	return 0;
}

int do_sbi_ext_console_putchar(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct sbiret ret;
	unsigned int value;
	const char *cmd;
	
	if (argc < 2)
		return CMD_RET_USAGE;
	
	cmd = argv[1];
	value = (0x00000000000000FF & *cmd);

	printf("%s\n",__func__);
	printf("value :  %x\n", value);
	printf("argv :  %c\n", *argv[1]);
	ret = sbi_ecall(SBI_EXT_0_1_CONSOLE_PUTCHAR, 0, value, 0, 0, 0, 0, 0);
	printf("\n");
	if(ret.error)
	{
		printf("error :  %ld\n", ret.error);
	}
	return 0;
}

U_BOOT_CMD_COMPLETE(sbi_get_mcache_ctl_status, 1, 0, do_sbi_get_mcache_ctl_status,"OpenSBI DEBUG:GET_MCACHE_CTL_STATUS",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_get_mmisc_ctl_status, 1, 0, do_sbi_get_mmisc_ctl_status,"OpenSBI DEBUG:GET_MMISC_CTL_STATUS",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_set_mcache_ctl, 2, 0, do_sbi_set_mcache_ctl,"OpenSBI DEBUG:SET_MCACHE_CTL",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_set_mmisc_ctl, 2, 0, do_sbi_set_mmisc_ctl,"OpenSBI DEBUG:SET_MMISC_CTL",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_icache_op, 2, 0, do_sbi_icache,"OpenSBI DEBUG:ICACHE_OP",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_dcache_op, 2, 0, do_sbi_dcache,"OpenSBI DEBUG:DCACHE_OP",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_l1cache_i_prefetch, 2, 0,do_sbi_l1cache_i_prefetch,"OpenSBI DEBUG:L1CACHE_I_PREFETCH",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_l1cache_d_prefetch, 2, 0, do_sbi_l1cache_d_prefetch,"OpenSBI DEBUG:L1CACHE_D_PREFETCH",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_non_blocking_load_store, 2, 0, do_sbi_non_blocking_load_store,"OpenSBI DEBUG:NON_BLOCKING_LOAD_STORE",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_write_around, 2, 0, do_sbi_write_around,"OpenSBI DEBUG:WRITE_AROUND",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_ecall_timer, 2, 0, do_sbi_ecall_timer,"OpenSBI DEBUG:ECALL_TIMER",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_set_timer, 2, 0, do_sbi_ext_set_timer,"OpenSBI DEBUG:SET_TIMER",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_get_spec_version, 1, 0, do_sbi_ext_get_spec_version,"OpenSBI DEBUG:SPEC VERDION",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_get_impl_id, 1, 0, do_sbi_ext_get_impl_id,"OpenSBI DEBUG:GET IMP ID",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_console_putchar, 2, 0, do_sbi_ext_console_putchar,"OpenSBI DEBUG:PUT CHAR",NULL, NULL);

#endif
