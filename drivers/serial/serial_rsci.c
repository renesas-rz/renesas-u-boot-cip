// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <errno.h>
#include <clk.h>
#include <dm.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <serial.h>
#include <linux/compiler.h>
#include <dm/platform_data/serial_sh.h>
#include <linux/delay.h>
#include "serial_rsci.h"

DECLARE_GLOBAL_DATA_PTR;

static int scif_rxfill(struct uart_port *port)
{
	unsigned int rxfill_val = (sci_in(port, FRSR) & FRSR_RFDC_MASK) >> FRSR_RFDC_SHIFT;

	return rxfill_val;
}

static void sh_serial_init_generic(struct uart_port *port)
{
	sci_out(port, CCR3, (CCR3_INIT | CCR3_FM));
	sci_out(port, CCR3, (CCR3_INIT | CCR3_FM));

	sci_out(port, CCR2, CCR2_INIT_BRR);
	sci_out(port, CCR2, CCR2_INIT_BRR);

	sci_out(port, CCR1, 0);
	sci_out(port, CCR1, 0);

	sci_out(port, RSCFCR, (sci_in(port, RSCFCR) | (FCR_RFRST | FCR_TFRST)));
	sci_in(port, RSCFCR);
	sci_out(port, RSCFCR, FCR_INIT);

	sci_out(port, CCR0, CCR0_INIT);
	sci_out(port, CCR0, CCR0_INIT);
}

static void
sh_serial_setbrg_generic(struct uart_port *port, int clk, int baudrate)
{
	sci_out(port, CCR2, CCR2_INIT_BRR);
}

static void handle_error(struct uart_port *port)
{
	sci_out(port, CFCLR, (CFCLR_RDRFC | CFCLR_TDREC | CFCLR_ORERC | CFCLR_FERC | CFCLR_PERC));
	sci_out(port, FFCLR, FFCLR_DRC);
}

static int serial_raw_putc(struct uart_port *port, const char c)
{
	if (!(sci_in(port, RSCxSR) & CSR_TEND))
		return -EAGAIN;

	sci_out(port, RSCxTDR, c);
	sci_out(port, CFCLR, CFCLR_TDREC);

	return 0;
}

static int serial_rx_fifo_level(struct uart_port *port)
{
	return scif_rxfill(port);
}

static int sh_serial_tstc_generic(struct uart_port *port)
{
	unsigned int errors = 0;

	errors = sci_in(port, RSCxSR) & CSR_ERRORS;

	if (errors) {
		handle_error(port);
		return 0;
	}

	return serial_rx_fifo_level(port) ? 1 : 0;
}

static int serial_getc_check(struct uart_port *port)
{
	unsigned int status;

	status = sci_in(port, RSCxSR);
	if (status & CSR_ERRORS)
		handle_error(port);

	if (status & CSR_ORER)
		handle_error(port);

	status &= CSR_RDRF;
	if (status)
		return status;

	status = sci_in(port, FRSR);
	status &= FRSR_DR;
	if (status)
		return status;

	return scif_rxfill(port);
}

static int sh_serial_getc_generic(struct uart_port *port)
{
	unsigned int status;
	char ch;

	if (!serial_getc_check(port))
		return -EAGAIN;

	ch = sci_in(port, RSCxRDR);
	status = sci_in(port, RSCxSR);

	sci_out(port, CFCLR, CFCLR_RDRFC);

	if (status & (CSR_ERRORS | CSR_ORER))
		handle_error(port);

	return ch;
}

#if CONFIG_IS_ENABLED(DM_SERIAL)

static int sh_serial_pending(struct udevice *dev, bool input)
{
	struct uart_port *priv = dev_get_priv(dev);

	return sh_serial_tstc_generic(priv);
}

static int sh_serial_putc(struct udevice *dev, const char ch)
{
	struct uart_port *priv = dev_get_priv(dev);

	return serial_raw_putc(priv, ch);
}

static int sh_serial_getc(struct udevice *dev)
{
	struct uart_port *priv = dev_get_priv(dev);

	return sh_serial_getc_generic(priv);
}

