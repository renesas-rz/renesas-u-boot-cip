// SPDX-License-Identifier: GPL-2.0
/*
 * R-Car R9A08G045S Clock Module
 *
 * Copyright (C) 2022 Renesas Electronics Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 */

#include <clk-uclass.h>
#include <dm.h>
#include <linux/clk-provider.h>
#include <dt-bindings/clock/r9a08g045s-cpg.h>
#include "renesas-rzg2l-cpg.h"

/* Specific SEL/DIV register offsets of RZ/G3S */
#define G3S_OCTA_DDIV		(0x21C)
#define G3S_OCTA_SSEL		(0x400)
#define G3S_SPI_DDIV		(0x220)
#define G3S_SPI_SSEL		(0x404)
#define G3S_SDHI_DDIV		(0x218)
#define G3S_SDHI_DSEL		(0x244)
#define G3S_PLL_DSEL		(0x240)
#define G3S_PLL6_DDIV		(0x214)

/* For RZ/G3S */
#define SEL_SDHI0_G3S	(G3S_SDHI_DSEL << 20	| 0 << 12	| 2 << 8)
#define SEL_SDHI1_G3S	(G3S_SDHI_DSEL << 20	| 4 << 12	| 2 << 8)
#define SEL_SDHI2_G3S	(G3S_SDHI_DSEL << 20	| 8 << 12	| 2 << 8)
#define SEL_SPI_G3S	(G3S_SPI_SSEL << 20	| 0 << 12	| 3 << 8)
#define SEL_OCTA_G3S	(G3S_OCTA_SSEL << 20	| 0 << 12	| 2 << 8)
#define SEL_PLL4	(G3S_PLL_DSEL << 20	| 6 << 12	| 1 << 8)

/* For RZ/G3S */
#define DIV_SDHI0_G3S	(G3S_SDHI_DDIV << 20	| 0 << 12	| 2 << 8)
#define DIV_SDHI1_G3S	(G3S_SDHI_DDIV << 20	| 4 << 12	| 2 << 8)
#define DIV_SDHI2_G3S	(G3S_SDHI_DDIV << 20	| 8 << 12	| 2 << 8)
#define DIV_SPI_G3S	(G3S_SPI_DDIV << 20	| 0 << 12	| 3 << 8)
#define DIV_OCTA_G3S	(G3S_OCTA_DDIV << 20	| 0 << 12	| 3 << 8)
#define DIVPL6A_G3S	(G3S_PLL6_DDIV << 20	| 0 << 12	| 3 << 8)
#define DIVPL6B_G3S	(G3S_PLL6_DDIV << 20	| 4 << 12	| 3 << 8)

extern int rzg2l_clk_probe(struct udevice *dev);
extern int rzg2l_clk_remove(struct udevice *dev);
extern const struct clk_ops rzg2l_clk_ops;

enum clk_ids {
	/* Core Clock Outputs exported to DT */
	LAST_DT_CORE_CLK = R9A08G045S_OSCCLK,

	/* External Input Clocks */
	CLK_XINCLK,

	/* Internal Core Clocks */
	CLK_OSC_DIV1000,
	CLK_PLL1,
	CLK_PLL2,
	CLK_PLL2_DIV2,
	CLK_PLL2_DIV6,
	CLK_PLL2_DIV2_8,
	CLK_PLL3,
	CLK_PLL3_DIV2,
	CLK_PLL3_DIV2_2,
	CLK_PLL3_DIV2_4,
	CLK_PLL3_DIV2_8,
	CLK_PLL3_DIV6,
	CLK_PLL4,
	CLK_PLL6,
	CLK_PLL6_DIV2,
	CLK_SEL_SPI,
	CLK_SEL_SDHI0,
	CLK_SEL_SDHI1,
	CLK_SEL_SDHI2,
	CLK_SEL_OCTA,
	CLK_SEL_PLL4,

	/* Module Clocks */
	CLK_MODE_BASE,
};

