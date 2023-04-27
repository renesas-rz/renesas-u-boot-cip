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
* Copyright (C) 2021 Renesas Electronics Corporation. All rights reserved.
*******************************************************************************/ 
/*******************************************************************************
 * File Name    : rdk_pfc.h
 * Description  : register and API information for PFC
 ******************************************************************************/

#ifndef RDK_PFC_H_
#define RDK_PFC_H_

#define PFC_BASE_ADDRESS            (0xB6250000)

#define PFC_P00_GPIO_DO             (0x0000)
#define PFC_P00_GPIO_OE             (0x0004)
#define PFC_P00_GPIO_IE             (0x0008)
#define PFC_P00_PFSEL0              (0x0010)
#define PFC_P00_PFSEL1              (0x0014)
#define PFC_P00_PFSEL2              (0x0018)
#define PFC_P00_PFSEL3              (0x001C)
#define PFC_P00_DI_MON              (0x0020)
#define PFC_P00_PUPD                (0x0024)
#define PFC_P00_DRV                 (0x0028)
#define PFC_P00_SR                  (0x002C)
#define PFC_P00_DI_MSK              (0x0030)
#define PFC_P00_EN_MSK              (0x0034)

#define PFC_P01_GPIO_DO             (0x0040)
#define PFC_P01_GPIO_OE             (0x0044)
#define PFC_P01_GPIO_IE             (0x0048)
#define PFC_P01_PFSEL0              (0x0050)
#define PFC_P01_PFSEL1              (0x0054)
#define PFC_P01_PFSEL2              (0x0058)
#define PFC_P01_PFSEL3              (0x005C)
#define PFC_P01_DI_MON              (0x0060)
#define PFC_P01_PUPD                (0x0064)
#define PFC_P01_DRV                 (0x0068)
#define PFC_P01_SR                  (0x006C)
#define PFC_P01_DI_MSK              (0x0070)
#define PFC_P01_EN_MSK              (0x0074)

#define PFC_P02_GPIO_DO             (0x0080)
#define PFC_P02_GPIO_OE             (0x0084)
#define PFC_P02_GPIO_IE             (0x0088)
#define PFC_P02_PFSEL0              (0x0090)
#define PFC_P02_PFSEL1              (0x0094)
#define PFC_P02_DI_MON              (0x00A0)
#define PFC_P02_PUPD                (0x00A4)
#define PFC_P02_DRV                 (0x00A8)
#define PFC_P02_SR                  (0x00AC)
#define PFC_P02_DI_MSK              (0x00B0)
#define PFC_P02_EN_MSK              (0x00B4)

#define PFC_P03_GPIO_DO             (0x00C0)
#define PFC_P03_GPIO_OE             (0x00C4)
#define PFC_P03_GPIO_IE             (0x00C8)
#define PFC_P03_PFSEL0              (0x00D0)
#define PFC_P03_PFSEL1              (0x00D4)
#define PFC_P03_PFSEL2              (0x00D8)
#define PFC_P03_PFSEL3              (0x00DC)
#define PFC_P03_DI_MON              (0x00E0)
#define PFC_P03_PUPD                (0x00E4)
#define PFC_P03_DRV                 (0x00E8)
#define PFC_P03_SR                  (0x00EC)
#define PFC_P03_DI_MSK              (0x00F0)
#define PFC_P03_EN_MSK              (0x00F4)

#define PFC_P04_GPIO_DO             (0x0100)
#define PFC_P04_GPIO_OE             (0x0104)
#define PFC_P04_GPIO_IE             (0x0108)
#define PFC_P04_PFSEL0              (0x0110)
#define PFC_P04_PFSEL1              (0x0114)
#define PFC_P04_DI_MON              (0x0120)
#define PFC_P04_PUPD                (0x0124)
#define PFC_P04_DRV                 (0x0128)
#define PFC_P04_SR                  (0x012C)
#define PFC_P04_DI_MSK              (0x0130)
#define PFC_P04_EN_MSK              (0x0134)

