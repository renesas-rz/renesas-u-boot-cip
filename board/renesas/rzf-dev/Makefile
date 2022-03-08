# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2021, Renesas Electronics Corporation. All rights reserved.

obj-y	:= rzf-dev.o rzf-dev_cpg.o rzf-dev_pfc.o

obj-y	+= spi_multi/rzf-dev_spi.o
obj-$(CONFIG_RZF_SPI_FLASH_AT25QL128A)		+= spi_multi/AT25QL128A/rzf-dev_spi_multi_device.o
obj-$(CONFIG_RZF_SPI_FLASH_MT25QU512ABB)	+= spi_multi/MT25QU512ABB/rzf-dev_spi_multi_device.o

