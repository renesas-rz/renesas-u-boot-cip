/*
 * board/renesas/lager/lowlevel_init.S
 *     This file is lager low level initialize.
 *
 * Copyright (C) 2013-2015 Renesas Electronics Corporation
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

#include <asm-offsets.h>
#include <config.h>
#include <version.h>
#include <common.h>

	.globl lowlevel_init
lowlevel_init:
	mrc	p15, 0, r4, c0, c0, 5	/* mpidr */
	orr	r4, r4, r4, lsr #6
	and	r4, r4, #7		/* id 0-3 = ca15.0,1,2,3 */

#if 0
	tst	r4, #4
	ldreq	r1, =0xf0194000		/* ca15 snoop ctl */
	ldrne	r1, =0xf0195000		/* ca7 snoop ctl */
	tst	r4, #3
	moveq	r0, #3
	streq	r0, [r1, #0]		/* ca7.0, ca15.0 */
1:	ldr	r0, [r1, #0]		/* all */
	tst	r0, #3
	beq	1b
	ldr	r0, [r1, #0xc]
	tst	r0, #1
	bne	1b

	/* which core is the master? */
	ldr	r1, =0xe6188000
	ldr	r0, [r1, #0x1c]	/* RESCNT */
	tst	r0, #(1<<25)	/* MD8 */
	moveq	r0, #4		/* ca7.0 is master */
	movne	r0, #0		/* ca15.0 is master */
	cmp	r4, r0
	beq	do_lowlevel_init	@ CPU ID #0
	b	do_cpu_waiting		@ CPU ID #1-#3
#endif
	b	do_lowlevel_init

	.pool

/*
 * CPU ID #1-#3 come here
 */
	.align  4
do_cpu_waiting:
	ldr	r1, =0xe6180000	/* sysc */
1:	ldr	r0, [r1, #0x20]	/* sbar */
	tst	r0, r0
	beq	1b
	bx	r0

/*
 * Only CPU ID #0 comes here
 */
	.align  4
do_lowlevel_init:

	/* surpress wfe if ca15 */
	tst	r4, #4
	mrceq	p15, 0, r0, c1, c0, 1	/* actlr */
	orreq	r0, r0, #(1<<7)
	mcreq	p15, 0, r0, c1, c0, 1

	/* and set l2 latency */
	mrc	p15, 0, r0, c0, c0, 5		@ r0 = MPIDR
	and	r0, r0, #0xf00
	lsr	r0, r0, #8
	tst	r0, #1				@ only need for cluster 0
	bne	_exit_init_l2_a15

	mrc	p15, 1, r0, c9, c0, 2		@ r0 = L2CTLR
	and	r1, r0, #7
	cmp	r1, #3				@ has already been set up?
	bicne	r0, r0, #0xe7
	orrne	r0, r0, #0x83			@ L2CTLR[7:6] + L2CTLR[2:0]
	orrne	r0, r0, #0x20			@ L2CTLR[5]
	mcrne	p15, 1, r0, c9, c0, 2

	/* and set l2 auxiliary control */
	mrcne p15, 1, r0, c15, c0, 0		@ r0 = L2ACTLR
	orrne r0, r0, #(1<<3)			@ L2ACTLR[3]=1
	mcrne p15, 1, r0, c15, c0, 0

_exit_init_l2_a15:

	ldr	r3, =(CONFIG_SYS_INIT_SP_ADDR)
	sub	sp, r3, #4
	str	lr, [sp]

	/* initialize system */
	bl	s_init

	ldr	lr, [sp]
	mov	pc, lr
	nop