#define PFC_P05_GPIO_DO             (0x0140)
#define PFC_P05_GPIO_OE             (0x0144)
#define PFC_P05_GPIO_IE             (0x0148)
#define PFC_P05_PFSEL0              (0x0150)
#define PFC_P05_DI_MON              (0x0160)
#define PFC_P05_PUPD                (0x0164)
#define PFC_P05_DRV                 (0x0168)
#define PFC_P05_SR                  (0x016C)
#define PFC_P05_DI_MSK              (0x0170)
#define PFC_P05_EN_MSK              (0x0174)

#define PFC_P06_GPIO_DO             (0x0180)
#define PFC_P06_GPIO_OE             (0x0184)
#define PFC_P06_GPIO_IE             (0x0188)
#define PFC_P06_PFSEL0              (0x0190)
#define PFC_P06_PFSEL1              (0x0194)
#define PFC_P06_PFSEL2              (0x0198)
#define PFC_P06_DI_MON              (0x01A0)
#define PFC_P06_PUPD                (0x01A4)
#define PFC_P06_DRV                 (0x01A8)
#define PFC_P06_SR                  (0x01AC)
#define PFC_P06_DI_MSK              (0x01B0)
#define PFC_P06_EN_MSK              (0x01B4)

#define PFC_P07_GPIO_DO             (0x01C0)
#define PFC_P07_GPIO_OE             (0x01C4)
#define PFC_P07_GPIO_IE             (0x01C8)
#define PFC_P07_PFSEL0              (0x01D0)
#define PFC_P07_PFSEL1              (0x01D4)
#define PFC_P07_DI_MON              (0x01E0)
#define PFC_P07_PUPD                (0x01E4)
#define PFC_P07_DRV                 (0x01E8)
#define PFC_P07_SR                  (0x01EC)
#define PFC_P07_DI_MSK              (0x01F0)
#define PFC_P07_EN_MSK              (0x01F4)

#define PFC_P08_GPIO_DO             (0x0200)
#define PFC_P08_GPIO_OE             (0x0204)
#define PFC_P08_GPIO_IE             (0x0208)
#define PFC_P08_PFSEL0              (0x0210)
#define PFC_P08_PFSEL1              (0x0214)
#define PFC_P08_DI_MON              (0x0220)
#define PFC_P08_PUPD                (0x0224)
#define PFC_P08_DRV                 (0x0228)
#define PFC_P08_SR                  (0x022C)
#define PFC_P08_DI_MSK              (0x0230)
#define PFC_P08_EN_MSK              (0x0234)

#define PFC_P09_GPIO_DO             (0x0240)
#define PFC_P09_GPIO_OE             (0x0244)
#define PFC_P09_GPIO_IE             (0x0248)
#define PFC_P09_PFSEL0              (0x0250)
#define PFC_P09_PFSEL1              (0x0254)
#define PFC_P09_DI_MON              (0x0260)
#define PFC_P09_PUPD                (0x0264)
#define PFC_P09_DRV                 (0x0268)
#define PFC_P09_SR                  (0x026C)
#define PFC_P09_DI_MSK              (0x0270)
#define PFC_P09_EN_MSK              (0x0274)

#define PFC_P10_GPIO_DO             (0x0280)
#define PFC_P10_GPIO_OE             (0x0284)
#define PFC_P10_GPIO_IE             (0x0288)
#define PFC_P10_PFSEL0              (0x0290)
#define PFC_P10_PFSEL1              (0x0294)
#define PFC_P10_PFSEL2              (0x0298)
#define PFC_P10_DI_MON              (0x02A0)
#define PFC_P10_PUPD                (0x02A4)
#define PFC_P10_DRV                 (0x02A8)
#define PFC_P10_SR                  (0x02AC)
#define PFC_P10_DI_MSK              (0x02B0)
#define PFC_P10_EN_MSK              (0x02B4)

