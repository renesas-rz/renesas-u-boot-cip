/*
 * arch/arm/cpu/armv7/rmobile/cpu_info-r8a7743.c
 *     This file is r8a7743 processor support.
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

#ifdef CONFIG_IWG20M
#define BASE 0xe61f0000

#define REG_THSCR       0x12c
#define REG_THSSR       0x130

/* THSCR */
#define CPCTL   (1 << 12)

/* THSSR */
#define CTEMP   0x3f
#endif



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

#ifdef CONFIG_IWG20M
static void rcar_thermal_bset(u32 reg, u32 mask, u32 data)
{
        u32 val;

        val = readl(BASE + reg);
        val &= ~mask;
        val |= (data & mask);
        writel(val, BASE + reg);
        val = readl(BASE + reg);
}

static u32 rcar_thermal_read(u32 reg)
{
	return readl(BASE + reg);
}

int rcar_thermal_update_temp()
{
	u32 ctemp, new, old, i;

	ctemp = 0;
	old = ~0;

	/* Before reading the temperature , CPCTL(12th bit) in THSCR register
	   should be set to 1 Which indicates temperature value is determined 
	   automatically by hardware */
	rcar_thermal_bset(REG_THSCR, CPCTL, CPCTL);

	for (i = 0; i < 128; i++) {
		/*
		 * we need to wait 300us after changing comparator offset
		 * to get stable temperature.
		 * see "Usage Notes" on datasheet
		 */
		udelay(300);
		/* Temperature is read from CTEMP bit(0-5th bit) of THSSR register */
		new = rcar_thermal_read(REG_THSSR) & CTEMP;
		if (new == old) {
			ctemp = new;
			break;
		}
		old = new;
	}

	if (!ctemp) {
		printf("CPU: Temperature: Can't find sensor device\n");
		return -1;
	}

	/* Read temperature is calculated as per calculation T = CTEP[5:0] * 5 − 65 */
	ctemp = (ctemp * 5) - 65;
	printf("CPU: Temperature %d C\n", ctemp);
	return 0;
}
#endif
