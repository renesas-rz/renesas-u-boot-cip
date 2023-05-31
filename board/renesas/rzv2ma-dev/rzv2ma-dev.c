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
#include <common.h>
#include <malloc.h>
#include <fdtdec.h>
#include <linux/errno.h>
#include <asm/system.h>
#include "rzv2ma-dev_common.h"
#include "rzv2ma-dev_cmn_cpg.h"
#include "rzv2ma-dev_sys.h"
#include "rzv2ma-dev_psc.h"
#include <init.h>

DECLARE_GLOBAL_DATA_PTR;

static void drp_a_init(void){

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
	
	//Clear interrupt status with PMC
	CMN_REG_Write32(0xa3600048, 0x00000001);
	
	//Clock supply with PD_DRP
	CMN_REG_Write32(0xa350044c,0x00040004);
	CMN_DelayInUS(1000);
	
	//Isolation off with DRP
	CMN_REG_Write32(0xa360002c, 0x00000000);
	regval = CMN_REG_Read32(0xa360002c);
	while((regval & 0x8000) != 0x8000){
		regval = CMN_REG_Read32(0xa360002c);
		CMN_DelayInUS(1000);
	}
	//Clock supply and deassert with DRP
	CMN_REG_Write32(0xa350044c, 0x00030003);
	CMN_DelayInUS(10000);
	
	CMN_REG_Write32(0xa3500620, 0x00010001);
	CMN_DelayInUS(1);

}

static void drp_b_init(void){

	uint32_t regval;

	CMN_REG_Write32(0xa3600000, 0x00000016);
	regval = CMN_REG_Read32(0xa3600044);
	while((regval & 0x1) != 0x1){
		regval = CMN_REG_Read32(0xa3600044);
		CMN_DelayInUS(1);
	}
	
	//Clear interrupt status with PMC
	CMN_REG_Write32(0xa3600048, 0x00000001);

	CMN_REG_Write32(0xa3500450,0x00040004);
	CMN_DelayInUS(1000);

	//Isolation off with DRP
	CMN_REG_Write32(0xa3600030, 0x00000000);
	regval = CMN_REG_Read32(0xa3600030);
	while((regval & 0x8000) != 0x8000){
		regval = CMN_REG_Read32(0xa360002c);
		CMN_DelayInUS(1000);
	}
	
	//Clock supply and deassert with DRP
	CMN_REG_Write32(0xa3500450, 0x00030003);
	CMN_DelayInUS(10000);

	CMN_REG_Write32(0xa3500624, 0x00010001);
	CMN_DelayInUS(1);

}

static void vcd_init(void)
{
	uint32_t regval;

	// Power on with PD_VIDEO1
	// control PMC reg
	CMN_REG_Write32(0xa3600018, 0x00000BB7);   /* Set PD_ON_TIM bit     */
	regval = CMN_REG_Read32(0xa3600004);       /* Get PD_BUSY bit       */
	while(regval & 0x8000)
	{
		regval = CMN_REG_Read32(0xa3600004);   /* Get PD_BUSY bit       */
		CMN_DelayInUS(1000);
	}

	CMN_REG_Write32(0xa3600000, 0x00000013);   /* Set SPLY_ENA bit      */
	regval = CMN_REG_Read32(0xa3600044);       /* Get PD_NUM bit        */

	while ((regval & 0x1) != 0x1) 
	{
		regval = CMN_REG_Read32(0xa3600044);
		CMN_DelayInUS(1000);
	}

	CMN_REG_Write32(0xa3600048, 0x00000001);   /* Set PMC_INT_CLR bit   */

	/* Power domain control : CPG_PD_RST reg : release PD_VD1A/B, PD_VD0 reset */
	CPG_SetPDResetCtrl(0x000E, 0x000E);

	/* disable supply clock : use 400MHz no settiing */
	CPG_SetClockCtrl(26, 0x0F20, 0x0F20);      /* Set VCD_A/PCLK, JPG0/1_ACLK  */

	CMN_REG_Write32(0xa3600024, 0x00000000);   /* Clear PD_ISO_EN bit(D0 bit)  */
	regval = CMN_REG_Read32(0xa3600024);       /* Get PD_ISO_DONE bit(D15 bit) */

	while((regval & 0x8000) != 0x8000)
	{
		regval = CMN_REG_Read32(0xa3600024);   /* Get PD_ISO_DONE bit(D15 bit) */
		CMN_DelayInUS(3);
	}

	/* Control CPG_RST14 reg (offset 634h) : PD_VIDEO1 Reset / deassert all UNITs */
	CPG_SetResetCtrl(14, 0x03FD, 0x03FD);
	CMN_DelayInUS(3);

	/* Control CPG_CLK_ON25/26 reg (offset 460h/464h): PD_VIDEO1 clock on all UNITs */
	CPG_SetClockCtrl(25, 0x003B, 0x003B);
	CPG_SetClockCtrl(26, 0x001E, 0x001E);
	CMN_DelayInUS(3);

	/* Control CPG_RST7 reg (offset 618h) : set ICB_PD_VD1_RST_N */
	CPG_SetResetCtrl(7, 0x0008, 0x0008);
	CMN_DelayInUS(3);

	/* Control CPG_CLK_ON16/18 reg (offset 43Ch/444h): clock on ICB */
	CPG_SetClockCtrl(16, 0x0080, 0x0080);
	CPG_SetClockCtrl(18, 0x3F00, 0x3F00);
	CMN_DelayInUS(3);

	CMN_REG_Write32(0xa3600038, 0x00080000);   /* Clear PD_VIDEO1_IDLE_REQ bit(D0 bit)  */
	regval = CMN_REG_Read32(0xa360003C);       /* Get PD_VIDEO1_ACT_DONE bit(D11 bit) */

	while((regval & 0x0800) != 0x0800)
	{
		regval = CMN_REG_Read32(0xa360003C);   /* Get PD_VIDEO1_ACT_DONE bit(D11 bit) */
		CMN_DelayInUS(100);
	}
}

