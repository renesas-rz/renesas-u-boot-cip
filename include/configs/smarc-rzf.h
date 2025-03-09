// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Renesas Electronics Corporation
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#ifdef CONFIG_SPL
#ifdef CONFIG_SPL_NOR_SUPPORT
#define CFG_SYS_UBOOT_BASE       0x20020000
#endif

#ifdef CONFIG_SPL_MMC
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
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	64


/* boot option */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

/* PHY needs a longer autoneg timeout */
#define PHY_ANEG_TIMEOUT		20000

#define DRAM_RSV_SIZE			0x08000000
#define CONFIG_SYS_SDRAM_BASE		(0x40000000 + DRAM_RSV_SIZE)
#define CONFIG_SYS_SDRAM_SIZE		(CONFIG_RZF_DDR_SIZE - DRAM_RSV_SIZE)
#define CONFIG_MAX_MEM_MAPPED		(CONFIG_RZF_DDR_SIZE - DRAM_RSV_SIZE)
#define CONFIG_VERY_BIG_RAM

/* The HF/QSPI layout permits up to 1 MiB large bootloader blob */
#define CONFIG_BOARD_SIZE_LIMIT		3145728

/* ENV setting */
#define CONFIG_EXTRA_ENV_SETTINGS	\
	"bootm_size=0x10000000\0" \
		"bootargs root =" \
			"/dev/mmcblk0p2" \
		"bootcmd_mmc=" \
			"setenv bootargs root=/dev/mmcblk0p2 rw rootwait;" \
			"ext4load mmc 1:1 0x48080000 Image; " \
			"ext4load mmc 1:1 0x48000000 "CONFIG_DEFAULT_FDT_FILE"; " \
			"booti 0x48080000 - 0x48000000" \
		"bootcmd=" \
			"run bootcmd_mmc"

/* For board */
/* Ethernet RAVB */
#define CONFIG_BITBANGMII_MULTI

#endif /* __CONFIG_H */
