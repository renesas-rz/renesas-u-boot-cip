// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021, Renesas Electronics Corporation. All rights reserved.
 */

// include
#include <common.h>
#include <renesas/rzf-dev/rzf-dev_def.h>
#include "ddr_internal.h"
#include <linux/delay.h>
#include <asm/io.h>

#define	CEIL(a, div)	(((a) + ((div) - 1)) / (div))
#define	_MIN(a, b)		((a) < (b) ? (a) : (b))
#define	_MAX(a, b)		((a) > (b) ? (a) : (b))

#define	MAX_BYTE_LANES		(2U)
#define	MAX_BEST_VREF_SAVED	(30U)
#define	VREF_SETP			(1U)

extern const uint32_t mc_init_tbl[MC_INIT_NUM][2];
extern const uint32_t mc_odt_pins_tbl[4];
extern const uint32_t mc_mr1_tbl[2];
extern const uint32_t mc_mr2_tbl[2];
extern const uint32_t mc_mr5_tbl[2];
extern const uint32_t mc_mr6_tbl[2];
extern const uint32_t mc_phy_settings_tbl[MC_PHYSET_NUM][2];
extern const uint32_t swizzle_mc_tbl[SWIZZLE_MC_NUM][2];
extern const uint32_t swizzle_phy_tbl[SIZZLE_PHY_NUM][2];

// prototypes
void ddr_setup(void);
static void disable_phy_clk(void);
static void program_mc1(uint8_t *lp_auto_entry_en);
static void program_phy1(uint32_t sl_lanes, uint32_t byte_lanes);
static void exec_trainingWRLVL(uint32_t sl_lanes);
static void exec_trainingVREF(uint32_t sl_lanes, uint32_t byte_lanes);
static void setup_vref_training_registers(uint8_t vref_value, uint8_t cs, uint8_t turn_on_off_vref_training);
static void write_mr(uint8_t cs, uint8_t mrw_sel, uint16_t mrw_data);
static void exec_trainingBITLVL(uint32_t sl_lanes);
static void opt_delay(uint32_t sl_lanes, uint32_t byte_lanes);
static void exec_trainingSL(uint32_t sl_lanes);
static void program_phy2(void);
static void program_mc2(void);

extern void cpg_active_ddr(void (*disable_phy)(void));
extern void cpg_reset_ddr_mc(void);

#ifdef CONFIG_RZF_DDR_ECC
#define DDR_BASE_ADDRESS	0x40000000
#define TEST_BIT_NO			0
#define USER_WORD_DATA		0x5a5a5a5a5a5a5a5a
#define ECC_ERR_ADDRESS		0x60000000
static int force_ecc_ce_error(void);
static void ecc_prog_all0(uint64_t addr_start, uint64_t addr_end);
static void init_ecc(void);
char data_synd[] = {
			0xf4, 0xf1, 0xec, 0xea, 0xe9, 0xe6, 0xe5, 0xe3,
			0xdc, 0xda, 0xd9, 0xd6, 0xd5, 0xd3, 0xce, 0xcb,
			0xb5, 0xb0, 0xad, 0xab, 0xa8, 0xa7, 0xa4, 0xa2,
			0x9d, 0x9b, 0x98, 0x97, 0x94, 0x92, 0x8f, 0x8a,
			0x75, 0x70, 0x6d, 0x6b, 0x68, 0x67, 0x64, 0x62,
			0x5e, 0x5b, 0x58, 0x57, 0x54, 0x52, 0x4f, 0x4a,
			0x34, 0x31, 0x2c, 0x2a, 0x29, 0x26, 0x25, 0x23,
			0x1c, 0x1a, 0x19, 0x16, 0x15, 0x13, 0x0e, 0x0b
		  };
#endif

// main
void ddr_setup(void)
{
	uint32_t	sl_lanes, byte_lanes;
	uint8_t		runBITLVL, runSL, runVREF;
	uint8_t		lp_auto_entry_en = 0;
	uint32_t	tmp;
	int i;

	// Step2 - Step11
	cpg_active_ddr(disable_phy_clk);

	// Step12
	program_mc1(&lp_auto_entry_en);

	// Step13
	tmp = read_mc_reg(DDRMC_R019);
	sl_lanes	= ((tmp & 0x1) == 0) ? 3 : 1;
	byte_lanes	= ((tmp & 0x1) == 0) ? 2 : 1;
	tmp = read_mc_reg(DDRMC_R039);
	runBITLVL	= (tmp >> 20) & 0x1;
	runSL		= (tmp >> 21) & 0x1;
	runVREF		= (tmp >> 25) & 0x1;

	// Step14
	program_phy1(sl_lanes, byte_lanes);

	// Step15
	while ((read_phy_reg(DDRPHY_R42) & 0x00000003) != sl_lanes)
		;

	// Step16
	ddr_ctrl_reten_en_n(0);
	rmw_mc_reg(DDRMC_R007, 0xFFFFFEFF, 0x00000000);
	rmw_mc_reg(DDRMC_R001, 0xFEFFFFFF, 0x01000000);
	rmw_mc_reg(DDRMC_R000, 0xFFFFFFFE, 0x00000001);
	while ((read_mc_reg(DDRMC_R021) & 0x02000000) != 0x02000000)
		;
	rmw_phy_reg(DDRPHY_R74, 0xFFF7FFFF, 0x00080000);
	rmw_mc_reg(DDRMC_R029, 0xFF0000FF, 64 << 8);
	rmw_mc_reg(DDRMC_R027, 0xE00000FF, 111 << 8);
	rmw_mc_reg(DDRMC_R020, 0xFFFFFEFF, 0x00000100);
	udelay(1);
	rmw_phy_reg(DDRPHY_R74, 0xFFF7FFFF, 0x00000000);

	// Step17
	cpg_reset_ddr_mc();
	ddr_ctrl_reten_en_n(1);

	// Step18-19
	program_mc1(&lp_auto_entry_en);

	// Step20
	for (i = 0; i < ARRAY_SIZE(swizzle_mc_tbl); i++) {
		write_mc_reg(swizzle_mc_tbl[i][0], swizzle_mc_tbl[i][1]);
	}
	for (i = 0; i < ARRAY_SIZE(swizzle_phy_tbl); i++) {
		write_phy_reg(swizzle_phy_tbl[i][0], swizzle_phy_tbl[i][1]);
	}

	// Step21
	rmw_mc_reg(DDRMC_R000, 0xFFFFFFFE, 0x00000001);

	// Step22
	while ((read_mc_reg(DDRMC_R021) & 0x02000000) != 0x02000000)
		;

	// Step23
	rmw_mc_reg(DDRMC_R023, 0xFDFFFFFF, 0x02000000);

	// Step24
	exec_trainingWRLVL(sl_lanes);

	// Step25
	if (runVREF == 1)
		exec_trainingVREF(sl_lanes, byte_lanes);

	// Step26
	if (runBITLVL == 1)
		exec_trainingBITLVL(sl_lanes);

	// Step27
	opt_delay(sl_lanes, byte_lanes);

	// Step28
	if (runSL == 1)
		exec_trainingSL(sl_lanes);

	// Step29
	program_phy2();

	// Step30
	program_mc2();

	// Step31 is skipped because ECC is unused.
#ifdef CONFIG_RZF_DDR_ECC
	printf("NOTICE:  BL2: ECC MODE: ");
#ifdef CONFIG_RZF_DDR_ECC_DETECT_CORRECT)
	printf(" Error Detect and Correct\n");
#elif CONFIG_RZF_DDR_ECC_DETECT
	printf(" Error Detect\n");
#else
	printf(" Enable\n");
#endif
	printf("NOTICE:  BL2: ECC function initializing... ");
	tmp = read_mc_reg(DENALI_CTL_94);
	tmp |= (1 << 16);
	write_mc_reg(DENALI_CTL_94, tmp);
	mdelay(10);

	init_ecc();

	tmp = read_mc_reg(DENALI_CTL_94);
	tmp &= ~(1 << 16);
	write_mc_reg(DENALI_CTL_94, tmp);
	mdelay(10);

	// Step32
	// let the auto_exit_en to be value|0x8
	// recommended value is 0x0
	tmp = read_mc_reg(DENALI_CTL_60);
	tmp |= (0x8 << 8);
	write_mc_reg(DENALI_CTL_60, tmp);

	printf("DONE\n");
#endif

	// Step32
	rmw_mc_reg(DDRMC_R006, 0xFFFFFFF0, lp_auto_entry_en & 0xF);

#ifdef CONFIG_RZF_DDR_ECC
	// Extra step, test ECC CE function
	printf("NOTICE:  BL2: ECC CE function testing .... ");
	if (force_ecc_ce_error())
		printf("FAILED\n");
	else
		printf("OK\n");
#endif
}

