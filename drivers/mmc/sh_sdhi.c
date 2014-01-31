/*
 * linux/drivers/mmc/host/sh-sdhi.c
 *
 * SD/MMC driver.
 *
 * Copyright (C) 2011,2013 Renesas Electronics Corporation
 * Copyright (C) 2008-2009 Renesas Solutions Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 */

#include <config.h>
#include <common.h>
#include <command.h>
#include <mmc.h>
#include <malloc.h>
#include <mmc.h>
#include <asm/errno.h>
#include <asm/io.h>

#ifdef DEBUG
#define pr_debug	printf
#else
#define pr_debug(...)
#endif


#include "sh_sdhi.h"


#define DRIVER_NAME "sh-sdhi"

static void *mmc_priv(struct mmc *mmc)
{
	return (void *)mmc->priv;
}

static int detect_waiting;
static void sdhi_detect(struct sdhi_host *host)
{
	sdhi_writew(host, SDHI_OPTION,
			OPT_BUS_WIDTH_1 | sdhi_readw(host, SDHI_OPTION));

	detect_waiting = 0;
}

static int sdhi_intr(void *dev_id)
{
	struct sdhi_host *host = dev_id;
	int state1 = 0, state2 = 0;

	state1 = sdhi_readw(host, SDHI_INFO1);
	state2 = sdhi_readw(host, SDHI_INFO2);

	pr_debug("%s: state1 = %x, state2 = %x\n", __func__, state1, state2);

	/* CARD Insert */
	if (state1 & INFO1_CARD_IN) {
		sdhi_writew(host, SDHI_INFO1, ~INFO1_CARD_IN);
		if (!detect_waiting) {
			detect_waiting = 1;
			sdhi_detect(host);
		}
		sdhi_writew(host, SDHI_INFO1_MASK, INFO1M_RESP_END	|
				INFO1M_ACCESS_END		|
				INFO1M_CARD_IN		|
				INFO1M_DATA3_CARD_RE	|
				INFO1M_DATA3_CARD_IN);
		return -EAGAIN;
	}
	/* CARD Removal */
	if (state1 & INFO1_CARD_RE) {
		sdhi_writew(host, SDHI_INFO1, ~INFO1_CARD_RE);
		if (!detect_waiting) {
			detect_waiting = 1;
			sdhi_detect(host);
		}
		sdhi_writew(host, SDHI_INFO1_MASK, INFO1M_RESP_END |
				INFO1M_ACCESS_END		|
				INFO1M_CARD_RE		|
				INFO1M_DATA3_CARD_RE	|
				INFO1M_DATA3_CARD_IN);
		sdhi_writew(host, SDHI_SDIO_INFO1_MASK, SDIO_INFO1M_ON);
		sdhi_writew(host, SDHI_SDIO_MODE, SDIO_MODE_OFF);
		return -EAGAIN;
	}

	if (state2 & INFO2_ALL_ERR) {
		sdhi_writew(host, SDHI_INFO2, (unsigned short)~(INFO2_ALL_ERR));
		sdhi_writew(host, SDHI_INFO2_MASK,
			INFO2M_ALL_ERR | sdhi_readw(host, SDHI_INFO2_MASK));
		g_sd_error[host->ch] = 1;
		g_wait_int[host->ch] = 1;
		return 0;
	}
	/* Respons End */
	if (state1 & INFO1_RESP_END) {
		sdhi_writew(host, SDHI_INFO1, ~INFO1_RESP_END);
		sdhi_writew(host, SDHI_INFO1_MASK,
			INFO1M_RESP_END | sdhi_readw(host, SDHI_INFO1_MASK));
		g_wait_int[host->ch] = 1;
		return 0;
	}
	/* SD_BUF Read Enable */
	if (state2 & INFO2_BRE_ENABLE) {
		sdhi_writew(host, SDHI_INFO2, ~INFO2_BRE_ENABLE);
		sdhi_writew(host, SDHI_INFO2_MASK,
				INFO2M_BRE_ENABLE | INFO2M_BUF_ILL_READ |
				sdhi_readw(host, SDHI_INFO2_MASK));
		g_wait_int[host->ch] = 1;
		return 0;
	}
	/* SD_BUF Write Enable */
	if (state2 & INFO2_BWE_ENABLE) {
		sdhi_writew(host, SDHI_INFO2, ~INFO2_BWE_ENABLE);
		sdhi_writew(host, SDHI_INFO2_MASK,
				INFO2_BWE_ENABLE | INFO2M_BUF_ILL_WRITE |
				sdhi_readw(host, SDHI_INFO2_MASK));
		g_wait_int[host->ch] = 1;
		return 0;
	}
	/* Access End */
	if (state1 & INFO1_ACCESS_END) {
		sdhi_writew(host, SDHI_INFO1, ~INFO1_ACCESS_END);
		sdhi_writew(host, SDHI_INFO1_MASK,
			INFO1_ACCESS_END | sdhi_readw(host, SDHI_INFO1_MASK));
		g_wait_int[host->ch] = 1;
		return 0;
	}
	return -EAGAIN;
}