#define PFC_P11_GPIO_DO             (0x02C0)
#define PFC_P11_GPIO_OE             (0x02C4)
#define PFC_P11_GPIO_IE             (0x02C8)
#define PFC_P11_PFSEL0              (0x02D0)
#define PFC_P11_PFSEL1              (0x02D4)
#define PFC_P11_PFSEL2              (0x02D8)
#define PFC_P11_DI_MON              (0x02E0)
#define PFC_P11_PUPD                (0x02E4)
#define PFC_P11_DRV                 (0x02E8)
#define PFC_P11_SR                  (0x02EC)
#define PFC_P11_DI_MSK              (0x02F0)
#define PFC_P11_EN_MSK              (0x02F4)

#define PFC_P12_GPIO_DO             (0x0300)
#define PFC_P12_GPIO_OE             (0x0304)
#define PFC_P12_GPIO_IE             (0x0308)
#define PFC_P12_PFSEL0              (0x0310)
#define PFC_P12_DI_MON              (0x0330)
#define PFC_P12_PUPD                (0x0334)
#define PFC_P12_DRV                 (0x0338)
#define PFC_P12_SR                  (0x033C)
#define PFC_P12_DI_MSK              (0x0330)
#define PFC_P12_EN_MSK              (0x0334)

#define PFC_P13_GPIO_DO             (0x0340)
#define PFC_P13_GPIO_OE             (0x0344)
#define PFC_P13_GPIO_IE             (0x0348)
#define PFC_P13_PFSEL0              (0x0350)
#define PFC_P13_PFSEL1              (0x0354)
#define PFC_P13_PFSEL2              (0x0358)
#define PFC_P13_DI_MON              (0x0360)
#define PFC_P13_PUPD                (0x0364)
#define PFC_P13_DRV                 (0x0368)
#define PFC_P13_SR                  (0x036C)
#define PFC_P13_DI_MSK              (0x0370)
#define PFC_P13_EN_MSK              (0x0374)

#define PFC_P14_GPIO_DO             (0x0380)
#define PFC_P14_GPIO_OE             (0x0384)
#define PFC_P14_GPIO_IE             (0x0388)
#define PFC_P14_PFSEL0              (0x0390)
#define PFC_P14_PFSEL1              (0x0394)
#define PFC_P14_DI_MON              (0x03A0)
#define PFC_P14_PUPD                (0x03A4)
#define PFC_P14_DRV                 (0x03A8)
#define PFC_P14_SR                  (0x03AC)
#define PFC_P14_DI_MSK              (0x03B0)
#define PFC_P14_EN_MSK              (0x03B4)

#define PFC_P15_GPIO_DO             (0x03C0)
#define PFC_P15_GPIO_OE             (0x03C4)
#define PFC_P15_GPIO_IE             (0x03C8)
#define PFC_P15_PFSEL0              (0x03D0)
#define PFC_P15_PFSEL1              (0x03D4)
#define PFC_P15_PFSEL2              (0x03D8)
#define PFC_P15_PFSEL3              (0x03DC)
#define PFC_P15_DI_MON              (0x03E0)
#define PFC_P15_PUPD                (0x03E4)
#define PFC_P15_DRV                 (0x03E8)
#define PFC_P15_SR                  (0x03EC)
#define PFC_P15_DI_MSK              (0x03F0)
#define PFC_P15_EN_MSK              (0x03F4)

#define PFC_P16_GPIO_DO             (0x0400)
#define PFC_P16_GPIO_OE             (0x0404)
#define PFC_P16_GPIO_IE             (0x0408)
#define PFC_P16_PFSEL0              (0x0410)
#define PFC_P16_PFSEL1              (0x0414)
#define PFC_P16_PFSEL2              (0x0418)
#define PFC_P16_PFSEL3              (0x041C)
#define PFC_P16_DI_MON              (0x0420)
#define PFC_P16_PUPD                (0x0424)
#define PFC_P16_DRV                 (0x0428)
#define PFC_P16_SR                  (0x042C)
#define PFC_P16_DI_MSK              (0x0430)
#define PFC_P16_EN_MSK              (0x0434)

