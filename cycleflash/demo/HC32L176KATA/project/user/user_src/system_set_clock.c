/******************************************************************************
* ProjectName: 	GSM koolboks battery controler version1 projection
* FileName: 	system_set_clock.c 
* Description:	initialize clock and Timer
* 
* Author:		Poetry Cloud
* Date: 	 	2023/12/25
* Notes: 
* -----------------------------------------------------------------
* Histroy: V1.0 2023/12/25, Cloud create this file
* CPU:HC32L176KATA-LQFP64  FLASH:64K,128页每页512字节
******************************************************************************/

#include "system_set_clock.h"

/********************************************************************
 ** \brief  初始运行状态的系统时钟
 ** \retval 无
********************************************************************/
void system_clock_configuration_init(void)
{
    stc_sysctrl_clk_cfg_t	StcClkCfg;
	
	///< RCH时钟不同频率的切换，需要先将时钟切换到RCL
	// 这里切换为外部RCL，顺便使能外部RCL
    Sysctrl_SetRCLTrim(SysctrlRclFreq32768);
    Sysctrl_SetRCLStableTime(SysctrlRclStableCycle64);
    Sysctrl_ClkSourceEnable(SysctrlClkRCL, TRUE);
    Sysctrl_SysClkSwitch(SysctrlClkRCL);
	Sysctrl_ClkSourceEnable(SysctrlClkXTL, TRUE);
	//Sysctrl_SysClkSwitch(SysctrlClkXTL);
    
    Sysctrl_SetRCHTrim(SysctrlRchFreq24MHz);
    Sysctrl_ClkSourceEnable(SysctrlClkRCH, TRUE);
    Sysctrl_SysClkSwitch(SysctrlClkRCH);
    
    ///< HCLK不超过24M：此处设置FLASH读等待周期为0 cycle
    Flash_WaitCycle(FlashWaitCycle0);
	
	
    /** 修改系统时钟分频 *************************************/
	StcClkCfg.enClkSrc    = SysctrlClkRCH;
    StcClkCfg.enHClkDiv   = SysctrlHclkDiv1;
    StcClkCfg.enPClkDiv   = SysctrlPclkDiv1;
 
    Sysctrl_ClkInit(&StcClkCfg);

    Flash_Init(6, TRUE);
}


/********************************************************************
 ** \brief  设置时钟频率为休眠状态
 ** \retval 无
********************************************************************/
 void system_clock_sleep_configuration(void)
 {
     stc_sysctrl_clk_cfg_t	StcClkCfg;
	
 	///< RCH时钟不同频率的切换，需要先将时钟切换到RCL
 	// 这里切换为外部RCL
     Sysctrl_SetRCLTrim(SysctrlRclFreq32768);
     Sysctrl_SetRCLStableTime(SysctrlRclStableCycle64);
     Sysctrl_ClkSourceEnable(SysctrlClkXTL, TRUE);
     
     Sysctrl_SetRCHTrim(SysctrlRchFreq4MHz);

     Sysctrl_ClkSourceEnable(SysctrlClkRCH, FALSE);
	 
	 Sysctrl_SysClkSwitch(SysctrlClkXTL);
    
     Flash_WaitCycle(FlashWaitCycle0);
	
     /** 修改系统时钟分频 *************************************/
 	StcClkCfg.enClkSrc    = SysctrlClkXTL;
     StcClkCfg.enHClkDiv   = SysctrlHclkDiv1;
     StcClkCfg.enPClkDiv   = SysctrlPclkDiv1;
 
     Sysctrl_ClkInit(&StcClkCfg);

     Flash_Init(1, TRUE);
 }


/********************************************************************
 ** \brief  初始化定时器timer0，定时5ms
 ** \retval 无
********************************************************************/
void system_timer0_configuration(void)
{
    uint16_t u16ArrValue;

	uint16_t                   u16CntValue;
//    uint8_t                    u8ValidPeriod;
    stc_bt_mode0_cfg_t     stcBtBaseCfg;
	
	DDL_ZERO_STRUCT(stcBtBaseCfg);				

	Sysctrl_SetPeripheralGate(SysctrlPeripheralBaseTim, TRUE);
	
    stcBtBaseCfg.enWorkMode = BtWorkMode0;                  //定时器模式
    stcBtBaseCfg.enCT       = BtTimer;                      //定时器功能，计数时钟为内部PCLK
    stcBtBaseCfg.enPRS      = BtPCLKDiv4;                 //PCLK
    stcBtBaseCfg.enCntMode  = Bt16bitArrMode;               //自动重载16位计数器/定时器
    stcBtBaseCfg.bEnTog     = FALSE;
    stcBtBaseCfg.bEnGate    = FALSE;
    stcBtBaseCfg.enGateP    = BtGatePositive;
    Bt_Mode0_Init(TIM0, &stcBtBaseCfg);                     //TIM0 的模式0功能初始化
    
    u16ArrValue = 0x10000 - u16ArrValue_time0;
    Bt_M0_ARRSet(TIM0, u16ArrValue);                        //设置重载值(ARR = 0x10000 - 周期)
    
    u16CntValue = 0x10000 - u16ArrValue_time0;
    Bt_M0_Cnt16Set(TIM0, u16CntValue);                      //设置计数初值
    
    // Bt_ClearIntFlag(TIM0,BtUevIrq);                         //清中断标志   
    // Bt_Mode0_EnableIrq(TIM0);                               //使能TIM0中断(模式0时只有一个中断)
    // EnableNvic(TIM0_IRQn, IrqLevel3, TRUE);                 //TIM0中断使能
    // // Bt_M0_Run(TIM0);
    // Bt_M0_Stop(TIM0);  // 为功耗考虑
}