static void disable_phy_clk(void)
{
	// Initialization Step9
	write_phy_reg(DDRPHY_R77, 0x00000200);
	write_phy_reg(DDRPHY_R78, 0x00010001);
}

#ifdef CONFIG_RZF_DDR_ECC
#ifdef CONFIG_RZF_DDR_ECC_DETECT_CORRECT
extern void flush_dcache_range(unsigned long start, unsigned long end);
static int force_ecc_ce_error(void)
{
	int ret;
	uint64_t err_addr;
	uint64_t *user_word;
	uint32_t tmp, synd, xor_check_code;
	uint32_t retry = 0xffffffU;
	uint32_t bak_DENALI_CTL_94;

	bak_DENALI_CTL_94 = read_mc_reg(DENALI_CTL_94);

	// make checkcode
	xor_check_code = (data_synd[0] << 8) | 1;
	user_word = (uint64_t*)ECC_ERR_ADDRESS;

	do{
		tmp = read_mc_reg(DENALI_CTL_134);
	} while(tmp & 1);

	tmp = read_mc_reg(DENALI_CTL_94);
	tmp &= ~((0xff << 8) | 1);
	tmp |= xor_check_code;
	write_mc_reg(DENALI_CTL_94, tmp);

	*user_word = USER_WORD_DATA;
	flush_dcache_range((unsigned long)user_word, sizeof(uint64_t));
	(*(const volatile uint64_t*)user_word);

	tmp = read_mc_reg(DENALI_CTL_94);
	tmp &= ~((0xff << 8) | 1);
	write_mc_reg(DENALI_CTL_94, tmp);

	do {
		tmp = read_mc_reg(DENALI_CTL_141);
	} while((retry--) && !(tmp & 0xffffU));

	// out of retry or not CE
	if (!retry || !(tmp & 0x3)) {
		ret = -1;
		goto err;
	}

	// ack ecc int
	tmp = 0xffffU;
	write_mc_reg(DENALI_CTL_149, tmp);

	// check the error address
	err_addr = (uint64_t)(read_mc_reg(DENALI_CTL_102) & 0x3);
	err_addr = err_addr << 32;
	err_addr |= (uint64_t)read_mc_reg(DENALI_CTL_101);
	err_addr += DDR_BASE_ADDRESS;
	if (((uint64_t)user_word) != err_addr){
		ret = -1;
		goto err;
	}

	// check error synd
	synd = read_mc_reg(DENALI_CTL_102);
	synd = (synd >> 8) & 0xff;
	if (synd != (xor_check_code >> 8)){
		ret = -1;
		goto err;
	}

	if ((*user_word ^ USER_WORD_DATA) != BIT(TEST_BIT_NO)) {
		ret = -1;
		goto err;
	}

	write_mc_reg(DENALI_CTL_94, bak_DENALI_CTL_94);

	return 0;

err:
	write_mc_reg(DENALI_CTL_94, bak_DENALI_CTL_94);

	return ret;
}
#else
static int force_ecc_ce_error(void) {return 0;}
#endif

static void ecc_prog_all0(uint64_t addr_start, uint64_t addr_end)
{
	int i;
	uint32_t val;
	uint64_t addr, prog_size;
	uint32_t bak_lp_auto_entry_en, bak_in_order_accept;

	// 1
	addr = addr_start - DDR_BASE_ADDRESS;
	prog_size = addr_end - addr_start + 1;

	// 2
	val = read_mc_reg(DENALI_CTL_60);
	bak_lp_auto_entry_en = val & 0xf;
	val &= ~0xf;
	write_mc_reg(DENALI_CTL_60, val);

	val = read_mc_reg(DENALI_CTL_133);
	bak_in_order_accept = val & (1 << 16);
	val |= (1 << 16);
	write_mc_reg(DENALI_CTL_133, val);

	// 3
	val = read_mc_reg(DENALI_CTL_85);
	val |= (1 << 16); // set BIST_DATA_CHECK
	val &= ~(1 << 24);// unset BIST_ADDR_CHECK
	write_mc_reg(DENALI_CTL_85, val);

	val = read_mc_reg(DENALI_CTL_89);
	val &= ~0x7;
	val |= 0x4;
	write_mc_reg(DENALI_CTL_89, val);

	val = 0;
	write_mc_reg(DENALI_CTL_90, val);
	write_mc_reg(DENALI_CTL_91, val);

	val = read_mc_reg(DENALI_CTL_161);
	val &= ~(0xff << 16);
	val |= (1 << 16);
	write_mc_reg(DENALI_CTL_161, val);

	// 4
	for (i = 0 ; i < 34 ; i++) {
		if (((prog_size >> i) & 1) == 1) {
			val = addr & 0xffffffff;
			write_mc_reg(DENALI_CTL_86, val);

			val = (addr >> 32) & 0x3;
			write_mc_reg(DENALI_CTL_87, val);

			val = read_mc_reg(DENALI_CTL_85);
			val &= ~(0x3f << 8);
			val |= (i << 8);
			write_mc_reg(DENALI_CTL_85, val);

			mdelay(10);

			// bit_go=1
			val = read_mc_reg(DENALI_CTL_84);
			val |= (1 << 24);
			write_mc_reg(DENALI_CTL_84, val);

			do {
				val = read_mc_reg(DENALI_CTL_145);
			}while(!(val & (1 << 16)));

			// bit_go=0
			val = read_mc_reg(DENALI_CTL_84);
			val &= ~(1 << 24);
			write_mc_reg(DENALI_CTL_84, val);

			val = read_mc_reg(DENALI_CTL_153);
			val |= (1 << 16);
			write_mc_reg(DENALI_CTL_153, val);

			do {
				val = read_mc_reg(DENALI_CTL_145);
			}while(val & (1 << 16));

			addr += (1 << i);
		}
	}

	// 5
	// ack ecc int
	val = 0xffffU;
	write_mc_reg(DENALI_CTL_149, val);

	// wait unitl the ecc int clear
	do{
		val = read_mc_reg(DENALI_CTL_141);
	}while(val & 0xffffU);

	// 6
	val = read_mc_reg(DENALI_CTL_161);
	val &= ~(0xff << 16);
	write_mc_reg(DENALI_CTL_161, val);

	val = read_mc_reg(DENALI_CTL_85);
	val &= ~(1 << 16); // unset BIST_DATA_CHECK
	write_mc_reg(DENALI_CTL_85, val);

	// 7
	val = read_mc_reg(DENALI_CTL_60);
	val &= ~0xf;
	val |= bak_lp_auto_entry_en;
	write_mc_reg(DENALI_CTL_60, val);

	val = read_mc_reg(DENALI_CTL_133);
	val &= ~(1 << 16);
	val |= bak_in_order_accept;
	write_mc_reg(DENALI_CTL_133, val);
}

