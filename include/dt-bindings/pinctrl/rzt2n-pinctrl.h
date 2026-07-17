/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Defines macros and constants for Renesas RZ/T2N pin controller pin
 * muxing functions.
 */
#ifndef __DT_BINDINGS_RZT2N_PINCTRL_H
#define __DT_BINDINGS_RZT2N_PINCTRL_H

#define RZT2N_PINS_PER_PORT	8

/*
 * Create the pin index from its bank and position numbers and store in
 * the upper 16 bits the alternate function identifier
 */
#define RZT2_PORT_PINMUX(b, p, f)	((b) * RZT2N_PINS_PER_PORT + (p) | ((f) << 16))

/* Convert a port and pin label to its global pin index */
 #define RZT2_GPIO(port, pin)	((port) * RZT2N_PINS_PER_PORT + (pin))

#endif /* __DT_BINDINGS_RZT2N_PINCTRL_H */
