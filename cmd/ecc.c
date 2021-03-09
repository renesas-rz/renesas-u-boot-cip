// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Renesas Electronics Corporation
 */

/*
 * DRAM ECC support
 */

#include <common.h>
#include <command.h>
#include <linux/log2.h>
#include <linux/io.h>

#define	DFUSAAREACR		0xE6785000	/* DRAM FuSa Area Conf */
#define	DECCAREACR		0xE6785040	/* DRAM ECC Area Conf */
#define	NUM_DAREA		16
#define	ECC_ENABLE_MASK_BIT	(1<<31)

/* As the saddr, specify high-memory address (> 4 GB) */
#define	FUSAAREACR(en, size, saddr)	\
	(((uint32_t)en << 31) | ((uint32_t)size << 24) | (uint32_t)(((uintptr_t)saddr) >> 12))
#define	ECCAREACR(ecc, saddr) \
	(((uint32_t)ecc << 31) | (uint32_t)(((uintptr_t)saddr) >> 12))

#define SIZE_1MB		(1024 * 1024)
#define MAX_BLOCK_SIZE_MB 	(512)
#define DRAM_ADDR_BASE 		(0x400000000)

/* ecc_check_address(unsigned long long addr)
 * Check if address is 36 bits
 * Base address is defined as DRAM_ADDR_BASE
 */
int ecc_check_address(unsigned long long addr)
{
	if (addr < DRAM_ADDR_BASE)
		return 0;
	return 1;
}

/* ecc_list_setting()
 * Check All ECC registers then print value of them in format 
 * that User can understand.
 */
void ecc_list_setting(void)
{
	int i;
	uint32_t dfusaareacr;
	uint32_t deccareacr;
	printf("ECC: List of ECC area:\n");
	for (i = 0; i < NUM_DAREA; i++){
		dfusaareacr = readl(((uint32_t *)DFUSAAREACR + i));
		deccareacr = readl(((uint32_t *)DECCAREACR + i));
		if (dfusaareacr & ECC_ENABLE_MASK_BIT){
			printf("%2d: %s : Data from 0x%llx : ECC from 0x%llx : size  %d MB : mode %d bits\n",i, 
							dfusaareacr & ECC_ENABLE_MASK_BIT ? "Enabled" : "Disabled",
							(unsigned long long)(dfusaareacr<<8) << 4,
							(unsigned long long)(deccareacr<<8) << 4,
							1<<((dfusaareacr>>24)&0xF),
							deccareacr&ECC_ENABLE_MASK_BIT ? 64 : 8);
		} else {
			printf("%2d: Disabled\n",i);
		}
		
	}
}

/* ecc_add_configure( unsigned long long data_addr, unsigned long long ecc_addr, unsigned long block_size, unsigned long mode)
 * Set registers to enable an ECC area
 * data_addr : Data start address must be 36 bits address
 * ecc_addr : ECC start address must be 36 bits address
 * size : size of Data area 
 * mode : ECC mode : 64 / 8 : 64bitsdata/8bitsECC OR 8bitsdata/5bitsECC
 */