// follow DDRTOP_ApplicationNote_Rev01.14.excel
// capter SubProc->Init0_ECC
static void init_ecc(void)
{
	uint64_t addr_start, addr_end;
	uint32_t cs_val_upper, val;

	// 1. check DDR3/DDR3L/DDR4
	val = read_mc_reg(DENALI_CTL_413);
	val = (val >> 16) & 0xF;
	if (val == 2)
		addr_start = DDR_BASE_ADDRESS + 0x40;
	else
		addr_start = DDR_BASE_ADDRESS + 0x20;

	// 2
	cs_val_upper = read_mc_reg(DENALI_CTL_124);
	cs_val_upper = (cs_val_upper >> 16) & 0xffffU;
	val = read_mc_reg(DENALI_CTL_132);
	val = (val >> 16) & 0x3;
	if (val == 0x3) {
		val = read_mc_reg(DENALI_CTL_126);
		val = (val >> 16) & 0xffffU;
		if (val > cs_val_upper) {
			cs_val_upper = val;
		}
	}
	addr_end = (uint64_t)(((cs_val_upper + 1) << 18) - 1) + DDR_BASE_ADDRESS;

	// 3
	// ECC_DISABLE_W_UC_ERR <= 1
	val = read_mc_reg(DENALI_CTL_94);
	val |= (1 << 16);
	write_mc_reg(DENALI_CTL_94, val);

	// mask ECC interrupt
	val = read_mc_reg(DENALI_CTL_157);
	val &= ~0xffffU;
	val |= 0x1CF;
	write_mc_reg(DENALI_CTL_157, val);

	// 4. wait for 10 regACLK
	mdelay(10);

	// 5.prog_all0
	ecc_prog_all0(addr_start, addr_end);

	// 6.
	// unmask ECC interrupt
	val = read_mc_reg(DENALI_CTL_157);
	val &= ~0xffffU;
	write_mc_reg(DENALI_CTL_157, val);

	// ack ECC interrupt
	val = read_mc_reg(DENALI_CTL_149);
	val |= 0x1CF;
	write_mc_reg(DENALI_CTL_149, val);

	// ECC_DISABLE_W_UC_ERR = 0
	val = read_mc_reg(DENALI_CTL_94);
	val &= ~(1 << 16);
	write_mc_reg(DENALI_CTL_94, val);
	mdelay(10);
}

static void program_mc1_ecc_en(void)
{
	uint32_t tmp;
	uint16_t addr_diff[2], cs_size[2], cs_val_lower[2], row_start_val[2];
	int i, maxrow_cs;

	tmp = read_mc_reg(DENALI_CTL_93);
	tmp &= ~(0x3 << 24);
#ifdef CONFIG_RZF_DDR_ECC_DETECT_CORRECT
	tmp |= (0x3 << 24);
#elif CONFIG_RZF_DDR_ECC_DETECT
	tmp |= (0x2 << 24);
#else
	tmp |= (1 << 24);
#endif
	write_mc_reg(DENALI_CTL_93, tmp);

	tmp = read_mc_reg(DENALI_CTL_129);
	tmp &= ~(0x1 << 8);
	tmp |= (0x1 << 8);
	write_mc_reg(DENALI_CTL_129, tmp);

	tmp = read_mc_reg(DENALI_CTL_132);
	tmp &= ~0x1;
	write_mc_reg(DENALI_CTL_132, tmp);

	tmp = read_mc_reg(DENALI_CTL_133);
	tmp &= ~(0x1 << 16);
	tmp |= (0x1 << 16);
	write_mc_reg(DENALI_CTL_133, tmp);

	tmp = read_mc_reg(DENALI_CTL_131);
	tmp &= ~(0x1 << 24);
	write_mc_reg(DENALI_CTL_131, tmp);

	tmp = read_mc_reg(DENALI_CTL_122);
	addr_diff[0] = (uint16_t)((tmp >> 8) & 0x3);
	tmp = read_mc_reg(DENALI_CTL_123);
	addr_diff[0] += (uint16_t)((tmp >> 8) & 0xf);
	tmp = read_mc_reg(DENALI_CTL_122);
	addr_diff[0] += (uint16_t)((tmp >> 24) & 0x7);

	tmp = read_mc_reg(DENALI_CTL_122);
	addr_diff[1] = (uint16_t)((tmp >> 16) & 0x3);
	tmp = read_mc_reg(DENALI_CTL_123);
	addr_diff[1] += (uint16_t)((tmp>> 16) & 0xf);
	tmp = read_mc_reg(DENALI_CTL_123);
	addr_diff[1] += (uint16_t)(tmp & 0x7);

	for (i = 0; i < 2; i++) {
		cs_size[i] = (0xDFFF >> 1) >> addr_diff[i];
	}
	if (cs_size[0] >= cs_size[1]) {
		maxrow_cs = 0;
	} else {
		maxrow_cs = 1;
	}

	cs_val_lower[maxrow_cs] = 0x0000;
	cs_val_lower[(maxrow_cs + 1) % 2] = cs_val_lower[maxrow_cs] + cs_size[maxrow_cs] + 1;

	tmp = read_mc_reg(DENALI_CTL_124);
	tmp &= ~0xffffU;
	tmp |= (cs_val_lower[0] & 0xffffU);
	write_mc_reg(DENALI_CTL_124, tmp);

	tmp = read_mc_reg(DENALI_CTL_124);
	tmp &= ~(0xffffU << 16);
	tmp |= ((cs_val_lower[0] + cs_size[0]) << 16);
	write_mc_reg(DENALI_CTL_124, tmp);

	tmp = read_mc_reg(DENALI_CTL_132);
	tmp = (tmp >> 16) & 0x3;
	if (tmp == 0x3) {
		tmp = read_mc_reg(DENALI_CTL_126);
		tmp &= ~(0xffffU);
		tmp |= (cs_val_lower[1] & 0xffffU);
		write_mc_reg(DENALI_CTL_126, tmp);

		tmp = read_mc_reg(DENALI_CTL_126);
		tmp &= ~(0xffffU << 16);
		tmp |= ((cs_val_lower[1] + cs_size[1]) << 16);
		write_mc_reg(DENALI_CTL_126, tmp);
	}
	row_start_val[maxrow_cs] = 0x0;
	i = (maxrow_cs + 1) % 2;
	if (cs_size[i] == cs_size[maxrow_cs]) {
		row_start_val[i] = 0x1;
	} else if (cs_size[i] == (cs_size[maxrow_cs] >> 1)) {
		row_start_val[i] = 0x2;
	} else if (cs_size[i] == (cs_size[maxrow_cs] >> 2)) {
		row_start_val[i] = 0x4;
	} else {
		row_start_val[i] = 0x0;
	}

	tmp = read_mc_reg(DENALI_CTL_125);
	tmp &= ~0x7;
	tmp |= (row_start_val[0] & 0x7);
	write_mc_reg(DENALI_CTL_125, tmp);

	tmp = read_mc_reg(DENALI_CTL_127);
	tmp &= ~0x7;
	tmp |= (row_start_val[1] & 0x7);
	write_mc_reg(DENALI_CTL_127, tmp);
}
#endif