static int sh_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct sh_serial_plat *plat = dev_get_plat(dev);
	struct uart_port *priv = dev_get_priv(dev);

	sh_serial_setbrg_generic(priv, plat->clk, baudrate);

	return 0;
}

static int sh_serial_probe(struct udevice *dev)
{
	struct sh_serial_plat *plat = dev_get_plat(dev);
	struct uart_port *priv = dev_get_priv(dev);

	priv->membase	= (unsigned char *)plat->base;
	priv->mapbase	= plat->base;
	priv->type	= plat->type;
	priv->clk_mode	= plat->clk_mode;

	sh_serial_init_generic(priv);

	return 0;
}

static const struct dm_serial_ops sh_serial_ops = {
	.putc = sh_serial_putc,
	.pending = sh_serial_pending,
	.getc = sh_serial_getc,
	.setbrg = sh_serial_setbrg,
};

#if CONFIG_IS_ENABLED(OF_CONTROL)
static const struct udevice_id sh_serial_id[] ={
	{.compatible = "renesas,rsci"},
	{}
};

static int sh_serial_of_to_plat(struct udevice *dev)
{
	struct sh_serial_plat *plat = dev_get_plat(dev);
	struct clk sh_serial_clk;
	fdt_addr_t addr;
	int ret;

	addr = dev_read_addr(dev);
	if (!addr)
		return -EINVAL;

	plat->base = addr;

	ret = clk_get_by_name(dev, "fck", &sh_serial_clk);
	if (!ret) {
		ret = clk_enable(&sh_serial_clk);
		if (!ret)
			plat->clk = clk_get_rate(&sh_serial_clk);
	} else {
		plat->clk = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					   "clock", 1);
	}

	plat->type = dev_get_driver_data(dev);
	return 0;
}
#endif

U_BOOT_DRIVER(serial_sh) = {
	.name	= "serial_sh",
	.id	= UCLASS_SERIAL,
	.of_match = of_match_ptr(sh_serial_id),
	.of_to_plat = of_match_ptr(sh_serial_of_to_plat),
	.plat_auto	= sizeof(struct sh_serial_plat),
	.probe	= sh_serial_probe,
	.ops	= &sh_serial_ops,
#if !CONFIG_IS_ENABLED(OF_CONTROL)
	.flags	= DM_FLAG_PRE_RELOC,
#endif
	.priv_auto	= sizeof(struct uart_port),
};

#else /* CONFIG_DM_SERIAL */

static struct uart_port sh_sci = {
	.membase	= (unsigned char *)RSCI1_BASE,
	.mapbase	= RSCI1_BASE,
};

static void sh_serial_setbrg(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	struct uart_port *port = &sh_sci;

	sh_serial_setbrg_generic(port, CONFIG_SH_SCIF_CLK_FREQ, gd->baudrate);
}

static int sh_serial_init(void)
{
	struct uart_port *port = &sh_sci;

	sh_serial_init_generic(port);
	serial_setbrg();

	return 0;
}

static void sh_serial_putc(const char c)
{
	struct uart_port *port = &sh_sci;

	if (c == '\n') {
		while (1) {
			if  (serial_raw_putc(port, '\r') != -EAGAIN)
				break;
		}
	}
	while (1) {
		if  (serial_raw_putc(port, c) != -EAGAIN)
			break;
	}
}

static int sh_serial_tstc(void)
{
	struct uart_port *port = &sh_sci;

	return sh_serial_tstc_generic(port);
}

static int sh_serial_getc(void)
{
	struct uart_port *port = &sh_sci;
	int ch;

	while (1) {
		ch = sh_serial_getc_generic(port);
		if (ch != -EAGAIN)
			break;
	}

	return ch;
}

static struct serial_device sh_serial_drv = {
	.name	= "sh_serial",
	.start	= sh_serial_init,
	.stop	= NULL,
	.setbrg	= sh_serial_setbrg,
	.putc	= sh_serial_putc,
	.puts	= default_serial_puts,
	.getc	= sh_serial_getc,
	.tstc	= sh_serial_tstc,
};

void sh_serial_initialize(void)
{
	serial_register(&sh_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &sh_serial_drv;
}
#endif /* CONFIG_DM_SERIAL */
