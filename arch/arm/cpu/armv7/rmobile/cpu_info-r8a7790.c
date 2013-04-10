/*
 * arch/arm/cpu/armv7/rmobile/cpu_info-r8a7790.c
 *     This file is r8a7790 processor support.
 *
 * Copyright (C) 2013 Renesas Electronics Corporation
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

u32 rmobile_get_cpu_type(void)
{
	return 0;
}

u32 rmobile_get_cpu_rev(void)
{
	return 0;
}