static void program_mc1(uint8_t *lp_auto_entry_en)
{
	int i;

	// Step1
	for (i = 0; i < ARRAY_SIZE(mc_init_tbl); i++) {
		if (mc_init_tbl[i][0] == DDRMC_R006) {
			*lp_auto_entry_en = mc_init_tbl[i][1] & 0xF;
			write_mc_reg(DDRMC_R006, mc_init_tbl[i][1] & 0xFFFFFFF0);
		} else {
			write_mc_reg(mc_init_tbl[i][0], mc_init_tbl[i][1]);
		}
	}

	// Step2
	rmw_mc_reg(DDRMC_R025, 0xFCFFFFFF, mc_odt_pins_tbl[0] << 24);
	rmw_mc_reg(DDRMC_R026, 0xFFFFFCFF, mc_odt_pins_tbl[1] << 8);
	rmw_mc_reg(DDRMC_R025, 0xFFFCFFFF, mc_odt_pins_tbl[2] << 16);
	rmw_mc_reg(DDRMC_R026, 0xFFFFFFFC, mc_odt_pins_tbl[3] << 0);

	// Step3
	rmw_mc_reg(DDRMC_R009, ~(mc_mr1_tbl[0]), mc_mr1_tbl[1]);
	rmw_mc_reg(DDRMC_R011, ~(mc_mr1_tbl[0]), mc_mr1_tbl[1]);

	// Step4
	rmw_mc_reg(DDRMC_R010, ~(mc_mr2_tbl[0]), mc_mr2_tbl[1]);
	rmw_mc_reg(DDRMC_R012, ~(mc_mr2_tbl[0]), mc_mr2_tbl[1]);

	// Step5
	rmw_mc_reg(DDRMC_R015, ~(mc_mr5_tbl[0]), mc_mr5_tbl[1]);
	rmw_mc_reg(DDRMC_R016, ~(mc_mr5_tbl[0]), mc_mr5_tbl[1]);

	// Step6
	rmw_mc_reg(DDRMC_R017, ~(mc_mr6_tbl[0]), mc_mr6_tbl[1]);
	rmw_mc_reg(DDRMC_R018, ~(mc_mr6_tbl[0]), mc_mr6_tbl[1]);

	// Step7
	for (i = 0; i < ARRAY_SIZE(mc_phy_settings_tbl); i++) {
		write_mc_reg(mc_phy_settings_tbl[i][0], mc_phy_settings_tbl[i][1]);
	}

#ifdef CONFIG_RZF_DDR_ECC
	// Step8, Enable ECC if needed.
	program_mc1_ecc_en();
#endif
}

