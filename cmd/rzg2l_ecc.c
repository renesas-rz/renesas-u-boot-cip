// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Renesas Electronics Corporation
 */

/*
 * DRAM ECC support
 */

#include <common.h>
#include <cpu_func.h>
#include <command.h>
#include <linux/log2.h>
#include <linux/io.h>
#include <mapmem.h>
#include <dm.h>

enum RZG2L_ECC_MODE {
	ECC_DISABLE = 0,
	ECC_ENABLED = 1,
	ECC_DETECT = 2,
	ECC_DETECT_CORRECT = 3
};

struct DDRMC_REG {
	unsigned long offset;
};

unsigned long g2l_ddrmc_regs[] = {
	0x174,	/* For ECC_ENABLE */
	0x1ac,	/* For Non-ECC region 0 */
	0x1b4,	/* For endisable Non-ECC region */
	0x218,	/* For Checking controller busy */
};

unsigned long v2l_ddrmc_regs[] = {
	0x18c,
	0x1c4,
	0x1cc,
	0x230,
};

#define RZG2L_DDR_CTRL_BUSY		0x1
#define RZG2L_DDR_BASE_ADDR		0x40000000

struct soc_info {
	unsigned long soc_reg;
	uint32_t soc_id;
	unsigned long memc_base;
	unsigned long *regs;
} soc_infos[] = {
	/*  rzg2l  */
	{.soc_reg = 0x11020A04, .soc_id = 0x841c447, .memc_base = 0x11410000,
		.regs = g2l_ddrmc_regs},
	/*  rzg2ul */
	{.soc_reg = 0x11020A04, .soc_id = 0x8450447, .memc_base = 0x11410000,
		.regs = g2l_ddrmc_regs},
	/*  rzv2l  */
	{.soc_reg = 0x11020A04, .soc_id = 0x8447447, .memc_base = 0x11410000,
		.regs = v2l_ddrmc_regs},
	/*  rzfive */
	{.soc_reg = 0x11020A04, .soc_id = 0x8450447, .memc_base = 0x11410000,
		.regs = g2l_ddrmc_regs},
	/*  rzfive */
	{.soc_reg = 0x11020A04, .soc_id = 0x847c447, .memc_base = 0x11410000,
		.regs = g2l_ddrmc_regs},
};

static struct soc_info* rzg2l_get_cpu_type(void)
{
	int i;

	for (i = 0 ; i < sizeof(soc_infos)/sizeof(struct soc_info) ; i++) {
		if (readl(soc_infos[i].soc_reg) == soc_infos[i].soc_id)
			return &soc_infos[i];
	}

	return NULL;
}

static enum RZG2L_ECC_MODE rzg2l_ecc_mode(struct soc_info* si)
{
	uint32_t val;

	val = readl((uint32_t *)(si->memc_base + si->regs[0]));
	val = (val >> 24) & 0x3;

	return val;
}

static void rzg2l_ecc_help(void)
{
	printf("usage: g2lecc <command> <function> [<args>]\n");
	printf("\n");
	printf("There is 1 function supported in this SoC\n"
		"  -n,--nonecc    Non-ECC function. The ECC function is\n"
		"                 enabled in all DDR memory by default, if \n"
		"                 user choose the ECC function. This Non-ECC\n"
		"                 function can specify 2 memory regions not \n"
		"                 to use ECC function.\n\n");
	printf("Supported commands\n");
	printf("  set            Set Non-ECC region.\n"
		"                 args with -n <start_address> <end_address> <id>\n"
		"                 <start_address> : the start address of Non-ECC area.\n"
		"                 <end_address> : the end address of Non-ECC area.\n"
		"                 <id> : the area id for the Non-ECC area. \"id\" \n"
		"                        should be 1 or 2. This SoC support to set 2\n"
		"                        Non-ECC areas. Select the Non-ECC areas by \n"
		"                        setting id.\n");
	printf("  rm             Remove Non-ECC region\n"
		"                 arg with -n <id>\n"
		"                 <id> : id for the Non-ECC region\n");
	printf("  list           List Non-ECC region\n"
		"                 list -n: list enabled nonecc regions.\n");
	printf("  help           Print this help log.\n");
}

