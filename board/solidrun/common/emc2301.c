// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 SolidRun ltd. Based on code from NXP LX2160A RDB
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <asm/io.h>

#include "emc2301.h"

DECLARE_GLOBAL_DATA_PTR;

void set_fan_speed(u8 data)
{
	if (i2c_write(I2C_EMC2301_ADDR, I2C_EMC2301_FAN, 1, &data, 1) != 0) {
		puts("Error: failed to change fan speed\n");
	}
}

void emc2301_init(void)
{
	u8 data;

	data = I2C_EMC2301_CMD;
	if (i2c_write(I2C_EMC2301_ADDR, I2C_EMC2301_CONF, 1, &data, 1) != 0)
		puts("Error: failed to configure EMC2301\n");
}