static void program_phy1(uint32_t sl_lanes, uint32_t byte_lanes)
{
	uint16_t dram_clk_period;
	uint8_t dram;
	uint8_t odt_wr_map_cs0, odt_rd_map_cs0;
	uint8_t CL, CWL, AL, PL, RL, WL;
	uint32_t mr1, mr1_wl, mr1_wl_mask;
	uint32_t mr2, mr2_wl, mr2_wl_mask;
	uint8_t clk_drive, dq_dqs_drive, dq_dqs_term, vref_value, vref_ca_value;
	uint8_t read_lat, trim_lat;
	uint32_t tmp;
	int i;

	// Step1
	tmp = read_mc_reg(DDRMC_R039);
	dram_clk_period = tmp & 0xFFFF;
	dram = (tmp >> 16) & 0xF;

	tmp = read_mc_reg(DDRMC_R025);
	odt_wr_map_cs0 = (tmp >> 24) & 0x3;
	odt_rd_map_cs0 = (tmp >> 16) & 0x3;

	// Step2
	tmp = read_mc_reg(DDRMC_R002);
	CL = (tmp >> 17) & 0x1F;
	CWL = (tmp >> 24) & 0x1F;

	tmp = read_mc_reg(DDRMC_R003);
	AL = tmp & 0x1F;
	PL = (tmp >> 8) & 0xF;

	RL = CL + AL + PL;
	WL = CWL + AL + PL;

	// Step3
	mr1 = read_mc_reg(DDRMC_R009) & 0xFFFF;
	mr2 = read_mc_reg(DDRMC_R010) & 0xFFFF;
	if (dram == 2) {
		// DDR4
		mr1_wl_mask = (0x7 << 8) | (0x1 << 7);	// 0x78
		mr2_wl_mask = 0x7 << 9;					// 0xe0
		switch ((mr2 & mr2_wl_mask) >> 9) {
		case 0:
			mr1_wl = 0x0;
			mr1_wl_mask = 0x0000;
			break;
		case 1:
			mr1_wl = 0x2 << 8;
			break;
		case 2:
			mr1_wl = 0x4 << 8;
			break;
		case 4:
			mr1_wl = 0x6 << 8;
			break;
		default:
			panic("PANIC");
		}
	} else {
		// DDR3L, DDR3
		mr1_wl_mask = (0x1 << 9) | (0x1 << 7) | (0x1 << 6) | (0x1 << 2);
		mr2_wl_mask = 0x3 << 9;
		switch ((mr2 & mr2_wl_mask) >> 9) {
		case 0:
			mr1_wl = 0x0;
			mr1_wl_mask = 0x0000;
			break;
		case 1:
			mr1_wl = (0x0 << 9) | (0x0 << 6) | (0x1 << 2);
			break;
		case 2:
			mr1_wl = (0x0 << 9) | (0x1 << 6) | (0x0 << 2);
			break;
		default:
			panic("PANIC");
		}
	}
	mr1_wl |= (mr1 & (0xFFFF ^ mr1_wl_mask)) | (0x1 << 7);
	mr2_wl = (mr2 & (0xFFFF ^ mr2_wl_mask)) | (0x0 << 9);

	// Step4
	tmp = read_mc_reg(DDRMC_R040);
	clk_drive = tmp & 0xF;
	dq_dqs_drive = (tmp >> 4) & 0xF;
	dq_dqs_term = (tmp >> 8) & 0xF;
	vref_value = (tmp >> 16) & 0xFF;
	vref_ca_value = (tmp >> 24) & 0xFF;
	read_lat = (dram == 2 ? 12 : 10) + (CEIL(RL, 2) * 2) - CEIL(WL, 2) + 28;
	trim_lat = (dram == 2 ? 11 : 9) + CEIL(RL, 2) - CEIL(WL, 2) + 29;

	// Step5
	tmp = ((WL == 5 ? 0x1 : 0x0) << 16) | 0x00100000;
	write_phy_reg(DDRPHY_R77, tmp);

	// Step6
	write_phy_reg(DDRPHY_R05, 0x00000006);

	// Step7
	if (dram == 2) {
		// DDR4
		write_phy_reg(DDRPHY_R65, 0x00000009);
	}

	// Step8
	write_phy_reg(DDRPHY_R67, (dram == 2 ? 1 : 0) << 27);
	write_phy_reg(DDRPHY_R47, (dram == 2 ? 1 : 0) << 24);

	// Step9
	tmp = ((dram == 0 ? 0 : 1) << 15) | ((dram_clk_period < 1000 ? 1 : 0) << 8) | 0x10004000;
	write_phy_reg(DDRPHY_R26, tmp);

	// Step10
	write_phy_reg(DDRPHY_R13,
		clk_drive | (clk_drive << 4) | (clk_drive << 8) | (clk_drive<<12) |
		(dq_dqs_drive << 16) | (dq_dqs_drive << 20));

	// Step11
	tmp = dq_dqs_term | ((dram == 2) ? 0 : (dq_dqs_term << 4));
	write_phy_reg(DDRPHY_R14, tmp);

	// Step12
	write_phy_reg(DDRPHY_R10, 0x00000000);

	// Step13
	for (i = 0; i < byte_lanes; i++) {
		write_phy_reg(DDRPHY_R29, 7 * i);
		write_phy_reg(DDRPHY_R66, (vref_value << 4) | 0x00000004);
	}

	// Step14
	write_phy_reg(DDRPHY_R12, vref_ca_value);

	// Step15
	write_phy_reg(DDRPHY_R61, 0x1A09002D);

	// Step16
	write_phy_reg(DDRPHY_R41, 0x00000000);

	// Step17
	write_phy_reg(DDRPHY_R74, 0x0000001A);

	// Step18
	tmp = ((dram == 2 ? 0 : 1) << 2) | (CEIL(CL, 2) << 4) | (CEIL(AL, 2) << 8) |
			(odt_rd_map_cs0 << 16) | (odt_wr_map_cs0 << 24) | 0x00000001;
	write_phy_reg(DDRPHY_R24, tmp);

	// Step19
	tmp = (CEIL(CWL, 2) << 8) | ((((WL % 2) == 0) ? 0 : 1) << 24) | 0x80000001;
	write_phy_reg(DDRPHY_R25, tmp);

	// Step20
	write_phy_reg(DDRPHY_R45, sl_lanes ^ 0x3);

	// Step21
	write_phy_reg(DDRPHY_R64, (trim_lat << 4) | (read_lat << 12));

	// Step22
	tmp = ((WL % 2) == 0) & 0x1;
	write_phy_reg(DDRPHY_R63, tmp);

	// Step23
	write_phy_reg(DDRPHY_R72, 0x00000170);

	// Step24
	write_phy_reg(DDRPHY_R38, mr2_wl | (mr2 << 16));

	// Step25
	write_phy_reg(DDRPHY_R39, mr1 | (mr1_wl << 16));

	// Step26
	write_phy_reg(DDRPHY_R27, 0xAC001000);

	// Step27
	udelay(10);

	// Step28 is skipped because dll_mas_dly is unused.

	// Step29
	write_phy_reg(DDRPHY_R44, 0x00000000);

	// Step30
	write_phy_reg(DDRPHY_R43, 0x00000000);

	// Step31
	for (i = 0; i < byte_lanes; i++) {
		write_phy_reg(DDRPHY_R29, 6 * i);
		write_phy_reg(DDRPHY_R30, 9);
		write_phy_reg(DDRPHY_R32, 10);
	}

	// Step32
	write_phy_reg(DDRPHY_R28, 26 | 0x00000200);

	// Step33
	write_phy_reg(DDRPHY_R29, 0);
	write_phy_reg(DDRPHY_R57, 26 | 0x00000080);

	// Step34
	write_phy_reg(DDRPHY_R27, 26 | (0x10 << 8) | 0xAC000000);

	// Step35
	write_phy_reg(DDRPHY_R21, 0x00035076);

	// Step36
	write_phy_reg(DDRPHY_R07, 0x00000032);

	// lpddr4_combo_io_cal
	{
		// Step37-2
		rmw_phy_reg(DDRPHY_R47, 0xFF800000, 0x00000000);

		// Step37-3
		switch (dram) {
		case 0:
			tmp = 0x00003200;
			break;
		case 1:
			tmp = 0x00005200;
			break;
		case 2:
			tmp = 0x08009200;
			break;
		default:
			tmp = 0x00000000;
			break;
		}
		write_phy_reg(DDRPHY_R67, tmp);
		write_phy_reg(DDRPHY_R46, 0x00000002);
		while ((read_phy_reg(DDRPHY_R46) & 0x00000008) != 0x00000008)
			;

		write_phy_reg(DDRPHY_R46, 0x00000000);
		udelay(100);

		// Step37-4
		switch (dram) {
		case 0:
		case 1:
			tmp = 0x00041200;
			break;
		case 2:
			tmp = 0x08101300;
			break;
		default:
			tmp = 0x00000000;
			break;
		}
		write_phy_reg(DDRPHY_R67, tmp);
		write_phy_reg(DDRPHY_R46, 0x00000001);
		while ((read_phy_reg(DDRPHY_R46) & 0x00000004) != 0x00000004)
			;

		rmw_phy_reg(DDRPHY_R46, 0xFFFFFFEF, 0x00000010);
		rmw_phy_reg(DDRPHY_R46, 0xFFFFFFEF, 0x00000000);
		udelay(1);
	}

	// Step38
	rmw_phy_reg(DDRPHY_R27, 0xFBFFFFFF, 0x00000000);

	// Step39
	rmw_phy_reg(DDRPHY_R78, 0xFFFFF0FE, (sl_lanes << 8));
}

