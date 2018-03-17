#
# Copyright (C) 2015 Stefan Roese <sr@denx.de>
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-$(CONFIG_TARGET_CLEARFOG)	:= clearfog.o
obj-$(CONFIG_TARGET_CLEARFOG_BASE)	:= clearfog.o

ifeq ($(CONFIG_SPL_BUILD),)

extra-$(CONFIG_TARGET_CLEARFOG) += kwbimage.cfg
extra-$(CONFIG_TARGET_CLEARFOG_BASE) += kwbimage.cfg

KWB_REPLACE += BOOT_FROM
ifneq ($(CONFIG_MVEBU_SPL_BOOT_DEVICE_MMC),)
        KWB_CFG_BOOT_FROM=sdio
endif
ifneq ($(CONFIG_MVEBU_SPL_BOOT_DEVICE_SATA),)
        KWB_CFG_BOOT_FROM=sata
endif
ifneq ($(CONFIG_MVEBU_SPL_BOOT_DEVICE_SDHC),)
        KWB_CFG_BOOT_FROM=sdio
endif
ifneq ($(CONFIG_MVEBU_SPL_BOOT_DEVICE_SPI),)
        KWB_CFG_BOOT_FROM=spi
endif
ifneq ($(CONFIG_MVEBU_SPL_BOOT_DEVICE_UART),)
        KWB_CFG_BOOT_FROM=uart
endif

ifneq ($(CONFIG_SECURED_MODE_IMAGE),)
KWB_REPLACE += CSK_INDEX
KWB_CFG_CSK_INDEX = $(CONFIG_SECURED_MODE_CSK_INDEX)

KWB_REPLACE += SEC_BOOT_DEV
KWB_CFG_SEC_BOOT_DEV=$(patsubst "%",%, \
	$(if $(CONFIG_MVEBU_SPL_BOOT_DEVICE_MMC),0x31) \
	$(if $(CONFIG_MVEBU_SPL_BOOT_DEVICE_SCSI),0x22) \
	$(if $(CONFIG_MVEBU_SPL_BOOT_DEVICE_SDHC),0x31) \
	$(if $(CONFIG_MVEBU_SPL_BOOT_DEVICE_SPI)),0x34) \
	)

KWB_REPLACE += SEC_FUSE_DUMP
KWB_CFG_SEC_FUSE_DUMP = a38x
endif

$(src)/kwbimage.cfg: $(src)/kwbimage.cfg.in include/autoconf.mk \
		include/config/auto.conf
	$(Q)sed -ne '$(foreach V,$(KWB_REPLACE),s/^#@$(V)/$(V) $(KWB_CFG_$(V))/;)p' \
	<$< >$(dir $<)$(@F)

endif
