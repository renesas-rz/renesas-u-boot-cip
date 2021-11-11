/*
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stddef.h>
#include <pfc_regs.h>
#include <lib/mmio.h>

static PFC_REGS pfc_mux_reg_tbl[PFC_MUX_TBL_NUM] = {
	/* P18(sd0) */
	{
		{ PFC_ON,  (uintptr_t)PFC_PMC22,  0x03 },					/* PMC */
		{ PFC_ON,  (uintptr_t)PFC_PFC22,  0x00000011 },				/* PFC */
		{ PFC_OFF, (uintptr_t)PFC_IOLH22, 0x0000000000000101 },		/* IOLH */
		{ PFC_OFF, (uintptr_t)PFC_PUPD22, 0x0000000000000000 },		/* PUPD */
		{ PFC_OFF, (uintptr_t)PFC_SR22,   0x0000000000000101 },		/* SR */
		{ PFC_OFF, (uintptr_t)NULL,       0 }						/* IEN */
	},
	/* P19(sd1) */
	{
		{ PFC_ON,  (uintptr_t)PFC_PMC23,  0x03 },					/* PMC */
		{ PFC_ON,  (uintptr_t)PFC_PFC23,  0x00000011 },				/* PFC */
		{ PFC_OFF, (uintptr_t)PFC_IOLH23, 0x0000000000000101 },		/* IOLH */
		{ PFC_OFF, (uintptr_t)PFC_PUPD23, 0x0000000000000000 },		/* PUPD */
		{ PFC_OFF, (uintptr_t)PFC_SR23,   0x0000000000000101 },		/* SR */
		{ PFC_OFF, (uintptr_t)NULL,       0 }						/* IEN */
	},
	/* P38(scif0) */
	{
		{ PFC_ON,  (uintptr_t)PFC_PMC36,  0x03 },					/* PMC */
		{ PFC_ON,  (uintptr_t)PFC_PFC36,  0x00000011 },				/* PFC */
		{ PFC_OFF, (uintptr_t)PFC_IOLH36, 0x0000000000000101 },		/* IOLH */
		{ PFC_OFF, (uintptr_t)PFC_PUPD36, 0x0000000000000000 },		/* PUPD */
		{ PFC_OFF, (uintptr_t)PFC_SR36,   0x0000000000000101 },		/* SR */
		{ PFC_OFF, (uintptr_t)NULL,       0 }						/* IEN */
	},
	/* P39(scif0) */
	{
		{ PFC_ON,  (uintptr_t)PFC_PMC37,  0x07 },					/* PMC */
		{ PFC_ON,  (uintptr_t)PFC_PFC37,  0x00000111 },				/* PFC */
		{ PFC_OFF, (uintptr_t)PFC_IOLH37, 0x0000000000010101 },		/* IOLH */
		{ PFC_OFF, (uintptr_t)PFC_PUPD37, 0x0000000000000000 },		/* PUPD */
		{ PFC_OFF, (uintptr_t)PFC_SR37,   0x0000000000010101 },		/* SR */
		{ PFC_OFF, (uintptr_t)NULL,       0 }						/* IEN */
	}
};

static PFC_REGS  pfc_qspi_reg_tbl[PFC_QSPI_TBL_NUM] = {
	/* QSPI0 */
	{
		{ PFC_OFF, (uintptr_t)NULL,       0 },						/* PMC */
		{ PFC_OFF, (uintptr_t)NULL,       0 },						/* PFC */
		{ PFC_ON,  (uintptr_t)PFC_IOLH0A, 0x0000020202020202 },		/* IOLH */
		{ PFC_ON,  (uintptr_t)PFC_PUPD0A, 0x0000000000000000 },		/* PUPD */
		{ PFC_ON,  (uintptr_t)PFC_SR0A,   0x0000010101010101 },		/* SR */
		{ PFC_OFF, (uintptr_t)NULL,       0 }						/* IEN */
	},
	/* QSPI1 */
	{
		{ PFC_OFF, (uintptr_t)NULL,       0 },						/* PMC */
		{ PFC_OFF, (uintptr_t)NULL,       0 },						/* PFC */
		{ PFC_ON,  (uintptr_t)PFC_IOLH0B, 0x0000020202020202 },		/* IOLH */
		{ PFC_ON,  (uintptr_t)PFC_PUPD0B, 0x0000000000000000 },		/* PUPD */
		{ PFC_ON,  (uintptr_t)PFC_SR0B,   0x0000010101010101 },		/* SR */
		{ PFC_OFF, (uintptr_t)NULL,       0 }						/* IEN */
	},
	/* QSPIn */
	{
		{ PFC_OFF, (uintptr_t)NULL,       0 },						/* PMC */
		{ PFC_OFF, (uintptr_t)NULL,       0 },						/* PFC */
		{ PFC_ON,  (uintptr_t)PFC_IOLH0C, 0x0000000000020202 },		/* IOLH */
		{ PFC_ON,  (uintptr_t)PFC_PUPD0C, 0x0000000000000000 },		/* PUPD */
		{ PFC_ON,  (uintptr_t)PFC_SR0C,   0x0000000000010000 },		/* SR */
		{ PFC_OFF, (uintptr_t)NULL,       0 }						/* IEN */
	}
};

