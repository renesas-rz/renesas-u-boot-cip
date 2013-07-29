/*
 * board/renesas/koelsch/koelsch.c
 *     This file is koelsch board support.
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
#include <malloc.h>
#include <asm/processor.h>
#include <asm/mach-types.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/arch/rmobile.h>

DECLARE_GLOBAL_DATA_PTR;

#define s_init_wait(cnt) \
		({	\
			volatile u32 i = 0x10000 * cnt;	\
			while (i > 0)	\
				i--;	\
		})

void s_init(void)
{
	struct r8a7791_rwdt *rwdt = (struct r8a7791_rwdt *)RWDT_BASE;
	struct r8a7791_swdt *swdt = (struct r8a7791_swdt *)SWDT_BASE;
	struct r8a7791_lbsc *lbsc = (struct r8a7791_lbsc *)LBSC_BASE;
	struct r8a7791_dbsc3 *dbsc3_0 = (struct r8a7791_dbsc3 *)DBSC3_0_BASE;

	/* Watchdog init */
	writel(0xA5A5A500, &rwdt->rwtcsra);
	writel(0xA5A5A500, &swdt->swtcsra);

	/* QoS */
	qos_init();

#if defined(CONFIG_NORFLASH)
	/* LBSC */
	writel(0x00000020, &lbsc->cs0ctrl);
	writel(0x00000020, &lbsc->cs1ctrl);
	writel(0x00002020, &lbsc->ecs0ctrl);
	writel(0x00002020, &lbsc->ecs1ctrl);

	writel(0x077F077F, &lbsc->cswcr0);
	writel(0x077F077F, &lbsc->cswcr1);
	writel(0x077F077F, &lbsc->ecswcr0);
	writel(0x077F077F, &lbsc->ecswcr1);

	/* DBSC3 */
	s_init_wait(10);

	writel(0x0000A55A, &dbsc3_0->dbpdlck);
	writel(0x00000001, &dbsc3_0->dbpdrga);
	writel(0x80000000, &dbsc3_0->dbpdrgd);
	writel(0x00000004, &dbsc3_0->dbpdrga);
	while ((readl(&dbsc3_0->dbpdrgd) & 0x00000001) != 0x00000001)
		;

	writel(0x00000006, &dbsc3_0->dbpdrga);
	writel(0x0001C000, &dbsc3_0->dbpdrgd);

	writel(0x00000023, &dbsc3_0->dbpdrga);
	writel(0x00FD2480, &dbsc3_0->dbpdrgd);

	writel(0x00000010, &dbsc3_0->dbpdrga);
	writel(0xF004649B, &dbsc3_0->dbpdrgd);

	writel(0x0000000F, &dbsc3_0->dbpdrga);
	writel(0x00181EE4, &dbsc3_0->dbpdrgd);

	writel(0x0000000E, &dbsc3_0->dbpdrga);
	writel(0x33C03812, &dbsc3_0->dbpdrgd);

	writel(0x00000003, &dbsc3_0->dbpdrga);
	writel(0x0300C481, &dbsc3_0->dbpdrgd);

	writel(0x00000007, &dbsc3_0->dbkind);
	writel(0x10030A02, &dbsc3_0->dbconf0);
	writel(0x00000001, &dbsc3_0->dbphytype);
	writel(0x00000000, &dbsc3_0->dbbl);
	writel(0x0000000B, &dbsc3_0->dbtr0);
	writel(0x00000008, &dbsc3_0->dbtr1);
	writel(0x00000000, &dbsc3_0->dbtr2);
	writel(0x0000000B, &dbsc3_0->dbtr3);
	writel(0x000C000B, &dbsc3_0->dbtr4);
	writel(0x00000027, &dbsc3_0->dbtr5);
	writel(0x0000001C, &dbsc3_0->dbtr6);
	writel(0x00000005, &dbsc3_0->dbtr7);
	writel(0x00000018, &dbsc3_0->dbtr8);
	writel(0x00000008, &dbsc3_0->dbtr9);
	writel(0x0000000C, &dbsc3_0->dbtr10);
	writel(0x00000009, &dbsc3_0->dbtr11);
	writel(0x00000012, &dbsc3_0->dbtr12);
	writel(0x000000D0, &dbsc3_0->dbtr13);
	writel(0x00140005, &dbsc3_0->dbtr14);
	writel(0x00050004, &dbsc3_0->dbtr15);
	writel(0x70233005, &dbsc3_0->dbtr16);
	writel(0x000C0000, &dbsc3_0->dbtr17);
	writel(0x00000300, &dbsc3_0->dbtr18);
	writel(0x00000040, &dbsc3_0->dbtr19);
	writel(0x00000001, &dbsc3_0->dbrnk0);
	writel(0x00020001, &dbsc3_0->dbadj0);
	writel(0x20082008, &dbsc3_0->dbadj2);
	writel(0x00020002, &dbsc3_0->dbwt0cnf0);
	writel(0x0000000F, &dbsc3_0->dbwt0cnf4);

	writel(0x00000015, &dbsc3_0->dbpdrga);
	writel(0x00000D70, &dbsc3_0->dbpdrgd);

	writel(0x00000016, &dbsc3_0->dbpdrga);
	writel(0x00000006, &dbsc3_0->dbpdrgd);

	writel(0x00000017, &dbsc3_0->dbpdrga);
	writel(0x00000018, &dbsc3_0->dbpdrgd);

	writel(0x00000012, &dbsc3_0->dbpdrga);
	writel(0x9D5CBB66, &dbsc3_0->dbpdrgd);

	writel(0x00000013, &dbsc3_0->dbpdrga);
	writel(0x1A868300, &dbsc3_0->dbpdrgd);

	writel(0x00000023, &dbsc3_0->dbpdrga);
	writel(0x00FDB6C0, &dbsc3_0->dbpdrgd);

	writel(0x00000014, &dbsc3_0->dbpdrga);
	writel(0x300214D8, &dbsc3_0->dbpdrgd);

	writel(0x0000001A, &dbsc3_0->dbpdrga);
	writel(0x930035C7, &dbsc3_0->dbpdrgd);

	writel(0x00000060, &dbsc3_0->dbpdrga);
	writel(0x330657B2, &dbsc3_0->dbpdrgd);

	writel(0x00000011, &dbsc3_0->dbpdrga);
	writel(0x1000040B, &dbsc3_0->dbpdrgd);

	writel(0x0000FA00, &dbsc3_0->dbcmd);
	writel(0x00000001, &dbsc3_0->dbpdrga);
	writel(0x00000071, &dbsc3_0->dbpdrgd);

	writel(0x00000004, &dbsc3_0->dbpdrga);
	while ((readl(&dbsc3_0->dbpdrgd) & 0x00000001) != 0x00000001)
		;

	writel(0x0000FA00, &dbsc3_0->dbcmd);
	writel(0x2100FA00, &dbsc3_0->dbcmd);
	writel(0x0000FA00, &dbsc3_0->dbcmd);
	writel(0x0000FA00, &dbsc3_0->dbcmd);
	writel(0x0000FA00, &dbsc3_0->dbcmd);
	writel(0x0000FA00, &dbsc3_0->dbcmd);
	writel(0x0000FA00, &dbsc3_0->dbcmd);
	writel(0x0000FA00, &dbsc3_0->dbcmd);
	writel(0x0000FA00, &dbsc3_0->dbcmd);

	writel(0x110000DB, &dbsc3_0->dbcmd);

	writel(0x00000001, &dbsc3_0->dbpdrga);
	writel(0x00000181, &dbsc3_0->dbpdrgd);

	writel(0x00000004, &dbsc3_0->dbpdrga);
	while ((readl(&dbsc3_0->dbpdrgd) & 0x00000001) != 0x00000001)
		;

	writel(0x00000001, &dbsc3_0->dbpdrga);
	writel(0x0000FE01, &dbsc3_0->dbpdrgd);

	writel(0x00000004, &dbsc3_0->dbpdrga);
	while ((readl(&dbsc3_0->dbpdrgd) & 0x00000001) != 0x00000001)
		;

	writel(0x00000000, &dbsc3_0->dbbs0cnt1);
	writel(0x01004C20, &dbsc3_0->dbcalcnf);
	writel(0x014000AA, &dbsc3_0->dbcaltr);
	writel(0x00000140, &dbsc3_0->dbrfcnf0);
	writel(0x00081860, &dbsc3_0->dbrfcnf1);
	writel(0x00010000, &dbsc3_0->dbrfcnf2);
	writel(0x00000001, &dbsc3_0->dbrfen);
	writel(0x00000001, &dbsc3_0->dbacen);
