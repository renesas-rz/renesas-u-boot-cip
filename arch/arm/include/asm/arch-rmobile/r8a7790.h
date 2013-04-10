/*
 * arch/arm/include/asm/arch-rmobile/r8a7790.h
 *     This file is r8a7790 processor definition.
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

#ifndef __ASM_ARCH_R8A7790_H
#define __ASM_ARCH_R8A7790_H

/*
 * R8A7790 I/O Addresses
 */
#define	RWDT_BASE	0xE6020000
#define	SWDT_BASE	0xE6030000
#define	LBSC_BASE	0xFEC00200
#define DBSC3_0_BASE	0xE6790000
#define DBSC3_1_BASE	0xE67A0000
#define TMU_BASE	0xE61E0000
#define	GPIO5_BASE	0xE6055000

#ifndef __ASSEMBLY__
#include <asm/types.h>

/* RWDT */
struct r8a7790_rwdt {
	u32 rwtcnt;	/* 0x00 */
	u32 rwtcsra;	/* 0x04 */
	u16 rwtcsrb;	/* 0x08 */
};

/* SWDT */
struct r8a7790_swdt {
	u32 swtcnt;	/* 0x00 */
	u32 swtcsra;	/* 0x04 */
	u16 swtcsrb;	/* 0x08 */
};

/* LBSC */
struct r8a7790_lbsc {
	u32 cs0ctrl;
	u32 cs1ctrl;
	u32 ecs0ctrl;
	u32 ecs1ctrl;
	u32 ecs2ctrl;
	u32 ecs3ctrl;
	u32 ecs4ctrl;
	u32 ecs5ctrl;
	u32 dummy0[4];	/* 0x20 .. 0x2C */
	u32 cswcr0;
	u32 cswcr1;
	u32 ecswcr0;
	u32 ecswcr1;
	u32 ecswcr2;
	u32 ecswcr3;
	u32 ecswcr4;
	u32 ecswcr5;
	u32 exdmawcr0;
	u32 exdmawcr1;
	u32 exdmawcr2;
	u32 dummy1[9];	/* 0x5C .. 0x7C */
	u32 cspwcr0;
	u32 cspwcr1;
	u32 ecspwcr0;
	u32 ecspwcr1;
	u32 ecspwcr2;
	u32 ecspwcr3;
	u32 ecspwcr4;
	u32 ecspwcr5;
	u32 exwtsync;
	u32 dummy2[3];	/* 0xA4 .. 0xAC */
	u32 cs0bstctl;
	u32 cs0btph;
	u32 dummy3[2];	/* 0xB8 .. 0xBC */
	u32 cs1gdst;
	u32 ecs0gdst;
	u32 ecs1gdst;
	u32 ecs2gdst;
	u32 ecs3gdst;
	u32 ecs4gdst;
	u32 ecs5gdst;
	u32 dummy4[5];	/* 0xDC .. 0xEC */
	u32 exdmaset0;
	u32 exdmaset1;
	u32 exdmaset2;
	u32 dummy5[5];	/* 0xFC .. 0x10C */
	u32 exdmcr0;
	u32 exdmcr1;
	u32 exdmcr2;
	u32 dummy6[5];	/* 0x11C .. 0x12C */
	u32 bcintsr;
	u32 bcintcr;
	u32 bcintmr;
	u32 dummy7;	/* 0x13C */
	u32 exbatlv;
	u32 exwtsts;
	u32 dummy8[14];	/* 0x148 .. 0x17C */
	u32 atacsctrl;
	u32 dummy9[15]; /* 0x184 .. 0x1BC */
	u32 exbct;
	u32 extct;
};

/* DBSC3 */
struct r8a7790_dbsc3 {
	u32 dummy0[3];	/* 0x00 .. 0x08 */
	u32 dbstate1;
	u32 dbacen;
	u32 dbrfen;
	u32 dbcmd;
	u32 dbwait;
	u32 dbkind;
	u32 dbconf0;
	u32 dummy1[2];	/* 0x28 .. 0x2C */
	u32 dbphytype;
	u32 dummy2[3];	/* 0x34 .. 0x3C */
	u32 dbtr0;
	u32 dbtr1;
	u32 dbtr2;
	u32 dummy3;	/* 0x4C */
	u32 dbtr3;
	u32 dbtr4;
	u32 dbtr5;
	u32 dbtr6;
	u32 dbtr7;
	u32 dbtr8;
	u32 dbtr9;
	u32 dbtr10;
	u32 dbtr11;
	u32 dbtr12;
	u32 dbtr13;
	u32 dbtr14;
	u32 dbtr15;
	u32 dbtr16;
	u32 dbtr17;
	u32 dbtr18;
	u32 dbtr19;
	u32 dummy4[7];	/* 0x94 .. 0xAC */
	u32 dbbl;
	u32 dummy5[3];	/* 0xB4 .. 0xBC */
	u32 dbadj0;
	u32 dummy6;	/* 0xC4 */
	u32 dbadj2;
	u32 dummy7[5];	/* 0xCC .. 0xDC */
	u32 dbrfcnf0;
	u32 dbrfcnf1;
	u32 dbrfcnf2;
	u32 dummy8[2];	/* 0xEC .. 0xF0 */
	u32 dbcalcnf;
	u32 dbcaltr;
	u32 dummy9;	/* 0xFC */
	u32 dbrnk0;
	u32 dummy10[31];	/* 0x104 .. 0x17C */
	u32 dbpdncnf;
	u32 dummy11[47];	/* 0x184 ..0x23C */
	u32 dbdfistat;
	u32 dbdficnt;
	u32 dummy12[14];	/* 0x248 .. 0x27C */
	u32 dbpdlck;
	u32 dummy13[3];	/* 0x284 .. 0x28C */
	u32 dbpdrga;
	u32 dummy14[3];	/* 0x294 .. 0x29C */
	u32 dbpdrgd;
	u32 dummy15[24];	/* 0x2A4 .. 0x300 */
	u32 dbbs0cnt1;
	u32 dummy16[30];	/* 0x308 .. 0x37C */
	u32 dbwt0cnf0;
	u32 dbwt0cnf1;
	u32 dbwt0cnf2;
	u32 dbwt0cnf3;
	u32 dbwt0cnf4;
};

/* GPIO */
struct r8a7790_gpio {
	u32 iointsel;
	u32 inoutsel;
	u32 outdt;
	u32 indt;
	u32 intdt;
	u32 intclr;
	u32 intmsk;
	u32 posneg;
	u32 edglevel;
	u32 filonoff;
	u32 intmsks;
	u32 mskclrs;
	u32 outdtsel;
	u32 outdth;
	u32 outdtl;
	u32 bothedge;
};

#endif

#endif /* __ASM_ARCH_R8A7790_H */
