/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Defines macros and constants for Renesas RZ/T2H pin controller pin
 * muxing functions.
 */
#ifndef __DT_BINDINGS_RZT2H_PINCTRL_H
#define __DT_BINDINGS_RZT2H_PINCTRL_H

#define RZT2H_PINS_PER_PORT	8

/*
 * Store the pin index from its port and position number in bits[11-0].
 * And store its peripheral function mode identifier in 3 bits [14-12]
 */
#define RZT2H_PINMUX(port, pos, func)	\
	(((port) * RZT2H_PINS_PER_PORT + (pos)) | ((func) << 12))

#endif /* __DT_BINDINGS_RZT2H_PINCTRL_H */