static void pcie_init (void)
{
	// set SYS register
	CMN_REG_Write32(0xA3F03094,0x00000000);
	CMN_REG_Write32(0xA3F03090,0x00000001);
	CMN_DelayInUS(10);

	/* assert RESET */
	CPG_SetResetCtrl(3, 0x1000, 0x0000);
	CMN_DelayInUS(2);

	/* disable supply clock */
	CPG_SetClockCtrl(4, 0x0007, 0x0000);
	CMN_DelayInUS(2);

	/* deassert RESET */
	CPG_SetResetCtrl(3, 0x1000, 0x1000);
	CMN_DelayInUS(2);

	/* enable supply clock */
	CPG_SetClockCtrl(4, 0x0007, 0x0007);
	CMN_DelayInUS(2);
}

static void div_init(void)
{
	int t;

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

}

static void sdi_emm_init(void)
{
	/* select SDI0 SDI1 EMM clock source(200MHz) */
	CPG_WriteReg(CPG_SDIEMM_SSEL, (CPG_SDIEMM_SSEL_SELSDI|CPG_SDIEMM_SSEL_WEN_SELSDI));

	/* enable supply clock */
	CPG_SetClockCtrl(3, 0x0fff, 0x0fff);
	CMN_DelayInUS(1);

	/* asset RESET */
	CPG_SetResetCtrl(3, 0x0007, 0x0000);
	CPG_WaitResetMon(0, CPG_RST_MON_SDI0, CPG_RST_MON_SDI0);
	CPG_WaitResetMon(0, CPG_RST_MON_SDI1, CPG_RST_MON_SDI1);
	CPG_WaitResetMon(0, CPG_RST_MON_EMM,  CPG_RST_MON_EMM);

	/* deasset RESET */
	CPG_SetResetCtrl(3, 0x0007, 0x0007);
	CPG_WaitResetMon(0, CPG_RST_MON_SDI0, 0);
	CPG_WaitResetMon(0, CPG_RST_MON_SDI1, 0);
	CPG_WaitResetMon(0, CPG_RST_MON_EMM,  0);
}

static void usb_init(void)
{
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
}

static void iic_init(void)
{
	/* asset RESET */
	/* IIC_GPA_PRESETN:IIC0,IIC1 */
	/* IIC_GPB_PRESETN:IIC2,IIC3 */
	CPG_SetResetCtrl(6, 0x0300, 0x0000);
	CMN_DelayInUS(1);

	/* disable supply clock */
	CPG_SetClockCtrl(9, 0x1000, 0x0000);
	CPG_SetClockCtrl(10, 0x1000, 0x0000);
	CMN_DelayInUS(1);

	/* deasset RESET */
	/* IIC_PCLK[0]:IIC0,IIC1 */
	/* IIC_PCLK[1]:IIC2,IIC3 */
	CPG_SetResetCtrl(6, 0x0300, 0x0300);
	CMN_DelayInUS(1);

	/* enable supply clock */
	/* IIC_PCLK[0]:IIC0,IIC1 */
	/* IIC_PCLK[1]:IIC2,IIC3 */
	CPG_SetClockCtrl(9, 0x1000, 0x1000);
	CPG_SetClockCtrl(10, 0x1000, 0x1000);
	CMN_DelayInUS(1);
}

