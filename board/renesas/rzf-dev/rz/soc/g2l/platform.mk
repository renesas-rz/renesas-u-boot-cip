#
# Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include plat/renesas/rz/common/rz_common.mk
include plat/renesas/rz/board/${BOARD}/rz_board.mk

PLAT_INCLUDES	+=	-Iplat/renesas/rz/soc/g2l/include

DDR_SOURCES += plat/renesas/rz/soc/g2l/drivers/ddr/ddr_g2l.c

