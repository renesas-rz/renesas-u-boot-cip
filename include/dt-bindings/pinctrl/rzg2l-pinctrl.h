/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-2-Clause) */
/*
 * This header provides constants for Renesas RZ/G2L family pinctrl bindings.
 *
 * Copyright (C) 2021 Renesas Electronics Corp.
 *
 */

#ifndef __DT_BINDINGS_RZG2L_PINCTRL_H
#define __DT_BINDINGS_RZG2L_PINCTRL_H

#define RZG2L_PINS_PER_PORT	8

/*
 * Create the pin index from its bank and position numbers and store in
 * the upper 16 bits the alternate function identifier
 */
#if defined(CONFIG_PINCTRL_RZG2L)
    //For RZ/G2L, This is the format used to define a pin:
    //pfc-rzg2l.c defines how the format is parsed.
    //if other platforms use the pfc-rzg2l.c, then this format is used.
    #define RZG2L_PINMUX(port, pos, func)	\
	    (((port) * RZG2L_PINS_PER_PORT + (pos)) | ((func) << 12))
    #define RZG2L_PORT_PINMUX(port, pos, func)	\
	    (((port) * RZG2L_PINS_PER_PORT + (pos)) | ((func) << 16))
#else
    #define RZG2L_PINMUX(b, p, f)	((b) * RZG2L_PINS_PER_PORT + (p) | ((f) << 16))
#endif

/* Convert a port and pin label to its global pin index */
#define RZG2L_GPIO(port, pin)	((port) * RZG2L_PINS_PER_PORT + (pin))

#endif /* __DT_BINDINGS_RZG2L_PINCTRL_H */
