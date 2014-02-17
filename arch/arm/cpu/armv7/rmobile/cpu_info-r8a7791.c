/*
 * arch/arm/cpu/armv7/rmobile/cpu_info-r8a7791.c
 *     This file is r8a7791 processor support.
 *
 * Copyright (C) 2013-2014 Renesas Electronics Corporation
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
#include <common.h>
#include <asm/io.h>

#define PRR 0xFF000044

u32 rmobile_get_cpu_type(void)
{
	u32 product;

	product = readl(PRR);

	return (u32)((product & 0x00007F00) >> 8);
}

u32 rmobile_get_cpu_rev_integer(void)
{
	u32 product;

	product = readl(PRR);

	return (u32)(((product & 0x000000F0) >> 4) + 1);
}

u32 rmobile_get_cpu_rev_fraction(void)
{
	u32 product;

	product = readl(PRR);

	return (u32)(product & 0x0000000F);
}
