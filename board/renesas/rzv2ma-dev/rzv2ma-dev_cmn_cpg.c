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
 * File Name    : evk_cmn_cpg.c
 * Description  : operation functions for CPG
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

#include "rzv2ma-dev_common.h"
#include "rzv2ma-dev_cmn_cpg.h"

#define CPG_NO_REGISTER_OFFSET          (0xFFFFFFFF)
#define CPG_TIMEOUT_UNIT_IS_US          (10)

typedef struct
{
    unsigned int pll_used_cnt[CPG_PLL_MAX+1];
} st_cpg_priv_t;

static st_cpg_priv_t gl_cpg_priv =
{
    .pll_used_cnt = { 0 },
};

static st_cpg_pll_param_t pll1_param =
{
    .ssc.word    = 0x00000004,
    .clk.word[0] = 0x000014C2,
    .clk.word[1] = 0x00150801
};

static st_cpg_pll_param_t pll2_param =
{
    .ssc.word    = 0x00000004,
    .clk.word[0] = 0x00001903,
    .clk.word[1] = 0x000E0E00
};

/** 600MHz w/o SSCG */
static st_cpg_pll_param_t pll3_param =
{
    .ssc.word    = 0x00000000,
    .clk.word[0] = 0x00001903,
    .clk.word[1] = 0x00100800
};

static st_cpg_pll_param_t pll4_param =
{
    .ssc.word    = 0x00000000,
    .clk.word[0] = 0x00001802,
    .clk.word[1] = 0x00000002
};

static st_cpg_pll_param_t pll6_param =
{
    .ssc.word    = 0x00000000,
    .clk.word[0] = 0x00001A42,
    .clk.word[1] = 0x00150A01
};

static st_cpg_pll_param_t pll7_param =
{
    .ssc.word    = 0x00000004,
    .clk.word[0] = 0x000018C2,
    .clk.word[1] = 0x00150A01
};

static st_cpg_pll_param_t * const gl_priv_pll_param_tbl[CPG_PLL_MAX+1] =
{
    NULL,
    &pll1_param,
    &pll2_param,
    &pll3_param,
    &pll4_param,
    NULL,
    &pll6_param,
    &pll7_param
};

static const unsigned int gl_priv_pll_top_offset_table[CPG_PLL_MAX+1] =
{
    CPG_NO_REGISTER_OFFSET,
    CPG_PLL1_STBY,
    CPG_PLL2_STBY,
    CPG_PLL3_STBY,
    CPG_PLL4_STBY,
    CPG_NO_REGISTER_OFFSET,
    CPG_PLL6_STBY,
    CPG_PLL7_STBY,
};

#define CPG_REG_WEN_SHIFT           (16)
#define CPG_SET_DATA_MASK           (0x0000FFFFUL)

#define CPG_PLLN_STBY_OFFSET        (0x00)
#define CPG_PLLN_CLK1_OFFSET        (0x04)
#define CPG_PLLN_CLK2_OFFSET        (0x08)
#define CPG_PLLN_MON_OFFSET         (0x0C)

#define CPG_PLL_CCTRL_REG_EXIT_FLAGS    (0x0000009E)

#define CPG_TIMEOUT_UNIT_IN_US      (10)
#define CPG_PLL_TURN_MODE_TIMEOUT   (500000)
#define CPG_CLK_FINISH_CHANGE_TIMEOUT   (500000)
#define CPG_PLL_LOCKED  (CPG_PLL_MON_PLL_LOCK)


#define CPG_WAIT_EVENT_PLL(m_pll, m_toc, m_err_code, m_condition, m_rslt)   \
    {   \
        unsigned int count = (m_toc);   \
        unsigned int data;  \
        while (true) {  \
            CPG_GetStatusPLL((m_pll), &data);   \
            if ((m_condition) == (data & CPG_PLL_LOCKED)) { \
                (m_rslt) = CMN_SUCCESS; \
                break;  \
            }   \
            if ((0 == (m_toc)) || (0 < count)) {    \
                CMN_DelayInUS(CPG_TIMEOUT_UNIT_IN_US);  \
                count--;    \
            } else {    \
                (m_rslt) = (m_err_code);    \
                break;  \
            }   \
        }   \
    }

