/*
 * sh_eth_miiphybb.h - MII bit-bang driver for Renesas ethernet controler.
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

#ifdef CONFIG_SH_ETHER
extern int sh_eth_bb_init(struct bb_miiphy_bus *bus);
extern int sh_eth_bb_mdio_active(struct bb_miiphy_bus *bus);
extern int sh_eth_bb_mdio_tristate(struct bb_miiphy_bus *bus);
extern int sh_eth_bb_set_mdio(struct bb_miiphy_bus *bus, int v);
extern int sh_eth_bb_get_mdio(struct bb_miiphy_bus *bus, int *v);
extern int sh_eth_bb_set_mdc(struct bb_miiphy_bus *bus, int v);
extern int sh_eth_bb_delay(struct bb_miiphy_bus *bus);
#endif
#ifdef CONFIG_RAVB
extern int ravb_bb_init(struct bb_miiphy_bus *bus);
extern int ravb_bb_mdio_active(struct bb_miiphy_bus *bus);
extern int ravb_bb_mdio_tristate(struct bb_miiphy_bus *bus);
extern int ravb_bb_set_mdio(struct bb_miiphy_bus *bus, int v);
extern int ravb_bb_get_mdio(struct bb_miiphy_bus *bus, int *v);
extern int ravb_bb_set_mdc(struct bb_miiphy_bus *bus, int v);
extern int ravb_bb_delay(struct bb_miiphy_bus *bus);
#endif
