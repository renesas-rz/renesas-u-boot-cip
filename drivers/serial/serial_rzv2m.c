/*******************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only
* intended for use with Renesas products. No other uses are authorized. This
* software is owned by Renesas Electronics Corporation and is protected under
* all applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT
* LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
* AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR
* ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE
* BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software
* and to discontinue the availability of this software. By using this software,
* you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
* Copyright (C) 2021 Renesas Electronics Corporation. All rights reserved.
*******************************************************************************/ 
/*******************************************************************************
 * File Name    : serial_rzv2m.c
 * Version      : 1.0
 ******************************************************************************/

#include <common.h>
#include <asm/io.h>
#include <linux/delay.h>
#include "serial.h"

#include "serial_rzv2m.h"
#include <init.h>

/*-----------------------------------------------------------*/
DECLARE_GLOBAL_DATA_PTR;

struct uart_port *port;

#if defined(CONFIG_CONS_UART0)
#define UART_BASE	UART0_BASE
#elif defined(CONFIG_CONS_UART1)
#define UART_BASE	UART1_BASE
#else
#define UART_BASE	UART0_BASE
#endif

#define	UART_STAT_NONE			(0U)
#define	UART_STAT_ERROR			(0x80000000U)
#define	UART_STAT_RX_READY		(0x00000001U)
#define	UART_STAT_TX_READY		(0x00000002U)

static struct uart_port rzv2m_port = {
	.membase	= (unsigned char *)UART_BASE,
	.mapbase	= UART_BASE,
};

static void rzv2m_serial_init_generic(struct uart_port *port)
{
	unsigned long val;
	unsigned long t;

	uart_out(port, UART_FCR, UART_MODE_CONST_FCR_RST);
	val = uart_in(port, UART_HCR0);
	uart_out(port, UART_HCR0, val | 0x0080);
	t = (1000000*6)/PCLK;// Delay Over 6*PLCK(For fail soft)
	udelay(t);
	uart_out(port, UART_HCR0, val & ~(0x0080));// S/W Reset release
	udelay(t);

	uart_out(port, UART_IER, UART_MODE_CONST_IER_P);	// Set Int. All Disable
	uart_out(port, UART_FCR, UART_MODE_CONST_FCR_P);	// Set Int. FIFO Trigger Level,etc.
	uart_out(port, UART_MCR, UART_MODE_CONST_MCR_P);	// Set Flow Ctl. Enable/Disble,etc
	
	uart_out(port, UART_HCR0, 0x0000);
}


static int rzv2m_serial_init(void)
{
	int ret = 0;

	port = &rzv2m_port;

	rzv2m_serial_init_generic(port);
	serial_setbrg();
	
	return ret; 
}

static void rzv2m_serial_setbrg_generic(struct uart_port *port, unsigned int baudrate)
{
	unsigned long bit_rate;
	unsigned long val;
	
	val = uart_in(port,UART_LCR);
	uart_out(port, UART_LCR, val | 0x0080);	// Select Div. Latch Register

	bit_rate = SCLK / (baudrate*16);
	uart_out(port, UART_DLL,(uint8_t)bit_rate);
	uart_out(port, UART_DLM,(uint8_t)bit_rate>>8);
	
	uart_out(port, UART_LCR, UART_MODE_CONST_LCR);	// Select Div. Latch Register

}

static void rzv2m_serial_setbrg(void)
{
	struct uart_port *port = &rzv2m_port;

	rzv2m_serial_setbrg_generic(port,gd->baudrate);
}

static int serial_raw_putc(struct uart_port *port, const char c)
{
	int status = 0;

	if(0 == port){
		return -EAGAIN;
	}
	status = uart_in(port,UART_LSR);
	if(0 == (status & (UART_16550_LINE_STATUS_THRE | UART_16550_LINE_STATUS_TEMT)))
	{
		return -EAGAIN;
	}

	uart_out(port,UART_TX,c)

	return 0;
}

static void rzv2m_serial_putc(const char c)
{
	struct uart_port *port = &rzv2m_port;

	if(c == '\n') {
		while (1) {
			if(serial_raw_putc(port, '\r') != -EAGAIN)
				break;
		}
	}
	while (1) {
		if(serial_raw_putc(port, c) != -EAGAIN)
			break;
	}
}

#if 0
 static void rzv2m_serial_puts(const char * s)
 {
 	struct serial_device *dev = get_current();
 	while(*s)
 		dev->putc(*s++);
 }
#endif

uint32_t uart_trx_ready_poll(struct uart_port *port)
{
	uint32_t sts;
	uint32_t res = UART_STAT_NONE;

        sts = uart_in(port,UART_LSR);           /** Read Line Status */
        if(sts & UART_LSR_ERROR_BITS)           /** Error(FE/PE/OV) ? */

        { 
               uart_in(port,UART_RX);                  /*Dummy read*/
                res = (UART_STAT_ERROR);
	
	}
	else
	{
		if((sts & UART_LSR_DR))                 /** Rx ready **/
		{
			res = (UART_STAT_RX_READY);
		}
		else if((sts & (UART_LSR_TEMT |UART_LSR_THRE)) == (UART_LSR_TEMT |UART_LSR_THRE))       /** Tx & Rx ready **/
		{
			res = (UART_STAT_TX_READY);
		}
	}

	return res;
}

static int rzv2m_serial_getc_generic(struct uart_port *port)
{
	uint32_t sts;
	
	sts = uart_trx_ready_poll(port);
	if(sts & UART_STAT_ERROR){
		return -EAGAIN;
	}
	if(sts & UART_STAT_RX_READY)   /** Rx ready ? */
    {
        return  uart_in(port,UART_RX);  /** Read Rx data */
    }
    else /*UART_STAT_ERROR or UART_STAT_TX_READY or UART_STAT_NONE*/
    {
		return -EAGAIN;
    }
}

static int rzv2m_serial_getc(void)
{
	struct uart_port *port = &rzv2m_port;
	int ch;

	while(1) {
		ch = rzv2m_serial_getc_generic(port);
		if (ch != -EAGAIN)
			break;
	}
	return (int)ch;
}

static int rzv2m_serial_tstc(void)
{
	struct uart_port *port = &rzv2m_port;
	return ((uart_trx_ready_poll(port) & UART_STAT_RX_READY) != 0);
}

static struct serial_device rzv2m_serial_drv = {
	.name	= "rzv2m_serial",
	.start	= rzv2m_serial_init,
	.stop	= NULL,
	.setbrg	= rzv2m_serial_setbrg,
	.putc	= rzv2m_serial_putc,
	.puts	= default_serial_puts,
	.getc	= rzv2m_serial_getc,
	.tstc	= rzv2m_serial_tstc,
};

void rzv2m_serial_initialize(void)
{
	serial_register(&rzv2m_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &rzv2m_serial_drv;
}