static int sdhi_wait_interrupt_flag(struct sdhi_host *host)
{
	int timeout = 10000000;

	while (1) {
		timeout--;
		if (timeout < 0) {
			printf("timeout\n");
			return 0;
		}

		if (!sdhi_intr(host))
			break;

		udelay(1);	/* 1 usec */
	}

	return 1;	/* Return value: NOT 0 = complete waiting */
}

static void sdhi_clock_control(struct sdhi_host *host, unsigned long clk)
{
	u32 clkdiv, i;

	if (sdhi_readw(host, SDHI_INFO2) & (1 << 14)) {
		printf(DRIVER_NAME": Busy state ! Cannot change the clock\n");
		return;
	}

	sdhi_writew(host, SDHI_CLK_CTRL,
			~CLK_ENABLE & sdhi_readw(host, SDHI_CLK_CTRL));

	if (clk == 0)
		return;

	clkdiv = 0x80;
	i = CONFIG_SH_SDHI_FREQ >> (0x8 + 1);
	for ( ; clkdiv && clk >= (i << 1); (clkdiv >>= 1))
		i <<= 1;

	sdhi_writew(host, SDHI_CLK_CTRL, clkdiv);

	/* Waiting for SD Bus busy to be cleared */
	while ((sdhi_readw(host, SDHI_INFO2) & 0x2000) == 0)
		;

	sdhi_writew(host, SDHI_CLK_CTRL,
			CLK_ENABLE | sdhi_readw(host, SDHI_CLK_CTRL));
}

static void sdhi_sync_reset(struct sdhi_host *host)
{
	sdhi_writew(host, SDHI_SOFT_RST, SOFT_RST_ON);
	sdhi_writew(host, SDHI_SOFT_RST, SOFT_RST_OFF);
	sdhi_writew(host, SDHI_CLK_CTRL,
			CLK_ENABLE | sdhi_readw(host, SDHI_CLK_CTRL));

	while (sdhi_readw(host, SDHI_INFO2) & INFO2_CBUSY)
		udelay(100);

#if defined(LAGER_SDRAM_BASE)
	if (host->ch < 2)
		sdhi_writew(host, SDHI_HOST_MODE, 1);	/* 16bit access */
#elif defined(KOELSCH_SDRAM_BASE)
	if (host->ch == 0)
		sdhi_writew(host, SDHI_HOST_MODE, 1);	/* 16bit access */
#else
#error
#endif
}

static int sdhi_error_manage(struct sdhi_host *host)
{
	unsigned short e_state1, e_state2;
	int ret;

	g_sd_error[host->ch] = 0;
	g_wait_int[host->ch] = 0;

#if 0
	if (!(sdhi_readw(host, SDHI_INFO1) & INFO1_ISD0CD)) {
		pr_debug("%s: card remove(INFO1 = %04x)\n", \
			DRIVER_NAME, sdhi_readw(host, SDHI_INFO1));
		return NO_CARD_ERR;
	}
#endif

	e_state1 = sdhi_readw(host, SDHI_ERR_STS1);
	e_state2 = sdhi_readw(host, SDHI_ERR_STS2);
	if (e_state2 & ERR_STS2_SYS_ERROR) {
		if (e_state2 & ERR_STS2_RES_STOP_TIMEOUT)
			ret = TIMEOUT;
		else
			ret = -EILSEQ;
		pr_debug("%s: ERR_STS2 = %04x\n", \
				DRIVER_NAME, sdhi_readw(host, SDHI_ERR_STS2));
		sdhi_sync_reset(host);
		sdhi_writew(host, SDHI_INFO1_MASK,
				INFO1M_DATA3_CARD_RE | INFO1M_DATA3_CARD_IN);
		return ret;
	}
	if (e_state1 & ERR_STS1_CRC_ERROR || e_state1 & ERR_STS1_CMD_ERROR)
		ret = -EILSEQ;
	else
		ret = TIMEOUT;

	pr_debug("%s: ERR_STS1 = %04x \n", DRIVER_NAME,
		sdhi_readw(host, SDHI_ERR_STS1));
	sdhi_sync_reset(host);
	sdhi_writew(host, SDHI_INFO1_MASK,
			INFO1M_DATA3_CARD_RE | INFO1M_DATA3_CARD_IN);
	return ret;
}

