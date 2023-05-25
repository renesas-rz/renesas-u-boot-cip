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
 * File Name    : serial_rzv2m.h
 * Version      : 1.0
 ******************************************************************************/
#ifndef SERIAL_RZV2M_H
#define SERIAL_RZV2M_H

#include <linux/serial_reg.h>

#define RZV2M_SERIAL_QUEUELEN 256
#define RZV2M_SERIAL_DELAY (0xffffffffffffffff)


#define	UART0_BASE		    (0xA4040000UL)
#define	UART1_BASE		    (0xA4040080UL)

#define uart_in(port, reg)  readl(port->membase + (reg*4))
#define uart_out(port, reg, value)   writel(value, port->membase + (reg*4));

#define 	PCLK					100000000
#define     SCLK                    48000000

struct uart_port {
	unsigned long	iobase;		/* in/out[bwl] */
	unsigned char	*membase;	/* read/write[bwl] */
	unsigned long	mapbase;	/* for ioremap */
};

/* To redefine the register offset for RZ/V2M, the macros already defined are cleared by "UNDEF". */
#undef UART_FCR
#undef UART_LCR
#undef UART_MCR
#undef UART_LSR
#undef UART_MSR
#undef UART_SCR
#undef UART_DLL
#undef UART_DLM

#define UART_FCR	(0x03)	/* Out: FIFO Control Register */
#define UART_LCR	(0x04)	/* Out: Line Control Register */

#define UART_MCR	(0x05)	/* Out: Modem Control Register */
#define UART_LSR	(0x06)	/* In:  Line Status Register */
#define UART_MSR	(0x07)	/* In:  Modem Status Register */

#define UART_SCR	(0x08)	/* I/O: Scratch Register */

/*
	* DLAB=1
	*/
#define UART_DLL	(0x09)	/* Out: Divisor Latch Low */
#define UART_DLM	(0x0A)	/* Out: Divisor Latch High */


#define UART_HCR0	(0x0B)	/* HW Control Register 0 */
#define UART_HCR0_SWRST	(0x80)	/* SW Reset */
#define UART_HCR0_RM	(0x40)	/* RTS Mode */
#define UART_HCR0_RTDRD	(0x10)	/* Receiver time-out DMA REQ Disable */
#define UART_HCR0_RDE	(0x02)	/* Receiver DMA Enable */
#define UART_HCR0_TDE	(0x01)	/* Transmitter DMA Enable */


//	UART software control Modes Define
//	1=Polling,2=Interrupt,3=DMA(&Interrupt)
#define		UART_CTL_NODEF			0
#define		UART_CTL_POL			1
#define		UART_CTL_INT			2
#define		UART_CTL_TX_DMA			3
#define		UART_CTL_RX_DMA			4

//
//	UART Control Flag
//
#define		UART_ERR				0x80000000
#define		UART_RX_END				0x00800000
#define		UART_TX_END				0x00400000
#define		UART_TRX_END			0x00C00000
#define		UART_RX_EN				0x00000001
#define		UART_TX_EN				0x00000002
#define		UART_TRX_EN				0x00000003
//
// For IP
//
#define		UART_IER_RX_EN			0x0001
#define		UART_IER_TX_EN			0x0002
#define		UART_IER_TRX_EN			0x0003

//
//	UART Max Ch., UART Tx/Rx Buffer Max Size
//
#define		UART_MAX				4
#define		UART_RBFSZ				512
#define		UART_TEST_LEN			256

//
//	UART Define
//

// SCLK=24MHz BPS=38461
#define		BIT_RATE_CONST			39

#define		UART_MODE_CONST_LCR		0x0003

#define		UART_MODE_CONST_FCR_RST	0x0006	// Tx,Rx FIFO Reset

// UART_CTL_POLL
#define		UART_MODE_CONST_IER_P	0x0000	// All Disable
#define		UART_MODE_CONST_FCR_P	0x0006	// FIFO Trg=1Byte,Clear FIFO&Disable

//#define		UART_MODE_CONST_FCR_PE	0x00a7	// FIFO Trg=8Byte&64B FIFO,Clear FIFO&Enable
#define		UART_MODE_CONST_FCR_PE	0x0087	// FIFO Trg=8Byte,Clear FIFO&Enable

#define		UART_MODE_CONST_MCR_P	0x000F	// DTR OFF,Auto Flow Disable

// UART_CTL_INT
#define		UART_MODE_CONST_IER_I	0x0007	// Rx Status,Rxrdy,Txrdy(IE2,IE1,IE0) Enable
#define		UART_MODE_CONST_FCR_I	0x0006	// FIFO Trg=1Byte,Clear FIFO&Disable

//#define		UART_MODE_CONST_FCR_IE	0x0027	// FIFO Trg=1Byte,64B FIFO,Clear FIFO&Enable
//#define		UART_MODE_CONST_FCR_IE	0x00e7	// FIFO Trg=56Byte,64B FIFO,Clear FIFO&Enable
//#define		UART_MODE_CONST_FCR_IE	0x00c7	// FIFO Trg=14Byte,Clear FIFO&Enable
//#define		UART_MODE_CONST_FCR_IE	0x0047	// FIFO Trg=4Byte,Clear FIFO&Enable
//#define		UART_MODE_CONST_FCR_IE	0x0007	// FIFO Trg=1Byte,Clear FIFO&Enable
#define		UART_MODE_CONST_FCR_IE	0x0087	// FIFO Trg=8Byte,Clear FIFO&Enable

#define		UART_MODE_CONST_MCR_I	0x000F	// DTR OFF,Auto Flow Disable

