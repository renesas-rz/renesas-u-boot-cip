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

	/* early setup Clock and Reset */
	cpg_early_setup();

	/* initialize SYC */
	syc_init(RZF_SYC_INCK_HZ);

	/* setup PFC */
	pfc_setup();

	/* setup Clock and Reset */
	cpg_setup();

}

void rzf_platform_setup(void)
{

#ifndef CONFIG_DEBUG_RZG2L_FPGA
	/* initialize DDR */
	ddr_setup();
#endif /* DEBUG_FPGA */

	rz_io_setup();
}
