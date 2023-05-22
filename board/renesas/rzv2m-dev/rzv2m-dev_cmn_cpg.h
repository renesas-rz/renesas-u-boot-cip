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
* Copyright (C) 2022 Renesas Electronics Corporation. All rights reserved.
*******************************************************************************/ 
/*******************************************************************************
 * File Name    : evk_cmn_cpg.h
 * Description  : register and API information for CPG
 ******************************************************************************/

#ifndef EVK_CMN_CPG_H
#define EVK_CMN_CPG_H

/* CPG */
#define CPG_BASE_ADDRESS        (0x0A3500000ULL)

/** Registor Offset */
#define CPG_PLL1_STBY           (0x0000)
#define CPG_PLL1_CLK1           (0x0004)
#define CPG_PLL1_CLK2           (0x0008)
#define CPG_PLL1_MON            (0x000C)
#define CPG_PLL2_STBY           (0x0010)
#define CPG_PLL2_CLK1           (0x0014)
#define CPG_PLL2_CLK2           (0x0018)
#define CPG_PLL2_MON            (0x001C)
#define CPG_PLL3_STBY           (0x0020)
#define CPG_PLL3_CLK1           (0x0024)
#define CPG_PLL3_CLK2           (0x0028)
#define CPG_PLL3_MON            (0x002C)
#define CPG_PLL6_STBY           (0x0030)
#define CPG_PLL6_CLK1           (0x0034)
#define CPG_PLL6_CLK2           (0x0038)
#define CPG_PLL6_MON            (0x003C)
#define CPG_PLL7_STBY           (0x0040)
#define CPG_PLL7_CLK1           (0x0044)
#define CPG_PLL7_CLK2           (0x0048)
#define CPG_PLL7_MON            (0x004C)


#define CPG_PLL4_STBY           (0x0100)
#define CPG_PLL4_CLK1           (0x0104)
#define CPG_PLL4_CLK2           (0x0108)
#define CPG_PLL4_MON            (0x010C)


#define CPG_PLL1_CCTRL_RST      (0x0180)
#define CPG_PLL2_CCTRL_RST      (0x0184)
#define CPG_PLL3_CCTRL_RST      (0x0188)
#define CPG_PLL4_CCTRL_RST      (0x018C)

#define CPG_PLL7_CCTRL_RST      (0x0198)

#define CPG_CA53_DDIV           (0x0200)
#define CPG_SYS_DDIV            (0x0204)
#define CPG_MMCDDI_DDIV         (0x0210)
#define CPG_CLK48_DSEL          (0x0214)
#define CPG_CLKSTATUS           (0x0224)


#define CPG_SDIEMM_SSEL         (0x0300)
#define CPG_GMCLK_SDIV          (0x031C)
#define CPG_GMCLK_SSEL          (0x0320)
#define CPG_URT_RCLK_SDIV       (0x0328)
#define CPG_URT_RCLK_SSEL       (0x032C)
#define CPG_CSI_RCLK_SSEL       (0x0330)


#define CPG_CLK_ON1             (0x0400)
#define CPG_CLK_ON2             (0x0404)
#define CPG_CLK_ON3             (0x0408)
#define CPG_CLK_ON4             (0x040C)
#define CPG_CLK_ON5             (0x0410)
#define CPG_CLK_ON6             (0x0414)
#define CPG_CLK_ON7             (0x0418)
#define CPG_CLK_ON8             (0x041C)
#define CPG_CLK_ON9             (0x0420)
#define CPG_CLK_ON10            (0x0424)
#define CPG_CLK_ON11            (0x0428)
#define CPG_CLK_ON12            (0x042C)
#define CPG_CLK_ON13            (0x0430)
#define CPG_CLK_ON14            (0x0434)
#define CPG_CLK_ON15            (0x0438)
#define CPG_CLK_ON16            (0x043C)
#define CPG_CLK_ON17            (0x0440)
#define CPG_CLK_ON18            (0x0444)
#define CPG_CLK_ON19            (0x0448)
#define CPG_CLK_ON20            (0x044C)
#define CPG_CLK_ON21            (0x0450)
#define CPG_CLK_ON22            (0x0454)
#define CPG_CLK_ON23            (0x0458)
#define CPG_CLK_ON24            (0x045C)
#define CPG_CLK_ON25            (0x0460)
#define CPG_CLK_ON26            (0x0464)
#define CPG_CLK_ON27            (0x0468)

#define CPG_RST_MSK             (0x0504)