// UART_CTL_TX_DMA
#define		UART_MODE_CONST_IER_DT	0x0005	// Rx Status,Rxrdy(IE2,IE0) Enable
#define		UART_MODE_CONST_FCR_DT	0x0006	// FIFO Trg=1Byte,Clear FIFO&Disable

#define		UART_MODE_CONST_FCR_DTE	0x0027	// FIFO Trg=1Byte,64B FIFO,Clear FIFO&Enable
//#define		UART_MODE_CONST_FCR_DTE	0x0087	// FIFO Trg=8Byte,Clear FIFO&Enable
//#define		UART_MODE_CONST_FCR_DTE	0x0007	// FIFO Trg=1Byte,Clear FIFO&Enable

#define		UART_MODE_CONST_MCR_DT	0x000F	// DTR OFF,Auto Flow Disable

// UART_CTL_RX_DMA
#define		UART_MODE_CONST_IER_DR	0x0006	// Rx Status,Txrdy(IE2,IE1) Enable
#define		UART_MODE_CONST_FCR_DR	0x0006	// FIFO Trg=1Byte,Clear FIFO&Disable

#define		UART_MODE_CONST_FCR_DRE	0x0027	// FIFO Trg=1Byte,64B FIFO,Clear FIFO&Enable
//#define		UART_MODE_CONST_FCR_DRE	0x0087	// FIFO Trg=8Byte,Clear FIFO&Enable
//#define		UART_MODE_CONST_FCR_DRE	0x0007	// FIFO Trg=1Byte,Clear FIFO&Enable

#define		UART_MODE_CONST_MCR_DR	0x000F	// DTR OFF,Auto Flow Disable

/*Line status*/
/*!
 * Receiver FIFO Error. This status indicates that one or more parity
 * error, framing error, or break indication exists in the receiver FIFO.
 * It is only set when FIFO is enabled. This status cleared when line
 * status is read, the character with the issue is at the top of the FIFO,
 * and when no other issues exist in the FIFO.
 */
#define UART_16550_LINE_STATUS_RFE	(1 << 7)

/*!
 * Transmitter EMpTy (Empty). This status indicates that transmitter shift
 * register is empty. If FIFOs are enabled, the status is set when the
 * transmitter FIFO is also empty. This status is cleared when the
 * transmitter shift registers is loaded by writing to the UART
 * transmitter buffer or transmitter FIFO if FIFOs are enabled. This is
 * done by calling alt_16550_write() and alt_16550_fifo_write()
 * respectively.
 */
#define UART_16550_LINE_STATUS_TEMT	(1 << 6)

/*!
 * Transmitter Holding Register Empty. This status indicates that the 
 * transmitter will run out of data soon. The definition of soon depends
 * on whether the FIFOs are enabled.
 *
 * If FIFOs are disabled, this status indicates that the transmitter will
 * run out of data to send after the current transmit shift register
 * completes. In this case, this status is cleared when the data is
 * written to the UART. This can be done by calling alt_16550_write().
 *
 * If FIFOs are enabled, this status indicates that the transmitter FIFO
 * level is below the transmitter trigger level specified. In this case,
 * this status is cleared by writing a sufficiently large buffer to the
 * transmitter FIFO such that the FIFO is filled above the transmitter
 * trigger level specified by calling alt_16550_fifo_write() or by
 * adjusting the transmitter trigger level appropriately by calling 
 * alt_16550_fifo_trigger_set_tx().
 *
 * \internal
 * The implementation of the UART driver always ensures that IER[7] is
 * set. This means that the UART always has Programmable THRE (Transmitter
 * Holding Register Empty) Interrupt Mode Enable (PTIME) enabled.
 * \endinternal
 */
#define UART_16550_LINE_STATUS_THRE	(1 << 5)

/*!
 * Break Interrupt. This status indicates that a break interrupt sequence
 * is detected in the incoming serial data. This happens when the the data
 * is 0 for longer than a frame would normally be transmitted. The break
 * interrupt status is cleared by reading the line status by calling
 * alt_16550_line_status_get().
 *
 * If FIFOs are enabled, this status will be set when the character with
 * the break interrupt status is at the top of the receiver FIFO.
 */
#define UART_16550_LINE_STATUS_BI	(1 << 4)

/*!
 * Framing Error. This status indicates that a framing error occurred in
 * the receiver. This happens when the receiver detects a missing or
 * incorrect number of stopbit(s).
 *
 * If FIFOs are enabled, this status will be set when the character with
 * the framing error is at the top of the FIFO. When a framing error
 * occurs, the UART attempts to resynchronize with the transmitting UART.
 * This status is also set if break interrupt occurred.
 */
#define UART_16550_LINE_STATUS_FE	(1 << 3)

/*!
 * Parity Error. This status indicates that a parity error occurred in the
 * receiver.
 *
 * If FIFOs are enabled, this status will be set when the character with
 * the parity error is at the top of the receiver FIFO. This status is
 * also set if a break interrupt occurred.
 */
#define UART_16550_LINE_STATUS_PE	(1 << 2)

/*!
 * Overrun Error. This status indicates that an overrun occurred in the
 * receiver.
 *
 * If FIFOs are disabled, the arriving character will overwrite the
 * existing character in the receiver. Any previously existing
 * character(s) will be lost.
 *
 * If FIFOs are disabled, the arriving character will be discarded. The
 * buffer will continue to contain the preexisting characters.
 */
#define UART_16550_LINE_STATUS_OE	(1 << 1)

/*!
 * Data Ready. This status indicates that the receiver or receiver FIFO
 * contains at least one character.
 */
#define UART_16550_LINE_STATUS_DR	(1 << 0)



#ifndef MIN
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#endif

#endif /*SERIAL_RZV2M_H*/