#define CPG_WAIT_EVENT(m_toc, m_err_code, m_condition, m_rslt)  \
    {   \
        unsigned int count = (m_toc)/CPG_TIMEOUT_UNIT_IN_US;    \
        while (true) {  \
            if ((m_condition)) {    \
                (m_rslt) = CMN_SUCCESS; \
                break;  \
            }   \
            if ((0 == (m_toc)) || (0 < count)) {    \
                CMN_DelayInUS(CPG_TIMEOUT_UNIT_IN_US);  \
                count--;    \
            } else {    \
                (m_rslt) = (m_err_code);    \
                break;  \
            }   \
        }   \
    }


unsigned int CPG_ReadReg(unsigned int offset)
{
    return (CMN_REG_Read32(CPG_BASE_ADDRESS + offset));
}

void CPG_WriteReg(unsigned int offset, unsigned int value)
{
    CMN_REG_Write32((CPG_BASE_ADDRESS + offset), value);
}


int CPG_MoveToActivePLL(e_cpg_pll_num_t pll_num, st_cpg_pll_param_t *p_set_data)
{
    unsigned int offset;
    unsigned int value;

    if ((pll_num < CPG_PLL_MIN) || (CPG_PLL_MAX < pll_num))
    {
        return (CPG_ERROR_NO_REGISTER);
    }

    offset = gl_priv_pll_top_offset_table[pll_num];
    if (CPG_NO_REGISTER_OFFSET == offset)
    {
        return (CPG_ERROR_NO_REGISTER);
    }

    value = CPG_ReadReg(offset+CPG_PLLN_MON_OFFSET);
    if (0 != (value & CPG_PLL_MON_RESETB))
    {
        return (CPG_ERROR_PLL_ACTIVE);
    }

    if (NULL != p_set_data)
    {
        value = p_set_data->clk.word[0];
        CPG_WriteReg(offset+CPG_PLLN_CLK1_OFFSET, value);
        value = p_set_data->clk.word[1];
        CPG_WriteReg(offset+CPG_PLLN_CLK2_OFFSET, value);
        value = p_set_data->ssc.word;
    }else
    {
        value = CPG_ReadReg(offset+CPG_PLLN_STBY_OFFSET);
    }

    if (4 == pll_num)
    {
        value = 0x00010001;
    }else
    {
        value |= 0x00150001;
        value = 0x00050001;    /* Setting up 1chip sim environment */
    }

    CPG_WriteReg(offset+CPG_PLLN_STBY_OFFSET, value);

    return (CMN_SUCCESS);
}

int CPG_GetStatusPLL(e_cpg_pll_num_t pll_num, unsigned int *p_data)
{
    unsigned int offset;
    if ((pll_num < CPG_PLL_MIN) || (CPG_PLL_MAX < pll_num))
    {
        return (CPG_ERROR_NO_REGISTER);
    }

    offset = gl_priv_pll_top_offset_table[pll_num];
    if (CPG_NO_REGISTER_OFFSET == offset)
    {
        return (CPG_ERROR_NO_REGISTER);
    }

    if (NULL == p_data)
    {
        return (CPG_ERROR_NULL_POINTER);
    }

    offset += CPG_PLLN_MON_OFFSET;

    *p_data = CPG_ReadReg(offset);

    return (CMN_SUCCESS);
}


void CPG_SetDifClkFreq(e_cpg_divsel_t target_reg, unsigned short int target, unsigned short int set_value)
{
    unsigned int offset = CPG_CA53_DDIV;
    unsigned int value;

    while (true)
    {
        if (0 == (CPG_ReadReg(CPG_CLKSTATUS) & CPG_CLKSTATUS_DIVX))
        {
            break;
        }
        CMN_DelayInUS(CPG_TIMEOUT_UNIT_IN_US);
    }

    offset += (target_reg * sizeof(unsigned int));

    value = ((unsigned int)target << CPG_REG_WEN_SHIFT)
                | (set_value & CPG_MMCDDI_DDIV_DIVX_SET_MSK);

    CPG_WriteReg(offset, value);

    while (true)
    {
        if (0 == (CPG_ReadReg(CPG_CLKSTATUS) & CPG_CLKSTATUS_DIVX))
        {
            break;
        }
        CMN_DelayInUS(CPG_TIMEOUT_UNIT_IN_US);
    }
}


int CPG_SetClockCtrl(unsigned char reg_num, unsigned short int target, unsigned short int set_value)
{
    unsigned int offset = CPG_CLK_ON1;
    unsigned int value;

    if (reg_num < CPG_CLK_ON_REG_MIN || CPG_CLK_ON_REG_MAX < reg_num)
    {
        return (CPG_ERROR_NO_REGISTER);
    }

    offset += ((reg_num - 1) * sizeof(unsigned int));

    value = ((unsigned int)target << CPG_REG_WEN_SHIFT)
            | (set_value & CPG_SET_DATA_MASK);

    CPG_WriteReg(offset, value);

    return (CMN_SUCCESS);
}


