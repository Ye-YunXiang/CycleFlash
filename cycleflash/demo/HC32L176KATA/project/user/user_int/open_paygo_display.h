/******************************************************************************
* ProjectName: 	GSM koolboks battery controler version1 projection
* FileName: 	open_paygo_display.c 
* Description:	Manage the Open PAYGO state display
* 
* Author:		Poetry Cloud
* Date: 	 	2023/12/25
* Notes: 
* -----------------------------------------------------------------
* Histroy: V1.0 2023/12/25, Cloud create this file
* CPU:HC32L176KATA-LQFP64  FLASH:64K,128页每页512字节
******************************************************************************/

#ifndef __OPEN_PAYGO_DISABLE_H_
#define __OPEN_PAYGO_DISABLE_H_

#include "main_file_define.h"

#define UI_DISPLAY_TIMEE	20

enum DISPLAY_STATE{
    DISPLAY_FALSE,              // 啥也没有
    DISPLAY_SLOGAN,             // 开机广告
    DISPLAY_UP_LOADING,         // 开机等待
    DISPLAY_NO_ID,              // 没有id的时候，并且没有余额
    DISPLAY_NO_PAYGO,           // 有id但是没有余额的时候
    DISPLAY_RTC_ERROR,           // RTC坏掉了

    DISPLAY_ACTIVATED_TIME,      // 活动计时
    DISPLAY_WAIT,               // 等待
    DISPLAY_UNLOCK,              // unlok

    DISPLAY_NO_TOKEN,           // 输入的Token没用
    DISPLAY_TOKEN_DISABLE,      // 输入了disable码
    DISPLAY_TOKEN_ID,           // 输入了查看ID
    DISPLAY_TOKEN_COUNT,        // 输入查看count
    DISPLAY_TOKEN_COUNT_CLOSE,   // 输入了清除count

    DISPLAY_TOKEN_EFFICACIOUS,   // TOKEN输入有效

    DISPLAY_KEYBOARD_LOCK,        // 键盘上锁

    DISPLAY_TAMPER          // 后盖被打开
};


typedef struct display_state
{
    uint16_t display_day;
    uint8_t display_hour;
    // uint16_t all_display_min;
    
    _Bool display_state_flag;
    _Bool display_state_activity;
    enum  DISPLAY_STATE display_state_token_enum;
    enum  DISPLAY_STATE display_state_enum;
}display_state_t;

extern display_state_t display_state_manage;

// 初始显示管理结构体
void open_paygo_display_struct(void);

// 所有显示
void open_paygo_display_startup(enum DISPLAY_STATE state);

// 运行中管理显示
void open_paygo_display_running_manage(void);

// 运行过程用RTC坏了，显示
void open_paygo_display_rtc_error_run_manage(void);

// LCD的亮屏管理
void open_paygo_display_light_time_manage(void);

// led灯闪烁管理
void open_paygo_display_led_activity_blink_manage(void);

// 输入显示管理
void open_paygo_token_data_display(char temp_last_key,uint8_t temp_num);


#endif

