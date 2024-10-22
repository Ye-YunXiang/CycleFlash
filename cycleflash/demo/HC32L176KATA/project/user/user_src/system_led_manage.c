/******************************************************************************
* ProjectName: 	GSM koolboks battery controler version1 projection
* FileName: 	system_led_manage.c 
* Description:	Initialize LED
* 
* Author:		Poetry Cloud
* Date: 	 	2023/12/25
* Notes: 
* -----------------------------------------------------------------
* Histroy: V1.0 2023/12/25, Cloud create this file
* CPU:HC32L176KATA-LQFP64  FLASH:64K,128页每页512字节
******************************************************************************/

#include "system_led_manage.h"

/********************************************************************
 ** \brief  LED灯引脚初始化
 ** \retval 无
********************************************************************/
void system_led_init(void)
{
	stc_gpio_cfg_t 	stc_out_led_init;
	
	Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);
	
	stc_out_led_init.enDir		= GpioDirOut;
	stc_out_led_init.enDrv		= GpioDrvL;
	stc_out_led_init.enPu 		= GpioPuDisable;
	stc_out_led_init.enPd 		= GpioPdDisable;
	stc_out_led_init.enCtrlMode	= GpioAHB;
	
	Gpio_Init(LED_RED_PORT, LED_RED_PIN, &stc_out_led_init);
	Gpio_Init(LED_GREEN_PORT, GREEN_PIN, &stc_out_led_init);

    Gpio_ClrIO(LED_RED_PORT, LED_RED_PIN);
    Gpio_ClrIO(LED_GREEN_PORT, GREEN_PIN);
}


/********************************************************************
 ** \brief  红灯闪烁一次
 ** \retval 无
********************************************************************/
void system_led_blink_red_one(void)
{
	if(__sleep_mode.device_power_on_state != FALSE)
	{
		RED_SATE_OPEN();
		
		delay1ms(250);

		RED_SATE_CLOSE();

		delay1ms(250);
	}
}


/********************************************************************
 ** \brief  绿灯闪烁一次
 ** \retval 无
********************************************************************/
void system_led_blink_green_one(void)
{
	if(__sleep_mode.device_power_on_state != FALSE)
	{
		GREEN_SATE_OPEN();
		
		delay1ms(250);

		GREEN_SATE_CLOSE();

		delay1ms(250);
	}
}

