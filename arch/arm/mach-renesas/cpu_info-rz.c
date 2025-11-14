// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021,2023 Renesas Electronics Corporation
 *
 */

#include <mach/renesas.h>
#include <asm/io.h>
#include <linux/libfdt.h>
#include <stdio.h>

/* DEBUG: Temporary logging - remove before final commit */
#define CPU_INFO_DEBUG 0

/* SYSC register addresses vary by platform family */
#if defined(CONFIG_R9A09G047) || defined(CONFIG_R9A09G057) || defined(CONFIG_R9A09G056)
	/* G3E, V2H, V2N */
	#define SYSC_LSI_DEVID		(0x10430000 + 0x304)
#else
	/* G2L, G2UL, G2LC, V2L, G3S */
	#define SYSC_LSI_DEVID		(0x11020000 + 0xA04)
#endif

/* DEVID register: bits 31:28 = revision, bits 27:0 = fixed product ID */
#define DEVID_MASK			0x0FFFFFFF

/* If the firmware passed a device tree, use it for soc identification. */
extern u64 rcar_atf_boot_args[];

/* CPU information table */
struct tfa_info {
	const char *soc_name;
	const char *cpu_name;
	u32 cpu_type;
};

static const struct tfa_info tfa_info[] = {
	/* G2 family */
	{ "renesas,r9a07g044l", "R9A07G044L", RENESAS_CPU_TYPE_R9A07G044L }, //g2l
	{ "renesas,r9a07g043u", "R9A07G043U", RENESAS_CPU_TYPE_R9A07G043U }, //g2ul
	{ "renesas,r9a07g044c", "R9A07G044C", RENESAS_CPU_TYPE_R9A07G044C }, //g2lc
	{ "renesas,r9a07g054l", "R9A07G054L", RENESAS_CPU_TYPE_R9A07G054L }, //v2l
	/* G3/V2 family */
	{ "renesas,r9a09g047", "R9A09G047", RENESAS_CPU_TYPE_R9A09G047 },   //g3e
	{ "renesas,r9a08g045s", "R9A08G045S", RENESAS_CPU_TYPE_R9A08G045S }, //g3s
	{ "renesas,r9a09g057", "R9A09G057", RENESAS_CPU_TYPE_R9A09G057 },   //v2h
	{ "renesas,r9a09g056", "R9A09G056", RENESAS_CPU_TYPE_R9A09G056 },   //v2n
};

static const struct tfa_info invalid_tfa_info = { NULL, "(invalid)", 0 };

u32 get_devid(void)
{
	u32 devid = readl(SYSC_LSI_DEVID);
#if CPU_INFO_DEBUG
	printf("DEBUG: get_devid() raw=0x%08x from addr=0x%08x\n", devid, SYSC_LSI_DEVID);
#endif
	return devid;
}

static const struct tfa_info *get_tfa_info(void)
{
	void *atf_fdt_blob = (void *)(rcar_atf_boot_args[1]);
	
	if (atf_fdt_blob == NULL) {
		#if CPU_INFO_DEBUG
		printf("DEBUG: get_tfa_info() no ATF FDT blob pointer\n");	
		#endif
		return NULL;
	}

	if (fdt_magic(atf_fdt_blob) == FDT_MAGIC) {
		unsigned int i;
		for (i = 0; i < ARRAY_SIZE(tfa_info); i++) {
			if (!fdt_node_check_compatible(atf_fdt_blob, 0,tfa_info[i].soc_name)){
				#if CPU_INFO_DEBUG
				printf("DEBUG: get_tfa_info() found matching ATF FDT compatible: %s\n", tfa_info[i].soc_name);	
				#endif
				return &tfa_info[i];
			}
		}
	}
	#if CPU_INFO_DEBUG
		printf("DEBUG: get_tfa_info() no matching ATF FDT compatible\n");	
		#endif
	return NULL;
}

static const struct tfa_info *get_dev_info(void)
{
	int i = 0;
	// Get the CPU type by masking off the revision bits 27:0
	u32 cpu_type = get_devid() & DEVID_MASK;

#if CONFIG_R9A07G044C
	// For R9A07G044C (G2LC), the DEVID is the same as R9A07G044L (G2L)
	// So we need to adjust the cpu_type to match R9A07G044C
	if( cpu_type == RENESAS_CPU_TYPE_R9A07G044L && renesas_get_cpu_rev_integer() == 2) {
		cpu_type = RENESAS_CPU_TYPE_R9A07G044C;
	}
#endif

#if CPU_INFO_DEBUG
	printf("DEBUG: get_dev_info() masked cpu_type=0x%08x\n", cpu_type);
	u32 rev = renesas_get_cpu_rev_integer();
	printf("DEBUG:   CPU revision=0x%08x\n", rev);
#endif
	for (i = 0; i < ARRAY_SIZE(tfa_info); i++) {
#if CPU_INFO_DEBUG
		printf("DEBUG:   comparing with tfa_info[%d].cpu_type=0x%08x (%s)\n",
		       i, tfa_info[i].cpu_type, tfa_info[i].cpu_name);
#endif
		if (tfa_info[i].cpu_type == cpu_type) {
#if CPU_INFO_DEBUG
			printf("DEBUG:   MATCH found: %s\n", tfa_info[i].cpu_name);
#endif
			return &tfa_info[i];
		}
	}
#if CPU_INFO_DEBUG
	printf("DEBUG:   NO MATCH - returning invalid\n");
#endif
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
	u32 val = (get_devid() >> 28) + 1;
#if CPU_INFO_DEBUG
	printf("DEBUG: renesas_get_cpu_rev_integer() returning 0x%08x\n", val);
#endif
	return val;
}

u32 renesas_get_cpu_rev_fraction(void)
{
	u32 val = 0;
#if CPU_INFO_DEBUG
	printf("DEBUG: renesas_get_cpu_rev_fraction() returning 0x%x\n", val);
#endif
	return val;
}
