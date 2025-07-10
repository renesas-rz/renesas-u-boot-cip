// SPDX-License-Identifier: GPL-2.0+
/*
 * Pin controller driver for Renesas RZ/T2H SoCs.
 *
 * Copyright (C) 2020 Renesas Electronics Corporation
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

#define PRCRN			0x80294200
#define PRCRN_PRKEY		(0xa5 << 8)
#define PRCRN_WR_EN		0xF
#define PRCRS                   0x81296000
#define PRCRS_PRKEY             (0xa5 << 8)
#define PRCRS_WR_EN             0xF

#define RZT2H_MAX_PINS_PER_PORT		8


DECLARE_GLOBAL_DATA_PTR;

struct rzt2h_pinctrl_priv {
	void __iomem	*regs, *regs1;
};

void rzt2h_pinctrl_writeb(struct rzt2h_pinctrl_priv *priv, u8 port, u8 val, u16 offset)
{
	if (port > 12) {
		/* Disable Write protect to enable writing */
		*(volatile u32 *)PRCRN = PRCRN_PRKEY | PRCRN_WR_EN;
		*(volatile u32 *)PRCRS = PRCRS_PRKEY | PRCRS_WR_EN;
		debug("write port %d addr %p value %x \n", port,priv->regs + offset,val);
		writeb(val, priv->regs + offset);
		/* Enable Write protect to disable writing */
		*(volatile u32 *)PRCRN = PRCRN_PRKEY;
		*(volatile u32 *)PRCRS = PRCRS_PRKEY;
	} else
	{
		debug("write port %d addr %p value %d \n", port,priv->regs1 + offset,val);
		writeb(val, priv->regs1 + offset);
	}
}

void rzt2h_pinctrl_writew(struct rzt2h_pinctrl_priv *priv, u8 port, u16 val, u16 offset)
{
	if (port > 12) {
		/* Disable Write protect to enable writing */
		*(volatile u32 *)PRCRN = PRCRN_PRKEY | PRCRN_WR_EN;
		*(volatile u32 *)PRCRS = PRCRS_PRKEY | PRCRS_WR_EN;
		debug("write port %d addr %p value %x \n", port,priv->regs + offset,val);
		writew(val, priv->regs + offset);
		/* Enable Write protect to disable writing */
		*(volatile u32 *)PRCRN = PRCRN_PRKEY;
		*(volatile u32 *)PRCRS = PRCRS_PRKEY;
	} else
	{
		debug("write port %d addr %p value %x \n", port,priv->regs1 + offset,val);
		writew(val, priv->regs1 + offset);
	}
}

void rzt2h_pinctrl_writeq(struct rzt2h_pinctrl_priv *priv, u8 port, u64 val, u16 offset)
{
	if (port > 12) {
		/* Disable Write protect to enable writing */
		*(volatile u32 *)PRCRN = PRCRN_PRKEY | PRCRN_WR_EN;
		*(volatile u32 *)PRCRS = PRCRS_PRKEY | PRCRS_WR_EN;
		debug("write port %d addr %p value %llx \n", port,priv->regs + offset,val);
		writeq(val, priv->regs + offset);
		/* Enable Write protect to disable writing */
		*(volatile u32 *)PRCRN = PRCRN_PRKEY;
		*(volatile u32 *)PRCRS = PRCRS_PRKEY;
	} else
	{
		debug("write port %d addr %p value %llx \n", port,priv->regs1 + offset,val);
		writeq(val, priv->regs1 + offset);
	}
}

static u8 rzt2h_pinctrl_readb(struct rzt2h_pinctrl_priv *priv, u8 port, u16 offset)
{
	u8 rv;
	if (port < 13)
	{
		rv = readb(priv->regs1 + offset);
		debug("read port %d addr %p value %x \n", port,priv->regs1 + offset,rv);
	}
	else
	{
		rv =readb(priv->regs + offset);
		debug("read port %d addr %p value %x \n", port,priv->regs + offset,rv);
	}
	return rv;
};

static u64 rzt2h_pinctrl_readq(struct rzt2h_pinctrl_priv *priv, u8 port, u16 offset)
{
	u64 rv;
	if (port < 13)
	{
		rv = readq(priv->regs1 + offset);
		debug("read port %d addr %p value %llx \n", port,priv->regs1 + offset,rv);
	}
	else
	{
		rv = readq(priv->regs + offset);
		debug("read port %d addr %p value %llx \n", port,priv->regs1 + offset,rv);
	}
	return rv;
};

static void rzt2h_pinctrl_set_function(struct rzt2h_pinctrl_priv *priv,
				       u16 port, u8 pin, u8 func)
{
	u64 reg64;
	u8 reg8;

	/* Set GPIO or Func in PMC, then set Func in PFC */
	reg8 = rzt2h_pinctrl_readb(priv, port, PMC(port));
	reg8 = (reg8 & ~(1 << pin)) | BIT(pin);
	rzt2h_pinctrl_writeb(priv, port, reg8, PMC(port));

	reg64 = rzt2h_pinctrl_readq(priv, port, PFC(port));
	reg64 = (reg64 & ~(0x3F << (pin * 8))) | (func << (pin * 8));
	rzt2h_pinctrl_writeq(priv, port, reg64, PFC(port));

}