static int sdhi_single_read(struct sdhi_host *host, struct mmc_data *data)
{
	int ch = host->ch;
	long time;
	unsigned short blocksize, i;
	unsigned short *p = (unsigned short *)data->dest;

	if ((unsigned long)p & 0x00000001) {
		printf("%s: The data pointer is unaligned.", __func__);
		return -EIO;
	}

	g_wait_int[ch] = 0;
	sdhi_writew(host, SDHI_INFO2_MASK,
			~(INFO2M_BRE_ENABLE | INFO2M_BUF_ILL_READ) &
			sdhi_readw(host, SDHI_INFO2_MASK));
	sdhi_writew(host, SDHI_INFO1_MASK,
			~INFO1M_ACCESS_END & sdhi_readw(host, SDHI_INFO1_MASK));
	time = sdhi_wait_interrupt_flag(host);
	if (time == 0 || g_sd_error[ch] != 0)
		return sdhi_error_manage(host);

	g_wait_int[ch] = 0;
	blocksize = sdhi_readw(host, SDHI_SIZE);
	for (i = 0; i < blocksize / 2; i++)
		*p++ = sdhi_readw(host, SDHI_BUF0);

	time = sdhi_wait_interrupt_flag(host);
	if (time == 0 || g_sd_error[ch] != 0)
		return sdhi_error_manage(host);

	g_wait_int[ch] = 0;
	return 0;
}

static int sdhi_multi_read(struct sdhi_host *host, struct mmc_data *data)
{
	int ch = host->ch;
	long time;
	unsigned short blocksize, i, sec;
	unsigned short *p = (unsigned short *)data->dest;

	if ((unsigned long)p & 0x00000001) {
		printf("%s: The data pointer is unaligned.", __func__);
		return -EIO;
	}

	pr_debug("%s: blocks = %d, blocksize = %d\n",
			__func__, data->blocks, data->blocksize);

	g_wait_int[ch] = 0;
	for (sec = 0; sec < data->blocks; sec++) {
		sdhi_writew(host, SDHI_INFO2_MASK,
				~(INFO2M_BRE_ENABLE | INFO2M_BUF_ILL_READ) &
				sdhi_readw(host, SDHI_INFO2_MASK));

		time = sdhi_wait_interrupt_flag(host);
		if (time == 0 || g_sd_error[ch] != 0)
			return sdhi_error_manage(host);

		g_wait_int[ch] = 0;
		blocksize = sdhi_readw(host, SDHI_SIZE);
		for (i = 0; i < blocksize / 2; i++)
			*p++ = sdhi_readw(host, SDHI_BUF0);
	}

	return 0;
}

static int sdhi_single_write(struct sdhi_host *host, struct mmc_data *data)
{
	int ch = host->ch;
	long time;
	unsigned short blocksize, i;
	const unsigned short *p = (const unsigned short *)data->src;

	if ((unsigned long)p & 0x00000001) {
		printf("%s: The data pointer is unaligned.", __func__);
		return -EIO;
	}

	pr_debug("%s: blocks = %d, blocksize = %d\n",
			__func__, data->blocks, data->blocksize);

	g_wait_int[ch] = 0;
	sdhi_writew(host, SDHI_INFO2_MASK,
			~(INFO2M_BWE_ENABLE | INFO2M_BUF_ILL_WRITE) &
			sdhi_readw(host, SDHI_INFO2_MASK));
	sdhi_writew(host, SDHI_INFO1_MASK,
			~INFO1M_ACCESS_END & sdhi_readw(host, SDHI_INFO1_MASK));

	time = sdhi_wait_interrupt_flag(host);
	if (time == 0 || g_sd_error[ch] != 0)
		return sdhi_error_manage(host);

	g_wait_int[ch] = 0;
	blocksize = sdhi_readw(host, SDHI_SIZE);
	for (i = 0; i < blocksize / 2; i++)
		sdhi_writew(host, SDHI_BUF0, *p++);

	time = sdhi_wait_interrupt_flag(host);
	if (time == 0 || g_sd_error[ch] != 0)
		return sdhi_error_manage(host);

	g_wait_int[ch] = 0;
	return 0;
}