#define CPG_RST1                (0x0600)
#define CPG_RST2                (0x0604)
#define CPG_RST3                (0x0608)
#define CPG_RST4                (0x060C)
#define CPG_RST5                (0x0610)
#define CPG_RST6                (0x0614)
#define CPG_RST7                (0x0618)
#define CPG_RST8                (0x061C)
#define CPG_RST9                (0x0620)
#define CPG_RST10               (0x0624)
#define CPG_RST11               (0x0628)
#define CPG_RST12               (0x062C)
#define CPG_RST13               (0x0630)
#define CPG_RST14               (0x0634)
#define CPG_RST15               (0x0638)


#define CPG_RST_MON             (0x0680)

#define CPG_PD_RST              (0x0800)


/** Bit assign */
#define CPG_PLL_STBY_RESETB                 (0x00000001)
#define CPG_PLL_STBY_WEN_RESETB             (0x00010000)
#define CPG_PLL_STBY_WEN_SSE_EN             (0x00040000)
#define CPG_PLL_STBY_WEN_SSC_MODE           (0x00100000)

#define CPG_PLL_MON_RESETB                  (0x00000001)
#define CPG_PLL_MON_PLL_LOCK                (0x00000010)

#define CPG_PLL1_CCTRL_RST_P1_0_RST         (0x00000001)
#define CPG_PLL1_CCTRL_WEN_RST_P1_0_RST     (0x00010000)

#define CPG_CA53_DDIV_DIVA_SET_MIN          (0)
#define CPG_CA53_DDIV_WEN_DIVA              (0x00010000)

#define CPG_SYS_DDIV_DIVD_SET_SHIFT         (4)
#define CPG_SYS_DDIV_DIVD_SET_MIN           (0)
#define CPG_SYS_DDIV_DIVE_SET_SHIFT         (8)
#define CPG_SYS_DDIV_DIVE_SET_MIN           (0)
#define CPG_SYS_DDIV_WEN_DIVB               (0x00010000)
#define CPG_SYS_DDIV_WEN_DIVD               (0x00100000)
#define CPG_SYS_DDIV_WEN_DIVE               (0x01000000)

#define CPG_MMCDDI_DDIV_DIVX_SET_MSK        (0x00000003)
#define CPG_MMCDDI_DDIV_DIVX_SET_SHIFT      (0)
#define CPG_MMCDDI_DDIV_DIVX_SET_MAX        (2)
#define CPG_MMCDDI_DDIV_DIVX_SET_MIN        (0)
#define CPG_MMCDDI_DDIV_WEN_DIVX            (0x00010000)

#define CPG_CLK48_DSEL_SELD                 (0x00000002)
#define CPG_CLK48_DSEL_SELE                 (0x00000004)
#define CPG_CLK48_DSEL_WEN_SELD             (0x00020000)
#define CPG_CLK48_DSEL_WEN_SELE             (0x00040000)

#define CPG_SDIEMM_SSEL_SELSDI              (0x00000001)
#define CPG_SDIEMM_SSEL_WEN_SELSDI          (0x00010000)

#define CPG_URT_RCLK_SSEL_WEN_SELW0         (0x00010000)

#define CPG_CLKSTATUS_DIVA                  (0x00000001)
#define CPG_CLKSTATUS_DIVB                  (0x00000002)
#define CPG_CLKSTATUS_DIVD                  (0x00000004)
#define CPG_CLKSTATUS_DIVE                  (0x00000008)
#define CPG_CLKSTATUS_DIVF                  (0x00000010)
#define CPG_CLKSTATUS_DIVG                  (0x00000020)
#define CPG_CLKSTATUS_DIVNFI                (0x00000040)
#define CPG_CLKSTATUS_DIVX                  (0x00000080)
#define CPG_CLKSTATUS_DIVH                  (0x00000100)
#define CPG_CLKSTATUS_DIVI                  (0x00000200)
#define CPG_CLKSTATUS_DIVJ                  (0x00000400)
#define CPG_CLKSTATUS_DIVM                  (0x00000800)
#define CPG_CLKSTATUS_DIVH2                 (0x00001000)

#define CPG_PD_RST_MEM_RSTB                 (0x00000001)
#define CPG_PD_RST_WEN_MEM_RSTB             (0x00010000)