/*Divider tables*/
static struct clk_div_table div_table_1_2[] = {
	{0, 1},
	{1, 2},
};

static struct clk_div_table div_table_1_8[] = {
	{0, 1},
	{1, 2},
	{2, 4},
	{3, 8},
};

static struct clk_div_table div_table_1_32[] = {
	{0, 1},
	{1, 2},
	{2, 4},
	{3, 8},
	{4, 32},
};

/*MUX clock tables*/
static const char *const sel_spi[] = { ".pll3_div2_2", ".pll3_div6", ".pll6_div2" };
static const char *const sel_sdhi[] = { ".pll2_div2", ".pll6", ".pll2_div6" };
static const char *const sel_pll4[] = { ".osc_div1000", ".pll4" };

static const struct cpg_core_clk r9a08g045s_core_clks[] = {
	/* External Clock Inputs */
	DEF_INPUT("xinclk",	 CLK_XINCLK),

	/* Internal Core Clocks */
	DEF_FIXED(".osc", R9A08G045S_OSCCLK, CLK_XINCLK, 1, 1),
	DEF_FIXED(".osc2", R9A08G045S_OSCCLK2, CLK_XINCLK, 1, 3),
	DEF_FIXED(".osc_div1000", CLK_OSC_DIV1000, CLK_XINCLK, 1, 1000),
	DEF_BASE(".pll1", CLK_PLL1, CLK_TYPE_PLL1, CLK_XINCLK),
	DEF_BASE(".pll2", CLK_PLL2, CLK_TYPE_PLL2, CLK_XINCLK),
	DEF_BASE(".pll3", CLK_PLL3, CLK_TYPE_PLL3, CLK_XINCLK),
	DEF_BASE(".pll4", CLK_PLL4, CLK_TYPE_PLL4, CLK_XINCLK),
	DEF_BASE(".pll6", CLK_PLL6, CLK_TYPE_PLL6, CLK_XINCLK),
	DEF_FIXED(".pll2_div2", CLK_PLL2_DIV2, CLK_PLL2, 1, 2),
	DEF_FIXED(".pll2_div6", CLK_PLL2_DIV6, CLK_PLL2, 1, 6),
	DEF_FIXED(".pll2_div2_8", CLK_PLL2_DIV2_8, CLK_PLL2_DIV2, 1, 8),
	DEF_FIXED(".pll3_div2", CLK_PLL3_DIV2, CLK_PLL3, 1, 2),
	DEF_FIXED(".pll3_div2_2", CLK_PLL3_DIV2_2, CLK_PLL3_DIV2, 1, 2),
	DEF_FIXED(".pll3_div2_4", CLK_PLL3_DIV2_4, CLK_PLL3_DIV2, 1, 4),
	DEF_FIXED(".pll3_div2_8", CLK_PLL3_DIV2_8, CLK_PLL3_DIV2, 1, 8),
	DEF_FIXED(".pll3_div6", CLK_PLL3_DIV6, CLK_PLL3, 1, 6),
	DEF_FIXED(".pll6_div2", CLK_PLL6_DIV2, CLK_PLL6, 1, 2),
	DEF_MUX(".sel_spi", CLK_SEL_SPI, SEL_SPI_G3S, sel_spi,
		ARRAY_SIZE(sel_spi), CLK_MUX_HIWORD_MASK),
	DEF_MUX(".sel_sdhi0", CLK_SEL_SDHI0, SEL_SDHI0_G3S, sel_sdhi,
		ARRAY_SIZE(sel_sdhi), CLK_MUX_HIWORD_MASK),
	DEF_MUX(".sel_sdhi1", CLK_SEL_SDHI1, SEL_SDHI1_G3S, sel_sdhi,
		ARRAY_SIZE(sel_sdhi), CLK_MUX_HIWORD_MASK),
	DEF_MUX(".sel_sdhi2", CLK_SEL_SDHI2, SEL_SDHI2_G3S, sel_sdhi,
		ARRAY_SIZE(sel_sdhi), CLK_MUX_HIWORD_MASK),
	DEF_MUX(".sel_octa", CLK_SEL_OCTA, SEL_OCTA_G3S, sel_spi,
		ARRAY_SIZE(sel_spi), CLK_MUX_HIWORD_MASK),
	DEF_MUX(".sel_pll4", CLK_SEL_PLL4, SEL_PLL4, sel_pll4,
		ARRAY_SIZE(sel_pll4), CLK_MUX_HIWORD_MASK),

	/* Core output clk*/
	DEF_DIV("I", R9A08G045S_CLK_I, CLK_PLL1, DIVPL1,
		div_table_1_8, CLK_DIVIDER_HIWORD_MASK),
	DEF_DIV("P0", R9A08G045S_CLK_P0, CLK_PLL2_DIV2_8, DIVPL2B,
		div_table_1_32, CLK_DIVIDER_HIWORD_MASK),
	DEF_FIXED("P4", R9A08G045S_CLK_P4, CLK_PLL2_DIV2, 1, 5),
	DEF_FIXED("P5", R9A08G045S_CLK_P5, CLK_PLL2_DIV2, 1, 4),
	DEF_FIXED("TSU", R9A08G045S_CLK_TSU, CLK_PLL2_DIV2, 1, 8),
	DEF_DIV("SD0", R9A08G045S_CLK_SD0, CLK_SEL_SDHI0, DIV_SDHI0_G3S,
		div_table_1_2, CLK_DIVIDER_HIWORD_MASK),
	DEF_DIV("SD1", R9A08G045S_CLK_SD1, CLK_SEL_SDHI1, DIV_SDHI1_G3S,
		div_table_1_2, CLK_DIVIDER_HIWORD_MASK),
	DEF_DIV("SD2", R9A08G045S_CLK_SD2, CLK_SEL_SDHI2, DIV_SDHI2_G3S,
		div_table_1_2, CLK_DIVIDER_HIWORD_MASK),
	DEF_FIXED("M0", R9A08G045S_CLK_M0, CLK_PLL3_DIV2, 1, 4),
	DEF_DIV("P1", R9A08G045S_CLK_P1, CLK_PLL3_DIV2_4, DIVPL3A,
		div_table_1_32, CLK_DIVIDER_HIWORD_MASK),
	DEF_DIV("P2", R9A08G045S_CLK_P2, CLK_PLL3_DIV2_8, DIVPL3B,
		div_table_1_32, CLK_DIVIDER_HIWORD_MASK),
	DEF_DIV("P3", R9A08G045S_CLK_P3, CLK_PLL3_DIV2_4, DIVPL3C,
		div_table_1_32, CLK_DIVIDER_HIWORD_MASK),
	DEF_FIXED("AT", R9A08G045S_CLK_AT, CLK_PLL3_DIV2, 1, 2),
	DEF_FIXED("ZT", R9A08G045S_CLK_ZT, CLK_PLL3_DIV2, 1, 8),
	DEF_DIV("OC0", R9A08G045S_CLK_OC0, CLK_SEL_OCTA, DIV_OCTA_G3S,
		div_table_1_32, CLK_DIVIDER_HIWORD_MASK),
	DEF_FIXED("OC1", R9A08G045S_CLK_OC1, R9A08G045S_CLK_OC0, 1, 2),
	DEF_DIV("SPI0", R9A08G045S_CLK_SPI0, CLK_SEL_SPI, DIV_SPI_G3S,
		div_table_1_32, CLK_DIVIDER_HIWORD_MASK),
	DEF_FIXED("SPI1", R9A08G045S_CLK_SPI1, R9A08G045S_CLK_SPI0, 1, 2),
	DEF_FIXED("S0", R9A08G045S_CLK_S0, CLK_SEL_PLL4, 1, 2),
	DEF_DIV("I2", R9A08G045S_CLK_I2, CLK_PLL6_DIV2, DIVPL6A_G3S,
		div_table_1_32, CLK_DIVIDER_HIWORD_MASK),
	DEF_DIV("I3", R9A08G045S_CLK_I2, CLK_PLL6_DIV2, DIVPL6B_G3S,
		div_table_1_32, CLK_DIVIDER_HIWORD_MASK),
	DEF_FIXED("HP", R9A08G045S_CLK_HP, CLK_PLL6_DIV2, 1, 1),
};

