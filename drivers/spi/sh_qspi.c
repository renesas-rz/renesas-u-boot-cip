/*
 * drivers/spi/sh_qspi.c
 *     SH QSPI driver
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

#include <common.h>
#include <malloc.h>
#include <spi.h>
#include <asm/io.h>
#include "sh_qspi.h"

static void sh_qspi_writeb(unsigned char data, unsigned char *reg)
{
	writeb(data, reg);
}

static void sh_qspi_writew(unsigned short data, unsigned short *reg)
{
	writew(data, reg);
}

static void sh_qspi_writel(unsigned long data, unsigned long *reg)
{
	writel(data, reg);
}

static unsigned char sh_qspi_readb(unsigned char *reg)
{
	return readb(reg);
}

static unsigned short sh_qspi_readw(unsigned short *reg)
{
	return readw(reg);
}

static unsigned long sh_qspi_readl(unsigned long *reg)
{
	return readl(reg);
}

void spi_init(void)
{
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	struct sh_qspi *ss;

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	ss = malloc(sizeof(struct sh_qspi));
	if (!ss)
		return NULL;

	ss->slave.bus = bus;
	ss->slave.cs = cs;
	ss->regs = (struct sh_qspi_regs *)CONFIG_SH_QSPI_BASE;

	/* QSPI initialize */
	sh_qspi_writeb(0x08, &ss->regs->spcr);
	sh_qspi_writeb(0x00, &ss->regs->sslp);
	sh_qspi_writeb(0x06, &ss->regs->sppcr);
	sh_qspi_writeb(0x01, &ss->regs->spbr);
	sh_qspi_writeb(0x00, &ss->regs->spdcr);
	sh_qspi_writeb(0x00, &ss->regs->spckd);
	sh_qspi_writeb(0x00, &ss->regs->sslnd);
	sh_qspi_writeb(0x00, &ss->regs->spnd);
	sh_qspi_writew(0xe084, &ss->regs->spcmd0);
	sh_qspi_writew(0x8084, &ss->regs->spcmd0);
	sh_qspi_writeb(0xc0, &ss->regs->spbfcr);
	sh_qspi_writeb(0x00, &ss->regs->spbfcr);
	sh_qspi_writeb(0x00, &ss->regs->spscr);
	sh_qspi_writeb(0x48, &ss->regs->spcr);

	return &ss->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct sh_qspi *spi = to_sh_qspi(slave);

	free(spi);
}

int spi_claim_bus(struct spi_slave *slave)
{
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
}

static int sh_qspi_xfer(struct sh_qspi *ss, unsigned char *tdata,
			unsigned char *rdata, unsigned long flags)
{

	while (!(sh_qspi_readb(&ss->regs->spsr) & SH_QSPI_SPTEF)) {
		if (ctrlc())
			return 1;
		udelay(10);
	}

	sh_qspi_writeb(*tdata, (unsigned char *)(&ss->regs->spdr));

	while ((sh_qspi_readw(&ss->regs->spbdcr) != 0x01)) {
		if (ctrlc())
			return 1;
		{
			int i = 100;
			while (i--)
				;
		}
	}

	while (!(sh_qspi_readb(&ss->regs->spsr) & SH_QSPI_SPRFF)) {
		if (ctrlc())
			return 1;
		udelay(10);
	}

	*rdata = sh_qspi_readb((unsigned char *)(&ss->regs->spdr));

	return 0;
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
	     void *din, unsigned long flags)
{
	struct sh_qspi *ss = to_sh_qspi(slave);
	unsigned int nbyte;
	int ret = 0;
	unsigned char *tdata, *rdata, dtdata = 0, drdata;

	if (dout == NULL && din == NULL) {
		if (flags & SPI_XFER_END)
			sh_qspi_writeb(0x08, &ss->regs->spcr);
		return 0;
	}

	if (bitlen % 8) {
		printf("spi_xfer: bitlen is not 8bit alined %d", bitlen);
		return 1;
	}

	nbyte = bitlen / 8;

	if (flags & SPI_XFER_BEGIN) {
		sh_qspi_writeb(0x08, &ss->regs->spcr);

		sh_qspi_writew(0xe084, &ss->regs->spcmd0);

		if (flags & SPI_XFER_END)
			sh_qspi_writel(nbyte, &ss->regs->spbmul0);
		else
			sh_qspi_writel(0x100000, &ss->regs->spbmul0);

		sh_qspi_writeb(0xc0, &ss->regs->spbfcr);
		sh_qspi_writeb(0x00, &ss->regs->spbfcr);
		sh_qspi_writeb(0x00, &ss->regs->spscr);
		sh_qspi_writeb(0x48, &ss->regs->spcr);
	}

	if (dout != NULL)
		tdata = (unsigned char *)dout;
	else
		tdata = &dtdata;

	if (din != NULL)
		rdata = din;
	else
		rdata = &drdata;

	while (nbyte > 0) {
		ret = sh_qspi_xfer(ss, tdata, rdata, flags);
		if (ret)
			break;

		if (dout != NULL)
			tdata++;

		if (din != NULL)
			rdata++;

		nbyte--;
	}

	if (flags & SPI_XFER_END)
		sh_qspi_writeb(0x08, &ss->regs->spcr);

	return ret;
}

int  spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return 1;
}

void spi_cs_activate(struct spi_slave *slave)
{
}

void spi_cs_deactivate(struct spi_slave *slave)
{
}
