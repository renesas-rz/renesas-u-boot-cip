// SPDX-License-Identifier: (GPL-2.0-only OR BSD-2-Clause)
/*
 * Pin controller driver for Renesas RZ/T2N SoCs.
 *
 * Copyright (C) 2024 Renesas Electronics Corporation
 */

#include <common.h>
#include <dm.h>
#include <dm/lists.h>
#include <dm/pinctrl.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/err.h>
#include <fdt_support.h>
#include <dm/device_compat.h>

#define P(n)	(0x000 + 0x001 * (n))	/* Port Register */
#define PM(n)	(0x200 + 0x002 * (n))	/* Port Mode Register */
#define PMC(n)	(0x400 + 0x001 * (n))	/* Port Mode Control Register */
#define PFC(n)	(0x600 + 0x008 * (n))	/* Port Function Control Register */
#define PIN(n)	(0x800 + 0x001 * (n))	/* Port Input Register */

#define RSELPSR		0x1F04
#define RSELPSR_RS	BIT(0)
#define SLPSR		0x1F00
#define SLPSR_SL	0x0

#define RZT2N_MAX_PINS_PER_PORT		8

#define T2N_SAFETY_IO_PORTS_MAX     7

DECLARE_GLOBAL_DATA_PTR;

struct rzt2n_pinctrl_priv {
	void __iomem	*regs, *regs1;
};

void rzt2n_pinctrl_writeb(struct rzt2n_pinctrl_priv *priv, u8 port, u8 val, u16 offset)
{
	if (port > T2N_SAFETY_IO_PORTS_MAX) {
		void __iomem *PRCRN = ioremap(0x80294200, 0x100);
		iowrite32(0x0000A504, PRCRN + 0x0);
		writeb(val, priv->regs + offset);
		iowrite32(0x0000A500, PRCRN + 0x0);
		iounmap(PRCRN);
	} else
		writeb(val, priv->regs1 + offset);
}

void rzt2n_pinctrl_writew(struct rzt2n_pinctrl_priv *priv, u8 port, u16 val, u16 offset)
{
	if (port > T2N_SAFETY_IO_PORTS_MAX) {
		void __iomem *PRCRN = ioremap(0x80294200, 0x100);
		iowrite32(0x0000A504, PRCRN + 0x0);
		writew(val, priv->regs + offset);
		iowrite32(0x0000A500, PRCRN + 0x0);
		iounmap(PRCRN);
	} else
		writew(val, priv->regs1 + offset);
}

void rzt2n_pinctrl_writeq(struct rzt2n_pinctrl_priv *priv, u8 port, u64 val, u16 offset)
{
	if (port > T2N_SAFETY_IO_PORTS_MAX) {
		void __iomem *PRCRN = ioremap(0x80294200, 0x100);
		iowrite32(0x0000A504, PRCRN + 0x0);
		writeq(val, priv->regs + offset);
		iowrite32(0x0000A500, PRCRN + 0x0);
		iounmap(PRCRN);
	} else
		writeq(val, priv->regs1 + offset);
}

static u8 rzt2n_pinctrl_readb(struct rzt2n_pinctrl_priv *priv, u8 port, u16 offset)
{
	if (port <= T2N_SAFETY_IO_PORTS_MAX)
		return readb(priv->regs1 + offset);
	else
		return readb(priv->regs + offset);
};

static u64 rzt2n_pinctrl_readq(struct rzt2n_pinctrl_priv *priv, u8 port, u16 offset)
{
	if (port <= T2N_SAFETY_IO_PORTS_MAX)
		return readq(priv->regs1 + offset);
	else
		return readq(priv->regs + offset);
};