// 打开timer0中断
void system_timer0_open(void)
{
	// 打开时钟
//	Sysctrl_SetPeripheralGate(SysctrlPeripheralBaseTim, TRUE);
	
    Bt_ClearIntFlag(TIM0,BtUevIrq);                         //清中断标志   
    Bt_Mode0_EnableIrq(TIM0);                               //使能TIM0中断(模式0时只有一个中断)
    EnableNvic(TIM0_IRQn, IrqLevel3, TRUE);                 //TIM0中断使能
    //Bt_M0_Run(TIM0);
    Bt_M0_Stop(TIM0);  // 为功耗考虑
}


// 关闭timer0中断
void system_timer0_close(void)
{	
    Bt_M0_Stop(TIM0);  // 为功耗考虑
    Bt_ClearIntFlag(TIM0,BtUevIrq);                         //清中断标志   
    Bt_Mode0_DisableIrq(TIM0);                               //使能TIM0中断(模式0时只有一个中断)
    EnableNvic(TIM0_IRQn, IrqLevel3, FALSE);                 //TIM0中断使能
	
//	Sysctrl_SetPeripheralGate(SysctrlPeripheralBaseTim, FALSE);
}


/********************************************************************
 ** \brief  初始化运行状态的LPTim0，使用 RCL,分频256，1S
 ** \retval 无
********************************************************************/
void system_run_lptimer_configuration(void)
{
	stc_lptim_cfg_t    stcLptCfg;    
    DDL_ZERO_STRUCT(stcLptCfg);

    ///< 使能LPTIM0 外设时钟
    Sysctrl_SetPeripheralGate(SysctrlPeripheralLpTim0, TRUE);
    
    stcLptCfg.enPrs	   = LptimPrsDiv1;
    stcLptCfg.enGate   = LptimGateLow;
    stcLptCfg.enGatep  = LptimGatePLow;
	stcLptCfg.enTcksel = LptimXtl;   // 使用外部RCL
    // stcLptCfg.enTcksel = LptimRcl;   // 使用内部RCL
    // stcLptCfg.enTogen  = LptimTogEnLow;
    stcLptCfg.enCt     = LptimTimerFun;           //计数器功能
    stcLptCfg.enMd     = LptimMode2;            //工作模式为模式2：自动重装载16位计数器/定时器
    stcLptCfg.u16Arr   = u16ArrValue_lptime_rum;                 //预装载寄存器值，计数初值 
    Lptim_Init(M0P_LPTIMER0, &stcLptCfg);

    // 清零结构体rtc_state and irq_time
	open_paygo_struct_init();    // * 复位清除计时的结构体 ************************************************
    
    Lptim_ClrItStatus(M0P_LPTIMER0);            //清除中断标志位
    Lptim_ConfIt(M0P_LPTIMER0, TRUE);           //允许LPTIMER中断    
    EnableNvic(LPTIM_0_1_IRQn, IrqLevel3, TRUE); 
    Lptim_Cmd(M0P_LPTIMER0, TRUE);   ///< 启动lptimer0运行
}

// 打开lpTime0中断
void system_run_lptimer_open(void)
{
	 ///< 使能LPTIM0 外设时钟
    Sysctrl_SetPeripheralGate(SysctrlPeripheralLpTim0, TRUE);
	
    Lptim_ClrItStatus(M0P_LPTIMER0);            //清除中断标志位
    Lptim_ConfIt(M0P_LPTIMER0, TRUE);           //允许LPTIMER中断    
    EnableNvic(LPTIM_0_1_IRQn, IrqLevel3, TRUE); 
    Lptim_Cmd(M0P_LPTIMER0, TRUE);   ///< 启动lptimer0运行
}

// 关闭lpTime0中断
void system_run_lptimer_close(void)
{
    Lptim_Cmd(M0P_LPTIMER0, FALSE);   ///< 关闭lptimer0运行
    Lptim_ClrItStatus(M0P_LPTIMER0);            //清除中断标志位
    EnableNvic(LPTIM_0_1_IRQn, IrqLevel3, FALSE); 
    Lptim_ConfIt(M0P_LPTIMER0, FALSE);           

    // 清零结构体rtc_state and irq_time
	open_paygo_struct_init();    // * 复位清除计时的结构体 ************************************************
    irq_time.sleep_open = TRUE;             // 打开睡眠模式
	
	 ///< 使能LPTIM0 外设时钟
    Sysctrl_SetPeripheralGate(SysctrlPeripheralLpTim0, TRUE);
}


/********************************************************************
 ** \brief  未定
 ** \retval 无
********************************************************************/
// void system_time_clock_config(void)
// {
// 	en_flash_waitcycle_t      enWaitCycle; 
//     stc_sysctrl_pll_cfg_t     stcPLLCfg;
    
//     DDL_ZERO_STRUCT(stcPLLCfg);								
    
//     enWaitCycle = FlashWaitCycle1;
//     Flash_WaitCycle(enWaitCycle);
    
//     stcPLLCfg.enInFreq    = SysctrlPllInFreq4_6MHz;     //RCH 4MHz
//     stcPLLCfg.enOutFreq   = SysctrlPllOutFreq36_48MHz;  //PLL 输出48MHz
//     stcPLLCfg.enPllClkSrc = SysctrlPllRch;              //输入时钟源选择RCH
//     stcPLLCfg.enPllMul    = SysctrlPllMul2;            //4MHz x 12 = 48MHz
//     Sysctrl_SetPLLFreq(&stcPLLCfg);
//     Sysctrl_SetPLLStableTime(SysctrlPllStableCycle16384);
//     Sysctrl_ClkSourceEnable(SysctrlClkPLL, TRUE);
    
//     Sysctrl_SysClkSwitch(SysctrlClkPLL);  				//时钟切换
// }
