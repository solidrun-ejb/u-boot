/*
 * Copyright 2008-2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2018 NXP Semiconductor
 *
 * SPDX-License-Identifier:     GPL-2.0
 */

#ifndef _ASM_ARCH_LAYERSCAPE_GPIO_H_
#define _ASM_ARCH_LAYERSCAPE_GPIO_H_

#ifdef CONFIG_FSL_LSCH2
#include <asm/arch/immap_lsch2.h>
#endif
#ifdef CONFIG_FSL_LSCH3
#include <asm/arch/immap_lsch3.h>
#endif

struct mpc8xxx_gpio_plat {
	ulong addr;
	unsigned long size;
	uint ngpios;
};

#endif
