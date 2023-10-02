// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <command.h>
#include <cpu_func.h>
#include <linux/delay.h>

int do_reset(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	printf("resetting ...\n");

#ifdef CONFIG_ARCH_RZMPU
	mdelay(50);                             /* wait 50 ms */

	reset_cpu();
#else
	printf("reset not supported yet\n");
	hang();
#endif

	return 0;
}
