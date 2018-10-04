/*
 * board/renesas/lager/lager.h
 *     This file is lager board definition.
 *
 * Copyright (C) 2014-2015 Renesas Electronics Corporation
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

#define DA9063_I2C_ADDR	0x58	/* I2C ch3 */

#define REG_CONTROL_F	0x13
#define REG_LDO5_CONT	0x2A

#define L_SHUTDOWN	(1 << 1)
#define L_LDO5_PD_DIS	(1 << 3)

extern int sh_timer_init(void);
extern int mmcif_mmc_init(void);
extern int sdhi_mmc_init(unsigned long addr, int ch);

extern void arch_preboot_os(void);

extern void qos_init(void);

