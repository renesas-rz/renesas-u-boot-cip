/*
 * ravb.c - Driver for Renesas Ethernet AVB.
 *
 * Copyright (C) 2015  Renesas Electronics Corporation
 *
 * Based on the SuperH Ethernet driver.
 *
 * This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>
#include <common.h>
#include <malloc.h>
#include <net.h>
#include <netdev.h>
#include <miiphy.h>
#include <asm/errno.h>
#include <asm/io.h>

#include "ravb.h"

static inline void ravb_flush_dcache(u32 addr, u32 len)
{
	flush_dcache_range(addr, addr + len);
}

static inline void ravb_invalidate_dcache(u32 addr, u32 len)
{
	u32 start = addr & ~((u32)ARCH_DMA_MINALIGN - 1);
	u32 end = roundup(addr + len, ARCH_DMA_MINALIGN);
	invalidate_dcache_range(start, end);
}

#define TIMEOUT_CNT 1000

int ravb_send(struct eth_device *dev, void *packet, int len)
{
	struct ravb_dev *eth = dev->priv;
	struct ravb_txdesc *desc = eth->tx_desc_cur;
	int ret = 0, timeout;

	if (!packet || len > 0xffff) {
		printf(CARDNAME ": %s: Invalid argument\n", __func__);
		ret = -EINVAL;
		goto err;
	}

	/* packet must be a 4 byte boundary */
	if ((int)packet & 3) {
		printf(CARDNAME ": %s: packet not 4 byte alligned\n", __func__);
		ret = -EFAULT;
		goto err;
	}

	/* Update tx descriptor */
	ravb_flush_dcache((u32)packet, len);
	memset(desc, 0x0, sizeof(struct ravb_txdesc));
	desc->dptr = (u32)packet;
	desc->ds = len;
	desc->dt = DT_FSINGLE;
	ravb_flush_dcache((u32)desc, sizeof(struct ravb_txdesc));
	/* Restart the transmitter if disabled */
        ravb_write(eth, ravb_read(eth, TCCR) | TCCR_TSRQ0, TCCR);

        while(ravb_read(eth, TCCR) & TCCR_TSRQ0);

	/* Wait until packet is transmitted */
	timeout = TIMEOUT_CNT;
	ravb_invalidate_dcache((u32)desc, sizeof(struct ravb_txdesc));
	while ((desc->dt == DT_FSINGLE) && timeout--) {
		udelay(10);
		ravb_invalidate_dcache((u32)desc, sizeof(struct ravb_txdesc));
	}

	if (timeout < 0) {
		printf(CARDNAME ": transmit timeout\n");
		ret = -ETIMEDOUT;
		goto err;
	}

	eth->tx_desc_cur++;
	if (eth->tx_desc_cur >= eth->tx_desc_base + NUM_TX_DESC)
		eth->tx_desc_cur = eth->tx_desc_base;

err:
	return ret;
}

int ravb_recv(struct eth_device *dev)
{
	struct ravb_dev *eth = dev->priv;
	struct ravb_rxdesc *desc = eth->rx_desc_cur;
	int len = 0;
	int limit = NUM_RX_DESC;
	u8 *packet;

        while(ravb_read(eth, TCCR) & TCCR_TSRQ0);
	/* Check if the rx descriptor is ready */
	ravb_invalidate_dcache((u32)desc, sizeof(struct ravb_rxdesc));
	while (desc->dt != DT_FEMPTY) {
		/* Check for errors */
		if (desc->msc & MSC_RX_ERR_MASK) {
			desc->msc = 0x0;
		} else {
			len = desc->ds;
			packet = (u8 *)desc->dptr;
			ravb_invalidate_dcache((u32)packet, len);
			NetReceive(packet, len);
		}

		/* Make current descriptor available again */
		desc->ds = MAX_BUF_SIZE;
		desc->dt = DT_FEMPTY;
		ravb_flush_dcache((u32)desc, sizeof(struct ravb_rxdesc));
	}
		/* Point to the next descriptor */
		eth->rx_desc_cur++;
		if (eth->rx_desc_cur >=
		    eth->cur_desc_rx + NUM_RX_DESC)
			eth->rx_desc_cur = eth->rx_desc_base;
		desc = eth->rx_desc_cur;
		ravb_invalidate_dcache((u32)desc, sizeof(struct ravb_rxdesc));

	return len;
}

