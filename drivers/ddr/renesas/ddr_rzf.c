// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021, Renesas Electronics Corporation. All rights reserved.
 */

#include <common.h>
#include "include/rzf-dev_def.h"
#include "include/ddr_internal.h"

#define	SYS_LSI_DEVID	(0x11020A04)

void ddr_ctrl_reten_en_n(uint8_t val)
{
	val &= 1;
	if ((mmio_read_32(SYS_LSI_DEVID) >> 28) == 0)
	{
		rmw_phy_reg(DDRPHY_R78, 0xFFFEFFFF, (val << 16));
	}
	else
	{
		write_phy_reg(DDRPHY_R79, (val << 1));
	}
}
