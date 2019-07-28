# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

MINIMAL=

ifdef CONFIG_SPL_BUILD
ifdef CONFIG_SPL_INIT_MINIMAL
MINIMAL=y
endif
endif

ifdef MINIMAL
# necessary to create built-in.o
obj- := __dummy__.o
else
obj-$(CONFIG_VID)		+= vid.o
obj-$(CONFIG_EMC2301)		+= emc2301.o
ifdef CONFIG_SECURE_BOOT
obj-$(CONFIG_CMD_ESBC_VALIDATE) += fsl_validate.o cmd_esbc_validate.o
endif
obj-$(CONFIG_CHAIN_OF_TRUST) += fsl_chain_of_trust.o
endif
