// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 Schneider Electric
 *
 * Long Luu <long.luu.ur@renesas.com>
 */

#include <common.h>
#include <asm/io.h>
#include <miiphy.h>
#include <netdev.h>
#include <linux/bitfield.h>

#define ETH_SUBSYSTEM_REG_BASE		0x80110000

#define ETHSS_PRCMD			0x0

#define ETHSS_MODCTRL			0x8
#define ETHSS_MODCTRL_SW_MODE		GENMASK(2, 0)

#define ETHSS_CONVCTRL(port)		(0x100 + (port) * 4)

#define ETHSS_CONVCTRL_CONV_SPEED	GENMASK(1, 0)
#define CONV_MODE_10MBPS		0
#define CONV_MODE_100MBPS		1
#define CONV_MODE_1000MBPS		2

#define ETHSS_CONVCTRL_CONV_MODE		GENMASK(3, 2)
#define CONV_MODE_MII			0
#define CONV_MODE_RMII			1
#define CONV_MODE_RGMII			2

#define ETHSS_CONVCTRL_FULLD		BIT(8)
#define ETHSS_CONVCTRL_RGMII_LINK	BIT(12)
#define ETHSS_CONVCTRL_RGMII_DUPLEX	BIT(13)
#define ETHSS_CONVCTRL_RGMII_SPEED	GENMASK(15, 14)

#define ETHSS_CONVRST			0x114
#define ETHSS_CONVRST_PHYIF_RST(port)	BIT(port)
#define ETHSS_CONVRST_PHYIF_RST_MASK	GENMASK(3, 0)

#define ETHSS_SWCTRL			0x304
#define ETHSS_SWDUPC			0x308

#define ETHSS_MAX_NR_PORTS		4

static void ethss_reg_writel(int offset, u32 value)
{
	writel(value, ETH_SUBSYSTEM_REG_BASE + offset);
}

static u32 ethss_reg_readl(int offset)
{
	return readl(ETH_SUBSYSTEM_REG_BASE + offset);
}

static void ethss_reg_rmw(int offset, u32 mask, u32 val)
{
	u32 reg;

	reg = ethss_reg_readl(offset);
	reg &= ~mask;
	reg |= val;
	ethss_reg_writel(offset, reg);
}

static void ethss_converter_enable(int port, int enable)
{
	u32 val = 0;

	if (enable)
		val = ETHSS_CONVRST_PHYIF_RST(port);

	ethss_reg_rmw(ETHSS_CONVRST, ETHSS_CONVRST_PHYIF_RST(port), val);
}

int ethss_config(int port, phy_interface_t interface)
{
	u32 speed, conv_mode, val, mask;

	switch (interface) {
	case PHY_INTERFACE_MODE_RMII:
		conv_mode = CONV_MODE_RMII;
		speed = CONV_MODE_100MBPS;
		break;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
		conv_mode = CONV_MODE_RGMII;
		speed = CONV_MODE_1000MBPS;
		break;
	case PHY_INTERFACE_MODE_MII:
		conv_mode = CONV_MODE_MII;
		/* When in MII mode, speed should be set to 0 (which is actually
		 * CONV_MODE_10MBPS)
		 */
		speed = CONV_MODE_10MBPS;
		break;
	default:
		return -EOPNOTSUPP;
	}

	val = FIELD_PREP(ETHSS_CONVCTRL_CONV_MODE, conv_mode);
	mask = ETHSS_CONVCTRL_CONV_MODE;

	/* Update speed only if we are going to change the interface because
	 * the link might already be up and it would break it if the speed is
	 * changed.
	 */
	val |= FIELD_PREP(ETHSS_CONVCTRL_CONV_SPEED, speed);
	mask |= ETHSS_CONVCTRL_CONV_SPEED;

	ethss_reg_rmw(ETHSS_CONVCTRL(port), mask, val);
	ethss_converter_enable(port, 1);

	return 0;
}

void ethss_link_up(int port, phy_interface_t interface,
		   int speed, int duplex)
{
	u32 conv_speed = 0, val = 0;

	if (duplex == DUPLEX_FULL)
		val |= ETHSS_CONVCTRL_FULLD;

	/* No speed in MII through-mode */
	if (interface != PHY_INTERFACE_MODE_MII) {
		switch (speed) {
		case SPEED_1000:
			conv_speed = CONV_MODE_1000MBPS;
			break;
		case SPEED_100:
			conv_speed = CONV_MODE_100MBPS;
			break;
		case SPEED_10:
			conv_speed = CONV_MODE_10MBPS;
			break;
		default:
			return;
		}
	}

	val |= FIELD_PREP(ETHSS_CONVCTRL_CONV_SPEED, conv_speed);

	ethss_reg_rmw(ETHSS_CONVCTRL(port),
		      (ETHSS_CONVCTRL_CONV_SPEED | ETHSS_CONVCTRL_FULLD), val);
}

void ethss_init_hw(u32 cfg_mode)
{
	int port;

	/* Unlock write access to accessory registers (cf datasheet). If this
	 * is going to be used in conjunction with the Cortex-M3, this sequence
	 * will have to be moved in register write
	 */
	ethss_reg_writel(ETHSS_PRCMD, 0x00A5);
	ethss_reg_writel(ETHSS_PRCMD, 0x0001);
	ethss_reg_writel(ETHSS_PRCMD, 0xFFFE);
	ethss_reg_writel(ETHSS_PRCMD, 0x0001);

	ethss_reg_writel(ETHSS_MODCTRL,
			 FIELD_PREP(ETHSS_MODCTRL_SW_MODE, cfg_mode));

	for (port = 0; port < ETHSS_MAX_NR_PORTS; port++) {
		ethss_converter_enable(port, 0);
		/* Disable speed/duplex control from these registers, datasheet
		 * says switch registers should be used to setup switch port
		 * speed and duplex.
		 */
		ethss_reg_writel(ETHSS_SWCTRL, 0x0);
		ethss_reg_writel(ETHSS_SWDUPC, 0x0);
	}
}
