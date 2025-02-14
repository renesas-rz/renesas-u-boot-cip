/*
 * Copy and modify from linux/drivers/serial/sh-sci.h
 */

#include <dm/platform_data/serial_sh.h>
#include <linux/bitops.h>

struct uart_port {
	unsigned long	iobase;		/* in/out[bwl] */
	unsigned char	*membase;	/* read/write[bwl] */
	unsigned long	mapbase;	/* for ioremap */
	enum sh_serial_type type;	/* port type */
	enum sh_clk_mode clk_mode;	/* clock mode */
};

#define sci_in(port, reg) sci_##reg##_in(port)
#define sci_out(port, reg, value) sci_##reg##_out(port, value)

#define SCI_IN(offset)\
	return readl(port->membase + (offset))
#define SCI_OUT(offset, value)\
	writel(value, port->membase + (offset))

#define CPU_RSCI_FNS(name, sci_offset)\
	static inline unsigned int sci_##name##_in(struct uart_port *port) {\
		SCI_IN(sci_offset);\
	}\
	static inline void sci_##name##_out(struct uart_port *port,\
					unsigned int value) {\
		SCI_OUT(sci_offset, value);\
	}

#define RSCI_FNS(name, scif_offset) \
	CPU_RSCI_FNS(name, scif_offset)

/* RSCI registers */
RSCI_FNS(RSCxRDR, 0x00)
RSCI_FNS(RSCxTDR, 0x04)
RSCI_FNS(CCR0,	  0x08)
RSCI_FNS(CCR1,	  0x0C)
RSCI_FNS(CCR2,	  0x10)
RSCI_FNS(CCR3,	  0x14)
RSCI_FNS(CCR4,	  0x18)
RSCI_FNS(RSCFCR,  0x24)
RSCI_FNS(RSCxSR,  0x48)
RSCI_FNS(FRSR,	  0x50)
RSCI_FNS(CFCLR,	  0x68)
RSCI_FNS(FFCLR,	  0x70)

#define FRSR_RFDC_MASK			0x00003F00 		/* Receive FIFO Data Count */
#define FRSR_RFDC_SHIFT			8
#define FRSR_DR   			BIT(0)
#define FFCLR_DRC			BIT(0)

#define CCR0_INIT			(BIT(4)|BIT(0))  	/* TE=1, RE=1 */

#define CCR2_INIT_BRR			0xFF001904		/* Bit rate setting: 115200bps */

#define CCR3_INIT			0x00001283
#define CCR3_FM				BIT(20)			/* FIFO Mode select */

#define FCR_INIT			0x1F1F0000
#define FCR_RFRST 			BIT(23)			/* Receive FIFO Data Register Reset */
#define FCR_TFRST 			BIT(15)			/* Transmit FIFO Data Register Reset */

#define CSR_RDRF 			BIT(31)
#define CSR_TEND 			BIT(30)
#define CSR_FER 			BIT(28)
#define CSR_PER 			BIT(27)
#define CSR_ORER 			BIT(24)
#define CSR_ERRORS 			(CSR_PER | CSR_FER)

#define CFCLR_RDRFC			BIT(31)
#define CFCLR_TDREC			BIT(29)
#define CFCLR_ORERC			BIT(24)
