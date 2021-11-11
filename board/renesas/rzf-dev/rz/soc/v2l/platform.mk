#
# Copyright (c) 2021, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include plat/renesas/rz/common/rz_common.mk
include plat/renesas/rz/board/${BOARD}/rz_board.mk

PLAT_INCLUDES	+=	-Iplat/renesas/rz/soc/v2l/include

DDR_SOURCES += plat/renesas/rz/soc/v2l/drivers/ddr/ddr_v2l.c