static void exec_trainingWRLVL(uint32_t sl_lanes)
{
	uint32_t tmp;

	// Step2
	tmp = read_phy_reg(DDRPHY_R24);
	write_phy_reg(DDRPHY_R24, tmp | 0x01000000);

	// Step3
	write_phy_reg(DDRPHY_R37, sl_lanes);

	// Step4
	write_phy_reg(DDRPHY_R31, 0x00010000);

	// Step5
	write_phy_reg(DDRPHY_R18, 0x50200000);

	// Step6
	while ((read_phy_reg(DDRPHY_R18) & 0x10000000) != 0x00000000)
		;

	// Step7 - Step8
	if (((read_phy_reg(DDRPHY_R36) & sl_lanes) != sl_lanes) ||
	   ((read_phy_reg(DDRPHY_R37) & sl_lanes) != 0)) {
		panic("PANIC");
	}

	// Step9
	write_phy_reg(DDRPHY_R24, tmp);
}

static void exec_trainingVREF(uint32_t sl_lanes, uint32_t byte_lanes)
{
	uint32_t vref_mid_level_code;
	uint32_t vref_training_value;
	uint32_t sweep_range;
	uint16_t current_vref = 0;
	uint32_t best_window_diff_so_far[MAX_BYTE_LANES];
	uint32_t num_best_vref_matches[MAX_BYTE_LANES];
	uint32_t all_best_vref_matches[MAX_BYTE_LANES][MAX_BEST_VREF_SAVED];
	uint8_t window_0, window_1, window_diff;
	uint32_t highest_best_vref_val, lowest_best_vref_val;
	uint8_t orig_cs_config;
	uint32_t tmp;
	int i, j;

	// Step2
	for (i = 0; i < byte_lanes; i++) {
		write_phy_reg(DDRPHY_R29, i);
		rmw_phy_reg(DDRPHY_R07, 0xFFFFFFCF, 0x00000010);
	}
	// Step3
	vref_mid_level_code = (read_mc_reg(DDRMC_R040) >> 16) & 0xFF;
	sweep_range = read_mc_reg(DDRMC_R043) & 0xFF;

	// Step4
	for (i = 0; i < byte_lanes; i++) {
		best_window_diff_so_far[i] = 255;
		num_best_vref_matches[i] = 0;
	}

	// Step5
	for (vref_training_value = 0;
		 vref_training_value < (sweep_range * 2) + 1;
		 vref_training_value += VREF_SETP) {
		// Step5.1
		if (vref_training_value < sweep_range + 1) {
			if (vref_mid_level_code < vref_training_value + 2) {
				vref_training_value = sweep_range;
				continue;
			} else {
				current_vref = vref_mid_level_code - vref_training_value;
			}
		} else {
			if ((vref_mid_level_code + vref_training_value - sweep_range) > 126) {
				break;
			} else {
				current_vref = vref_mid_level_code + vref_training_value - sweep_range;
			}
		}
		for (i = 0; i < byte_lanes; i++) {
			write_phy_reg(DDRPHY_R29, 7 * i);
			write_phy_reg(DDRPHY_R66, (current_vref << 4) | 0x00000001);
		}

		// Step5.2
		write_phy_reg(DDRPHY_R18, 0x30800000);
		while ((read_phy_reg(DDRPHY_R18) & 0x10000000) != 0x00000000)
			;

		// Step5.3
		for (i = 0; i < byte_lanes; i++) {
			if (((read_phy_reg(DDRPHY_R59) >> (14 + i)) & 0x1) == 0x0) {
				write_phy_reg(DDRPHY_R29, i * 6);
				window_0 = read_phy_reg(DDRPHY_R69) & 0x3F;
				window_1 = (read_phy_reg(DDRPHY_R69) >> 8) & 0x3F;
				window_diff = (window_0 > window_1) ?
								window_0 - window_1 : window_1 - window_0;
				if (window_diff < best_window_diff_so_far[i]) {
					best_window_diff_so_far[i] = window_diff;
					all_best_vref_matches[i][0] = current_vref;
					num_best_vref_matches[i] = 1;
				} else if ((window_diff == best_window_diff_so_far[i]) &&
						(num_best_vref_matches[i] < MAX_BEST_VREF_SAVED)) {
					all_best_vref_matches[i][num_best_vref_matches[i]] = current_vref;
					num_best_vref_matches[i] += 1;
				}
			} else {
			}
		}
		// Step5.4
	}

	// Step6
	for (i = 0; i < byte_lanes; i++) {
		highest_best_vref_val = 0x0;
		lowest_best_vref_val = 0x7F;
		for (j = 0; j < num_best_vref_matches[i]; j++) {
			highest_best_vref_val =
				_MAX(all_best_vref_matches[i][j], highest_best_vref_val);
			lowest_best_vref_val  =
				_MIN(all_best_vref_matches[i][j], lowest_best_vref_val);
		}
		current_vref = (highest_best_vref_val + lowest_best_vref_val) >> 1;
		write_phy_reg(DDRPHY_R29, 7 * i);
		write_phy_reg(DDRPHY_R66, current_vref << 4);
	}

	// Step7
	write_phy_reg(DDRPHY_R19, 0xFF00FF00);
	write_phy_reg(DDRPHY_R20, 0xFF00FF00);

	// Step8
	write_phy_reg(DDRPHY_R18, 0x30800000);
	while ((read_phy_reg(DDRPHY_R18) & 0x10000000) != 0x00000000)
		;

	// Step9
	tmp = (read_phy_reg(DDRPHY_R59) >> 14) & sl_lanes;
	if ((tmp ^ sl_lanes) != sl_lanes) {
		panic("PANIC");
	}

	// Step10
	rmw_phy_reg(DDRPHY_R54, 0xFFFFFF7F, 0x00000080);

	// Step11
	vref_mid_level_code = (read_mc_reg(DDRMC_R043) >> 8) & 0xFF;
	sweep_range = (read_mc_reg(DDRMC_R043) >> 16) & 0xFF;

	// Step12
	orig_cs_config = read_phy_reg(DDRPHY_R25) & 0x3;

	// Step13
	setup_vref_training_registers(vref_mid_level_code, sl_lanes, 1);

	// Step14
	rmw_phy_reg(DDRPHY_R66, 0xFFFFFFFE, 0x00000001);

	// Step15
	for (i = 0; i < byte_lanes; i++) {
		best_window_diff_so_far[i] = 255;
		num_best_vref_matches[i] = 0;
	}

	// Step16
	for (vref_training_value = 0;
		 vref_training_value < (sweep_range * 2) + 1;
		 vref_training_value += VREF_SETP) {
		// Step16.1
		if (vref_training_value < (sweep_range + 1)) {
			if (vref_training_value > vref_mid_level_code) {
				vref_training_value = sweep_range;
				continue;
			} else {
				current_vref = vref_mid_level_code - vref_training_value;
			}
		} else {
			if ((vref_mid_level_code + vref_training_value - sweep_range) <= 73) {
				current_vref =
					vref_mid_level_code + vref_training_value - sweep_range;
			} else {
				break;
			}
		}
		setup_vref_training_registers(current_vref, orig_cs_config, 0);

		// Step16.2
		write_phy_reg(DDRPHY_R18, 0x30500000);
		while ((read_phy_reg(DDRPHY_R18) & 0x10000000) != 0x00000000)
			;

		// Step16.3
		tmp = (read_phy_reg(DDRPHY_R64) >> 20) & sl_lanes;
		for (i = 0; i < byte_lanes; i++) {
			if ((tmp ^ sl_lanes) == sl_lanes) {
				write_phy_reg(DDRPHY_R29, i * 6);
				window_0 = read_phy_reg(DDRPHY_R69) & 0x3F;
				window_1 = (read_phy_reg(DDRPHY_R69) >> 8) & 0x3F;
				window_diff = (window_0 > window_1) ?
								window_0 - window_1 : window_1 - window_0;
				if (window_diff < best_window_diff_so_far[i]) {
					best_window_diff_so_far[i] = window_diff;
					all_best_vref_matches[i][0] = current_vref;
					num_best_vref_matches[i] = 1;
				} else if ((window_diff == best_window_diff_so_far[i]) &&
						(num_best_vref_matches[i] < MAX_BEST_VREF_SAVED)) {
					all_best_vref_matches[i][num_best_vref_matches[i]] = current_vref;
					num_best_vref_matches[i] += 1;
				}
			} else {
			}
		}
		// Step16.4
	}

	// Step17
	highest_best_vref_val = 0x0;
	lowest_best_vref_val = 0x7F;
	for (i = 0; i < byte_lanes; i++) {
		for (j = 0; j < num_best_vref_matches[i]; j++) {
			highest_best_vref_val =
				_MAX(all_best_vref_matches[i][j], highest_best_vref_val);
			lowest_best_vref_val  =
				_MIN(all_best_vref_matches[i][j], lowest_best_vref_val);
		}
	}
	current_vref = (highest_best_vref_val + lowest_best_vref_val) >> 1;

	// Step18
	setup_vref_training_registers(current_vref, sl_lanes, 0);

	// Step19
	rmw_mc_reg(DDRMC_R044, 0xFFFFFF00, current_vref);

	// Step20
	rmw_phy_reg(DDRPHY_R66, 0xFFFFFFFE, 0x00000000);

	// Step21
	setup_vref_training_registers(current_vref, sl_lanes, 2);

	// Step22
	rmw_phy_reg(DDRPHY_R54, 0xFFFFFF7F, 0x00000000);

	// Step23
	for (i = 0; i < byte_lanes; i++) {
		write_phy_reg(DDRPHY_R29, i);
		rmw_phy_reg(DDRPHY_R07, 0xFFFFFFCF, 0x00000030);
	}
}