#define PFC_P17_GPIO_DO             (0x0440)
#define PFC_P17_GPIO_OE             (0x0444)
#define PFC_P17_GPIO_IE             (0x0448)
#define PFC_P17_PFSEL0              (0x0450)
#define PFC_P17_DI_MON              (0x0460)
#define PFC_P17_PUPD                (0x0464)
#define PFC_P17_DRV                 (0x0468)
#define PFC_P17_SR                  (0x046C)
#define PFC_P17_DI_MSK              (0x0470)
#define PFC_P17_EN_MSK              (0x0474)

#define PFC_P20_GPIO_DO             (0x0500)
#define PFC_P20_GPIO_OE             (0x0504)
#define PFC_P20_GPIO_IE             (0x0508)
#define PFC_P20_PFSEL0              (0x0510)
#define PFC_P20_DI_MON              (0x0520)
#define PFC_P20_DRV                 (0x0528)
#define PFC_P20_DI_MSK              (0x0530)
#define PFC_P20_EN_MSK              (0x0534)

#define PFC_P21_GPIO_DO             (0x0540)
#define PFC_P21_GPIO_OE             (0x0544)
#define PFC_P21_GPIO_IE             (0x0548)
#define PFC_P21_DI_MON              (0x0560)
#define PFC_P21_DRV                 (0x0568)
#define PFC_P21_SR                  (0x056C)

#define PFC_CSRXD_SEL               (0x0580)
#define PFC_ROP_DI_SEL              (0x0584)

#define PFC_PEX0_DRV                (0x0590)
#define PFC_PEX0_SR                 (0x0594)

#define PFC_EXTINT_INV0             (0x05A0)
#define PFC_EXTINT_INV1             (0x05A4)
#define PFC_EXTINT_INV2             (0x05A8)

#define PFC_EXTINT_MSK0             (0x05B0)
#define PFC_EXTINT_MSK1             (0x05B4)
#define PFC_EXTINT_MSK2             (0x05B8)


/** Bit assign **/
#define PFC_PORT_PIN00              (0x00000001)
#define PFC_PORT_PIN01              (0x00000002)
#define PFC_PORT_PIN02              (0x00000004)
#define PFC_PORT_PIN03              (0x00000008)
#define PFC_PORT_PIN04              (0x00000010)
#define PFC_PORT_PIN05              (0x00000020)
#define PFC_PORT_PIN06              (0x00000040)
#define PFC_PORT_PIN07              (0x00000080)
#define PFC_PORT_PIN08              (0x00000100)
#define PFC_PORT_PIN09              (0x00000200)
#define PFC_PORT_PIN10              (0x00000400)
#define PFC_PORT_PIN11              (0x00000800)
#define PFC_PORT_PIN12              (0x00001000)
#define PFC_PORT_PIN13              (0x00002000)
#define PFC_PORT_PIN14              (0x00004000)
#define PFC_PORT_PIN15              (0x00008000)

#define PFC_CSRXD_RXD0_RXD0         (0x00000000)
#define PFC_CSRXD_RXD1_RXD1         (0x00000000)
#define PFC_CSRXD_RXD2_RXD2         (0x00000000)
#define PFC_CSRXD_RXD3_RXD3         (0x00000000)
#define PFC_CSRXD_RXD4_RXD4         (0x00000000)
#define PFC_CSRXD_RXD5_RXD5         (0x00000000)
#define PFC_CSRXD_RXD0_TXD0         (0x00000001)
#define PFC_CSRXD_RXD1_TXD1         (0x00000002)
#define PFC_CSRXD_RXD2_TXD2         (0x00000004)
#define PFC_CSRXD_RXD3_TXD3         (0x00000008)
#define PFC_CSRXD_RXD4_TXD4         (0x00000010)
#define PFC_CSRXD_RXD5_TXD5         (0x00000020)