static void ether_init(void)
{
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
}

static void pwm_init(void)
{
	CPG_SetClockCtrl(13, 0x0FF1, 0x0FF1);
	CPG_SetClockCtrl(14, 0x0FF1, 0x0FF1);
	CMN_DelayInUS(1);
	
	/* PWM0-15 reset assart */
	CPG_SetResetCtrl(6, 0x0030, 0x0000);
	CPG_WaitResetMon(0, CPG_RST_MON_PWM_0 | CPG_RST_MON_PWM_1, CPG_RST_MON_PWM_0 | CPG_RST_MON_PWM_1);
	
	/* PWM0-15 reset deassert */
	CPG_SetResetCtrl(6, 0x0030, 0x0030);
	CPG_WaitResetMon(0, CPG_RST_MON_PWM_0 | CPG_RST_MON_PWM_1, 0);
}

static void csi_init(void)
{
	CPG_SetClockCtrl(15, 0x3F03, 0x3F03);
	CMN_DelayInUS(1);
	
	/* CSI0-5 reset assart */
	CPG_SetResetCtrl(6, 0x00C0, 0x0000);
	CPG_WaitResetMon(0, CPG_RST_MON_CSI_1|CPG_RST_MON_CSI_2, CPG_RST_MON_CSI_1|CPG_RST_MON_CSI_2);
	
	/* CSI0-5 reset deassert */
	CPG_SetResetCtrl(6, 0x00C0, 0x00C0);
	CPG_WaitResetMon(0, CPG_RST_MON_CSI_1|CPG_RST_MON_CSI_2, 0);

	CPG_SetClockCtrl(4, 0x0007, 0x0007);
	CMN_DelayInUS(1);
	CPG_SetResetCtrl(3, 0x1000, 0x0000);
	CMN_DelayInUS(1);
	CPG_SetResetCtrl(3, 0x1000, 0x1000);
	CMN_DelayInUS(1);
}

static void tim_init(void)
{
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
}
	
static void dmaa_init(void)
{
	/* DMAA clock on */ 
	CPG_SetClockCtrl(1, 0x0800, 0x0800);
		
	/* DMAA reset assart */
	CPG_SetResetCtrl(1, 0x0080, 0x0000);
	CPG_WaitResetMon(0, CPG_RST_MON_DMAA,CPG_RST_MON_DMAA);
	
	/* DMAA reset deassert */
	CPG_SetResetCtrl(1, 0x0080, 0x0080);
	CPG_WaitResetMon(0, CPG_RST_MON_DMAA,0);
}

	
static void evk_cpg_init(void)
{
	do
	{	
		/**********************************/
		/*    Clock division control      */
		/**********************************/
		div_init();
		
		/**********************************/
		/*    SDI0 SDI1 EMM               */
		/**********************************/
		sdi_emm_init();
		
		/**********************************/
		/*    USB                         */
		/**********************************/
		usb_init();
		
		/**********************************/
		/*    IIC0-3                      */
		/**********************************/
		iic_init();
		
		/**********************************/
		/*    ETH0                        */
		/**********************************/
		ether_init();
		
		/**********************************/
		/*    PWM0-15                     */
		/**********************************/
		pwm_init();
		
		/**********************************/
		/*    CSI0-5                      */
		/**********************************/
		csi_init();
		
		/**********************************/
		/*    TIM0-31                     */
		/**********************************/
		tim_init();
		
		/**********************************/
		/*    DMAA                        */
		/**********************************/
		dmaa_init();

		/**********************************/
		/*    DRPA                        */
		/**********************************/
		drp_a_init();
		drp_b_init();
		
		/**********************************/
		/*    VCD                         */
		/**********************************/
		vcd_init();
		pcie_init();

	}
	while (0);
	return;
}

int32_t ilStartUpUART(void)
{
	return CMN_SUCCESS;
}

extern void _start(void);

void s_init(void)
{

	/* change the EL2 with ca53 core0 */
	if(3 == current_el()){

		armv8_switch_to_el2((u64)NULL, 0, 0, 0,
							(u64)_start,
							ES_TO_AARCH64);
	}
	
	return;
}

int board_early_init_f(void)
{
	/* move to active for SRAM in ETH0 */
	SYS_MoveToActiveSRAM(SYS_SRAM_ETH0);

	evk_cpg_init();
	evk_psc_init();
	
	/*- Release Ether PHY reset : PFC_P17_GPIO_DO -*/
	CMN_REG_Write32(0xB6250440, 0x10001);
	
	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_TEXT_BASE + 0x50000;

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

