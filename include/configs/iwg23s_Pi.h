/*
 *
 * Copyright (c) 2015 iWave Systems Technologies Pvt. Ltd.
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

#ifndef __IWG23S_H
#define __IWG23S_H

#undef DEBUG
#define CONFIG_ARMV7
#define CONFIG_R8A7747X
#define CONFIG_IWG23S
#define CONFIG_RMOBILE
#define CONFIG_RMOBILE_BOARD_STRING "R8A77947X iW-RainboW-G23S-Pi\n"
#define CONFIG_SH_GPIO_PFC
#define CONFIG_SYS_THUMB_BUILD
#define CONFIG_BOARD_LATE_INIT
#define CONFIG_CMD_BDI
#define CONFIG_CMD_BOOTD
#define CONFIG_ENV_OVERWRITE
#define CONFIG_DEFAULT_FDT_FILE         "r8a7747x-iwg23s_Pi.dtb"
#define CONFIG_SYS_HUSH_PARSER


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

#define	CONFIG_CMDLINE_TAG
#define	CONFIG_SETUP_MEMORY_TAGS
#define	CONFIG_INITRD_TAG
#define	CONFIG_CMDLINE_EDITING

#define CONFIG_OF_LIBFDT
#define BOARD_LATE_INIT

#define CONFIG_BAUDRATE		115200
#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOTARGS		"console=ttySC1"

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
#define CONFIG_SYS_INIT_SP_ADDR		0x7003FFFC
#else
#define CONFIG_SYS_INIT_SP_ADDR		0xE633FFFC
#endif
#define STACK_AREA_SIZE				0xC000
#define LOW_LEVEL_MERAM_STACK	\
		(CONFIG_SYS_INIT_SP_ADDR + STACK_AREA_SIZE - 4)

/* MEMORY */
#define IWG23S_SDRAM_BASE	0x40000000
#define IWG23S_SDRAM_SIZE	0x20000000
#define IWG23S_UBOOT_SDRAM_SIZE	0x20000000

#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_PROMPT		"iWave-G23S> "
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_PBSIZE		256
#define CONFIG_SYS_MAXARGS		32
#define CONFIG_SYS_BARGSIZE		512
#define CONFIG_SYS_BAUDRATE_TABLE	{ 38400, 115200 }

/* SCIF */
#define CONFIG_SCIF_CONSOLE
#define CONFIG_CONS_SCIF1
#define SCIF1_BASE		0xE6E68000
#undef	CONFIG_SYS_CONSOLE_INFO_QUIET
#undef	CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
#undef	CONFIG_SYS_CONSOLE_ENV_OVERWRITE

#define CONFIG_SYS_MEMTEST_START	(IWG23S_SDRAM_BASE)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + \
					 504 * 1024 * 1024)
#undef	CONFIG_SYS_IWG23S_MEMTEST
#undef	CONFIG_SYS_MEMTEST_SCRATCH
#undef	CONFIG_SYS_LOADS_BAUD_CHANGE

#define CONFIG_SYS_SDRAM_BASE		(IWG23S_SDRAM_BASE)
#define CONFIG_SYS_SDRAM_SIZE		(IWG23S_UBOOT_SDRAM_SIZE)
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
#define CONFIG_SPI_FLASH_STMICRO
#define CONFIG_SPI_FLASH_QUAD
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
	"bootfile=uImage\0" \
	"bootm_size=0x100000\0" \
	"loadaddr=0x40007fc0\0" \
	"fdt_addr=0x40f00000\0" \
	"ethaddr=00:01:02:03:04:05\0" \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"kernel=uImage\0" \
                "bootargs_base=console=ttySC1,115200n8 ignore_loglevel vmalloc=384M\0" \
                "bootargs_mmc=setenv bootargs ${bootargs_base} " \
                        "root=/dev/mmcblk1p2 rootwait rootfstype=ext4 rw\0" \
                "bootcmd_mmc=run bootargs_mmc;mmc dev 1;" \
                        "fatload mmc 1 ${loadaddr} ${kernel};fatload mmc 1 ${fdt_addr} ${fdt_file};bootm ${loadaddr} - ${fdt_addr}\0" \
       		"bootargs_msd=setenv bootargs ${bootargs_base} " \
                        "root=/dev/mmcblk0p2 rootwait rootfstype=ext4 rw\0" \
                "bootcmd_msd=run bootargs_msd;mmc dev 0;" \
                        "fatload mmc 0 ${loadaddr} ${kernel};fatload mmc 0 ${fdt_addr} ${fdt_file};bootm ${loadaddr} - ${fdt_addr}\0" \
                "bootargs_net=setenv bootargs ${bootargs_base} root=/dev/nfs ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp\0" \
		"bootcmd_net=run bootargs_net;tftpboot ${loadaddr} ${serverip}:${kernel};" \
			"tftpboot ${fdt_addr} ${serverip}:${fdt_file};bootm ${loadaddr} - ${fdt_addr}\0" \
                "bootcmd=run bootcmd_mmc\0" \
        "\0" 

/* network */
#define CONFIG_IWG23S_ETHERB
#define CONFIG_NFS_TIMEOUT 10000UL

/* Ethernet AVB */
#if defined(CONFIG_IWG23S_ETHERB)
#define CONFIG_RAVB
#define CONFIG_RAVB_PHY_ADDR	0x0
#define CONFIG_RAVB_PHY_MODE PHY_INTERFACE_MODE_GMII
#endif

/* Renesas Ethernet controler Common */
#if defined(CONFIG_RAVB)
#define CONFIG_NET_MULTI
#define CONFIG_PHYLIB
#define CONFIG_PHY_MICREL
#define CONFIG_PHY_MICREL_KSZ9031
#define CONFIG_BITBANGMII
#define CONFIG_BITBANGMII_MULTI
#define CONFIG_SH_ETHER_BITBANG
#endif

/* Board Clock */
#define	CONFIG_SYS_CLK_FREQ	65000000
#define       CONFIG_SCIF_CLK_FREQ    65000000

#define CONFIG_SYS_TMU_CLK_DIV	4
#define CONFIG_SYS_HZ		1000

/* I2C */
#define CONFIG_SH_I2C		1
#define CONFIG_SYS_I2C_MODULE	1
#define CONFIG_SYS_I2C_SPEED	400000 /* 400 kHz */
#define CONFIG_SYS_I2C_SLAVE	0x7F
#define CONFIG_SH_I2C_DATA_HIGH	4
#define CONFIG_SH_I2C_DATA_LOW	5
#define CONFIG_SH_I2C_CLOCK	10000000
#define CONFIG_SH_I2C_BASE0	0xE6518000

/* FS */
#define CONFIG_DOS_PARTITION
#define CONFIG_SUPPORT_VFAT

/* USB */
#define CONFIG_USB_STORAGE
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_R8A779x
#define CONFIG_USB_MAX_CONTROLLER_COUNT	2

/* SD */
#define CONFIG_MMC			1
#define CONFIG_GENERIC_MMC		1
#define CONFIG_SH_SDHI			1
#define CONFIG_SH_SDHI_FREQ		78000000
#define CONFIG_MMC_SH_SDHI_NR_CHANNEL	3

/* eMMC */
#define CONFIG_SH_SDHI_MMC

/* USB-ether */
#define CONFIG_MII
#define CONFIG_USB_HOST_ETHER   /* Enable USB Ethernet adapters */
#define CONFIG_USB_ETHER_ASIX   /* Asix, or whatever driver(s) you want */

#endif	/* __IWG23S_H */
