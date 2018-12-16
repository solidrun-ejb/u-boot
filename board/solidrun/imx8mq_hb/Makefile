#
# Copyright 2018 Solid-Run Ltd.
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += imx8mq_hb.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_timing.o
else
obj-$(CONFIG_USB_TCPC) += ../../freescale/common/tcpc.o
endif

obj-$(CONFIG_POWER_PFUZE100)    += ../../freescale/common/pfuze.o
obj-$(CONFIG_$(SPL_)DM_PMIC_PFUZE100) += ../../freescale/common/pfuze_dm.o
