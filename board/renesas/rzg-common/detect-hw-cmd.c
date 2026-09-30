// SPDX-License-Identifier: GPL-2.0+
/*
 * detect-hw-cmd.c - Detect SoC type and board revision,
 *                   set soc_id and board_rev environment variables.
 *
 * Unified for HiHope RZ/G2[HMN] and SiLinux EK874 (RZ/G2E) boards.
 *
 * Called from CONFIG_BOOTCOMMAND after 'env default -a' to ensure
 * soc_id and board_rev are always set from real hardware regardless
 * of environment reset.
 *
 * Copyright (C) 2026 Renesas Electronics Corporation
 */

#include <common.h>
#include <command.h>
#include <env.h>
#include <stdio.h>
#include <linux/io.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/arch/rmobile.h>

/* Hihope (RZ/G2H,M,N) board revision detection GPIOs */
#define GPIO_HIHOPE_REV_BIT1		113	/* GP5_19 */
#define GPIO_HIHOPE_REV_BIT0		115	/* GP5_21 */
#define GPIO_HIHOPE_REV2_BOARD_CHECK	119	/* GP5_25 */

/* EK874 (RZ/G2E) board revision detection registers */
#define PFC_PMMR	0xE6060000
#define PFC_PUEN4	0xE6060410
#define PFC_PUD4	0xE6060450
#define PFC_GPSR5	0xE6060114
#define GPIO_IOINTSEL5	0xE6055000
#define GPIO_INOUTSEL5	0xE6055004
#define GPIO_INDT5	0xE605500C
#define MSTP907_GPIO5	BIT(7)

/**
 * rzg2_get_board_rev() - re-detect board revision from GPIO/registers.
 *
 * For HiHope boards returns an integer (2, 3, 4, ...).
 * For EK874 returns an ASCII character value ('B', 'C', or 'E').
 */
static int rzg2_get_board_rev(void)
{
	int ret;
	u32 reg;

	switch (rmobile_get_cpu_type()) {
	case RMOBILE_CPU_TYPE_R8A7795:   /* RZ/G2H */
		ret = 4;
		break;
	case RMOBILE_CPU_TYPE_R8A7796:   /* RZ/G2M */
		if ((rmobile_get_cpu_rev_integer() == 1) &&
		    (rmobile_get_cpu_rev_fraction() < 3)) {
			ret = 2;
		} else {
			gpio_request(GPIO_HIHOPE_REV_BIT0, "hihope_rev_bit0");
			gpio_request(GPIO_HIHOPE_REV_BIT1, "hihope_rev_bit1");
			gpio_direction_input(GPIO_HIHOPE_REV_BIT1);
			gpio_direction_input(GPIO_HIHOPE_REV_BIT0);
			ret = 0x03 +
			      ((gpio_get_value(GPIO_HIHOPE_REV_BIT1) << 1) |
				gpio_get_value(GPIO_HIHOPE_REV_BIT0));
		}
		break;
	case RMOBILE_CPU_TYPE_R8A77965:  /* RZ/G2N */
		gpio_request(GPIO_HIHOPE_REV2_BOARD_CHECK, "hihope_rev2_check");
		gpio_direction_input(GPIO_HIHOPE_REV2_BOARD_CHECK);
		if (gpio_get_value(GPIO_HIHOPE_REV2_BOARD_CHECK)) {
			ret = 2;
		} else {
			gpio_request(GPIO_HIHOPE_REV_BIT0, "hihope_rev_bit0");
			gpio_request(GPIO_HIHOPE_REV_BIT1, "hihope_rev_bit1");
			gpio_direction_input(GPIO_HIHOPE_REV_BIT1);
			gpio_direction_input(GPIO_HIHOPE_REV_BIT0);
			ret = 0x03 +
			      ((gpio_get_value(GPIO_HIHOPE_REV_BIT1) << 1) |
				gpio_get_value(GPIO_HIHOPE_REV_BIT0));
		}
		break;
	case RMOBILE_CPU_TYPE_R8A77990: /* RZ/G2E - EK874 */
		/*
		 * Mirrors check_rev() in ek874.c exactly.
		 * Returns ASCII character: 'B', 'C', or 'E'.
		 */
		if ((rmobile_get_cpu_rev_integer() == 1) &&
		   (rmobile_get_cpu_rev_fraction() < 1)) {
			ret = 'B';
		} else {
			/* Switch GP5_19 to GPIO input mode */
			reg = readl(PFC_GPSR5) & BIT(19);
			if (reg) {
				reg = readl(PFC_GPSR5) & ~BIT(19);
				writel(~reg, PFC_PMMR);
				writel(reg, PFC_GPSR5);
			}
			/* Enable clock to GPIO5 module if not already on */
			reg = readl(MSTPSR9) & MSTP907_GPIO5;
			if (reg)
				writel(readl(SMSTPCR9) &
				       ~MSTP907_GPIO5, SMSTPCR9);
			/* Enable pull up/down, set pull up */
			writel(readl(PFC_PUEN4) | BIT(17), PFC_PUEN4);
			writel(readl(PFC_PUD4)  | BIT(17), PFC_PUD4);

			/* Set GP5_19 as general input */
			writel(readl(GPIO_IOINTSEL5) & ~BIT(19),
				GPIO_IOINTSEL5);
			writel(readl(GPIO_INOUTSEL5) & ~BIT(19),
				GPIO_INOUTSEL5);
			/* Read GP5_19 value */
			ret = readl(GPIO_INDT5) & BIT(19);

			/* Restore: pull down, disable pull up/down */
			writel(readl(PFC_PUD4)  & BIT(17), PFC_PUD4);
			writel(readl(PFC_PUEN4) & BIT(17), PFC_PUEN4);

			ret = (ret ? 'C' : 'E');
		}
		break;
	default:
		ret = -1;
		break;
	}
	return ret;
}