int ecc_add_configure( unsigned long long data_addr, unsigned long long ecc_addr, unsigned long block_size, unsigned long mode)
{
	int i;
	uint32_t dfusaareacr;
	uint32_t deccareacr;

	for (i = 0; i < NUM_DAREA; i++){
		if (readl(((uint32_t *)DFUSAAREACR + i)) & ECC_ENABLE_MASK_BIT){
			if(i == NUM_DAREA -1)
				goto err_no_resource;
			continue;
		}

		if(mode == 64) {
			writel(FUSAAREACR(1, ilog2(block_size), data_addr),((uint32_t *)DFUSAAREACR + i));
			writel(ECCAREACR(1, ecc_addr) ,((uint32_t *)DECCAREACR + i));
		} else if (mode == 8) {
			writel(FUSAAREACR(1, ilog2(block_size), data_addr),((uint32_t *)DFUSAAREACR + i));
			writel(ECCAREACR(0, ecc_addr) ,((uint32_t *)DECCAREACR + i));
		} else {
			printf("ECC: ERROR: not support mode %ld\n", mode);
			return 0;
		}

		dfusaareacr = readl(((uint32_t *)DFUSAAREACR + i));
		deccareacr = readl(((uint32_t *)DECCAREACR + i));
		if (dfusaareacr & ECC_ENABLE_MASK_BIT){
		printf("ECC: Enabled an ECC area  : Data from 0x%llx : ECC from 0x%llx : size  %d MB : mode %d bits\n",
										(unsigned long long)(dfusaareacr<<8) << 4,
										(unsigned long long)(deccareacr<<8) << 4,
										1<<((dfusaareacr>>24)&0xF),
										deccareacr&ECC_ENABLE_MASK_BIT ? 64 : 8);
		} else {
			printf("ECC: Failed to enable ECC at 0x%llx\n",data_addr);
		}

		break;
	}

	return 1;

err_no_resource:
	printf("ECC: ERROR : Not Enough Resource, only support %d Areas\n", NUM_DAREA);
	return 0;
}

/* ecc_add_setting( unsigned long long data_start_addr, unsigned long long ecc_start_addr,
 *										unsigned long size, unsigned long mode)
 * Get user input and call ecc_add_configure to enable ECC areas
 * data_start_addr : Data start address must be 36 bits address
 * ecc_start_addr : ECC start address must be 36 bits address
 * size : size of Data area 
 * mode : ECC mode : 64 / 8 : 64bitsdata/8bitsECC OR 8bitsdata/5bitsECC
 *
 *  ________________________  data_start_addr
 * |                        |   |
 * |                        |   |
 * |     Data area          |    > size
 * |                        |   |
 * |________________________|   |
 * |                        |
 * |  this gap will be 0    |
 *    if ecc_start_addr     
 *    not set               
 * |                        |
 * |________________________| ecc_start_addr
 * |                        |   |
 * |       ECC area         |   |
 * |                        |    > auto-caculated size depend on mode
 * |                        |   |
 * |________________________|   |
 */
 
void ecc_add_setting( unsigned long long data_start_addr, unsigned long long ecc_start_addr,
			unsigned long size, unsigned long mode)
{
	unsigned int block_size = MAX_BLOCK_SIZE_MB;
	unsigned long long data_addr= data_start_addr;
	unsigned long long ecc_addr = ecc_start_addr;

	if(ecc_addr == 0)
		ecc_addr = data_start_addr + size * SIZE_1MB;

	while ((block_size >= 1) && (size != 0))
	{
		if (size >= block_size){
			ecc_add_configure (data_addr, ecc_addr, block_size, mode);
			data_addr = data_addr + block_size* SIZE_1MB;
			if (mode == 64){
				ecc_addr = ecc_addr + block_size*SIZE_1MB/8;
			} else if (mode == 8) {
				ecc_addr = ecc_addr + block_size*SIZE_1MB;
			}
			size = size - block_size;
		} else {
			block_size = block_size/2;
		}
	}
}

/* ecc_rm_setting(unsigned int id)
 * Remove an ECC area by id
 * id : input id , can get id by 'ecc list'
 */
void ecc_rm_setting(unsigned int id)
{
	uint32_t dfusaareacr;

	if (id > NUM_DAREA){
		printf("ECC: Not support ECC at id %d\n", id);
		return;
	}

	writel(0,((uint32_t *)DFUSAAREACR + id));
	writel(0,((uint32_t *)DECCAREACR + id));
	
	dfusaareacr = readl(((uint32_t *)DFUSAAREACR + id));

	if (dfusaareacr & ECC_ENABLE_MASK_BIT){
		printf("ECC: Failed to disable ECC at id %d\n", id);
	} else {
		printf("ECC: Disabled ECC at id %d\n", id);
	}
}

/* ecc_help()
 * print usage log
 */
