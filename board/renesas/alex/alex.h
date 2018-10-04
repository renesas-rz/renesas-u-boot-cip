/*
 * board/renesas/alex/alex.h
 *     This file is ale6/ale4 board definition.
 *
 * Copyright (C) 2015 Renesas Electronics Corporation
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

#ifdef CONFIG_I2C_DA9063_USE
#define DA9063_I2C_ADDR 0x5A

#define REG_CONTROL_F   0x13
#define REG_LDO5_CONT   0x2A

#define L_SHUTDOWN      (1 << 1)
#define L_LDO5_PD_DIS   (1 << 3)
#else
#define RWDT_MSTP402        0x00000004

#define RST_BASE           0xE6160000
#define RST_WDTRSTCR       0x00000054
#define RST_CA7BAR         0x00000030
#define RST_WDTRSTCR_RWDT  0xA55A0000
#define RST_CABAR_BOOTADR  0x00E63410

#define RWDT_BASE          0xE6020000
#define RWDT_RWTCNT        0x00000000
#define RWDT_RWTCSRA       0x00000004
#define RWDT_RWTCNT_FULL   0x5A5AFFFF
#define RWDT_RWTCSRA_START 0xA5A5A580
#endif

extern int sh_timer_init(void);
extern int mmcif_mmc_init(void);
extern int sdhi_mmc_init(unsigned long addr, int ch);

extern void arch_preboot_os(void);