static void rzt2n_pinctrl_set_function(struct rzt2n_pinctrl_priv *priv,
				       u16 port, u8 pin, u8 func)
{
	u32 reg64;
	u8 reg8;

	/* Set GPIO or Func in PMC, then set Func in PFC */
	reg8 = rzt2n_pinctrl_readb(priv, port, PMC(port));
	reg8 = (reg8 & ~(1 << pin)) | BIT(pin);
	rzt2n_pinctrl_writeb(priv, port, reg8, PMC(port));

	reg64 = rzt2n_pinctrl_readq(priv, port, PFC(port));
	reg64 = (reg64 & ~(0x3F << (pin * 8))) | (func << (pin * 8));
	rzt2n_pinctrl_writeq(priv, port, reg64, PFC(port));

}

static int rzt2n_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	struct rzt2n_pinctrl_priv *priv = dev_get_plat(dev);
	u16 port;
	u16 port_max = (u16)dev_get_driver_data(dev);
	u8 pin, func;
	int i, count;
	const u32 *data;
	u32 cells[port_max * RZT2N_MAX_PINS_PER_PORT];

	data = dev_read_prop(config, "pinmux", &count);
	if (count < 0) {
		debug("%s: bad array size %d\n", __func__, count);
		return -EINVAL;
	}

	count /= sizeof(u32);
	if (count > port_max * RZT2N_MAX_PINS_PER_PORT) {
		debug("%s: unsupported pins array count %d\n",
		      __func__, count);
		return -EINVAL;
	}

	for (i = 0 ; i < count; i++) {
		cells[i] = fdt32_to_cpu(data[i]);
		func = (cells[i] >> 12) & 0xf;
		port = (cells[i] / RZT2N_MAX_PINS_PER_PORT) & 0x1ff;
		pin = cells[i] % RZT2N_MAX_PINS_PER_PORT;
		if (func > 64 || port >= port_max || pin >= RZT2N_MAX_PINS_PER_PORT) {
			printf("Invalid cell %i in node %s!\n",
			       count, ofnode_get_name(dev_ofnode(config)));
			continue;
		}

		rzt2n_pinctrl_set_function(priv, port, pin, func);
	}

	return 0;
}

const struct pinctrl_ops rzt2n_pinctrl_ops  = {
	.set_state = rzt2n_pinctrl_set_state,
};

static int rzt2n_pinctrl_probe(struct udevice *dev)
{
	struct rzt2n_pinctrl_priv *priv = dev_get_plat(dev);
	ofnode pin_node;
	struct fdt_resource res, res1;
	void *fdt = (void *)gd->fdt_blob;
	int node = dev_of_offset(dev);
	int ret;

	ret = fdt_get_named_resource(fdt, node, "reg", "reg-names",
					"port08_25", &res);
	if (ret < 0) {
		printf("pinctrl: resource port13_35 not found\n");
		return ret;
	}
	priv->regs = ioremap(res.start, fdt_resource_size(&res));

	ret = fdt_get_named_resource(fdt, node, "reg", "reg-names",
					"safety", &res1);
	if (ret < 0) {
		printf("pinctrl: resource safety not found\n");
		return ret;
	}
	priv->regs1 = ioremap(res1.start, fdt_resource_size(&res1));

	dev_for_each_subnode(pin_node, dev) {
		struct udevice *gpiodev;

		if (!ofnode_read_bool(pin_node, "gpio-controller"))
			continue;

		device_bind_driver_to_node(dev, "rzt2n-gpio",
					   ofnode_get_name(pin_node),
					   pin_node, &gpiodev);
	}

	return 0;
}

static const struct udevice_id rzt2n_pinctrl_match[] = {
	{ .compatible = "renesas,r9a07g076-pinctrl", .data = 26 },
	{}
};

U_BOOT_DRIVER(rzt2n_pinctrl) = {
	.name		= "rzt2n_pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= rzt2n_pinctrl_match,
	.probe		= rzt2n_pinctrl_probe,
	.plat_auto	= sizeof(struct rzt2n_pinctrl_priv),
	.ops		= &rzt2n_pinctrl_ops,
};
