// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022, Renesas Electronics Corporation. All rights reserved.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#ifdef CONFIG_SPL
#define CONFIG_SPL_STACK	    	0x00030000
#define CONFIG_SPL_MAX_SIZE		    0x00013000

#define CONFIG_SPL_BSS_START_ADDR	0x48000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x00100000

#ifdef CONFIG_SPL_NOR_SUPPORT
#define CONFIG_SYS_UBOOT_BASE       0x20020000
#endif

#ifdef CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME		"u-boot.itb"
#endif

#ifdef CONFIG_SPL_BUILD
/* use CONFIG_XIP configuration for Elimination of atomic instructions */
#define CONFIG_XIP  1
#endif /* CONFIG_SPL_BUILD */

#endif /*CONFIG_SPL */

/*
 * CPU and Board Configuration Options
 */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_CBSIZE	2048	/* Console I/O Buffer Size */

/*
 * Print Buffer Size
 */
#define CONFIG_SYS_PBSIZE	\
	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

/*
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	64

/*
 * max number of command args
 */
#define CONFIG_SYS_MAXARGS	64

/*
 * Boot Argument Buffer Size
 */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE


/* Init Stack Pointer */
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_TEXT_BASE - GENERATED_GBL_DATA_SIZE)


/* boot option */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

/* PHY needs a longer autoneg timeout */
#define PHY_ANEG_TIMEOUT		20000

/* SDHI clock freq */
#define CONFIG_SH_SDHI_FREQ		133333333

#define DRAM_RSV_SIZE			0x08000000
#define CONFIG_SYS_SDRAM_BASE		(0x40000000 + DRAM_RSV_SIZE)
#define CONFIG_SYS_SDRAM_SIZE		(CONFIG_RZF_DDR_SIZE - DRAM_RSV_SIZE)
#define CONFIG_MAX_MEM_MAPPED		(CONFIG_RZF_DDR_SIZE - DRAM_RSV_SIZE)
#define CONFIG_SYS_MALLOC_LEN		(64 * 1024 * 1024)
#define CONFIG_SYS_LOAD_ADDR		0x58000000
#define CONFIG_LOADADDR			CONFIG_SYS_LOAD_ADDR // Default load address for tfpt,bootp...
#define CONFIG_VERY_BIG_RAM

#define CONFIG_SYS_MONITOR_BASE		0x00000000
#define CONFIG_SYS_MONITOR_LEN		(1 * 1024 * 1024)
#define CONFIG_SYS_BOOTM_LEN		(64 << 20)

/* The HF/QSPI layout permits up to 1 MiB large bootloader blob */
#define CONFIG_BOARD_SIZE_LIMIT		1048576

/* ENV setting */
#define CONFIG_EXTRA_ENV_SETTINGS	\
	"usb_pgood_delay=2000\0" \
	"bootm_size=0x10000000 \0" \
	"prodsdbootargs=setenv bootargs rw rootwait earlycon root=/dev/mmcblk1p2 \0" \
	"prodemmcbootargs=setenv bootargs rw rootwait earlycon root=/dev/mmcblk0p2 \0" \
	"bootimage=unzip 0x4A080000 0x48080000; booti 0x48080000 - 0x48000000 \0" \
	"emmcload=ext4load mmc 0:2 0x48080000 boot/Image;ext4load mmc 0:2 0x48000000 boot/r9a07g043f01-smarc.dtb;run prodemmcbootargs \0" \
	"sd1load=ext4load mmc 1:2 0x48080000 boot/Image;ext4load mmc 1:2 0x48000000 boot/r9a07g043f01-smarc.dtb;run prodsdbootargs \0" \
	"bootcmd_check=if mmc dev 1; then run sd1load; else run emmcload; fi \0"

#define CONFIG_BOOTCOMMAND	"env default -a; run bootcmd_check; run bootimage"

/* For board */
/* Ethernet RAVB */
#define CONFIG_BITBANGMII_MULTI

#endif /* __CONFIG_H */