static int sdhi_multi_write(struct sdhi_host *host, struct mmc_data *data)
{
	int ch = host->ch;
	long time;
	unsigned short i, sec, blocksize;
	const unsigned short *p = (const unsigned short *)data->src;

	pr_debug("%s: blocks = %d, blocksize = %d\n",
			__func__, data->blocks, data->blocksize);

	g_wait_int[ch] = 0;
	for (sec = 0; sec < data->blocks; sec++) {
		sdhi_writew(host, SDHI_INFO2_MASK,
				~(INFO2M_BWE_ENABLE | INFO2M_BUF_ILL_WRITE) &
				sdhi_readw(host, SDHI_INFO2_MASK));

		time = sdhi_wait_interrupt_flag(host);
		if (time == 0 || g_sd_error[ch] != 0)
			return sdhi_error_manage(host);

		g_wait_int[ch] = 0;
		blocksize = sdhi_readw(host, SDHI_SIZE);
		for (i = 0; i < blocksize / 2; i++)
			sdhi_writew(host, SDHI_BUF0, *p++);
	}

	return 0;
}

static void sdhi_get_response(struct sdhi_host *host, struct mmc_cmd *cmd)
{
	unsigned short i, j;
	volatile unsigned short resp[8];
	volatile unsigned long *p1, *p2;

	if (cmd->resp_type & MMC_RSP_136) {
		resp[0] = sdhi_readw(host, SDHI_RSP00);
		resp[1] = sdhi_readw(host, SDHI_RSP01);
		resp[2] = sdhi_readw(host, SDHI_RSP02);
		resp[3] = sdhi_readw(host, SDHI_RSP03);
		resp[4] = sdhi_readw(host, SDHI_RSP04);
		resp[5] = sdhi_readw(host, SDHI_RSP05);
		resp[6] = sdhi_readw(host, SDHI_RSP06);
		resp[7] = sdhi_readw(host, SDHI_RSP07);

		/* SDHI REGISTER SPECIFICATION */
		for (i = 7, j = 6; i > 0; i--) {
			resp[i] = (resp[i] << 8) & 0xff00;
			resp[i] |= (resp[j--] >> 8) & 0x00ff;
		}
		resp[0] = (resp[0] << 8) & 0xff00;
		/* SDHI REGISTER SPECIFICATION */

		p1 = ((unsigned long *)resp) + 3;
		p2 = (unsigned long *)cmd->response;
#if defined(__BIG_ENDIAN_BITFIELD)
		for (i = 0; i < 4; i++) {
			*p2++ = ((*p1 >> 16) & 0x0000ffff) |
					((*p1 << 16) & 0xffff0000);
			p1--;
		}
#else
		for (i = 0; i < 4; i++)
			*p2++ = *p1--;
#endif /* __BIG_ENDIAN_BITFIELD */

	} else {
		resp[0] = sdhi_readw(host, SDHI_RSP00);
		resp[1] = sdhi_readw(host, SDHI_RSP01);

		p1 = ((unsigned long *)resp);
		p2 = (unsigned long *)cmd->response;
#if defined(__BIG_ENDIAN_BITFIELD)
		*p2 = ((*p1 >> 16) & 0x0000ffff) | ((*p1 << 16) & 0xffff0000);
#else
		*p2 = *p1;
#endif /* __BIG_ENDIAN_BITFIELD */
	}
}

static unsigned short sdhi_set_cmd(struct sdhi_host *host,
			struct mmc_data *data, unsigned short opc)
{
	switch (opc) {
	case SD_CMD_APP_SEND_OP_COND:
	case SD_CMD_APP_SEND_SCR:
		opc |= SDHI_APP;
		break;
	case SD_CMD_APP_SET_BUS_WIDTH:
		 /* SD_APP_SET_BUS_WIDTH*/
		if (host->data == 0)
			opc |= SDHI_APP;
		else /* SD_SWITCH */
			opc = SDHI_SD_SWITCH;
		break;
	default:
		break;
	}
	return opc;
}

