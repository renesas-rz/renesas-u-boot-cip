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
 * @file iwg21m_q7.h 
 *
 * @brief Config file for iwaves iMx6x Q7 SOMs
 *
 * @ingroup High level Configration file
 */


#ifndef __IWG21M_Q7_H
#define __IWG21M_Q7_H

#undef DEBUG
#define CONFIG_ARMV7
#define CONFIG_IWG21M
#define CONFIG_RMOBILE
#define CONFIG_RMOBILE_BOARD_STRING "RZ/G1H iW-RainboW-G21M-Q7\n"
/* IWG21M: Ethernet: Ethernet PHY defined*/
#define CONFIG_PHY_MICREL_KSZ9031
#define CONFIG_PHY_MICREL_KSZ8081
#define CONFIG_SH_GPIO_PFC
#define CONFIG_SYS_THUMB_BUILD
#define CONFIG_MACH_SKRZG1H
#define CONFIG_BOARD_LATE_INIT
#define CONFIG_CMD_BDI
#define CONFIG_CMD_BOOTD
#define CONFIG_ENV_OVERWRITE
#define CONFIG_AUTO_COMPLETE

#include <asm/arch/rmobile.h>

#define	CONFIG_CMD_EDITENV
#define	CONFIG_CMD_SAVEENV
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
#define CONFIG_CMD_PART

#define CONFIG_PARTITION_UUIDS

#define	CONFIG_CMDLINE_TAG
#define	CONFIG_SETUP_MEMORY_TAGS
#define	CONFIG_INITRD_TAG
#define	CONFIG_CMDLINE_EDITING

#define CONFIG_OF_LIBFDT
#define BOARD_LATE_INIT

#define CONFIG_BAUDRATE		115200
#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOTARGS		"console=ttySC2,115200n8 ignore_loglevel vmalloc=384M"

#define CONFIG_VERSION_VARIABLE
#undef	CONFIG_SHOW_BOOT_PROGRESS

#define CONFIG_ARCH_CPU_INIT
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_USE_ARCH_MEMSET
#define CONFIG_USE_ARCH_MEMCPY
#define CONFIG_TMU_TIMER

/* STACK */
#if defined(CONFIG_EXTRAM_BOOT)
#define CONFIG_SYS_INIT_SP_ADDR		0xB003FFFC
#else
#define CONFIG_SYS_INIT_SP_ADDR		0xE827FFFC
#endif
#define STACK_AREA_SIZE				0xC000
#define LOW_LEVEL_MERAM_STACK	\
		(CONFIG_SYS_INIT_SP_ADDR + STACK_AREA_SIZE - 4)

/* MEMORY */
#define IWG21M_SDRAM_BASE	0x40000000
#define IWG21M_SDRAM_SIZE	0x80000000
#define IWG21M_UBOOT_SDRAM_SIZE	0x20000000

#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_PROMPT		"iWave-G21M >"
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_PBSIZE		256
#define CONFIG_SYS_MAXARGS		32
#define CONFIG_SYS_BARGSIZE		512
#define CONFIG_SYS_BAUDRATE_TABLE	{ 38400, 115200 }

/* SCIF */
#define CONFIG_SCIF_CONSOLE
#define CONFIG_SCIF_A
#define CONFIG_CONS_SCIFA2
#define SCIFA2_BASE             0xe6c60000
#undef	CONFIG_SYS_CONSOLE_INFO_QUIET
#undef	CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
#undef	CONFIG_SYS_CONSOLE_ENV_OVERWRITE

#define CONFIG_SYS_MEMTEST_START	(IWG21M_SDRAM_BASE)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + \
					 504 * 1024 * 1024)
#undef	CONFIG_SYS_ALT_MEMTEST
#undef	CONFIG_SYS_MEMTEST_SCRATCH
#undef	CONFIG_SYS_LOADS_BAUD_CHANGE

#define CONFIG_SYS_SDRAM_BASE		(IWG21M_SDRAM_BASE)
#define CONFIG_SYS_SDRAM_SIZE		(IWG21M_UBOOT_SDRAM_SIZE)
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x7fc0)
#define CONFIG_NR_DRAM_BANKS		1

#define CONFIG_SYS_MONITOR_BASE		0x00000000
#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(1 * 1024 * 1024)
#define CONFIG_SYS_GBL_DATA_SIZE	(256)
#define CONFIG_SYS_BOOTMAPSZ		(8 * 1024 * 1024)

#if defined(CONFIG_EXTRAM_BOOT)
#define CONFIG_SYS_TEXT_BASE	0xB0000000
#else
#define CONFIG_SYS_TEXT_BASE	0xE8080000
#endif

/* FLASH */
#define CONFIG_SPI
#define CONFIG_SH_QSPI
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_SST
#define CONFIG_SYS_NO_FLASH
#define CONFIG_SH_QSPI_BASE	0xE6B10000

/* ENV setting */
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SECT_SIZE	(8 * 1024)
#define CONFIG_ENV_ADDR		0xC0000
#define CONFIG_ENV_OFFSET	(CONFIG_ENV_ADDR)
#define CONFIG_ENV_SIZE		(CONFIG_ENV_SECT_SIZE)

