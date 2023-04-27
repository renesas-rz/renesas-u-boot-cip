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
#include <common.h>
#include <malloc.h>
#include <fdtdec.h>
#include <linux/errno.h>
#include <asm/system.h>
#include "rdk_common.h"
#include "rdk_pfc.h"
#include "rdk_cmn_cpg.h"
#include "rdk_usbtest.h"
#include "rdk_sys.h"
#include "rdk_psc.h"
#include <init.h>

/*
TARGET_BANK0_UNIT_LINUX settings as follows:
               BA_WE00 | BA00  //EMM
               BA_WE02 | BA02  //SDI0
               BA_WE04 | BA04  //SDI1
               BA_WE08 | BA08  //USB HOST
               BA_WE10 | BA10  //USB PERI
               BA_WE12 | BA12  //PCI
*/
#define TARGET_BANK0_UNIT_LINUX        (BA_WE00 | BA00 | \
                                                                       BA_WE02 | BA02 | \
                                                                       BA_WE04 | BA04 | \
                                                                       BA_WE08 | BA08 | \
                                                                       BA_WE10 | BA10 | \
                                                                       BA_WE12 | BA12)


#define TARGET_BANK1_UNIT_LINUX        (BA_WE00 | BA00)       //ETH0 BANK
/*
       TARGET_DRP_UNIT_LINUX settings as follows:
               BA_WE00 | BA00  //DRPA M0
               BA_WE02 | BA02  //DRPA M1
               BA_WE04 | BA04  //DRPA M2
               BA_WE06 | BA06  //DRPA M3
*/
#define TARGET_DRP_UNIT_LINUX  (BA_WE00 | BA00 | \
                                                                       BA_WE02 | BA02 | \
                                                                       BA_WE04 | BA04 | \
                                                                       BA_WE06 | BA06)


DECLARE_GLOBAL_DATA_PTR;

static void rdk_pfc_init(void)
{
	do
	{
		PFC_SetPinFunc(PFC_SELECT_URT0);
		PFC_SetPinFunc(PFC_SELECT_URT1);
		PFC_SetPinFunc(PFC_SELECT_EMM);
		PFC_SetPinFunc(PFC_SELECT_SDI0);
		PFC_SetPinFunc(PFC_SELECT_ETH0);
		PFC_WriteReg(PFC_P17_GPIO_DO, 0x10001);
		CMN_DelayInUS(50);
		PFC_SetPinFunc(PFC_SELECT_IIC00);
		PFC_SetPinFunc(PFC_SELECT_PWM0_7);
		PFC_SetPinFunc(PFC_SELECT_PWM8_15);
		PFC_SetPinFunc(PFC_SELECT_CSI2);
		PFC_SetPinFunc(PFC_SELECT_CSI3);
		PFC_SetPinFunc(PFC_SELECT_CSI4);
		PFC_SetPinFunc(PFC_SELECT_CSI5);
	}
	while (0);
	return;
}

static void drp_init(void){

	uint32_t regval;
	
    //Initialize the PLL6
	regval = CMN_REG_Read32(0xa350003c);
	while((regval & 0x11) != 0x00){
		regval = CMN_REG_Read32(0xa350003c);
		CMN_DelayInUS(1);
	}
	CMN_REG_Write32(0xa3500034, 0x00001a42);
	CMN_REG_Write32(0xA3500038, 0x00150A01);

	//Release standby mode with PLL6
	CMN_REG_Write32(0xA3500030, 0x00010001);
	regval = CMN_REG_Read32(0xa350003c);
	while((regval & 0x11) != 0x11){
		regval = CMN_REG_Read32(0xa350003c);
		CMN_DelayInUS(1);
	}

    //Power on with PD_DRPA
	CMN_REG_Write32(0xA3600018, 0x0BB70BB7);
	CMN_REG_Write32(0xa3600000, 0x00000015);
	regval = CMN_REG_Read32(0xa3600044);
	while((regval & 0x1) != 0x1){
		regval = CMN_REG_Read32(0xa3600044);
		CMN_DelayInUS(1000);
	}

	CMN_REG_Write32(0xa3600000, 0x00000016);
	regval = CMN_REG_Read32(0xa3600044);
	while((regval & 0x1) != 0x1){
		regval = CMN_REG_Read32(0xa3600044);
		CMN_DelayInUS(1);
	}

    //Clear interrupt status with PMC
	CMN_REG_Write32(0xa3600048, 0x00000001);

    //Clock supply with PD_DRP
	CMN_REG_Write32(0xa350044c,0x00040004);
	CMN_DelayInUS(1000);
	CMN_REG_Write32(0xa3500450,0x00040004);
	CMN_DelayInUS(1000);

    //Isolation off with DRP
	CMN_REG_Write32(0xa360002c, 0x00000000);
	regval = CMN_REG_Read32(0xa360002c);
	while((regval & 0x8000) != 0x8000){
		regval = CMN_REG_Read32(0xa360002c);
		CMN_DelayInUS(1000);
	}

	CMN_REG_Write32(0xa3600030, 0x00000000);
	regval = CMN_REG_Read32(0xa3600030);
	while((regval & 0x8000) != 0x8000){
		regval = CMN_REG_Read32(0xa360002c);
		CMN_DelayInUS(1000);
	}

	//Clock supply and deassert with DRP
	CMN_REG_Write32(0xa350044c, 0x00030003);
	CMN_DelayInUS(10000);
	CMN_REG_Write32(0xa3500450, 0x00030003);
	CMN_DelayInUS(10000);

	CMN_REG_Write32(0xa3500620, 0x00010001);
	CMN_REG_Write32(0xa3500624, 0x00010001);
	CMN_DelayInUS(1);

}

