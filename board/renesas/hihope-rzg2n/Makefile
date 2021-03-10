#
# board/renesas/hihope-rzg2n/Makefile
#
# Copyright (C) 2021 Renesas Electronics Corporation
#
# SPDX-License-Identifier: GPL-2.0+
#

ifdef CONFIG_SPL_BUILD
obj-y	:= ../rcar-common/gen3-spl.o
else
obj-y	:= hihope-rzg2n.o ../rcar-common/common.o ../rzg-common/update_fdt.o
endif
