/******************************************************************************
* ProjectName: 	GSM koolboks battery controler version1 projection
* FileName: 	system_set_clock.h
* Description:	initialize clock and Timer
* 
* Author:		Poetry Cloud
* Date: 	 	2023/12/25
* Notes: 
* -----------------------------------------------------------------
* Histroy: V1.0 2023/12/25, Cloud create this file
* CPU:HC32L176KATA-LQFP64  FLASH:64K,128页每页512字节
******************************************************************************/

#ifndef __SYSTEM_CLOCK_SET_H
#define __SYSTEM_CLOCK_SET_H

#include "main_file_define.h"

// 这里定5ms
#define u16ArrValue_time0               30000    // 定时器
// 这里定1s
#define u16ArrValue_lptime_rum          32768    // 运行状态定时器，使用 RCL  ,分频256
// 这里定（128为一秒）
#define u16ArrValue_lptime_sleep        0    // 睡眠状态定时器，使用 RCL  ,分频256

#define TIMER_5ms_ON()                    Bt_M0_Run(TIM0)
#define TIMER_5ms_OFF()                   Bt_M0_Stop(TIM0)


void system_clock_configuration_init(void);
// 系统运行时时钟设置
void system_clock_sleep_configuration(void);
// 系统睡眠时时钟设置
void system_clock_sleep_configuration(void);

void system_timer0_configuration(void);            // 1s

// 打开timer0
void system_timer0_open(void);
// 关闭timer0
void system_timer0_close(void);

void system_run_lptimer_configuration(void);       // 1s
// 打开lpTime0中断
void system_run_lptimer_open(void);
// 关闭lpTime0中断
void system_run_lptimer_close(void);

void system_time_clock_config(void);


#endif //__SYSTEM_CLOCK_SET_H