static int ravb_wait_setting(struct ravb_dev *eth, u16 reg, u32 bits)
{
	int i;

	for (i = 0; i < 100; i++) {
		if (ravb_read(eth, reg) & bits)
			break;
		mdelay(1);
	}
	if (i >= 100)
		return -ETIMEDOUT;

	return 0;
}

static int ravb_reset(struct ravb_dev *eth)
{
	int ret = 0;

	/* set config mode */
	ravb_write(eth, (ravb_read(eth, CCC) & ~CCC_OPC) | CCC_OPC_CONFIG, CCC);

	/* check the operating mode is changed to the config mode */
	ret = ravb_wait_setting(eth, CSR, CSR_OPS_CONFIG);
	if (ret < 0)
		printf(CARDNAME  ": Software reset timeout\n");

	return ret;
}

static int ravb_desc_bat_init(struct ravb_dev *eth)
{
	int i, ret = 0;
	u32 alloc_desc_size = DBAT_ENTRY_NUM * sizeof(struct ravb_desc);
	struct ravb_desc *cur_desc;

	/* Allocate descriptor base address table. They should be aligned */
	/* to size of struct ravb_desc. */
	eth->desc_bat_base =
		memalign(sizeof(struct ravb_desc), alloc_desc_size);
	if (!eth->desc_bat_base) {
		printf(CARDNAME ": memalign failed\n");
		ret = -ENOMEM;
		goto err;
	}

	/* Initialize all descriptors */
	memset(eth->desc_bat_base, 0x0, alloc_desc_size);
	for (cur_desc = eth->desc_bat_base, i = 0; i < DBAT_ENTRY_NUM;
	     cur_desc++, i++) {
		cur_desc->dt = DT_EOS;
	}
	ravb_flush_dcache((u32)eth->desc_bat_base, alloc_desc_size);

	/* Register the descriptor base address table */
	ravb_write(eth, (u32)eth->desc_bat_base, DBAT);

err:
	return ret;
}

static int ravb_tx_desc_init(struct ravb_dev *eth)
{
	int i, ret = 0;
	u32 alloc_desc_size = (NUM_TX_DESC + 1) * sizeof(struct ravb_txdesc);
        u32 desc_size = DBAT_ENTRY_NUM * sizeof(struct ravb_desc);
	struct ravb_txdesc *cur_tx_desc;
	struct ravb_desc *desc;

	/* Allocate rx descriptors. They must be aligned to size of struct */
	/* ravb_txdesc. */
	eth->tx_desc_base =
		memalign(sizeof(struct ravb_txdesc), alloc_desc_size);
	if (!eth->tx_desc_base) {
		printf(CARDNAME ": memalign failed\n");
		ret = -ENOMEM;
		goto err;
	}
	eth->tx_desc_cur = eth->tx_desc_base;

	/* Initialize all descriptors */
	memset(eth->tx_desc_base, 0x0, alloc_desc_size);
	for (cur_tx_desc = eth->tx_desc_base, i = 0; i < NUM_TX_DESC;
	     cur_tx_desc++, i++) {
		cur_tx_desc->dt = DT_EEMPTY;
	}
	/* Mark the end of the descriptors */
	cur_tx_desc->dt = DT_LINKFIX;
	cur_tx_desc->dptr = (u32)eth->tx_desc_base;
	ravb_flush_dcache((u32)eth->tx_desc_base, alloc_desc_size);

	/* Point the controller to the tx descriptor list. Must use physical */
	/* addresses */
	desc = &eth->desc_bat_base[TX_QUEUE];
	desc->dt = DT_LINKFIX;
	desc->dptr = (u32)eth->tx_desc_base;
	desc = &eth->desc_bat_base[RX_QUEUE];
	desc->dt = DT_LINKFIX;
	desc->dptr = (u32)eth->rx_desc_base;
	//ravb_flush_dcache((u32)desc, sizeof(struct ravb_desc));
	ravb_flush_dcache(ravb_read(eth, DBAT), desc_size);

err:
	return ret;
}