static unsigned short sdhi_data_trans(struct sdhi_host *host,
				struct mmc_data *data, unsigned short opc)
{
	unsigned short ret;

	switch (opc) {
	case MMC_CMD_READ_MULTIPLE_BLOCK:
		ret = sdhi_multi_read(host, data);
		break;
	case MMC_CMD_WRITE_MULTIPLE_BLOCK:
		ret = sdhi_multi_write(host, data);
		break;
	case MMC_CMD_WRITE_SINGLE_BLOCK:
		ret = sdhi_single_write(host, data);
		break;
	case MMC_CMD_READ_SINGLE_BLOCK:
	case SDHI_SD_APP_SEND_SCR:
	case SDHI_SD_SWITCH: /* SD_SWITCH */
		ret = sdhi_single_read(host, data);
		break;
	default:
		printf(DRIVER_NAME": SD: NOT SUPPORT CMD = d'%04d\n", opc);
		ret = -EINVAL;
		break;
	}
	return ret;

}

static int sdhi_start_cmd(struct sdhi_host *host,
			struct mmc_data *data, struct mmc_cmd *cmd)
{
	long time;
	unsigned short opc = cmd->cmdidx;
	int ret = 0;

	pr_debug("opc = %d, arg = %x, resp_type = %x\n",
		 opc, cmd->cmdarg, cmd->resp_type);

	if (opc == MMC_CMD_STOP_TRANSMISSION) {
		/* SDHI sends the STOP command automatically by STOP reg */
		sdhi_writew(host, SDHI_INFO1_MASK, ~INFO1M_ACCESS_END &
				sdhi_readw(host, SDHI_INFO1_MASK));

		time = sdhi_wait_interrupt_flag(host);
		if (time == 0 || g_sd_error[host->ch] != 0)
			return sdhi_error_manage(host);

		sdhi_get_response(host, cmd);
		return 0;
	}

	if (host->data) {
		if ((opc == MMC_CMD_READ_MULTIPLE_BLOCK) ||
		     opc == MMC_CMD_WRITE_MULTIPLE_BLOCK) {
			sdhi_writew(host, SDHI_STOP, STOP_SEC_ENABLE);
			sdhi_writew(host, SDHI_SECCNT, data->blocks);
		}
		sdhi_writew(host, SDHI_SIZE, data->blocksize);
	}
	opc = sdhi_set_cmd(host, data, opc);

	/*
	 *  U-boot cannot use interrupt.
	 *  So this flag may not be clear by timing
	 */
	sdhi_writew(host, SDHI_INFO1, ~INFO1_RESP_END);

	sdhi_writew(host, SDHI_INFO1_MASK,
			INFO1M_RESP_END | sdhi_readw(host, SDHI_INFO1_MASK));
	sdhi_writew(host, SDHI_ARG0, (unsigned short)(cmd->cmdarg & ARG0_MASK));
	sdhi_writew(host, SDHI_ARG1,
			(unsigned short)((cmd->cmdarg >> 16) & ARG1_MASK));

	/* Waiting for SD Bus busy to be cleared */
	while ((sdhi_readw(host, SDHI_INFO2) & 0x2000) == 0)
		;

	sdhi_writew(host, SDHI_CMD, (unsigned short)(opc & CMD_MASK));

	g_wait_int[host->ch] = 0;
	sdhi_writew(host, SDHI_INFO1_MASK,
			~INFO1M_RESP_END & sdhi_readw(host, SDHI_INFO1_MASK));
	sdhi_writew(host, SDHI_INFO2_MASK,
			~(INFO2M_CMD_ERROR | INFO2M_CRC_ERROR |
			  INFO2M_END_ERROR | INFO2M_TIMEOUT   |
			  INFO2M_RESP_TIMEOUT | INFO2M_ILA)   &
			  sdhi_readw(host, SDHI_INFO2_MASK));

	time = sdhi_wait_interrupt_flag(host);
	if (time == 0)
		return sdhi_error_manage(host);

