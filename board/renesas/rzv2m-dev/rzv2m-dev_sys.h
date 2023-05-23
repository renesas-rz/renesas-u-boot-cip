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
 * File Name    : evk_sys.h
 * Description  : register and API information for SYS
 ******************************************************************************/

#ifndef EVK_SYS_H_
#define EVK_SYS_H_

#define SYS_BASE_ADDRESS            (0xA3F03000)


#define SYS_PERI0_BANK              (0x0030)
#define SYS_PERI1_BANK              (0x0034)
#define SYS_DRP_BANK                (0x0038)


#define SYS_MD_MON                  (0x0100)
#define SYS_VERSION                 (0x0104)
#define SYS_RAMA_ECC_CNT            (0x0110)
#define SYS_RAMA_INIT               (0x0114)
#define SYS_RAMA_INIT_END           (0x0118)


#define SYS_SRAM_SDM1               (0x0180)
#define SYS_SRAM_SDM2               (0x0184)
#define SYS_SRAM_SDM3               (0x0188)


#define SYS_SRAM_RS1                (0x0190)
#define SYS_SRAM_RS2                (0x0194)
#define SYS_SRAM_RS3                (0x0198)


#define BA00                            BIT(0)
#define BA01                            BIT(1)
#define BA02                            BIT(2)
#define BA03                            BIT(3)
#define BA04                            BIT(4)
#define BA05                            BIT(5)
#define BA06                            BIT(6)
#define BA07                            BIT(7)
#define BA08                            BIT(8)
#define BA09                            BIT(9)
#define BA10                            BIT(10)
#define BA11                            BIT(11)
#define BA12                            BIT(12)
#define BA13                            BIT(13)
#define BA14                            BIT(14)
#define BA15                            BIT(15)
#define BA_WE00                         BIT(16)
#define BA_WE01                         BIT(17)
#define BA_WE02                         BIT(18)
#define BA_WE03                         BIT(19)
#define BA_WE04                         BIT(20)
#define BA_WE05                         BIT(21)
#define BA_WE06                         BIT(22)
#define BA_WE07                         BIT(23)
#define BA_WE08                         BIT(24)
#define BA_WE09                         BIT(25)
#define BA_WE10                         BIT(26)
#define BA_WE11                         BIT(27)
#define BA_WE12                         BIT(28)
#define BA_WE13                         BIT(29)
#define BA_WE14                         BIT(30)
#define BA_WE15                         BIT(31)


typedef enum
{
    SYS_SRAM_CST        = 0x100,
    SYS_SRAM_SDI0,
    SYS_SRAM_SDI1,
    SYS_SRAM_EMMC,
    SYS_SRAM_DMY1,
    SYS_SRAM_DMY2,
    SYS_SRAM_USB_HOST,
    SYS_SRAM_USB_FUNC,
    SYS_SRAM_ETH0,
    SYS_SRAM_DMY3        = 0x10A,
    SYS_SRAM_DMY4       = 0x10C,
    SYS_SRAM_DMY5,
    SYS_SRAM_DMY6       = 0x200,
    SYS_SRAM_DMY7,
    SYS_SRAM_DMY8,
    SYS_SRAM_SRAMA      = 0x204,
    SYS_SRAM_SRAMB0     = 0x208,
    SYS_SRAM_SRAMB1,
    SYS_SRAM_SRAMB2,
    SYS_SRAM_SRAMB3,
    SYS_SRAM_RESERVE0       = 0x300,
    SYS_SRAM_RESERVE1,
    SYS_SRAM_RESERVE2,
    SYS_SRAM_RESERVE3,
    SYS_SRAM_RESERVE4,
    SYS_SRAM_RESERVE5,
    SYS_SRAM_RESERVE9       = 0x309,
    SYS_SRAM_RESERVE13      = 0x30D,
} e_sys_target_sram_t;

unsigned int SYS_ReadReg(unsigned int offset);
void SYS_WriteReg(unsigned int offset, unsigned int value);

void SYS_MoveToActiveSRAM(e_sys_target_sram_t target);

#endif  /* !defined(EVK_SYS_H_) */
