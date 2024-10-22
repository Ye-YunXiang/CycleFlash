/******************************************************************************
* ProjectName: 	GSM koolboks battery controler version1 projection
* FileName: 	system_led_manage.h
* Description:	Initialize LED
* 
* Author:		Poetry Cloud
* Date: 	 	2023/12/25
* Notes: 
* -----------------------------------------------------------------
* Histroy: V1.0 2023/12/25, Cloud create this file
* CPU:HC32L176KATA-LQFP64  FLASH:64K,128页每页512字节
******************************************************************************/
#ifndef __SYSTEM_LED_MANAGE_H_
#define __SYSTEM_LED_MANAGE_H_

#include "main_file_define.h"

#define RED_SATE_OPEN()       if(__sleep_mode.device_power_on_state == TRUE) Gpio_SetIO(LED_RED_PORT, LED_RED_PIN)

#define RED_SATE_CLOSE()      Gpio_ClrIO(LED_RED_PORT, LED_RED_PIN)

#define GREEN_SATE_OPEN()     if(__sleep_mode.device_power_on_state == TRUE) Gpio_SetIO(LED_GREEN_PORT, GREEN_PIN)

#define GREEN_SATE_CLOSE()    Gpio_ClrIO(LED_GREEN_PORT, GREEN_PIN)

void system_led_init(void);
// 红色闪烁一次
void system_led_blink_red_one(void);
// 绿灯闪烁一次
void system_led_blink_green_one(void);

#endif
