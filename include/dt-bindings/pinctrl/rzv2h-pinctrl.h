/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Defines macros and constants for Renesas RZ/V2H pin controller pin
 * muxing functions.
 */
#ifndef __DT_BINDINGS_RZV2H_PINCTRL_H
#define __DT_BINDINGS_RZV2H_PINCTRL_H

#define RZV2H_PINS_PER_PORT	8

#define PORT_0		0
#define PORT_1		1
#define PORT_2		2
#define PORT_3		3
#define PORT_4		4
#define PORT_5		5
#define PORT_6		6
#define PORT_7		7
#define PORT_8		8
#define PORT_9		9
#define PORT_A		10
#define PORT_B		11
#define PORT_C		12
#define PORT_D		13
#define PORT_E		14
#define PORT_F		15
#define PORT_G		16
#define PORT_H		17
#define PORT_I		18
#define PORT_J		19
#define PORT_K		20
#define PORT_L		21
#define PORT_M		22
#define PORT_N		23
#define PORT_O		24
#define PORT_P		25
#define PORT_Q		26
#define PORT_R		27
#define PORT_S		28
#define PORT_T		29
#define PORT_U		30
#define PORT_V		31
#define PORT_W		32
#define PORT_X		33
#define PORT_Y		34
#define PORT_Z		35

/*
 * Store the pin index from its port and position number in bits[11-0].
 * And store its peripheral function mode identifier in 3 bits [14-12]
 */
#define RZV2H_PINMUX(port, pos, func)	\
	((PORT_##port * RZV2H_PINS_PER_PORT + (pos)) | ((func) << 12))

#endif /* __DT_BINDINGS_RZV2H_PINCTRL_H */