static void setup_vref_training_registers(uint8_t vref_value, uint8_t cs, uint8_t turn_on_off_vref_training)
{
	uint8_t vref_op_code;
	uint16_t mr;

	// Step1
	if (vref_value > 50) {
		vref_op_code = vref_value - 23;
	} else {
		vref_op_code = vref_value | (1 << 6);
	}

	// Step2
	mr = read_mc_reg(DDRMC_R017) & 0xFF00;
	write_mr(cs, 6,
		mr | (((turn_on_off_vref_training == 2) ? 0 : 1) << 7) | vref_op_code);

	// Step3
	udelay(1);
}

static void write_mr(uint8_t cs, uint8_t mrw_sel, uint16_t mrw_data)
{
	uint8_t mrw_cs;
	uint8_t mrw_allcs;

	// Step1
	mrw_cs = 0;
	if (cs & 0x1) {
		rmw_mc_reg(DDRMC_R013, 0xFFFF0000, mrw_data);
		mrw_cs = 0;
	}
	if (cs & 0x2) {
		rmw_mc_reg(DDRMC_R014, 0xFFFF0000, mrw_data);
		mrw_cs = 1;
	}
	mrw_allcs = ((cs & 0x3) == 0x3) ? 1 : 0;

	// Step2
	rmw_mc_reg(DDRMC_R008, 0xFC000000,
		0x02800000 | (mrw_allcs << 24) | (mrw_cs << 8) | mrw_sel);

	// Step3
	while ((read_mc_reg(DDRMC_R022) & (1 << 3)) != (1 << 3))
		;

	// Step4
	rmw_mc_reg(DDRMC_R024, 0xFFFFFFF7, 0x00000008);
}

static void exec_trainingBITLVL(uint32_t sl_lanes)
{
	uint32_t tmp;

	// Step2
	write_phy_reg(DDRPHY_R62, 0x00000000);

	// Step3
	write_phy_reg(DDRPHY_R19, 0xFF00FF00);
	write_phy_reg(DDRPHY_R20, 0xFF00FF00);

	// Step4
	write_phy_reg(DDRPHY_R18, 0x30A00000);

	// Step5
	while ((read_phy_reg(DDRPHY_R18) & 0x10000000) != 0x00000000)
		;

	// Step6
	tmp = (read_phy_reg(DDRPHY_R59) >> 14) & sl_lanes;
	if ((tmp ^ sl_lanes) != sl_lanes) {
		panic("PANIC");
	}

	// Step7
	rmw_phy_reg(DDRPHY_R54, 0xFFFFFF7F, 0x00000080);

	// Step8
	write_phy_reg(DDRPHY_R18, 0x30700000);

	// Step9
	while ((read_phy_reg(DDRPHY_R18) & 0x10000000) != 0x00000000)
		;

	// Step10
	tmp = (read_phy_reg(DDRPHY_R64) >> 20) & sl_lanes;
	if ((tmp ^ sl_lanes) != sl_lanes) {
		panic("PANIC");
	}

	// Step11
	rmw_phy_reg(DDRPHY_R54, 0xFFFFFF7F, 0x00000000);

	// Step12
	write_phy_reg(DDRPHY_R51, 0x00080000);

	// Step13
	write_phy_reg(DDRPHY_R18, 0x11200000);

	// Step14
	while ((read_phy_reg(DDRPHY_R18) & 0x10000000) != 0x00000000)
		;

	// Step15
	write_phy_reg(DDRPHY_R51, 0x00000000);

	// Step16
	write_phy_reg(DDRPHY_R18, 0x30A00000);

	// Step17
	while ((read_phy_reg(DDRPHY_R18) & 0x10000000) != 0x00000000)
		;
}