void ecc_help(void)
{
	printf(	"Enable/disable ECC for a memory area\n"
		"ecc list\n"
		"    - list all ECC areas enalbed\n"
		"ecc add <data address> [ecc address] <size> <mode>\n"
		"    - add an ECC area start from address with size\n"
		"      <data address> : 36 bits data start address\n"
		"      [ecc address] : 36 bits ecc start address, if it is not set\n"
		"                      ecc will be placed right after data area\n"
		"      <size> : size in MB\n"
		"               ECC support in blocksize of 2^n MB (1MB, 2MB, 4MB,..., 512MB)\n"
		"               if size is not 2^n, many blocks will be used\n"
		"      <mode> : 8 / 64 : 8bitsdata/5bitsECC Or 64bitsdata/8bitsECC\n"
		"ecc rm <id>\n"
		"    - disable an ECC area with id (use 'ecc list' to get id)\n"
		"ecc help\n"
		"    - print this help log\n" );
}

/* do_ecc(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
 * Main process parse user command
 * argc : number of parameters
 * argv : array of parameters
 */
int do_ecc(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#ifdef CONFIG_TARGET_EK874
	if (strncmp(argv[1], "list", 4) == 0) {
		/* List all Register ID with ECC status*/
		ecc_list_setting();
	} else if (strncmp(argv[1], "add", 3) == 0) {
		/* Add an ECC area*/
		unsigned long long data_addr = 0;
		unsigned long long ecc_addr = 0;
		unsigned long size;
		unsigned long mode;
		data_addr = simple_strtoull(argv[2], NULL, 16);

		if (argc == 6) {
			ecc_addr = simple_strtoull(argv[3], NULL, 16);
			if (ecc_check_address(ecc_addr) == 0){
				printf("ECC: Wrong address, please use 36 bits address start from 0x%llx\n", 
					(unsigned long long)DRAM_ADDR_BASE);
				return 0;
			}
			size = simple_strtoul(argv[4], NULL, 10);
			mode = simple_strtoul(argv[5], NULL, 10);
		} else if (argc == 5){
			ecc_addr = 0;
			size = simple_strtoul(argv[3], NULL, 10);
			mode = simple_strtoul(argv[4], NULL, 10);
		} else {
			ecc_help();
			return 0;
		}

		if (ecc_check_address(data_addr))
			ecc_add_setting(data_addr, ecc_addr, size, mode);
		else
			printf("ECC: Wrong address, please use 36 bits address start from 0x%llx\n", 
					(unsigned long long)DRAM_ADDR_BASE);

	} else if (strncmp(argv[1], "rm", 2) == 0) {
		/* Remove an ECC area*/
		unsigned long id;
		id = simple_strtoul(argv[2], NULL, 10);
		ecc_rm_setting(id);
	} else {
		/* print usage log*/
		ecc_help();
	}
#else
	printf("ECC: Command not support for this platform yet\n");
#endif
	return 1;
}

U_BOOT_CMD(
	ecc,	6,	0,	do_ecc,
	"Enable/disable ECC for a memory area\n",
	"ecc list\n"
	"    - list all ECC areas enalbed\n"
	"ecc add <data address> [ecc address] <size> <mode>\n"
	"    - add an ECC area start from address with size\n"
	"      <data address> : 36 bits data start address\n"
	"      [ecc address] : 36 bits ecc start address, if it is not set\n"
	"                      ecc will be placed right after data area\n"
	"      <size> : size in MB\n"
	"               ECC support in blocksize of 2^n MB (1MB, 2MB, 4MB,..., 512MB)\n"
	"               if size is not 2^n, many blocks will be used\n"
	"      <mode> : 8 / 64 : 8bitsdata/5bitsECC Or 64bitsdata/8bitsECC\n"
	"ecc rm <id>\n"
	"    - disable an ECC area with id (use 'ecc list' to get id)\n"
	"ecc help\n"
	"    - print this help log\n"
);
