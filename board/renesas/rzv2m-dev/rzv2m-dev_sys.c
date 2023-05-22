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
 * File Name    : rzv2m-dev_sys.c
 * Description  : operation functions for SYS
 ******************************************************************************/

#include <stdint.h>

#include "rzv2m-dev_common.h"
#include "rzv2m-dev_sys.h"

/*******************************************************************************
 * Function Name: sys_GetRegOffset
 * Description  : get register offset
 *
 * Arguments    : value -
 *                    target value
 * Return Value : offset -
 *                    register offset value
 ******************************************************************************/
static inline unsigned int sys_GetRegOffset(unsigned int value)
{
    return (((((value) & 0xFF00) >> 8) - 1) * 4);
}
/*******************************************************************************
 End of function sys_GetRegOffset
 ******************************************************************************/


/*******************************************************************************
 * Function Name: SYS_ReadReg
 * Description  : Read access to a SYSTEM Register in unsigned int.
 *
 * Arguments    : offset -
 *                    SYSTEM Register Offset.
 * Return Value : read data -
 *
 ******************************************************************************/
unsigned int SYS_ReadReg(unsigned int offset)
{
    return (CMN_REG_Read32(SYS_BASE_ADDRESS + offset));
}
/*******************************************************************************
 End of function SYS_ReadReg
 ******************************************************************************/

/*******************************************************************************
 * Function Name: SYS_WriteReg
 * Description  : Write access to a SYSTEM Register in unsigned int.
 *
 * Arguments    : offset -
 *                    SYSTEM Register Offset.
 *                value -
 *                    Write data.
 * Return Value : none -
 *
 ******************************************************************************/
void SYS_WriteReg(unsigned int offset, unsigned int value)
{
    CMN_REG_Write32((SYS_BASE_ADDRESS + offset), value);
}
/*******************************************************************************
 End of function SYS_WriteReg
 ******************************************************************************/


/*******************************************************************************
 * Function Name: SYS_MoveToActiveSRAM
 * Description  : target SRAM change state to active.
 *
 * Arguments    : target -
 *                    target SRAM kind
 * Return Value : none -
 *
 ******************************************************************************/
void SYS_MoveToActiveSRAM(e_sys_target_sram_t target)
{
    unsigned int reg_addr_sram_sdm = SYS_SRAM_SDM1;
    unsigned int reg_addr_sram_rs  = SYS_SRAM_RS1;
    unsigned int target_bit;

    reg_addr_sram_sdm += sys_GetRegOffset(target);
    reg_addr_sram_rs  += sys_GetRegOffset(target);
    target_bit = 0x01 << (target & 0xFF);
    target_bit = (target_bit << 16);

    SYS_WriteReg( reg_addr_sram_rs, target_bit);
    CMN_DelayInUS(1);
    SYS_WriteReg( reg_addr_sram_sdm, target_bit);
    CMN_DelayInUS(1);
}
/*******************************************************************************
 End of function SYS_MoveToActiveSRAM
 ******************************************************************************/