static void opt_delay(uint32_t sl_lanes, uint32_t byte_lanes)
{
	uint32_t tmp;
	uint16_t dlls_trim_ca;
	uint16_t dlls_trim_2[MAX_BYTE_LANES];
	uint16_t op_dqs_trim[MAX_BYTE_LANES];
	uint16_t min_WL;
	uint16_t min_WD = 128;
	int i, j;

	// Step2
	rmw_phy_reg(DDRPHY_R27, 0xFBFFFFFF, 0x04000000);

	// Step3
	rmw_mc_reg(DDRMC_R004, ~(0x7F << LP_CMD_OFFSET), (0x00000011 << LP_CMD_OFFSET));
	while (((read_mc_reg(DDRMC_R005) >> 24) & 0x7F) != 0x48)
		;

	// Step4
	write_phy_reg(DDRPHY_R29, 0);
	dlls_trim_ca = read_phy_reg(DDRPHY_R57) & 0x7F;
	min_WL = dlls_trim_ca;

	for (i = 0; i < byte_lanes; i++) {
		write_phy_reg(DDRPHY_R29, 6 * i);
		dlls_trim_2[i] = read_phy_reg(DDRPHY_R31) & 0x3F;
		min_WL = _MIN(min_WL, dlls_trim_2[i]);

		write_phy_reg(DDRPHY_R29, (7 * i) | 0x00000900);
		op_dqs_trim[i] = read_phy_reg(DDRPHY_R56) & 0x3F;
		min_WD = _MIN(min_WD, op_dqs_trim[i]);
		for (j = 0; j < 9; j++) {
			write_phy_reg(DDRPHY_R29, (i * 7) | (j << 8));
			tmp = read_phy_reg(DDRPHY_R56) & 0x7F;
			tmp = (tmp & 0x40) ?
				(op_dqs_trim[i] + (tmp & 0x3F)) : (op_dqs_trim[i] - (tmp & 0x3F));
			min_WD = _MIN(min_WD, tmp);
		}
	}

	// Step5
	tmp = (dlls_trim_ca - min_WL) & 0x7F;
	write_phy_reg(DDRPHY_R29, 0);
	write_phy_reg(DDRPHY_R57, tmp | 0x00000080);
	write_phy_reg(DDRPHY_R28, tmp | 0x00000200);
	rmw_phy_reg(DDRPHY_R27, 0xFFFFFF80, tmp);

	for (i = 0; i < byte_lanes; i++) {
		tmp = (dlls_trim_2[i] - min_WL) & 0x3F;
		write_phy_reg(DDRPHY_R29, 6 * i);
		rmw_phy_reg(DDRPHY_R31, 0xFFFFFFC0, tmp);

		write_phy_reg(DDRPHY_R29, (7 * i) | 0x00000900);
		tmp = (op_dqs_trim[i] - min_WD) & 0x3F;
		rmw_phy_reg(DDRPHY_R56, 0xFFFFFF80, tmp);
	}

	// Step6
	rmw_mc_reg(DDRMC_R004, ~(0x7F << LP_CMD_OFFSET), (0x00000002 << LP_CMD_OFFSET));
	while (((read_mc_reg(DDRMC_R005) >> 24) & 0x7F) != 0x40)
		;

	// Step6
	rmw_phy_reg(DDRPHY_R27, 0xFBFFFFFF, 0x00000000);
	while ((read_phy_reg(DDRPHY_R42) & 0x3) != sl_lanes)
		;
}

static void exec_trainingSL(uint32_t sl_lanes)
{
	// Step2
	write_phy_reg(DDRPHY_R62, 0x00000001);

	// Step3
	write_phy_reg(DDRPHY_R34, 0x00000010);

	// Step4
	write_phy_reg(DDRPHY_R19, 0x789B3DE0);
	write_phy_reg(DDRPHY_R20, 0xF10E4A56);

	// Step5
	write_phy_reg(DDRPHY_R18, 0x11200000);

	// Step6
	while ((read_phy_reg(DDRPHY_R18) & 0x10000000) != 0x00000000)
		;

	// Step7
	write_phy_reg(DDRPHY_R18, 0x11200000);

	// Step8
	while ((read_phy_reg(DDRPHY_R18) & 0x10000000) != 0x00000000)
		;

	// Step9
	write_phy_reg(DDRPHY_R18, 0x34200000);

	// Step10
	while ((read_phy_reg(DDRPHY_R18) & 0x10000000) != 0x00000000)
		;

	// Step11
	if ((read_phy_reg(DDRPHY_R18) & sl_lanes) != sl_lanes) {
		panic("PANIC");
	}

	// Step12
	write_phy_reg(DDRPHY_R62, 0x00000003);
}

static void program_phy2(void)
{
	uint16_t dram_clk_period;
	uint32_t tmp, b21, b22, b23;

	// Step1
	tmp = read_mc_reg(DDRMC_R039);
	dram_clk_period = tmp & 0xFFFF;
	b21 = (tmp >> 21) & 0x1;
	b22 = (tmp >> 22) & 0x1;
	b23 = (tmp >> 23) & 0x1;

	// Step2
	rmw_phy_reg(DDRPHY_R64, 0xFFFFFFFE, b23);
	rmw_phy_reg(DDRPHY_R59, 0xFFFFFFFE, (b23 == 1 ? 0 : b22));
	write_phy_reg(DDRPHY_R55, (b21 << 24) |
		_MIN(1000000000000 / (2 * dram_clk_period * 256), 0xFFFFFF));

	// Step3
	rmw_phy_reg(DDRPHY_R27, 0xFBFFFFFF, 0x04000000);
	rmw_phy_reg(DDRPHY_R27, 0xFC0000FF,
		_MIN(1000000000000 / (dram_clk_period * 256), 0x3FFFF) << 8);
	rmw_phy_reg(DDRPHY_R27, 0xFBFFFFFF, 0x00000000);
}

static void program_mc2(void)
{
	uint8_t main_clk_dly;
	uint8_t tphy_rdlat;
	uint32_t tmp;

	// Step1
	main_clk_dly = (read_phy_reg(DDRPHY_R21) >> 4) & 0xF;
	tmp = (read_mc_reg(DDRMC_R028) >> 24) & 0x7F;
	tphy_rdlat = ((main_clk_dly + 1 + 1) * 2) + 2 + ((tmp == 1) ? 2 : 0);

	// Step2
	rmw_mc_reg(DDRMC_R027, 0xFFFFFF80, tphy_rdlat & 0x7F);
}
