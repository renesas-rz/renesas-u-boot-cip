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
 * File Name    : evk_common.h
 ******************************************************************************/

#ifndef EVK_COMMON_H
#define EVK_COMMON_H

/*
 * Macro definitions
 */
#ifndef NULL
#define NULL    ((void*)0)
#endif  /* !defined(NULL) */

/*
 * Global Typedef definitions
 */
typedef enum
{
    CMN_SUCCESS = 0,
    CMN_ERROR   = -1
} e_evk_cmn_error_no_t;

/******************************************************************************
 Prototype define
 *****************************************************************************/
void CMN_DelayInUSec(unsigned long int us);
unsigned long int CMN_GetSysCnt(void);
unsigned int CMN_GetFreq4SysCnt(void);

/*******************************************************************************
 * Function Name: CMN_REG_Read32
 * Description  : read access to Register in unsigned int.
 *
 * Arguments    : addr -
 *                    address for read access
 * Return Value : value -
 *                    read data.
 ******************************************************************************/
static inline unsigned int CMN_REG_Read32(unsigned int addr)
{
    /** pointer is (unsigned int *) */
    return *((volatile unsigned int *)((unsigned long)addr));
}

/*******************************************************************************
 * Function Name: CMN_REG_Write32
 * Description  : write access to Register in unsigned int.
 *
 * Arguments    : addr -
 *                    address for write access
 *                value -
 *                    write data
 * Return Value : non -
 *
 ******************************************************************************/
static inline void CMN_REG_Write32(unsigned int addr, unsigned int value)
{
    /** sizeof(unsigned int) == sizeof(unsigned int *) */
    *((volatile unsigned int *)((unsigned long)addr)) = value;
}

/*******************************************************************************
 * Function Name: CMN_REG_Read64
 * Description  : read access to Register in unsigned long int.
 *
 * Arguments    : addr -
 *                    address for read access
 * Return Value : value -
 *                    read data.
 ******************************************************************************/
static inline unsigned long int CMN_REG_Read64(unsigned int addr)
{
    /** sizeof(unsigned int) == sizeof(unsigned long int *) */
    return *((volatile unsigned long int *)((unsigned long)addr));
}

/*******************************************************************************
 * Function Name: CMN_REG_Write64
 * Description  : write access to Register in unsigned long int.
 *
 * Arguments    : addr -
 *                    address for write access
 *                value -
 *                    write data
 * Return Value : non -
 *
 ******************************************************************************/
static inline void CMN_REG_Write64(unsigned int addr, unsigned long int value)
{
    /** sizeof(unsigned int) == sizeof(unsigned long int *) */
    *((volatile unsigned long int *)((unsigned long)addr)) = value;
}

/*******************************************************************************
 * Function Name: CMN_DelayInUS
 * Description  : Does an example task. Making this longer just to see how it
 *                wraps.
 * Arguments    : index -
 *                    Where to start looking
 *                p_output -
 *                    Pointer of where to put the output data
 * Return Value : count -
 *                    How many entries were found
 ******************************************************************************/
static inline void CMN_DelayInUS(unsigned long int us)
{
    CMN_DelayInUSec(us);
}

#endif /* !defined EVK_COMMON_H */
