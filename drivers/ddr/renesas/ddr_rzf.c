// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021, Renesas Electronics Corporation. All rights reserved.
 */

#include <common.h>
#include "include/rzf-dev_def.h"
#include "include/ddr_internal.h"
#include "include/rzf-dev_sys_regs.h"

void ddr_ctrl_reten_en_n(uint8_t val)
{
	val &= 1;
	if ((mmio_read_32(SYS_LSI_DEVID) >> 28) + 1 > 1)
	{
		write_phy_reg(DDRPHY_R79, (val << 1));
	}
	else
	{
		rmw_phy_reg(DDRPHY_R78, 0xFFFEFFFF, (val << 16));
	}
}

char *ddr_get_version(void)
{
	return (((mmio_read_32(SYS_LSI_DEVID) >> 28) + 1 > 1) ? AN_VERSION : AN_VERSION_0);
}