static PFC_REGS  pfc_sd_reg_tbl[PFC_SD_TBL_NUM] = {
	/* SD0_CLK */
	{
		{ PFC_ON,  (uintptr_t)PFC_PMC22,  0x0003 },					/* PMC */
		{ PFC_ON,  (uintptr_t)PFC_PFC22,  0x00000003 },				/* PFC */
		{ PFC_ON,  (uintptr_t)PFC_IOLH06, 0x0000000000020202 },		/* IOLH */
		{ PFC_ON,  (uintptr_t)PFC_PUPD06, 0x0000000000000000 },		/* PUPD */
		{ PFC_ON,  (uintptr_t)PFC_SR06,   0x0000000000010101 },		/* SR */
		{ PFC_ON,  (uintptr_t)PFC_IEN06,  0x0000000000000100 }		/* IEN */
	},
	/* SD0_DATA */
	{
		{ PFC_OFF, (uintptr_t)NULL,       0 },						/* PMC */
		{ PFC_OFF, (uintptr_t)NULL,       0 },						/* PFC */
		{ PFC_ON,  (uintptr_t)PFC_IOLH07, 0x0202020202020202 },		/* IOLH */
		{ PFC_ON,  (uintptr_t)PFC_PUPD07, 0x0000000000000000 },		/* PUPD */
		{ PFC_ON,  (uintptr_t)PFC_SR07,   0x0101010101010101 },		/* SR */
		{ PFC_ON,  (uintptr_t)PFC_IEN07,  0x0101010101010101 }		/* IEN */
	},
	/* SD1_CLK */
	{
		{ PFC_ON,  (uintptr_t)PFC_PMC23,  0x0003 },					/* PMC */
		{ PFC_ON,  (uintptr_t)PFC_PFC23,  0x00000003 },				/* PFC */
		{ PFC_ON,  (uintptr_t)PFC_IOLH08, 0x0000000000000202 },		/* IOLH */
		{ PFC_ON,  (uintptr_t)PFC_PUPD08, 0x0000000000000000 },		/* PUPD */
		{ PFC_ON,  (uintptr_t)PFC_SR08,   0x0000000000000101 },		/* SR */
		{ PFC_ON,  (uintptr_t)PFC_IEN08,  0x0000000000000100 }		/* IEN */
	},
	/* SD1_DATA */
	{
		{ PFC_OFF, (uintptr_t)NULL,       0 },						/* PMC */
		{ PFC_OFF, (uintptr_t)NULL,       0 },						/* PFC */
		{ PFC_ON,  (uintptr_t)PFC_IOLH09, 0x0000000002020202 },		/* IOLH */
		{ PFC_ON,  (uintptr_t)PFC_PUPD09, 0x0000000000000000 },		/* PUPD */
		{ PFC_ON,  (uintptr_t)PFC_SR09,   0x0000000001010101 },		/* SR */
		{ PFC_ON,  (uintptr_t)PFC_IEN09,  0x0000000001010101 }		/* IEN */
	}
};