#define PFC_CSRXD_RXD0_WE           (0x00010000)
#define PFC_CSRXD_RXD1_WE           (0x00020000)
#define PFC_CSRXD_RXD2_WE           (0x00040000)
#define PFC_CSRXD_RXD3_WE           (0x00080000)
#define PFC_CSRXD_RXD4_WE           (0x00100000)
#define PFC_CSRXD_RXD5_WE           (0x00200000)

#define PFC_CSRXD_RXD0              (0x0001)
#define PFC_CSRXD_RXD1              (0x0002)
#define PFC_CSRXD_RXD2              (0x0004)
#define PFC_CSRXD_RXD3              (0x0008)
#define PFC_CSRXD_RXD4              (0x0010)
#define PFC_CSRXD_RXD5              (0x0020)


#define PFC_ROP_DI_SEL9_PM1         (0x00000000)
#define PFC_ROP_DI_SEL9_P0609       (0x00000100)
#define PFC_ROP_DI_SEL9_GETXC       (0x00000200)

#define PFC_PEX0_NAWPN              (0x00000001)
#define PFC_PEX0_IM0CLK             (0x00000002)
#define PFC_PEX0_IM1CLK             (0x00000004)
#define PFC_PEX0_DETDO              (0x00000020)
#define PFC_PEX0_DETMS              (0x00000040)
#define PFC_PEX0_PCCLKREQB          (0x00000800)
#define PFC_PEX0_PCRSTOUTB          (0x00001000)
#define PFC_PEX0_USPWEN             (0x00004000)

typedef enum
{
    PFC_SUCCESSED                   = CMN_SUCCESS,
    PFC_ERROR_GENERAL               = -100,
    PFC_ERROR_NO_EXIST_REG          = -101,
    PFC_ERROR_NULL_POINTER          = -102,
    PFC_ERROR_INVALID_ARG           = -103
} e_pfc_result_t;

typedef union
{
    struct
    {
        unsigned char     sel[8];
    } byte;
    struct
    {
        unsigned short int    sel[4];
    } half;
    struct
    {
        unsigned int    sel[2];
    } word;
    struct
    {
        unsigned int    pin00:3;
        unsigned int    :1;
        unsigned int    pin01:3;
        unsigned int    :1;
        unsigned int    pin02:3;
        unsigned int    :1;
        unsigned int    pin03:3;
        unsigned int    :1;
        unsigned int    pin04:3;
        unsigned int    :1;
        unsigned int    pin05:3;
        unsigned int    :1;
        unsigned int    pin06:3;
        unsigned int    :1;
        unsigned int    pin07:3;
        unsigned int    :1;
        unsigned int    pin08:3;
        unsigned int    :1;
        unsigned int    pin09:3;
        unsigned int    :1;
        unsigned int    pin10:3;
        unsigned int    :1;
        unsigned int    pin11:3;
        unsigned int    :1;
        unsigned int    pin12:3;
        unsigned int    :1;
        unsigned int    pin13:3;
        unsigned int    :1;
        unsigned int    pin14:3;
        unsigned int    :1;
        unsigned int    pin15:3;
        unsigned int    :1;
    } func_sel;
} u_pfc_pfsel_t;

typedef union
{
    struct
    {
        unsigned char     drv_sel[4];
    } byte;
    struct
    {
        unsigned short int    drv_sel[2];
    } half;
    struct
    {
        unsigned int    drv_sel[1];
    } word;
    struct
    {
        unsigned int    pin00:2;
        unsigned int    pin01:2;
        unsigned int    pin02:2;
        unsigned int    pin03:2;
        unsigned int    pin04:2;
        unsigned int    pin05:2;
        unsigned int    pin06:2;
        unsigned int    pin07:2;
        unsigned int    pin08:2;
        unsigned int    pin09:2;
        unsigned int    pin10:2;
        unsigned int    pin11:2;
        unsigned int    pin12:2;
        unsigned int    pin13:2;
        unsigned int    pin14:2;
        unsigned int    pin15:2;
    } drv_sel;
} u_pfc_drvsel_t;