#define CPG_RST_MON_DMAA                    (0x00000010)
#define CPG_RST_MON_SDI0                    (0x00000040)
#define CPG_RST_MON_SDI1                    (0x00000080)
#define CPG_RST_MON_EMM                     (0x00000100)
#define CPG_RST_MON_ETH0                    (0x00000800)
#define CPG_RST_MON_PWM_0                   (0x00400000)
#define CPG_RST_MON_PWM_1                   (0x00800000)
#define CPG_RST_MON_CSI_1                   (0x01000000)
#define CPG_RST_MON_CSI_2                   (0x02000000)
#define CPG_RST_MON_URT                     (0x04000000)
#define CPG_PD_RST_RFX_RSTB                 (0x00000010)
#define CPG_PD_RST_WEN_RFX_RSTB             (0x00100000)

#define CPG_PLL_MIN                         (1)
#define CPG_PLL_MAX                         (7)

#define CPG_CLK_ON_REG_MIN                  (1)
#define CPG_CLK_ON_REG_MAX                  (27)

#define CPG_RST_REG_MIN                     (1)
#define CPG_RST_REG_MAX                     (15)

typedef struct
{
    union
    {
        unsigned int        word;
        struct
        {
            unsigned int    :2;
            unsigned int    enable:1;
            unsigned int    :1;
            unsigned int    mode:2;
            unsigned int    :26;
        }bit;
    }ssc;
    union
    {
        unsigned int        word[2];
        struct
        {
            unsigned int    p:6;
            unsigned int    m:10;
            unsigned int    k:16;
            unsigned int    s:3;
            unsigned int    :5;
            unsigned int    mrr:6;
            unsigned int    :2;
            unsigned int    mfr:8;
            unsigned int    :8;
        } bit;
    } clk;
} st_cpg_pll_param_t;

typedef enum
{
    CPG_ERROR_ARGUMENT               = -201,
    CPG_ERROR_NO_REGISTER            = -202,
    CPG_ERROR_NULL_POINTER           = -203,
    CPG_ERROR_PLL_TURN_MODE_TIMEOUT  = -204,
    CPG_ERROR_PLL_ACTIVE             = -205,
    CPG_ERROR_PLL_STANDBY            = -206,
    CPG_ERROR_TURN_RESET_TIMEOUT     = -207,
    CPG_ERROR_CLK_CHANGE_TIMEOUT     = -208,
    CPG_ERROR_PLL_NOT_ACTIVE         = -280,
    CPG_ERROR_PLL_NOT_STANDBY        = -290
} e_cpg_error_code_t;

typedef enum
{
    CPG_PLL_1 = 1,
    CPG_PLL_2 = 2,
    CPG_PLL_3 = 3,
    CPG_PLL_4 = 4,
    CPG_PLL_6 = 6,
    CPG_PLL_7 = 7
} e_cpg_pll_num_t;

typedef enum
{
    CPG_DDIV_CA53 = 0,
    CPG_DDIV_SYS,
    CPG_dmy1,
    CPG_dmy2,
    CPG_DDIV_MMCDDI,
    CPG_DSEL_CLK48,
    CPG_dmy3,
    CPG_dmy4,
    CPG_dmy5,
    CPG_SSEL_SDIEMM = 64,
    CPG_dmy6,
    CPG_dmy7,
    CPG_dmy8,
    CPG_dmy9,
    CPG_dmyA,
    CPG_dmyB,
    CPG_SDIV_GMCLK,
    CPG_SSEL_GMCLK,
    CPG_dmyC,
    CPG_SDIV_URT_RCLK,
    CPG_SSEL_URT_RCLK,
    CPG_SSEL_CSI_RCLK
} e_cpg_divsel_t;

/** prototype defined **/
unsigned int CPG_ReadReg(unsigned int offset);
void CPG_WriteReg(unsigned int offset, unsigned int value);

int CPG_MoveToActivePLL(e_cpg_pll_num_t pll_num, st_cpg_pll_param_t *p_set_data);
int CPG_GetStatusPLL(e_cpg_pll_num_t pll_num, unsigned int *p_data);

void CPG_SetDifClkFreq(e_cpg_divsel_t target_reg, unsigned short int target, unsigned short int set_value);
int CPG_SetClockCtrl(unsigned char reg_num, unsigned short int target, unsigned short int set_value);
int CPG_SetResetCtrl(unsigned char reg_num, unsigned short int target, unsigned short int set_value);
int CPG_SetPDResetCtrl(unsigned short int target, unsigned short int set_value);
int CPG_WaitResetMon(unsigned int timeout_c, unsigned int msk, unsigned int val);
int CPG_WakeUpPLL(e_cpg_pll_num_t pll_num);

#endif /* EVK_CMN_CPG_H */
