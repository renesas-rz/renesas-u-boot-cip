/*
 * drivers/spi/sh_qspi.h
 *     SH QSPI driver definition
 *
 * Copyright (C) 2013 Renesas Electronics Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __SH_QSPI_H__
#define __SH_QSPI_H__

#include <spi.h>

struct sh_qspi_regs {
	unsigned char spcr;
	unsigned char sslp;
	unsigned char sppcr;
	unsigned char spsr;
	unsigned long spdr;
	unsigned char spscr;
	unsigned char spssr;
	unsigned char spbr;
	unsigned char spdcr;
	unsigned char spckd;
	unsigned char sslnd;
	unsigned char spnd;
	unsigned char dummy0;
	unsigned short spcmd0;
	unsigned short spcmd1;
	unsigned short spcmd2;
	unsigned short spcmd3;
	unsigned char spbfcr;
	unsigned char dummy1;
	unsigned short spbdcr;
	unsigned long spbmul0;
	unsigned long spbmul1;
	unsigned long spbmul2;
	unsigned long spbmul3;
};

/* SPSR */
#define SH_QSPI_SPRFF	0x80
#define SH_QSPI_SPTEF	0x20

struct sh_qspi {
	struct spi_slave	slave;
	struct sh_qspi_regs	*regs;
/*	unsigned char cmd;*/
};

static inline struct sh_qspi *to_sh_qspi(struct spi_slave *slave)
{
	return container_of(slave, struct sh_qspi, slave);
}

/* SPI COMMAND */
/*#define CMD_READ_ARRAY_QUAD		0x6b*/

#endif