typedef union
{
    struct
    {
        unsigned char     pupd[4];
    } byte;
    struct
    {
        unsigned short int    pupd[2];
    } half;
    struct
    {
        unsigned int    pupd[1];
    } word;
    struct
    {
        unsigned int    pin00:2;
        unsigned int    pin01:2;
        unsigned int    pin02:2;
        unsigned int    pin03:2;
        unsigned int    pin04:2;
        unsigned int    pin05:2;
        unsigned int    pin06:2;
        unsigned int    pin07:2;
        unsigned int    pin08:2;
        unsigned int    pin09:2;
        unsigned int    pin10:2;
        unsigned int    pin11:2;
        unsigned int    pin12:2;
        unsigned int    pin13:2;
        unsigned int    pin14:2;
        unsigned int    pin15:2;
    } pupd;
} u_pfc_pupd_t;

typedef union
{
    struct
    {
        unsigned char     pin[4];
    } byte;
    struct
    {
        unsigned short int    pin[2];
    } half;
    struct
    {
        unsigned int    pin[1];
    } word;
    struct
    {
        unsigned int    pin00:1;
        unsigned int    pin01:1;
        unsigned int    pin02:1;
        unsigned int    pin03:1;
        unsigned int    pin04:1;
        unsigned int    pin05:1;
        unsigned int    pin06:1;
        unsigned int    pin07:1;
        unsigned int    pin08:1;
        unsigned int    pin09:1;
        unsigned int    pin10:1;
        unsigned int    pin11:1;
        unsigned int    pin12:1;
        unsigned int    pin13:1;
        unsigned int    pin14:1;
        unsigned int    pin15:1;
    } bit;
} u_pfc_port_t;

typedef union
{
    u_pfc_pfsel_t   func_sel;
    u_pfc_drvsel_t  drv_sel;
    u_pfc_pupd_t    pupd;
    u_pfc_port_t    pin;
    unsigned int        uw_data;
    unsigned short int        uh_data;
} u_pfc_data_t;

typedef enum
{
    PFC_PORT00 = 0,
    PFC_PORT01 = 1,
    PFC_PORT02 = 2,
    PFC_PORT03 = 3,
    PFC_PORT04 = 4,
    PFC_PORT05 = 5,
    PFC_PORT06 = 6,
    PFC_PORT07 = 7,
    PFC_PORT08 = 8,
    PFC_PORT09 = 9,
    PFC_PORT10 = 10,
    PFC_PORT11 = 11,
    PFC_PORT12 = 12,
    PFC_PORT13 = 13,
    PFC_PORT14 = 14,
    PFC_PORT15 = 15,
    PFC_PORT16 = 16,
    PFC_PORT17 = 17,
    PFC_PORT20 = 20,
    PFC_PORT21 = 21
} e_pfc_port_num_t;

typedef enum
{
    PFC_PORT_GPIO_DO = 0,
    PFC_PORT_GPIO_OE,
    PFC_PORT_GPIO_IE,
    PFC_PORT_PFSEL,
    PFC_PORT_DI_MON,
    PFC_PORT_PUPD,
    PFC_PORT_DRV,
    PFC_PORT_SR,
    PFC_PORT_DI_MSK,
    PFC_PORT_EN_MSK
} e_pfc_kind_reg_t;

typedef union
{
    struct
    {
        unsigned char     drv[4];
    } byte;
    struct
    {
        unsigned short int    drv[2];
    } half;
    struct
    {
        unsigned int    drv[1];
    } word;
    struct
    {
        unsigned int    nawpn:2;
        unsigned int    im0clk:2;
        unsigned int    im1clk:2;
        unsigned int    :4;
        unsigned int    detdo:2;
        unsigned int    detms:2;
        unsigned int    :8;
        unsigned int    pcclkreqb:2;
        unsigned int    pcrstoutb:2;
        unsigned int    :2;
        unsigned int    uspwen:2;
        unsigned int    :2;
    } drv;
} u_pfc_pex_drv_t;

