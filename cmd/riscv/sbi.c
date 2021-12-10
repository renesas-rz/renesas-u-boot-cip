// SPDX-License-Identifier: GPL-2.0+
/*
 * The 'sbi' command displays information about the SBI implementation.
 *
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#include <common.h>
#include <command.h>
#include <asm/sbi.h>

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

#ifdef CONFIG_OPENSBI_DEBUG

#define SBI_EXT_RENESAS 0x90000000

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

#define SBI_EN 1
#define SBI_DI 0

int do_sbi_get_mcache_ctl_status(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct sbiret ret;

	ret = sbi_ecall(SBI_EXT_RENESAS, SBI_EXT_RENESAS_GET_MCACHE_CTL_STATUS,
			0, 0, 0, 0, 0, 0);
	printf("  %ld\n", ret.value);
	return 0;
}

int sbi_get_mmisc_ctl_status(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct sbiret ret;
	ret = sbi_ecall(SBI_EXT_RENESAS, SBI_EXT_RENESAS_GET_MMISC_CTL_STATUS,
			0, 0, 0, 0, 0, 0);
	printf("  %ld\n", ret.value);
	return 0;
}

int sbi_set_mcache_ctl(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct sbiret ret;
	
	ret = sbi_ecall(SBI_EXT_RENESAS, SBI_EXT_RENESAS_SET_MCACHE_CTL,
			0, 0, 0, 0, 0, 0);
	if (!ret.error)
		if (ret.value)
			return ret.value;

	return -ENOTSUPP;
}

int sbi_set_mmisc_ctl(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct sbiret ret;

	ret = sbi_ecall(SBI_EXT_RENESAS, SBI_EXT_RENESAS_SET_MMISC_CTL,
			0, 0, 0, 0, 0, 0);
	if (!ret.error)
		if (ret.value)
			return ret.value;

	return -ENOTSUPP;
}

int sbi_icache_op_en(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct sbiret ret;
	
	ret = sbi_ecall(SBI_EXT_RENESAS, SBI_EXT_RENESAS_ICACHE_OP,
			SBI_EN, 0, 0, 0, 0, 0);
	if (!ret.error)
		if (ret.value)
			return ret.value;

	return -ENOTSUPP;
}

int sbi_icache_op_dis(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct sbiret ret;
	
	ret = sbi_ecall(SBI_EXT_RENESAS, SBI_EXT_RENESAS_ICACHE_OP,
			SBI_DI, 0, 0, 0, 0, 0);
	if (!ret.error)
		if (ret.value)
			return ret.value;

	return -ENOTSUPP;
}

int sbi_dcache_op_en(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct sbiret ret;

	ret = sbi_ecall(SBI_EXT_RENESAS, SBI_EXT_RENESAS_DCACHE_OP,
			SBI_EN, 0, 0, 0, 0, 0);
	if (!ret.error)
		if (ret.value)
			return ret.value;

	return -ENOTSUPP;
}

int sbi_dcache_op_dis(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct sbiret ret;

	ret = sbi_ecall(SBI_EXT_RENESAS, SBI_EXT_RENESAS_DCACHE_OP,
			SBI_DI, 0, 0, 0, 0, 0);
	if (!ret.error)
		if (ret.value)
			return ret.value;

	return -ENOTSUPP;
}

int sbi_l1cache_i_prefetch_en(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct sbiret ret;

	ret = sbi_ecall(SBI_EXT_RENESAS, SBI_EXT_RENESAS_L1CACHE_I_PREFETCH,
			SBI_EN, 0, 0, 0, 0, 0);
	if (!ret.error)
		if (ret.value)
			return ret.value;

	return -ENOTSUPP;
}

int sbi_l1cache_i_prefetch_dis(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct sbiret ret;

	ret = sbi_ecall(SBI_EXT_RENESAS, SBI_EXT_RENESAS_L1CACHE_I_PREFETCH,
			SBI_DI, 0, 0, 0, 0, 0);
	if (!ret.error)
		if (ret.value)
			return ret.value;

	return -ENOTSUPP;
}

int sbi_l1cache_d_prefetch_en(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct sbiret ret;

	ret = sbi_ecall(SBI_EXT_RENESAS, SBI_EXT_RENESAS_L1CACHE_D_PREFETCH,
			SBI_EN, 0, 0, 0, 0, 0);
	if (!ret.error)
		if (ret.value)
			return ret.value;

	return -ENOTSUPP;
}

int sbi_l1cache_d_prefetch_dis(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct sbiret ret;

	ret = sbi_ecall(SBI_EXT_RENESAS, SBI_EXT_RENESAS_L1CACHE_D_PREFETCH,
			SBI_DI, 0, 0, 0, 0, 0);
	if (!ret.error)
		if (ret.value)
			return ret.value;

	return -ENOTSUPP;
}

int sbi_non_blocking_load_store_en(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct sbiret ret;

	ret = sbi_ecall(SBI_EXT_RENESAS, SBI_EXT_RENESAS_NON_BLOCKING_LOAD_STORE,
			SBI_EN, 0, 0, 0, 0, 0);
	if (!ret.error)
		if (ret.value)
			return ret.value;

	return -ENOTSUPP;
}

int sbi_non_blocking_load_store_dis(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct sbiret ret;

	ret = sbi_ecall(SBI_EXT_RENESAS, SBI_EXT_RENESAS_NON_BLOCKING_LOAD_STORE,
			SBI_DI, 0, 0, 0, 0, 0);
	if (!ret.error)
		if (ret.value)
			return ret.value;

	return -ENOTSUPP;
}

int sbi_write_around_en(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct sbiret ret;

	ret = sbi_ecall(SBI_EXT_RENESAS, SBI_EXT_RENESAS_WRITE_AROUND,
			SBI_EN, 0, 0, 0, 0, 0);
	if (!ret.error)
		if (ret.value)
			return ret.value;

	return -ENOTSUPP;
}

int sbi_write_around_dis(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct sbiret ret;

	ret = sbi_ecall(SBI_EXT_RENESAS, SBI_EXT_RENESAS_WRITE_AROUND,
			SBI_DI, 0, 0, 0, 0, 0);
	if (!ret.error)
		if (ret.value)
			return ret.value;

	return -ENOTSUPP;
}

U_BOOT_CMD_COMPLETE(sbi_get_mcache_ctl_status, 0, 0, do_sbi_get_mcache_ctl_status,"OpenSBI DEBUG:GET_MCACHE_CTL_STATUS",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_get_mmisc_ctl_status, 0, 0, sbi_get_mmisc_ctl_status,"OpenSBI DEBUG:GET_MMISC_CTL_STATUS",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_set_mcache_ctl, 1, 0, sbi_set_mcache_ctl,"OpenSBI DEBUG:SET_MCACHE_CTL",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_set_mmisc_ctl, 1, 0, sbi_set_mmisc_ctl,"OpenSBI DEBUG:SET_MMISC_CTL",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_icache_op_en, 1, 0, sbi_icache_op_en,"OpenSBI DEBUG:ICACHE_OP_EN",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_icache_op_dis, 1, 0, sbi_icache_op_en,"OpenSBI DEBUG:ICACHE_OP_DIS",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_dcache_op_en, 1, 0, sbi_dcache_op_dis,"OpenSBI DEBUG:DCACHE_OP_EN",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_dcache_op_dis, 1, 0, sbi_dcache_op_dis,"OpenSBI DEBUG:DCACHE_OP_DIS",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_l1cache_i_prefetch_en, 1, 0, sbi_l1cache_i_prefetch_en,"OpenSBI DEBUG:L1CACHE_I_PREFETCH_EN",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_l1cache_i_prefetch_dis, 1, 0, sbi_l1cache_i_prefetch_dis,"OpenSBI DEBUG:L1CACHE_I_PREFETCH_DIS",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_l1cache_d_prefetch_en, 1, 0, sbi_l1cache_d_prefetch_en,"OpenSBI DEBUG:L1CACHE_D_PREFETCH_EN",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_l1cache_d_prefetch_dis, 1, 0, sbi_l1cache_d_prefetch_dis,"OpenSBI DEBUG:L1CACHE_D_PREFETCH_DIS",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_non_blocking_load_store_en, 1, 0, sbi_non_blocking_load_store_en,"OpenSBI DEBUG:NON_BLOCKING_LOAD_STORE_EN",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_non_blocking_load_store_dis, 1, 0, sbi_non_blocking_load_store_dis,"OpenSBI DEBUG:NON_BLOCKING_LOAD_STORE_DIS",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_write_around_en, 1, 0, sbi_write_around_en,"OpenSBI DEBUG:WRITE_AROUND_EN",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_write_around_dis, 1, 0, sbi_write_around_dis,"OpenSBI DEBUG:WRITE_AROUND_DIS",NULL, NULL);


int sbi_ext_set_timer(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	int ret = 0;
	
	sbi_set_timer(0);
	
	return ret;
}

int sbi_ext_get_spec_version(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	int ret = 0;
	long value;
	
	value = sbi_get_spec_version();
	printf("  %ld\n", value);
	
	return ret;
}

int sbi_ext_get_impl_id(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	int value,ret = 0;
	
	value = sbi_get_impl_id();
	printf("  %d\n", value);

	return ret;
}
#ifdef CONFIG_SBI_V01
int sbi_ext_console_putchar(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	int ret = 0;
	int ch = 0;
	
	sbi_console_putchar(ch);
	
	return ret;
}
#endif
U_BOOT_CMD_COMPLETE(sbi_set_timer, 0, 0, sbi_ext_set_timer,"OpenSBI DEBUG:SET_TIMER",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_get_spec_version, 0, 0, sbi_ext_get_spec_version,"OpenSBI DEBUG:",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_get_impl_id, 0, 0, sbi_ext_get_impl_id,"OpenSBI DEBUG:",NULL, NULL);
U_BOOT_CMD_COMPLETE(sbi_console_putchar, 0, 0, sbi_ext_console_putchar,"OpenSBI DEBUG:",NULL, NULL);


#endif