static int rzg2l_non_ecc_area_set(struct soc_info *si, unsigned long start_addr, unsigned long end_addr, int id)
{
	uint32_t val;
	uint32_t retry = 0xffff;
	unsigned long non_ecc_reg;

	if ((id < 1) || (id > 2))
		return -1;

	if ((end_addr - start_addr) < 0x200000) {
		printf("Enabled regions may not be adjacent and should be a minimum of 2 MB apart.");
		return -1;
	}

	id -= 1;

	non_ecc_reg = si->regs[1] + id * 4;
	start_addr -= RZG2L_DDR_BASE_ADDR;
	end_addr -= RZG2L_DDR_BASE_ADDR;

	/* wait ddr controller to be idle */
	do{
		val = readl((uint32_t *)(si->memc_base + si->regs[3]));
	} while(retry-- && (val & RZG2L_DDR_CTRL_BUSY));
	if (!retry)
		return -1;

	/* set the non_ecc area */
	val = ((end_addr >> 20) << 16) | (start_addr >> 20);
	writel(val, (uint32_t *)(si->memc_base + non_ecc_reg));

	/* enable the non_ecc area */
	val = readl((uint32_t *)(si->memc_base + si->regs[2]));
	val |= (1 << id);
	writel(val, (uint32_t *)(si->memc_base + si->regs[2]));

	return 0;
}

static int rzg2l_non_ecc_area_rm(struct soc_info *si, int id)
{
	uint32_t val;
	uint32_t retry = 0xffff;

	if ((id < 1) || (id > 2))
		return -1;

	/* wait ddr controller to be idle */
	do{
		val = readl((uint32_t *)(si->memc_base +si->regs[3]));
	} while(retry-- && (val & RZG2L_DDR_CTRL_BUSY));
	if (!retry)
		return -1;

	id -= 1;

	/* disable the non_ecc area */
	val = readl((uint32_t *)(si->memc_base + si->regs[2]));
	val &= ~(1 << id);
	writel(val, (uint32_t *)(si->memc_base + si->regs[2]));

	return 0;
}

static int rzg2l_non_ecc_area_ls(struct soc_info *si)
{
	int i;
	uint32_t val;
	unsigned long non_ecc_reg;
	unsigned long start_addr, end_addr;

	printf("Registered Non-ECC areas:\n");
	val = readl((uint32_t *)(si->memc_base + si->regs[2]));
	for (i = 0 ; i < 2 ; i++) {
		if (!(val & (1 << i)))
			continue;

		non_ecc_reg = si->regs[1] + i * 4;
		start_addr = readl((uint32_t *)(si->memc_base + non_ecc_reg));
		end_addr = (start_addr & 0xffff0000U) << 4;
		start_addr = (start_addr & 0xffffU) << 20;

		printf("\t id: %d area: 0x%08lx - 0x%08lx\n", i+1, \
				start_addr + RZG2L_DDR_BASE_ADDR, \
				end_addr + RZG2L_DDR_BASE_ADDR);
	}

	return 0;
}

static int rzg2l_do_ecc(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	int id;
	unsigned long start_addr;
	unsigned long end_addr;
	uint32_t arg;
	struct soc_info *si = NULL;

	if (argc < 2)
		goto show_help;

	si = rzg2l_get_cpu_type();
	if (!si) {
		printf("ECC: Command not support for this platform yet\n");
		return 1;
	}

	if (rzg2l_ecc_mode(si) == ECC_DISABLE) {
		printf("ECC is not enabled\n");
		return 1;
	}

	if (!strcmp(argv[1], "set")) {
		if (argc < 6)
			goto show_help;
		if (strcmp(argv[2], "-n") && strcmp(argv[2], "--nonecc"))
			goto show_help;

		start_addr = simple_strtoull(argv[3], NULL, 16);
		end_addr = simple_strtoull(argv[4], NULL, 16);
		id = simple_strtoul(argv[5], NULL, 10);
		if(rzg2l_non_ecc_area_set(si, start_addr, end_addr, id))
			printf("Set non_ecc area failed\n");
	} else if (!strcmp(argv[1], "rm")) {
		if (argc < 4)
			goto show_help;
		if (strcmp(argv[2], "-n") && strcmp(argv[2], "--nonecc"))
			goto show_help;

		id = simple_strtoul(argv[3], NULL, 10);
		if (rzg2l_non_ecc_area_rm(si, id))
			printf("Remove non_ecc area failed\n");
	} else if (!strcmp(argv[1], "list")) {
		if (argc < 3)
			goto show_help;
		if (strcmp(argv[2], "-n") && strcmp(argv[2], "--nonecc"))
			goto show_help;

		rzg2l_non_ecc_area_ls(si);
	} else
		goto show_help;

	return 0;

show_help:
		rzg2l_ecc_help(); /* print usage log*/

	return 1;
}

U_BOOT_CMD(
	g2lecc,	6,	0,	rzg2l_do_ecc,
	"ECC function support on RZ/G2L series SoC\n", NULL
);