int CPG_GetClockCtrl(unsigned char reg_num, unsigned int *p_data)
{
    unsigned int offset = CPG_CLK_ON1;

    if (reg_num < CPG_CLK_ON_REG_MIN || CPG_CLK_ON_REG_MAX < reg_num)
    {
        return (CPG_ERROR_NO_REGISTER);
    }
    if (NULL == p_data)
    {
        return (CPG_ERROR_NULL_POINTER);
    }

    offset += ((reg_num - 1) * sizeof(unsigned int));

    *p_data = CPG_ReadReg(offset);

    return (CMN_SUCCESS);
}


int CPG_SetResetCtrl(unsigned char reg_num, unsigned short int target, unsigned short int set_value)
{
    unsigned int offset = CPG_RST1;
    unsigned int value;

    if (reg_num < CPG_RST_REG_MIN || CPG_RST_REG_MAX < reg_num)
    {
        return (CPG_ERROR_NO_REGISTER);
    }

    offset += ((reg_num - 1) * sizeof(unsigned int));

    value = ((unsigned int)target << CPG_REG_WEN_SHIFT) | (set_value & CPG_SET_DATA_MASK);

    CPG_WriteReg(offset, value);

    return (CMN_SUCCESS);
}


int CPG_SetPDResetCtrl(unsigned short int target, unsigned short int set_value)
{
    unsigned int offset = CPG_PD_RST;
    unsigned int value;

    value = ((unsigned int)target << CPG_REG_WEN_SHIFT) | (set_value & CPG_SET_DATA_MASK);

    CPG_WriteReg(offset, value);

    return (CMN_SUCCESS);
}


int CPG_WaitResetMon(unsigned int timeout_c, unsigned int msk, unsigned int val)
{
    int rslt = CMN_ERROR;
    unsigned int count;

    do
    {
        if (0 == msk)
        {
            rslt = CMN_SUCCESS;
            break;
        }

        count = timeout_c;
        while (true)
        {
            if (val == (CPG_ReadReg(CPG_RST_MON) & msk))
            {
                rslt = CMN_SUCCESS;
                break;
            }
            if ((0 == timeout_c) || (0 < count))
            {
                CMN_DelayInUS(CPG_TIMEOUT_UNIT_IS_US);
                count--;
            }else
            {
                rslt = CPG_ERROR_TURN_RESET_TIMEOUT;
                break;
            }
        }
    }
    while (0);

    return rslt;
}


int cpg_resumePLL(e_cpg_pll_num_t pll_num)
{
    int rslt;
    unsigned int data;
    st_cpg_pll_param_t *p_data = NULL;

    do
    {
        rslt = CPG_GetStatusPLL(pll_num, &data);
        if (CMN_SUCCESS != rslt)
        {
            break;
        }
        if (0 != (data & CPG_PLL_LOCKED))
        {
            rslt = CPG_ERROR_PLL_NOT_STANDBY | pll_num;
            break;
        }

        p_data = gl_priv_pll_param_tbl[pll_num];
        rslt = CPG_MoveToActivePLL(pll_num, p_data);

        if (CMN_SUCCESS != rslt)
        {
            break;
        }

        CPG_WAIT_EVENT_PLL(pll_num, CPG_PLL_TURN_MODE_TIMEOUT, CPG_ERROR_PLL_TURN_MODE_TIMEOUT, CPG_PLL_LOCKED, rslt);

        if (CMN_SUCCESS != rslt)
        {
            break;
        }
            }

    while (0);

    return rslt;
}

int CPG_WakeUpPLL(e_cpg_pll_num_t pll_num)
{
    int rslt = CMN_ERROR;

    do
    {
        if ((pll_num < CPG_PLL_MIN) || (CPG_PLL_MAX < pll_num))
        {
            rslt = CPG_ERROR_NO_REGISTER;
            break;
        }

        if (0 == gl_cpg_priv.pll_used_cnt[pll_num])
        {
            rslt = cpg_resumePLL(pll_num);
            if (CMN_SUCCESS != rslt)
            {
                break;
            }
            gl_cpg_priv.pll_used_cnt[pll_num]++;
        }else
        {
            gl_cpg_priv.pll_used_cnt[pll_num]++;
            rslt = CMN_SUCCESS;
        }
    }
    while (0);

    return rslt;
}


/*- End of file -*/