static int ravb_rx_desc_init(struct ravb_dev *eth)
{
	int i , ret = 0;
	u32 alloc_desc_size = (NUM_RX_DESC + 1) * sizeof(struct ravb_rxdesc);
	u32 alloc_buf_size = NUM_RX_DESC * MAX_BUF_SIZE;
	struct ravb_rxdesc *cur_rx_desc;
	struct ravb_desc *desc;
	u8 *rx_buf;

	/* Allocate rx descriptors. They must be aligned to size of struct */
	/* ravb_rxdesc. */
	eth->rx_desc_base =
		memalign(sizeof(struct ravb_rxdesc), alloc_desc_size);
	if (!eth->rx_desc_base) {
		printf(CARDNAME ": memalign failed\n");
		ret = -ENOMEM;
		goto err;
	}
	eth->rx_desc_cur = eth->rx_desc_base;
	eth->cur_desc_rx = eth->rx_desc_base;

	/* Allocate rx data buffers. They should be RAVB_ALIGN bytes */
	/* aligned. */
	eth->rx_buf_base =
		(u8 *)memalign(RAVB_ALIGN, alloc_buf_size);
	if (!eth->rx_buf_base) {
		printf(CARDNAME ": alloc failed\n");
		ret = -ENOMEM;
		goto err_buf_alloc;
	}
	memset(eth->rx_buf_base, 0x0, alloc_buf_size);
	ravb_flush_dcache((u32)eth->rx_buf_base, alloc_buf_size);

	/* Initialize all descriptors */
	memset(eth->rx_desc_base, 0x0, alloc_desc_size);
	for (cur_rx_desc = eth->rx_desc_base,
	     rx_buf = eth->rx_buf_base, i = 0;
	     i < NUM_RX_DESC; cur_rx_desc++, rx_buf += MAX_BUF_SIZE, i++) {
		cur_rx_desc->dt = DT_FEMPTY;
		cur_rx_desc->ds = MAX_BUF_SIZE;
		cur_rx_desc->dptr = (u32)rx_buf;
	}
	/* Mark the end of the descriptors */
	cur_rx_desc->dt = DT_LINKFIX;
	cur_rx_desc->dptr = (u32)eth->rx_desc_base;
	ravb_flush_dcache((u32)eth->rx_desc_base, alloc_desc_size);

	/* Point the controller to the rx descriptor list */

	return ret;

err_buf_alloc:
	free(eth->rx_desc_base);
	eth->rx_desc_base = NULL;

err:
	return ret;
}

static void ravb_desc_bat_free(struct ravb_dev *eth)
{
	if (eth->desc_bat_base) {
		free(eth->desc_bat_base);
		eth->desc_bat_base = NULL;
	}
}

static void ravb_tx_desc_free(struct ravb_dev *eth)
{
	if (eth->tx_desc_base) {
		free(eth->tx_desc_base);
		eth->tx_desc_base = NULL;
	}
}

static void ravb_rx_desc_free(struct ravb_dev *eth)
{
	if (eth->rx_desc_base) {
		free(eth->rx_desc_base);
		eth->rx_desc_base = NULL;
	}

	if (eth->rx_buf_base) {
		free(eth->rx_buf_base);
		eth->rx_buf_base = NULL;
	}
}

static int ravb_desc_init(struct ravb_dev *eth)
{
	int ret = 0;

	ret = ravb_rx_desc_init(eth);
	if (ret)
		goto err_rx_init;

	ret = ravb_tx_desc_init(eth);
	if (ret)
		goto err_tx_init;

	return ret;

err_rx_init:
	ravb_tx_desc_free(eth);

err_tx_init:
	return ret;
}