	if (g_sd_error[host->ch]) {
		switch (cmd->cmdidx) {
		case MMC_CMD_ALL_SEND_CID:
		case MMC_CMD_SELECT_CARD:
		case SD_CMD_SEND_IF_COND:
		case MMC_CMD_APP_CMD:
			ret = TIMEOUT;
			break;
		default:
			printf(DRIVER_NAME": Cmd(d'%d) err\n", opc);
			printf(DRIVER_NAME": cmdidx = %d\n", cmd->cmdidx);
			ret = sdhi_error_manage(host);
			break;
		}
		g_sd_error[host->ch] = 0;
		g_wait_int[host->ch] = 0;
		return ret;
	}
	if (sdhi_readw(host, SDHI_INFO1) & INFO1_RESP_END)
		return -EINVAL;

	if (g_wait_int[host->ch] == 1) {
		sdhi_get_response(host, cmd);
		g_wait_int[host->ch] = 0;
	}
	if (host->data)
		ret = sdhi_data_trans(host, data, opc);

	pr_debug("ret = %d, resp = %08x, %08x, %08x, %08x\n",
		 ret, cmd->response[0], cmd->response[1],
		 cmd->response[2], cmd->response[3]);
	return ret;
}

static int sdhi_request(struct mmc *mmc, struct mmc_cmd *cmd,
			struct mmc_data *data)
{
	struct sdhi_host *host = mmc_priv(mmc);
	int ret;

#if 0
	if (!(sdhi_readw(host, SDHI_INFO1) & INFO1_ISD0CD))
		return NO_CARD_ERR;
#endif

	g_sd_error[host->ch] = 0;

	host->data = data;
	ret = sdhi_start_cmd(host, data, cmd);
	host->data = NULL;

	return ret;
}

static void sdhi_set_ios(struct mmc *mmc)
{
	struct sdhi_host *host = mmc_priv(mmc);

	if (mmc->clock)
		host->clock = mmc->clock;

	switch (mmc->clock) {
	case CLKDEV_INIT:
	case CLKDEV_SD_DATA:
	case CLKDEV_HS_DATA:
	case CLKDEV_MMC_DATA:
		sdhi_clock_control(host, host->clock);
		break;
	case 0:
	default:
		sdhi_clock_control(host, mmc->clock);
		break;
	}

	if (mmc->bus_width == 4)
		sdhi_writew(host, SDHI_OPTION, ~OPT_BUS_WIDTH_1 &
					sdhi_readw(host, SDHI_OPTION));
	else
		sdhi_writew(host, SDHI_OPTION, OPT_BUS_WIDTH_1 |
					sdhi_readw(host, SDHI_OPTION));

	pr_debug("clock = %d, buswidth = %d\n", mmc->clock, mmc->bus_width);
}

static int sdhi_init(struct mmc *mmc)
{
	return 0;
}

int sdhi_mmc_init(unsigned long addr, int ch)
{
	int ret = 0;
	struct mmc *mmc;
	struct sdhi_host *host = NULL;

	mmc = malloc(sizeof(struct mmc));
	if (!mmc)
		return -ENOMEM;
	host = malloc(sizeof(struct sdhi_host));
	if (!host)
		return -ENOMEM;

	mmc->f_min = CLKDEV_INIT;
	mmc->f_max = CLKDEV_HS_DATA;
	mmc->voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
	mmc->host_caps = MMC_MODE_4BIT | MMC_MODE_HS;
	memcpy(mmc->name, DRIVER_NAME, sizeof(DRIVER_NAME));
	mmc->send_cmd = sdhi_request;
	mmc->set_ios = sdhi_set_ios;
	mmc->init = sdhi_init;
	host->ch = ch;
	host->addr = addr;
#if defined(LAGER_SDRAM_BASE)
	if (ch < 2)
		host->bus_shift = 1;
	else
		host->bus_shift = 0;
#elif defined(KOELSCH_SDRAM_BASE)
	if (ch == 0)
		host->bus_shift = 1;
	else
		host->bus_shift = 0;
#else
#error
#endif
	mmc->priv = host;

	sdhi_sync_reset(host);
	sdhi_writew(host, SDHI_PORTSEL, USE_1PORT);

#if defined(__BIG_ENDIAN_BITFIELD)
	sdhi_writew(host, SDHI_EXT_SWAP, SET_SWAP);
#endif

	mmc_register(mmc);

	sdhi_writew(host, SDHI_INFO1_MASK, INFO1M_RESP_END | INFO1M_ACCESS_END
			| INFO1M_CARD_RE | INFO1M_DATA3_CARD_RE
			| INFO1M_DATA3_CARD_IN);
	return ret;
}