static int rzg2_set_dtb_name(const char *soc_id, int board_rev)
{
	const char *dtb_name;

	if (!strcmp(soc_id, "r8a774a1")) {
		if (board_rev == 2)
			dtb_name =
				"r8a774a1-hihope-rzg2m-rev2-ex.dtb";
		else
			dtb_name =
				"r8a774a1-hihope-rzg2m-ex.dtb";
	} else if (!strcmp(soc_id, "r8a774a3")) {
		dtb_name = "r8a774a3-hihope-rzg2m-ex.dtb";
	} else if (!strcmp(soc_id, "r8a774b1")) {
		if (board_rev == 2)
			dtb_name =
				"r8a774b1-hihope-rzg2n-rev2-ex.dtb";
		else
			dtb_name =
				"r8a774b1-hihope-rzg2n-ex.dtb";
	} else if (!strcmp(soc_id, "r8a774e1")) {
		dtb_name = "r8a774e1-hihope-rzg2h-ex.dtb";
	} else if (!strcmp(soc_id, "r8a774c0")) {
		if (board_rev == 'E')
			dtb_name = "r8a774c0-ek874.dtb";
		else
			dtb_name = "r8a774c0-ek874-revc.dtb";
	} else {
		printf("WARNING: Cannot select DTB for SoC %s\n",
		       soc_id);
		env_set("dtb_name", NULL);
		return -EINVAL;
	}

	env_set("dtb_name", dtb_name);

	return 0;
}

static int do_detect_hw(struct cmd_tbl *cmdtp, int flag,
			int argc, char *const argv[])
{
	char board_rev_str[4];
	const char *soc_id;
	u8 cpu_type = rmobile_get_cpu_type();
	int cpu_rev_int = rmobile_get_cpu_rev_integer();
	int board_rev;
	int ret;

	/* Detect SoC ID */
	switch (cpu_type) {
	case RMOBILE_CPU_TYPE_R8A7796:   /* RZ/G2M */
		/*
		 * R8A7796 covers both r8a774a1 (cut < 3.0) and
		 * r8a774a3 (cut >= 3.0).
		 */
		if (cpu_rev_int >= 3)
			soc_id = "r8a774a3";
		else
			soc_id = "r8a774a1";
		break;

	case RMOBILE_CPU_TYPE_R8A77965:  /* RZ/G2N */
		soc_id = "r8a774b1";
		break;

	case RMOBILE_CPU_TYPE_R8A7795:   /* RZ/G2H */
		soc_id = "r8a774e1";
		break;

	case RMOBILE_CPU_TYPE_R8A77990:  /* RZ/G2E - EK874 */
		soc_id = "r8a774c0";
		break;

	default:
		printf("WARNING: Unknown CPU type 0x%x\n",
		       (unsigned int)cpu_type);
		env_set("soc_id", "unknown");
		env_set("board_rev", NULL);
		env_set("dtb_name", NULL);

		return CMD_RET_FAILURE;
	}

	env_set("soc_id", soc_id);

	/* Detect board revision */
	board_rev = rzg2_get_board_rev();
	if (board_rev < 0) {
		printf("WARNING: Failed to detect board revision: %d\n",
		       board_rev);
		env_set("board_rev", NULL);
		env_set("dtb_name", NULL);

		return CMD_RET_FAILURE;
	}

	if (cpu_type == RMOBILE_CPU_TYPE_R8A77990) {
		/*
		 * EK874 revision is represented by an ASCII character:
		 * 'B', 'C', or 'E'.
		 */
		board_rev_str[0] = (char)board_rev;
		board_rev_str[1] = '\0';
	} else {
		/*
		 * Current HiHope board revisions are single decimal
		 * digits.
		 */
		snprintf(board_rev_str, sizeof(board_rev_str),
			 "%d", board_rev);
	}

	env_set("board_rev", board_rev_str);

	/* Select DTB from detected SoC and board revision */
	ret = rzg2_set_dtb_name(soc_id, board_rev);
	if (ret)
		return CMD_RET_FAILURE;

	printf("detect_hw: soc_id=%s board_rev=%s dtb_name=%s\n",
		env_get("soc_id"),
		env_get("board_rev"),
		env_get("dtb_name"));

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	detect_hw, 1, 1, do_detect_hw,
	"Detect SoC and board revision, then select the DTB",
	""
);
