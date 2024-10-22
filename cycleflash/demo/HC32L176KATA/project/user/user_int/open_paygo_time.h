/******************************************************************************
* ProjectName: 	GSM koolboks battery controler version1 projection
* FileName: 	open_paygo_time.h
* Description:	Manage OpenPAYGO time calculations
* 
* Author:		Poetry Cloud
* Date: 	 	2023/12/25
* Notes: 
* -----------------------------------------------------------------
* Histroy: V1.0 2023/12/25, Cloud create this file
* CPU:HC32L176KATA-LQFP64  FLASH:64K,128页每页512字节
******************************************************************************/
#ifndef __OPEN_PAYGO_TIME_H_
#define __OPEN_PAYGO_TIME_H_

#include "main_struct_typedef.h"
#include "main_file_define.h"


#define GREEN_TIME_2023_1_1_0_0_0       1672502400
#define GREEN_TIME_2070_1_1_0_0_0       3155731200

enum RTC_STATE
{
	RTC_NORMAL,
	RTC_WARNING,
	RTC_ERROR
};

typedef struct
{
    uint8_t     rtc_run_state;          // RTC运行状态
    _Bool       rtc_starting_test;      // RTC启动检测信号
    _Bool       rtc_run_test_flag;      // 运行时间检测信号
    uint8_t     rtc_read_error_cont;    // RTC运行状态检测计数
} rtc_state_t;

// 0 未触发， 1 触发
typedef struct
{
    uint32_t    lptimer_time_count;      // 每秒计时
    uint32_t    irq_10_min;
    uint32_t    irq_30_min;
    

    _Bool       lptimer_test_flag;       // 达到上面测试或者存储的信号
    _Bool       lptimer_flash_flag;         // 达到上面测试或者存储的信号

    _Bool       lptimer_led_1s_flag;         // LED的每秒信号
    _Bool       lptimer_1s_flag;         // rtc的每秒信号
    uint8_t     lptimer_2s_flag;         // rtc的2秒信号
    _Bool       timer_250ms_flag;
    uint8_t     lcd_light_up_time;       // lcd灯的亮屏时间

    uint16_t    timer_5_ms_count;       // 5ms计时

    // * 睡眠相关 **********************************************
    _Bool       sleep_open;            // 定时器状态 ，睡眠模式或者启动模式
    uint8_t     lp_sleep_count;          // 唤醒次数
} irq_time_t;


// 中断计数
extern volatile irq_time_t irq_time;
// rtc状态
extern volatile rtc_state_t rtc_state;

extern uint8_t rtc_error_log;


// 清零结构体rtc_state and irq_time
void open_paygo_struct_init(void);

// 比较存储时间的rtc_time_t结构体成员是否一样
_Bool compare_struct_rtc_time_t(const rtc_time_t *s1, const rtc_time_t *s2);
// 检测 rtc_time_t 组织成员是否有非法值
_Bool struct_rtc_time_invalid_value_examine(const rtc_time_t *s1);
// 计算格林时间到现在的秒数
uint32_t date_conversion_second(rtc_time_t *s1 );
// 格林时间秒数换算为日期
void second_conversion_date(rtc_time_t * date, time_t timestamp);
// 重新赋值RTC时间
void example_set_rtc_time(rtc_time_t * date);
// 获取直接获取RTC的秒数，验证RTC是否正常在运行
_Bool starting_run_examine_get_sec(uint32_t * data, _Bool temp_flag);
// 读取RTC，经过验证无误后赋值给结构体
_Bool open_paygo_time_read_tow_examine(rtc_time_t * date );

// 运行 验证时间
void open_paygo_time_run_examine(void);
// 验证RTC是否正常在走
uint8_t open_paygo_time_starting_run_examine(void);


#endif
