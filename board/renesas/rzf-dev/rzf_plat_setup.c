/*
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common.h>
#include "include/rzf_def.h"
#include "include/mmio.h"
#include "include/pfc.h"
#include "include/cpg.h"
#include "include/syc.h"
#include "include/ddr.h"
#include "include/sys_regs.h"
#include "include/rzf_def.h"
#include "include/rz_private.h"


void rzf_early_platform_setup(void)
{
//	int ret;

	/* early setup Clock and Reset */
	cpg_early_setup();

	/* initialize SYC */
	syc_init(RZF_SYC_INCK_HZ);

	/* initialize Timer */
//	generic_delay_timer_init();

	/* setup PFC */
	pfc_setup();

	/* setup Clock and Reset */
	cpg_setup();

	/* initialize console driver */
//	ret = console_rzg2l_register(
//							RZG2L_SCIF0_BASE,
//							RZG2L_UART_INCK_HZ,
//							RZG2L_UART_BARDRATE,
//							&rzg2l_bl31_console);
//	if (!ret)
//		panic();
//
//	console_set_scope(&rzg2l_bl31_console,
//			CONSOLE_FLAG_BOOT | CONSOLE_FLAG_CRASH);
}

void rzf_platform_setup(void)
{
	/* Setup TZC-400, Access Control */
//	plat_security_setup();

//#if !DEBUG_RZG2L_FPGA
	/* initialize DDR */
	ddr_setup();
//#endif /* DEBUG_FPGA */

	rz_io_setup();
}
