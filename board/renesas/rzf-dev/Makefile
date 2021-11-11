# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2017 Andes Technology Corporation.
# Rick Chen, Andes Technology Corporation <rick@andestech.com>

obj-y	:= rzf-dev.o rzf_plat_setup.o drivers/cpg.o drivers/syc.o drivers/pfc.o drivers/ddr/ddr.o drivers/ddr/ddr_rzf.o rzf_plat_storage.o drivers/spi_multi.o
obj-$(CONFIG_RZF_DDR_C_010_D3_02_2)	+= drivers/ddr/param_mc_C-010_D3-02-2.o
obj-$(CONFIG_RZF_DDR_C_010_D4_01_1)	+= drivers/ddr/param_mc_C-010_D4-01-1.o
obj-$(CONFIG_RZF_DDR_C_010_D4_01_2)	+= drivers/ddr/param_mc_C-010_D4-01-2.o
obj-$(CONFIG_RZF_DDR_C_011_D4_01_1)	+= drivers/ddr/param_mc_C-011_D4-01-1.o

obj-$(CONFIG_RZF_DDR_SWIZZLE_T1BC)	+= drivers/ddr/param_swizzle_T1bc.o
obj-$(CONFIG_RZF_DDR_SWIZZLE_T1C)	+= drivers/ddr/param_swizzle_T1c.o
obj-$(CONFIG_RZF_DDR_SWIZZLE_T2C)	+= drivers/ddr/param_swizzle_T2c.o
obj-$(CONFIG_RZF_DDR_SWIZZLE_T3CL)	+= drivers/ddr/param_swizzle_T3cl.o

