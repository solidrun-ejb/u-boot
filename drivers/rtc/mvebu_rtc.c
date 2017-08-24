/*
 * (C) Copyright 2015 Solid Run Ltd.
 * Author: Jon Nettleton <jon@solid-run.com>
 * 
 * Based on Linux Kernel driver rtc-armada38x.c
 *   Copyright (C) 2015 Marvell
 *   Gregory Clement <gregory.clement@free-electrons.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <dm.h>
#include <rtc.h>

DECLARE_GLOBAL_DATA_PTR;

#define RTC_NOMINAL_TIMING              0x2000

#define RTC_STATUS          0x0
#define RTC_STATUS_ALARM1   BIT(0)
#define RTC_STATUS_ALARM2   BIT(1)
#define RTC_TIME            0xC
#define RTC_ALARM1          0x10
#define RTC_CLOCK_CORR      0x18
#define RTC_TEST_CONF       0x1C

/* armada38x SoC registers  */
#define RTC_38X_BRIDGE_TIMING_CTRL_REG_OFFS             0x0
#define RTC_38X_WRCLK_PERIOD_OFFS                       0
#define RTC_38X_WRCLK_PERIOD_MASK                       (0x3FF << RTC_38X_WRCLK_PERIOD_OFFS)
#define RTC_38X_READ_OUTPUT_DELAY_OFFS                  26
#define RTC_38X_READ_OUTPUT_DELAY_MASK                  (0x1F << RTC_38X_READ_OUTPUT_DELAY_OFFS)

struct mvebu_rtc_platdata {
	fdt_addr_t base;
	fdt_addr_t soc_base;
};

/*
 * According to the datasheet, the OS should wait 5us after every
 * register write to the RTC hard macro so that the required update
 * can occur without holding off the system bus
 * According to errata FE-3124064, Write to any RTC register
 * may fail. As a workaround, before writing to RTC
 * register, issue a dummy write of 0x0 twice to RTC Status
 * register.
 */

static void rtc_delayed_write(u32 val, struct mvebu_rtc_platdata *rtc, int offset)
{
        writel(0, rtc->base + RTC_STATUS);
        writel(0, rtc->base + RTC_STATUS);
        writel(val, rtc->base + offset);
        mdelay(10);
}

static unsigned long read_rtc_reg(struct mvebu_rtc_platdata *rtc, uint8_t rtc_reg)
{
        unsigned long value = readl(rtc->base + rtc_reg);

        return value;
}

static void rtc_update_38x_mbus_timing_params(struct mvebu_rtc_platdata *rtc)
{
        uint32_t reg;

        reg = readl(rtc->soc_base + RTC_38X_BRIDGE_TIMING_CTRL_REG_OFFS);
        reg &= ~RTC_38X_WRCLK_PERIOD_MASK;
        reg |= 0x3FF << RTC_38X_WRCLK_PERIOD_OFFS; /*Maximum value*/
        reg &= ~RTC_38X_READ_OUTPUT_DELAY_MASK;
        reg |= 0x1F << RTC_38X_READ_OUTPUT_DELAY_OFFS; /*Maximum value*/
        writel(reg, rtc->soc_base + RTC_38X_BRIDGE_TIMING_CTRL_REG_OFFS);
}

static int mvebu_rtc_get(struct udevice *dev, struct rtc_time *tm)
{
	struct mvebu_rtc_platdata *rtc = dev_get_platdata(dev);

	rtc_to_tm(read_rtc_reg(rtc, RTC_TIME), tm);

	return 0;
}

static int mvebu_rtc_set(struct udevice *dev, const struct rtc_time *tm)
{
	struct mvebu_rtc_platdata *rtc = dev_get_platdata(dev);
	unsigned long time;

	time = rtc_mktime(tm);
	rtc_delayed_write(time, rtc, RTC_TIME);

	return 0;
}

static int mvebu_rtc_reset(struct udevice *dev)
{
	struct mvebu_rtc_platdata *rtc = dev_get_platdata(dev);

	rtc_delayed_write(0, rtc, RTC_TEST_CONF);
	rtc_delayed_write(0, rtc, RTC_TIME);
	rtc_delayed_write((RTC_STATUS_ALARM1 | RTC_STATUS_ALARM2), rtc, RTC_STATUS);
	rtc_delayed_write(RTC_NOMINAL_TIMING, rtc, RTC_CLOCK_CORR);

	return 0;
}

static int mvebu_rtc_read8(struct udevice *dev, unsigned int reg)
{
	return -ENOSYS;
}

static int mvebu_rtc_write8(struct udevice *dev, unsigned int reg, int val)
{
	return -ENOSYS;
}

static int mvebu_rtc_probe(struct udevice *dev)
{
	struct mvebu_rtc_platdata *rtc = dev_get_platdata(dev);

	rtc->base = devfdt_get_addr(dev);
	rtc->soc_base = devfdt_get_addr_name(dev, "rtc-soc");

	rtc_update_38x_mbus_timing_params(rtc);
	
	return 0;
}

static const struct rtc_ops mvebu_rtc_ops = {
	.get = mvebu_rtc_get,
	.set = mvebu_rtc_set,
	.reset = mvebu_rtc_reset,
	.read8 = mvebu_rtc_read8,
	.write8 = mvebu_rtc_write8,
};

static const struct udevice_id mvebu_rtc_ids[] = {
	{ .compatible = "marvell,armada-380-rtc" },
	{ }
};

U_BOOT_DRIVER(rtc_mvebu) = {
	.name	= "rtc-mvebu",
	.id	= UCLASS_RTC,
	.of_match = mvebu_rtc_ids,
	.probe = mvebu_rtc_probe,
	.ops	= &mvebu_rtc_ops,
};