#define CONFIG_EXTRA_ENV_SETTINGS \
	"bootm_low=0x40e00000\0" \
	"bootm_size=0x100000\0" \
	"loadaddr=0x40007fc0\0" \
        "fdt_addr=0x40f00000\0" \
        "ethaddr=00:01:02:03:04:05\0" \
	"eth1addr=11:22:33:44:55:66\0" \
	"vin0_camera=ov5640\0" \
	"vin1_camera=ov5640\0" \
	"vin2_camera=ov5640\0" \
	"vin3_camera=ov5640\0" \
        "fdt_file=r8a7742-iwg21d-q7.dtb\0" \
	"bootfile=uImage\0" \
        "kernel=uImage\0" \
		"bootargs_base=console=ttySC2,115200n8 ignore_loglevel vmalloc=384M\0" \
		"bootcmd_net=dhcp;tftpboot ${loadaddr} ${serverip}:${kernel};tftpboot ${fdt_addr} ${serverip}:${fdt_file};" \
			"run bootargs_net;bootm ${loadaddr} - ${fdt_addr}\0" \
		"bootargs_net=setenv bootargs ${bootargs_base} " \
			"root=/dev/nfs ip=dhcp nfsroot=${serverip}:${nfsroot} rootwait rw,v3,tcp\0" \
                "bootargs_mmc=part uuid mmc 1:2 mmc_uuid;" \
                        "setenv bootargs ${bootargs_base} root=PARTUUID=${mmc_uuid} " \
                        "rootwait rootfstype=ext4 rw\0" \
                "bootcmd_mmc=run bootargs_mmc;mmc dev 1;" \
                        "fatload mmc 1 ${loadaddr} ${kernel};fatload mmc 1 ${fdt_addr} ${fdt_file};bootm ${loadaddr} - ${fdt_addr}\0" \
                "bootcmd=run bootcmd_mmc\0" \
                "bootargs_ssd=part uuid mmc 0:2 ssd_uuid;" \
                        "setenv bootargs ${bootargs_base} root=PARTUUID=${ssd_uuid} " \
                        "rootwait rootfstype=ext4 rw\0" \
                "bootcmd_ssd=run bootargs_ssd;mmc dev 0;" \
                        "fatload mmc 0 ${loadaddr} ${kernel};fatload mmc 0 ${fdt_addr} ${fdt_file};bootm ${loadaddr} - ${fdt_addr}\0" \
		"\0"

/* network */
#define CONFIG_NFS_TIMEOUT 10000UL
/* SH Ether */
#define	CONFIG_NET_MULTI
#define CONFIG_SH_ETHER_USE_PORT	0
#define CONFIG_SH_ETHER_PHY_ADDR	0x1
#define CONFIG_SH_ETHER_PHY_MODE PHY_INTERFACE_MODE_RMII
#define CONFIG_SH_ETHER_CACHE_WRITEBACK
#define CONFIG_SH_ETHER_CACHE_INVALIDATE
#define CONFIG_PHYLIB
#define CONFIG_PHY_MICREL
#define CONFIG_BITBANGMII
#define CONFIG_BITBANGMII_MULTI

/* Ether-AVB */
#define CONFIG_SH_ETHER_RAVB
#define CONFIG_SH_ETHER_RAVB_USE_PORT        0
#define CONFIG_SH_ETHER_RAVB_PHY_ADDR        0x3
#define CONFIG_SH_ETHER_RAVB_PHY_MODE PHY_INTERFACE_MODE_GMII
#define CONFIG_SH_ETHER_RAVB_CACHE_WRITEBACK
#define CONFIG_SH_ETHER_RAVB_CACHE_INVALIDATE

/* Board Clock */
#define	CONFIG_SYS_CLK_FREQ	10000000
#define	CONFIG_SCIF_CLK_FREQ	52000000
#define CONFIG_SYS_TMU_CLK_DIV	4
#define CONFIG_SYS_HZ		1000

/* i2c */
#define CONFIG_RCAR_I2C
#define CONFIG_RCAR_I2C_BASE	0xE6540000

/* FS */
#define CONFIG_DOS_PARTITION
#define CONFIG_SUPPORT_VFAT

/* USB */
#define CONFIG_USB_STORAGE
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_R8A779x
#define CONFIG_USB_MAX_CONTROLLER_COUNT        3

/* MMCIF */
#define CONFIG_MMC			1
#define CONFIG_GENERIC_MMC		1

#define CONFIG_SH_MMCIF			1
#define CONFIG_SH_MMCIF_ADDR		0xee220000	/* ch1 */
#define CONFIG_SH_MMCIF_CLK		48000000
#define CONFIG_SH_MMCIF_FREQ		100000000

/* SD */
#define CONFIG_SH_SDHI			1
#define CONFIG_SH_SDHI_FREQ		97500000
#define CONFIG_MMC_SH_SDHI_NR_CHANNEL	4

/* USB-ether */
#define CONFIG_MII

#endif	/* __IWG21M_Q7H */
