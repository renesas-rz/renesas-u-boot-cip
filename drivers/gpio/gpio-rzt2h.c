// SPDX-License-Identifier: GPL-2.0+
/*
 * GPIO driver for Renesas RZ/T2H SoCs.
 *
 * Copyright (C) 2020 Renesas Electronics Corporation
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/io.h>
#include <linux/bitops.h>
#include <dm/device_compat.h>

#define P(n)	(0x000 + 0x001 * (n))	/* Port Register */
#define PM(n)	(0x200 + 0x002 * (n))	/* Port Mode Register */
#define PMC(n)	(0x400 + 0x001 * (n))	/* Port Mode Control Register */
#define PFC(n)	(0x600 + 0x008 * (n))	/* Port Function Control Register */
#define PIN(n)	(0x800 + 0x001 * (n))	/* Port Input Register */

#define PM_INPUT			0x1 /* Input Mode */
#define PM_OUTPUT			0x2 /* Output Mode (disable Input) */
#define PM_OUTPUT_INPUT			0x3 /* Output Mode (enable Input) */
#define PM_MASK				0x03

#define RZT2H_MAX_GPIO_PER_BANK		8

DECLARE_GLOBAL_DATA_PTR;
struct rzt2h_gpio_priv {
	void __iomem	*regs, *regs1;
	int		bank;
};

void rzt2h_gpio_clrbits_8_security(struct rzt2h_gpio_priv *priv, u8 bank, u16 offset, u16 clear)
{
	if (priv->bank > 12) {
		void __iomem *PRCRN = ioremap(0x80294200, 0x100);
		iowrite32(0x0000A504, PRCRN + 0x0);
		clrbits_8(priv->regs + offset, clear);
		iowrite32(0x0000A500, PRCRN + 0x0);
		iounmap(PRCRN);
	} else
		clrbits_8(priv->regs1 + offset, clear);
}

void rzt2h_gpio_writeb_security(struct rzt2h_gpio_priv *priv, u8 bank, u8 val, u16 offset)
{
	if (priv->bank > 12) {
		void __iomem *PRCRN = ioremap(0x80294200, 0x100);
		iowrite32(0x0000A504, PRCRN + 0x0);
		writeb(val, priv->regs + offset);
		iowrite32(0x0000A500, PRCRN + 0x0);
		iounmap(PRCRN);
	} else
		writeb(val, priv->regs1 + offset);
}

void rzt2h_gpio_writew_security(struct rzt2h_gpio_priv *priv, u8 bank, u16 val, u16 offset)
{
	if (priv->bank > 12) {
		void __iomem *PRCRN = ioremap(0x80294200, 0x100);
		iowrite32(0x0000A504, PRCRN + 0x0);
		writew(val, priv->regs + offset);
		iowrite32(0x0000A500, PRCRN + 0x0);
		iounmap(PRCRN);
	} else
		writew(val, priv->regs1 + offset);
}

static u8 rzt2h_gpio_readb(struct rzt2h_gpio_priv *priv, u8 bank, u16 offset)
{
	if (priv->bank < 13)
		return readb(priv->regs1 + offset);
	else
		return readb(priv->regs + offset);
};

static u16 rzt2h_gpio_readw(struct rzt2h_gpio_priv *priv, u8 bank, u16 offset)
{
	if (priv->bank < 13)
		return readw(priv->regs1 + offset);
	else
		return readw(priv->regs + offset);
};

static u16 rzt2h_gpio_writew(struct rzt2h_gpio_priv *priv, u8 bank, u16 val, u16 offset)
{
	if (priv->bank < 13)
		return writew(val, priv->regs1 + offset);
	else
		return writew(val, priv->regs + offset);
};

static int rzt2h_gpio_get_value(struct udevice *dev, unsigned int offset)
{
	struct rzt2h_gpio_priv *priv = dev_get_priv(dev);
	const u8 bit = BIT(offset);
	u16 reg16;

	reg16 = rzt2h_gpio_readw(priv, priv->bank, PM(priv->bank));
	reg16 = (reg16 >> offset * 2) & PM_MASK;

	if (reg16 == PM_INPUT || reg16 == PM_OUTPUT_INPUT)
		return !!(rzt2h_gpio_readb(priv, priv->bank, PIN(priv->bank)) & bit);
	else if (reg16 == PM_OUTPUT)
		return !!(rzt2h_gpio_readb(priv, priv->bank, P(priv->bank)) & bit);
	else
		return 0;
}