typedef union
{
    struct
    {
        unsigned char     pin[4];
    } byte;
    struct
    {
        unsigned short int    pin[2];
    } half;
    struct
    {
        unsigned int    pin[1];
    } word;
    struct
    {
        unsigned int    nawpn:1;
        unsigned int    im0clk:1;
        unsigned int    im1clk:1;
        unsigned int    :2;
        unsigned int    detdo:1;
        unsigned int    detms:1;
        unsigned int    :4;
        unsigned int    pcclkreqb:1;
        unsigned int    pcrstoutb:1;
        unsigned int    :1;
        unsigned int    uspwen:1;
        unsigned int    :1;
    } pin;
} u_pfc_pex_t;

typedef enum
{
    PFC_EXTINT0_INV = 0,
    PFC_EXTINT1_INV = 1,
    PFC_EXTINT2_INV = 2,
    PFC_EXTINT0_MSK = 4,
    PFC_EXTINT1_MSK = 5,
    PFC_EXTINT2_MSK = 6
} e_pfc_extint_reg_num_t;

typedef enum
{
    PFC_SELECT_EMM= 0,
    PFC_SELECT_URT0,
    PFC_SELECT_URT1,
    PFC_SELECT_ETH0,
    PFC_SELECT_SDI0,
    PFC_SELECT_SDI1,
    PFC_SELECT_IIC00,
	PFC_SELECT_PWM0_7,
    PFC_SELECT_PWM8_15,
	PFC_SELECT_CSI2,
    PFC_SELECT_CSI3,
    PFC_SELECT_CSI4,
    PFC_SELECT_CSI5,
    PFC_SELECT_NUM
} e_pfc_select_ip_t;

unsigned int PFC_ReadReg(unsigned int offset);
void PFC_WriteReg(unsigned int offset, unsigned int value);

int PFC_SetPortParam(e_pfc_port_num_t port_num, e_pfc_kind_reg_t kind_reg,
    unsigned short int target, u_pfc_data_t *p_set_data);
int PFC_GetPortParam(e_pfc_port_num_t port_num, e_pfc_kind_reg_t kind_reg,
    u_pfc_data_t *p_data);
int PFC_SetPortParamH(e_pfc_port_num_t port_num, e_pfc_kind_reg_t kind_reg,
    unsigned short int target, unsigned short int set_data);

static inline int PFC_SetGPIO_DO(e_pfc_port_num_t port_num,
    unsigned short int target, unsigned short int set_data)
{
    return PFC_SetPortParamH(port_num, PFC_PORT_GPIO_DO, target, set_data);
}

static inline int PFC_GetGPIO_DO(e_pfc_port_num_t port_num,
    unsigned int *p_data)
{
    return PFC_GetPortParam(port_num, PFC_PORT_GPIO_DO,
                (u_pfc_data_t *)p_data);
}

static inline int PFC_SetGPIO_OE(e_pfc_port_num_t port_num,
    unsigned short int target, unsigned short int set_data)
{
    return PFC_SetPortParamH(port_num, PFC_PORT_GPIO_OE, target, set_data);
}

static inline int PFC_GetGPIO_OE(e_pfc_port_num_t port_num,
    unsigned int *p_data)
{
    return PFC_GetPortParam(port_num, PFC_PORT_GPIO_OE,
                (u_pfc_data_t *)p_data);
}

static inline int PFC_SetGPIO_IE(e_pfc_port_num_t port_num,
    unsigned short int target, unsigned short int set_data)
{
    return PFC_SetPortParamH(port_num, PFC_PORT_GPIO_IE, target, set_data);
}

static inline int PFC_GetGPIO_IE(e_pfc_port_num_t port_num,
    unsigned int *p_data)
{
    return PFC_GetPortParam(port_num, PFC_PORT_GPIO_OE,
                (u_pfc_data_t *)p_data);
}

static inline int PFC_SetPFSEL(e_pfc_port_num_t port_num,
    unsigned short int target, u_pfc_pfsel_t *p_set_data)
{
    return PFC_SetPortParam(port_num, PFC_PORT_PFSEL, target,
                (u_pfc_data_t *)p_set_data);
}