static int rzt2h_node_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	struct rzt2h_pinctrl_priv *priv = dev_get_plat(dev);
	u16 port;
	u16 port_max = (u16)dev_get_driver_data(dev);
	u8 pin, func;
	int i, count;
	const u32 *data;
	u32 cells[port_max * RZT2H_MAX_PINS_PER_PORT];

	data = dev_read_prop(config, "pinmux", &count);
	if (count < 0) {
		debug("%s: bad array size %d\n", __func__, count);
		return -EINVAL;
	}

	count /= sizeof(u32);
	if (count > port_max * RZT2H_MAX_PINS_PER_PORT) {
		debug("%s: unsupported pins array count %d\n",
		      __func__, count);
		return -EINVAL;
	}

	for (i = 0 ; i < count; i++) {
		cells[i] = fdt32_to_cpu(data[i]);
		func = (cells[i] >> 12) & 0x3f;
		port = (cells[i] / RZT2H_MAX_PINS_PER_PORT) & 0x1ff;
		pin = cells[i] % RZT2H_MAX_PINS_PER_PORT;
		if (func > 64 || port >= port_max || pin >= RZT2H_MAX_PINS_PER_PORT) {
			debug("Invalid cell %i in node %s!\n",
			       count, ofnode_get_name(dev_ofnode(config)));
			continue;
		}

		rzt2h_pinctrl_set_function(priv, port, pin, func);
	}

	return 0;
}
static int rzt2h_subnode_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	struct rzt2h_pinctrl_priv *priv = dev_get_plat(dev);
	u16 port;
	u16 port_max = (u16)dev_get_driver_data(dev);
	u8 pin, func;
	int i, count,rv;
	u32 cells[port_max * RZT2H_MAX_PINS_PER_PORT];
	ofnode node = dev_ofnode(config);
	ofnode subnode;
	ofnode_for_each_subnode(subnode, node) {
		count = ofnode_read_size(subnode, "pinmux");
		if (count < 0)
		{
			printf("ofnode_read_size return size %d\n", count);
			return count;
		}
		count = count / sizeof(cells[0]);
		debug("No of pinmux entries= %d\n", count);

		if (count > port_max * RZT2H_MAX_PINS_PER_PORT) {
			debug("%s: unsupported pins array count %d\n",
		      __func__, count);
		return -EINVAL;
		}
		rv = ofnode_read_u32_array(subnode, "pinmux", cells, count);
		if (rv < 0)
		{
			printf("ofnode_read_u32_array return size %d\n", rv);
			return rv;
		}
		for (i = 0 ; i < count; i++) {
			func = (cells[i] >> 12) & 0x3f;
			port = (cells[i] / RZT2H_MAX_PINS_PER_PORT) & 0x1ff;
			pin = cells[i] % RZT2H_MAX_PINS_PER_PORT;
			debug("subnode func %x port %d pin %d \n",func,port,pin);
			if (func > 64 || port >= port_max || pin >= RZT2H_MAX_PINS_PER_PORT) {
				printf("Invalid cell %i in node %s!\n",
			       count, ofnode_get_name(dev_ofnode(config)));
				continue;
			}
			rzt2h_pinctrl_set_function(priv, port, pin, func);
	}

	}
	debug("set state subnode complete \n");
	return 0;
}

static int rzt2h_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	int rv,rv1;
	rv = rzt2h_node_pinctrl_set_state(dev,config);
	rv1 = rzt2h_subnode_pinctrl_set_state(dev,config);
	return (rv | rv1);
}
const struct pinctrl_ops rzt2h_pinctrl_ops  = {
	.set_state = rzt2h_pinctrl_set_state,
};

static int rzt2h_pinctrl_probe(struct udevice *dev)
{
	struct rzt2h_pinctrl_priv *priv = dev_get_plat(dev);
	ofnode pin_node;
	struct fdt_resource res, res1;
	void *fdt = (void *)gd->fdt_blob;
	int node = dev_of_offset(dev);
	int ret;

	ret = fdt_get_named_resource(fdt, node, "reg", "reg-names",
					"port13_35", &res);
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

		device_bind_driver_to_node(dev, "rzt2h-gpio",
					   ofnode_get_name(pin_node),
					   pin_node, &gpiodev);
	}

	return 0;
}

static const struct udevice_id rzt2h_pinctrl_match[] = {
	{ .compatible = "renesas,r9a09g077-pinctrl", .data = 36 },
	{ .compatible = "renesas,r9a09g087-pinctrl", .data = 36 },
	{}
};

U_BOOT_DRIVER(rzt2h_pinctrl) = {
	.name		= "rzt2h_pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= rzt2h_pinctrl_match,
	.probe		= rzt2h_pinctrl_probe,
	.plat_auto	= sizeof(struct rzt2h_pinctrl_priv),
	.ops		= &rzt2h_pinctrl_ops,
};
