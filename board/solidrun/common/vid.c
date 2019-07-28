// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Copyright 2018 NXP
 */
#include <common.h>
#include <command.h>
#include <i2c.h>
#include <asm/io.h>
#ifdef CONFIG_FSL_LSCH2
#include <asm/arch/immap_lsch2.h>
#elif defined(CONFIG_FSL_LSCH3)
#include <asm/arch/immap_lsch3.h>
#else
#include <asm/immap_85xx.h>
#endif
#include "vid.h"

int __weak i2c_multiplexer_select_vid_channel(u8 channel)
{
	return 0;
}

/*
 * Compensate for a board specific voltage drop between regulator and SoC
 * return a value in mV
 */
int __weak board_vdd_drop_compensation(void)
{
	return 0;
}

/*
 * Board specific settings for specific voltage value
 */
int __weak board_adjust_vdd(int vdd)
{
	return 0;
}

/* Maximum loop count waiting for new voltage to take effect */
#define MAX_LOOP_WAIT_NEW_VOL		100
/* Maximum loop count waiting for the voltage to be stable */
#define MAX_LOOP_WAIT_VOL_STABLE	100
/*
 * read_voltage from sensor on I2C bus
 * We use average of 4 readings, waiting for WAIT_FOR_ADC before
 * another reading
 */
#define NUM_READINGS    4       /* prefer to be power of 2 for efficiency */

/* If an INA220 chip is available, we can use it to read back the voltage
 * as it may have a higher accuracy than the IR chip for the same purpose
 */
#ifdef CONFIG_VOL_MONITOR_INA220
#define WAIT_FOR_ADC	532	/* wait for 532 microseconds for ADC */
#define ADC_MIN_ACCURACY	4
#else
#define WAIT_FOR_ADC	138	/* wait for 138 microseconds for ADC */
#define ADC_MIN_ACCURACY	4
#endif

/* read the current value of the LTC Regulator Voltage */
static int read_voltage_from_LTC(int i2caddress)
{
	int  ret, vcode = 0;
	u8 chan = PWM_CHANNEL0;

	/* select the PAGE 0 using PMBus commands PAGE for VDD*/
	ret = i2c_write(I2C_VOL_MONITOR_ADDR,
			PMBUS_CMD_PAGE, 1, &chan, 1);
	if (ret) {
		printf("VID: failed to select VDD Page 0\n");
		return ret;
	}

	/*read the output voltage using PMBus command READ_VOUT*/
	ret = i2c_read(I2C_VOL_MONITOR_ADDR,
		       PMBUS_CMD_READ_VOUT, 1, (void *)&vcode, 2);
	if (ret) {
		printf("VID: failed to read the volatge\n");
		return ret;
	}

	/* Scale down to the real mV as LTC resolution is 1/4096V,rounding up */
	vcode = DIV_ROUND_UP(vcode * 1000, 4096);

	return vcode;
}

static int read_voltage(int i2caddress)
{
	int voltage_read;
	voltage_read = read_voltage_from_LTC(i2caddress);
	return voltage_read;
}

/* this function sets the VDD and returns the value set */
static int set_voltage_to_LTC(int i2caddress, int vdd)
{
	int ret, vdd_last, vdd_target = vdd;
	int count = 100, temp = 0;

	/* Scale up to the LTC resolution is 1/4096V */
	vdd = (vdd * 4096) / 1000;

	/* 5-byte buffer which needs to be sent following the
	 * PMBus command PAGE_PLUS_WRITE.
	 */
	u8 buff[5] = {0x04, PWM_CHANNEL0, PMBUS_CMD_VOUT_COMMAND,
			vdd & 0xFF, (vdd & 0xFF00) >> 8};

	/* Write the desired voltage code to the regulator */
	ret = i2c_write(I2C_VOL_MONITOR_ADDR,
			PMBUS_CMD_PAGE_PLUS_WRITE, 1, (void *)&buff, 5);
	if (ret) {
		printf("VID: I2C failed to write to the volatge regulator\n");
		return -1;
	}

	/* Wait for the volatge to get to the desired value */
	do {
		vdd_last = read_voltage_from_LTC(i2caddress);
		if (vdd_last < 0) {
			printf("VID: Couldn't read sensor abort VID adjust\n");
			return -1;
		}
		count--;
		temp = vdd_last - vdd_target;
	} while ((abs(temp) > 2)  && (count > 0));

	return vdd_last;
}

static int set_voltage(int i2caddress, int vdd)
{
	int vdd_last = -1;

	vdd_last = set_voltage_to_LTC(i2caddress, vdd);
	return vdd_last;
}