static int ravb_phy_config(struct ravb_dev *eth)
{
	int ret = 0;
	struct eth_device *dev = eth->dev;
	struct phy_device *phydev;

#ifdef CONFIG_IWG23S
	int addr;
         for (addr = 0; addr <  32; addr++)
        {
                phydev = phy_connect(
                        miiphy_get_dev_by_name(dev->name),
                        addr, dev, CONFIG_RAVB_PHY_MODE);
                if (phydev == NULL)
                        continue;
                else
                {
                        printf("PHY detected at addr %d\n", addr);
                        break;
                }

        }
#else
	phydev = phy_connect(
			miiphy_get_dev_by_name(dev->name),
			eth->phy_addr, dev, CONFIG_RAVB_PHY_MODE);
#endif
	if (!phydev)
		return -1;

	eth->phydev = phydev;
	phy_config(phydev);

	/* 10BASE is not supported for Ethernet AVB MAC */
	phydev->supported &= ~(SUPPORTED_10baseT_Full
			       | SUPPORTED_10baseT_Half);

	return ret;
}

/* Set Mac address */
static int ravb_write_hwaddr(struct eth_device *dev)
{
	struct ravb_dev *eth = dev->priv;
	u32 val;
	val = dev->enetaddr[0] << 24 | dev->enetaddr[1] << 16 |
	      dev->enetaddr[2] << 8 | dev->enetaddr[3];
	ravb_write(eth, val, MAHR);

	val = dev->enetaddr[4] << 8 | dev->enetaddr[5];
	ravb_write(eth, val, MALR);

	return 0;
}

/* E-MAC init function */
static int ravb_mac_init(struct ravb_dev *eth)
{
	struct eth_device *dev = eth->dev;

	/* Disable MAC Interrupt */
	ravb_write(eth, 0, ECSIPR);

	/* Recv frame limit set register */
	ravb_write(eth, RFLR_RFL_MIN, RFLR);

	/* Set Mac address */
	ravb_write_hwaddr(dev);

	return 0;
}

/* AVB-DMAC init function */
static int ravb_dmac_init(struct ravb_dev *eth)
{
	int ret = 0;

	/* Set CONFIG mode */
	ret = ravb_reset(eth);
	if (ret)
		return ret;

	/* Disable all interrupts */
	ravb_write(eth, 0, RIC0);
	ravb_write(eth, 0, RIC1);
	ravb_write(eth, 0, RIC2);
	ravb_write(eth, 0, TIC);

#if defined(__LITTLE_ENDIAN)
	ravb_write(eth, ravb_read(eth, CCC) & ~CCC_BOC, CCC);
#else
	ravb_write(eth, ravb_read(eth, CCC) | CCC_BOC, CCC);
#endif

	/* AVB rx set */
	ravb_write(eth, 0x18000001, RCR);

	/* FIFO size set */
	ravb_write(eth, 0x00222210, TGC);

	return ret;
}

