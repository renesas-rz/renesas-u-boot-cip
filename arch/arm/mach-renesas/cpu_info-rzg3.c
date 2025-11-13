// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021,2023 Renesas Electronics Corporation
 *
 */

#include <mach/renesas.h>
#include <asm/io.h>
#include <linux/libfdt.h>

#define RENESAS_CPU_TYPE_R9A08G046	0x87d9447 //G3L

/* SYSC */
#define SYSC_BASE			(0x11020000)
#define SYSC_LSI_DEVID		(SYSC_BASE + 0xA04)

/* If the firmware passed a device tree, use it for soc identification. */
extern u64 rcar_atf_boot_args[];

/* CPU information table */
struct tfa_info {
	const char *soc_name;
	const char *cpu_name;
	u32 cpu_type;
};

static const struct tfa_info tfa_info[] = {
	{ "renesas,r9a08g046", "R9A08G046", RENESAS_CPU_TYPE_R9A08G046}, //g3l
};

static const struct tfa_info invalid_tfa_info = { NULL, "(invalid)", 0 };

u32 get_devid(void)
{
	return readl(SYSC_LSI_DEVID);
}

static const struct tfa_info *get_tfa_info(void)
{
	void *atf_fdt_blob = (void *)(rcar_atf_boot_args[1]);
	
	if( atf_fdt_blob == NULL ) {
		return NULL;
	}

	u32 magic = fdt_magic(atf_fdt_blob);
	
	if (magic == FDT_MAGIC) {
		unsigned int i;
		for (i = 0; i < ARRAY_SIZE(tfa_info); i++) {
			int ret = fdt_node_check_compatible(atf_fdt_blob, 0, tfa_info[i].soc_name);
			if (!ret) {
				return &tfa_info[i];
			}
		}
	}
	return NULL;
}

static const struct tfa_info *get_dev_info(void)
{
	int i=0;
	u32 cpu_type = get_devid();

	for (i = 0; i < ARRAY_SIZE(tfa_info); i++) {
		if (tfa_info[i].cpu_type == cpu_type) {
			return &tfa_info[i];
		}
	}
	return &invalid_tfa_info;
}

const u8 *rzg_get_cpu_name(void)
{
	const struct tfa_info *tfa_info = get_tfa_info();
	if( tfa_info != NULL){
		return tfa_info->cpu_name;
	}
	return get_dev_info()->cpu_name;
}

u32 renesas_get_cpu_type(void)
{
	const struct tfa_info *tfa_info = get_tfa_info();
	if( tfa_info != NULL){
		return tfa_info->cpu_type;
	}
	return get_dev_info()->cpu_type;
}

u32 renesas_get_cpu_rev_integer(void)
{
	return (get_devid() >> 28) + 1;
}

u32 renesas_get_cpu_rev_fraction(void)
{
	return 0;
}