static void rdk_cpg_init(void)
{
	int t;
	do
	{	
		/* select  DEVB(400MHz) ,select DEVD(200MHz) and select DEVE(100MHz) */
		CPG_WriteReg(CPG_SYS_DDIV, (CPG_SYS_DDIV_WEN_DIVB|CPG_SYS_DDIV_WEN_DIVE|CPG_SYS_DDIV_WEN_DIVD));
		t = 0;
		while (1000000 > t++)
		{
			if (0 == (CPG_ReadReg(CPG_CLKSTATUS) & CPG_CLKSTATUS_DIVX))
			{
				break;
			}
			CMN_DelayInUS(10);
		}

		/**********************************/
		/**** URT *******************/
		/**********************************/
		/* select URT clock source(48MHz) */
		//CPG_WriteReg(CPG_URT_RCLK_SSEL, CPG_URT_RCLK_SSEL_WEN_SELW0 | CPG_URT_RCLK_SSEL_WEN_SELW1);

		/* enable supply clock to URT */
		CPG_SetClockCtrl(15, 0x0070, 0x0070);
		CMN_DelayInUS(1);

		/* asset RESET to URT0,1 */
		CPG_SetResetCtrl(6, 0x0400, 0x0000);
		CPG_WaitResetMon(/*500000*/0, CPG_RST_MON_URT, CPG_RST_MON_URT);

		/* deasset RESET to URT0,1 */
		CPG_SetResetCtrl(6, 0x0400, 0x0400);
		CPG_WaitResetMon(/*500000*/0, CPG_RST_MON_URT, 0);

		/**********************************/
		/**** SDI0 EMM ************/
		/**********************************/
		/* select SDI0 EMM clock source(200MHz) */
		CPG_WriteReg(CPG_SDIEMM_SSEL, (CPG_SDIEMM_SSEL_SELSDI|CPG_SDIEMM_SSEL_WEN_SELSDI));

		/* enable supply clock */
		CPG_SetClockCtrl(3, 0x0f0f, 0x0f0f);
		CMN_DelayInUS(1);

		/* asset RESET */
		CPG_SetResetCtrl(3, 0x0005, 0x0000);
		CPG_WaitResetMon(0, CPG_RST_MON_SDI0, CPG_RST_MON_SDI0);
		CPG_WaitResetMon(0, CPG_RST_MON_EMM,  CPG_RST_MON_EMM);

		/* deasset RESET */
		CPG_SetResetCtrl(3, 0x0005, 0x0005);
		CPG_WaitResetMon(0, CPG_RST_MON_SDI0, 0);
		CPG_WaitResetMon(0, CPG_RST_MON_EMM,  0);

		/**********************************/
		/**** USB *******************/
		/**********************************/
		/* select USB DEVD(200MHz) and select USB DEVE(100MHz) */
		//CPG_WriteReg(CPG_SYS_DDIV, (CPG_SYS_DDIV_WEN_DIVE|CPG_SYS_DDIV_WEN_DIVD));	
		// while (true)
		// {
		// 	if (0 == (CPG_ReadReg(CPG_CLKSTATUS) & CPG_CLKSTATUS_DIVX))
		// 	{
		// 		break;
		// 	}
		// 	CMN_DelayInUS(10);
		// }

		/* usb reset assart */
		CPG_SetResetCtrl(3, 0x0780, 0x0000);
		CMN_DelayInUS(1);//50ns

		/* usb clock off */
		CPG_SetClockCtrl(4, 0x0070, 0x0000);
		CMN_DelayInUS(1);//125ns

		/* usb reset assart */
		CPG_SetResetCtrl(3, 0x0780, 0x0780);
		CMN_DelayInUS(1);//50ns

		/* usb clock on */
		CPG_SetClockCtrl(4, 0x0070, 0x0070);
		CMN_DelayInUS(1);//125ns

		/**********************************/
		/**** IIC0-3****************/
		/**********************************/
		/* asset RESET */
		CPG_SetResetCtrl(6, 0x0300, 0x0000);
		CMN_DelayInUS(1);

		/* disable supply clock */
		CPG_SetClockCtrl(9, 0x1000, 0x0000);
		CPG_SetClockCtrl(10, 0x1000, 0x0000);
		CMN_DelayInUS(1);

		/* deasset RESET */
		CPG_SetResetCtrl(6, 0x0300, 0x0300);
		CMN_DelayInUS(1);

		/* enable supply clock */
		CPG_SetClockCtrl(9, 0x1000, 0x1000);
		CPG_SetClockCtrl(10, 0x1000, 0x1000);
		CMN_DelayInUS(1);

		/**********************************/
		/**** ETH0 ****************/
		/**********************************/

		/* eth0 clock on */
		CPG_SetClockCtrl(4, 0x0300, 0x0300);
		CPG_SetClockCtrl(16, 0x2000, 0x2000);
		CMN_DelayInUS(1);

		/* eth0 reset assart */
		CPG_SetResetCtrl(3, 0x0800, 0x0000);
		CPG_WaitResetMon(0, CPG_RST_MON_ETH0, CPG_RST_MON_ETH0);

		/* eth0 reset deassert */
		CPG_SetResetCtrl(3, 0x0800, 0x0800);
		CPG_WaitResetMon(0, CPG_RST_MON_ETH0, 0);

		/**********************************/
		/**** PWM0-15 ****************/
		/**********************************/
		CPG_SetClockCtrl(13, 0x0FF1, 0x0FF1);
		CPG_SetClockCtrl(14, 0x0FF1, 0x0FF1);
		CMN_DelayInUS(1);
		
		/* PWM0-7 reset assart */
		CPG_SetResetCtrl(6, 0x0030, 0x0000);
		CPG_WaitResetMon(0, CPG_RST_MON_PWM_0 | CPG_RST_MON_PWM_1, CPG_RST_MON_PWM_0 | CPG_RST_MON_PWM_1);
		
		/* PWM0-7 reset deassert */
		CPG_SetResetCtrl(6, 0x0030, 0x0030);
		CPG_WaitResetMon(0, CPG_RST_MON_PWM_0 | CPG_RST_MON_PWM_1, 0);

		/**********************************/
		/**** CSI2-5 ****************/
		/**********************************/
		/* select CSI REF */
		//Keep defalt setting
		//CPG_WriteReg(CPG_CSI_RCLK_SSEL, (CPG_CSI_RCLK_SSEL_WEN_SELCSI2|CPG_CSI_RCLK_SSEL_WEN_SELCSI3|CPG_CSI_RCLK_SSEL_WEN_SELCSI4|CPG_CSI_RCLK_SSEL_WEN_SELCSI5));

		CPG_SetClockCtrl(15, 0x3C03, 0x3C03);
		CMN_DelayInUS(1);
		
		/* CSI4 reset assart */
		CPG_SetResetCtrl(6, 0x00C0, 0x0000);
		CPG_WaitResetMon(0, CPG_RST_MON_CSI_1|CPG_RST_MON_CSI_2, CPG_RST_MON_CSI_1|CPG_RST_MON_CSI_2);
		
		/* CSI4 reset deassert */
		CPG_SetResetCtrl(6, 0x00C0, 0x00C0);
		CPG_WaitResetMon(0, CPG_RST_MON_CSI_1|CPG_RST_MON_CSI_2, 0);

	    
		CPG_SetClockCtrl(4, 0x0007, 0x0007);
		CMN_DelayInUS(1);
		CPG_SetResetCtrl(3, 0x1000, 0x0000);
		CMN_DelayInUS(1);
		CPG_SetResetCtrl(3, 0x1000, 0x1000);
		CMN_DelayInUS(1);
		
		/**********************************/
		/**** TIM0-31 ****************/
		/**********************************/
		/* TIM group A-D reset assart */
		CPG_SetResetCtrl(6, 0x000F, 0x0000);
		CMN_DelayInUS(1);
		
		/* TIM group A and TIM 0-7 clock off */
		CPG_SetClockCtrl(9, 0x0FF1, 0x0000);
		/* TIM group B and TIM 8-15 clock off */
		CPG_SetClockCtrl(10, 0x0FF1, 0x0000);
		/* TIM group C and TIM 16-23 clock off */
		CPG_SetClockCtrl(11, 0x0FF1, 0x0000);
		/* TIM group D and TIM 24-31 clock off */
		CPG_SetClockCtrl(12, 0x0FF1, 0x0000);
		CMN_DelayInUS(1);

		/* TIM group A-D reset deassert */
		CPG_SetResetCtrl(6, 0x000F, 0x000F);
		CMN_DelayInUS(1);

		/* TIM group A and TIM 0-7 clock on */
		CPG_SetClockCtrl(9, 0x0FF1, 0x0FF1);
		/* TIM group B and TIM 8-15 clock on */
		CPG_SetClockCtrl(10, 0x0FF1, 0x0FF1);
		/* TIM group C and TIM 16-23 clock on */
		CPG_SetClockCtrl(11, 0x0FF1, 0x0FF1);
		/* TIM group D and TIM 24-31 clock on */
		CPG_SetClockCtrl(12,0x0FF1, 0x0FF1);
		CMN_DelayInUS(1);
		
		/**********************************/
		/**** DMAA ****************/
		/**********************************/

		/* DMAA clock on */ 
		CPG_SetClockCtrl(1, 0x0800, 0x0800);
			
		/* DMAA reset assart */
		CPG_SetResetCtrl(1, 0x0080, 0x0000);
		CPG_WaitResetMon(0, CPG_RST_MON_DMAA,CPG_RST_MON_DMAA);
		
		/* DMAA reset deassert */
		CPG_SetResetCtrl(1, 0x0080, 0x0080);
		CPG_WaitResetMon(0, CPG_RST_MON_DMAA,0);


		/**********************************/
		/**** DRPA ****************/
		/**********************************/
		drp_init();
	}
	while (0);
	return;
}

