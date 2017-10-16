/*
 * Copyright (c) 2016 iWave Systems Technologies Pvt. Ltd
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
 * @file  lowlevel_init.S
 * 
 * @brief interrupts and L2 cache memory setting for iwaves SM SOMs 
 *
 * @ingroup interrupt Setting
 */
#include <config.h>
#include <linux/linkage.h>

ENTRY(lowlevel_init)
	ldr	r3, =(CONFIG_SYS_INIT_SP_ADDR)
	sub	sp, r3, #4
	str	lr, [sp]

	bl	s_init

	ldr	lr, [sp]
	mov	pc, lr
	nop
ENDPROC(lowlevel_init)
	.ltorg