static int ravb_config(struct ravb_dev *eth, bd_t *bd)
{
	int ret = 0;
	struct phy_device *phy;
        u32 val;
        struct eth_device *dev = eth->dev;

        /* all ravb int mask disable*/
        ravb_write(eth, ( ravb_read(eth, ECMR) & ~( ECMR_TE | ECMR_RE )), ECMR);
        ravb_write(eth, 0, RIC0);
        ravb_write(eth, 0, RIC1);
        ravb_write(eth, 0, RIC2);
        ravb_write(eth, 0, TIC);
        ravb_write(eth, ravb_read(eth, CCC) & ~CCC_BOC, CCC);

        ravb_write(eth, 0x18000001, RCR);

        /* FIFO size set */
        ravb_write(eth, 0x00222210, TGC);
#ifdef NETETH
        /* delay CLK: 2ns */
        ravb_write(eth, 0x1ul << 14, PSR);
#endif

        /* Timestamp Enable */
        ravb_write(eth, 0x00000100, TCCR);
        ravb_write(eth, 0x00000003, RIC0);
        /* Receive FIFO full warning */
        ravb_write(eth, 0x80000000, RIC1);
        /* Receive FIFO full error, Descriptor Empty */
        ravb_write(eth, 0x80000003, RIC2);
        /* Frame Transmited, Timestamp FIFO updated */
        ravb_write(eth, 0x00000103, TIC);

        ravb_write(eth, RFLR_RFL_MIN, RFLR);

        /* Setting the control will start the AVB-DMAC process. */
        ravb_write(eth,(ravb_read(eth, CCC) & ~CCC_OPC) | CCC_OPC_OPERATION, CCC);

        ravb_write_hwaddr(dev);
        /* mask reset */
        ravb_write(eth, MPR_MP, MPR);
        /* E-MAC Status Register clear */
        ravb_write(eth, ECSR_ICD | ECSR_MPD, ECSR);
        /* Configure phy */
        ravb_write(eth, ECSIPR_LCHNGIP | ECSIPR_ICDIP | ECSIPR_MPDIP, ECSIPR);

	/* Configure AVB-DMAC register */
	ravb_dmac_init(eth);

	/* Configure E-MAC registers */
	ravb_mac_init(eth);

	/* Configure phy */
	ret = ravb_phy_config(eth);
	if (ret) {
		printf(CARDNAME ": Fail to connect phy\n");
		goto err_phy_cfg;
	}
	phy = eth->phydev;

	ret = phy_read(phy, MDIO_DEVAD_NONE, 0x1e);
	ret &= ~0xc000;
	ret |= 0x4000;
	phy_write(phy, MDIO_DEVAD_NONE, 0x1e, (u16)ret);

	ret = phy_startup(phy);
	if (ret) {
		printf(CARDNAME ": phy startup failure\n");
		return ret;
	}

	/* Set the transfer speed */
	if (phy->speed == 100) {
		printf(CARDNAME ": 100Base/");
		ravb_write(eth, 0, GECMR);
	} else if (phy->speed == 1000) {
		printf(CARDNAME ": 1000Base/");
		ravb_write(eth, 1, GECMR);
	} else if (phy->speed == 10)
                printf(CARDNAME ": not supported /");

	/* Check if full duplex mode is supported by the phy */
	if (phy->duplex) {
		printf("Full\n");
        ravb_write(eth, ( ravb_read(eth, ECMR) | ECMR_TE |ECMR_DM | ECMR_RE | ECMR_ZPF | ECMR_TXF), ECMR);
	} else {
		printf("Half\n");
        ravb_write(eth,  (ravb_read(eth, ECMR) |  ECMR_TE | ECMR_RE | ECMR_RTM | ECMR_CHG_DM), ECMR);
	}

	return ret;

err_phy_cfg:
	return ret;
}

static void ravb_start(struct ravb_dev *eth)
{
	/* Setting the control will start the AVB-DMAC process. */
	ravb_write(eth, CCC_OPC_OPERATION, CCC);
        udelay(100);
}

int ravb_init(struct eth_device *dev, bd_t *bd)
{
	int ret = 0;
	struct ravb_dev *eth = dev->priv;

	ret = ravb_reset(eth);
	if (ret)
		goto err;

	ret = ravb_desc_bat_init(eth);
	if (ret)
		goto err;

	ret = ravb_desc_init(eth);
	if (ret)
		goto err;

	ret = ravb_config(eth, bd);
	if (ret)
		goto err_config;

	ravb_start(eth);

	return ret;

err_config:
	ravb_tx_desc_free(eth);
	ravb_rx_desc_free(eth);
	ravb_desc_bat_free(eth);
err:
	return ret;
}

static void ravb_stop(struct ravb_dev *eth)
{
        int timeout = 100, timeout1 = 100;
        /* Request for transfer suspension */
        ravb_write(eth, CCC_DTSR, CCC);
        while(ravb_read(eth, CSR) == CSR_DTS && timeout )
                timeout--;

	/* Setting the control will stop the Hardware process. */
	ravb_write(eth, CCC_OPC_RESET, CCC);

        /*Wait until reset is done */
        while(ravb_read(eth, CSR) == CSR_OPS && timeout1)
                timeout1--;
}