#endif /* CONFIG_NORFLASH */
}

#define	MSTPSR1		0xE6150038
#define	SMSTPCR1	0xE6150134
#define TMU0_MSTP125	(1 << 25)

#define	MSTPSR7		0xE61501C4
#define	SMSTPCR7	0xE615014C
#define SCIF0_MSTP721	(1 << 21)

#define	MSTPSR8		0xE61509A0
#define	SMSTPCR8	0xE6150990
#define ETHER_MSTP813	(1 << 13)

#define	PMMR		0xE6060000
#define	GPSR4		0xE6060014
#define	IPSR14		0xE6060058

#define	SetGuardREG(addr, mask, value)		\
{ \
	u32	val; \
	val = (readl(addr) & ~(mask)) | (value);	\
	writel(~val, PMMR); \
	writel(val, addr); \
}

int board_early_init_f(void)
{
	u32 val;

	/* TMU0 */
	val = readl(MSTPSR1);
	val &= ~TMU0_MSTP125;
	writel(val, SMSTPCR1);

#if defined(CONFIG_NORFLASH)
	/* SCIF0 */
	SetGuardREG(GPSR4, 0x34000000, 0x00000000);
	SetGuardREG(IPSR14, 0x00000FC7, 0x00000481);
	SetGuardREG(GPSR4,  0x00000000, 0x34000000);
#endif

	val = readl(MSTPSR7);
	val &= ~SCIF0_MSTP721;
	writel(val, SMSTPCR7);

	/* ETHER */
	val = readl(MSTPSR8);
	val &= ~ETHER_MSTP813;
	writel(val, SMSTPCR8);

	return 0;
}