int adjust_vdd(ulong vdd_override)
{
	int re_enable = disable_interrupts();
	struct ccsr_gur *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 fusesr;
	u8 vid;
	int vdd_target, vdd_current, vdd_last;
	int ret, i2caddress;
	unsigned long vdd_string_override;
	char *vdd_string;
	static const u16 vdd[32] = {
		8250,
		7875,
		7750,
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		8000,
		8125,
		8250,
		0,      /* reserved */
		8500,
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
	};
	struct vdd_drive {
		u8 vid;
		unsigned voltage;
	};
	ret = i2c_multiplexer_select_vid_channel(I2C_MUX_CH_VOL_MONITOR);
	if (ret) {
		debug("VID: I2C failed to switch channel\n");
		ret = -1;
		goto exit;
	}
	/* get the voltage ID from fuse status register */
	fusesr = in_le32(&gur->dcfg_fusesr);
	vid = (fusesr >> FSL_CHASSIS3_DCFG_FUSESR_ALTVID_SHIFT) &
		FSL_CHASSIS3_DCFG_FUSESR_ALTVID_MASK;
	if ((vid == 0) || (vid == FSL_CHASSIS3_DCFG_FUSESR_ALTVID_MASK)) {
		vid = (fusesr >> FSL_CHASSIS3_DCFG_FUSESR_VID_SHIFT) &
			FSL_CHASSIS3_DCFG_FUSESR_VID_MASK;
	}
	vdd_target = vdd[vid];
	printf ("vid FUSE index %d (vdd_target = %d)\n",vid,vdd_target);

	/* check override variable for overriding VDD */
	vdd_string = env_get(CONFIG_VID_FLS_ENV);
	if (vdd_override == 0 && vdd_string &&
	    !strict_strtoul(vdd_string, 10, &vdd_string_override))
		vdd_override = vdd_string_override;

	if (vdd_override >= VDD_MV_MIN && vdd_override <= VDD_MV_MAX) {
		vdd_target = vdd_override * 10; /* convert to 1/10 mV */
		debug("VDD override is %lu\n", vdd_override);
	} else if (vdd_override != 0) {
		printf("Invalid value.\n");
	}

	/* divide and round up by 10 to get a value in mV */
	vdd_target = DIV_ROUND_UP(vdd_target, 10);
	if (vdd_target == 0) {
		debug("VID: VID not used\n");
		ret = 0;
		goto exit;
	} else if (vdd_target < VDD_MV_MIN || vdd_target > VDD_MV_MAX) {
		/* Check vdd_target is in valid range */
		printf("VID: Target VID %d mV is not in range.\n",
		       vdd_target);
		ret = -1;
		goto exit;
	} else {
		debug("VID: vid = %d mV\n", vdd_target);
	}

	/*
	 * Read voltage monitor to check real voltage.
	 */
	vdd_last = read_voltage(i2caddress);
	if (vdd_last < 0) {
		printf("VID: Couldn't read sensor abort VID adjustment\n");
		ret = -1;
		goto exit;
	}
	vdd_current = vdd_last;
	debug("VID: Core voltage is currently at %d mV\n", vdd_last);

	/* Set the target voltage */
	vdd_last = vdd_current = set_voltage(i2caddress, vdd_target);
	if (board_adjust_vdd(vdd_target) < 0) {
		ret = -1;
		goto exit;
	}

	if (vdd_last > 0)
		printf("VID: Core voltage after adjustment is at %d mV\n",
		       vdd_last);
	else
		ret = -1;
exit:
	if (re_enable)
		enable_interrupts();
	i2c_multiplexer_select_vid_channel(I2C_MUX_CH_DEFAULT);
	return ret;
}

static int print_vdd(void)
{
	int vdd_last, ret, i2caddress;

	ret = i2c_multiplexer_select_vid_channel(I2C_MUX_CH_VOL_MONITOR);
	if (ret) {
		debug("VID : I2c failed to switch channel\n");
		return -1;
	}
	/*
	 * Read voltage monitor to check real voltage.
	 */
	vdd_last = read_voltage(i2caddress);
	if (vdd_last < 0) {
		printf("VID: Couldn't read sensor abort VID adjustment\n");
		goto exit;
	}
	printf("VID: Core voltage is at %d mV\n", vdd_last);
exit:
	i2c_multiplexer_select_vid_channel(I2C_MUX_CH_DEFAULT);

	return ret < 0 ? -1 : 0;

}

static int do_vdd_override(cmd_tbl_t *cmdtp,
			   int flag, int argc,
			   char * const argv[])
{
	ulong override;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (!strict_strtoul(argv[1], 10, &override))
		adjust_vdd(override);   /* the value is checked by callee */
	else
		return CMD_RET_USAGE;
	return 0;
}

static int do_vdd_read(cmd_tbl_t *cmdtp,
			 int flag, int argc,
			 char * const argv[])
{
	if (argc < 1)
		return CMD_RET_USAGE;
	print_vdd();

	return 0;
}

U_BOOT_CMD(
	vdd_override, 2, 0, do_vdd_override,
	"override VDD",
	" - override with the voltage specified in mV, eg. 1050"
);

U_BOOT_CMD(
	vdd_read, 1, 0, do_vdd_read,
	"read VDD",
	" - Read the voltage specified in mV"
)