static struct mssr_mod_clk r9a08g045s_mod_clks[] = {
	DEF_MOD("gic",		R9A08G045S_CLK_GIC600,
				R9A08G045S_CLK_P1,
				MSSR(5, BIT(0), (BIT(0) | BIT(1)))),
	DEF_MOD("ia55",		R9A08G045S_CLK_IA55,
				R9A08G045S_CLK_P1,
				MSSR(6, (BIT(0) | BIT(1)), BIT(0))),
	DEF_MOD("mhu",		R9A08G045S_CLK_MHU,
				R9A08G045S_CLK_P1,
				MSSR(8, BIT(0), BIT(0))),
	DEF_MOD("syc",		R9A08G045S_CLK_SYC,
				CLK_XINCLK,
				MSSR(10, BIT(0), BIT(0))),
	DEF_MOD("dmac",		R9A08G045S_CLK_DMAC,
				R9A08G045S_CLK_P3,
				MSSR(11, (BIT(0) | BIT(1)),
					 (BIT(0) | BIT(1)))),
	DEF_MOD("sysc",		R9A08G045S_CLK_SYSC,
				CLK_XINCLK,
				MSSR(12, (BIT(0) | BIT(1)),
					 (BIT(0) | BIT(1) | BIT(2)))),
	DEF_MOD("ostm0",	R9A08G045S_CLK_OSTM0,
				R9A08G045S_CLK_P0,
				MSSR(13, BIT(0), BIT(0))),
	DEF_MOD("ostm1",	R9A08G045S_CLK_OSTM1,
				R9A08G045S_CLK_P0,
				MSSR(13, BIT(1), BIT(1))),
	DEF_MOD("ostm2",	R9A08G045S_CLK_OSTM2,
				R9A08G045S_CLK_P0,
				MSSR(13, BIT(2), BIT(2))),
	DEF_MOD("ostm3",	R9A08G045S_CLK_OSTM3,
				R9A08G045S_CLK_P0,
				MSSR(13, BIT(3), BIT(3))),
	DEF_MOD("ostm4",	R9A08G045S_CLK_OSTM4,
				R9A08G045S_CLK_P0,
				MSSR(13, BIT(4), BIT(4))),
	DEF_MOD("ostm5",	R9A08G045S_CLK_OSTM5,
				R9A08G045S_CLK_P0,
				MSSR(13, BIT(5), BIT(5))),
	DEF_MOD("ostm6",	R9A08G045S_CLK_OSTM6,
				R9A08G045S_CLK_P0,
				MSSR(13, BIT(6), BIT(6))),
	DEF_MOD("ostm7",	R9A08G045S_CLK_OSTM7,
				R9A08G045S_CLK_P0,
				MSSR(13, BIT(7), BIT(7))),
	DEF_MOD("wdt0",		R9A08G045S_CLK_WDT0,
				R9A08G045S_CLK_P0,
				MSSR(18, (BIT(0) | BIT(1)), BIT(0))),
	DEF_MOD("wdt1",		R9A08G045S_CLK_WDT1,
				R9A08G045S_CLK_P0,
				MSSR(18, (BIT(2) | BIT(3)), BIT(1))),
	DEF_MOD("wdt2",		R9A08G045S_CLK_WDT2,
				R9A08G045S_CLK_P0,
				MSSR(18, (BIT(4) | BIT(5)), BIT(2))),
	DEF_MOD("spi-multi",	R9A08G045S_CLK_SPI,
				R9A08G045S_CLK_SPI1,
				MSSR(20, (BIT(0) | BIT(1) | BIT(3) | BIT(4)),
					 (BIT(0) | BIT(1)))),
	DEF_MOD("sdhi0",	R9A08G045S_CLK_SDHI0,
				R9A08G045S_CLK_SD0,
				MSSR(21, (BIT(0) | BIT(1) | BIT(2) | BIT(3)),
					  BIT(0))),
	DEF_MOD("sdhi1",	R9A08G045S_CLK_SDHI1,
				R9A08G045S_CLK_SD1,
				MSSR(21, (BIT(4) | BIT(5) | BIT(6) | BIT(7)),
					  BIT(1))),
	DEF_MOD("sdhi2",	R9A08G045S_CLK_SDHI2,
				R9A08G045S_CLK_SD2,
				MSSR(21, (BIT(8) | BIT(9) | BIT(10) | BIT(11)),
					  BIT(2))),
	DEF_MOD("usb0",		R9A08G045S_CLK_USB0,
				R9A08G045S_CLK_P1,
				MSSR(30, (BIT(0) | BIT(2) | BIT(3)),
					 (BIT(0) | BIT(2) | BIT(3)))),
	DEF_MOD("usb1",		R9A08G045S_CLK_USB1,
				R9A08G045S_CLK_P1,
				MSSR(30, (BIT(1) | BIT(3)),
					 (BIT(1) | BIT(3)))),
	DEF_MOD("eth0",		R9A08G045S_CLK_ETH0,
				R9A08G045S_CLK_HP,
				MSSR(31, BIT(0), BIT(0))),
	DEF_MOD("eth1",		R9A08G045S_CLK_ETH1,
				R9A08G045S_CLK_HP,
				MSSR(31, BIT(1), BIT(1))),
	DEF_MOD("i2c0",		R9A08G045S_CLK_I2C0,
				R9A08G045S_CLK_P0,
				MSSR(32, BIT(0), BIT(0))),
	DEF_MOD("i2c1",		R9A08G045S_CLK_I2C1,
				R9A08G045S_CLK_P0,
				MSSR(32, BIT(1), BIT(1))),
	DEF_MOD("i2c2",		R9A08G045S_CLK_I2C2,
				R9A08G045S_CLK_P0,
				MSSR(32, BIT(2), BIT(2))),
	DEF_MOD("i2c3",		R9A08G045S_CLK_I2C3,
				R9A08G045S_CLK_P0,
				MSSR(32, BIT(3), BIT(3))),
	DEF_MOD("scif0",	R9A08G045S_CLK_SCIF0,
				R9A08G045S_CLK_P0,
				MSSR(33, BIT(0), BIT(0))),
	DEF_MOD("scif1",	R9A08G045S_CLK_SCIF1,
				R9A08G045S_CLK_P0,
				MSSR(33, BIT(1), BIT(1))),
	DEF_MOD("scif2",	R9A08G045S_CLK_SCIF2,
				R9A08G045S_CLK_P0,
				MSSR(33, BIT(2), BIT(2))),
	DEF_MOD("scif3",	R9A08G045S_CLK_SCIF3,
				R9A08G045S_CLK_P0,
				MSSR(33, BIT(3), BIT(3))),
	DEF_MOD("scif4",	R9A08G045S_CLK_SCIF4,
				R9A08G045S_CLK_P0,
				MSSR(33, BIT(4), BIT(4))),
	DEF_MOD("scif5",	R9A08G045S_CLK_SCIF5,
				R9A08G045S_CLK_P0,
				MSSR(33, BIT(5), BIT(5))),
	DEF_MOD("sci0",		R9A08G045S_CLK_SCI0,
				R9A08G045S_CLK_P0,
				MSSR(34, BIT(0), BIT(0))),
	DEF_MOD("sci1",		R9A08G045S_CLK_SCI1,
				R9A08G045S_CLK_P0,
				MSSR(34, BIT(1), BIT(1))),
	DEF_MOD("gpio",		R9A08G045S_CLK_GPIO,
				CLK_XINCLK,
				MSSR(38, BIT(0), (BIT(0) | BIT(1) | BIT(2)))),
	DEF_MOD("rspi0",	R9A08G045S_CLK_RSPI0,
				R9A08G045S_CLK_P0,
				MSSR(36, BIT(0), BIT(0))),
	DEF_MOD("rspi1",	R9A08G045S_CLK_RSPI1,
				R9A08G045S_CLK_P0,
				MSSR(36, BIT(1), BIT(1))),
	DEF_MOD("rspi2",	R9A08G045S_CLK_RSPI2,
				R9A08G045S_CLK_P0,
				MSSR(36, BIT(2), BIT(2))),
	DEF_MOD("canfd",	R9A08G045S_CLK_CANFD,
				R9A08G045S_CLK_P4,
				MSSR(37, (BIT(0) | BIT(1)), (BIT(0) | BIT(1)))),
	DEF_MOD("adc",		R9A08G045S_CLK_ADC,
				R9A08G045S_CLK_TSU,
				MSSR(42, (BIT(0) | BIT(1)), (BIT(0) | BIT(1)))),
	DEF_MOD("tsu",		R9A08G045S_CLK_TSU_PCLK,
				R9A08G045S_CLK_TSU,
				MSSR(43, BIT(0), BIT(0))),
};

