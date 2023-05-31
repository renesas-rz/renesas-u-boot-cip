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
 * File Name    : rzv2m-dev_common.c
 ******************************************************************************/

#include <stdint.h>
#include "rzv2m-dev_common.h"

#define SYSCNT_FREQUENCY    (24000000)
#define SYSCNT_BASE_ADDRESS (0xA3F00000)

#define SYSCNT_CNTCV        (0x1000)

/*******************************************************************************
 * Function Name: CMN_GetSysCnt
 * Description  : get system counter
 * Arguments    : none
 * Return Value : system counter
 ******************************************************************************/
unsigned long int CMN_GetSysCnt(void)
{
	unsigned int g_syscnt_base = SYSCNT_BASE_ADDRESS;
    return CMN_REG_Read64(g_syscnt_base + SYSCNT_CNTCV);
}

/*******************************************************************************
 * Function Name: CMN_GetFreq4SysCnt
 * Description  : get system counter
 * Arguments    : none
 * Return Value : system counter
 ******************************************************************************/
unsigned int CMN_GetFreq4SysCnt(void)
{
    return SYSCNT_FREQUENCY;
}

/*******************************************************************************
 * Function Name: CMN_DelayInUSec
 * Description  : delayed time in microsecond
 * Arguments    :
 *                us -
 *                    delay times in microsecond
 * Return Value : none
 ******************************************************************************/
void CMN_DelayInUSec(unsigned long int us)
{
    unsigned long int start = CMN_GetSysCnt();
    unsigned long int cycles = (CMN_GetFreq4SysCnt() / 1000000UL) * us;

    while ((CMN_GetSysCnt() - start) < cycles)
    {
        asm volatile("nop");
    }
}

