/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 SolidRun ltd.
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __EMC2301_H_
#define __EMC2301_H_

#define I2C_EMC2301_CONF		0x20
#define I2C_EMC2301_FAN			0x30

void emc2301_init(void);
void set_fan_speed(u8 data);

#endif  /* __EMC2305_H_ */
