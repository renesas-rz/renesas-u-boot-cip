# SPDX-License-Identifier: GPL-2.0+

include $(srctree)/scripts/Makefile.dts

targets += $(dtb-y)

# overlay dtbo build
dtb-$(CONFIG_R9A09G057) += renesas/rzv2-evk-cn15-emmc.dtbo
dtb-$(CONFIG_R9A09G057) += renesas/rzv2-evk-cn15-sd.dtbo

# Add any required device tree compiler flags here
DTC_FLAGS += -a 0x8

ifdef CONFIG_RCAR_64
DTC_FLAGS += -R 4 -p 0x1000
endif

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := */*.dtb */*.dtbo
