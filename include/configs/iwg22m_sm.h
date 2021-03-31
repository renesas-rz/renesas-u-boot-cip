/*
 * Copyright (c) 2016 iWave Systems Technologies Pvt. Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/*
 * @file iwg22m_sm.h 
 *
 * @brief Config file for iwaves iMx6x Q7 SOMs
 *
 * @ingroup High level Configration file
 */

#ifndef __IWG22M_SM_H
#define __IWG22M_SM_H


#undef DEBUG
#define CONFIG_ARMV7
#define CONFIG_IWG22M
#define CONFIG_RMOBILE
#define CONFIG_RMOBILE_BOARD_STRING "RZ/G1E iW-RainboW-G22M-SM\n"
#define CONFIG_SH_GPIO_PFC
#define CONFIG_SYS_THUMB_BUILD
#define CONFIG_BOARD_LATE_INIT
#define CONFIG_CMD_BDI
#define CONFIG_CMD_BOOTD
#define CONFIG_ENV_OVERWRITE

#include <asm/arch/rmobile.h>

#define CONFIG_CMD_EDITENV
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_DFL
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_RUN
#define CONFIG_CMD_LOADS
#define CONFIG_CMD_NET
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_NFS
#define CONFIG_CMD_BOOTZ
#define CONFIG_CMD_USB
#define CONFIG_CMD_FAT
#define CONFIG_FAT_WRITE
#define CONFIG_CMD_MMC
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_EXT4_WRITE
#define CONFIG_CMD_SF
#define CONFIG_CMD_SPI
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_PART

#define CONFIG_PARTITION_UUIDS

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_CMDLINE_EDITING

#define CONFIG_OF_LIBFDT
#define BOARD_LATE_INIT

#define CONFIG_BAUDRATE		115200
#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOTARGS		"ttySC3"

#define CONFIG_VERSION_VARIABLE
#undef  CONFIG_SHOW_BOOT_PROGRESS

#define CONFIG_ARCH_CPU_INIT
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_USE_ARCH_MEMSET
#define CONFIG_USE_ARCH_MEMCPY
#define CONFIG_TMU_TIMER

/* STACK */
#if defined(CONFIG_EXTRAM_BOOT)
#define CONFIG_SYS_INIT_SP_ADDR		0x7003FFFC
#else
#define CONFIG_SYS_INIT_SP_ADDR		0xE633FFFC
#endif
#define STACK_AREA_SIZE			0xC000
#define LOW_LEVEL_MERAM_STACK	\
		(CONFIG_SYS_INIT_SP_ADDR + STACK_AREA_SIZE - 4)

/* MEMORY */
#define IWG22M_SDRAM_BASE		0x40000000
#define IWG22M_SDRAM_SIZE		0x20000000
#define IWG22M_UBOOT_SDRAM_SIZE		0x20000000

#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_PROMPT		"iWave-G22M> "
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_PBSIZE		256
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_BARGSIZE		512
#define CONFIG_SYS_BAUDRATE_TABLE	{ 38400, 115200 }

/* SCIF */
#define CONFIG_SCIF_CONSOLE
#define CONFIG_CONS_SCIF4
#define CONFIG_SCIF_USE_EXT_CLK
#define SCIF4_BASE	0xe6ee0000
#undef  CONFIG_SYS_CONSOLE_INFO_QUIET
#undef  CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
#undef  CONFIG_SYS_CONSOLE_ENV_OVERWRITE

#define CONFIG_SYS_MEMTEST_START	(IWG22M_SDRAM_BASE)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + \
					 504 * 1024 * 1024)
#undef	CONFIG_SYS_MEMTEST_SCRATCH
#undef	CONFIG_SYS_LOADS_BAUD_CHANGE

#define CONFIG_SYS_SDRAM_BASE		(IWG22M_SDRAM_BASE)
#define CONFIG_SYS_SDRAM_SIZE		(IWG22M_UBOOT_SDRAM_SIZE)
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x7fc0)
#define CONFIG_NR_DRAM_BANKS		1

#define CONFIG_SYS_MONITOR_BASE		0x00000000
#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(1 * 1024 * 1024)
#define CONFIG_SYS_GBL_DATA_SIZE	(256)
#define CONFIG_SYS_BOOTMAPSZ		(8 * 1024 * 1024)

#if defined(CONFIG_EXTRAM_BOOT)
#define CONFIG_SYS_TEXT_BASE	0x70000000
#else
#define CONFIG_SYS_TEXT_BASE	0xE6304000
#endif

/* FLASH */
#define CONFIG_SPI
#define CONFIG_SH_QSPI
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_SST
#define CONFIG_SPI_FLASH_QUAD
#define CONFIG_SYS_NO_FLASH
#define CONFIG_SH_QSPI_BASE	0xE6B10000

/* ENV setting */
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SECT_SIZE	(8 * 1024)
#define CONFIG_ENV_ADDR		0xC0000
#define CONFIG_ENV_OFFSET	(CONFIG_ENV_ADDR)
#define CONFIG_ENV_SIZE		(CONFIG_ENV_SECT_SIZE)

