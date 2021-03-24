/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * include/configs/hihope-rzg2h.h
 *     This file is HOPERUN HiHope-RZG2H board configuration.
 *
 * Copyright (C) 2020 Renesas Electronics Corporation
 */

#ifndef __HIHOPE_RZG2H_H
#define __HIHOPE_RZG2H_H

#include "rcar-gen3-common.h"

/* Ethernet RAVB */
#define CONFIG_BITBANGMII
#define CONFIG_BITBANGMII_MULTI

/* Generic Timer Definitions (use in assembler source) */
#define COUNTER_FREQUENCY	0xFE502A	/* 16.66MHz from CPclk */

#define CONFIG_BOARD_LATE_INIT

/* ENV setting */
#define CONFIG_ENV_OVERWRITE

#endif /* __HIHOPE_RZG2H_H */