DECLARE_GLOBAL_DATA_PTR;
int board_init(void)
{
	u32 val;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = KOELSCH_SDRAM_BASE + 0x100;

	/* Init PFC controller */
	r8a7791_pinmux_init();

	/* ETHER Enable */
	gpio_request(GPIO_FN_ETH_CRS_DV, NULL);
	gpio_request(GPIO_FN_ETH_RX_ER, NULL);
	gpio_request(GPIO_FN_ETH_RXD0, NULL);
	gpio_request(GPIO_FN_ETH_RXD1, NULL);
	gpio_request(GPIO_FN_ETH_LINK, NULL);
	gpio_request(GPIO_FN_ETH_REFCLK, NULL);
	gpio_request(GPIO_FN_ETH_MDIO, NULL);
	gpio_request(GPIO_FN_ETH_TXD1, NULL);
	gpio_request(GPIO_FN_ETH_TX_EN, NULL);
	gpio_request(GPIO_FN_ETH_TXD0, NULL);
	gpio_request(GPIO_FN_ETH_MDC, NULL);
	gpio_request(GPIO_FN_IRQ0, NULL);

	val = readl(0xe6060114);
	val &= ~0x37FC0000;
	writel(val, 0xe6060114);

	sh_timer_init();

	gpio_request(GPIO_GP_5_22, NULL);	/* PHY_RST */
	val = readl(0xe6060114);
	val &= ~0x08000000;
	writel(val, 0xe6060114);

	gpio_direction_output(GPIO_GP_5_22, 0);
	mdelay(20);
	gpio_set_value(GPIO_GP_5_22, 1);
	udelay(1);

	return 0;
}

int board_eth_init(bd_t *bis)
{
	int ret = -ENODEV;
	u32 val;
	unsigned char enetaddr[6];

#ifdef CONFIG_SH_ETHER
	ret = sh_eth_initialize(bis);
	if (!eth_getenv_enetaddr("ethaddr", enetaddr))
		return ret;

	/* Set Mac address */
	val = enetaddr[0] << 24 | enetaddr[1] << 16 |
	    enetaddr[2] << 8 | enetaddr[3];
	writel(val, 0xEE7003C0);

	val = enetaddr[4] << 8 | enetaddr[5];
	writel(val, 0xEE7003C8);
#endif

	return ret;
}

int dram_init(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	return 0;
}

const struct rmobile_sysinfo sysinfo = {
	CONFIG_RMOBILE_BOARD_STRING
};

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = KOELSCH_SDRAM_BASE;
	gd->bd->bi_dram[0].size = KOELSCH_SDRAM_SIZE;
}

int board_late_init(void)
{
	return 0;
}

void reset_cpu(ulong addr)
{
}