static const unsigned int r9a08g045s_crit_mod_clks[] = {
	CLK_MODE_BASE + R9A08G045S_CLK_GIC600,
};

/* clock type, register offset1, register offset2, register offset3*/
static const struct cpg_pll_info cpg_pll_configs[] = {
	{ CLK_TYPE_PLL1, PLL146_CLK1_R(0), PLL146_CLK2_R(0), 0},
	{ CLK_TYPE_PLL2, PLL235_CLK1_R(0), PLL235_CLK3_R(0), PLL235_CLK4_R(0)},
	{ CLK_TYPE_PLL3, PLL235_CLK1_R(1), PLL235_CLK3_R(1), PLL235_CLK4_R(1)},
	{ CLK_TYPE_PLL4, PLL146_CLK1_R(1), PLL146_CLK2_R(1), 0},
	{ CLK_TYPE_PLL5, PLL235_CLK1_R(2), PLL235_CLK3_R(2), PLL235_CLK4_R(2)},
	{ CLK_TYPE_PLL6, PLL146_CLK1_R(2), PLL146_CLK2_R(2), 0},
};

/* Some struct value not defined: mstp_table, reset_node,get_pll_config */
const struct cpg_mssr_info r9a08g045s_cpg_info = {
	.core_clk		= r9a08g045s_core_clks,
	.core_clk_size		= ARRAY_SIZE(r9a08g045s_core_clks),
	.mod_clk		= r9a08g045s_mod_clks,
	.mod_clk_size		= ARRAY_SIZE(r9a08g045s_mod_clks),
	.clk_extal_id		= CLK_XINCLK,
	.mod_clk_base		= CLK_MODE_BASE,
};

static const struct udevice_id r9a08g045s_clk_ids[] = {
	{
		.compatible	= "renesas,r9a08g045s-cpg",
		.data		= (ulong)&r9a08g045s_cpg_info,
	},
	{ }
};

U_BOOT_DRIVER(clk_r9a08g045s) = {
	.name		= "clk_r9a08g045s",
	.id		= UCLASS_CLK,
	.of_match	= r9a08g045s_clk_ids,
	.priv_auto	= sizeof(struct rzg2l_clk_priv),
	.ops		= &rzg2l_clk_ops,
	.probe		= rzg2l_clk_probe,
	.remove		= rzg2l_clk_remove,
};
