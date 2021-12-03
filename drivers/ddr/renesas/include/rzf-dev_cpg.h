// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021, Renesas Electronics Corporation. All rights reserved.
 */

#ifndef __RZF_DEV_CPG_H__
#define __RZF_DEV_CPG_H__

void cpg_setup(void);
void cpg_active_ddr(void (*disable_phy)(void));
void cpg_reset_ddr_mc(void);

#endif /* __RZF_DEV_CPG_H__ */
