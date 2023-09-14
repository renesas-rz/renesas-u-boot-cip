/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * include/configs/rzv2ma.h
 *     This file is rzv2ma board configuration.
 *
 * Copyright (C) 2022 Renesas Electronics Corporation
 */

#ifndef __RZV2MA_H
#define __RZV2MA_H

#include <asm/arch/rmobile.h>

/* Enter the kernel on EL1 */
#define CONFIG_ARMV8_SWITCH_TO_EL1

/* Ethernet RAVB */
#define CONFIG_BITBANGMII_MULTI

/* Generic Timer Definitions (use in assembler source) */
#define COUNTER_FREQUENCY	24000000	/* 24MHz from CNT_CLK */

/* Environment in eMMC, at the end of 2nd "boot sector" */
#undef CONFIG_SYS_MMC_ENV_DEV
#define CONFIG_SYS_MMC_ENV_DEV		1
#undef CONFIG_SYS_MMC_ENV_PART
#define CONFIG_SYS_MMC_ENV_PART		2

#define CONFIG_REMAKE_ELF

#undef CONFIG_DISPLAY_CPUINFO
#define CONFIG_SYS_INIT_SP_ADDR 0x000BFFFC

/* Generic Interrupt Controller Definitions */
#define CONFIG_GICV2
#define GICD_BASE	0x82010000
#define GICC_BASE	0x82020000

/* MEMORY */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_SDRAM_SIZE		0x80000000
#define CONFIG_SYS_LOAD_ADDR		0x40007fc0
#define CONFIG_SYS_MONITOR_LEN		(768 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(1 * 1024 * 1024)


/* ENV setting */
#define CONFIG_ENV_SECT_SIZE	(128 * 1024)
#define CONFIG_EXTRA_ENV_SETTINGS \
	"ethaddr=02:11:22:33:44:55\0" \
	"ipaddr=192.168.1.11\0" \
	"serverip=192.168.1.10\0" \
	"gatewayip=192.168.0.1\0" \
	"netmask=255.255.255.0\0" \
	"loadaddr=0x000080000\0" \
	"fdt_addr=0x000000000\0" \
	"fdt_high=0x028000000\0" \
	"fdt_file=r9a09g055-v2maevk2.dtb\0" \
	"kernel=Image\0" \
	"core1addr=0x0\0" \
	"core1_vector=0x0\0" \
	"bootargs_sd=setenv bootargs root=/dev/mmcblk0p2 rootwait rootfstype=ext4 rw\0" \
	"bootsd=run bootargs_sd;" \
	"ext4load mmc 0:2 ${loadaddr} boot/${kernel};ext4load mmc 0:2 ${fdt_addr} boot/${fdt_file};booti ${loadaddr} - ${fdt_addr}\0" \
	"bootargs_nfs=setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:/nfs/rzv2m,nfsvers=3 ip=${ipaddr}:${serverip}::${netmask}:rzv2m:eth0\0" \
	"bootnfs=run bootargs_nfs;" \
	"tftp ${loadaddr} ${kernel};tftp ${fdt_addr} ${fdt_file};booti ${loadaddr} - ${fdt_addr}\0" \
	"bootcmd=run bootsd\0" \

#endif /* __RZV2M_H */