static inline int PFC_GetPFSEL(e_pfc_port_num_t port_num,
    u_pfc_pfsel_t *p_data)
{
    return PFC_GetPortParam(port_num, PFC_PORT_PFSEL,
                (u_pfc_data_t *)p_data);
}

static inline int PFC_GetDI_MON(e_pfc_port_num_t port_num,
    unsigned int *p_data)
{
    return PFC_GetPortParam(port_num, PFC_PORT_DI_MON,
                (u_pfc_data_t *)p_data);
}

static inline int PFC_SetPUPD(e_pfc_port_num_t port_num,
    unsigned short int target, u_pfc_pupd_t *p_set_data)
{
    return PFC_SetPortParam(port_num, PFC_PORT_PUPD, target,
            (u_pfc_data_t *)p_set_data);
}

static inline int PFC_GetPUPD(e_pfc_port_num_t port_num, unsigned int *p_data)
{
    return PFC_GetPortParam(port_num, PFC_PORT_PUPD,
            (u_pfc_data_t *)p_data);
}

static inline int PFC_SetDRV(e_pfc_port_num_t port_num,
    unsigned short int target, u_pfc_drvsel_t *p_set_data)
{
    return PFC_SetPortParam(port_num, PFC_PORT_DRV, target,
            (u_pfc_data_t *)p_set_data);
}

static inline int PFC_GetDRV(e_pfc_port_num_t port_num, unsigned int *p_data)
{
    return PFC_GetPortParam(port_num, PFC_PORT_DRV, (u_pfc_data_t *)p_data);
}

static inline int PFC_SetSR(e_pfc_port_num_t port_num,
    unsigned short int target, unsigned short int set_data)
{
    return PFC_SetPortParamH(port_num, PFC_PORT_SR, target, set_data);
}

static inline int PFC_GetSR(e_pfc_port_num_t port_num, unsigned int *p_data)
{
    return PFC_GetPortParam(port_num, PFC_PORT_SR, (u_pfc_data_t *)p_data);
}

static inline int PFC_SetDI_MSK(e_pfc_port_num_t port_num,
    unsigned short int target, unsigned short int set_data)
{
    return PFC_SetPortParamH(port_num, PFC_PORT_DI_MSK, target, set_data);
}

static inline int PFC_GetDI_MSK(e_pfc_port_num_t port_num,
    unsigned int *p_data)
{
    return PFC_GetPortParam(port_num, PFC_PORT_DI_MSK,
                (u_pfc_data_t *)p_data);
}

static inline int PFC_SetEN_MSK(e_pfc_port_num_t port_num,
    unsigned short int target, unsigned short int set_data)
{
    return PFC_SetPortParamH(port_num, PFC_PORT_EN_MSK, target, set_data);
}

static inline int PFC_GetEN_MSK(e_pfc_port_num_t port_num,
    unsigned int *p_data)
{
    return PFC_GetPortParam(port_num, PFC_PORT_EN_MSK,
                (u_pfc_data_t *)p_data);
}


int PFC_SetCXRXD_SEL(unsigned short int target, unsigned short int set_data);
int PFC_GetCXRXD_SEL(unsigned short int *p_data);

int PFC_SetROP_DI_SEL(unsigned int set_data);
int PFC_GetROP_DI_SEL(unsigned int *p_data);

int PFC_SetPEXDRV(unsigned short int target, u_pfc_pex_drv_t *p_set_data);
int PFC_GetPEXDRV(unsigned int *p_data);

int PFC_SetPEXSR(unsigned short int target, unsigned short int set_data);
int PFC_GetPEXSR(unsigned int *p_data);

int PFC_SetEXTINT(e_pfc_extint_reg_num_t reg_num, unsigned short int target_pin,
        unsigned short int set_data);
int PFC_GetEXTINT(e_pfc_extint_reg_num_t reg_num, unsigned int *p_data);

int PFC_SetPinFunc(e_pfc_select_ip_t tgt_ip);
#endif  /* !defined(RDK_PFC_H_) */