static void rdk_module_init(void)
{
	//USB TEST Module initialize
	CMN_REG_Write32(USBTEST_RESET2,  0x00000001);
	CMN_DelayInUS(10);
	CMN_REG_Write32(USBTEST_CLKRST2, 0x00000005);
	CMN_REG_Write32(USBTEST_CLKRST2, 0x00000004);
	CMN_DelayInUS(20);
	CMN_REG_Write32(USBTEST_RESET1,  0x00000300);
	CMN_REG_Write32(USBTEST_RESET1,  0x00000000);
	CMN_DelayInUS(75);

}
int32_t ilStartUpUART(void)
{
	return CMN_SUCCESS;
}

extern void _start(void);

void s_init(void)
{

	if(3 == current_el()){

		armv8_switch_to_el2((u64)NULL, 0, 0, 0,
							_start,
							ES_TO_AARCH64);
	}
	
	return;
}

int board_early_init_f(void)
{
    
	rdk_pfc_init();
    
	/* move to active for SRAM in ETH0 */
	SYS_MoveToActiveSRAM(SYS_SRAM_ETH0);
    
	rdk_cpg_init();

	rdk_module_init();
    
	rdk_psc_init();
	
	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_TEXT_BASE + 0x50000;

	/* init UART */
	//serial_init();

	return 0;
}

int dram_init(void)
{
	if (fdtdec_setup_mem_size_base() != 0)
		return -EINVAL;

	return 0;
}

int dram_init_banksize(void)
{
	fdtdec_setup_memory_banksize();

	return 0;
}

void reset_cpu(ulong addr)
{
}

void board_cleanup_before_linux(void)
{
       //PERI0 BANK SETTING
       SYS_WriteReg(SYS_PERI0_BANK, TARGET_BANK0_UNIT_LINUX);
       //PERI1 BANK SETTING
       SYS_WriteReg(SYS_PERI1_BANK, TARGET_BANK1_UNIT_LINUX);
       //DRP BANK SETTING
       SYS_WriteReg(SYS_DRP_BANK, TARGET_DRP_UNIT_LINUX);

}
