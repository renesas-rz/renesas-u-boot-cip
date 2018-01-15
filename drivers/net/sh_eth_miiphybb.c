/*
 * sh_eth_miiphybb.c - MII bit-bang driver for Renesas ethernet controler.
 *
 * Copyright (C) 2015  Renesas Electronics Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>
#include <miiphy.h>

#include "sh_eth_miiphybb.h"

struct bb_miiphy_bus bb_miiphy_buses[] = {
#ifdef CONFIG_SH_ETHER
	{
		.name		= "sh_eth",
		.init		= sh_eth_bb_init,
		.mdio_active	= sh_eth_bb_mdio_active,
		.mdio_tristate	= sh_eth_bb_mdio_tristate,
		.set_mdio	= sh_eth_bb_set_mdio,
		.get_mdio	= sh_eth_bb_get_mdio,
		.set_mdc	= sh_eth_bb_set_mdc,
		.delay		= sh_eth_bb_delay,
	},
#endif
#ifdef CONFIG_RAVB
	{
		.name		= "ravb",
		.init		= ravb_bb_init,
		.mdio_active	= ravb_bb_mdio_active,
		.mdio_tristate	= ravb_bb_mdio_tristate,
		.set_mdio	= ravb_bb_set_mdio,
		.get_mdio	= ravb_bb_get_mdio,
		.set_mdc	= ravb_bb_set_mdc,
		.delay		= ravb_bb_delay,
	},
#endif
};
int bb_miiphy_buses_num = ARRAY_SIZE(bb_miiphy_buses);
