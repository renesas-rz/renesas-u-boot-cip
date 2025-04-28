#
# board/renesas/rzt2n-dev/Makefile
#
# Copyright (C) 2023 Renesas Electronics Corporation
#
# SPDX-License-Identifier: GPL-2.0+
#

ifdef CONFIG_SPL_BUILD
obj-y	:= ../rcar-common/gen3-spl.o
else
obj-y	:= rzt2n-dev.o rzt2n-dev_eth.o ../rcar-common/rzt2n/common.o
endif
