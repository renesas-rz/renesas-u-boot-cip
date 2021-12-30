/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * include/configs/silinux-ek874.h
 *     This file is Silicon Linux EK874 board configuration.
 *
 * Copyright (C) 2021 Renesas Electronics Corporation
 */

#ifndef __SILINUX_EK874_H
#define __SILINUX_EK874_H

#include "rcar-gen3-common.h"

/* Reserved mem for Kernel/dtb ECC area */
#define CONFIG_PRAM		(32 * 1024)

#endif /* __SILINUX_EK874_H */