static void pfc_mux_setup(void)
{
	int      cnt;

	/* multiplexer terminal switching */
	mmio_write_32(PFC_PWPR, 0x0);
	mmio_write_32(PFC_PWPR, PWPR_PFCWE);

	for (cnt = 0; cnt < PFC_MUX_TBL_NUM; cnt++) {
		/* PMC */
		if (pfc_mux_reg_tbl[cnt].pmc.flg == PFC_ON) {
			mmio_write_8(pfc_mux_reg_tbl[cnt].pmc.reg, pfc_mux_reg_tbl[cnt].pmc.val);
		}
		/* PFC */
		if (pfc_mux_reg_tbl[cnt].pfc.flg == PFC_ON) {
			mmio_write_32(pfc_mux_reg_tbl[cnt].pfc.reg, pfc_mux_reg_tbl[cnt].pfc.val);
		}
		/* IOLH */
		if (pfc_mux_reg_tbl[cnt].iolh.flg == PFC_ON) {
			mmio_write_64(pfc_mux_reg_tbl[cnt].iolh.reg, pfc_mux_reg_tbl[cnt].iolh.val);
		}
		/* PUPD */
		if (pfc_mux_reg_tbl[cnt].pupd.flg == PFC_ON) {
			mmio_write_64(pfc_mux_reg_tbl[cnt].pupd.reg, pfc_mux_reg_tbl[cnt].pupd.val);
		}
		/* SR */
		if (pfc_mux_reg_tbl[cnt].sr.flg == PFC_ON) {
			mmio_write_64(pfc_mux_reg_tbl[cnt].sr.reg, pfc_mux_reg_tbl[cnt].sr.val);
		}
	}

	mmio_write_32(PFC_PWPR, 0x0);
	mmio_write_32(PFC_PWPR, PWPR_B0Wl);
}

static void pfc_qspi_setup(void)
{
	int      cnt;

	for (cnt = 0; cnt < PFC_QSPI_TBL_NUM; cnt++) {
		/* IOLH */
		if (pfc_qspi_reg_tbl[cnt].iolh.flg == PFC_ON) {
			mmio_write_64(pfc_qspi_reg_tbl[cnt].iolh.reg, pfc_qspi_reg_tbl[cnt].iolh.val);
		}
		/* PUPD */
		if (pfc_qspi_reg_tbl[cnt].pupd.flg == PFC_ON) {
			mmio_write_64(pfc_qspi_reg_tbl[cnt].pupd.reg, pfc_qspi_reg_tbl[cnt].pupd.val);
		}
		/* SR */
		if (pfc_qspi_reg_tbl[cnt].sr.flg == PFC_ON) {
			mmio_write_64(pfc_qspi_reg_tbl[cnt].sr.reg, pfc_qspi_reg_tbl[cnt].sr.val);
		}
	}
}

static void pfc_sd_setup(void)
{
	int      cnt;

	/* Since SDx is 3.3V, the initial value will be set. */
	mmio_write_32(PFC_SD_ch0, 1);
	mmio_write_32(PFC_SD_ch1, 0);

	for (cnt = 0; cnt < PFC_SD_TBL_NUM; cnt++) {
		/* PMC */
		if (pfc_sd_reg_tbl[cnt].pmc.flg == PFC_ON) {
			mmio_write_8(pfc_sd_reg_tbl[cnt].pmc.reg, pfc_sd_reg_tbl[cnt].pmc.val);
		}
		/* PFC */
		if (pfc_sd_reg_tbl[cnt].pfc.flg == PFC_ON) {
			mmio_write_32(pfc_sd_reg_tbl[cnt].pfc.reg, pfc_sd_reg_tbl[cnt].pfc.val);
		}
		/* IOLH */
		if (pfc_sd_reg_tbl[cnt].iolh.flg == PFC_ON) {
			mmio_write_64(pfc_sd_reg_tbl[cnt].iolh.reg, pfc_sd_reg_tbl[cnt].iolh.val);
		}
		/* PUPD */
		if (pfc_sd_reg_tbl[cnt].pupd.flg == PFC_ON) {
			mmio_write_64(pfc_sd_reg_tbl[cnt].pupd.reg, pfc_sd_reg_tbl[cnt].pupd.val);
		}
		/* SR */
		if (pfc_sd_reg_tbl[cnt].sr.flg == PFC_ON) {
			mmio_write_64(pfc_sd_reg_tbl[cnt].sr.reg, pfc_sd_reg_tbl[cnt].sr.val);
		}
		/* IEN */
		if (pfc_sd_reg_tbl[cnt].ien.flg == PFC_ON) {
			mmio_write_64(pfc_sd_reg_tbl[cnt].ien.reg, pfc_sd_reg_tbl[cnt].ien.val);
		}
	}
}

void pfc_setup(void)
{
	pfc_mux_setup();
	pfc_qspi_setup();
	pfc_sd_setup();
}