/* IWG22M: Environmental: default boot emmc environmental variable defined */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"eth_vin=camera\0" \
	"bootm_low=0x40e00000\0" \
	"bootm_size=0x100000\0" \
  	"loadaddr=0x40007fc0\0" \
        "fdt_addr=0x40f00000\0" \
        "ethaddr=00:01:02:03:04:05\0" \
	"ethaddr1=1a:2b:3c:4d:5e:6f\0" \
        "fdt_file=r8a7745-iwg22d-sodimm.dtb\0" \
	"vin2_camera=ov5640\0" \
	"disp=lcd\0" \
	"wifi_sel=wifi\0" \
        "kernel=uImage\0" \
	"bootargs_base=console=ttySC3,115200n8 ignore_loglevel vmalloc=200M\0" \
		"bootargs_msd1=part uuid mmc 1:2 msd1_uuid;" \
			"setenv bootargs ${bootargs_base} root=PARTUUID=${msd1_uuid} " \
			"rootwait rootfstype=ext4 rw\0" \
                "bootcmd_msd1=run bootargs_msd1;mmc dev 1;" \
                        "fatload mmc 1 ${loadaddr} ${kernel};fatload mmc 1 ${fdt_addr} ${fdt_file};bootm ${loadaddr} - ${fdt_addr}\0" \
		"bootargs_msd0=part uuid mmc 0:2 msd0_uuid;" \
			"setenv bootargs ${bootargs_base} root=PARTUUID=${msd0_uuid} " \
			"rootwait rootfstype=ext4 rw\0" \
                "bootcmd_msd0=run bootargs_msd0;mmc dev 0;" \
                        "fatload mmc 0 ${loadaddr} ${kernel};fatload mmc 0 ${fdt_addr} ${fdt_file};bootm ${loadaddr} - ${fdt_addr}\0" \
		"bootargs_mmc=part uuid mmc 2:2 mmc_uuid;" \
			"setenv bootargs ${bootargs_base} root=PARTUUID=${mmc_uuid} " \
			"rootwait rootfstype=ext4 rw\0" \
                "bootcmd_mmc=run bootargs_mmc;mmc dev 2;" \
                        "fatload mmc 2 ${loadaddr} ${kernel};fatload mmc 2 ${fdt_addr} ${fdt_file};bootm ${loadaddr} - ${fdt_addr}\0" \
                "bootcmd=run bootcmd_mmc\0" \
              

/* SH Ether */
#define CONFIG_NET_MULTI
#define CONFIG_SH_ETHER_USE_PORT	0
#define CONFIG_SH_ETHER_PHY_ADDR	0x4
#define CONFIG_SH_ETHER_PHY_MODE	PHY_INTERFACE_MODE_RMII
#define CONFIG_SH_ETHER_CACHE_WRITEBACK
#define CONFIG_SH_ETHER_CACHE_INVALIDATE
#define CONFIG_PHYLIB
#define CONFIG_PHY_MICREL
#define CONFIG_BITBANGMII_MULTI
#define CONFIG_BITBANGMII

/* Ether-AVB */
#define CONFIG_SH_ETHER_RAVB
#define CONFIG_SH_ETHER_RAVB_USE_PORT        0
#define CONFIG_SH_ETHER_RAVB_PHY_ADDR        0x3
#define CONFIG_SH_ETHER_RAVB_PHY_MODE PHY_INTERFACE_MODE_GMII
#define CONFIG_SH_ETHER_RAVB_CACHE_WRITEBACK
#define CONFIG_SH_ETHER_RAVB_CACHE_INVALIDATE


/* IWG22M: Ethernet: Ethernet PHY defined*/
#define CONFIG_PHY_MICREL_KSZ9031
#define CONFIG_PHY_MICREL_KSZ8081

/* Board Clock */
#define CONFIG_SYS_CLK_FREQ	20000000
#define CONFIG_COM_PER_CLK	32500000
#define CONFIG_SCIF_CLK_FREQ	260000000
#define CONFIG_SYS_TMU_CLK_DIV	4
#define CONFIG_SYS_HZ		1000

/* FS */
#define CONFIG_DOS_PARTITION
#define CONFIG_SUPPORT_VFAT
#define CONFIG_NFS_TIMEOUT	10000UL

/* USB */
#define CONFIG_USB_STORAGE
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_R8A779x
#define CONFIG_USB_MAX_CONTROLLER_COUNT	2

/* MMCIF */
#define CONFIG_MMC			1
#define CONFIG_GENERIC_MMC		1

#define CONFIG_SH_MMCIF			1
#define CONFIG_SH_MMCIF_ADDR		0xee200000
#define CONFIG_SH_MMCIF_CLK		48000000
#define CONFIG_SH_MMCIF_FREQ		100000000

/* SD */
#define CONFIG_SH_SDHI			1
#define CONFIG_SH_SDHI_FREQ		97500000
#define CONFIG_MMC_SH_SDHI_NR_CHANNEL	2

/* USB-ether */
#define CONFIG_MII
#define CONFIG_USB_HOST_ETHER	/* Enable USB Ethernet adapters */
#define CONFIG_USB_ETHER_ASIX	/* Asix, or whatever driver(s) you want */

#endif	/* __IWG22M_SM_H */
