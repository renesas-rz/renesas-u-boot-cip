// SPDX-License-Identifier: GPL-2.0+

#include <command.h>
#include <env.h>
#include <asm/arch/renesas.h>

static int set_platform(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	const u32 cpu_type = renesas_get_cpu_type();

	if (cpu_type == RENESAS_CPU_TYPE_RZT2H) {
		env_set("platform", "r9a09g077");
		printf("platform=r9a09g077\n");
	} else if (cpu_type == RENESAS_CPU_TYPE_RZN2H) {
		env_set("platform", "r9a09g087");
		printf("platform=r9a09g087\n");
	} else {
		env_set("platform", "unknown");
		printf("platform=unknown\n");
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
    rz_platform,    /* command name */
    1,              /* maximum number of arguments */
    1,              /* command is repeatable */
    set_platform,   /* implementation */
    "Identify and set plaform=r9a09g077 for T2H plaform=r9a09g087 for N2H",
    "[argument]\n"
    "    - Not required"
);