static void ravb_halt(struct eth_device *dev)
{
	struct ravb_dev *eth = dev->priv;
	int i;

	while(ravb_read(eth, CCC) & CCC_OPC_OPERATION)
	{
		/*Disable receive */
		ravb_write(eth, ravb_read(eth, ECMR) & ~ECMR_RE, ECMR);
		ravb_write(eth, ravb_read(eth, ECMR) & ~ECMR_TE, ECMR);
		for (i = 0; i < 100; i++) {
			if (!(ravb_read(eth, CSR) & (CSR_TPO0 | CSR_RPO)))
				break;
			udelay(100);
		}
		if (i >= 100)
			printf("Timeout error\n");

		ravb_reset(eth);
		ravb_stop(eth);
	}
}

int ravb_initialize(bd_t *bd)
{
	int ret = 0;
	struct ravb_dev *eth = NULL;
	struct eth_device *dev = NULL;

	eth = (struct ravb_dev *)malloc(sizeof(struct ravb_dev));
	if (!eth) {
		printf(CARDNAME ": %s: malloc failed\n", __func__);
		ret = -ENOMEM;
		goto err;
	}

	dev = (struct eth_device *)malloc(sizeof(struct eth_device));
	if (!dev) {
		printf(CARDNAME ": %s: malloc failed\n", __func__);
		ret = -ENOMEM;
		goto err;
	}
	memset(dev, 0, sizeof(struct eth_device));
	memset(eth, 0, sizeof(struct ravb_dev));

	eth->phy_addr = CONFIG_RAVB_PHY_ADDR;

	dev->priv = (void *)eth;
	dev->iobase = 0;
	dev->init = ravb_init;
	dev->halt = ravb_halt;
	dev->send = ravb_send;
	dev->recv = ravb_recv;
	dev->write_hwaddr = ravb_write_hwaddr;
	eth->dev = dev;

	sprintf(dev->name, CARDNAME);

	/* Register Device to EtherNet subsystem */
	eth_register(dev);

        /*Configuration mode*/
        ravb_write(eth,(ravb_read(eth, CCC) & ~CCC_OPC) | CCC_OPC_CONFIG, CCC);

	bb_miiphy_buses[dev->index].priv = eth;
	miiphy_register(dev->name, bb_miiphy_read, bb_miiphy_write);

        if (!eth_getenv_enetaddr("ethaddr", dev->enetaddr))
                puts("Please set MAC address\n");


	return ret;

err:
	if (dev)
		free(dev);

	if (eth)
		free(eth);

	printf(CARDNAME ": Failed\n");
	return ret;
}

/******* for bb_miiphy *******/
int ravb_bb_init(struct bb_miiphy_bus *bus)
{
        struct ravb_dev *eth = bus->priv;
        int ret;

        ret = ravb_reset(eth);
        if (ret)
                return ret;

	return 0;
}

int ravb_bb_mdio_active(struct bb_miiphy_bus *bus)
{
	struct ravb_dev *eth = bus->priv;

	ravb_write(eth, ravb_read(eth, PIR) | PIR_MMD, PIR);

	return 0;
}

int ravb_bb_mdio_tristate(struct bb_miiphy_bus *bus)
{
	struct ravb_dev *eth = bus->priv;

	ravb_write(eth, ravb_read(eth, PIR) & ~PIR_MMD, PIR);

	return 0;
}

int ravb_bb_set_mdio(struct bb_miiphy_bus *bus, int v)
{
	struct ravb_dev *eth = bus->priv;

	if (v)
		ravb_write(eth, ravb_read(eth, PIR) | PIR_MDO, PIR);
	else
		ravb_write(eth, ravb_read(eth, PIR) & ~PIR_MDO, PIR);

	return 0;
}

int ravb_bb_get_mdio(struct bb_miiphy_bus *bus, int *v)
{
	struct ravb_dev *eth = bus->priv;

	*v = (ravb_read(eth, PIR) & PIR_MDI) >> 3;

	return 0;
}

int ravb_bb_set_mdc(struct bb_miiphy_bus *bus, int v)
{
	struct ravb_dev *eth = bus->priv;

	if (v)
		ravb_write(eth, ravb_read(eth, PIR) | PIR_MDC, PIR);
	else
		ravb_write(eth, ravb_read(eth, PIR) & ~PIR_MDC, PIR);

	return 0;
}

int ravb_bb_delay(struct bb_miiphy_bus *bus)
{
	udelay(10);

	return 0;
}