static int rzt2h_gpio_set_value(struct udevice *dev, unsigned int offset,
				int value)
{
	struct rzt2h_gpio_priv *priv = dev_get_priv(dev);
	const u8 bit = BIT(offset);

	if (priv->bank > 12) {
		if (value)
			setbits_8(priv->regs + P(priv->bank), bit);
		else
			clrbits_8(priv->regs + P(priv->bank), bit);
	} else {
		if (value)
			setbits_8(priv->regs1 + P(priv->bank), bit);
		else
			clrbits_8(priv->regs1 + P(priv->bank), bit);
	}
	return 0;
}

static void rzt2h_gpio_set_direction(struct rzt2h_gpio_priv *priv,
				     unsigned int offset, bool output)
{
	u16 reg16;

	/* Select GPIO mode in PMC Register */
	rzt2h_gpio_clrbits_8_security(priv, priv->bank, PMC(priv->bank), BIT(offset));

	reg16 = rzt2h_gpio_readw(priv, priv->bank, PM(priv->bank));
	reg16 = reg16 & ~(PM_MASK << (offset * 2));

	if (output)
		rzt2h_gpio_writew(priv, priv->bank, reg16 | (PM_OUTPUT << (offset * 2)),
					PM(priv->bank));
	else
		rzt2h_gpio_writew(priv, priv->bank, reg16 | (PM_INPUT << (offset * 2)),
					PM(priv->bank));
}

static int rzt2h_gpio_direction_input(struct udevice *dev, unsigned int offset)
{
	struct rzt2h_gpio_priv *priv = dev_get_priv(dev);

	rzt2h_gpio_set_direction(priv, offset, false);

	return 0;
}

static int rzt2h_gpio_direction_output(struct udevice *dev, unsigned int offset,
				       int value)
{
	struct rzt2h_gpio_priv *priv = dev_get_priv(dev);

	/* write GPIO value to output before selecting output mode of pin */
	rzt2h_gpio_set_value(dev, offset, value);
	rzt2h_gpio_set_direction(priv, offset, true);

	return 0;
}

static int rzt2h_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	struct rzt2h_gpio_priv *priv = dev_get_priv(dev);
	const u8 bit = BIT(offset);

	if (!(rzt2h_gpio_readb(priv, priv->bank, PMC(priv->bank)) & bit)) {
		u16 reg16;

		reg16 = rzt2h_gpio_readw(priv, priv->bank, PM(priv->bank));
		reg16 = (reg16 >> offset * 2) & PM_MASK;
		if (reg16 == PM_OUTPUT || reg16 == PM_OUTPUT_INPUT)
			return GPIOF_OUTPUT;
		else if (reg16 == PM_INPUT)
			return GPIOF_INPUT;
		else
			return GPIOF_UNUSED;
	}

	return GPIOF_FUNC;
}

static const struct dm_gpio_ops rzt2h_gpio_ops = {
	.direction_input	= rzt2h_gpio_direction_input,
	.direction_output	= rzt2h_gpio_direction_output,
	.get_value		= rzt2h_gpio_get_value,
	.set_value		= rzt2h_gpio_set_value,
	.get_function		= rzt2h_gpio_get_function,
};

static int rzt2h_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct rzt2h_gpio_priv *priv = dev_get_priv(dev);
	int ret;
	struct ofnode_phandle_args args;

	priv->regs = dev_remap_addr_name(dev_get_parent(dev), "port13_35");
	if (!priv->regs) {
		dev_err(dev, "can't get address\n");
		return -EINVAL;
	}

	priv->regs1 = dev_remap_addr_name(dev_get_parent(dev), "safety");
	if (!priv->regs1) {
		dev_err(dev, "can't get address\n");
		return -EINVAL;
	}


	uc_priv->bank_name = dev->name;

	ret = ofnode_parse_phandle_with_args(dev_ofnode(dev), "gpio-ranges",
					     NULL, 3, 0, &args);
	priv->bank = ret == 0 ? (args.args[1] / RZT2H_MAX_GPIO_PER_BANK) : -1;
	uc_priv->gpio_count = ret == 0 ? args.args[2] : RZT2H_MAX_GPIO_PER_BANK;

	return 0;
}

U_BOOT_DRIVER(rzt2h_gpio) = {
	.name   	= "rzt2h-gpio",
	.id     	= UCLASS_GPIO,
	.ops    	= &rzt2h_gpio_ops,
	.priv_auto	= sizeof(struct rzt2h_gpio_priv),
	.probe  	= rzt2h_gpio_probe,
};
