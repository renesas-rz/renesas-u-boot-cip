/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Renesas Electronics Corporation
 */

#ifndef __SMARC_RZG2L_H
#define __SMARC_RZG2L_H

#include <asm/arch/renesas.h>

#define CONFIG_REMAKE_ELF

#ifdef CONFIG_SPL
#define CONFIG_SPL_TARGET	"spl/u-boot-spl.scif"
#endif

/* boot option */

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

/* Generic Interrupt Controller Definitions */
/* RZ/G2L use GIC-v3 */
#define CONFIG_GICV3
#define GICD_BASE	0x11900000
#define GICR_BASE	0x11960000

/* console */
#define CONFIG_SYS_CBSIZE		2048
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_BAUDRATE_TABLE	{ 115200, 38400 }

/* PHY needs a longer autoneg timeout */
#define PHY_ANEG_TIMEOUT		20000

/* MEMORY */
#define CONFIG_SYS_INIT_SP_ADDR		CONFIG_TEXT_BASE

/* SDHI clock freq */
#define CONFIG_SH_SDHI_FREQ		133000000


/* ENV setting */
#if defined(CONFIG_RZG2L_DISTRO_BOOT)
#define CFG_EXTRA_ENV_SETTINGS	\
	"usb_pgood_delay=2000\0" \
	"fdt_addr_r=0x48000000\0" \
	"fdtfile="CONFIG_DEFAULT_FDT_FILE"\0" \
	"kernel_addr_r=0x48080000\0" \
	"boot_efi_binary=efi/boot/bootaa64.efi\0" \
	"scan_for_usb_dev=" \
		"usb start; " \
		"if test ! -e usb ${devnum}:1 /; then usb reset; fi;\0" \
	"scan_boot_efi=" \
		"part list ${devtype} ${devnum} devplist; "  \
		"env exists devplist || setenv devplist 1; " \
		"for distro_bootpart in ${devplist}; do " \
			"if test -e ${devtype} ${devnum}:${distro_bootpart} ${boot_efi_binary}; then " \
				"load ${devtype} ${devnum}:${distro_bootpart} " \
				"${kernel_addr_r} ${boot_efi_binary};"          \
				"echo BootEFI from <${devtype}> [${devnum}:${distro_bootpart}]; "\
				"bootefi ${kernel_addr_r};"                     \
			"fi;" \
		"done;\0" \
	"mmc0=" \
			"setenv devnum 0;" \
			"setenv devtype mmc;" \
			"run scan_boot_efi;\0" \
	"mmc1=" \
			"setenv devnum 1;" \
			"setenv devtype mmc;" \
			"run scan_boot_efi;\0" \
	"usb0=" \
			"setenv devnum 0;" \
			"setenv devtype usb;" \
			"run scan_for_usb_dev;"\
			"run scan_boot_efi;\0"\
	"usb1=" \
			"setenv devnum 1;" \
			"setenv devtype usb;" \
			"run scan_for_usb_dev;"\
			"run scan_boot_efi;\0" \
	"boot_targets=" \
			"usb0 usb1 mmc0 mmc1\0" \
	"dfu_alt_info=" \
			"sf 0:0=fip.bin raw 0x60000 0x1F0000\0" \
	"dfu_bufsiz=" \
			"0x1F0000\0" \
	"ipaddr=" \
			"192.168.10.7\0" \
	"serverip=" \
			"192.168.10.3\0" \
	"distro_bootcmd=" \
			"env exists boot_targets || setenv boot_targets mmc0 mmc1 usb0 usb1; " \
			"for target in ${boot_targets}; do "\
				"run ${target};" \
			"done;" \
	"bootcmd=run distro_bootcmd\0"

#else

/* The HF/QSPI layout permits up to 1 MiB large bootloader blob */
#define CONFIG_BOARD_SIZE_LIMIT		1048576

/* ENV setting */

#define CFG_EXTRA_ENV_SETTINGS \
	"usb_pgood_delay=2000\0" \
	"bootm_size=0x10000000 \0" \
	"prodsdbootargs=setenv bootargs rw rootwait earlycon root=/dev/mmcblk1p2 \0" \
	"prodemmcbootargs=setenv bootargs rw rootwait earlycon root=/dev/mmcblk0p2 \0" \
	"bootimage=booti 0x48080000 - 0x48000000 \0" \
	"emmcload=ext4load mmc 0:2 0x48080000 boot/Image;ext4load mmc 0:2 0x48000000 boot/r9a07g044l2-smarc.dtb;run prodemmcbootargs \0" \
	"sd1load=ext4load mmc 1:2 0x48080000 boot/Image;ext4load mmc 1:2 0x48000000 boot/r9a07g044l2-smarc.dtb;run prodsdbootargs \0" \
	"bootcmd_check=if mmc dev 1; then run sd1load; else run emmcload; fi \0"

#define CONFIG_BOOTCOMMAND	"env default -a;run bootcmd_check;run bootimage"

#endif
/* For board */
/* Ethernet RAVB */
#define CONFIG_BITBANGMII_MULTI

#endif /* __SMARC_RZG2L_H */
