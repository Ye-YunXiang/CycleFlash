#include "system_mode.h"

struct SLEEP_MODE_STRUCT __sleep_mode = {1, _SYSTEM_BOOT_SCREEN, 0, 0, 0};

// 系统所有外设初始化
void system_mode_peripherals_init(void)
{			
    //swd as gpio
    //Sysctrl_SetFunc(SysctrlSWDUseIOEn, TRUE);

    op_manage_load_activation_variables();              // 加载内存激活变量
            
    rtc_alarm_irq_close();                              // 关闭RTC闹钟
	
    system_run_lptimer_configuration();                 // 低功耗定时器初始化

    system_timer0_open();                               // 打开timer0中断

	//system_lpuart0_port_init();                         // lpuart引脚初始化

	system_lpuart0_config_init();                       // lpuart功能初始化 19200

    system_gsm_port_init();                             // GSM引脚初始化

	system_adc_bat_control_port_init();					// 检测电池ADC开关引脚初始化

	system_adc_port_init();								// ADC 采样端口初始化

	system_adc_run_init();								// 检测电池ADC初始化

    system_refrigerator_control_init();                 // 初始化冰箱使能引脚
	
	system_lcd1602_display_port_init();	                // LCD1602引脚初始化
	
    system_lcd1602_dispaly_config_init();				// LCD显示初始化
	
	system_matrix_key_scan_init();                      // 矩阵按键引脚初始化

  	system_led_init();                                  // LED显示引脚初始化
	
	open_paygo_display_struct();						// 初始显示管理结构体

    system_mode_ID_uart_select_open();                  // id烧录口是否打开
	
    __NOP();
}

// id烧录口是否打开
void system_mode_ID_uart_select_open(void)
{
    // 没有烧录ID，开启定时器判断UART是否空闲
    if(_system_paygo_state.paygo_id != TRUE)
    {
        system_uart0_port_init();                           // uart引脚初始化
		
        system_uart0_config_init();                         // uart功能初始化   9600

        TIMER_5ms_ON();     // 打开5ms定时器 为功耗考虑
    }
}


// 唤醒后m25上报初始化
void system_mode_rouse_reported_init(void)
{	
    op_manage_load_activation_variables();              // 加载内存激活变量

    rtc_alarm_irq_close();                               // 关闭RTC闹钟

    system_run_lptimer_configuration();                 // 低功耗定时器初始化

    system_timer0_open();                               // 打开timer0中断

	system_lpuart0_port_init();                         // lpuart引脚初始化

	system_lpuart0_config_init();                       // lpuart功能初始化 19200

    system_gsm_port_init();                             // GSM引脚初始化

	system_adc_bat_control_port_init();					// 检测电池ADC开关引脚初始化

	system_adc_port_init();								// ADC 采样端口初始化

	system_adc_run_init();								// 检测电池ADC初始化
	
	open_paygo_display_struct();						// 初始显示管理结构体
	
    __NOP();
}


// 系统唤醒或睡眠流程,TRUE为开机流程，FALSE为全局状态
void system_mode_wakeUP_or_sleep_prrocess(_Bool state)
{
    if(state == TRUE)
    {
        if(__sleep_mode.device_power_on_state != FALSE)
        {
            __sleep_mode.run_state++;
        }
        else
        {
            __sleep_mode.run_state = _SYSTEM_SLEEP;
        }
    }
    else
    {
        // 电平状态为开机，并且运行模式为SLEEP
        if(__sleep_mode.device_power_on_state == TRUE \
			&&(__sleep_mode.run_state == _SYSTEM_SLEEP || __sleep_mode.run_state == _SYSTEM_SLEEP_ROUSE))
		{
			__sleep_mode.run_state = _SYSTEM_BOOT_SCREEN;
		}
        // 电平状态为关机,并且检测完成信号打开
		else if( __sleep_mode.device_power_on_state == FALSE && __sleep_mode.state_flip_5v == TRUE)
		{
			__sleep_mode.run_state = _SYSTEM_SLEEP;
		}
    }
}


//  开机初始化所有引脚
void system_mode_IO_init(void)
{
	 ///< 打开GPIO外设时钟门控
    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);
	
    ///< 配置为端口输入（除LED端口外）
    M0P_GPIO->PADIR = 0XFFFF;
    M0P_GPIO->PBDIR = 0XFFFF;
    M0P_GPIO->PCDIR = 0XFFFF;
    M0P_GPIO->PDDIR = 0XFFFF;
    M0P_GPIO->PEDIR = 0XFFFF;
    M0P_GPIO->PFDIR = 0XFFFF;

    ///< 输入下拉（除 5V_ADC 端口以外）
    M0P_GPIO->PAPD = 0xFEFF;
    M0P_GPIO->PBPD = 0xFFFF;
    M0P_GPIO->PCPD = 0xFFFF;
    M0P_GPIO->PDPD = 0xFFFF;
    M0P_GPIO->PEPD = 0xFFFF;
    M0P_GPIO->PFPD = 0xFFFF;
}


//  进入睡眠设置引脚
void system_mode_sleep_IO_init(void)
{
	///< 打开GPIO外设时钟门控
    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);
    ///< 配置为端口输入
    M0P_GPIO->PADIR = 0XFFFF;
    M0P_GPIO->PBDIR = 0XFFFF;
    M0P_GPIO->PCDIR = 0XFFFF;
    M0P_GPIO->PDDIR = 0XFFFF;
    M0P_GPIO->PEDIR = 0XFFFF;
    M0P_GPIO->PFDIR = 0XFFFF;

    ///< 输入下拉（A08,C14,C15）
    M0P_GPIO->PAPD = 0xFEFF;
    M0P_GPIO->PBPD = 0xFFFF;
    M0P_GPIO->PCPD = 0xFFFF;
    M0P_GPIO->PDPD = 0xFFFF;
    M0P_GPIO->PEPD = 0xFFFF;
    M0P_GPIO->PFPD = 0xFFFF;
}


// 关机设置
void system_mode_power_off_set(void)
{
    System_LCD_Light_Disable();		// 关闭显示
    RED_SATE_CLOSE();				// 关闭红灯
    GREEN_SATE_CLOSE();				// 关闭绿灯
    ADC_TAMPER_CLOSE_TEST_IRQ();			// 关闭破解中断检测

    system_lcd1602_display_clear_dispaly();		// 清除屏幕
    
    ADC_TAMPER_CLOSE_TEST_IRQ();       //关闭所有后盖检测

    ADC_OPEN_5V_ON_TEST_IRQ();		// 打开开机检测
}

// 判断进入深度睡眠
void system_mode_judge_depp_sleep_open(void)
{
    // 进入深度睡眠
    //if(__sleep_mode.state_flip_5v == TRUE && ADV_5V_GET_STATE() == FALSE)
	if(ADV_5V_GET_STATE() == FALSE)
    {
        __sleep_mode.state_flip_5v = FALSE;
		
        // 关闭ADC的所有相关时钟
        system_adc_sleep();		
		
		// 关闭LPuart
		if(_system_paygo_state.paygo_id != TRUE)
		{
			system_uart0_config_off();
		}
		
		// 关闭LPuart端口
		system_lpuart0_port_close();
        // 关闭LPuart0功能
        system_lpuart0_config_close();
        // 关闭timer0
        system_timer0_close();
        // 先关LPTime时器
        system_run_lptimer_close();     

		system_mode_IO_init();
		
		// 打开RTC闹钟
        rtc_alarm_irq_open();
		
		delay1ms(20);
	
		Lpm_GotoSleep(FALSE);		
    }
}
