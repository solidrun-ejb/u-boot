/*
 * Copyright 2017 2018 NXP.
 * Copyright 2018 SolidRun Ltd.
 *
 * FSL i.MX8M USB PHY Controller
 *
 * Author: Jon Nettleton <jon@solid-run.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <generic-phy.h>
#include <linux/errno.h>
#include <linux/compat.h>
#include <asm/arch/sys_proto.h>

#define PHY_CTRL0			0x0
#define PHY_CTRL0_REF_SSP_EN		BIT(2)

#define PHY_CTRL1			0x4
#define PHY_CTRL1_RESET			BIT(0)
#define PHY_CTRL1_COMMONONN		BIT(1)
#define PHY_CTRL1_ATERESET		BIT(3)
#define PHY_CTRL1_VDATSRCENB0		BIT(19)
#define PHY_CTRL1_VDATDETENB0		BIT(20)

#define PHY_CTRL2			0x8
#define PHY_CTRL2_TXENABLEN0		BIT(8)

struct imx8mq_usb_phy {
	fdt_addr_t	base;
	struct clk	clk;
};

static int imx8mq_usb_phy_start(struct phy *_phy)
{
#if CONFIG_IS_ENABLED(CLK)
	struct udevice *dev = _phy->dev;
	struct imx8mq_usb_phy *phy = dev_get_priv(dev);

        return clk_enable(&phy->clk);
#else
	return 0;
#endif
}

static int imx8mq_usb_phy_exit(struct phy *_phy)
{
#if CONFIG_IS_ENABLED(CLK)
	struct udevice *dev = _phy->dev;
	struct imx8mq_usb_phy *phy = dev_get_priv(dev);

        return clk_disable(&phy->clk);
#else
	return 0;
#endif
}

struct phy_ops imx8mq_usb_phy_ops = {
        .init           = imx8mq_usb_phy_start,
        .exit           = imx8mq_usb_phy_exit,
};

static void imx8mq_usb_phy_init(struct imx8mq_usb_phy *phy)
{
        u32 value;

        value = readl(phy->base + PHY_CTRL1);
        value &= ~(PHY_CTRL1_VDATSRCENB0 | PHY_CTRL1_VDATDETENB0 |
                   PHY_CTRL1_COMMONONN);
        value |= PHY_CTRL1_RESET | PHY_CTRL1_ATERESET;
        writel(value, phy->base + PHY_CTRL1);

        value = readl(phy->base + PHY_CTRL0);
        value |= PHY_CTRL0_REF_SSP_EN;
        writel(value, phy->base + PHY_CTRL0);

        value = readl(phy->base + PHY_CTRL2);
        value |= PHY_CTRL2_TXENABLEN0;
        writel(value, phy->base + PHY_CTRL2);

        value = readl(phy->base + PHY_CTRL1);
        value &= ~(PHY_CTRL1_RESET | PHY_CTRL1_ATERESET);
        writel(value, phy->base + PHY_CTRL1);
}

static int imx8mq_usb_phy_probe(struct udevice *dev)
{
	struct imx8mq_usb_phy *phy = dev_get_priv(dev);
#if CONFIG_IS_ENABLED(CLK)
	int ret = 0;
#endif

	phy->base = devfdt_get_addr(dev);
        if (phy->base == FDT_ADDR_T_NONE)
                return -EINVAL;

#if CONFIG_IS_ENABLED(CLK)
	/* Enable clock */
	ret = clk_get_by_name(dev, "usb_phy_root_clk", &phy->clk);
	if (ret)
		return ret;
#endif

	imx8mq_usb_phy_init(phy);

	return 0;
}

static int imx8mq_usb_phy_remove(struct udevice *dev)
{
#if CONFIG_IS_ENABLED(CLK)
	struct imx8mq_usb_phy *phy = dev_get_priv(dev);
	
	clk_free(&phy->clk);
#endif
	return 0;
}

static const struct udevice_id imx8mq_usb_phy_ids[] = {
	{ .compatible = "fsl,imx8mq-usb-phy", },
	{ }
};

U_BOOT_DRIVER(imx8mq_usb_phy_driver) = {
	.name	= "imx8mq-usb-phy",
	.id	= UCLASS_PHY,
	.of_match = imx8mq_usb_phy_ids,
	.probe = imx8mq_usb_phy_probe,
	.remove = imx8mq_usb_phy_remove,
	.ops	= &imx8mq_usb_phy_ops,
	.priv_auto_alloc_size = sizeof(struct imx8mq_usb_phy),
};
